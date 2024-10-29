#ifndef CANOPY_BAR_STRING_H
#define CANOPY_BAR_STRING_H

/**
 * @file bar_string.h
 * @brief Dynamic bit array string related operations
 *
 * Provides definitions and functions for printing bit arrays (bars) as strings
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

#include <cstdio>
#include <cinttypes>
#include "bit_array.h"
#include "bar_constants.h"

// Used in debugging
#ifndef NDEBUG
#define DEBUG_PRINT(msg,...) printf("[%s:%i] " msg, __FILE__, __LINE__, ##__VA_ARGS__);
#define DEBUG_VALIDATE(a) validate_bitarr((a), __FILE__, __LINE__)
#else
#define DEBUG_PRINT(msg,...)
#define DEBUG_VALIDATE(a)
#endif

// Mostly used for debugging
static inline void _print_word(word_t word, FILE* out)
{
    word_offset_t i;
    for(i = 0; i < WORD_SIZE; i++)
    {
        fprintf(out, "%c", ((word >> i) & (word_t)0x1) == 0 ? '0' : '1');
    }
}

// prints right to left
static inline char* _word_to_str(word_t word, char str[WORD_SIZE+1])
__attribute__((unused));

static inline char* _word_to_str(word_t word, char str[WORD_SIZE+1])
{
    word_offset_t i;
    for(i = 0; i < WORD_SIZE; i++)
    {
        str[WORD_SIZE-i-1] = ((word >> i) & (word_t)0x1) == 0 ? '0' : '1';
    }
    str[WORD_SIZE] = '\0';
    return str;
}

/**
 * @brief Convert a word's bits to a binary string in little-endian order.
 *
 * Converts the bits of a word (from least significant bit to most significant bit)
 * into a binary string representation. e.g. 0b11010 (26 in decimal) would come out as "01011"
 *
 * @param ptr        Pointer to the word containing the bits.
 * @param num_of_bits Number of bits to convert (e.g., 32 for a `uint32_t`).
 * @param str        Buffer to store the resulting binary string.
 * @return Pointer to the resulting string (`str`).
 *
 * @note The function does not null-terminate the string.
 *
 * @example
 * @code
 * char binary_str[33];
 * bit_array_word2str(&word, 32, binary_str);
 * binary_str[32] = '\0'; // Null-terminate if necessary
 * printf("Binary: %s\n", binary_str);
 * @endcode
 */
char* bit_array_word2str(const void *ptr, size_t num_of_bits, char *str) {
    const auto* d = (const uint8_t*)ptr;

    size_t i;
    for(i = 0; i < num_of_bits; i++)
    {
        uint8_t bit = (d[i/8] >> (i % 8)) & 0x1;
        str[i] = bit ? '1' : '0';
    }
    str[num_of_bits] = '\0';
    return str;
}

/**
 * @brief Convert a word's bits to a binary string in big-endian order.
 *
 * Converts the bits of a word (from most significant bit to least significant bit)
 * into a binary string representation.
 *
 * @param ptr         Pointer to the word containing the bits.
 * @param num_of_bits Number of bits to convert (e.g., 32 for a `uint32_t`).
 * @param str         Buffer to store the resulting binary string.
 * @return Pointer to the resulting string (`str`).
 *
 * @note The function does not null-terminate the string.
 *
 * @example
 * @code
 * char binary_str[33];
 * bit_array_word2str_rev(&word, 32, binary_str);
 * binary_str[32] = '\0'; // Null-terminate if necessary
 * printf("Reversed Binary: %s\n", binary_str);
 * @endcode
 */
char* bit_array_word2str_rev(const void *ptr, size_t num_of_bits, char *str) {
    const auto* d = (const uint8_t*)ptr;

    size_t i;
    for(i = 0; i < num_of_bits; i++)
    {
        uint8_t bit = (d[i/8] >> (i % 8)) & 0x1;
        str[num_of_bits-1-i] = bit ? '1' : '0';
    }
    str[num_of_bits] = '\0';
    return str;
}


#endif //CANOPY_BAR_STRING_H
