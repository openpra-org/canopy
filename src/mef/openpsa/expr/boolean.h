
/// @file
/// Boolean expressions.

#pragma once

#include <functional>

#include "mef/openpsa/expression.h"

namespace mef::openpsa {

using Not = NaryExpression<std::logical_not<>, 1>;  ///< Logical negation.
using And = NaryExpression<std::logical_and<>, -1>;  ///< Logical conjunction.
using Or = NaryExpression<std::logical_or<>, -1>;  ///< Logical disjunction.
using Eq = NaryExpression<std::equal_to<>, 2>;  ///< Equality test.
using Df = NaryExpression<std::not_equal_to<>, 2>;  ///< Inequality test.
using Lt = NaryExpression<std::less<>, 2>;  ///< (<) test.
using Gt = NaryExpression<std::greater<>, 2>;  ///< (>) test.
using Leq = NaryExpression<std::less_equal<>, 2>;  ///< (<=) test.
using Geq = NaryExpression<std::greater_equal<>, 2>;  ///< (>=) test.

}  // namespace scram::mef
