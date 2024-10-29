/**
* 64 bit words
* Array length can be zero
* Unused top bits must be zero
*/

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <cstring> // memset()
#include <cassert>
#include <unistd.h>  // need for getpid() for seeding rand number
#include <cctype>  // need for tolower()
#include <sys/time.h> // for seeding random

// Windows includes
#if defined(_WIN32)
#include <intrin.h>
#endif

#include "bar/bit_array.h"
#include "bit_macros.h"

void validate_bitarr(BIT_ARRAY *arr, const char *file, int lineno)
{
   // Check top word is masked
   word_addr_t tw = arr->num_of_words == 0 ? 0 : arr->num_of_words - 1;
   bit_index_t top_bits = bits_in_top_word(arr->num_of_bits);
   int err = 0;

   if(arr->words[tw] > bitmask64(top_bits))
   {
       _print_word(arr->words[tw], stderr);
       fprintf(stderr, "\n[%s:%i] Expected %i bits in top word[%i]\n",
               file, lineno, (int)top_bits, (int)tw);
       err = 1;
   }

   // Check num of words is correct
   word_addr_t num_words = roundup_bits2words64(arr->num_of_bits);
   if(num_words != arr->num_of_words)
   {
       fprintf(stderr, "\n[%s:%i] num of words wrong "
                       "[bits: %i, word: %i, actual words: %i]\n", file, lineno,
               (int)arr->num_of_bits, (int)num_words, (int)arr->num_of_words);
       err = 1;
   }

   if(err) abort();
}

//
// Constructor
//

// If cannot allocate memory, set errno to ENOMEM, return nullptr
BIT_ARRAY* bit_array_alloc(BIT_ARRAY* bitarr, bit_index_t nbits)
{
   bitarr->num_of_bits = nbits;
   bitarr->num_of_words = roundup_bits2words64(nbits);
   bitarr->capacity_in_words = MAX(8, roundup2pow(bitarr->num_of_words));
   bitarr->words = (word_t*)calloc(bitarr->capacity_in_words, sizeof(word_t));
   if(bitarr->words == nullptr) {
       errno = ENOMEM;
       return nullptr;
   }
   return bitarr;
}

void bit_array_dealloc(BIT_ARRAY* bitarr)
{
   free(bitarr->words);
   memset(bitarr, 0, sizeof(BIT_ARRAY));
}


/**
* Constructor - create a new bit array of length nbits. If cannot allocate memory, set errno to ENOMEM, return nullptr
* @param nbits bit array of length nbits
* @return newly created bit array, nullptr if allocation fails
*/
BIT_ARRAY* bit_array_create(bit_index_t nbits)
{
   BIT_ARRAY* bitarr = (BIT_ARRAY*)malloc(sizeof(BIT_ARRAY));

   // error if we could not allocate enough memory
   if(bitarr == nullptr || bit_array_alloc(bitarr, nbits) == nullptr)
   {
       if(bitarr != nullptr) free(bitarr);
       errno = ENOMEM;
       return nullptr;
   }

   DEBUG_PRINT("Creating BIT_ARRAY (bits: %lu; allocated words: %lu; "
               "using words: %lu; WORD_SIZE: %i)\n",
               (unsigned long)nbits, (unsigned long)bitarr->capacity_in_words,
               (unsigned long)roundup_bits2words64(nbits), (int)WORD_SIZE);

   DEBUG_VALIDATE(bitarr);

   return bitarr;
}

//
// Destructor
//
void bit_array_free(BIT_ARRAY* bitarr)
{
   if(bitarr->words != nullptr)
       free(bitarr->words);

   free(bitarr);
}

bit_index_t bit_array_length(const BIT_ARRAY* bit_arr)
{
   return bit_arr->num_of_bits;
}

// Change the size of a bit array. Enlarging an array will add zeros
// to the end of it. Returns 1 on success, 0 on failure (e.g. not enough memory)
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits)
{
   word_addr_t old_num_of_words = bitarr->num_of_words;
   word_addr_t new_num_of_words = roundup_bits2words64(new_num_of_bits);

   bitarr->num_of_bits = new_num_of_bits;
   bitarr->num_of_words = new_num_of_words;

   DEBUG_PRINT("Resize: old_num_of_words: %i; new_num_of_words: %i capacity: %i\n",
               (int)old_num_of_words, (int)new_num_of_words,
               (int)bitarr->capacity_in_words);

   if(new_num_of_words > bitarr->capacity_in_words)
   {
       // Need to change the amount of memory used
       word_addr_t old_capacity_in_words = bitarr->capacity_in_words;
       size_t old_capacity_in_bytes = old_capacity_in_words * sizeof(word_t);

       bitarr->capacity_in_words = roundup2pow(new_num_of_words);
       bitarr->capacity_in_words = MAX(8, bitarr->capacity_in_words);

       size_t new_capacity_in_bytes = bitarr->capacity_in_words * sizeof(word_t);
       bitarr->words = (word_t*)realloc(bitarr->words, new_capacity_in_bytes);

       if(bitarr->words == nullptr)
       {
           // error - could not allocate enough memory
           perror("resize realloc");
           errno = ENOMEM;
           return 0;
       }

       // Need to zero new memory
       size_t num_bytes_to_zero = new_capacity_in_bytes - old_capacity_in_bytes;
       memset(bitarr->words + old_capacity_in_words, 0, num_bytes_to_zero);

       DEBUG_PRINT("zeroing from word %i for %i bytes\n", (int)old_capacity_in_words,
                   (int)num_bytes_to_zero);
   }
   else if(new_num_of_words < old_num_of_words)
   {
       // Shrunk -- need to zero old memory
       size_t num_bytes_to_zero = (old_num_of_words - new_num_of_words)*sizeof(word_t);

       memset(bitarr->words + new_num_of_words, 0, num_bytes_to_zero);
   }

   // Mask top word
   _mask_top_word(bitarr);
   DEBUG_VALIDATE(bitarr);
   return 1;
}

void bit_array_resize_critical(BIT_ARRAY* bitarr, bit_index_t num_of_bits)
{
   bit_index_t old_num_of_bits = bitarr->num_of_bits;

   if(!bit_array_resize(bitarr, num_of_bits))
   {
       fprintf(stderr, "Ran out of memory resizing [%lu -> %lu]",
               (unsigned long)old_num_of_bits, (unsigned long)num_of_bits);
       abort();
   }
}

// If bitarr length < num_bits, resizes to num_bits
char bit_array_ensure_size(BIT_ARRAY* bitarr, bit_index_t ensure_num_of_bits)
{
   if(bitarr->num_of_bits < ensure_num_of_bits)
   {
       return bit_array_resize(bitarr, ensure_num_of_bits);
   }

   return 1;
}

void bit_array_ensure_size_critical(BIT_ARRAY* bitarr, bit_index_t num_of_bits)
{
   if(num_of_bits > bitarr->num_of_bits)
   {
       bit_array_resize_critical(bitarr, num_of_bits);
   }
}

static inline
       void _bit_array_ensure_nwords(BIT_ARRAY* bitarr, word_addr_t nwords,
                                const char *file, int lineno, const char *func)
{
   size_t newmem, oldmem;
   if(bitarr->capacity_in_words < nwords) {
       oldmem = bitarr->capacity_in_words * sizeof(word_t);
       bitarr->capacity_in_words = roundup2pow(nwords);
       newmem = bitarr->capacity_in_words * sizeof(word_t);
       bitarr->words = (word_t*)realloc(bitarr->words, newmem);

       if(bitarr->words == nullptr) {
           fprintf(stderr, "[%s:%i:%s()] Ran out of memory resizing [%zu -> %zu]",
                   file, lineno, func, oldmem, newmem);
           abort();
       }

       DEBUG_PRINT("Ensure nwords realloc %zu -> %zu\n", oldmem, newmem);
   }
}


//
// Get, set, clear, assign and toggle individual bits
//

// Get the value of a bit (returns 0 or 1)
char bit_array_get_bit(const BIT_ARRAY* bitarr, bit_index_t b)
{
   assert(b < bitarr->num_of_bits);
   return bit_array_get(bitarr, b);
}

