#ifndef BITWISE_BIT_ARRAY_H
#define BITWISE_BIT_ARRAY_H

/**
 * @file bit_array.h
 * @brief Dynamic bit array data structure and associated operations.
 *
 * Provides definitions and functions for manipulating dynamic bit arrays.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

#include <cstdio>
#include <cinttypes>

#include "../bit_macros.h"

/**
 * @typedef BIT_ARRAY
 * @brief Opaque struct representing a dynamic bit array.
 *
 * Use the provided functions to operate on BIT_ARRAY objects.
 */
typedef struct BIT_ARRAY BIT_ARRAY;

/**
 * @typedef word_t
 * @brief 64-bit word type used internally.
 */
typedef uint64_t word_t;

/**
 * @typedef word_addr_t
 * @brief Word address type used for indexing words within the array.
 */
typedef uint64_t word_addr_t;

/**
 * @typedef bit_index_t
 * @brief Bit index type used for indexing bits within the array.
 */
typedef uint64_t bit_index_t;

/**
 * @typedef word_offset_t
 * @brief Offset within a 64-bit word (0 to 63).
 */
typedef uint8_t word_offset_t; // Offset within a 64-bit word

/**
 * @def BIT_INDEX_MIN
 * @brief Minimum valid bit index (0).
 */
#define BIT_INDEX_MIN 0

/**
 * @def BIT_INDEX_MAX
 * @brief Maximum valid bit index.
 */
#define BIT_INDEX_MAX (~(bit_index_t)0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct BIT_ARRAY
 * @brief Dynamic bit array structure.
 *
 * Represents a dynamic array of bits, supporting operations such as setting,
 * clearing, toggling bits, and various bitwise manipulations.
 */
struct BIT_ARRAY
{
    /**
     * @brief Pointer to the array of 64-bit words storing the bits.
     */
    word_t* words;

    /**
     * @brief Number of bits currently in the bit array.
     */
    bit_index_t num_of_bits;

    /**
     * @brief Number of words used.
     *
     * Calculated as `round_up(num_of_bits / 64)`.
     * If `num_of_bits == 0`, this is 0.
     */
    word_addr_t num_of_words;

    /**
     * @brief Capacity of the words array in words.
     *
     * For efficient allocation, realloc is used to double the size
     * when necessary, rather than adding every word individually.
     * Initial size is `INIT_CAPACITY_WORDS`.
     */
    word_addr_t capacity_in_words;
};

/**
 * @brief Create a new bit array with a specified number of bits.
 *
 * Allocates and initializes a new `BIT_ARRAY` with all bits set to 0.
 *
 * @param nbits The number of bits in the new bit array.
 * @return A pointer to the newly created `BIT_ARRAY`, or `NULL` on failure.
 *
 * @note The bit array should be freed with `bit_array_free()` when no longer needed.
 *
 * @example
 * @code
 * BIT_ARRAY* my_bitarray = bit_array_create(128);
 * // Use the bit array...
 * bit_array_free(my_bitarray);
 * @endcode
 */
BIT_ARRAY* bit_array_create(bit_index_t nbits);

/**
 * @brief Free the memory used by a bit array.
 *
 * Destroys a `BIT_ARRAY`, freeing its memory.
 *
 * @param bitarray The bit array to free.
 *
 * @note After calling this function, the pointer to the bit array becomes invalid.
 *
 * @example
 * @code
 * bit_array_free(my_bitarray);
 * @endcode
 */
void bit_array_free(BIT_ARRAY* bitarray);

/**
 * @brief Allocate a bit array using an existing struct.
 *
 * Initializes an existing `BIT_ARRAY` struct to have a specified number of bits.
 *
 * @param bitarr Pointer to the `BIT_ARRAY` struct to initialize.
 * @param nbits Number of bits for the bit array.
 * @return Pointer to the initialized `BIT_ARRAY` struct, or `NULL` on failure.
 *
 * @note The bit array should be deallocated with `bit_array_dealloc()` when no longer needed.
 *
 * @example
 * @code
 * BIT_ARRAY bitarr;
 * if (bit_array_alloc(&bitarr, 85)) {
 *     // Use the 85-bit bitarr...
 *     bit_array_dealloc(&bitarr);
 * }
 * @endcode
 */
BIT_ARRAY* bit_array_alloc(BIT_ARRAY* bitarr, bit_index_t nbits);

/**
 * @brief Deallocate the bit array allocated with `bit_array_alloc()`.
 *
 * Frees the memory associated with the `BIT_ARRAY` struct's internal data.
 *
 * @param bitarr Pointer to the `BIT_ARRAY` struct to deallocate.
 *
 * @example
 * @code
 * bit_array_dealloc(&bitarr);
 * @endcode
 */
void bit_array_dealloc(BIT_ARRAY* bitarr);

/**
 * @brief Get the length of the bit array.
 *
 * Returns the number of bits in the bit array.
 *
 * @param bit_arr The bit array whose length is to be retrieved.
 * @return The number of bits in the bit array.
 *
 * @example
 * @code
 * bit_index_t len = bit_array_length(bitarr);
 * @endcode
 */
bit_index_t bit_array_length(const BIT_ARRAY* bit_arr);

/**
 * @brief Change the size of a bit array.
 *
 * Resizes the bit array to hold a new number of bits. If the array is enlarged,
 * new bits are added to the end and initialized to 0.
 *
 * @param bitarr The bit array to resize.
 * @param new_num_of_bits The new number of bits for the bit array.
 * @return `1` on success, `0` on failure (e.g., not enough memory).
 *
 * @example
 * @code
 * if (!bit_array_resize(bitarr, 261)) {
 *     // Handle error...
 * }
 * @endcode
 */
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits);

/**
 * @brief Ensure the bit array is at least a certain size.
 *
 * If the bit array's length is less than `ensure_num_of_bits`, it is resized to
 * `ensure_num_of_bits`. If the array is enlarged, new bits are added to the end
 * and initialized to 0.
 *
 * @param bitarr The bit array to ensure size.
 * @param ensure_num_of_bits The minimum number of bits required.
 * @return `1` on success, `0` on failure (e.g., not enough memory).
 *
 * @example
 * @code
 * if (!bit_array_ensure_size(bitarr, 512)) {
 *     // Handle error...
 * }
 * @endcode
 */
char bit_array_ensure_size(BIT_ARRAY* bitarr, bit_index_t ensure_num_of_bits);

/**
 * @brief Resize the bit array, exiting on failure.
 *
 * Similar to `bit_array_resize()`, but exits the program with an error message
 * if memory allocation fails.
 *
 * @param bitarr The bit array to resize.
 * @param num_of_bits The new number of bits for the bit array.
 *
 * @example
 * @code
 * bit_array_resize_critical(bitarr, 1031);
 * @endcode
 */
void bit_array_resize_critical(BIT_ARRAY* bitarr, bit_index_t num_of_bits);

/**
 * @brief Ensure the bit array is at least a certain size, exiting on failure.
 *
 * Similar to `bit_array_ensure_size()`, but exits the program with an error message
 * if memory allocation fails.
 *
 * @param bitarr The bit array to ensure size.
 * @param num_of_bits The minimum number of bits required.
 *
 * @example
 * @code
 * bit_array_ensure_size_critical(bitarr, 1031);
 * @endcode
 */
void bit_array_ensure_size_critical(BIT_ARRAY* bitarr, bit_index_t num_of_bits);


