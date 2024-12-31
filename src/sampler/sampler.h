#pragma once

#include <vector>
#include <omp.h>
#include <random>

namespace canopy::random {

class sampler {
  public:
    template <typename DataType> static std::size_t _compute_bits_in_dtype() { return sizeof(DataType) * 8; }

    template <typename BitpackDataType> struct SampleShape {
        std::size_t num_events{};
        std::size_t batch_size{};
        std::size_t num_bits_per_pack{};
        std::size_t num_bits{};
        std::vector<std::size_t> sample_shape;
        std::vector<std::size_t> samples_reshaped_shape;
    };

    template <typename ProbType, typename BitpackDataType>
    static SampleShape<BitpackDataType> _compute_sample_shape(const std::vector<std::vector<ProbType>> &probs,
                                                              const std::size_t n_sample_packs_per_probability) {
        SampleShape<BitpackDataType> ss;

        ss.num_events = probs.size();
        ss.batch_size = probs[0].size();
        ss.num_bits_per_pack = _compute_bits_in_dtype<BitpackDataType>();
        ss.num_bits = ss.num_bits_per_pack * n_sample_packs_per_probability;

        ss.sample_shape = {ss.num_events, ss.batch_size, ss.num_bits};
        ss.samples_reshaped_shape = {ss.num_events, ss.batch_size, n_sample_packs_per_probability,
                                     ss.num_bits_per_pack};

        return ss;
    }

    template <typename BitpackDataType> static std::vector<BitpackDataType> _compute_bit_positions() {
        std::size_t num_bits = _compute_bits_in_dtype<BitpackDataType>();
        std::vector<BitpackDataType> positions(num_bits);
        for (std::size_t i = 0; i < num_bits; ++i) {
            positions[i] = static_cast<BitpackDataType>(i);
        }
        return positions;
    }

