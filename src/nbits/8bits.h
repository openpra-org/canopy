#pragma once


/**
 * @brief A specialization of the bits class using 8-bit wide windows.
 *
 * This class is a specialization of the bits class template, allowing any type that is
 * exactly 8 bits wide as the window type.
 *
 * @tparam window The type of each window, constrained to be 8 bits wide.
 *
 * @example
 * ```
 * canopy::bits8 myBit8;
 * ```
 */
template<EightBitWide window = std::uint_fast8_t>
class bits8 : public bits<window> {


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
     * canopy::bits8<uint8_t> a, b, c;
     * c = a | b;
     * ```
     */
    bits8 operator|(const bits8& other) const {
        assert(this->windows_.size() == other.windows_.size());
        bits8 result;
        const auto size = this->windows_.size();
        for (std::size_t i = 0; i < size; ++i) {
            result.windows_[i] = this->windows_[i] | other.windows_[i];
        }
        return result;
    }


};