/**
 * @def bit_array_get(arr, i)
 * @brief Get the value of a bit at index `i`.
 *
 * Macro for fast access without bounds checking.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @param i   Bit index.
 * @return The value of the bit at index `i` (0 or 1).
 *
 * @warning No bounds checking is performed. Use with caution.
 *
 * @example
 * @code
 * char bit = bit_array_get(bitarr, 10);
 * @endcode
 */
#define bit_array_get(arr, i) bitset_get((arr)->words, i)

/**
 * @def bit_array_set(arr, i)
 * @brief Set the bit at index `i` to 1.
 *
 * Macro for fast access without bounds checking.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @param i   Bit index.
 *
 * @warning No bounds checking is performed. Use with caution.
 *
 * @example
 * @code
 * bit_array_set(bitarr, 15);
 * @endcode
 */
#define bit_array_set(arr, i) bitset_set((arr)->words, i)

/**
 * @def bit_array_clear(arr, i)
 * @brief Clear the bit at index `i` (set to 0).
 *
 * Macro for fast access without bounds checking.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @param i   Bit index.
 *
 * @warning No bounds checking is performed. Use with caution.
 *
 * @example
 * @code
 * bit_array_clear(bitarr, 20);
 * @endcode
 */
#define bit_array_clear(arr, i) bitset_del((arr)->words, i)

/**
 * @def bit_array_toggle(arr, i)
 * @brief Toggle the bit at index `i`.
 *
 * Macro for fast access without bounds checking.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @param i   Bit index.
 *
 * @warning No bounds checking is performed. Use with caution.
 *
 * @example
 * @code
 * bit_array_toggle(bitarr, 25);
 * @endcode
 */
#define bit_array_toggle(arr, i) bitset_tgl((arr)->words, i)

/**
 * @def bit_array_assign(arr, i, c)
 * @brief Assign a value to the bit at index `i`.
 *
 * Macro for fast access without bounds checking.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @param i   Bit index.
 * @param c   Value to assign (must be 0 or 1).
 *
 * @warning No bounds checking is performed. Use with caution.
 *
 * @example
 * @code
 * bit_array_assign(bitarr, 30, 1);
 * @endcode
 */
#define bit_array_assign(arr, i, c) bitset_cpy((arr)->words, i, c)

/**
 * @def bit_array_len(arr)
 * @brief Get the length of the bit array.
 *
 * Macro for fast access to the length of the bit array.
 *
 * @param arr Pointer to `BIT_ARRAY`.
 * @return The number of bits in the bit array.
 *
 * @example
 * @code
 * bit_index_t len = bit_array_len(bitarr);
 * @endcode
 */
#define bit_array_len(arr) ((arr)->num_of_bits)

/**
 * @brief Get the value of a bit at a specified index.
 *
 * Safely retrieves the value of the bit at index `b`.
 * Performs bounds checking using `assert()`.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to retrieve.
 * @return The value of the bit at index `b` (0 or 1).
 *
 * @example
 * @code
 * char bit = bit_array_get_bit(bitarr, 50);
 * @endcode
 */
char bit_array_get_bit(const BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Set the bit at a specified index to 1.
 *
 * Safely sets the bit at index `b` to 1.
 * Performs bounds checking using `assert()`.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to set.
 *
 * @example
 * @code
 * bit_array_set_bit(bitarr, 55);
 * @endcode
 */
void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Clear the bit at a specified index (set to 0).
 *
 * Safely clears the bit at index `b`.
 * Performs bounds checking using `assert()`.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to clear.
 *
 * @example
 * @code
 * bit_array_clear_bit(bitarr, 60);
 * @endcode
 */
void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Toggle the bit at a specified index.
 *
 * Safely toggles the bit at index `b`.
 * Performs bounds checking using `assert()`.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to toggle.
 *
 * @example
 * @code
 * bit_array_toggle_bit(bitarr, 65);
 * @endcode
 */
void bit_array_toggle_bit(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Assign a value to the bit at a specified index.
 *
 * Safely sets or clears the bit at index `b` based on the value of `c`.
 * Performs bounds checking using `assert()`.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to assign.
 * @param c      If `c != 0`, the bit is set; otherwise, it is cleared.
 *
 * @example
 * @code
 * bit_array_assign_bit(bitarr, 70, 0);
 * @endcode
 */
void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c);

/**
 * @brief Get the value of a bit, resizing the array if necessary.
 *
 * Retrieves the value of the bit at index `b`. If `b` is beyond the current length
 * of the bit array, the array is resized to accommodate the index, and the new
 * bits are initialized to 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to retrieve.
 * @return The value of the bit at index `b` (0 or 1).
 *
 * @example
 * @code
 * char bit = bit_array_rget(bitarr, 200);
 * @endcode
 */
char bit_array_rget(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Set the bit at a specified index, resizing the array if necessary.
 *
 * Sets the bit at index `b` to 1. If `b` is beyond the current length
 * of the bit array, the array is resized to accommodate the index, and the new
 * bits are initialized to 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to set.
 *
 * @example
 * @code
 * bit_array_rset(bitarr, 250);
 * @endcode
 */
void bit_array_rset(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Clear the bit at a specified index, resizing the array if necessary.
 *
 * Clears the bit at index `b`. If `b` is beyond the current length
 * of the bit array, the array is resized to accommodate the index, and the new
 * bits are initialized to 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to clear.
 *
 * @example
 * @code
 * bit_array_rclear(bitarr, 300);
 * @endcode
 */
void bit_array_rclear(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Toggle the bit at a specified index, resizing the array if necessary.
 *
 * Toggles the bit at index `b`. If `b` is beyond the current length
 * of the bit array, the array is resized to accommodate the index, and the new
 * bits are initialized to 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to toggle.
 *
 * @example
 * @code
 * bit_array_rtoggle(bitarr, 350);
 * @endcode
 */
void bit_array_rtoggle(BIT_ARRAY* bitarr, bit_index_t b);

/**
 * @brief Assign a value to the bit at a specified index, resizing the array if necessary.
 *
 * Assigns the bit at index `b` to the value of `c`. If `b` is beyond the current length
 * of the bit array, the array is resized to accommodate the index, and the new
 * bits are initialized to 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param b      Index of the bit to assign.
 * @param c      If `c != 0`, the bit is set; otherwise, it is cleared.
 *
 * @example
 * @code
 * bit_array_rassign(bitarr, 400, 1);
 * @endcode
 */
void bit_array_rassign(BIT_ARRAY* bitarr, bit_index_t b, char c);

/**
 * @brief Get the offsets of the set bits in a range.
 *
 * Scans the bit array from index `start` to `end - 1` and stores the indices
 * of the bits that are set (value 1) into the array `dst`.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index (inclusive).
 * @param end    Ending index (exclusive).
 * @param dst    Pointer to an array where the indices of set bits will be stored.
 *               It must be able to hold at least `(end - start)` elements.
 * @return The number of bits set in the specified range.
 *
 * @example
 * @code
 * bit_index_t indices[100];
 * bit_index_t count = bit_array_get_bits(bitarr, 0, 100, indices);
 * // Now, indices[0..count-1] contain the indices of set bits between 0 and 99
 * @endcode
 */
bit_index_t bit_array_get_bits(const BIT_ARRAY* bitarr, bit_index_t start, bit_index_t end, bit_index_t* dst);

/**
 * @brief Set multiple bits at once.
 *
 * Sets the bits at the specified indices to 1.
 *
 * @param bitarr Pointer to the bit array.
 * @param n      The number of bits to set.
 * @param ...    The indices of the bits to set (of type `unsigned int`).
 *
 * @example
 * @code
 * // Set bits at indices 1, 20, and 31
 * bit_array_set_bits(bitarr, 3, 1, 20, 31);
 * @endcode
 */
void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...);

/**
 * @brief Clear multiple bits at once.
 *
 * Clears the bits at the specified indices (sets to 0).
 *
 * @param bitarr Pointer to the bit array.
 * @param n      The number of bits to clear.
 * @param ...    The indices of the bits to clear (of type `unsigned int`).
 *
 * @example
 * @code
 * // Clear bits at indices 1, 20, and 31
 * bit_array_clear_bits(bitarr, 3, 1, 20, 31);
 * @endcode
 */
void bit_array_clear_bits(BIT_ARRAY* bitarr, size_t n, ...);

/**
 * @brief Toggle multiple bits at once.
 *
 * Toggles the bits at the specified indices.
 *
 * @param bitarr Pointer to the bit array.
 * @param n      The number of bits to toggle.
 * @param ...    The indices of the bits to toggle (of type `unsigned int`).
 *
 * @example
 * @code
 * // Toggle bits at indices 1, 20, and 31
 * bit_array_toggle_bits(bitarr, 3, 1, 20, 31);
 * @endcode
 */
void bit_array_toggle_bits(BIT_ARRAY* bitarr, size_t n, ...);

/**
 * @brief Set all bits in a specified region to 1.
 *
 * Sets all bits from index `start` to `start + len - 1` to 1.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index of the region.
 * @param len    Length of the region.
 *
 * @example
 * @code
 * // Set bits from index 10 to 19
 * bit_array_set_region(bitarr, 10, 10);
 * @endcode
 */
void bit_array_set_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len);

/**
 * @brief Clear all bits in a specified region.
 *
 * Clears all bits from index `start` to `start + len - 1`.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index of the region.
 * @param len    Length of the region.
 *
 * @example
 * @code
 * // Clear bits from index 20 to 29
 * bit_array_clear_region(bitarr, 20, 10);
 * @endcode
 */
void bit_array_clear_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len);

/**
 * @brief Toggle all bits in a specified region.
 *
 * Toggles all bits from index `start` to `start + len - 1`.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index of the region.
 * @param len    Length of the region.
 *
 * @example
 * @code
 * // Toggle bits from index 30 to 39
 * bit_array_toggle_region(bitarr, 30, 10);
 * @endcode
 */
void bit_array_toggle_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len);

