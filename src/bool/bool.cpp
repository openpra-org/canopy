#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

#include <iomanip>
#include <random>
#include <CL/sycl.hpp>

#include "utils/random.h"
#include "utils/profiler.h"
#include "utils/stats.h"

// todo:: add doxygen comments

// TODO:: define a templated type, with concrete overrides for uint8_t, uint16_t, uint32_t, uint64_t, etc...
using sampling_distribution_type = float; /// typically 32-bit wide
using tally_float_type = float; /// typically 80-bit wide, larger than double_t
using bit_vector_type = uint_fast8_t;

// TODO:: encode repeating symbols
template<typename T>
using products = std::vector<T>;

// for expression F = ab'c + a'b + bc' + a'bc' + aa'baacc'ab, with:
//
// s = 3 unique symbols (excluding negations)
// n = 2*s = 6 total symbols (including negations)
// m = 5 products
//
// using w=8-bit words to encode n=6 symbols, starting from the MSB, and,
// m = 5 words, we fill out a m=5 element vector,
// where each element encodes one product
// -------------------------------------------------
// |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
// -------------------------------------------------
// |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
// -------------------------------------------------
static inline void set_F(products<bit_vector_type> &F, const size_t index) {
    // first element: encodes ab'c
    // -------------------------------------------------
    // |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  1  |  1  |  1  |
    // -------------------------------------------------
    F[index] = 0b01100111;

    // second element: encodes a'b
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  1  |  1  |  1  |
    // -------------------------------------------------
    F[index+1] = 0b10011111;

    // third element: encodes bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  1  |  0  |  1  |  1  |  0  |  1  |  1  |
    // -------------------------------------------------
    F[index+2] = 0b11011011;

    // fourth element: encodes a'bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  0  |  1  |  1  |
    // -------------------------------------------------
    F[index+3] = 0b10011011;

    // fifth element: encodes aa'aacc'
    // repeated terms have no effect
    /** TODO:: [optimization]
     * consider an encoding scheme such that when mutually exclusive events (such as a, a') are both set to 1, this
     * product is masked out of the computation. You can develop this logic by considering how non-existing mutually
     * exclusive events such as (b, b') are being treated in this case. They are both being set to 0, and will be
     * evaluated as such later in the eval step. They would have been removed had this term been encoded like a sparse
     * matrix (removing 0s). So a simple step would be to set the whole term to 0b00000000 when (x, x') is encountered,
     * leaving the empty set to be removed by the sparse-matrix encoding step that should eventually follow.
     *
     * another expensive step is to minimize this term (using boolean reduction) before encoding the term, which is the
     * more general method for the step explained above.
     *
     * another alternative is to bitwise XOR the term with the bitshift of itself (by 1).
     *
     * **/
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  0  |  1  |  1  |  0  |  0  |  1  |  1  |
    // -------------------------------------------------
    F[index+4] = 0b00010011;
}

/**
 * @brief Performs a bitwise AND reduction on all bits of a uint8_t value.
 *
 * This function returns `true` if all bits in the input byte are set to `1`,
 * and `false` otherwise.
 *
 * @param x The uint8_t value to be reduced via bitwise AND.
 * @return `true` if all bits are `1`, `false` otherwise.
 */
template<typename T>
constexpr inline bool bitwise_and_all(T x) noexcept {
    return x == std::numeric_limits<T>::max();
}

// return word-sized object instead of 1-bit.
static inline bool eval(const auto &F, const auto &sampled_x) {
    // todo:: [optimization]
    // todo:: confirm that any_of will yield and terminate the calculation as soon as any row evals to true.
    // todo:: confirm that any_of does not enforce any execution order for which rows are evaluated first.
    // todo:: together, these two constraints can leverage faster out-of-order, pre-emptive execution
    // todo:: ensure that this intent is communicated/articulated in the sycl kernels or stl functions
    return std::ranges::any_of(F, [&](bit_vector_type row) {
        return (sampled_x | row) == 0b11111111;
    });
}

// a | b =
static void fill_x(std::vector<sampling_distribution_type> &dist_x) {
    dist_x[0] = 1e-3;  // P(a)
    dist_x[1] = 1e-4;  // P(b)
    dist_x[2] = 1e-5;  // P(c)
}

