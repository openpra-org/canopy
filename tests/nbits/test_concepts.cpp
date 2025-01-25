#include <boost/test/unit_test.hpp>

#include <cstdint>
#include "nbits/nbits.h"

BOOST_AUTO_TEST_SUITE(NBitsConceptsTests)

/**
 * @brief Test the EightBitsWide concept.
 * This test verifies that types with 8 bits satisfy the EightBitsWide concept,
 * and types with other sizes do not.
 */
    BOOST_AUTO_TEST_CASE(EightBitsWideTest)
    {
        // Positive tests
        static_assert(canopy::EightBitsWide<uint8_t>, "uint8_t should be EightBitsWide");
        static_assert(canopy::EightBitsWide<int8_t>, "int8_t should be EightBitsWide");
        static_assert(canopy::EightBitsWide<unsigned char>, "unsigned char should be EightBitsWide");
        static_assert(canopy::EightBitsWide<signed char>, "signed char should be EightBitsWide");

        // Assuming CHAR_BIT == 8
        static_assert(CHAR_BIT == 8, "These tests assume CHAR_BIT == 8");

        // Negative tests
        static_assert(!canopy::EightBitsWide<uint16_t>, "uint16_t should not be EightBitsWide");
        static_assert(!canopy::EightBitsWide<int16_t>, "int16_t should not be EightBitsWide");
        static_assert(!canopy::EightBitsWide<char16_t>, "char16_t should not be EightBitsWide");
    }

/**
 * @brief Test the SixteenBitsWide concept.
 * This test verifies that types with 16 bits satisfy the SixteenBitsWide concept,
 * and types with other sizes do not.
 */
    BOOST_AUTO_TEST_CASE(SixteenBitsWideTest)
    {
        // Positive tests
        static_assert(canopy::SixteenBitsWide<uint16_t>, "uint16_t should be SixteenBitsWide");
        static_assert(canopy::SixteenBitsWide<int16_t>, "int16_t should be SixteenBitsWide");
        static_assert(canopy::SixteenBitsWide<char16_t>, "char16_t should be SixteenBitsWide");

        // Negative tests
        static_assert(!canopy::SixteenBitsWide<uint8_t>, "uint8_t should not be SixteenBitsWide");
        static_assert(!canopy::SixteenBitsWide<uint32_t>, "uint32_t should not be SixteenBitsWide");
        static_assert(!canopy::SixteenBitsWide<char32_t>, "char32_t should not be SixteenBitsWide");
    }

/**
 * @brief Test the ThirtyTwoBitsWide concept.
 * This test verifies that types with 32 bits satisfy the ThirtyTwoBitsWide concept,
 * and types with other sizes do not.
 */
    BOOST_AUTO_TEST_CASE(ThirtyTwoBitsWideTest)
    {
        // Positive tests
        static_assert(canopy::ThirtyTwoBitsWide<uint32_t>, "uint32_t should be ThirtyTwoBitsWide");
        static_assert(canopy::ThirtyTwoBitsWide<int32_t>, "int32_t should be ThirtyTwoBitsWide");
        static_assert(canopy::ThirtyTwoBitsWide<float>, "float should be ThirtyTwoBitsWide on most platforms");

        // Negative tests
        static_assert(!canopy::ThirtyTwoBitsWide<uint16_t>, "uint16_t should not be ThirtyTwoBitsWide");
        static_assert(!canopy::ThirtyTwoBitsWide<uint64_t>, "uint64_t should not be ThirtyTwoBitsWide");
        static_assert(!canopy::ThirtyTwoBitsWide<double>, "double should not be ThirtyTwoBitsWide");
    }

/**
 * @brief Test the SixtyFourBitsWide concept.
 * This test verifies that types with 64 bits satisfy the SixtyFourBitsWide concept,
 * and types with other sizes do not.
 */
    BOOST_AUTO_TEST_CASE(SixtyFourBitsWideTest)
    {
        // Positive tests
        static_assert(canopy::SixtyFourBitsWide<uint64_t>, "uint64_t should be SixtyFourBitsWide");
        static_assert(canopy::SixtyFourBitsWide<int64_t>, "int64_t should be SixtyFourBitsWide");
        static_assert(canopy::SixtyFourBitsWide<double>, "double should be SixtyFourBitsWide on most platforms");

        // Negative tests
        static_assert(!canopy::SixtyFourBitsWide<uint32_t>, "uint32_t should not be SixtyFourBitsWide");
        //static_assert(!canopy::SixtyFourBitsWide<long double>, "long double should not be SixtyFourBitsWide on most platforms");
        //static_assert(!canopy::SixtyFourBitsWide<void*>, "void* may not be SixtyFourBitsWide");
    }

/**
 * @brief Test the bits class with template specialization.
 * Ensures that bits can be instantiated with valid window types.
 */
    BOOST_AUTO_TEST_CASE(BitsClassInstantiationTest)
    {
        // Valid instantiations
        canopy::nbits<uint8_t> bits8;
        canopy::nbits<uint16_t> bits16;
        canopy::nbits<uint32_t> bits32;
        canopy::nbits<uint64_t> bits64;

        // Invalid instantiation with a type that doesn't fit any concept
        // Uncommenting the following line should cause a compile-time error
        // canopy::bits<CustomType> bitsCustom;
    }

BOOST_AUTO_TEST_SUITE_END()