/**
 * @brief Set all bits in the bit array to 1.
 *
 * @param bitarr Pointer to the bit array.
 *
 * @example
 * @code
 * bit_array_set_all(bitarr);
 * @endcode
 */
void bit_array_set_all(BIT_ARRAY* bitarr);

/**
 * @brief Clear all bits in the bit array (set to 0).
 *
 * @param bitarr Pointer to the bit array.
 *
 * @example
 * @code
 * bit_array_clear_all(bitarr);
 * @endcode
 */
void bit_array_clear_all(BIT_ARRAY* bitarr);

/**
 * @brief Toggle all bits in the bit array.
 *
 * @param bitarr Pointer to the bit array.
 *
 * @example
 * @code
 * bit_array_toggle_all(bitarr);
 * @endcode
 */
void bit_array_toggle_all(BIT_ARRAY* bitarr);

/**
 * @brief Get a 64-bit word from the bit array starting at a specified index.
 *
 * Retrieves 64 bits from the bit array, starting at index `start`.
 * If there are not enough bits, the missing bits are considered as 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index from which to retrieve the word.
 * @return The 64-bit word.
 *
 * @note The first bit is in the least significant bit position.
 *
 * @example
 * @code
 * uint64_t word = bit_array_get_word64(bitarr, 100);
 * @endcode
 */
uint64_t bit_array_get_word64(const BIT_ARRAY* bitarr, bit_index_t start);

/**
 * @brief Get a 32-bit word from the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index from which to retrieve the word.
 * @return The 32-bit word.
 *
 * @note The first bit is in the least significant bit position.
 *
 * @example
 * @code
 * uint32_t word = bit_array_get_word32(bitarr, 200);
 * @endcode
 */
uint32_t bit_array_get_word32(const BIT_ARRAY* bitarr, bit_index_t start);

/**
 * @brief Get a 16-bit word from the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index from which to retrieve the word.
 * @return The 16-bit word.
 *
 * @note The first bit is in the least significant bit position.
 *
 * @example
 * @code
 * uint16_t word = bit_array_get_word16(bitarr, 300);
 * @endcode
 */
uint16_t bit_array_get_word16(const BIT_ARRAY* bitarr, bit_index_t start);

/**
 * @brief Get an 8-bit word from the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index from which to retrieve the word.
 * @return The 8-bit byte.
 *
 * @note The first bit is in the least significant bit position.
 *
 * @example
 * @code
 * uint8_t byte = bit_array_get_word8(bitarr, 400);
 * @endcode
 */
uint8_t bit_array_get_word8(const BIT_ARRAY* bitarr, bit_index_t start);

/**
 * @brief Get an n-bit word from the bit array starting at a specified index.
 *
 * Retrieves `n` bits from the bit array, starting at index `start`.
 * If there are not enough bits, the missing bits are considered as 0.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index from which to retrieve the word.
 * @param n      Number of bits to retrieve (1 <= `n` <= 64).
 * @return The `n`-bit word (stored in a `uint64_t`).
 *
 * @note The first bit is in the least significant bit position.
 *
 * @example
 * @code
 * uint64_t word = bit_array_get_wordn(bitarr, 500, 40);
 * @endcode
 */
uint64_t bit_array_get_wordn(const BIT_ARRAY* bitarr, bit_index_t start, int n);

/**
 * @brief Set 64 bits in the bit array starting at a specified index.
 *
 * Sets 64 bits in the bit array, starting at index `start`, to the bits in `word`.
 * If setting beyond the end of the bit array, bits will not be set, but no error occurs.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index at which to set the word.
 * @param word   The 64-bit word to set.
 *
 * @note Does not extend the bit array. It is safe to try to set bits beyond the end
 *       of the array as long as `start` is less than `bit_array_length(bitarr)`.
 *
 * @example
 * @code
 * bit_array_set_word64(bitarr, 100, 0xFFFFFFFFFFFFFFFFULL);
 * @endcode
 */
void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word);

/**
 * @brief Set 32 bits in the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index at which to set the word.
 * @param word   The 32-bit word to set.
 *
 * @note Does not extend the bit array.
 *
 * @example
 * @code
 * bit_array_set_word32(bitarr, 200, 0xFFFFFFFFU);
 * @endcode
 */
void bit_array_set_word32(BIT_ARRAY* bitarr, bit_index_t start, uint32_t word);

/**
 * @brief Set 16 bits in the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index at which to set the word.
 * @param word   The 16-bit word to set.
 *
 * @note Does not extend the bit array.
 *
 * @example
 * @code
 * bit_array_set_word16(bitarr, 300, 0xFFFFU);
 * @endcode
 */
void bit_array_set_word16(BIT_ARRAY* bitarr, bit_index_t start, uint16_t word);

/**
 * @brief Set 8 bits in the bit array starting at a specified index.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index at which to set the byte.
 * @param byte   The 8-bit byte to set.
 *
 * @note Does not extend the bit array.
 *
 * @example
 * @code
 * bit_array_set_word8(bitarr, 400, 0xFFU);
 * @endcode
 */
