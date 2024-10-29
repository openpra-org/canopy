#include <boost/test/unit_test.hpp>

#include "src/bitwise/bar/bit_array.h"
#include <cstring>
#include <string>

BOOST_AUTO_TEST_SUITE(BitwiseMemoryTests)

BOOST_AUTO_TEST_CASE(StringAllocation4bits) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "1111");
    BOOST_CHECK_EQUAL(bit_array_length(bits), 4);
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(StringAllocation97bits) {
    BIT_ARRAY* bits = bit_array_create(0);
    std::string s = "1111010101111010101101101011001010110010010101010101101010100101001010101011010010100101001010101";
    BOOST_CHECK_EQUAL(s.length(), 97);
    bit_array_from_str(bits, s.data());
    BOOST_CHECK_EQUAL(bit_array_length(bits), 97);
    BOOST_CHECK_EQUAL(bit_array_num_bits_set(bits), 97-46);
    BOOST_CHECK_EQUAL(bit_array_num_bits_cleared(bits), 97-51);
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(SimpleAllocation1Gibibit) {
    const uint64_t numBits = 1024 * 1024;
    // const uint64_t numBits = 1024 * 1024;
    BIT_ARRAY* bits = bit_array_create(numBits);
    BOOST_CHECK_EQUAL(bit_array_length(bits), numBits);
    for (auto i = 0, j=0; i < numBits; i++) {
        bit_array_assign_bit(bits, i, j);
        j = (j == 0) ? 1 : 0;
    }
    bit_array_free(bits);
}

BOOST_AUTO_TEST_SUITE_END()
