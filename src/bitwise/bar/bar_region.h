/**
 * @file bar_region.h
 * @brief bit array region set, get, fill etc. ops
 *
 * Provides constants definitions and functions for bit array set, get, fill operations.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

#ifndef CANOPY_BAR_REGION_H
#define CANOPY_BAR_REGION_H


//
// Fill a region (internal use only)
//

#include "bit_array.h"
#include "bar_constants.h"

#define SET_REGION(arr,start,len)    _set_region((arr),(start),(len),FILL_REGION)
#define CLEAR_REGION(arr,start,len)  _set_region((arr),(start),(len),ZERO_REGION)
#define TOGGLE_REGION(arr,start,len) _set_region((arr),(start),(len),SWAP_REGION)


// FillAction is fill with 0 or 1 or toggle
typedef enum {ZERO_REGION, FILL_REGION, SWAP_REGION} FillAction;

static inline void _set_region(BIT_ARRAY* bitarr, bit_index_t start,
                               bit_index_t length, FillAction action)
{
    if(length == 0) return;

    word_addr_t first_word = bitset64_wrd(start);
    word_addr_t last_word = bitset64_wrd(start+length-1);
    word_offset_t foffset = bitset64_idx(start);
    word_offset_t loffset = bitset64_idx(start+length-1);

    if(first_word == last_word)
    {
        word_t mask = bitmask64(length) << foffset;

        switch(action)
        {
            case ZERO_REGION: bitarr->words[first_word] &= ~mask; break;
            case FILL_REGION: bitarr->words[first_word] |=  mask; break;
            case SWAP_REGION: bitarr->words[first_word] ^=  mask; break;
        }
    }
    else
    {
        // Set first word
        switch(action)
        {
            case ZERO_REGION: bitarr->words[first_word] &=  bitmask64(foffset); break;
            case FILL_REGION: bitarr->words[first_word] |= ~bitmask64(foffset); break;
            case SWAP_REGION: bitarr->words[first_word] ^= ~bitmask64(foffset); break;
        }

        word_addr_t i;

        // Set whole words
        switch(action)
        {
            case ZERO_REGION:
                for(i = first_word + 1; i < last_word; i++)
                    bitarr->words[i] = (word_t)0;
                break;
            case FILL_REGION:
                for(i = first_word + 1; i < last_word; i++)
                    bitarr->words[i] = WORD_MAX;
                break;
            case SWAP_REGION:
                for(i = first_word + 1; i < last_word; i++)
                    bitarr->words[i] ^= WORD_MAX;
                break;
        }

        // Set last word
        switch(action)
        {
            case ZERO_REGION: bitarr->words[last_word] &= ~bitmask64(loffset+1); break;
            case FILL_REGION: bitarr->words[last_word] |=  bitmask64(loffset+1); break;
            case SWAP_REGION: bitarr->words[last_word] ^=  bitmask64(loffset+1); break;
        }
    }
}

#endif //CANOPY_BAR_REGION_H
