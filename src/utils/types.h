#ifndef CANOPY_TYPES_H
#define CANOPY_TYPES_H

/**
* @file types.h
* @author Arjun Earthperson
* @date 10/21/2024
* @brief This file contains type definitions, typically serving as wrappers around commonly known types. The idea is to
* provide additional semantic meaning to the data-types in common use.
*/

#include <cmath>
#include <cstdint>

/**
 * @typedef sampling_distribution_type
 * @brief The data type used for sampling distribution in random number generation.
 * @details Typically a 32-bit floating-point type.
 */
using sampling_distribution_type = std::float_t;

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

#endif //CANOPY_TYPES_H
