/**
 * @file bar_compare.h
 * @brief bit array cmp ops
 *
 * Provides constants definitions and functions compare operations on bit arrays, words, sets of words, and
 * everything in-between.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */
#ifndef CANOPY_BAR_COMPARE_H
#define CANOPY_BAR_COMPARE_H

#include "bit_array.h"
#include "bar_constants.h"

/**
 * @brief Compare two bit arrays in little-endian order (LSB at index 0).
 *
 * Compares `bitarr1` and `bitarr2` based on their binary values, considering index `0` as the Least Significant Bit (LSB).
 * The arrays do not need to be the same length.
 *
 * @param bitarr1 Pointer to the first bit array.
 * @param bitarr2 Pointer to the second bit array.
 * @return
 *  - `1` if `bitarr1` > `bitarr2`
 *  - `0` if `bitarr1` == `bitarr2`
 *  - `-1` if `bitarr1` < `bitarr2`
 *
 * @note Comparison is based on the binary value, not on memory layout.
 *
 * @example
 * @code
 * int cmp = bit_array_cmp(arr1, arr2);
 * if (cmp > 0) {
 *     // arr1 is greater than arr2
 * } else if (cmp < 0) {
 *     // arr1 is less than arr2
 * } else {
 *     // arr1 is equal to arr2
 * }
 * @endcode
 */
int bit_array_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2) {
    word_addr_t i;
    word_t word1, word2;
    word_addr_t min_words = bitarr1->num_of_words;

    // i is unsigned so break when i == 0
    if(bitarr1->num_of_words > bitarr2->num_of_words) {
        min_words = bitarr2->num_of_words;
        for(i = bitarr1->num_of_words-1; ; i--) {
            if(bitarr1->words[i]) return 1;
            if(i == bitarr2->num_of_words) break;
        }
    }
    else if(bitarr1->num_of_words < bitarr2->num_of_words) {
        for(i = bitarr2->num_of_words-1; ; i--) {
            if(bitarr2->words[i]) return 1;
            if(i == bitarr1->num_of_words) break;
        }
    }

    if(min_words == 0) return 0;

    for(i = min_words-1; ; i--)
    {
        word1 = bitarr1->words[i];
        word2 = bitarr2->words[i];
        if(word1 != word2) return (word1 > word2 ? 1 : -1);
        if(i == 0) break;
    }

    if(bitarr1->num_of_bits == bitarr2->num_of_bits) return 0;
    return bitarr1->num_of_bits > bitarr2->num_of_bits ? 1 : -1;
}

/**
 * @brief Compare two bit arrays in big-endian order (MSB at index 0).
 *
 * Compares `bitarr1` and `bitarr2` based on their binary values, considering index `0` as the Most Significant Bit (MSB).
 * The arrays do not need to be the same length.
 *
 * @param bitarr1 Pointer to the first bit array.
 * @param bitarr2 Pointer to the second bit array.
 * @return
 *  - `1` if `bitarr1` > `bitarr2`
 *  - `0` if `bitarr1` == `bitarr2`
 *  - `-1` if `bitarr1` < `bitarr2`
 *
 * @note Comparison is based on the binary value, not on memory layout.
 *
 * @example
 * @code
 * int cmp = bit_array_cmp_big_endian(arr1, arr2);
 * if (cmp > 0) {
 *     // arr1 is greater than arr2
 * } else if (cmp < 0) {
 *     // arr1 is less than arr2
 * } else {
 *     // arr1 is equal to arr2
 * }
 * @endcode
 */
int bit_array_cmp_big_endian(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2) {
    word_addr_t min_words = MAX(bitarr1->num_of_words, bitarr2->num_of_words);

    word_addr_t i;
    word_t word1, word2;

    for(i = 0; i < min_words; i++) {
        word1 = _reverse_word(bitarr1->words[i]);
        word2 = _reverse_word(bitarr2->words[i]);
        if(word1 != word2) return (word1 > word2 ? 1 : -1);
    }

    // Check remaining words. Only one of these loops will execute
    for(; i < bitarr1->num_of_words; i++)
        if(bitarr1->words[i]) return 1;
    for(; i < bitarr2->num_of_words; i++)
        if(bitarr2->words[i]) return -1;

    if(bitarr1->num_of_bits == bitarr2->num_of_bits) return 0;
    return bitarr1->num_of_bits > bitarr2->num_of_bits ? 1 : -1;
}


/**
 * @brief Compare a bit array with another bit array shifted by a specified position.
 *
 * Compares `bitarr1` with `bitarr2` shifted left by `pos` bits.
 *
 * @param bitarr   Pointer to the first bit array.
 * @param pos      Number of bits to shift `bitarr2` to the left.
 * @param bitarr2  Pointer to the second bit array.
 * @return
 *  - `1` if `bitarr1` > (`bitarr2` << `pos`)
 *  - `0` if `bitarr1` == (`bitarr2` << `pos`)
 *  - `-1` if `bitarr1` < (`bitarr2` << `pos`)
 *
 * @example
 * @code
 * int cmp = bit_array_cmp_words(arr1, 5, arr2);
 * @endcode
 */
int bit_array_cmp_words(const BIT_ARRAY *bitarr, bit_index_t pos, const BIT_ARRAY *bitarr2) {
    if(bitarr->num_of_bits == 0 && bitarr2->num_of_bits == 0)
    {
        return 0;
    }

    bit_index_t top_bit1 = 0, top_bit2 = 0;

    char arr1_zero = !bit_array_find_last_set_bit(bitarr, &top_bit1);
    char arr2_zero = !bit_array_find_last_set_bit(bitarr2, &top_bit2);

    if(arr1_zero && arr2_zero) return 0;
    if(arr1_zero) return -1;
    if(arr2_zero) return 1;

    bit_index_t top_bit2_offset = top_bit2 + pos;

    if(top_bit1 != top_bit2_offset) {
        return top_bit1 > top_bit2_offset ? 1 : -1;
    }

    word_addr_t i;
    word_t word1, word2;

    for(i = top_bit2 / WORD_SIZE; i > 0; i--)
    {
        word1 = _get_word(bitarr, pos + i * WORD_SIZE);
        word2 = bitarr2->words[i];

        if(word1 > word2) return 1;
        if(word1 < word2) return -1;
    }

    word1 = _get_word(bitarr, pos);
    word2 = bitarr2->words[0];

    if(word1 > word2) return 1;
    if(word1 < word2) return -1;

    // return 1 if arr1[0..pos] != 0, 0 otherwise

    // Whole words
    word_addr_t num_words = pos / WORD_SIZE;

    for(i = 0; i < num_words; i++)
    {
        if(bitarr->words[i] > 0)
        {
            return 1;
        }
    }

    word_offset_t bits_remaining = pos - num_words * WORD_SIZE;

    if(bitarr->words[num_words] & bitmask64(bits_remaining))
    {
        return 1;
    }

    return 0;
}

#endif //CANOPY_BAR_COMPARE_H
