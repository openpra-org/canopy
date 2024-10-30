#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include <random>
#include <CL/sycl.hpp>

#include "utils/profiler.h"
#include "utils/stats.h"
#include "utils/bits.h"
#include "utils/types.h"
#include "utils/device_info.h"

/**
 * @brief Number of unique symbols (variables) in expression F.
 *
 * For expression \( F = ab'c + a'b + bc' + a'bc' + aa'aacc'a \), the number of symbols is 3 (a, b, c).
 */
static constexpr const size_t s_symbols = 3;

/**
 * @brief Number of product terms in expression F.
 *
 * For expression \( F = ab'c + a'b + bc' + a'bc' + aa'aacc'a \), there are 5 product terms.
 */
static constexpr const size_t m_products = 5;

/**
 * @brief Number of duplicates for each product term.
 *
 * Used to create multiple copies of the product terms to simulate a larger function.
 *
 * @note n_duplicates = 200,000
 */
static constexpr const size_t n_duplicates = 200000;

/**
 * @brief Total number of product terms after duplication.
 *
 * Calculated as \( F\_size = m\_products \times n\_duplicates \).
 *
 * @note F_size = 1,000,000 (i.e., 1e6)
 */
static constexpr const std::size_t F_size = m_products * n_duplicates;

/**
 * @typedef known_event_probabilities
 * @brief Array type to store the known probabilities of events (variables).
 *
 * The size of the array is given by `s_symbols`.
 */
using known_event_probabilities = std::array<tally_float_type, s_symbols>;

/**
 * @brief Alias for an array of product terms.
 *
 * @tparam T The type of the elements (e.g., bit_vector_type).
 * @tparam size The number of elements in the array.
 */
template<typename T, size_t size>
using products = std::array<T, size>;

/**
 * @brief Global array to store the encoded product terms of expression F.
 *
 * Declared as a static global array to give the compiler hints for compile-time optimizations.
 *
 * @note Investigate whether this has any meaningful performance implications.
 */
static products<bit_vector_type, F_size> F;

/**
 * @brief Known probabilities of the events (variables) a, b, c.
 *
 * @details Stored as an array of type `known_event_probabilities`.
 *
 * The probabilities are:
 * - P(a) = 1e-3
 * - P(b) = 1e-4
 * - P(c) = 1e-5
 *
 * @note Probabilities of variables (events) in expression F.
 */
static const constexpr known_event_probabilities Px = {
        1e-3, ///< P(a)
        1e-4, ///< P(b)
        1e-5  ///< P(c)
};

/**
 * @brief Number of Monte Carlo samples to generate.
 *
 * @note num_samples = 10,000,000 (i.e., 1e7)
 */
static constexpr const size_t num_samples = 1e7;

/**
 * @brief Initializes the global array `F` with encoded product terms of expression F.
 *
 * @details For expression \( F = ab'c + a'b + bc' + a'bc' + aa'aacc'a \):
 * - `s` = 3 unique symbols (excluding negations)
 * - `n` = 2*s = 6 total symbols (including negations)
 * - `m` = 5 product terms
 *
 * Using `w` = 8-bit words to encode `n` = 6 symbols, starting from the MSB.
 * Each product term is encoded into a bit vector, where each bit represents the inclusion of a symbol.
 *
 * The bit encoding is as follows:
 * - Bit 7 (MSB): a
 * - Bit 6: a'
 * - Bit 5: b
 * - Bit 4: b'
 * - Bit 3: c
 * - Bit 2: c'
 * - Bit 1: Unused
 * - Bit 0: Unused
 *
 * -------------------------------------------------
 * |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * -------------------------------------------------
 * |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
 * -------------------------------------------------
 *
 * The function duplicates the product terms multiple times to fill the array `F_`.
 *
 * @tparam size The size of the `products` array.
 * @param F_ Reference to the products array to initialize.
 * @param index The starting index in the array where the product terms should be placed.
 *
 * @note Repeated symbols in a product term have no effect.
 *
 * @example
 * @code
 * // Initialize the products array F
 * set_F<F_size>(F, 0);
 * @endcode
 */
