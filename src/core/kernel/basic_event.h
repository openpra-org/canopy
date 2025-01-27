#pragma once

#include "core/node.h"

#include <sycl/sycl.hpp>

namespace canopy::kernel {

    enum SampledBitWidth {
        one_bit = 1,
        two_bits = 2,
        four_bits = 4,
        eight_bits = 8,
        sixteen_bits = 16,
        thirtytwo_bits = 32,
        sixtyfour_bits = 64,
    };
    template<typename prob_t_, typename bitpack_t_, typename size_t_>
    class basic_event {
        canopy::basic_event<prob_t_, bitpack_t_> *basic_events_;
        const size_t_ num_basic_events_;
        const sample_shape<size_t_> sample_shape_;

    public:
        basic_event(
                canopy::basic_event<prob_t_, bitpack_t_> *basic_events,
                const size_t_ &num_basic_events,
                const sample_shape<size_t_> &sample_shape) : basic_events_(basic_events),
                                                            num_basic_events_(num_basic_events),
                                                            sample_shape_(sample_shape) {}

        // Philox PRNG implementation
        // Constants for Philox 2x32-10 PRNG
        static constexpr uint32_t PHILOX_W32A = 0x9E3779B9;
        static constexpr uint32_t PHILOX_W32B = 0xBB67AE85;
        static constexpr uint32_t PHILOX_M4x32A = 0xD2511F53;
        static constexpr uint32_t PHILOX_M4x32B = 0xCD9E8D57;

        struct philox128_state {
            uint32_t x[4];
        };

        // Philox 4x32-10 round function
        static inline void philox_round(uint32_t &k0, uint32_t &k1, philox128_state *counters) {
            // Multiply
            const uint64_t product0 = static_cast<uint64_t>(PHILOX_M4x32A) * counters->x[0];
            const uint64_t product1 = static_cast<uint64_t>(PHILOX_M4x32B) * counters->x[2];

            // Split into high and low parts
            philox128_state hi_lo;
            hi_lo.x[0] = static_cast<uint32_t>(product0 >> 32);
            hi_lo.x[1] = static_cast<uint32_t>(product0);
            hi_lo.x[2] = static_cast<uint32_t>(product1 >> 32);
            hi_lo.x[3] = static_cast<uint32_t>(product1);

            // Mix in the key
            counters->x[0] = hi_lo.x[2] ^ counters->x[1] ^ k0;
            counters->x[1] = hi_lo.x[3];
            counters->x[2] = hi_lo.x[0] ^ counters->x[3] ^ k1;
            counters->x[3] = hi_lo.x[1];

            // Bump the key
            k0 += PHILOX_W32A;
            k1 += PHILOX_W32B;
        }


        // Generate 4 random uint32_t values using Philox 4x32-10
        static void philox_generate(const philox128_state *seeds, philox128_state *results) {
            // Key
            uint32_t k0 = 382307844;
            uint32_t k1 = 293830103;

            // Counter
            philox128_state counters = *seeds;

            // Number of rounds; Philox 4x32 uses 10 rounds
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);
            philox_round(k0, k1, &counters);

            *results = counters;
        }

        // Generate a random floating-point number in the range [0, 1)
        template<SampledBitWidth width=SampledBitWidth::four_bits>
        static bitpack_t_ sample(const philox128_state *seeds, const prob_t_ probability) {
            philox128_state results;
            philox_generate(seeds, &results);
            // Use one of the generated values; we can also combine them if higher precision is needed
            static constexpr prob_t_ inv_uint32_max = static_cast<prob_t_>(1.0) / static_cast<prob_t_>(UINT32_MAX + 1ULL);

            bitpack_t_ sample = bitpack_t_(0);
            sample |= (static_cast<prob_t_>(results.x[0]) * inv_uint32_max < probability ? 1 : 0) << 0;
            sample |= (static_cast<prob_t_>(results.x[1]) * inv_uint32_max < probability ? 1 : 0) << 1;
            sample |= (static_cast<prob_t_>(results.x[2]) * inv_uint32_max < probability ? 1 : 0) << 2;
            sample |= (static_cast<prob_t_>(results.x[3]) * inv_uint32_max < probability ? 1 : 0) << 3;
            return sample;
        }

        struct sampler_args {
            uint32_t index_id, event_id, batch_id, bitpack_idx, iteration;
            prob_t_ probability;
        };

        static bitpack_t_ generate(const sampler_args &args) {
            bitpack_t_ bitpacked_sample = bitpack_t_(0);
            static constexpr std::uint32_t num_samples_bit_round_ = 4;
            static constexpr std::uint32_t num_bits_in_dtype_ = sizeof(bitpack_t_) * 8;
            static constexpr std::uint32_t num_bitpack_rounds_ = num_bits_in_dtype_ / num_samples_bit_round_;
            #pragma unroll
            for (auto i = 0; i < num_bitpack_rounds_; ++i) {
                philox128_state seeds;
                seeds.x[0] = args.index_id + 1;
                seeds.x[1] = args.event_id + 1;
                seeds.x[2] = args.batch_id + 1;
                seeds.x[3] = args.bitpack_idx + args.iteration << (num_bits_in_dtype_ << i);
                bitpacked_sample |= sample<four_bits>(&seeds, args.probability) << (num_samples_bit_round_ * i);
            }
            return bitpacked_sample;
        }

        void operator()(const sycl::nd_item<3> &item, const uint32_t iteration) const {
            sampler_args args = {
                .event_id    = static_cast<uint32_t>(item.get_global_id(0)),
                .batch_id    = static_cast<uint32_t>(item.get_global_id(1)),
                .bitpack_idx = static_cast<uint32_t>(item.get_global_id(2)),
                .iteration = iteration,
            };

            // Bounds checking
            if (args.event_id >= num_basic_events_ || args.batch_id >= sample_shape_.batch_size || args.bitpack_idx >= sample_shape_.bitpacks_per_batch) {
                return;
            }

            args.probability = basic_events_[args.event_id].probability;
            args.index_id = static_cast<uint32_t>(basic_events_[args.event_id].index);

            // Calculate the index within the generated_samples buffer
            const size_t_ index = args.batch_id * sample_shape_.bitpacks_per_batch + args.bitpack_idx;

            // Store the bitpacked samples into the buffer
            bitpack_t_ *output = basic_events_[args.event_id].buffer;
            const bitpack_t_ bitpack_value = generate(args);
            output[index] = bitpack_value;
        }

        static sycl::nd_range<3> get_range(const size_t_ num_events,
                                           const sycl::range<3> &local_range,
                                           const sample_shape<size_t_> &sample_shape_) {
            // Compute global range
            size_t global_size_x = num_events;
            size_t global_size_y = sample_shape_.batch_size;
            size_t global_size_z = sample_shape_.bitpacks_per_batch;

            // Adjust global sizes to be multiples of the corresponding local sizes
            global_size_x = ((global_size_x + local_range[0] - 1) / local_range[0]) * local_range[0];
            global_size_y = ((global_size_y + local_range[1] - 1) / local_range[1]) * local_range[1];
            global_size_z = ((global_size_z + local_range[2] - 1) / local_range[2]) * local_range[2];

            sycl::range<3> global_range(global_size_x, global_size_y, global_size_z);

            return {global_range, local_range};
        }
    };
}// namespace scram::canopy::kernel
