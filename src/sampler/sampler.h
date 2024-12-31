#pragma once

#include <string>
#include <vector>

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
                                                              const std::size_t n_sample_packs_per_probability);

    template <typename BitpackDataType> static std::vector<BitpackDataType> _compute_bit_positions();

    template <typename ProbType, typename BitpackDataType, typename SampleType = double>
    static std::vector<std::vector<std::vector<BitpackDataType>>>
    generate_bernoulli(const std::vector<std::vector<ProbType>> &probs,
                       const std::size_t n_sample_packs_per_probability);

};

}
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