template<size_t size>
static inline void set_F(products<bit_vector_type, size> &F_, const size_t index) {
    // first element: encodes ab'c
    // -------------------------------------------------
    // |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  1  |  1  |  1  |
    // -------------------------------------------------
    F_[index] = 0b01100111;

    // second element: encodes a'b
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  1  |  1  |  1  |
    // -------------------------------------------------
    F_[index+1] = 0b10011111;

    // third element: encodes bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  1  |  0  |  1  |  1  |  0  |  1  |  1  |
    // -------------------------------------------------
    F_[index+2] = 0b11011011;

    // fourth element: encodes a'bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  0  |  1  |  1  |
    // -------------------------------------------------
    F_[index+3] = 0b10011011;

    // fifth element: encodes aa'aacc'
    // repeated terms have no effect
    /**
     * @note [Optimization]
     * Consider an encoding scheme such that when mutually exclusive events (such as a, a') are both set to 1, this
     * product is masked out of the computation. You can develop this logic by considering how non-existing mutually
     * exclusive events such as (b, b') are being treated in this case. They are both being set to 0, and will be
     * evaluated as such later in the eval step. They would have been removed had this term been encoded like a sparse
     * matrix (removing 0s). So a simple step would be to set the whole term to 0b00000000 when (x, x') is encountered,
     * leaving the empty set to be removed by the sparse-matrix encoding step that should eventually follow.
     *
     * Another expensive step is to minimize this term (using boolean reduction) before encoding the term, which is the
     * more general method for the step explained above.
     *
     * Another alternative is to bitwise XOR the term with the bitshift of itself (by 1).
     */
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  0  |  1  |  1  |  0  |  0  |  1  |  1  |
    // -------------------------------------------------
    F_[index+4] = 0b00110011;
}

/**
 * @brief Computes the exact probability of the expression F based on known event probabilities.
 *
 * @tparam float_type The floating-point type to use for calculations.
 * @param dist_x The known probabilities of events (variables) a, b, and c.
 * @return The computed probability of expression F.
 *
 * @details
 * The function calculates the probability of the logical expression:
 * \( F = ab'c + a'b + bc' + a'bc' \).
 *
 * The probabilities are computed using the inclusion-exclusion principle.
 *
 * @note Assumes that variables are independent.
 *
 * @example
 * @code
 * known_event_probabilities Px = {0.001, 0.0001, 0.00001};
 * auto Pf = compute_exact_prob_F<float>(Px);
 * @endcode
 */
template<typename float_type>
static constexpr float_type compute_exact_prob_F(const known_event_probabilities &dist_x) {
    const auto Pa  = static_cast<float_type>(dist_x[0]);
    const auto Pb  = static_cast<float_type>(dist_x[1]);
    const auto Pc  = static_cast<float_type>(dist_x[2]);
    const auto P1  = static_cast<float_type>(1.0);
    const auto Pa_ = P1 - Pa;
    const auto Pb_ = P1 - Pb;
    const auto Pc_ = P1 - Pc;

    // Individual probabilities
    const float_type Pab_c = Pa * Pb_ * Pc;
    const float_type Pa_b  = Pa_ * Pb;
    const float_type Pbc_  = Pb * Pc_;

    // Intersection of A'B and BC' (i.e., A'BC')
    const float_type Pa_bANDPbc_ = Pa_ * Pb * Pc_;

    // Compute P(F)
    const float_type Pf = Pab_c + Pa_b + Pbc_ - Pa_bANDPbc_;
    assert(Pf <= 1.0 && Pf >= 0.0);
    return Pf;
}

/**
 * @brief Samples random truth assignments for variables a, b, c based on their known probabilities.
 *
 * @param to_sample_from The known probabilities of events a, b, c.
 * @param sampled_x Output vector to store the sampled truth assignments encoded as bit vectors.
 * @param seed Random seed for reproducibility (default is 372).
 *
 * @details
 * Parallelized using OpenMP to accelerate sampling. Each thread uses its own random number generator
 * to avoid race conditions and ensure thread safety.
 *
 * Generates `sampled_x.size()` samples, where each sample is a bit vector representing the truth assignments
 * of variables a, b, c and their negations. Each variable is assigned true (1) with its respective probability.
 * The resulting bit vector encodes the presence of each variable or its negation using the same bit encoding
 * as used in the product terms in `F`.
 *
 * @note OpenMP is used to parallelize the loop over samples.
 *
 * @example
 * @code
 * known_event_probabilities Px = {0.001, 0.0001, 0.00001};
 * std::vector<bit_vector_type> sampled_x(1000000);
 * sample_and_assign_truth_values(Px, sampled_x);
 * @endcode
 */