    template <typename ProbType, typename BitpackDataType, typename SampleType = double>
    static std::vector<std::vector<std::vector<BitpackDataType>>>
    generate_bernoulli(const std::vector<std::vector<ProbType>> &probs,
                       const std::size_t n_sample_packs_per_probability) {
        // Step 1: Compute Sample Shapes
        auto ss = _compute_sample_shape<ProbType, BitpackDataType>(probs, n_sample_packs_per_probability);
        const auto num_events = ss.num_events;
        const auto batch_size = ss.batch_size;
        const auto num_bits_per_pack = ss.num_bits_per_pack;
        const auto num_bits = ss.num_bits;

        // Step 2: Prepare Probability Data
        // Flatten the 2D probability vector into a 1D vector for easy access
        std::vector<ProbType> probs_flat;
        probs_flat.reserve(num_events * batch_size);
        for (const auto &event_probs : probs) {
            probs_flat.insert(probs_flat.end(), event_probs.begin(), event_probs.end());
        }

        // Step 3: Initialize Random Number Generators for Each Thread
        const int max_threads = omp_get_max_threads();
        std::vector<std::mt19937> rng_engines(max_threads);

        // Seed each RNG differently using a seed sequence
        std::seed_seq seed_seq{std::random_device{}(), std::random_device{}(), std::random_device{}()};
        std::vector<uint32_t> seeds(max_threads);
        seed_seq.generate(seeds.begin(), seeds.end());

        for (int i = 0; i < max_threads; ++i) {
            rng_engines[i].seed(seeds[i]);
        }

        // Step 4: Generate Bernoulli Samples
        // Initialize the samples vector
        // Dimensions: [num_events][batch_size][num_bits]
        std::vector<std::vector<std::vector<BitpackDataType>>> samples(
            num_events, std::vector<std::vector<BitpackDataType>>(batch_size, std::vector<BitpackDataType>(num_bits)));

        // Parallelize the sample generation
        #pragma omp parallel for collapse(2) schedule(static)
        for (std::size_t i = 0; i < num_events; ++i) {
            for (std::size_t j = 0; j < batch_size; ++j) {

                // Each thread gets its own RNG
                int thread_id = omp_get_thread_num();
                std::mt19937 &rng_engine = rng_engines[thread_id];
                std::uniform_real_distribution<SampleType> uniform_dist(0.0, 1.0);

                std::size_t idx_probs = i * batch_size + j;
                ProbType prob = probs_flat[idx_probs];

                for (std::size_t k = 0; k < num_bits; ++k) {
                    SampleType rnd = uniform_dist(rng_engine);
                    samples[i][j][k] = rnd < prob ? 1 : 0;
                }
            }
        }

        // Step 5: Compute Bit Positions (This is already thread-safe)
        auto positions = _compute_bit_positions<BitpackDataType>();

        // Step 6: Perform Bit-Packing
        // Initialize the packed bits vector
        // Dimensions: [num_events][batch_size][n_sample_packs_per_probability]
        std::vector<std::vector<std::vector<BitpackDataType>>> packed_bits(
            num_events, std::vector<std::vector<BitpackDataType>>(
                            batch_size, std::vector<BitpackDataType>(n_sample_packs_per_probability)));

        // Parallelize the bit-packing
        #pragma omp parallel for collapse(2) schedule(static)
        for (std::size_t i = 0; i < num_events; ++i) {
            for (std::size_t j = 0; j < batch_size; ++j) {
                for (std::size_t p = 0; p < n_sample_packs_per_probability; ++p) {
                    BitpackDataType packed_value = 0;

                    for (std::size_t b = 0; b < num_bits_per_pack; ++b) {
                        std::size_t k = p * num_bits_per_pack + b;
                        // if (k >= num_bits) {
                        //     // This should not happen, but guard against out-of-bounds
                        //     break;
                        // }

                        BitpackDataType sample_bit = samples[i][j][k];
                        BitpackDataType position = positions[b];

                        BitpackDataType shifted_bit = sample_bit << position;
                        packed_value |= shifted_bit;
                    }

                    packed_bits[i][j][p] = packed_value;
                }
            }
        }

        // Step 7: Return the Packed Bits
        return packed_bits;
    }
};

} // namespace canopy::random
  // template <typename ProbType, typename BitpackDataType, typename SampleType = double>
  // static std::vector<std::vector<std::vector<BitpackDataType>>> generate_bernoulli(cl::sycl::queue& queue,
  //     const std::vector<std::vector<ProbType>>& probs,
  //     const std::size_t n_sample_packs_per_probability,
  //     const std::string& name = ""
  // ) {
  //     // Compute sample shapes
  //     auto ss = _compute_sample_shape<ProbType, BitpackDataType>(probs, n_sample_packs_per_probability);
  //     const auto num_events = ss.num_events;
  //     const auto batch_size = ss.batch_size;
  //     const auto num_bits_per_pack = ss.num_bits_per_pack;
  //     const auto num_bits = ss.num_bits;
  //
  //     // Allocate device memory for probabilities and copy data
  //     ProbType* d_probs = cl::sycl::malloc_device<ProbType>(num_events * batch_size, queue);
  //     for (std::size_t i = 0; i < num_events; ++i) {
  //         queue.memcpy(d_probs + i * batch_size, probs[i].data(), batch_size * sizeof(ProbType));
  //     }
  //
  //     // Allocate device memory for samples
  //     BitpackDataType* d_samples = cl::sycl::malloc_device<BitpackDataType>(num_events * batch_size * num_bits,
  //     queue);
  //
  //     // Generate uniform random numbers using oneMKL
  //     const std::size_t total_random_numbers = num_events * batch_size * num_bits;
  //     SampleType* d_random_numbers = cl::sycl::malloc_device<SampleType>(total_random_numbers, queue);
  //     oneapi::mkl::rng::philox4x32x10 engine(queue, 777);
  //     oneapi::mkl::rng::uniform<SampleType> distr(0.0, 1.0);
  //     oneapi::mkl::rng::generate(distr, engine, total_random_numbers, d_random_numbers);
  //
  //     // Kernel to generate Bernoulli samples
  //     queue.submit([&](cl::sycl::handler& cgh) {
  //         cgh.parallel_for<class generate_samples_kernel>(
  //             cl::sycl::range<3>(num_events, batch_size, num_bits),
  //             [=](cl::sycl::id<3> idx) {
  //                 std::size_t i = idx[0];
  //                 std::size_t j = idx[1];
  //                 std::size_t k = idx[2];
  //
  //                 std::size_t idx_probs = i * batch_size + j;
  //                 std::size_t idx_samples = (i * batch_size + j) * num_bits + k;
  //                 std::size_t idx_random = idx_samples;
  //
  //                 SampleType prob = static_cast<SampleType>(d_probs[idx_probs]);
  //                 SampleType rnd = d_random_numbers[idx_random];
  //
  //                 d_samples[idx_samples] = rnd < prob ? 1 : 0;
  //             }
  //         );
  //     });
  //
  //     // Compute bit positions and copy to device
  //     auto positions_host = _compute_bit_positions<BitpackDataType>();
  //     BitpackDataType* d_positions = cl::sycl::malloc_device<BitpackDataType>(num_bits_per_pack, queue);
  //     queue.memcpy(d_positions, positions_host.data(), num_bits_per_pack * sizeof(BitpackDataType));
  //
  //     // Allocate memory for packed bits
  //     const std::size_t total_packed_bits = num_events * batch_size * n_sample_packs_per_probability;
  //     BitpackDataType* d_packed_bits = cl::sycl::malloc_device<BitpackDataType>(total_packed_bits, queue);
  //
  //     // Kernel for bit-packing
  //     queue.submit([&](cl::sycl::handler& cgh) {
  //         cgh.parallel_for<class bit_packing_kernel>(
  //             cl::sycl::range<3>(num_events, batch_size, n_sample_packs_per_probability),
  //             [=](cl::sycl::id<3> idx) {
  //                 std::size_t i = idx[0];
  //                 std::size_t j = idx[1];
  //                 std::size_t p = idx[2];
  //
  //                 BitpackDataType packed_value = 0;
  //                 for (std::size_t b = 0; b < num_bits_per_pack; ++b) {
  //                     std::size_t k = p * num_bits_per_pack + b;
  //                     std::size_t idx_samples = (i * batch_size + j) * num_bits + k;
  //
  //                     BitpackDataType sample_bit = d_samples[idx_samples];
  //                     BitpackDataType position = d_positions[b];
  //
  //                     BitpackDataType shifted_bit = sample_bit << position;
  //                     packed_value |= shifted_bit;
  //                 }
  //
  //                 std::size_t idx_packed = (i * batch_size + j) * n_sample_packs_per_probability + p;
  //                 d_packed_bits[idx_packed] = packed_value;
  //             }
  //         );
  //     });
  //
  //     // Copy packed bits back to host
  //     std::vector<std::vector<std::vector<BitpackDataType>>> packed_bits(num_events,
  //         std::vector<std::vector<BitpackDataType>>(batch_size,
  //             std::vector<BitpackDataType>(n_sample_packs_per_probability)));
  //     {
  //         BitpackDataType* h_packed_bits = new BitpackDataType[total_packed_bits];
  //         queue.memcpy(h_packed_bits, d_packed_bits, total_packed_bits * sizeof(BitpackDataType)).wait();
  //
  //         for (std::size_t i = 0; i < num_events; ++i) {
  //             for (std::size_t j = 0; j < batch_size; ++j) {
  //                 for (std::size_t p = 0; p < n_sample_packs_per_probability; ++p) {
  //                     std::size_t idx_packed = (i * batch_size + j) * n_sample_packs_per_probability + p;
  //                     packed_bits[i][j][p] = h_packed_bits[idx_packed];
  //                 }
  //             }
  //         }
  //
  //         delete[] h_packed_bits;
  //     }
  //
  //     // Free device memory
  //     cl::sycl::free(d_probs, queue);
  //     cl::sycl::free(d_samples, queue);
  //     cl::sycl::free(d_random_numbers, queue);
  //     cl::sycl::free(d_positions, queue);
  //     cl::sycl::free(d_packed_bits, queue);
  //
  //     return packed_bits;
  // }
// };
//
// } // namespace canopy::random
