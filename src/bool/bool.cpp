#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>
#include <random>
#include <CL/sycl.hpp>

#include "utils/profiler.h"
#include "utils/stats.h"
#include "bits.h"

/**
 * @typedef sampling_distribution_type
 * @brief The data type used for sampling distribution in random number generation.
 * @details Typically a 32-bit floating-point type.
 */
using sampling_distribution_type = float_t;

/**
 * @typedef tally_float_type
 * @brief The floating-point data type used for tallying probabilities.
 * @details Typically a 32-bit floating-point type.
 */
using tally_float_type = float_t;

/**
 * @typedef bit_vector_type
 * @brief The unsigned integer type used to represent bit vectors.
 * @details Using the fastest available unsigned integer type of at least 8 bits.
 */
using bit_vector_type = uint_fast8_t;

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
    F_[index+4] = 0b00010011;
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
 * Generates `sampled_x.size()` samples, where each sample is a bit vector representing the truth assignments
 * of variables a, b, c and their negations. Each variable is assigned true (1) with its respective probability.
 * The resulting bit vector encodes the presence of each variable or its negation using the same bit encoding
 * as used in the product terms in `F`.
 *
 * @example
 * @code
 * known_event_probabilities Px = {0.001, 0.0001, 0.00001};
 * std::vector<bit_vector_type> sampled_x(1000000);
 * sample_and_assign_truth_values(Px, sampled_x);
 * @endcode
 */