void bit_array_set_word8(BIT_ARRAY* bitarr, bit_index_t start, uint8_t byte);

/**
 * @brief Set `n` bits in the bit array starting at a specified index.
 *
 * Sets `n` bits in the bit array, starting at index `start`, to the bits in `word`.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index at which to set the word.
 * @param word   The word containing the bits to set.
 * @param n      Number of bits to set (1 <= `n` <= 64).
 *
 * @note Does not extend the bit array.
 *
 * @example
 * @code
 * bit_array_set_wordn(bitarr, 500, 0x12345ULL, 20);
 * @endcode
 */
void bit_array_set_wordn(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word, int n);

/**
 * @brief Get the number of bits set in the bit array.
 *
 * Computes the Hamming weight of the bit array (the number of bits set to 1).
 *
 * @param bitarr Pointer to the bit array.
 * @return The number of bits set to 1.
 *
 * @example
 * @code
 * bit_index_t count = bit_array_num_bits_set(bitarr);
 * @endcode
 */
bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr);

/**
 * @brief Get the number of bits cleared in the bit array.
 *
 * Computes the number of bits set to 0 (length of the array minus the number of bits set).
 *
 * @param bitarr Pointer to the bit array.
 * @return The number of bits set to 0.
 *
 * @example
 * @code
 * bit_index_t zeros = bit_array_num_bits_cleared(bitarr);
 * @endcode
 */
bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr);

/**
 * @brief Compute the Hamming distance between two bit arrays.
 *
 * Returns the number of bits that are set in one array and not the other.
 * This is equivalent to the Hamming weight of the XOR of the two arrays when they
 * are the same length.
 *
 * @param arr1 Pointer to the first bit array.
 * @param arr2 Pointer to the second bit array.
 * @return The Hamming distance between the two arrays.
 *
 * @example
 * @code
 * bit_index_t distance = bit_array_hamming_distance(arr1, arr2);
 * @endcode
 */
bit_index_t bit_array_hamming_distance(const BIT_ARRAY* arr1, const BIT_ARRAY* arr2);

/**
 * @brief Compute the parity of the bit array.
 *
 * Returns `1` if the number of bits set is odd, `0` if even.
 *
 * @param bitarr Pointer to the bit array.
 * @return `1` if the number of bits set is odd, `0` if even.
 *
 * @example
 * @code
 * char parity = bit_array_parity(bitarr);
 * @endcode
 */
char bit_array_parity(const BIT_ARRAY* bitarr);

/**
 * @brief Find the index of the next bit that is set, at or after a given offset.
 *
 * Searches the bit array for the next bit set to `1`, starting from index `offset`.
 * If a set bit is found, stores the index in `result` and returns `1`.
 * If no set bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr Pointer to the bit array.
 * @param offset Starting index for the search.
 * @param result Pointer to where the index of the next set bit will be stored.
 * @return `1` if a set bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_next_set_bit(bitarr, 50, &index)) {
 *     // Set bit found at index
 * } else {
 *     // No set bit found
 * }
 * @endcode
 */
char bit_array_find_next_set_bit(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result);

/**
* @brief Find the index of the next bit that is NOT set, at or after a given offset.
*
* Searches the bit array for the next bit that is clear (set to `0`), starting from index `offset`.
* If a clear bit is found, stores the index in `result` and returns `1`.
* If no clear bit is found, returns `0` and `result` is not modified.
*
* @param bitarr  Pointer to the bit array.
* @param offset  Starting index for the search.
* @param result  Pointer to where the index of the next clear bit will be stored.
* @return `1` if a clear bit is found, `0` otherwise.
*
* @example
* @code
* bit_index_t index;
* if (bit_array_find_next_clear_bit(bitarr, 50, &index)) {
*     // Clear bit found at index
* } else {
*     // No clear bit found
* }
* @endcode
*/
char bit_array_find_next_clear_bit(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result);

/**
 * @brief Find the index of the previous bit that is set, before a given offset.
 *
 * Searches the bit array for the previous bit that is set (set to `1`), starting from index `offset - 1`.
 * If a set bit is found, stores the index in `result` and returns `1`.
 * If no set bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param offset  Starting index for the search (exclusive).
 * @param result  Pointer to where the index of the previous set bit will be stored.
 * @return `1` if a set bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_prev_set_bit(bitarr, 100, &index)) {
 *     // Previous set bit found at index
 * } else {
 *     // No previous set bit found
 * }
 * @endcode
 */
char bit_array_find_prev_set_bit(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result);

/**
 * @brief Find the index of the previous bit that is NOT set, before a given offset.
 *
 * Searches the bit array for the previous bit that is clear (set to `0`), starting from index `offset - 1`.
 * If a clear bit is found, stores the index in `result` and returns `1`.
 * If no clear bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param offset  Starting index for the search (exclusive).
 * @param result  Pointer to where the index of the previous clear bit will be stored.
 * @return `1` if a clear bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_prev_clear_bit(bitarr, 100, &index)) {
 *     // Previous clear bit found at index
 * } else {
 *     // No previous clear bit found
 * }
 * @endcode
 */
char bit_array_find_prev_clear_bit(const BIT_ARRAY* bitarr, bit_index_t offset, bit_index_t* result);

/**
 * @brief Find the index of the first bit that is set.
 *
 * Searches the bit array for the first bit that is set (set to `1`).
 * If a set bit is found, stores the index in `result` and returns `1`.
 * If no set bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param result  Pointer to where the index of the first set bit will be stored.
 * @return `1` if a set bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_first_set_bit(bitarr, &index)) {
 *     // First set bit found at index
 * } else {
 *     // No set bit found
 * }
 * @endcode
 */
char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result);

/**
 * @brief Find the index of the first bit that is NOT set.
 *
 * Searches the bit array for the first bit that is clear (set to `0`).
 * If a clear bit is found, stores the index in `result` and returns `1`.
 * If no clear bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param result  Pointer to where the index of the first clear bit will be stored.
 * @return `1` if a clear bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_first_clear_bit(bitarr, &index)) {
 *     // First clear bit found at index
 * } else {
 *     // No clear bit found
 * }
 * @endcode
 */
char bit_array_find_first_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result);

/**
 * @brief Find the index of the last bit that is set.
 *
 * Searches the bit array for the last bit that is set (set to `1`).
 * If a set bit is found, stores the index in `result` and returns `1`.
 * If no set bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param result  Pointer to where the index of the last set bit will be stored.
 * @return `1` if a set bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_last_set_bit(bitarr, &index)) {
 *     // Last set bit found at index
 * } else {
 *     // No set bit found
 * }
 * @endcode
 */
char bit_array_find_last_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result);

/**
 * @brief Find the index of the last bit that is NOT set.
 *
 * Searches the bit array for the last bit that is clear (set to `0`).
 * If a clear bit is found, stores the index in `result` and returns `1`.
 * If no clear bit is found, returns `0` and `result` is not modified.
 *
 * @param bitarr  Pointer to the bit array.
 * @param result  Pointer to where the index of the last clear bit will be stored.
 * @return `1` if a clear bit is found, `0` otherwise.
 *
 * @example
 * @code
 * bit_index_t index;
 * if (bit_array_find_last_clear_bit(bitarr, &index)) {
 *     // Last clear bit found at index
 * } else {
 *     // No clear bit found
 * }
 * @endcode
 */
