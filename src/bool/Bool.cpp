#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

#include <CL/sycl.hpp>
#include <iomanip>

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
    dist_x[0] = 1e-2;  // P(a)
    dist_x[1] = 2e-3;  // P(b)
    dist_x[2] = 3e-1;  // P(c)
}

// for F = ab'c + a'b + bc'
//
// P(F)   =   P(ab'c) + P(a'b) + P(bc')
//          - P(ab'c) * P(a'b)
//          - P(a'b)  * P(bc')
//          - P(ab'c) * P(bc')
//          + P(ab'c) * P(a'b) * P(bc')
//
// where:
//
// P(ab'c) = P(a)  * P(b') * P(c)
// P(a'b)  = P(a') * P(b)
// P(bc')  = P(b)  * P(c')
//
//
// P(a')   = 1 - P(a)
// P(b')   = 1 - P(b)
// P(c')   = 1 - P(c)

template<typename float_type>
static float_type compute_exact_prob_F(std::vector<sampling_distribution_type> &dist_x) {
    const auto Pa = static_cast<float_type>(dist_x[0]);
    const auto Pb = static_cast<float_type>(dist_x[1]);
    const auto Pc = static_cast<float_type>(dist_x[2]);

    const float_type Pab_c = Pa * (1.0 - Pb) * Pc;
    const float_type Pa_b = (1.0 - Pa) * Pb;
    const float_type Pbc_ = Pb * (1.0 - Pc);

    const float_type Pf = Pab_c + Pa_b + Pbc_
            - (Pab_c * Pa_b)
            - (Pa_b  * Pbc_)
            - (Pab_c * Pbc_)
            + (Pab_c * Pa_b * Pbc_);

    assert(Pf <= 1.0 && Pf >= 0.0);
    return Pf;
}

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<sampling_distribution_type> uniform_dist(0.0, 1.0);

// TODO:: returned object should be aligned with cache-line
// maybe return 64-bit width value here?
static inline uint8_t generate_sample(const std::vector<sampling_distribution_type> &dist_x) {

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
    const size_t num_samples = 1e7;
    auto batches = std::vector<bit_vector_type>(num_samples);

    // Generate and collect the tallies
    using tally_float_type = long double;
    std::size_t count = 0;
    for (auto i = 0; i < num_samples; i++) {
        const auto sample = generate_sample(dist_x);
        const bool tally = eval(F, sample);
        if (tally) {
            count++;
        }
    }

    const auto mean = static_cast<tally_float_type>(count) / num_samples;
    const auto variance_of_mean = (mean - mean * mean) / num_samples;

    const auto known_P = compute_exact_prob_F<tally_float_type>(dist_x);
    const auto error = mean - known_P;
    const auto rel_error = 100.0 * error / known_P;
    std::cout << std::setprecision(8) << std::scientific;
    std::cout<<"F = ab'c + a'b + bc'"<<std::endl;
    std::cout<<"P(a): "<<dist_x[0]<<", P(b): "<<dist_x[1]<<", P(c): "<<dist_x[2]<<std::endl;
    std::cout<<"known P(f): "<<known_P<<std::endl;
    std::cout<<"\nStatistics for sampled_F:"<<std::endl;
    std::cout<<"tally:: "<<"num_true/num_samples: "<<count<<"/"<<num_samples<<",\tmean: "<<mean<<",\tvar: "<<variance_of_mean<<std::endl;
    std::cout<<"error [sampled - known]: "<<error<<",\trelative error (%): "<<rel_error<<std::endl;
    return 0;
}