// set a bit (to 1) at position b
void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
   assert(b < bitarr->num_of_bits);
   bit_array_set(bitarr,b);
   DEBUG_VALIDATE(bitarr);
}

// clear a bit (to 0) at position b
void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
   assert(b < bitarr->num_of_bits);
   bit_array_clear(bitarr, b);
   DEBUG_VALIDATE(bitarr);
}

// If bit is 0 -> 1, if bit is 1 -> 0.  AKA 'flip'
void bit_array_toggle_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
   assert(b < bitarr->num_of_bits);
   bit_array_toggle(bitarr, b);
   DEBUG_VALIDATE(bitarr);
}

// If char c != 0, set bit; otherwise clear bit
void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c)
{
   assert(b < bitarr->num_of_bits);
   bit_array_assign(bitarr, b, c ? 1 : 0);
   DEBUG_VALIDATE(bitarr);
}

//
// Get, set etc with resize
//

// Get the value of a bit (returns 0 or 1)
char bit_array_rget(BIT_ARRAY* bitarr, bit_index_t b)
{
   bit_array_ensure_size_critical(bitarr, b+1);
   return bit_array_get(bitarr, b);
}

// set a bit (to 1) at position b
void bit_array_rset(BIT_ARRAY* bitarr, bit_index_t b)
{
   bit_array_ensure_size_critical(bitarr, b+1);
   bit_array_set(bitarr,b);
   DEBUG_VALIDATE(bitarr);
}

// clear a bit (to 0) at position b
void bit_array_rclear(BIT_ARRAY* bitarr, bit_index_t b)
{
   bit_array_ensure_size_critical(bitarr, b+1);
   bit_array_clear(bitarr, b);
   DEBUG_VALIDATE(bitarr);
}

// If bit is 0 -> 1, if bit is 1 -> 0.  AKA 'flip'
void bit_array_rtoggle(BIT_ARRAY* bitarr, bit_index_t b)
{
   bit_array_ensure_size_critical(bitarr, b+1);
   bit_array_toggle(bitarr, b);
   DEBUG_VALIDATE(bitarr);
}

// If char c != 0, set bit; otherwise clear bit
void bit_array_rassign(BIT_ARRAY* bitarr, bit_index_t b, char c)
{
   bit_array_ensure_size_critical(bitarr, b+1);
   bit_array_assign(bitarr, b, c ? 1 : 0);
   DEBUG_VALIDATE(bitarr);
}

//
// Get, set, clear and toggle several bits at once
//

// Get the offsets of the set bits (for offsets start<=offset<end)
// Returns the number of bits set
// It is assumed that dst is at least of length (end-start)
bit_index_t bit_array_get_bits(const BIT_ARRAY* bitarr,
                              bit_index_t start, bit_index_t end,
                              bit_index_t* dst)
{
   bit_index_t i, n = 0;
   assert(end <= bitarr->num_of_bits);
   for(i = start; i < end; i++) {
       if(bit_array_get(bitarr, i)) {
           dst[n++] = i;
       }
   }
   return n;
}

// Set multiple bits at once.
// e.g. set bits 1, 20 & 31: bit_array_set_bits(bitarr, 3, 1,20,31);
void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
   size_t i;
   va_list argptr;
   va_start(argptr, n);

   for(i = 0; i < n; i++)
   {
       unsigned int bit_index = va_arg(argptr, unsigned int);
       bit_array_set_bit(bitarr, bit_index);
   }

   va_end(argptr);
   DEBUG_VALIDATE(bitarr);
}

// Clear multiple bits at once.
// e.g. clear bits 1, 20 & 31: bit_array_clear_bits(bitarr, 3, 1,20,31);
void bit_array_clear_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
   size_t i;
   va_list argptr;
   va_start(argptr, n);

   for(i = 0; i < n; i++)
   {
       unsigned int bit_index = va_arg(argptr, unsigned int);
       bit_array_clear_bit(bitarr, bit_index);
   }

   va_end(argptr);
   DEBUG_VALIDATE(bitarr);
}

// Toggle multiple bits at once
// e.g. toggle bits 1, 20 & 31: bit_array_toggle_bits(bitarr, 3, 1,20,31);
void bit_array_toggle_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
   size_t i;
   va_list argptr;
   va_start(argptr, n);

   for(i = 0; i < n; i++)
   {
       unsigned int bit_index = va_arg(argptr, unsigned int);
       bit_array_toggle_bit(bitarr, bit_index);
   }

   va_end(argptr);
   DEBUG_VALIDATE(bitarr);
}


//
// Set, clear and toggle all bits in a region
//

// Set all the bits in a region
void bit_array_set_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len)
{
   assert(start + len <= bitarr->num_of_bits);
   SET_REGION(bitarr, start, len);
   DEBUG_VALIDATE(bitarr);
}


// Clear all the bits in a region
void bit_array_clear_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len)
{
   assert(start + len <= bitarr->num_of_bits);
   CLEAR_REGION(bitarr, start, len);
   DEBUG_VALIDATE(bitarr);
}

// Toggle all the bits in a region
void bit_array_toggle_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len)
{
   assert(start + len <= bitarr->num_of_bits);
   TOGGLE_REGION(bitarr, start, len);
   DEBUG_VALIDATE(bitarr);
}


//
// Set, clear and toggle all bits at once
//

// set all elements of data to one
void bit_array_set_all(BIT_ARRAY* bitarr)
{
   bit_index_t num_of_bytes = bitarr->num_of_words * sizeof(word_t);
   memset(bitarr->words, 0xFF, num_of_bytes);
   _mask_top_word(bitarr);
   DEBUG_VALIDATE(bitarr);
}

// set all elements of data to zero
void bit_array_clear_all(BIT_ARRAY* bitarr)
{
   memset(bitarr->words, 0, bitarr->num_of_words * sizeof(word_t));
   DEBUG_VALIDATE(bitarr);
}

// Set all 1 bits to 0, and all 0 bits to 1. AKA flip
void bit_array_toggle_all(BIT_ARRAY* bitarr)
{
   word_addr_t i;
   for(i = 0; i < bitarr->num_of_words; i++)
   {
       bitarr->words[i] ^= WORD_MAX;
   }

   _mask_top_word(bitarr);
   DEBUG_VALIDATE(bitarr);
}

//
// Get a word at a time
//

uint64_t bit_array_get_word64(const BIT_ARRAY* bitarr, bit_index_t start)
{
   assert(start < bitarr->num_of_bits);
   return (uint64_t)_get_word(bitarr, start);
}

uint32_t bit_array_get_word32(const BIT_ARRAY* bitarr, bit_index_t start)
{
   assert(start < bitarr->num_of_bits);
   return (uint32_t)_get_word(bitarr, start);
}

uint16_t bit_array_get_word16(const BIT_ARRAY* bitarr, bit_index_t start)
{
   assert(start < bitarr->num_of_bits);
   return (uint16_t)_get_word(bitarr, start);
}

uint8_t bit_array_get_word8(const BIT_ARRAY* bitarr, bit_index_t start)
{
   assert(start < bitarr->num_of_bits);
   return (uint8_t)_get_word(bitarr, start);
}

uint64_t bit_array_get_wordn(const BIT_ARRAY* bitarr, bit_index_t start, int n)
{
   assert(start < bitarr->num_of_bits);
   assert(n <= 64);
   return (uint64_t)(_get_word(bitarr, start) & bitmask64(n));
}

//
// Set a word at a time
//
// Doesn't extend bit array. However it is safe to TRY to set bits beyond the
// end of the array, as long as: `start` is < `bit_array_length(arr)`
//

void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word)
{
   assert(start < bitarr->num_of_bits);
   _set_word(bitarr, start, (word_t)word);
}

void bit_array_set_word32(BIT_ARRAY* bitarr, bit_index_t start, uint32_t word)
{
   assert(start < bitarr->num_of_bits);
   word_t w = _get_word(bitarr, start);
   _set_word(bitarr, start, bitmask_merge(w, word, 0xffffffff00000000UL));
}

