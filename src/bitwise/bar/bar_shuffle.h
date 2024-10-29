#ifndef CANOPY_BAR_SHUFFLE_H
#define CANOPY_BAR_SHUFFLE_H

/**
 * @file bar_random.h
 * @brief Dynamic bit array shuffling, permutations, hashing, and randomization.
 *
 * Provides definitions and functions for bit array shuffling, permutations, hashing, and randomization.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <semaphore>

#include "bit_array.h"
#include "bar_constants.h"
#include "bar_string.h"
#include "bar_region.h"

// Have we initialised with srand() ?
static char rand_initiated = 0;

static void _seed_rand()
{
    if(!rand_initiated)
    {
        // Initialise random number generator
        struct timeval time{};
        gettimeofday(&time, nullptr);
        srand((((time.tv_sec ^ getpid()) * 1000001) + time.tv_usec));
        rand_initiated = 1;
    }
}

/* From: lookup3.c, by Bob Jenkins, May 2006, Public Domain. */
#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/* From: lookup3.c, by Bob Jenkins, May 2006, Public Domain. */
#define mix(a,b,c) \
{ \
 a -= c;  a ^= rot(c, 4);  c += b; \
 b -= a;  b ^= rot(a, 6);  a += c; \
 c -= b;  c ^= rot(b, 8);  b += a; \
 a -= c;  a ^= rot(c,16);  c += b; \
 b -= a;  b ^= rot(a,19);  a += c; \
 c -= b;  c ^= rot(b, 4);  b += a; \
}

/* From: lookup3.c, by Bob Jenkins, May 2006, Public Domain. */
#define final(a,b,c) \
{ \
 c ^= b; c -= rot(b,14); \
 a ^= c; a -= rot(c,11); \
 b ^= a; b -= rot(a,25); \
 c ^= b; c -= rot(b,16); \
 a ^= c; a -= rot(c,4);  \
 b ^= a; b -= rot(a,14); \
 c ^= b; c -= rot(b,24); \
}

/*
From: lookup3.c, by Bob Jenkins, May 2006, Public Domain.
--------------------------------------------------------------------
hashword2() -- same as hashword(), but take two seeds and return two
32-bit values.  pc and pb must both be nonnull, and *pc and *pb must
both be initialized with seeds.  If you pass in (*pb)==0, the output
(*pc) will be the same as the return value from hashword().
--------------------------------------------------------------------
*/
static void hashword2 (
        const uint32_t *k,                   /* the key, an array of uint32_t values */
        size_t          length,               /* the length of the key, in uint32_ts */
        uint32_t       *pc,                      /* IN: seed OUT: primary hash value */
        uint32_t       *pb)               /* IN: more seed OUT: secondary hash value */
{
    uint32_t a,b,c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + ((uint32_t)(length<<2)) + *pc;
    c += *pb;

    /*------------------------------------------------- handle most of the key */
    while (length > 3)
    {
        a += k[0];
        b += k[1];
        c += k[2];
        mix(a,b,c);
        length -= 3;
        k += 3;
    }

    /*------------------------------------------- handle the last 3 uint32_t's */
    switch(length)                     /* all the case statements fall through */
    {
        case 3 : c+=k[2];
        case 2 : b+=k[1];
        case 1 : a+=k[0];
            final(a,b,c);
        case 0:     /* case 0: nothing left to add */
            break;
    }
    /*------------------------------------------------------ report the result */
    *pc=c; *pb=b;
}

/**
 * @brief Compute a hash of the bit array using the Bob Jenkins' Lookup3 hash.
 *
 * Generates a 64-bit hash value for the bit array. The `seed` can be used for rehashing in cases of collisions.
 * Pass `0` as the seed for the initial hash computation, and use the previous hash value for subsequent rehashing.
 *
 * @param bitarr Pointer to the bit array to hash.
 * @param seed   Seed value for the hash function. Pass `0` on the first call.
 * @return 64-bit hash value.
 *
 * @note This function uses the Bob Jenkins' Lookup3 hash algorithm.
 *
 * @example
 * @code
 * uint64_t hash = bit_array_hash(bitarr, 0);
 * @endcode
 */
uint64_t bit_array_hash(const BIT_ARRAY* bitarr, uint64_t seed) {
    uint32_t seed32[2];
    memcpy(seed32, &seed, sizeof(uint32_t)*2);

    // Round up length to number 32bit words
    hashword2((uint32_t*)bitarr->words, (bitarr->num_of_bits + 31) / 32,
              &seed32[0], &seed32[1]);

    // XOR with array length. This ensures arrays with different length but same
    // contents have different hash values
    seed ^= bitarr->num_of_bits;

    return seed;
}

/**
 * @brief Set bits in the array randomly based on a probability.
 *
 * Sets each bit in the bit array to `1` with the probability `prob` (0 <= `prob` <= 1), and to `0` otherwise.
 *
 * @param bitarr Pointer to the bit array to modify.
 * @param prob   Probability of setting each bit to `1` (between `0.0` and `1.0`).
 *
 * @example
 * @code
 * bit_array_random(bitarr, 0.5f); // Sets each bit with a 50% probability
 * @endcode
 */
