#include <iostream>
#include <vector>

#include <CL/sycl.hpp>

#include <sampler/Sampler.h>

// TODO:: define a templated type, with concrete overrides for uint8_t, uint16_t, uint32_t, uint64_t, etc...
using sampling_distribution_type = double_t;

using bit_vector_type = uint8_t; //std::bitset<8>;

// TODO:: encode repeating symbols
template<typename T>
using products = std::vector<T>;

template<typename T>
using term = std::vector<T>;

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
static inline void set_F(products<uint8_t> &F) {
    // first element: encodes ab'c
    // -------------------------------------------------
    // |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  0  |  0  |  0  |
    // -------------------------------------------------
    F[0] = 0b01100111;

    // second element: encodes a'b
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  0  |  0  |  0  |
    // -------------------------------------------------
    F[1] = 0b10011111;

    // third element: encodes bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  0  |  1  |  0  |  0  |  1  |  0  |  0  |
    // -------------------------------------------------
    F[2] = 0b11011011;

    // fourth element: encodes a'bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  1  |  0  |  0  |
    // -------------------------------------------------
    F[3] = 0b10011011;

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
    // |  1  |  1  |  0  |  0  |  1  |  1  |  0  |  0  |
    // -------------------------------------------------
    F[4] = 0b00010011;
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
static constexpr inline bool eval(auto &F, auto &sampled_x) {
//    for (auto &row : F) {
//        if ((sampled_x | row) == 0b11111111)
//            return true;
//    }
//    return false;

    return std::ranges::any_of(F, [&](uint8_t row) {
        return (sampled_x | row) == 0b11111111;
    });
}

bool eval_and(const auto &F_and, const auto &x) {

}

bool eval_or(const auto &F_or, const auto &and_matrix) {

}

// a | b =
static void fill_x(std::vector<sampling_distribution_type> &dist_x) {
    dist_x[0] = 0.0;  // P(a)
    dist_x[1] = 1.0;  // P(b)
    dist_x[2] = 0.0;  // P(c)
}

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<sampling_distribution_type> uniform_dist(0.0, 1.0);

// TODO:: returned object should be aligned with cache-line
// maybe return 64-bit width value here?
static inline uint8_t sample(const std::vector<sampling_distribution_type> &dist_x) {

    const auto sampled =
            static_cast<uint8_t>(
                    (uniform_dist(gen) > dist_x[0] ? 0b01000000 : 0b10000000) |
                    (uniform_dist(gen) > dist_x[1] ? 0b00010000 : 0b00100000) |
                    (uniform_dist(gen) > dist_x[2] ? 0b00000100 : 0b00001000));

    return sampled;
}

int main() {
    // for expression F = ab'c + a'b + bc' + a'bc' + aa'aacc'a, with
    // m = 5 products
    const size_t m_products = 5;
    std::vector<bit_vector_type> F(m_products);

    // set the function
    set_F(F);

    // define the probabilities for X
    const size_t s_symbols = 3;
    auto dist_x = std::vector<sampling_distribution_type>(s_symbols);
    fill_x(dist_x);

    // sample from dist_x
    const size_t num_samples = 10000;
    auto batches = std::vector<bit_vector_type>(num_samples);


    // Initialize the accumulator
    accumulator_type acc_set(
            acc::tag::quantile::prob = 0.5 // Default quantile (median)
    );

    for (auto i = 0; i < num_samples; i++) {
        const auto sampled_x = sample(dist_x);
        const auto sampled_F = eval(F, sampled_x);
        // Convert sampled_F to double (0.0 or 1.0) for accumulation
        auto sampled_F_numeric = static_cast<std::double_t>(sampled_F);

        // Accumulate the sampled_F value
        acc_set(sampled_F_numeric);
        std::printf("i:%d, %b, %b\n", i, sampled_x, sampled_F);
    }

    try {
        double mean = get_mean(acc_set);
        double variance = get_variance(acc_set);
        double skewness = get_skewness(acc_set);
        double kurtosis = get_kurtosis(acc_set);
        double median = get_median(acc_set);
        double p5th = get_quantile(acc_set, 0.05);
        double p95th = get_quantile(acc_set, 0.95);

        // Display the results with appropriate formatting
        std::printf("Statistics for sampled_F (Boolean to Numeric Conversion):\n");
        std::printf("Mean: %.5f\n", mean);
        std::printf("Variance: %.5f\n", variance);
        std::printf("Skewness: %.5f\n", skewness);
        std::printf("Kurtosis: %.5f\n", kurtosis);
        std::printf("Median: %.5f\n", median);
        std::printf("5th Percentile (p5th): %.5f\n", p5th);
        std::printf("95th Percentile (p95th): %.5f\n", p95th);
    }
    catch (const std::exception& ex) {
        std::fprintf(stderr, "Error computing statistics: %s\n", ex.what());
        return EXIT_FAILURE;
    }


    return 0;
}