void bit_array_set_word16(BIT_ARRAY* bitarr, bit_index_t start, uint16_t word)
{
   assert(start < bitarr->num_of_bits);
   word_t w = _get_word(bitarr, start);
   _set_word(bitarr, start, bitmask_merge(w, word, 0xffffffffffff0000UL));
}

void bit_array_set_word8(BIT_ARRAY* bitarr, bit_index_t start, uint8_t byte)
{
   assert(start < bitarr->num_of_bits);
   _set_byte(bitarr, start, byte);
}

void bit_array_set_wordn(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word, int n)
{
   assert(start < bitarr->num_of_bits);
   assert(n <= 64);
   word_t w = _get_word(bitarr, start), m = bitmask64(n);
   _set_word(bitarr, start, bitmask_merge(word,w,m));
}

//
// Number of bits set
//

// Get the number of bits set (hamming weight)
bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr)
{
   word_addr_t i;

   bit_index_t num_of_bits_set = 0;

   for(i = 0; i < bitarr->num_of_words; i++)
   {
       if(bitarr->words[i] > 0)
       {
           num_of_bits_set += POPCOUNT(bitarr->words[i]);
       }
   }

   return num_of_bits_set;
}

// Get the number of bits not set (1 - hamming weight)
bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr)
{
   return bitarr->num_of_bits - bit_array_num_bits_set(bitarr);
}


// Get the number of bits set in on array and not the other.  This is equivalent
// to hamming weight of the XOR when the two arrays are the same length.
// e.g. 10101 vs 00111 => hamming distance 2 (XOR is 10010)
bit_index_t bit_array_hamming_distance(const BIT_ARRAY* arr1,
                                      const BIT_ARRAY* arr2)
{
   word_addr_t min_words = MIN(arr1->num_of_words, arr2->num_of_words);
   word_addr_t max_words = MAX(arr1->num_of_words, arr2->num_of_words);

   bit_index_t hamming_distance = 0;
   word_addr_t i;

   for(i = 0; i < min_words; i++)
   {
       hamming_distance += POPCOUNT(arr1->words[i] ^ arr2->words[i]);
   }

   if(min_words != max_words)
   {
       const BIT_ARRAY* long_arr
               = (arr1->num_of_words > arr2->num_of_words ? arr1 : arr2);

       for(i = min_words; i < max_words; i++)
       {
           hamming_distance += POPCOUNT(long_arr->words[i]);
       }
   }

   return hamming_distance;
}

// Parity - returns 1 if odd number of bits set, 0 if even
char bit_array_parity(const BIT_ARRAY* bitarr)
{
   word_addr_t w;
   unsigned int parity = 0;

   for(w = 0; w < bitarr->num_of_words; w++)
   {
       parity ^= PARITY(bitarr->words[w]);
   }

   return (char)parity;
}

//
// Find indices of set/clear bits
//

// Find the index of the next bit that is set/clear, at or after `offset`
// Returns 1 if such a bit is found, otherwise 0
// Index is stored in the integer pointed to by `result`
// If no such bit is found, value at `result` is not changed
#define _next_bit_func_def(FUNC,GET) \
char FUNC(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result) \
{ \
 assert(offset < bitarr->num_of_bits); \
 if(bitarr->num_of_bits == 0 || offset >= bitarr->num_of_bits) { return 0; } \
\
 /* Find first word that is greater than zero */ \
 word_addr_t i = bitset64_wrd(offset); \
 word_t w = GET(bitarr->words[i]) & ~bitmask64(bitset64_idx(offset)); \
\
 while(1) { \
   if(w > 0) { \
     bit_index_t pos = i * WORD_SIZE + trailing_zeros(w); \
     if(pos < bitarr->num_of_bits) { *result = pos; return 1; } \
     else { return 0; } \
   } \
   i++; \
   if(i >= bitarr->num_of_words) break; \
   w = GET(bitarr->words[i]); \
 } \
\
 return 0; \
}

// Find the index of the previous bit that is set/clear, before `offset`.
// Returns 1 if such a bit is found, otherwise 0
// Index is stored in the integer pointed to by `result`
// If no such bit is found, value at `result` is not changed
#define _prev_bit_func_def(FUNC,GET) \
char FUNC(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result) \
{ \
 assert(offset <= bitarr->num_of_bits); \
 if(bitarr->num_of_bits == 0 || offset == 0) { return 0; } \
\
 /* Find prev word that is greater than zero */ \
 word_addr_t i = bitset64_wrd(offset-1); \
 word_t w = GET(bitarr->words[i]) & bitmask64(bitset64_idx(offset-1)+1); \
\
 if(w > 0) { *result = (i+1) * WORD_SIZE - leading_zeros(w) - 1; return 1; } \
\
 /* i is unsigned so have to use break when i == 0 */ \
 for(--i; i != BIT_INDEX_MAX; i--) { \
   w = GET(bitarr->words[i]); \
   if(w > 0) { \
     *result = (i+1) * WORD_SIZE - leading_zeros(w) - 1; \
     return 1; \
   } \
 } \
\
 return 0; \
}

#define GET_WORD(x) (x)
#define NEG_WORD(x) (~(x))
_next_bit_func_def(bit_array_find_next_set_bit,  GET_WORD);
_next_bit_func_def(bit_array_find_next_clear_bit,NEG_WORD);
_prev_bit_func_def(bit_array_find_prev_set_bit,  GET_WORD);
_prev_bit_func_def(bit_array_find_prev_clear_bit,NEG_WORD);

// Find the index of the first bit that is set.
// Returns 1 if a bit is set, otherwise 0
// Index of first set bit is stored in the integer pointed to by result
// If no bits are set, value at `result` is not changed
char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
   return bit_array_find_next_set_bit(bitarr, 0, result);
}

// same same
char bit_array_find_first_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
   return bit_array_find_next_clear_bit(bitarr, 0, result);
}

// Find the index of the last bit that is set.
// Returns 1 if a bit is set, otherwise 0
// Index of last set bit is stored in the integer pointed to by `result`
// If no bits are set, value at `result` is not changed
char bit_array_find_last_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
   return bit_array_find_prev_set_bit(bitarr, bitarr->num_of_bits, result);
}

// same same
char bit_array_find_last_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
   return bit_array_find_prev_clear_bit(bitarr, bitarr->num_of_bits, result);
}

//
// "Sorting" bits
//

// Put all the 0s before all the 1s
void bit_array_sort_bits(BIT_ARRAY* bitarr)
{
   bit_index_t num_of_bits_set = bit_array_num_bits_set(bitarr);
   bit_index_t num_of_bits_cleared = bitarr->num_of_bits - num_of_bits_set;
   bit_array_set_all(bitarr);
   CLEAR_REGION(bitarr, 0, num_of_bits_cleared);
   DEBUG_VALIDATE(bitarr);
}

// Put all the 1s before all the 0s
void bit_array_sort_bits_rev(BIT_ARRAY* bitarr)
{
   bit_index_t num_of_bits_set = bit_array_num_bits_set(bitarr);
   bit_array_clear_all(bitarr);
   SET_REGION(bitarr, 0, num_of_bits_set);
   DEBUG_VALIDATE(bitarr);
}


//
// Strings and printing
//

// Construct a BIT_ARRAY from a substring with given on and off characters.
void bit_array_from_substr(BIT_ARRAY* bitarr, bit_index_t offset,
                          const char *str, size_t len,
                          const char *on, const char *off,
                          char left_to_right)
{
   bit_array_ensure_size(bitarr, offset + len);
   bit_array_clear_region(bitarr, offset, len);

   // BitArray region is now all 0s -- just set the 1s
   size_t i;
   bit_index_t j;

   for(i = 0; i < len; i++)
   {
       if(strchr(on, str[i]) != nullptr)
       {
           j = offset + (left_to_right ? i : len - i - 1);
           bit_array_set(bitarr, j);
       }
       else { assert(strchr(off, str[i]) != nullptr); }
   }

   DEBUG_VALIDATE(bitarr);
}

// From string method
void bit_array_from_str(BIT_ARRAY* bitarr, const char* str)
{
   bit_array_from_substr(bitarr, 0, str, strlen(str), "1", "0", 1);
}

