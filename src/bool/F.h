#pragma once

#include <climits>
#include "utils/types.h"

/// @brief Macro to calculate the number of windows required to store a given number of bits.
/// @param bit_count The total number of bits.
/// @param window_type The type of the window, which determines the size of each window.
/// @return The number of windows required.
/// @note Ensure that bit_count is a multiple of the size of window_type in bits.
#define WINDOW_COUNT(bit_count, window_type) ((bit_count) / (sizeof(window_type) * CHAR_BIT))

/**
 * @file F.h
 * @brief F function logic definition
 *
 * Defines the templated F function structs/classes
 *
 * @author Arjun Earthperson
 * @date 11/17/2024
 */

namespace canopy {
template <typename bit_vector_type = uint8_t>
class F {
    std::vector<bit_vector_type> _data;
    size_t _product_width;
public:
    explicit F(const products &products, const size_t num_inputs) : _data(products), _product_width(num_inputs) {

    }

    explicit F(const size_t num_products, const size_t num_inputs) {
        _product_width = WINDOW_COUNT(num_inputs, bit_vector_type);
        _data = std::vector<bit_vector_type>(num_products);
    }
};
}

template <typename size_type = std::size_t>
struct array {
    size_type x;
    //T *ary;
};