char bit_array_find_last_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result);

/**
 * @brief Sort the bits in the array with all `0`s before all `1`s.
 *
 * This function rearranges the bits in the bit array so that all bits set to `0` appear
 * before all bits set to `1`. The relative order of `0`s and `1`s is not preserved.
 *
 * @param bitarr Pointer to the bit array to sort.
 *
 * @example
 * @code
 * bit_array_sort_bits(bitarr);
 * @endcode
 */
void bit_array_sort_bits(BIT_ARRAY* bitarr);

/**
 * @brief Sort the bits in the array with all `1`s before all `0`s.
 *
 * This function rearranges the bits in the bit array so that all bits set to `1` appear
 * before all bits set to `0`. The relative order of `1`s and `0`s is not preserved.
 *
 * @param bitarr Pointer to the bit array to sort.
 *
 * @example
 * @code
 * bit_array_sort_bits_rev(bitarr);
 * @endcode
 */
void bit_array_sort_bits_rev(BIT_ARRAY* bitarr);

/**
     * @brief Construct a `BIT_ARRAY` from a string.
     *
     * Initializes the bit array based on a string representation where each character
     * corresponds to a bit. Typically, '1' represents a set bit and '0' represents a clear bit.
     *
     * @param bitarr Pointer to the bit array to initialize.
     * @param bitstr C-string representing the bits (e.g., "1101").
     *
     * @example
     * @code
     * bit_array_from_str(bitarr, "101010");
     * @endcode
     */
void bit_array_from_str(BIT_ARRAY* bitarr, const char* bitstr);

/**
 * @brief Construct a `BIT_ARRAY` from a substring with specified characters for set and clear bits.
 *
 * Initializes a portion of the bit array based on a substring and custom characters representing
 * set and clear bits. Allows specifying the direction (left-to-right or right-to-left).
 *
 * @param bitarr        Pointer to the bit array to initialize.
 * @param offset        Starting index in the bit array to begin setting bits.
 * @param str           Substring representing the bits.
 * @param len           Length of the substring.
 * @param on            Character representing a set bit (e.g., '1').
 * @param off           Character representing a clear bit (e.g., '0').
 * @param left_to_right Direction of bit assignment. If non-zero, bits are assigned left-to-right.
 *                      If zero, bits are assigned right-to-left.
 *
 * @example
 * @code
 * bit_array_from_substr(bitarr, 10, "AABB", 4, 'A', 'B', 1);
 * // Sets bits based on 'A' as set and 'B' as clear, left-to-right
 * @endcode
 */
void bit_array_from_substr(BIT_ARRAY* bitarr, bit_index_t offset, const char* str, size_t len, const char *on, const char *off, char left_to_right);

/**
 * @brief Convert the entire bit array to a string.
 *
 * Converts the bit array into a null-terminated string of '0's and '1's.
 * The `str` buffer must be large enough to hold `bitarr->num_of_bits + 1` characters.
 * The string is terminated with a null character (`'\0'`).
 *
 * @param bitarr Pointer to the bit array to convert.
 * @param str    Buffer to store the resulting string.
 * @return Pointer to the resulting string (`str`).
 *
 * @note The first bit is placed at the beginning of the string.
 *
 * @example
 * @code
 * char str[65];
 * bit_array_to_str(bitarr, str);
 * printf("Bit array: %s\n", str);
 * @endcode
 */
char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str);

/**
 * @brief Convert the entire bit array to a reversed string.
 *
 * Converts the bit array into a null-terminated string of '0's and '1's in reverse order.
 * The `str` buffer must be large enough to hold `bitarr->num_of_bits + 1` characters.
 * The string is terminated with a null character (`'\0'`).
 *
 * @param bitarr Pointer to the bit array to convert.
 * @param str    Buffer to store the resulting reversed string.
 * @return Pointer to the resulting string (`str`).
 *
 * @note The last bit is placed at the beginning of the string.
 *
 * @example
 * @code
 * char str[65];
 * bit_array_to_str_rev(bitarr, str);
 * printf("Reversed Bit array: %s\n", str);
 * @endcode
 */
char* bit_array_to_str_rev(const BIT_ARRAY* bitarr, char* str);

/**
 * @brief Get a substring representation of a region of the bit array.
 *
 * Generates a string representation for a specific region of the bit array using
 * specified characters for set and clear bits. The generated string is not null-terminated.
 *
 * @param bitarr        Pointer to the bit array.
 * @param start         Starting index of the region.
 * @param length        Length of the region.
 * @param str           Buffer to store the resulting substring.
 * @param on            Character representing a set bit.
 * @param off           Character representing a clear bit.
 * @param left_to_right Direction of bit assignment. If non-zero, bits are assigned left-to-right.
 *                      If zero, bits are assigned right-to-left.
 *
 * @example
 * @code
 * char substr[10];
 * bit_array_to_substr(bitarr, 50, 8, substr, 'X', '-', 1);
 * substr[8] = '\0'; // Manually null-terminate if needed
 * printf("Substring: %s\n", substr);
 * @endcode
 */
void bit_array_to_substr(const BIT_ARRAY* bitarr, bit_index_t start, bit_index_t length, char* str, char on, char off, char left_to_right);

/**
 * @brief Print the bit array to a file stream as '0's and '1's without a newline.
 *
 * Outputs the bit array to the specified file stream using characters '0' and '1'.
 * This function does not append a newline character at the end.
 *
 * @param bitarr Pointer to the bit array to print.
 * @param fout   File stream to which the bit array will be printed.
 *
 * @example
 * @code
 * FILE* file = fopen("output.txt", "w");
 * bit_array_print(bitarr, file);
 * fclose(file);
 * @endcode
 */
void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout);

/**
 * @brief Print the bit array to `stdout` with a newline at the end.
 *
 * Outputs the bit array to the standard output (`stdout`) using characters '0' and '1',
 * followed by a newline character.
 *
 * @param bitarr Pointer to the bit array to print.
 *
 * @example
 * @code
 * bit_array_print_stdout_newline(bitarr);
 * @endcode
 */
void bit_array_print_stdout_newline(const BIT_ARRAY* bitarr);

/**
 * @brief Print the bit array to `stdout` without a newline.
 *
 * Outputs the bit array to the standard output (`stdout`) using characters '0' and '1'.
 * This function does not append a newline character at the end.
 *
 * @param bitarr Pointer to the bit array to print.
 *
 * @example
 * @code
 * bit_array_print_stdout(bitarr);
 * @endcode
 */
void bit_array_print_stdout(const BIT_ARRAY* bitarr);

/**
 * @brief Print a substring representation of a region of the bit array to a file stream.
 *
 * Generates and prints a string representation for a specific region of the bit array
 * using specified characters for set and clear bits. The bits are printed in reverse order
 * (from highest to lowest index), which is useful for displaying binary numbers.
 *
 * @param bitarr        Pointer to the bit array.
 * @param start         Starting index of the region.
 * @param length        Length of the region.
 * @param fout          File stream to which the substring will be printed.
 * @param on            Character representing a set bit.
 * @param off           Character representing a clear bit.
 * @param left_to_right Direction of bit assignment. If non-zero, bits are assigned left-to-right.
 *                      If zero, bits are assigned right-to-left.
 *
 * @example
 * @code
 * bit_array_print_substr(bitarr, 60, 16, stdout, '1', '0', 0);
 * @endcode
 */