// Takes a char array to write to.  `str` must be bitarr->num_of_bits+1 in length
// Terminates string with '\0'
char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str)
{
   bit_index_t i;

   for(i = 0; i < bitarr->num_of_bits; i++)
   {
       str[i] = bit_array_get(bitarr, i) ? '1' : '0';
   }

   str[bitarr->num_of_bits] = '\0';

   return str;
}

char* bit_array_to_str_rev(const BIT_ARRAY* bitarr, char* str)
{
   bit_index_t i;

   for(i = 0; i < bitarr->num_of_bits; i++)
   {
       str[i] = bit_array_get(bitarr, bitarr->num_of_bits-i-1) ? '1' : '0';
   }

   str[bitarr->num_of_bits] = '\0';

   return str;
}


// Get a string representations for a given region, using given on/off characters.
// Note: does not null-terminate
void bit_array_to_substr(const BIT_ARRAY* bitarr,
                        bit_index_t start, bit_index_t length,
                        char* str, char on, char off,
                        char left_to_right)
{
   assert(start + length <= bitarr->num_of_bits);

   bit_index_t i, j;
   bit_index_t end = start + length - 1;

   for(i = 0; i < length; i++)
   {
       j = (left_to_right ? start + i : end - i);
       str[i] = bit_array_get(bitarr, j) ? on : off;
   }

   //  str[length] = '\0';
}

// Print this array to a file stream.  Prints '0's and '1'.  Doesn't print newline.
void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout)
{
   bit_index_t i;

   for(i = 0; i < bitarr->num_of_bits; i++)
   {
       fprintf(fout, "%c", bit_array_get(bitarr, i) ? '1' : '0');
   }
}

void bit_array_print_stdout_newline(const BIT_ARRAY* bitarr) {
   bit_array_print(bitarr, stdout);
   fprintf(stdout, "\n");
}

// Print a string representations for a given region, using given on/off characters.
void bit_array_print_substr(const BIT_ARRAY* bitarr,
                           bit_index_t start, bit_index_t length,
                           FILE* fout, char on, char off,
                           char left_to_right)
{
   assert(start + length <= bitarr->num_of_bits);

   bit_index_t i, j;
   bit_index_t end = start + length - 1;

   for(i = 0; i < length; i++)
   {
       j = (left_to_right ? start + i : end - i);
       fprintf(fout, "%c", bit_array_get(bitarr, j) ? on : off);
   }
}

//
// Decimal
//

// Get bit array as decimal str (e.g. 0b1101 -> "13")
// len is the length of str char array -- will write at most len-1 chars
// returns the number of characters needed
// return is the same as strlen(str)
size_t bit_array_to_decimal(const BIT_ARRAY *bitarr, char *str, size_t len)
{
   size_t i = 0;

   if(bit_array_cmp_uint64(bitarr, 0) == 0)
   {
       if(len >= 2)
       {
           *str = '0';
           *(str+1) = '\0';
       }

       return 1;
   }

   BIT_ARRAY *tmp = bit_array_clone(bitarr);
   uint64_t rem;

   str[len-1] = '\0';

   while(bit_array_cmp_uint64(tmp, 0) != 0)
   {
       bit_array_div_uint64(tmp, 10, &rem);

       if(i < len-1)
       {
           str[len-2-i] = '0' + rem;
       }

       i++;
   }

   if(i < len-1)
   {
       // Moves null-terminator as well
       memmove(str, str+len-i-1, i+1);
   }

   bit_array_free(tmp);

   return i;
}

// Get bit array from decimal str (e.g. "13" -> 0b1101)
// Returns number of characters used
size_t bit_array_from_decimal(BIT_ARRAY *bitarr, const char* decimal)
{
   bit_array_clear_all(bitarr);
   size_t i = 0;

   if(decimal[0] == '\0' || decimal[0] < '0' || decimal[0] > '9')
   {
       return 0;
   }

   bit_array_add_uint64(bitarr, decimal[i] - '0');
   i++;

   while(decimal[i] != '\0' && decimal[i] >= '0' && decimal[i] <= '9')
   {
       bit_array_mul_uint64(bitarr, 10);
       bit_array_add_uint64(bitarr, decimal[i] - '0');
       i++;
   }

   return i;
}

//
// Hexidecimal
//

char bit_array_hex_to_nibble(char c, uint8_t *b)
{
   c = tolower(c);

   if(c >= '0' && c <= '9')
   {
       *b = c - '0';
       return 1;
   }
   else if(c >= 'a' && c <= 'f')
   {
       *b = 0xa + (c - 'a');
       return 1;
   }
   else
   {
       return 0;
   }
}

char bit_array_nibble_to_hex(uint8_t b, char uppercase)
{
   if(b <= 9)
   {
       return '0' + b;
   }
   else
   {
       return (uppercase ? 'A' : 'a') + (b - 0xa);
   }
}

// Loads array from hex string
// Returns the number of bits loaded (will be chars rounded up to multiple of 4)
// (0 on failure)
bit_index_t bit_array_from_hex(BIT_ARRAY* bitarr, bit_index_t offset,
                              const char* str, size_t len)
{
   if(str[0] == '0' && tolower(str[1]) == 'x')
   {
       str += 2;
       len -= 2;
   }

   size_t i;
   for(i = 0; i < len; i++, offset += 4)
   {
       uint8_t b;
       if(bit_array_hex_to_nibble(str[i], &b))
       {
           bit_array_ensure_size(bitarr, offset + 4);
           _set_nibble(bitarr, offset, b);
       }
       else
       {
           break;
       }
   }

   return 4 * i;
}

// Returns number of characters written
size_t bit_array_to_hex(const BIT_ARRAY* bitarr,
                       bit_index_t start, bit_index_t length,
                       char* str, char uppercase)
{
   assert(start + length <= bitarr->num_of_bits);

   size_t k = 0;
   bit_index_t offset, end = start + length;

   for(offset = start; offset + WORD_SIZE <= end; offset += WORD_SIZE)
   {
       word_t w = _get_word(bitarr, offset);

       word_offset_t j;
       for(j = 0; j < 64; j += 4)
       {
           str[k++] = bit_array_nibble_to_hex((w>>j) & 0xf, uppercase);
       }
   }

   if(offset < end)
   {
       // Remaining full nibbles (4 bits)
       word_t w = _get_word(bitarr, offset);

       for(; offset + 4 <= end; offset += 4)
       {
           str[k++] = bit_array_nibble_to_hex(w & 0xf, uppercase);
           w >>= 4;
       }

       if(offset < end)
       {
           // Remaining bits
           str[k++] = bit_array_nibble_to_hex(w & bitmask64(end - offset), uppercase);
       }
   }

   str[k] = '\0';

   // Return number of characters written
   return k;
}

// Print bit array as hex
size_t bit_array_print_hex(const BIT_ARRAY* bitarr,
                          bit_index_t start, bit_index_t length,
                          FILE* fout, char uppercase)
{
   assert(start + length <= bitarr->num_of_bits);

   size_t k = 0;
   bit_index_t offset, end = start + length;

   for(offset = start; offset + WORD_SIZE <= end; offset += WORD_SIZE)
   {
       word_t w = _get_word(bitarr, offset);

       word_offset_t j;
       for(j = 0; j < 64; j += 4)
       {
           fprintf(fout, "%c", bit_array_nibble_to_hex((w>>j) & 0xf, uppercase));
           k++;
       }
   }

   if(offset < end)
   {
       // Remaining full nibbles (4 bits)
       word_t w = _get_word(bitarr, offset);

       for(; offset + 4 <= end; offset += 4)
       {
           fprintf(fout, "%c", bit_array_nibble_to_hex(w & 0xf, uppercase));
           w >>= 4;
           k++;
       }

       if(offset < end)
       {
           // Remaining bits
           char hex = bit_array_nibble_to_hex(w & bitmask64(end - offset), uppercase);
           fprintf(fout, "%c", hex);
           k++;
       }
   }

   return k;
}

//
// Clone and copy
//

