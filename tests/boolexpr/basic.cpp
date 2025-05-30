#include <catch2/catch_test_macros.hpp>

#include "boolexpr/node.h"

TEST_CASE("Boolexpr node test", "[Boolexpr]") {
    REQUIRE( canopy::boolexpr::ConstantNode{true}.value == true );
}