void bit_array_random(BIT_ARRAY* bitarr, float prob) {
    assert(prob >= 0 && prob <= 1);

    if(bitarr->num_of_bits == 0)
    {
        return;
    }
    else if(prob == 1)
    {
        bit_array_set_all(bitarr);
        return;
    }

    // rand() generates number between 0 and RAND_MAX inclusive
    // therefore we want to check if rand() <= p
    long p = static_cast<long>(RAND_MAX) * prob;

    _seed_rand();

    word_addr_t w;
    word_offset_t o;

    // Initialise to zero
    memset(bitarr->words, 0, bitarr->num_of_words * sizeof(word_t));

    for(w = 0; w < bitarr->num_of_words - 1; w++)
    {
        for(o = 0; o < WORD_SIZE; o++)
        {
            if(rand() <= p)
            {
                bitarr->words[w] |= ((word_t)0x1 << o);
            }
        }
    }

    // Top word
    word_offset_t bits_in_last_word = bits_in_top_word(bitarr->num_of_bits);
    w = bitarr->num_of_words - 1;

    for(o = 0; o < bits_in_last_word; o++)
    {
        if(rand() <= p)
        {
            bitarr->words[w] |= ((word_t)0x1 << o);
        }
    }

    DEBUG_VALIDATE(bitarr);
}

/**
 * @brief Shuffle the bits in the bit array randomly.
 *
 * Randomly permutes the order of bits in the bit array.
 *
 * @param bitarr Pointer to the bit array to shuffle.
 *
 * @example
 * @code
 * bit_array_shuffle(bitarr);
 * @endcode
 */
void bit_array_shuffle(BIT_ARRAY* bitarr) {
    if(bitarr->num_of_bits == 0)
        return;

    _seed_rand();

    bit_index_t i, j;

    for(i = bitarr->num_of_bits - 1; i > 0; i--)
    {
        j = (bit_index_t)rand() % i;

        // Swap i and j
        char x = (bitarr->words[bitset64_wrd(i)] >> bitset64_idx(i)) & 0x1;
        char y = (bitarr->words[bitset64_wrd(j)] >> bitset64_idx(j)) & 0x1;

        if(!y)
            bitarr->words[bitset64_wrd(i)] &= ~((word_t)0x1 << bitset64_idx(i));
        else
            bitarr->words[bitset64_wrd(i)] |= (word_t)0x1 << bitset64_idx(i);

        if(!x)
            bitarr->words[bitset64_wrd(j)] &= ~((word_t)0x1 << bitset64_idx(j));
        else
            bitarr->words[bitset64_wrd(j)] |= (word_t)0x1 << bitset64_idx(j);
    }

    DEBUG_VALIDATE(bitarr);
}

static word_t _next_permutation(word_t v)
{
    assert(v);
    // From http://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
    word_t t = v | (v - 1); // t gets v's least significant 0 bits set to 1
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    return (t+1) | (((~t & (t+1)) - 1) >> (trailing_zeros(v) + 1));
}

/**
 * @brief Generate the next permutation of bits in lexicographic order.
 *
 * Generates the next lexicographic permutation of the bit array with a fixed size and a given number of bits set.
 * This function cycles through all possible combinations of the bit array where a fixed number of bits are set.
 *
 * Example sequence for a bit array with 5 bits set out of 5:
 * ```
 * 00011 -> 00101 -> 00110 -> 01001 -> 01010 ->
 * 01100 -> 10001 -> 10010 -> 10100 -> 11000 -> 00011 (back to start)
 * ```
 *
 * @param bitarr Pointer to the bit array to permute.
 *
 * @example
 * @code
 * bit_array_next_permutation(bitarr);
 * @endcode
 */
void bit_array_next_permutation(BIT_ARRAY* bitarr)
{
    if(bitarr->num_of_bits == 0)
    {
        return;
    }

    word_addr_t w;

    char carry = 0;
    word_offset_t top_bits = bitset64_idx(bitarr->num_of_bits);

    for(w = 0; w < bitarr->num_of_words; w++)
    {
        word_t mask
                = (w < bitarr->num_of_words - 1 || top_bits == 0) ? WORD_MAX
                                                                  : bitmask64(top_bits);

        if(bitarr->words[w] > 0 &&
           (bitarr->words[w] | (bitarr->words[w]-1)) == mask)
        {
            // Bits in this word cannot be moved forward
            carry = 1;
        }
        else if(carry)
        {
            // 0111 -> 1000, 1000 -> 1001
            word_t tmp = bitarr->words[w] + 1;

            // Count bits previously set
            bit_index_t bits_previously_set = POPCOUNT(bitarr->words[w]);

            // set new word
            bitarr->words[w] = tmp;

            // note: w is unsigned
            // Zero words while counting bits set
            while(w > 0)
            {
                bits_previously_set += POPCOUNT(bitarr->words[w-1]);
                bitarr->words[w-1] = 0;
                w--;
            }

            // Set bits at the beginning
            SET_REGION(bitarr, 0, bits_previously_set - POPCOUNT(tmp));

            carry = 0;
            break;
        }
        else if(bitarr->words[w] > 0)
        {
            bitarr->words[w] = _next_permutation(bitarr->words[w]);
            break;
        }
    }

    if(carry)
    {
        // Loop around
        bit_index_t num_bits_set = bit_array_num_bits_set(bitarr);
        bit_array_clear_all(bitarr);
        SET_REGION(bitarr, 0, num_bits_set);
    }

    DEBUG_VALIDATE(bitarr);
}
#endif //CANOPY_BAR_SHUFFLE_H