// Returns nullptr if cannot malloc
BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr)
{
   BIT_ARRAY* cpy = bit_array_create(bitarr->num_of_bits);

   if(cpy == nullptr)
   {
       return nullptr;
   }

   // Copy across bits
   memcpy(cpy->words, bitarr->words, bitarr->num_of_words * sizeof(word_t));

   DEBUG_VALIDATE(cpy);
   return cpy;
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
static void _array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                       const BIT_ARRAY* src, bit_index_t srcindx,
                       bit_index_t length)
{
   DEBUG_PRINT("bit_array_copy(dst: %zu, src: %zu, length: %zu)\n",
               (size_t)dstindx, (size_t)srcindx, (size_t)length);

   // Num of full words to copy
   word_addr_t num_of_full_words = length / WORD_SIZE;
   word_addr_t i;

   word_offset_t bits_in_last_word = bits_in_top_word(length);

   if(dst == src && srcindx > dstindx)
   {
       // Work left to right
       DEBUG_PRINT("work left to right\n");

       for(i = 0; i < num_of_full_words; i++)
       {
           word_t word = _get_word(src, srcindx+i*WORD_SIZE);
           _set_word(dst, dstindx+i*WORD_SIZE, word);
       }

       if(bits_in_last_word > 0)
       {
           word_t src_word = _get_word(src, srcindx+i*WORD_SIZE);
           word_t dst_word = _get_word(dst, dstindx+i*WORD_SIZE);

           word_t mask = bitmask64(bits_in_last_word);
           word_t word = bitmask_merge(src_word, dst_word, mask);

           _set_word(dst, dstindx+num_of_full_words*WORD_SIZE, word);
       }
   }
   else
   {
       // Work right to left
       DEBUG_PRINT("work right to left\n");

       for(i = 0; i < num_of_full_words; i++)
       {
           word_t word = _get_word(src, srcindx+length-(i+1)*WORD_SIZE);
           _set_word(dst, dstindx+length-(i+1)*WORD_SIZE, word);
       }

       DEBUG_PRINT("Copy %i,%i to %i\n", (int)srcindx, (int)bits_in_last_word,
                   (int)dstindx);

       if(bits_in_last_word > 0)
       {
           word_t src_word = _get_word(src, srcindx);
           word_t dst_word = _get_word(dst, dstindx);

           word_t mask = bitmask64(bits_in_last_word);
           word_t word = bitmask_merge(src_word, dst_word, mask);
           _set_word(dst, dstindx, word);
       }
   }

   _mask_top_word(dst);
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
void bit_array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                   const BIT_ARRAY* src, bit_index_t srcindx,
                   bit_index_t length)
{
   assert(srcindx + length <= src->num_of_bits);
   assert(dstindx <= dst->num_of_bits);
   _array_copy(dst, dstindx, src, srcindx, length);
   DEBUG_VALIDATE(dst);
}

// Clone `src` into `dst`. Resizes `dst`.
void bit_array_copy_all(BIT_ARRAY* dst, const BIT_ARRAY* src)
{
   bit_array_resize_critical(dst, src->num_of_bits);
   memmove(dst->words, src->words, src->num_of_words * sizeof(word_t));
   DEBUG_VALIDATE(dst);
}


//
// Logic operators
//

// Destination can be the same as one or both of the sources
void bit_array_and(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
   // Ensure dst array is big enough
   word_addr_t max_bits = MAX(src1->num_of_bits, src2->num_of_bits);
   bit_array_ensure_size_critical(dst, max_bits);

   word_addr_t min_words = MIN(src1->num_of_words, src2->num_of_words);

   word_addr_t i;

   for(i = 0; i < min_words; i++)
   {
       dst->words[i] = src1->words[i] & src2->words[i];
   }

   // Set remaining bits to zero
   for(i = min_words; i < dst->num_of_words; i++)
   {
       dst->words[i] = (word_t)0;
   }

   DEBUG_VALIDATE(dst);
}

// Destination can be the same as one or both of the sources
static void _logical_or_xor(BIT_ARRAY* dst,
                           const BIT_ARRAY* src1,
                           const BIT_ARRAY* src2,
                           char use_xor)
{
   // Ensure dst array is big enough
   bit_array_ensure_size_critical(dst, MAX(src1->num_of_bits, src2->num_of_bits));

   word_addr_t min_words = MIN(src1->num_of_words, src2->num_of_words);
   word_addr_t max_words = MAX(src1->num_of_words, src2->num_of_words);

   word_addr_t i;

   if(use_xor)
   {
       for(i = 0; i < min_words; i++)
           dst->words[i] = src1->words[i] ^ src2->words[i];
   }
   else
   {
       for(i = 0; i < min_words; i++)
           dst->words[i] = src1->words[i] | src2->words[i];
   }

   // Copy remaining bits from longer src array
   if(min_words != max_words)
   {
       const BIT_ARRAY* longer = src1->num_of_words > src2->num_of_words ? src1 : src2;

       for(i = min_words; i < max_words; i++)
       {
           dst->words[i] = longer->words[i];
       }
   }

   // Set remaining bits to zero
   size_t size = (dst->num_of_words - max_words) * sizeof(word_t);
   memset(dst->words + max_words, 0, size);

   DEBUG_VALIDATE(dst);
}

void bit_array_or(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
   _logical_or_xor(dst, src1, src2, 0);
}

// Destination can be the same as one or both of the sources
void bit_array_xor(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
   _logical_or_xor(dst, src1, src2, 1);
}

// If dst is longer than src, top bits are set to 1
void bit_array_not(BIT_ARRAY* dst, const BIT_ARRAY* src)
{
   bit_array_ensure_size_critical(dst, src->num_of_bits);

   word_addr_t i;

   for(i = 0; i < src->num_of_words; i++)
   {
       dst->words[i] = ~(src->words[i]);
   }

   // Set remaining words to 1s
   for(i = src->num_of_words; i < dst->num_of_words; i++)
   {
       dst->words[i] = WORD_MAX;
   }

   _mask_top_word(dst);

   DEBUG_VALIDATE(dst);
}

//
// Comparisons
//

// Compare two bit arrays by value stored, with index 0 being the Most
// Significant Bit (MSB). Arrays do not have to be the same length.
// Example: 10.. > 01.. [index 0 is MSB at left hand side]
// Sorts on length if all zeros: (0,0) < (0,0,0)
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
int bit_array_cmp_big_endian(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)

// compare bitarr with (bitarr2 << pos)
// bit_array_cmp(bitarr1, bitarr2<<pos)
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
int bit_array_cmp_words(const BIT_ARRAY *arr1,
                       bit_index_t pos, const BIT_ARRAY *arr2)



//
// Reverse -- coords may wrap around
//

// No bounds checking
// length cannot be zero
static void _reverse_region(BIT_ARRAY* bitarr,
                           bit_index_t start,
                           bit_index_t length)
{
   bit_index_t left = start;
   bit_index_t right = (start + length - WORD_SIZE) % bitarr->num_of_bits;

   while(length >= 2 * WORD_SIZE)
   {
       // Swap entire words
       word_t left_word = _get_word_cyclic(bitarr, left);
       word_t right_word = _get_word_cyclic(bitarr, right);

       // reverse words individually
       left_word = _reverse_word(left_word);
       right_word = _reverse_word(right_word);

       // Swap
       _set_word_cyclic(bitarr, left, right_word);
       _set_word_cyclic(bitarr, right, left_word);

       // Update
       left = (left + WORD_SIZE) % bitarr->num_of_bits;
       right = (right < WORD_SIZE ? right + bitarr->num_of_bits : right) - WORD_SIZE;
       length -= 2 * WORD_SIZE;
   }

   word_t word, rev;

   if(length == 0)
   {
       return;
   }
   else if(length > WORD_SIZE)
   {
       // Words overlap
       word_t left_word = _get_word_cyclic(bitarr, left);
       word_t right_word = _get_word_cyclic(bitarr, right);

       rev = _reverse_word(left_word);
       right_word = _reverse_word(right_word);

       // fill left 64 bits with right word rev
       _set_word_cyclic(bitarr, left, right_word);

       // Now do remaining bits (length is between 1 and 64 bits)
       left += WORD_SIZE;
       length -= WORD_SIZE;

       word = _get_word_cyclic(bitarr, left);
   }
   else
   {
       word = _get_word_cyclic(bitarr, left);
       rev = _reverse_word(word);
   }

   rev >>= WORD_SIZE - length;
   word_t mask = bitmask64(length);

   word = bitmask_merge(rev, word, mask);

   _set_word_cyclic(bitarr, left, word);
}