template<typename float_type>
static float_type compute_exact_prob_F(std::vector<sampling_distribution_type> &dist_x) {
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

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<sampling_distribution_type> uniform_dist(0.0, 1.0);

// TODO:: returned object should be aligned with cache-line
// maybe return 64-bit width value here?
static inline bit_vector_type generate_sample(const std::vector<sampling_distribution_type> &dist_x) {

    const auto sampled =
            static_cast<bit_vector_type>(
                    (uniform_dist(gen) > dist_x[0] ? 0b01000000 : 0b10000000) |
                    (uniform_dist(gen) > dist_x[1] ? 0b00010000 : 0b00100000) |
                    (uniform_dist(gen) > dist_x[2] ? 0b00000100 : 0b00001000));

    return sampled;
}

int main() {
    // for expression F = ab'c + a'b + bc' + a'bc' + aa'aacc'a, with
    // m = 5 products
    const size_t m_products = 5;
    // n = 10 duplicates
    const size_t n_duplicates = 200000;
    const size_t F_size = m_products * n_duplicates;
    std::vector<bit_vector_type> F(F_size);

    // set the function and duplicate the products multiple times
    // todo:: std lambda syntax
    for (auto i = 0; i < n_duplicates; i++) {
        set_F(F, i*m_products);
    }

    // define the probabilities for X
    const size_t s_symbols = 3;
    auto dist_x = std::vector<sampling_distribution_type>(s_symbols);
    fill_x(dist_x);

    const size_t num_samples = 1e7;
    const size_t num_rands = num_samples * s_symbols;
    const auto rand_numbers = canopy::utils::random::generate_vector<sampling_distribution_type>(num_rands);

    // Create SYCL buffers
    cl::sycl::queue queue;
    cl::sycl::buffer<bit_vector_type, 1> F_buf(F.data(), cl::sycl::range<1>(F_size));
    cl::sycl::buffer<sampling_distribution_type, 1> dist_x_buf(dist_x.data(), cl::sycl::range<1>(s_symbols));
    cl::sycl::buffer<sampling_distribution_type, 1> rand_numbers_buf(rand_numbers.data(), cl::sycl::range<1>(num_rands));
    cl::sycl::buffer<int, 1> result_buf(cl::sycl::range<1>(1));

    const auto profiler = canopy::utils::Profiler([&]() {
        // Initialize result to zero
        {
            auto acc = result_buf.get_access<cl::sycl::access::mode::discard_write>();
            acc[0] = 0;
        }

        queue.submit([&](cl::sycl::handler& cgh) {
            auto F_acc = F_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto dist_x_acc = dist_x_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto rand_numbers_acc = rand_numbers_buf.get_access<cl::sycl::access::mode::read>(cgh);
            auto result_acc = result_buf.get_access<cl::sycl::access::mode::read_write>(cgh);

            cgh.parallel_for<class sample_eval_kernel>(cl::sycl::range<1>(num_samples), [=](cl::sycl::id<1> idx) {
                size_t i = idx[0];

                // Generate sample
                sampling_distribution_type rand_a = rand_numbers_acc[i * s_symbols];
                sampling_distribution_type rand_b = rand_numbers_acc[i * s_symbols + 1];
                sampling_distribution_type rand_c = rand_numbers_acc[i * s_symbols + 2];

                bit_vector_type sample = static_cast<bit_vector_type>(
                        (rand_a > dist_x_acc[0] ? 0b01000000 : 0b10000000) |
                        (rand_b > dist_x_acc[1] ? 0b00010000 : 0b00100000) |
                        (rand_c > dist_x_acc[2] ? 0b00000100 : 0b00001000)
                );

                // Evaluate F
                bool tally = false;
                for (size_t j = 0; j < F_size; j++) {
                    bit_vector_type row = F_acc[j];
                    if ((sample | row) == 0b11111111) {
                        tally = true;
                        break; // Early exit
                    }
                }

                // Atomic increment
                if (tally) {
                    cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::device, cl::sycl::access::address_space::global_space> count_atomic(result_acc[0]);
                    count_atomic.fetch_add(1);
                }
            });
        });
        queue.wait_and_throw();
    }, 2, 0, "F=ab'c+a'b+bc', x=3, term<width>=uint_fast8_t, products=1e6, samples=1e7").run();


    // Retrieve result
    size_t count = 0;
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::read>();
        count = acc[0];
    }

    const auto known_P = compute_exact_prob_F<tally_float_type>(dist_x);

    std::cout << std::setprecision(15) << std::scientific;
    std::cout<<"P(a): "<<dist_x[0]<<"\nP(b): "<<dist_x[1]<<"\nP(c): "<<dist_x[2]<<std::endl;


    const auto stats = canopy::utils::SummaryStatistics<tally_float_type, size_t>(count, num_samples, known_P);
    std::cout<<stats;

    std::cout<<profiler; // print the profiler summary

    return 0;
}