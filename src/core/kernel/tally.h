#pragma once

#include "core/node.h"
#include <sycl/sycl.hpp>

namespace canopy::kernel {
    template<typename prob_t_, typename bitpack_t_, typename size_t_>
    class tally {
        tally_event<bitpack_t_> *tally_nodes_;
        const size_t_ num_tallies_;
        const sample_shape<size_t_> sample_shape_;

    public:
        // Constructor
        tally(tally_event<bitpack_t_> *tally_nodes,
              const size_t_ &num_tallies,
              const sample_shape<size_t_> sample_shape)
            : tally_nodes_(tally_nodes),
              num_tallies_(num_tallies),
              sample_shape_(sample_shape) {}

        // Returns an nd_range over 3 dimensions:
        //   X-dimension -> number of tally nodes
        //   Y-dimension -> batch size
        //   Z-dimension -> bitpacks per batch
        static sycl::nd_range<3> get_range(const size_t_ num_tallies,
                                           const sycl::range<3> &local_range,
                                           const sample_shape<size_t_> &shape) {
            auto new_local_range = local_range;
            new_local_range[0] = 1;
            size_t global_x = (num_tallies + new_local_range[0] - 1) / new_local_range[0] * new_local_range[0];
            size_t global_y = (shape.batch_size + new_local_range[1] - 1) / new_local_range[1] * new_local_range[1];
            size_t global_z = (shape.bitpacks_per_batch + new_local_range[2] - 1) / new_local_range[2] * new_local_range[2];

            return {sycl::range<3>(global_x, global_y, global_z), new_local_range};
        }

        template<typename prob_vec_t_>
        static void update_tally_stats(tally_event<bitpack_t_> &tally, const prob_t_ &total_bits) {
            const prob_t_ bernoulli_mean = static_cast<prob_t_>(tally.num_one_bits) / total_bits;
            const prob_t_ bernoulli_variance = bernoulli_mean * (1.0 - bernoulli_mean);
            const prob_t_ bernoulli_std_error = sycl::sqrt(bernoulli_variance / total_bits);

            // Z-scores: first = 1.96 for ~95% CI, second = 2.58 for ~99% CI
            const auto z_scores = sycl::double2(1.959963984540054, 2.5758293035489004);
            const sycl::double2 margins = z_scores * bernoulli_std_error;
            const prob_vec_t_ raw_confidence_intervals(
                bernoulli_mean - margins.x(),
                bernoulli_mean + margins.x(),
                bernoulli_mean - margins.y(),
                bernoulli_mean + margins.y()
            );
            const prob_vec_t_ clamped_cis = sycl::clamp(raw_confidence_intervals, 0.0, 1.0);
            tally.mean = bernoulli_mean;
            tally.std_err = bernoulli_std_error;
            tally.ci = clamped_cis;
        }

        // 3D kernel for popcount
        void operator()(const sycl::nd_item<3> &item, const uint32_t iteration) const {
            // Map global IDs to (tally_id, batch_id, bitpack_id)
            const size_t tally_id = item.get_global_id(0);
            const size_t batch_id = item.get_global_id(1);
            const size_t bitpack_id = item.get_global_id(2);

            // Bounds check
            if (tally_id >= num_tallies_ ||
                batch_id >= sample_shape_.batch_size ||
                bitpack_id >= sample_shape_.bitpacks_per_batch) {
                return;
            }

            // Each work-item processes exactly one bitpack for this tally
            const std::size_t idx = batch_id * sample_shape_.bitpacks_per_batch + bitpack_id;
            const std::size_t local_sum = sycl::popcount(tally_nodes_[tally_id].buffer[idx]);

            // Use an intra-group reduction so we only do one atomic add per group
            const std::size_t group_sum = sycl::reduce_over_group(item.get_group(), local_sum, sycl::plus<>());

            // Have the subgroup/group leader accumulate bit counts in global memory
            if (item.get_local_linear_id() == 0) {
                auto atomic_bits = sycl::atomic_ref<
                        std::size_t,
                        sycl::memory_order::relaxed,
                        sycl::memory_scope::device,
                        sycl::access::address_space::global_space>(
                        tally_nodes_[tally_id].num_one_bits);
                atomic_bits.fetch_add(group_sum);
            }

            // Now, if each tally is handled by exactly one work-group,
            // we can compute final statistics on the group leader thread:
            // (This only works correctly if no other groups write to this tally.)
            if (item.get_local_linear_id()) {
                return;
            }

            // Barrier so that all work-items in this group have finished updating
            // num_one_bits for this tally.
            item.barrier(sycl::access::fence_space::local_space);

            static constexpr std::size_t num_bits_in_dtype_ = sizeof(bitpack_t_) * 8;
            const auto total_bits = static_cast<std::size_t>(iteration)
                              * static_cast<std::size_t>(sample_shape_.batch_size)
                              * static_cast<std::size_t>(sample_shape_.bitpacks_per_batch)
                              * static_cast<std::size_t>(num_bits_in_dtype_);

            update_tally_stats<sycl::double4>(tally_nodes_[tally_id], static_cast<prob_t_>(total_bits));
        }
    };

}// namespace scram::canopy::kernel