void bit_array_print_substr(const BIT_ARRAY* bitarr, bit_index_t start, bit_index_t length, FILE* fout, char on, char off, char left_to_right);

/**
 * @brief Convert the bit array to a decimal string.
 *
 * Converts the bit array into its decimal string representation (e.g., `0b1101` -> `"13"`).
 *
 * @param bitarr Pointer to the bit array to convert.
 * @param str    Buffer to store the resulting decimal string.
 * @param len    Length of the `str` buffer.
 * @return The number of characters written to `str`, excluding the null terminator.
 *
 * @example
 * @code
 * char decimal_str[21];
 * size_t chars = bit_array_to_decimal(bitarr, decimal_str, sizeof(decimal_str));
 * printf("Decimal: %s\n", decimal_str);
 * @endcode
 */
size_t bit_array_to_decimal(const BIT_ARRAY *bitarr, char *str, size_t len);

/**
 * @brief Populate the bit array from a decimal string.
 *
 * Converts a decimal string into its binary representation and populates the bit array.
 *
 * @param bitarr   Pointer to the bit array to populate.
 * @param decimal  C-string containing the decimal number (e.g., `"13"`).
 * @return The number of characters used in the conversion.
 *
 * @example
 * @code
 * bit_array_from_decimal(bitarr, "13");
 * @endcode
 */
size_t bit_array_from_decimal(BIT_ARRAY *bitarr, const char* decimal);

/**
 * @brief Load the bit array from a hexadecimal string.
 *
 * Initializes the bit array based on a hexadecimal string representation.
 * The number of bits loaded is the number of characters rounded up to a multiple of 8.
 * Returns `0` on failure.
 *
 * @param bitarr Pointer to the bit array to initialize.
 * @param offset Starting index in the bit array to begin setting bits.
 * @param str    C-string containing the hexadecimal number (e.g., `"1A3F"`).
 * @param len    Length of the hexadecimal string.
 * @return The number of bits loaded, or `0` on failure.
 *
 * @example
 * @code
 * bit_array_from_hex(bitarr, 0, "1A3F", 4);
 * @endcode
 */
bit_index_t bit_array_from_hex(BIT_ARRAY* bitarr, bit_index_t offset, const char* str, size_t len);

/**
 * @brief Convert a region of the bit array to a hexadecimal string.
 *
 * Converts a specified region of the bit array into its hexadecimal string representation.
 *
 * @param bitarr    Pointer to the bit array to convert.
 * @param start     Starting index of the region.
 * @param length    Length of the region in bits.
 * @param str       Buffer to store the resulting hexadecimal string.
 * @param uppercase If non-zero, hexadecimal letters are uppercase; otherwise, lowercase.
 * @return The number of characters written to `str`.
 *
 * @example
 * @code
 * char hex_str[17];
 * size_t chars = bit_array_to_hex(bitarr, 0, 64, hex_str, 1);
 * printf("Hexadecimal: %s\n", hex_str);
 * @endcode
 */
size_t bit_array_to_hex(const BIT_ARRAY* bitarr, bit_index_t start, bit_index_t length, char* str, char uppercase);

/**
 * @brief Print a region of the bit array as a hexadecimal string to a file stream.
 *
 * Converts a specified region of the bit array into its hexadecimal string representation and prints it to the given file stream.
 *
 * @param bitarr    Pointer to the bit array to convert.
 * @param start     Starting index of the region.
 * @param length    Length of the region in bits.
 * @param fout      File stream to which the hexadecimal string will be printed.
 * @param uppercase If non-zero, hexadecimal letters are uppercase; otherwise, lowercase.
 * @return The number of characters written to the file stream.
 *
 * @example
 * @code
 * bit_array_print_hex(bitarr, 0, 64, stdout, 1);
 * @endcode
 */
size_t bit_array_print_hex(const BIT_ARRAY* bitarr, bit_index_t start, bit_index_t length, FILE* fout, char uppercase);

/**
 * @brief Duplicate (clone) a `BIT_ARRAY`.
 *
 * Creates a copy of the given bit array, including all data it holds.
 *
 * @param bitarr Pointer to the bit array to clone.
 * @return Pointer to the newly cloned `BIT_ARRAY`, or `NULL` on failure.
 *
 * @note The cloned bit array should be freed with `bit_array_free()` when no longer needed.
 *
 * @example
 * @code
 * BIT_ARRAY* clone = bit_array_clone(original_bitarr);
 * if (clone != NULL) {
 *     // Use the cloned bit array...
 *     bit_array_free(clone);
 * }
 * @endcode
 */
#define bit_array_dup bit_array_clone
BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr);

/**
 * @brief Copy bits from one bit array to another.
 *
 * Copies a range of bits from the source bit array to the destination bit array.
 * The destination and source can be the same bit array, and regions can overlap.
 *
 * @param dst    Pointer to the destination bit array.
 * @param dstindx Starting index in the destination array where bits will be copied to.
 * @param src    Pointer to the source bit array.
 * @param srcindx Starting index in the source array from where bits will be copied.
 * @param length Number of bits to copy.
 *
 * @note Use the `bit_array_copy` macro for convenience.
 *
 * @example
 * @code
 * bit_array_copy(destination, 10, source, 20, 30);
 * @endcode
 */
void bit_array_copy(BIT_ARRAY* dst, bit_index_t dstindx, const BIT_ARRAY* src, bit_index_t srcindx, bit_index_t length);

/**
 * @brief Copy all bits from one bit array to another.
 *
 * Copies the entire content of the source bit array to the destination bit array.
 * The destination array is resized to match the source array's size if necessary.
 *
 * @param dst  Pointer to the destination bit array.
 * @param src  Pointer to the source bit array.
 *
 * @example
 * @code
 * bit_array_copy_all(destination, source);
 * @endcode
 */
void bit_array_copy_all(BIT_ARRAY* dst, const BIT_ARRAY* src);

/**
 * @brief Perform a bitwise AND operation between two bit arrays.
 *
 * Computes the bitwise AND of `src1` and `src2`, storing the result in `dest`.
 * If `dest` is shorter than the longer of `src1` or `src2`, it is resized accordingly.
 *
 * @param dest Pointer to the destination bit array where the result will be stored.
 * @param src1 Pointer to the first source bit array.
 * @param src2 Pointer to the second source bit array.
 *
 * @example
 * @code
 * bit_array_and(dest, src1, src2);
 * @endcode
 */