static void sample_and_assign_truth_values(const known_event_probabilities &to_sample_from, std::vector<bit_vector_type> &sampled_x, const std::size_t seed = 372) {
    // Parallelize the sampling using OpenMP
    #pragma omp parallel num_threads(1) default(none) shared(seed, to_sample_from, sampled_x)
    {
        // Each thread creates its own random number generator and distribution
        int thread_num = omp_get_thread_num();
        std::mt19937 stream(seed + thread_num);
        std::uniform_real_distribution<sampling_distribution_type> uniform(0, 1);

        // Distribute the loop iterations among threads
        #pragma omp for
        for(bit_vector_type &i : sampled_x) {
            bit_vector_type sample1 = (0b10000000 >> (2 * 0 + (uniform(stream) > to_sample_from[0])));
            bit_vector_type sample2 = (0b10000000 >> (2 * 1 + (uniform(stream) > to_sample_from[1])));
            bit_vector_type sample3 = (0b10000000 >> (2 * 2 + (uniform(stream) > to_sample_from[2])));
            i = (sample1 | sample2 | sample3);
        }
    }
}
/**
 * @brief Entry point of the program.
 *
 * @details
 * The program performs a Monte Carlo simulation to estimate the probability of expression F
 * by sampling truth assignments of variables a, b, c based on their known probabilities,
 * and evaluating F for each sample using SYCL for parallel computation.
 * It also computes the exact probability for comparison.
 */
