#include <iostream>
#include <vector>

#include <CL/sycl.hpp>

// TODO:: define a templated type, with concrete overrides for uint8_t, uint16_t, uint32_t, uint64_t, etc...
using sampling_distribution_type = double_t;
using bit_vector_type = uint8_t; //std::bitset<8>;

// TODO:: encode repeating symbols
// for expression F = ab'c + a'b + bc' + a'bc' + aa'baacc'ab, with:
//
// s = 3 unique symbols (excluding negations)
// n = 2*s = 6 total symbols (including negations)
// m = 4 products
//
// using w=8-bit words to encode n=6 symbols, starting from the MSB, and,
// m = 4 words, we fill out a m=4 element vector,
// where each element encodes one product
// -------------------------------------------------
// |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
// -------------------------------------------------
// |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
// -------------------------------------------------
static inline void set_F(std::vector<bit_vector_type> &F) {
    // first element: encodes ab'c
    // -------------------------------------------------
    // |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  1  |  0  |  0  |  1  |  1  |  0  |  0  |  0  |
    // -------------------------------------------------
    F[0] = 0b10011000;

    // second element: encodes a'b
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  0  |  0  |  0  |
    // -------------------------------------------------
    F[1] = 0b01100000;

    // third element: encodes bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  0  |  1  |  0  |  0  |  1  |  0  |  0  |
    // -------------------------------------------------
    F[2] = 0b00100100;

    // fourth element: encodes a'bc'
    // -------------------------------------------------
    // |  a  |  a' |  b  |  b' |  c  |  c' |  -  |  -  |
    // -------------------------------------------------
    // |  0  |  1  |  1  |  0  |  0  |  1  |  0  |  0  |
    // -------------------------------------------------
    F[3] = 0b01100100;
}

bool eval(auto &F, auto &x) {

}

// a | b =
static void fill_x(std::vector<sampling_distribution_type> &dist_x) {
    dist_x[0] = 0.28023;  // P(a)
    dist_x[1] = 0.00291;  // P(b)
    dist_x[2] = 0.12000;  // P(c)
}

static void sample(std::vector<bit_vector_type> &samples_x, const std::vector<sampling_distribution_type> &dist_x) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<sampling_distribution_type> uniform_dist(0.0, 1.0);

    for(unsigned char & i : samples_x) {
        i = (uniform_dist(gen) > dist_x[0] ? 0b01000000 : 0b10000000) |
                (uniform_dist(gen) > dist_x[1] ? 0b00010000 : 0b00100000) |
                       (uniform_dist(gen) > dist_x[2] ? 0b00000100 : 0b00001000);
    }
}

int main() {
    // for expression F = ab'c + a'b + bc' + a'bc', with
    // m = 4 products
    const size_t m_products = 4;
    std::vector<bit_vector_type> F(m_products);

    // set the function
    set_F(F);

    // define the probabilities for X
    const size_t s_symbols = 3;
    auto dist_x = std::vector<sampling_distribution_type>(s_symbols);
    fill_x(dist_x);

    // sample from dist_x
    const size_t num_samples = 100;
    const size_t num_batches = 1;
    auto batches = std::vector<bit_vector_type>(num_samples);
    sample(batches, dist_x);

    return 0;
}