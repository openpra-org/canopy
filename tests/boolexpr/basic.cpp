#include <catch2/catch_test_macros.hpp>
#include <variant>

#include <string>
#include <fmt/core.h>

#include "boolexpr/expression.h"
#include "boolexpr/util/arena.h"


TEST_CASE("Arena test", "[Arena]") {
    using namespace canopy::boolexpr::util;
    using CharArena = ObjectArena<char, std::uint8_t>;

    constexpr CharArena::idx_type CAPACITY{4};

    CharArena arena{CAPACITY};
    REQUIRE(arena.capacity() == CAPACITY);
    REQUIRE(arena.storage_capacity() == CAPACITY);
    REQUIRE(arena.size() == 0);
    REQUIRE(arena.count() == 0);

    const auto p_idx_opt = arena.create('P');
    REQUIRE(p_idx_opt.has_value());
    const auto p_idx = p_idx_opt.value();
    REQUIRE(p_idx == 0);
    REQUIRE(arena.count() == 1);
    REQUIRE(arena.size() == 1);
    REQUIRE(arena.occupied(p_idx));
    const auto p_ptr = arena.at(p_idx);
    REQUIRE(p_ptr != nullptr);
    REQUIRE(*p_ptr == 'P');

    const auto q_idx_opt = arena.create('Q');
    REQUIRE(q_idx_opt.has_value());
    const auto q_idx = q_idx_opt.value();
    REQUIRE(q_idx == 1);
    REQUIRE(arena.count() == 2);
    REQUIRE(arena.size() == 2);
    REQUIRE(arena.occupied(q_idx));
    const auto q_ptr = arena.at(q_idx);
    REQUIRE(q_ptr != nullptr);
    REQUIRE(*q_ptr == 'Q');

    const auto r_idx_opt = arena.create('R');
    REQUIRE(r_idx_opt.has_value());
    const auto r_idx = r_idx_opt.value();
    REQUIRE(r_idx == 2);
    REQUIRE(arena.count() == 3);
    REQUIRE(arena.size() == 3);
    REQUIRE(arena.occupied(r_idx));
    const auto r_ptr = arena.at(r_idx);
    REQUIRE(r_ptr != nullptr);
    REQUIRE(*r_ptr == 'R');

    const auto s_idx_opt = arena.create('S');
    REQUIRE(s_idx_opt.has_value());
    const auto s_idx = s_idx_opt.value();
    REQUIRE(s_idx == 3);
    REQUIRE(arena.count() == 4);
    REQUIRE(arena.size() == 4);
    REQUIRE(arena.occupied(s_idx));
    const auto s_ptr = arena.at(s_idx);
    REQUIRE(s_ptr != nullptr);
    REQUIRE(*s_ptr == 'S');

    auto t_idx_opt = arena.create('T');
    REQUIRE_FALSE(t_idx_opt.has_value());

    REQUIRE(arena.destroy(p_idx));
    REQUIRE(arena.count() == 3);
    REQUIRE(arena.size() == 4);
    REQUIRE_FALSE(arena.occupied(p_idx));
    REQUIRE_FALSE(arena.destroy(p_idx));

    t_idx_opt = arena.create('T');
    REQUIRE(t_idx_opt.has_value());
    const auto t_idx = t_idx_opt.value();
    REQUIRE(t_idx == p_idx);
    REQUIRE(arena.count() == 4);
    REQUIRE(arena.size() == 4);
    REQUIRE(arena.occupied(t_idx));
    const auto t_ptr = arena.at(t_idx);
    REQUIRE(t_ptr != nullptr);
    REQUIRE(*t_ptr == 'T');
}


TEST_CASE("Boolean Expression", "[BooleanExpression]") {
    using namespace canopy::boolexpr;

    BooleanExpression manager{};

    // fmt::print("{}\n", manager.to_string(manager.True()));
    // fmt::print("{}\n", manager.to_string(manager.False()));

    const auto p = manager.variable("P");
    const auto q = manager.variable("Q");
    const auto r = manager.variable("R");
    REQUIRE(p != q);
    REQUIRE(p != r);
    REQUIRE(r != q);

    const auto p_children = manager.get_children(*p);
    REQUIRE(p_children.size() == 0);

    const auto p_repeat = manager.variable("P");
    REQUIRE(p_repeat == p);

    const auto expr1 = manager.conjunct({*p, *q});
    fmt::println("{}: {}", *expr1, manager.to_string(*expr1));

    const auto expr2 = manager.disjunct({*expr1, *r});
    fmt::println("{}: {}", *expr2, manager.to_string(*expr2));

    const auto expr3 = manager.negate(expr2);
    fmt::println("{}: {}", *expr3, manager.to_string(*expr3));
}