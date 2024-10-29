#ifndef CANOPY_BAR_GETSET_H
#define CANOPY_BAR_GETSET_H
/**
 * @file bar_getset.h
 * @brief bit array internal functions for getting and setting different sized operands.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

//
// Get and set words (internal use only -- no bounds checking)
//

#include "bit_array.h"
#include "bar_constants.h"
#include "bar_string.h"

static inline word_t _get_word(const BIT_ARRAY* bitarr, bit_index_t start)
{
    word_addr_t word_index = bitset64_wrd(start);
    word_offset_t word_offset = bitset64_idx(start);

    word_t result = bitarr->words[word_index] >> word_offset;

    word_offset_t bits_taken = WORD_SIZE - word_offset;

    // word_offset is now the number of bits we need from the next word
    // Check the next word has at least some bits
    if(word_offset > 0 && start + bits_taken < bitarr->num_of_bits)
    {
        result |= bitarr->words[word_index+1] << (WORD_SIZE - word_offset);
    }

    return result;
}

// Set 64 bits from a particular start position
// Doesn't extend bit array
static inline void _set_word(BIT_ARRAY* bitarr, bit_index_t start, word_t word)
{
    word_addr_t word_index = bitset64_wrd(start);
    word_offset_t word_offset = bitset64_idx(start);

    if(word_offset == 0)
    {
        bitarr->words[word_index] = word;
    }
    else
    {
        bitarr->words[word_index]
                = (word << word_offset) |
                  (bitarr->words[word_index] & bitmask64(word_offset));

        if(word_index+1 < bitarr->num_of_words)
        {
            bitarr->words[word_index+1]
                    = (word >> (WORD_SIZE - word_offset)) |
                      (bitarr->words[word_index+1] & (WORD_MAX << word_offset));
        }
    }

    // Mask top word
    _mask_top_word(bitarr);
    DEBUG_VALIDATE(bitarr);
}

static inline void _set_byte(BIT_ARRAY *bitarr, bit_index_t start, uint8_t byte)
{
    word_t w = _get_word(bitarr, start);
    _set_word(bitarr, start, (w & ~(word_t)0xff) | byte);
}

// 4 bits
static inline void _set_nibble(BIT_ARRAY *bitarr, bit_index_t start,
                               uint8_t nibble)
{
    word_t w = _get_word(bitarr, start);
    _set_word(bitarr, start, (w & ~(word_t)0xf) | nibble);
}

// Wrap around
static inline word_t _get_word_cyclic(const BIT_ARRAY* bitarr, bit_index_t start)
{
    word_t word = _get_word(bitarr, start);

    bit_index_t bits_taken = bitarr->num_of_bits - start;

    if(bits_taken < WORD_SIZE)
    {
        word |= (bitarr->words[0] << bits_taken);

        if(bitarr->num_of_bits < (bit_index_t)WORD_SIZE)
        {
            // Mask word to prevent repetition of the same bits
            word = word & bitmask64(bitarr->num_of_bits);
        }
    }

    return word;
}

// Wrap around
static inline void _set_word_cyclic(BIT_ARRAY* bitarr,
                                    bit_index_t start, word_t word)
{
    _set_word(bitarr, start, word);

    bit_index_t bits_set = bitarr->num_of_bits - start;

    if(bits_set < WORD_SIZE && start > 0)
    {
        word >>= bits_set;

        // Prevent overwriting the bits we've just set
        // by setting 'start' as the upper bound for the number of bits to write
        word_offset_t bits_remaining = MIN(WORD_SIZE - bits_set, start);
        word_t mask = bitmask64(bits_remaining);

        bitarr->words[0] = bitmask_merge(word, bitarr->words[0], mask);
    }
}

#endif //CANOPY_BAR_GETSET_H