void bit_array_and(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Perform a bitwise OR operation between two bit arrays.
 *
 * Computes the bitwise OR of `src1` and `src2`, storing the result in `dest`.
 * If `dest` is shorter than the longer of `src1` or `src2`, it is resized accordingly.
 *
 * @param dest Pointer to the destination bit array where the result will be stored.
 * @param src1 Pointer to the first source bit array.
 * @param src2 Pointer to the second source bit array.
 *
 * @example
 * @code
 * bit_array_or(dest, src1, src2);
 * @endcode
 */
void bit_array_or (BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Perform a bitwise XOR operation between two bit arrays.
 *
 * Computes the bitwise XOR of `src1` and `src2`, storing the result in `dest`.
 * If `dest` is shorter than the longer of `src1` or `src2`, it is resized accordingly.
 *
 * @param dest Pointer to the destination bit array where the result will be stored.
 * @param src1 Pointer to the first source bit array.
 * @param src2 Pointer to the second source bit array.
 *
 * @example
 * @code
 * bit_array_xor(dest, src1, src2);
 * @endcode
 */
void bit_array_xor(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Perform a bitwise NOT operation on a bit array.
 *
 * Computes the bitwise NOT of `src`, storing the result in `dest`.
 *
 * @param dest Pointer to the destination bit array where the result will be stored.
 * @param src  Pointer to the source bit array.
 *
 * @example
 * @code
 * bit_array_not(dest, src);
 * @endcode
 */
void bit_array_not(BIT_ARRAY* dest, const BIT_ARRAY* src);

/**
 * @brief Shift the bit array to the right by a specified distance with a fill bit.
 *
 * Shifts all bits in the bit array to the right by `shift_dist` positions.
 * The vacated bits on the left are filled with `fill` (either `0` or `1`).
 *
 * @param bitarr     Pointer to the bit array to shift.
 * @param shift_dist Number of positions to shift.
 * @param fill       Bit value to fill the vacated positions (`0` or `1`).
 *
 * @example
 * @code
 * bit_array_shift_right(bitarr, 10, 0);
 * @endcode
 */
void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill);

/**
 * @brief Shift the bit array to the left by a specified distance with a fill bit.
 *
 * Shifts all bits in the bit array to the left by `shift_dist` positions.
 * The vacated bits on the right are filled with `fill` (either `0` or `1`).
 *
 * @param bitarr     Pointer to the bit array to shift.
 * @param shift_dist Number of positions to shift.
 * @param fill       Bit value to fill the vacated positions (`0` or `1`).
 *
 * @example
 * @code
 * bit_array_shift_left(bitarr, 5, 1);
 * @endcode
 */
void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill);

/**
 * @brief Shift the bit array to the left by a specified distance without losing any bits.
 *
 * Shifts all bits in the bit array to the left by `shift_dist` positions.
 * The bit array is resized to accommodate the shifted bits, ensuring that no bits are lost.
 * The vacated bits on the right are filled with `fill` (either `0` or `1`).
 *
 * @param bitarr     Pointer to the bit array to shift.
 * @param shift_dist Number of positions to shift.
 * @param fill       Bit value to fill the vacated positions (`0` or `1`).
 *
 * @example
 * @code
 * bit_array_shift_left_extend(bitarr, 8, 0);
 * @endcode
 */
void bit_array_shift_left_extend(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill);

/**
 * @brief Perform a cyclic right shift on the bit array.
 *
 * Rotates the bits in the bit array to the right by `dist` positions.
 * Bits shifted out on the right are wrapped around to the left.
 *
 * @param bitarr Pointer to the bit array to shift.
 * @param dist   Number of positions to cyclically shift.
 *
 * @example
 * @code
 * bit_array_cycle_right(bitarr, 3);
 * @endcode
 */
void bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t dist);

/**
 * @brief Perform a cyclic left shift on the bit array.
 *
 * Rotates the bits in the bit array to the left by `dist` positions.
 * Bits shifted out on the left are wrapped around to the right.
 *
 * @param bitarr Pointer to the bit array to shift.
 * @param dist   Number of positions to cyclically shift.
 *
 * @example
 * @code
 * bit_array_cycle_left(bitarr, 4);
 * @endcode
 */
void bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t dist);

/**
 * @brief Interleave two bit arrays into a destination bit array.
 *
 * Interleaves the bits from `src1` and `src2` into `dst`. Specifically, it alternates bits from `src1` and `src2`.
 * For example:
 * - `abcd` and `1234` become `a1b2c3d4`
 * - `0011` and `0000` become `00001010`
 *
 * If `dst` is too short, it is extended. If `dst` is longer than the combined length of `src1` and `src2`,
 * the remaining bits in `dst` are not altered.
 *
 * @param dst   Pointer to the destination bit array where the interleaved bits will be stored.
 * @param src1  Pointer to the first source bit array.
 * @param src2  Pointer to the second source bit array.
 *
 * @note `dst` must not be the same as `src1` or `src2`.
 *
 * @example
 * @code
 * bit_array_interleave(dst, src1, src2);
 * @endcode
 */
