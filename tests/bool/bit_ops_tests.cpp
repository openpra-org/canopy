#include <boost/test/unit_test.hpp>

#include "bitwise/bit_ops.h"
#include <cstring>
#include <string>

BOOST_AUTO_TEST_SUITE(BitwiseOperationsTests)


BOOST_AUTO_TEST_CASE(Logical_KN_4_of_4) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "1111");
    BOOST_CHECK(kn(4, bits));
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(Logical_KN_3_of_4) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "1011");
    BOOST_CHECK(kn(3, bits));
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(Logical_KN_2_of_4) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "1010");
    BOOST_CHECK(kn(2, bits));
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(Logical_KN_1_of_4) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "0010");
    BOOST_CHECK(kn(1, bits));
    bit_array_free(bits);
}

BOOST_AUTO_TEST_CASE(Logical_KN_0_of_4) {
    BIT_ARRAY* bits = bit_array_create(0);
    bit_array_from_str(bits, "0000");
    BOOST_CHECK(kn(0, bits));
    bit_array_free(bits);
}

BOOST_AUTO_TEST_SUITE_END()
