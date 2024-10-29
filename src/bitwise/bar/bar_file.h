#ifndef CANOPY_BAR_FILE_H
#define CANOPY_BAR_FILE_H

/**
 * @file bar_file.h
 * @brief bit array file ops
 *
 * Provides constants definitions and functions loading and saving bit arrays to C/C++ files.
 *
 * @author Arjun Earthperson
 * @date 10/29/2024
 */

#include "bit_array.h"
#include "bar_constants.h"
#include "bar_string.h"

/**
 * @brief Save the bit array to a file.
 *
 * Saves the bit array to a file in the following format:
 * - First 8 bytes: Number of bits in the array (stored as a `bit_index_t`).
 * - Following bytes: Bit data, packed as bytes.
 *   The number of data bytes is `(num_of_bits + 7) / 8`.
 *
 * @param bitarr Pointer to the bit array to save.
 * @param f      File pointer to the file where the bit array will be saved.
 * @return The number of bytes written to the file.
 *
 * @example
 * @code
 * FILE* file = fopen("bitarray.dat", "wb");
 * size_t bytes_written = bit_array_save(bitarr, file);
 * fclose(file);
 * @endcode
 */
bit_index_t bit_array_save(const BIT_ARRAY* bitarr, FILE* f) {
    bit_index_t num_of_bytes = roundup_bits2bytes(bitarr->num_of_bits);
    bit_index_t bytes_written = 0;

    const int endian = 1;
    if(*(uint8_t*)&endian == 1)
    {
        // Little endian machine
        // Write 8 bytes to store the number of bits in the array
        bytes_written += fwrite(&bitarr->num_of_bits, 1, 8, f);

        // Write the array
        bytes_written += fwrite(bitarr->words, 1, num_of_bytes, f);
    }
    else
    {
        // Big endian machine
        uint64_t i, w, whole_words = num_of_bytes/sizeof(word_t);
        uint64_t rem_bytes = num_of_bytes - whole_words*sizeof(word_t);
        uint64_t n_bits = byteswap64(bitarr->num_of_bits);

        // Write 8 bytes to store the number of bits in the array
        bytes_written += fwrite(&n_bits, 1, 8, f);

        // Write the array
        for(i = 0; i < whole_words; i++) {
            w = byteswap64(bitarr->words[i]);
            bytes_written += fwrite(&w, 1, 8, f);
        }

        if(rem_bytes > 0) {
            w = byteswap64(bitarr->words[whole_words]);
            bytes_written += fwrite(&w, 1, rem_bytes, f);
        }
    }

    return bytes_written;
}

// Load a uint64 from little endian format.
// Works for both big and little endian architectures
static inline uint64_t le64_to_cpu(const uint8_t *x) {
    return (((uint64_t)(x[0]))       | ((uint64_t)(x[1]) << 8)  |
            ((uint64_t)(x[2]) << 16) | ((uint64_t)(x[3]) << 24) |
            ((uint64_t)(x[4]) << 32) | ((uint64_t)(x[5]) << 40) |
            ((uint64_t)(x[6]) << 48) | ((uint64_t)(x[7]) << 56));
}

/**
 * @brief Load the bit array from a file.
 *
 * Loads the bit array from a file with the format specified in `bit_array_save()`.
 * The bit array is resized and filled based on the file's contents.
 *
 * @param bitarr Pointer to the bit array to load data into.
 * @param f      File pointer to the file from which the bit array will be loaded.
 * @return `1` on successful loading, `0` on failure.
 *
 * @example
 * @code
 * FILE* file = fopen("bitarray.dat", "rb");
 * if (bit_array_load(bitarr, file)) {
 *     // Successfully loaded
 * } else {
 *     // Handle failure
 * }
 * fclose(file);
 * @endcode
 */
char bit_array_load(BIT_ARRAY* bitarr, FILE* f) {
    // Read in number of bits, return 0 if we can't read in
    bit_index_t num_bits;
    if(fread(&num_bits, 1, 8, f) != 8) return 0;
    num_bits = le64_to_cpu((uint8_t*)&num_bits);

    // Resize
    bit_array_resize_critical(bitarr, num_bits);

    // Have to calculate how many bytes are needed for the file
    // (Note: this may be different from num_of_words * sizeof(word_t))
    bit_index_t num_of_bytes = roundup_bits2bytes(bitarr->num_of_bits);
    if(fread(bitarr->words, 1, num_of_bytes, f) != num_of_bytes) return 0;

    // Fix endianness
    word_addr_t i;
    for(i = 0; i < bitarr->num_of_words; i++)
        bitarr->words[i] = le64_to_cpu((uint8_t*)&bitarr->words[i]);

    // Mask top word
    _mask_top_word(bitarr);
    DEBUG_VALIDATE(bitarr);
    return 1;
}

#endif //CANOPY_BAR_FILE_H
