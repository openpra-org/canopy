#include <catch2/catch_test_macros.hpp>

#include <string>

#include "boolexpr/expression.h"
#include "boolexpr/fmt.h"

TEST_CASE("Boolexpr node test", "[Boolexpr]") {
    using namespace canopy::boolexpr;
    using BE = BooleanExpression<std::string>;

    auto p = BE::variable("P");
    const auto q = BE::variable("Q");
    const auto r = BE::variable("R");
    const auto T = BE::constant(true);
    const auto F = BE::constant(false);

    auto expr1 = !!(p && T);
    fmt::println("{}", expr1);

    auto expr2 = !(p && q) || !(r && F);
    fmt::println("{}", expr2);

    REQUIRE(fmt::format("{}", p) == "P");
}