void bit_array_interleave(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Reverse the entire bit array.
 *
 * Reverses the order of all bits in the bit array. The first bit becomes the last, the second becomes the second last, and so on.
 *
 * @param bitarr Pointer to the bit array to reverse.
 *
 * @example
 * @code
 * bit_array_reverse(bitarr);
 * @endcode
 */
void bit_array_reverse(BIT_ARRAY* bitarr);

/**
 * @brief Reverse a specific region of the bit array.
 *
 * Reverses the order of bits in a specified region of the bit array.
 * Bits outside the specified region remain unchanged.
 *
 * @param bitarr Pointer to the bit array.
 * @param start  Starting index of the region to reverse.
 * @param len    Length of the region to reverse.
 *
 * @example
 * @code
 * bit_array_reverse_region(bitarr, 50, 10);
 * @endcode
 */
void bit_array_reverse_region(BIT_ARRAY* bitarr, bit_index_t start, bit_index_t len);

/**
 * @brief Convert the bit array to a numerical value.
 *
 * Converts the bit array into a numerical value stored in a `uint64_t`.
 * Returns `1` on success if the bit array can be represented within 64 bits,
 * otherwise returns `0` if the bit array is too large.
 *
 * @param bitarr Pointer to the bit array to convert.
 * @param result Pointer to a `uint64_t` where the numerical result will be stored.
 * @return `1` on successful conversion, `0` if the bit array is too large.
 *
 * @example
 * @code
 * uint64_t number;
 * if (bit_array_as_num(bitarr, &number)) {
 *     printf("Number: %llu\n", number);
 * } else {
 *     // Handle error: bit array too large
 * }
 * @endcode
 */
char bit_array_as_num(const BIT_ARRAY* bitarr, uint64_t* result);

/**
 * @brief Compare a bit array with a `uint64_t` value.
 *
 * Compares the bit array with a `uint64_t` value.
 *
 * @param bitarr Pointer to the bit array.
 * @param value  `uint64_t` value to compare against.
 * @return
 *  - `1` if `bitarr` > `value`
 *  - `0` if `bitarr` == `value`
 *  - `-1` if `bitarr` < `value`
 *
 * @example
 * @code
 * int cmp = bit_array_cmp_uint64(bitarr, 1000);
 * if (cmp > 0) {
 *     // bitarr is greater than 1000
 * } else if (cmp < 0) {
 *     // bitarr is less than 1000
 * } else {
 *     // bitarr is equal to 1000
 * }
 * @endcode
 */
int bit_array_cmp_uint64(const BIT_ARRAY* bitarr, uint64_t value);

/**
 * @brief Add a `uint64_t` value to the bit array.
 *
 * Adds the `value` to the bit array. The bit array will be extended if needed
 * to accommodate the addition.
 *
 * @param bitarr   Pointer to the bit array.
 * @param value    `uint64_t` value to add.
 *
 * @example
 * @code
 * bit_array_add_uint64(bitarr, 500);
 * @endcode
 */
void bit_array_add_uint64(BIT_ARRAY* bitarr, uint64_t value);

/**
 * @brief Add a `uint64_t` value to the bit array at a specified position.
 *
 * Adds the `add` value to the bit array at the specified bit position `pos`.
 * This operation is equivalent to `bitarr + (add << pos)`.
 * The bit array will be resized if `pos` is beyond the current length.
 *
 * @param bitarr Pointer to the bit array.
 * @param pos    Bit position at which to add the value.
 * @param add    `uint64_t` value to add.
 *
 * @example
 * @code
 * bit_array_add_word(bitarr, 10, 3);
 * @endcode
 */
void bit_array_add_word(BIT_ARRAY *bitarr, bit_index_t pos, uint64_t add);

/**
 * @brief Add a `BIT_ARRAY` value to the bit array at a specified position.
 *
 * Adds the `add` bit array to the destination bit array at the specified bit position `pos`.
 * The bit array will be resized if needed to accommodate the addition.
 *
 * @param bitarr Pointer to the bit array.
 * @param pos    Bit position at which to add the bit array.
 * @param add    Pointer to the bit array to add.
 *
 * @example
 * @code
 * bit_array_add_words(bitarr, 20, add_bitarr);
 * @endcode
 */
void bit_array_add_words(BIT_ARRAY *bitarr, bit_index_t pos, const BIT_ARRAY *add);

/**
 * @brief Subtract a `uint64_t` value from the bit array.
 *
 * Subtracts the `value` from the bit array. If the `value` is greater than the bit array's current value,
 * the bit array remains unchanged and the function returns `0`. Otherwise, the subtraction is performed
 * and the function returns `1`.
 *
 * @param bitarr Pointer to the bit array.
 * @param value  `uint64_t` value to subtract.
 * @return `1` on successful subtraction, `0` if `value` is greater than the bit array.
 *
 * @example
 * @code
 * if (!bit_array_sub_uint64(bitarr, 250)) {
 *     // Handle error: value > bit array
 * }
 * @endcode
 */
char bit_array_sub_uint64(BIT_ARRAY* bitarr, uint64_t value);

/**
 * @brief Subtract a `word_t` value from the bit array at a specified position.
 *
 * Subtracts the `minus` value from the bit array at the specified bit position `pos`.
 * This operation is equivalent to `bitarr - (minus << pos)`.
 * Returns `1` on successful subtraction, or `0` if `value > bitarr`.
 *
 * @param bitarr Pointer to the bit array.
 * @param pos    Bit position at which to subtract the value.
 * @param minus  `word_t` value to subtract.
 * @return `1` on success, `0` if `value > bitarr`.
 *
 * @example
 * @code
 * if (!bit_array_sub_word(bitarr, 15, 5)) {
 *     // Handle error: value > bit array
 * }
 * @endcode
 */
char bit_array_sub_word(BIT_ARRAY *bitarr, bit_index_t pos, word_t minus);

/**
 * @brief Subtract a `BIT_ARRAY` value from the bit array at a specified position.
 *
 * Subtracts the `minus` bit array from the destination bit array at the specified bit position `pos`.
 *
 * @param bitarr Pointer to the bit array.
 * @param pos    Bit position at which to subtract the bit array.
 * @param minus  Pointer to the bit array to subtract.
 * @return `1` on successful subtraction, `0` if `value > bitarr`.
 *
 * @example
 * @code
 * if (!bit_array_sub_words(bitarr, 25, minus_bitarr)) {
 *     // Handle error: value > bit array
 * }
 * @endcode
 */
char bit_array_sub_words(BIT_ARRAY* bitarr, bit_index_t pos, BIT_ARRAY* minus);

/**
 * @brief Multiply the bit array by a `uint64_t` multiplier.
 *
 * Multiplies the bit array's numerical value by the given `multiplier`.
 * The bit array is extended if necessary to accommodate the result.
 *
 * @param bitarr     Pointer to the bit array to multiply.
 * @param multiplier `uint64_t` value to multiply by.
 *
 * @example
 * @code
 * bit_array_mul_uint64(bitarr, 10);
 * @endcode
 */
void bit_array_mul_uint64(BIT_ARRAY *bitarr, uint64_t multiplier);

/**
 * @brief Divide the bit array by a `uint64_t` divisor.
 *
 * Divides the bit array's numerical value by the given `divisor`, rounding down.
 * Stores the quotient in the bit array and the remainder in `rem`.
 *
 * @param bitarr Pointer to the bit array to divide (will hold the quotient).
 * @param divisor `uint64_t` value to divide by.
 * @param rem     Pointer to a `uint64_t` where the remainder will be stored.
 *
 * @example
 * @code
 * uint64_t remainder;
 * bit_array_div_uint64(bitarr, 3, &remainder);
 * printf("Quotient: ");
 * bit_array_print_stdout(bitarr);
 * printf("\nRemainder: %llu\n", remainder);
 * @endcode
 */
void bit_array_div_uint64(BIT_ARRAY *bitarr, uint64_t divisor, uint64_t *rem);

/**
 * @brief Add two bit arrays and store the result in a destination bit array.
 *
 * Computes the sum of `src1` and `src2`, storing the result in `dst`.
 * If `dst` is shorter than either `src1` or `src2`, it is resized accordingly.
 *
 * @param dst  Pointer to the destination bit array where the result will be stored.
 * @param src1 Pointer to the first source bit array.
 * @param src2 Pointer to the second source bit array.
 *
 * @example
 * @code
 * bit_array_add(dest, src1, src2);
 * @endcode
 */
void bit_array_add(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Subtract one bit array from another and store the result in a destination bit array.
 *
 * Computes the difference of `src1` minus `src2`, storing the result in `dst`.
 * If `dst` is shorter than `src1`, it is resized to match `src1`.
 * It is required that `src1` is greater than or equal to `src2` (`src1 >= src2`).
 *
 * @param dst  Pointer to the destination bit array where the result will be stored.
 * @param src1 Pointer to the first source bit array (minuend).
 * @param src2 Pointer to the second source bit array (subtrahend).
 *
 * @example
 * @code
 * bit_array_subtract(dest, src1, src2);
 * @endcode
 */
void bit_array_subtract(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2);

/**
 * @brief Multiply two bit arrays and store the result in a destination bit array.
 *
 * Computes the product of `src1` and `src2`, storing the result in `dst`.
 * The destination bit array must not be the same as `src1` or `src2`.
 *
 * @param dst   Pointer to the destination bit array where the result will be stored.
 * @param src1  Pointer to the first source bit array.
 * @param src2  Pointer to the second source bit array.
 *
 * @note `dst` cannot point to the same bit array as `src1` or `src2`.
 *
 * @example
 * @code
 * bit_array_multiply(dest, src1, src2);
 * @endcode
 */
void bit_array_multiply(BIT_ARRAY *dst, BIT_ARRAY *src1, BIT_ARRAY *src2);

/**
 * @brief Divide one bit array by another and store the quotient and remainder.
 *
 * Divides `dividend` by `divisor`, storing the quotient in `quotient` and the remainder in `dividend`.
 * After the operation, `dividend` holds the remainder, and `quotient` holds the quotient.
 *
 * @param dividend Pointer to the bit array containing the dividend. After division, it will hold the remainder.
 * @param quotient Pointer to the bit array where the quotient will be stored.
 * @param divisor  Pointer to the bit array containing the divisor.
 *
 * @example
 * @code
 * BIT_ARRAY dividend, quotient, divisor;
 * // Initialize dividend and divisor...
 * bit_array_divide(&dividend, &quotient, &divisor);
 * // Now, dividend contains the remainder and quotient contains the division result
 * @endcode
 */
void bit_array_divide(BIT_ARRAY *dividend, BIT_ARRAY *quotient, BIT_ARRAY *divisor);

#ifdef __cplusplus
}
#endif

#endif //BITWISE_BIT_ARRAY_H