void bit_array_reverse_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len)
{
   assert(start + len <= bitarr->num_of_bits);
   if(len > 0) _reverse_region(bitarr, start, len);
   DEBUG_VALIDATE(bitarr);
}

void bit_array_reverse(BIT_ARRAY* bitarr)
{
   if(bitarr->num_of_bits > 0) _reverse_region(bitarr, 0, bitarr->num_of_bits);
   DEBUG_VALIDATE(bitarr);
}

//
// Shift left / right
//

// Shift towards MSB / higher index
void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
   if(shift_dist >= bitarr->num_of_bits)
   {
       fill ? bit_array_set_all(bitarr) : bit_array_clear_all(bitarr);
       return;
   }
   else if(shift_dist == 0)
   {
       return;
   }

   FillAction action = fill ? FILL_REGION : ZERO_REGION;

   bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
   _array_copy(bitarr, shift_dist, bitarr, 0, cpy_length);
   _set_region(bitarr, 0, shift_dist, action);
}

// shift left extend - don't truncate bits when shifting UP, instead
// make room for them.
void bit_array_shift_left_extend(BIT_ARRAY* bitarr, bit_index_t shift_dist,
                                char fill)
{
   bit_index_t newlen = bitarr->num_of_bits + shift_dist;
   bit_index_t cpy_length = bitarr->num_of_bits;

   if(shift_dist == 0)
   {
       return;
   }

   bit_array_resize_critical(bitarr, newlen);

   FillAction action = fill ? FILL_REGION : ZERO_REGION;
   _array_copy(bitarr, shift_dist, bitarr, 0, cpy_length);
   _set_region(bitarr, 0, shift_dist, action);
}

// Shift towards LSB / lower index
void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
   if(shift_dist >= bitarr->num_of_bits)
   {
       fill ? bit_array_set_all(bitarr) : bit_array_clear_all(bitarr);
       return;
   }
   else if(shift_dist == 0)
   {
       return;
   }

   FillAction action = fill ? FILL_REGION : ZERO_REGION;

   bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
   bit_array_copy(bitarr, 0, bitarr, shift_dist, cpy_length);

   _set_region(bitarr, cpy_length, shift_dist, action);
}

//
// Cycle
//

// Cycle towards index 0
void bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
   if(bitarr->num_of_bits == 0)
   {
       return;
   }

   cycle_dist = cycle_dist % bitarr->num_of_bits;

   if(cycle_dist == 0)
   {
       return;
   }

   bit_index_t len1 = cycle_dist;
   bit_index_t len2 = bitarr->num_of_bits - cycle_dist;

   _reverse_region(bitarr, 0, len1);
   _reverse_region(bitarr, len1, len2);
   bit_array_reverse(bitarr);
}

// Cycle away from index 0
void bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
   if(bitarr->num_of_bits == 0)
   {
       return;
   }

   cycle_dist = cycle_dist % bitarr->num_of_bits;

   if(cycle_dist == 0)
   {
       return;
   }

   bit_index_t len1 = bitarr->num_of_bits - cycle_dist;
   bit_index_t len2 = cycle_dist;

   _reverse_region(bitarr, 0, len1);
   _reverse_region(bitarr, len1, len2);
   bit_array_reverse(bitarr);
}

//
// Next permutation
//

//
// Interleave
//

// dst cannot point to the same bit array as src1 or src2
// src1, src2 may point to the same bit array
// abcd 1234 -> a1b2c3d4
// 0011 0000 -> 00001010
// 1111 0000 -> 10101010
// 0101 1010 -> 01100110
void bit_array_interleave(BIT_ARRAY* dst,
                         const BIT_ARRAY* src1,
                         const BIT_ARRAY* src2)
{
   // dst cannot be either src1 or src2
   assert(dst != src1 && dst != src2);
   // Behaviour undefined when src1 length != src2 length",
   assert(src1->num_of_bits == src2->num_of_bits);

   // Need at least src1->num_of_words + src2->num_of_words
   size_t nwords = MIN(src1->num_of_words + src2->num_of_words, 2);
   _bit_array_ensure_nwords(dst, nwords, __FILE__, __LINE__, __func__);
   dst->num_of_bits = src1->num_of_bits + src2->num_of_bits;
   dst->num_of_words = roundup_bits2words64(dst->num_of_bits);

   word_addr_t i, j;

   for(i = 0, j = 0; i < src1->num_of_words; i++)
   {
       word_t a = src1->words[i];
       word_t b = src2->words[i];

       dst->words[j++] =  morton_table0[(a      ) & 0xff] |
                         morton_table1[(b      ) & 0xff] |
                         (morton_table0[(a >>  8) & 0xff] << 16) |
                         (morton_table1[(b >>  8) & 0xff] << 16) |
                         (morton_table0[(a >> 16) & 0xff] << 32) |
                         (morton_table1[(b >> 16) & 0xff] << 32) |
                         (morton_table0[(a >> 24) & 0xff] << 48) |
                         (morton_table1[(b >> 24) & 0xff] << 48);

       dst->words[j++] =  morton_table0[(a >> 32) & 0xff] |
                         morton_table1[(b >> 32) & 0xff] |
                         (morton_table0[(a >> 40) & 0xff] << 16) |
                         (morton_table1[(b >> 40) & 0xff] << 16) |
                         (morton_table0[(a >> 48) & 0xff] << 32) |
                         (morton_table1[(b >> 48) & 0xff] << 32) |
                         (morton_table0[(a >> 56)       ] << 48) |
                         (morton_table1[(b >> 56)       ] << 48);
   }

   DEBUG_VALIDATE(dst);
}

//
// Arithmetic
//

// Returns 1 on success, 0 if value in array is too big
char bit_array_as_num(const BIT_ARRAY* bitarr, uint64_t* result)
{
   if(bitarr->num_of_bits == 0)
   {
       *result = 0;
       return 1;
   }

   word_addr_t w;

   for(w = bitarr->num_of_words-1; w > 0; w--)
   {
       if(bitarr->words[w] > 0)
       {
           return 0;
       }
   }

   *result = bitarr->words[0];
   return 1;
}


// 1 iff bitarr > value
// 0 iff bitarr == value
// -1 iff bitarr < value
int bit_array_cmp_uint64(const BIT_ARRAY* bitarr, uint64_t value)
{
   uint64_t arr_num = 0;

   // If cannot put bitarr in uint64, it is > value
   if(!bit_array_as_num(bitarr, &arr_num)) return 1;

   if(arr_num > value)      return  1;
   else if(arr_num < value) return -1;
   else                     return  0;
}

// If value is zero, no change is made
void bit_array_add_uint64(BIT_ARRAY* bitarr, uint64_t value)
{
   if(value == 0)
   {
       return;
   }
   else if(bitarr->num_of_bits == 0)
   {
       bit_array_resize_critical(bitarr, WORD_SIZE - leading_zeros(value));
       bitarr->words[0] = (word_t)value;
       return;
   }

   char carry = 0;
   word_addr_t i;

   for(i = 0; i < bitarr->num_of_words; i++)
   {
       if(WORD_MAX - bitarr->words[i] < value)
       {
           carry = 1;
           bitarr->words[i] += value;
           value = 1;
       }
       else
       {
           // Carry is absorbed
           bitarr->words[i] += value;
           carry = 0;
           break;
       }
   }

   if(carry)
   {
       // Bit array full, need another bit after all words filled
       bit_array_resize_critical(bitarr, bitarr->num_of_words * WORD_SIZE + 1);

       // Set top word to 1
       bitarr->words[bitarr->num_of_words-1] = 1;
   }
   else
   {
       word_t final_word = bitarr->words[bitarr->num_of_words-1];
       word_offset_t expected_bits = bits_in_top_word(bitarr->num_of_bits);
       word_offset_t actual_bits = WORD_SIZE - leading_zeros(final_word);

       if(actual_bits > expected_bits)
       {
           // num_of_bits has increased -- num_of_words has not
           bitarr->num_of_bits += (actual_bits - expected_bits);
       }
   }
}

