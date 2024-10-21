#ifndef CANOPY_BITS_H
#define CANOPY_BITS_H

#include <limits>
#include <array>
#include <cstdint>
#include <climits>

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
     * @tparam num_bits The total number of bits to manage.
     *
     * @example
     * ```
     * canopy::bits<uint_fast16_t, 128> myBits;
     * ```
     */
    template<std::size_t num_bits, typename window>
    class bits {

    public:
        //std::array<window, WINDOW_COUNT(num_bits, window)> windows_;
        std::array<window, num_bits> windows_;

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
    concept EightBitWide = (sizeof(T) == 1);

    /**
     * @brief A specialization of the bits class using 8-bit wide windows.
     *
     * This class is a specialization of the bits class template, allowing any type that is
     * exactly 8 bits wide as the window type.
     *
     * @tparam window The type of each window, constrained to be 8 bits wide.
     * @tparam num_bits The total number of bits to manage.
     *
     * @example
     * ```
     * canopy::bits8<uint8_t, 64> myBit8;
     * ```
     */
    template<std::size_t num_bits, EightBitWide window = std::uint_fast8_t>
    class bits8 : public bits<num_bits, window> {


        /**
         * @brief Overload the bitwise OR operator for bits8 instances.
         *
         * This operator allows the use of the `|` operator to perform a bitwise OR operation
         * between two bits8 instances. The result is stored in a new bits8 instance.
         *
         * @param other The other bits8 instance to OR with.
         * @return A new bits8 instance containing the result of the OR operation.
         *
         * @example
         * ```
         * canopy::bits8<uint8_t, 64> a, b, c;
         * c = a | b;
         * ```
         */
        bits8 operator|(const bits8& other) const {
            bits8 result;
            for (std::size_t i = 0; i < this->windows_.size(); ++i) {
                result.windows_[i] = this->windows_[i] | other.windows_[i];
            }
            return result;
        }
    };

}


#endif //CANOPY_BITS_H
