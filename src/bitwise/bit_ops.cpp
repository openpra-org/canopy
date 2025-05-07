#include "bitwise/bit_ops.h"
#include "bitwise/bit_array.h"

bool any(BIT_ARRAY *bits) {
    bit_index_t result;
    return (bit_array_find_first_set_bit(bits, &result) > 0);
}

/**
 *
 * @param bits
 * @return
 */
bool all(BIT_ARRAY *bits) {
    bit_array_toggle_all(bits);
    bool all_bits_set = !(any(bits));
    bit_array_toggle_all(bits);
    return all_bits_set;
}

/**
 * Evaluate k/n combinations for @see BIT_ARRAY
 * @param k
 * @param nbits The bit array
 * @return True if k = 0. False if k > n. Evaluates #any if k = 1, #all if k = n.
 */
bool kn(bit_index_t k, BIT_ARRAY* source) {

    //    if (k == 0) {
    //        return true;
    //    }
    //
    //    if (k == 1) {
    //        return any(source);
    //    }

    const auto size = bit_array_length(source);

    //    if (k > size) {
    //        return false;
    //    }
    //
    //    if (k == size) {
    //        return all(source);
    //    }

    bit_index_t numSet = 0;
    for(bit_index_t count = 0; count < size; count++) {
        if(bit_array_get_bit(source, count)) {
            numSet++;
        }
    }


    //    bit_array_print_stdout_newline(accumulator);
    //    for(bit_index_t _and = k-1; _and > 0; _and--) {
    //        bit_array_cycle_left(accumulator, 1); // left shift by 1
    //        bit_array_print_stdout_newline(accumulator);
    //        bit_array_and(accumulator, accumulator, source); // then AND(source, accumulator)
    //        bit_array_print_stdout_newline(accumulator);
    //    }

    return numSet >= k;

    //    std::cout << "n0          = " << std::bitset<WORD_LENGTH>(n.) << '\n';
    //
    //    WORD r1 = std::rotr(n, 1);
    //    std::cout << "r1          = " << std::bitset<WORD_LENGTH>(r1) << '\n';
    //
    //    WORD a1 = r1 & n;
    //    std::cout << "a1          = " << std::bitset<WORD_LENGTH>(a1) << '\n';
    //
    //    WORD r2 = std::rotr(n, 2);
    //    std::cout << "r2          = " << std::bitset<WORD_LENGTH>(r2) << '\n';
    //
    //    WORD a2 = r2 & n;
    //    std::cout << "a2          = " << std::bitset<WORD_LENGTH>(a2) << '\n';


    //    const std::bitset<WORD_LENGTH> r1 =
}