// If value is greater than bitarr, bitarr is not changed and 0 is returned
// Returns 1 on success, 0 if value > bitarr
char bit_array_sub_uint64(BIT_ARRAY* bitarr, uint64_t value)
{
   if(value == 0)
   {
       return 1;
   }
   else if(bitarr->words[0] >= value)
   {
       bitarr->words[0] -= value;
       return 1;
   }

   value -= bitarr->words[0];

   word_addr_t i;

   for(i = 1; i < bitarr->num_of_words; i++)
   {
       if(bitarr->words[i] > 0)
       {
           // deduct one
           bitarr->words[i]--;

           for(; i > 0; i--)
           {
               bitarr->words[i] = WORD_MAX;
           }

           // -1 since we've already deducted 1
           bitarr->words[0] = WORD_MAX - value - 1;

           return 1;
       }
   }

   // subtract value is greater than array
   return 0;
}

//
// Arithmetic between bit arrays
//

// src1, src2 and dst can all be the same BIT_ARRAY
static void _arithmetic(BIT_ARRAY* dst,
                       const BIT_ARRAY* src1,
                       const BIT_ARRAY* src2,
                       char subtract)
{
   word_addr_t max_words = MAX(src1->num_of_words, src2->num_of_words);

   // Adding: dst_words >= max(src1 words, src2 words)
   // Subtracting: dst_words is >= src1->num_of_words

   char carry = subtract ? 1 : 0;

   word_addr_t i;
   word_t word1, word2;

   for(i = 0; i < max_words; i++)
   {
       word1 = (i < src1->num_of_words ? src1->words[i] : 0);
       word2 = (i < src2->num_of_words ? src2->words[i] : 0);

       if(subtract)
           word2 = ~word2;

       dst->words[i] = word1 + word2 + carry;
       // Update carry
       carry = WORD_MAX - word1 < word2 || WORD_MAX - word1 - word2 < (word_t)carry;
   }

   if(subtract)
   {
       carry = 0;
   }
   else
   {
       // Check last word
       word_offset_t bits_on_last_word = bits_in_top_word(dst->num_of_bits);

       if(bits_on_last_word < WORD_SIZE)
       {
           word_t mask = bitmask64(bits_on_last_word);

           if(dst->words[max_words-1] > mask)
           {
               // Array has overflowed, increase size
               dst->num_of_bits++;
           }
       }
       else if(carry)
       {
           // Carry onto a new word
           if(dst->num_of_words == max_words)
           {
               // Need to resize for the carry bit
               bit_array_resize_critical(dst, dst->num_of_bits+1);
           }

           dst->words[max_words] = (word_t)1;
       }
   }

   // Zero the rest of dst array
   for(i = max_words+carry; i < dst->num_of_words; i++)
   {
       dst->words[i] = (word_t)0;
   }

   DEBUG_VALIDATE(dst);
}

// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than either of src1, src2, it is enlarged
void bit_array_add(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
   bit_array_ensure_size_critical(dst, MAX(src1->num_of_bits, src2->num_of_bits));
   _arithmetic(dst, src1, src2, 0);
}

// dst = src1 - src2
// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than src1, it will be extended to be as long as src1
// src1 must be greater than or equal to src2 (src1 >= src2)
void bit_array_subtract(BIT_ARRAY* dst,
                       const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
   // subtraction by method of complements:
   // a - b = a + ~b + 1 = src1 + ~src2 +1

   assert(bit_array_cmp(src1, src2) >= 0); // Require src1 >= src2

   bit_array_ensure_size_critical(dst, src1->num_of_bits);
   _arithmetic(dst, src1, src2, 1);
}


// Add `add` to `bitarr` at `pos`
// Bounds checking not needed as out of bounds is valid
void bit_array_add_word(BIT_ARRAY *bitarr, bit_index_t pos, uint64_t add)
{
   DEBUG_VALIDATE(bitarr);

   if(add == 0)
   {
       return;
   }
   else if(pos >= bitarr->num_of_bits)
   {
       // Resize and add!
       bit_index_t num_bits_required = pos + (WORD_SIZE - leading_zeros(add));
       bit_array_resize_critical(bitarr, num_bits_required);
       _set_word(bitarr, pos, (word_t)add);
       return;
   }

   /*
   char str[1000];
   printf(" add_word: %s\n", bit_array_to_str_rev(bitarr, str));
   printf("     word: %s [pos: %i]\n", _word_to_str(add, str), (int)pos);
   */

   word_t w = _get_word(bitarr, pos);
   word_t sum = w + add;
   char carry = WORD_MAX - w < add;

   // Ensure array is big enough
   bit_index_t num_bits_required = pos + (carry ? WORD_SIZE + 1
                                                : (WORD_SIZE - leading_zeros(sum)));

   bit_array_ensure_size(bitarr, num_bits_required);

   _set_word(bitarr, pos, sum);
   pos += WORD_SIZE;

   if(carry)
   {
       word_offset_t offset = pos % WORD_SIZE;
       word_addr_t addr = bitset64_wrd(pos);

       add = (word_t)0x1 << offset;
       carry = (WORD_MAX - bitarr->words[addr] < add);
       sum = bitarr->words[addr] + add;

       num_bits_required = addr * WORD_SIZE +
                           (carry ? WORD_SIZE + 1 : (WORD_SIZE - leading_zeros(sum)));

       bit_array_ensure_size(bitarr, num_bits_required);

       bitarr->words[addr++] = sum;

       if(carry)
       {
           while(addr < bitarr->num_of_words && bitarr->words[addr] == WORD_MAX)
           {
               bitarr->words[addr++] = 0;
           }

           if(addr == bitarr->num_of_words)
           {
               bit_array_resize_critical(bitarr, addr * WORD_SIZE + 1);
           }
           else if(addr == bitarr->num_of_words-1 &&
                    bitarr->words[addr] == bitmask64(bits_in_top_word(bitarr->num_of_bits)))
           {
               bit_array_resize_critical(bitarr, bitarr->num_of_bits + 1);
           }

           bitarr->words[addr]++;
       }
   }

   DEBUG_VALIDATE(bitarr);
}

// Add `add` to `bitarr` at `pos`
// Bounds checking not needed as out of bounds is valid
void bit_array_add_words(BIT_ARRAY *bitarr, bit_index_t pos, const BIT_ARRAY *add)
{
   assert(bitarr != add); // bitarr and add cannot point to the same bit array

   bit_index_t add_top_bit_set;

   if(!bit_array_find_last_set_bit(add, &add_top_bit_set))
   {
       // No bits set in add
       return;
   }
   else if(pos >= bitarr->num_of_bits)
   {
       // Just resize and copy!
       bit_index_t num_bits_required = pos + add_top_bit_set + 1;
       bit_array_resize_critical(bitarr, num_bits_required);
       _array_copy(bitarr, pos, add, 0, add->num_of_bits);
       return;
   }
   else if(pos == 0)
   {
       bit_array_add(bitarr, bitarr, add);
       return;
   }

   /*
   char str[1000];
   printf(" add_words1: %s\n", bit_array_to_str_rev(bitarr, str));
   printf(" add_words2: %s\n", bit_array_to_str_rev(add, str));
   printf(" [pos: %i]\n", (int)pos);
   */

   bit_index_t num_bits_required = pos + add_top_bit_set + 1;
   bit_array_ensure_size(bitarr, num_bits_required);

   word_addr_t first_word = bitset64_wrd(pos);
   word_offset_t first_offset = bitset64_idx(pos);

   word_t w = add->words[0] << first_offset;
   unsigned char carry = (WORD_MAX - bitarr->words[first_word] < w);

   bitarr->words[first_word] += w;

   word_addr_t i = first_word + 1;
   bit_index_t offset = WORD_SIZE - first_offset;

   for(; carry || offset <= add_top_bit_set; i++, offset += WORD_SIZE)
   {
       w = offset < add->num_of_bits ? _get_word(add, offset) : (word_t)0;

       if(i >= bitarr->num_of_words)
       {
           // Extend by a word
           bit_array_resize_critical(bitarr, (bit_index_t)(i+1)*WORD_SIZE+1);
       }

       word_t prev = bitarr->words[i];

       bitarr->words[i] += w + carry;

       carry = (WORD_MAX - prev < w || (carry && prev + w == WORD_MAX)) ? 1 : 0;
   }

   word_offset_t top_bits
           = WORD_SIZE - leading_zeros(bitarr->words[bitarr->num_of_words-1]);

   bit_index_t min_bits = (bitarr->num_of_words-1)*WORD_SIZE + top_bits;

   if(bitarr->num_of_bits < min_bits)
   {
       // Extend within the last word
       bitarr->num_of_bits = min_bits;
   }

   DEBUG_VALIDATE(bitarr);
}

