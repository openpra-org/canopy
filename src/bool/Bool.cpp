/*
    Copyright (C) 2024 OpenPRA Initiative

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <vector>
#include <numeric>

#include <CL/sycl.hpp>

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
    const size_t num_samples = 10000;
    auto batches = std::vector<bit_vector_type>(num_samples);

    // Generate and collect the tallies
    std::vector<double_t> tallies;
    std::vector<double_t> tallies_squared;
    for (auto i = 0; i < num_samples; i++) {
        const auto sample = generate_sample(dist_x);

        // Generate the tallies
        // TODO:: Complete this as we discussed
        const auto evaluated_tally = eval(F, sample);

        // Collect the tallies
        tallies.push_back(evaluated_tally);
        tallies_squared.push_back(evaluated_tally * evaluated_tally);
        std::printf("i:%i, %d, %d\n", i, sample, evaluated_tally);
    }

    double_t tallies_sum = std::accumulate(tallies.begin(), tallies.end(), 0.0);
    double_t tallies_squared_sum = std::accumulate(tallies_squared.begin(), tallies_squared.end(), 0.0);

    double_t mean = tallies_sum / num_samples;
    double_t variance_of_mean = (tallies_squared_sum / num_samples - mean * mean) / num_samples;

    std::printf("Statistics for sampled_F (Boolean to Numeric Conversion):\n");
    std::printf("Mean: %.5f\n", mean);
    std::printf("Variance of Mean: %.5f\n", variance_of_mean);

    return 0;
}