int main() {

    /// Sets the function and duplicates the products multiple times.
    for (auto i = 0; i < n_duplicates; i++) {
        set_F<F_size>(F, i * m_products);
    }

    /**
     * @brief Sampling of truth assignments.
     *
     * Will be filled by sampling `num_samples` items.
     * Once filled, it represents the truth assignments for all variables in x, each sampled using their known
     * probability distribution [Px]. In essence, we are moving the X sampling logic out of the compute kernels since it
     * is not as efficient to move the support matrix (rand_numbers) (typically float32 or float64) to device, then
     * sample out the truth values, only to discard the support matrix immediately.
     *
     * The benefits here are:
     * - Fewer host to device transfers.
     * - Fewer malloc blocks on device.
     * - Less work done on device (potential parallelism loss), but it can improve cache coherence if the kernel
     *   is small, both in terms of instruction cache, as well as data-fetches due to cache misses.
     *
     * In the future, as the width of `bit_vector_type` grows from the current (8,16,32,64-bit) blocks to k-bit blocks,
     * the assumptions made here will be challenged. In such cases, we will develop logic to traverse the k-bit wide
     * sampled_X vector as we sample over (k * num_samples) floats from the support vector. The question about
     * generating random numbers inside the kernel is a valid one, and requires more work.
     */
    //canopy::bits8<num_samples> sampled_x;
    std::vector<bit_vector_type> sampled_x(num_samples);

    std::cout << canopy::utils::Profiler([&]() {
        sample_and_assign_truth_values(Px, sampled_x);
    }, 1, 0, "generate random number vector, num_samples=1e7, float32").run();

    cl::sycl::queue queue;
    auto dev = queue.get_device();
    auto device_info = DeviceInfo(dev);
    std::cout<<device_info<<std::endl;

    cl::sycl::buffer<bit_vector_type, 1> F_buf(F.data(), cl::sycl::range<1>(F_size));
    cl::sycl::buffer<bit_vector_type, 1> sampled_x_buf(sampled_x.data(), cl::sycl::range<1>(sampled_x.size()));
    cl::sycl::buffer<int, 1> result_buf(cl::sycl::range<1>(1));

    const auto profiler = canopy::utils::Profiler([&]() {
        // Initialize result to zero
        {
            auto acc = result_buf.get_access<cl::sycl::access::mode::discard_write>();
            acc[0] = 0;
        }
        // Define work-group and sub-group sizes based on hardware capabilities
        const size_t sub_group_size = 32;    // From your hardware info
        const size_t work_group_size = 256;  // Multiple of sub-group size and divides max work-group size
        const size_t num_compute_units = dev.get_info<cl::sycl::info::device::max_compute_units>(); // From your hardware info

        // Calculate number of work-groups to match the number of compute units
        const size_t num_work_groups = num_compute_units;
        std::cout<<"num_work_groups: "<<num_compute_units<<std::endl;
        // Adjust global range accordingly
        const size_t global_range = num_work_groups * work_group_size;

        // Calculate samples per work-item
        const size_t total_work_items = global_range;
        std::cout<<"total_work_items: "<<total_work_items<<std::endl;
        const size_t samples_per_work_item = (num_samples + total_work_items - 1) / total_work_items;
        std::cout<<"samples_per_work_item: "<<samples_per_work_item<<std::endl;
        // Divide F_size among work-groups
        const size_t F_per_group = (F_size + num_work_groups - 1) / num_work_groups;

        std::cout<<"F_per_group: "<<F_per_group<<std::endl;

        queue.submit([&](cl::sycl::handler &cgh) {
            auto F_acc = F_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto sampled_x_acc = sampled_x_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto result_acc = result_buf.get_access<cl::sycl::access::mode::read_write>(cgh);

            // Local memory for storing a chunk of F
            cl::sycl::local_accessor<bit_vector_type, 1> local_F(F_per_group, cgh);

            cgh.parallel_for<class sample_eval_kernel>(
                    cl::sycl::nd_range<1>(cl::sycl::range<1>(global_range), cl::sycl::range<1>(work_group_size)),
                    [=](cl::sycl::nd_item<1> item) {
                        size_t global_id = item.get_global_linear_id();
                        size_t local_id = item.get_local_linear_id();
                        size_t group_id = item.get_group_linear_id();

                        // Each work-group loads its portion of F into local memory
                        size_t F_start = group_id * F_per_group;
                        size_t F_end = cl::sycl::min(F_start + F_per_group, F_size);
                        size_t F_chunk_size = F_end - F_start;

                        // Load F into local memory in a parallel fashion
                        for (size_t i = local_id; i < F_chunk_size; i += work_group_size) {
                            local_F[i] = F_acc[F_start + i];
                        }

                        // Synchronize to ensure all local_F is loaded
                        item.barrier(cl::sycl::access::fence_space::local_space);

                        // Each work-item processes multiple samples
                        size_t sample_start = global_id * samples_per_work_item;
                        size_t sample_end = cl::sycl::min(sample_start + samples_per_work_item, num_samples);

                        int local_count = 0;

                        for (size_t i = sample_start; i < sample_end; ++i) {
                            if (i >= num_samples) break; // Guard against overrun

                            const auto sample = sampled_x_acc[i];
                            bool sample_satisfies_F = false;

                            // Evaluate F over the chunk
                            for (size_t j = 0; j < F_chunk_size; ++j) {
                                if ((sample | local_F[j]) == 0b11111111) {
                                    sample_satisfies_F = true;
                                    break; // Early exit
                                }
                            }

                            if (sample_satisfies_F) {
                                local_count += 1;
                            }
                        }

                        // Reduce local counts within the work-group
                        // Use local memory for reduction
                        cl::sycl::local_accessor<int, 1> local_sums;

                        // Initialize local sum
                        if (local_id == 0) {
                            local_sums[0] = 0;
                        }

                        item.barrier(cl::sycl::access::fence_space::local_space);

                        // Atomic add local count to local sum
                        cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed,
                                cl::sycl::memory_scope::work_group,
                                cl::sycl::access::address_space::local_space>
                                local_sum_atomic(local_sums[0]);
                        local_sum_atomic.fetch_add(local_count);

                        item.barrier(cl::sycl::access::fence_space::local_space);

                        // Work-group leader updates the global result
                        if (local_id == 0) {
                            cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed,
                                    cl::sycl::memory_scope::device,
                                    cl::sycl::access::address_space::global_space>
                                    result_atomic(result_acc[0]);
                            result_atomic.fetch_add(local_sums[0]);
                        }
                    });
        });
        queue.wait_and_throw();
    }, 1, 0, "Optimized evaluation of F with blocking").run();
    // Retrieve result
    size_t count = 0;
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::read>();
        count = acc[0];
    }

    const auto known_P = compute_exact_prob_F<tally_float_type>(Px);
    std::cout << std::setprecision(15) << std::scientific;
    std::cout << "P(a): " << Px[0] << "\nP(b): " << Px[1] << "\nP(c): " << Px[2] << std::endl;
    const auto stats = canopy::utils::SummaryStatistics<tally_float_type, size_t>(count, num_samples, known_P);
    std::cout << stats;
    std::cout << profiler; // print the profiler summary

    return 0;
}