char bit_array_sub_word(BIT_ARRAY* bitarr, bit_index_t pos, word_t minus)
{
   DEBUG_VALIDATE(bitarr);

   if(minus == 0)
   {
       return 1;
   }

   word_t w = _get_word(bitarr, pos);

   if(w >= minus)
   {
       _set_word(bitarr, pos, w - minus);
       DEBUG_VALIDATE(bitarr);
       return 1;
   }

   minus -= w;

   bit_index_t offset;
   for(offset = pos + WORD_SIZE; offset < bitarr->num_of_bits; offset += WORD_SIZE)
   {
       w = _get_word(bitarr, offset);

       if(w > 0)
       {
           // deduct one
           _set_word(bitarr, offset, w - 1);

           SET_REGION(bitarr, pos, offset-pos);

           // -1 since we've already deducted 1
           minus--;

           _set_word(bitarr, pos, WORD_MAX - minus);

           DEBUG_VALIDATE(bitarr);
           return 1;
       }
   }

   DEBUG_VALIDATE(bitarr);

   return 0;
}

char bit_array_sub_words(BIT_ARRAY* bitarr, bit_index_t pos, BIT_ARRAY* minus)
{
   assert(bitarr != minus); // bitarr and minus cannot point to the same bit array

   int cmp = bit_array_cmp_words(bitarr, pos, minus);

   if(cmp == 0)
   {
       bit_array_clear_all(bitarr);
       return 1;
   }
   else if(cmp < 0)
   {
       return 0;
   }

   bit_index_t bitarr_length = bitarr->num_of_bits;

   bit_index_t bitarr_top_bit_set;
   bit_array_find_last_set_bit(bitarr, &bitarr_top_bit_set);

   // subtraction by method of complements:
   // a - b = a + ~b + 1 = src1 + ~src2 +1

   bit_array_not(minus, minus);

   bit_array_add_words(bitarr, pos, minus);
   bit_array_add_word(bitarr, pos, (word_t)1);

   bit_array_sub_word(bitarr, pos+minus->num_of_bits, 1);
   bit_array_resize(bitarr, bitarr_length);

   bit_array_not(minus, minus);

   DEBUG_VALIDATE(bitarr);

   return 1;
}

void bit_array_mul_uint64(BIT_ARRAY *bitarr, uint64_t multiplier)
{
   if(bitarr->num_of_bits == 0 || multiplier == 1)
   {
       return;
   }
   else if(multiplier == 0)
   {
       bit_array_clear_all(bitarr);
       return;
   }

   bit_index_t i;

   for(i = bitarr->num_of_bits; i > 0; i--)
   {
       if(bit_array_get(bitarr, i-1))
       {
           bit_array_clear(bitarr, i-1);
           bit_array_add_word(bitarr, i-1, multiplier);
       }
   }

   DEBUG_VALIDATE(bitarr);
}

void bit_array_multiply(BIT_ARRAY *dst, BIT_ARRAY *src1, BIT_ARRAY *src2)
{
   if(src1->num_of_bits == 0 || src2->num_of_bits == 0)
   {
       bit_array_clear_all(dst);
       return;
   }

   // Cannot pass the same array as dst, src1 AND src2
   assert(dst != src1 || dst != src2);

   // Dev: multiplier == 1?

   BIT_ARRAY *read_arr, *add_arr;

   if(src1 == dst)
   {
       read_arr = src1;
       add_arr = src2;
   }
   else
   {
       read_arr = src2;
       add_arr = src1;
   }

   if(dst != src1 && dst != src2)
   {
       bit_array_clear_all(dst);
   }

   bit_index_t i;

   for(i = read_arr->num_of_bits; i > 0; i--)
   {
       if(bit_array_get(read_arr, i-1))
       {
           bit_array_clear(dst, i-1);
           bit_array_add_words(dst, i-1, add_arr);
       }
   }

   DEBUG_VALIDATE(dst);
}

// bitarr = round_down(bitarr / divisor)
// rem = bitarr % divisor
void bit_array_div_uint64(BIT_ARRAY *bitarr, uint64_t divisor, uint64_t *rem)
{
   assert(divisor != 0); // cannot divide by zero

   bit_index_t div_top_bit = 63 - leading_zeros(divisor);
   bit_index_t bitarr_top_bit;

   if(!bit_array_find_last_set_bit(bitarr, &bitarr_top_bit))
   {
       *rem = 0;
       return;
   }

   if(bitarr_top_bit < div_top_bit)
   {
       *rem = bitarr->words[0];
       bit_array_clear_all(bitarr);
       return;
   }

   // When div is shifted by offset, their top set bits are aligned
   bit_index_t offset = bitarr_top_bit - div_top_bit;

   uint64_t tmp = _get_word(bitarr, offset);
   _set_word(bitarr, offset, (word_t)0);

   // Carry if 1 if the top bit was set before left shift
   char carry = 0;

   // offset unsigned so break when offset == 0
   while(1)
   {
       if(carry)
       {
           // (carry:tmp) - divisor = (WORD_MAX+1+tmp)-divisor
           tmp = WORD_MAX - divisor + tmp + 1;
           bit_array_set(bitarr, offset);
       }
       else if(tmp >= divisor)
       {
           tmp -= divisor;
           bit_array_set(bitarr, offset);
       }
       else
       {
           bit_array_clear(bitarr, offset);
       }

       if(offset == 0)
           break;

       offset--;

       // Is the top bit set (that we're about to shift off)?
       carry = tmp & 0x8000000000000000;

       tmp <<= 1;
       tmp |= bit_array_get(bitarr, offset);
   }

   *rem = tmp;
}

// Results in:
//   quotient = dividend / divisor
//   dividend = dividend % divisor
// (dividend is used to return the remainder)
void bit_array_divide(BIT_ARRAY *dividend, BIT_ARRAY *quotient, BIT_ARRAY *divisor)
{
   assert(bit_array_cmp_uint64(divisor, 0) != 0); // Cannot divide by zero

   bit_array_clear_all(quotient);

   int cmp = bit_array_cmp(dividend, divisor);

   if(cmp == 0)
   {
       bit_array_ensure_size(quotient, 1);
       bit_array_set(quotient, 0);
       bit_array_clear_all(dividend);
       return;
   }
   else if(cmp < 0)
   {
       // dividend is < divisor, quotient is zero -- done
       return;
   }

   // now we know: dividend > divisor, quotient is zero'd,
   //              dividend != 0, divisor != 0
   bit_index_t dividend_top_bit = 0, div_top_bit = 0;

   bit_array_find_last_set_bit(dividend, &dividend_top_bit);
   bit_array_find_last_set_bit(divisor, &div_top_bit);

   // When divisor is shifted by offset, their top set bits are aligned
   bit_index_t offset = dividend_top_bit - div_top_bit;

   // offset unsigned so break when offset == 0
   for(; ; offset--)
   {
       if(bit_array_cmp_words(dividend, offset, divisor) >= 0)
       {
           bit_array_sub_words(dividend, offset, divisor);
           bit_array_ensure_size(quotient, offset+1);
           bit_array_set(quotient, offset);
       }

       if(offset == 0)
           break;
   }
}