static void sample_and_assign_truth_values(const known_event_probabilities &to_sample_from, std::vector<bit_vector_type> &sampled_x, const std::size_t seed = 372) {
    std::random_device rd;
    std::mt19937 stream(seed);
    std::uniform_real_distribution<sampling_distribution_type> uniform(0, 1);

    // Use std::generate to fill the vector with random numbers
    std::generate(sampled_x.begin(), sampled_x.end(), [&]() {
        const size_t x_max = 3;
        bit_vector_type sample = 0b00000000;
        for(auto o = 0; o < x_max; o++) {
            const auto shift_by = 2 * o + (uniform(stream) > Px[o]);
            sample |= (0b10000000 >> shift_by);
        }
        return sample;
    });
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

    // Create SYCL buffers
    cl::sycl::queue queue;
    cl::sycl::buffer<bit_vector_type, 1> F_buf(F.data(), cl::sycl::range<1>(F_size));
    cl::sycl::buffer<bit_vector_type, 1> sampled_x_buf(sampled_x.data(), cl::sycl::range<1>(sampled_x.size()));
    cl::sycl::buffer<int, 1> result_buf(cl::sycl::range<1>(1));

    const auto profiler = canopy::utils::Profiler([&]() {
        // Initialize result to zero
        {
            auto acc = result_buf.get_access<cl::sycl::access::mode::discard_write>();
            acc[0] = 0;
        }

        queue.submit([&](cl::sycl::handler& cgh) {
            auto F_acc = F_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto sampled_x_acc = sampled_x_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto result_acc = result_buf.get_access<cl::sycl::access::mode::read_write>(cgh);

            cgh.parallel_for<class sample_eval_kernel>(cl::sycl::range<1>(num_samples), [=](cl::sycl::id<1> idx) {
                const size_t i = idx[0];
                const auto sample = sampled_x_acc[i];

                // Evaluate F
                for (auto j = 0; j < F_size; j++) {
                    /**
                     * @note todo: [optimization]
                     * - confirm that any_of will yield and terminate the calculation as soon as any row evals to true.
                     * - confirm that any_of does not enforce any execution order for which rows are evaluated first.
                     * - together, these two constraints can leverage faster out-of-order, pre-emptive execution
                     * - ensure that this intent is communicated/articulated in the sycl kernels or stl functions
                    **/
                    if ((sample | F_acc[j]) == 0b11111111) {
                        // Atomic increment the tally and exit
                        cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::device,
                                cl::sycl::access::address_space::global_space>
                                count_atomic(result_acc[0]);
                        count_atomic.fetch_add(1);
                        break; // Early exit
                    }
                }

            /**
             * @brief Implicants / Products
             * -----------
             * A product (F_acc[j]) is an implicant for F if ANY assignment (sample_x[i] | F_acc[j]) evals to 0b11111111.
             * This is because in the SOP representation, any product evaluating to ⊤ will make F ⊤. This because
             * F, as defined, is in SOP form. So, for F in the form F = p1 + p2 + p3, if any product p evals to ⊤,
             * F evals to ⊤. You can think of operations in (sample_x[i] | F_acc[j]) as the AND-PLANE in an AND-OR
             * PLA (programmable logic array).
             *
             * We expect that a well-formed product will be an implicant. What do I mean by well-formed?
             *  (1) no mutually exclusive events i.e. aa'
             *  (2) no always empty events: abcd, where d was not part of X, so it never gets a truth assignment
             *
             * So what's left now are products that take at-least *some* input from the sampling vector X, which
             * makes the product evaluate to ⊤. of course, during sampling, we might not have encountered such an
             * input, *yet*.
             *
             * In a more straight forward sense, the assignment for X that makes any product ⊤ is the definition
             * of the product itself, i.e. a product a'b'c evals to ⊤ when a=⊥, b=⊥, c=⊤. This is what product is.
             **/

             /**
             * @brief Prime Implicants
             * -----------------
             * A prime implicant is an implicant that cannot be further reduced by removing any literals without
             * ceasing to be an implicant. That is, it is a minimal implicant in terms of the number of literals.
             *
             * In the function F(a, b) = ab + a'c,
             *
             *     - The product term **P = ab** is a prime implicant; removing any literal (either **a** or **b**)
             *       results in a term that is not an implicant of F because it would evaluate to true in cases where F
             *       is false.
             *
             *     - However, the term **abc** is not a prime implicant because it can be reduced to **ab**
             *       (by eliminating **c**) while still being an implicant of F. Therefore, **abc** is not minimal and
             *       thus not prime.
             **/

             /**
             * @brief Essential Prime Implicants
             * ---------------------------
             * A prime implicant is `essential` if there exists an assignment X for that prime implicant that makes
             * F eval to ⊤ while all other products make F eval to ⊥ for that assignment. In other words, An
             * essential prime implicant is a prime implicant that covers at least one minterm (truth-table row
             * where the function evaluates to true) that is not covered by any other prime implicant. The key is
             * that there are outputs that only this prime implicant can account for.
             *
             * In other words, an essential prime implicant is a prime implicant that covers at least one minterm
             * (a combination of variable assignments where F is true) that is not covered by any other prime implicant.
             * These minterms are exclusively covered by this prime implicant.
             *
             * Consider F(a, b, c) with the truth table where F is true for minterms m1, m2, and m5.
             *      Let:
             *          - **P1** be a prime implicant covering minterms m1 and m5.
             *          - **P2** be a prime implicant covering minterms m2 and m5.
             *
             *      Minterm m1 is only covered by **P1**, so **P1** is an essential prime implicant.
             *      Minterm m2 is only covered by **P2**, so **P2** is also an essential prime implicant.
             *      Minterm m5 is covered by both **P1** and **P2**, so it does not affect the essentiality of either implicant.
             **/

             /**
             * @brief Minimal Sum-of-Products (SoP
             * ------------------------
             *
             * Every minimal SoP consists of a sum of prime implicants. This does not imply that a minimal SoP
             * contains *all* prime implicants. Rather, a minimal SoP contains *all* essential prime implicants, but
             * it may also contain non-essential prime implicants.
             *
             * Therefore, A minimal SoP expression for F is an expression that represents F using the
             * fewest possible product terms (and thus the fewest literals). It consists of all essential prime
             * implicants and, if necessary, additional prime implicants to cover all minterms where F is true.
             **/
            });
        });
        queue.wait_and_throw();
    }, 1, 0, "F=ab'c+a'b+bc', x=3, term<width>=uint_fast8_t, products=1e6, samples=1e7").run();

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
