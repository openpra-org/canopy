#pragma once

/**
 * @file nbits.h
 * @author Arjun Earthperson
 * @date 10/21/2024
 * @brief This file contains logic for wrapping bit-manipulations around multi-word-sized primitives. The objective is
 * to enable a few handful, highly restricted, and highly optimized access patterns, essentially getting the benefits
 * of multi-word-wide ops without the added overhead of maintaining bit-level address-ability. For performance, we use
 * runtime bounds safety and checks from cassert, giving us the flexibility to remove such checks for release builds.
 * Word-level optimizations can be made for different bit-widths using template specialization for differing data types.
 */

#include <vector>
#include <climits>
#include <cassert>

/// @brief Macro to calculate the number of windows required to store a given number of bits.
/// @param bit_count The total number of bits.
/// @param window_type The type of the window, which determines the size of each window.
/// @return The number of windows required.
/// @note Ensure that bit_count is a multiple of the size of window_type in bits.
#define WINDOW_COUNT(bit_count, window_type) ((bit_count) / (sizeof(window_type) * CHAR_BIT))

namespace canopy {
    /**
     * @brief A template class to manage a fixed number of bits using a specified window type.
     *
     * This class provides a way to handle a fixed number of bits using an array of windows.
     * Each window is of the specified type, and the number of windows is calculated based on
     * the total number of bits and the size of the window type.
     *
     * @tparam window The type of each window, typically an integer type.
     *
     * @example
     * ```
     * canopy::nbits<uint_fast16_t> myBits;
     * ```
     */
    template<typename window>
    class nbits {

    public:
        std::vector<window> windows_;

        // todo:: copy & move constructors
        // todo:: copy constructors for same and different window sizes

        // todo:: (for same or different window size), num_bit-wide bitwise {OR, AND} ops, which operate on a window size at a time

        /**
         * @brief Overload the bitwise OR operator for bits instances.
         *
         * This operator allows the use of the `|` operator to perform a bitwise OR operation
         * between two bits instances. The result is stored in a new bits instance.
         *
         * @param other The other bits instance to OR with.
         * @return A new bits instance containing the result of the OR operation.
         *
         * @note This function is pure virtual and must be implemented by derived classes.
         */
        //window operator|(const bits& other) const = 0;
    };

    /**
     * @brief Concept to ensure the window type is 8 bits wide.
     *
     * This concept checks that the size of the window type is exactly 1 byte (8 bits).
     *
     * @tparam T The type to check.
     */
    template<typename T>
    concept EightBitsWide = sizeof(T) == 1;

    /**
     * @brief Concept to ensure the window type is 16 bits wide.
     *
     * This concept checks that the size of the window type is exactly 2 bytes (16 bits).
     *
     * @tparam T The type to check.
     */
    template<typename T>
    concept SixteenBitsWide = sizeof(T) == 2;

    /**
     * @brief Concept to ensure the window type is 32 bits wide.
     *
     * This concept checks that the size of the window type is exactly 4 bytes (32 bits).
     *
     * @tparam T The type to check.
     */
    template<typename T>
    concept ThirtyTwoBitsWide = sizeof(T) == 4;

    /**
     * @brief Concept to ensure the window type is 64 bits wide.
     *
     * This concept checks that the size of the window type is exactly 8 bytes (64 bits).
     *
     * @tparam T The type to check.
     */
    template<typename T>
    concept SixtyFourBitsWide = sizeof(T) == 8;
}

