/// @file
/// A collection of numerical expressions.
/// @note The PI value is located in constant expressions.

#pragma once

#include <cmath>

#include <functional>
#include <vector>

#include "mef/openpsa/expr/constant.h"
#include "mef/openpsa/expression.h"

namespace mef::openpsa {

/// Creates a functor out of function pointer to common cmath functions.
template <double (*F)(double)>
struct Functor {
   /// Forwards the call to the wrapped function.
   double operator()(double arg) { return F(arg); }
};

/// Expression with a functor wrapping a function.
template <double (*F)(double)>
using FunctorExpression = NaryExpression<Functor<F>, 1>;

/// Creates a functor for functions with two arguments.
template <double (*F)(double, double)>
struct Bifunctor {  // Nasty abuse of terminology :(. Haskellers will hate this.
   /// Forwards the call to the wrapped function.
   double operator()(double arg_one, double arg_two) {
       return F(arg_one, arg_two);
   }
};

/// Expression with a bifunctor wrapping a function.
template <double (*F)(double, double)>
using BifunctorExpression = NaryExpression<Bifunctor<F>, 2>;

using Neg = NaryExpression<std::negate<>, 1>;  ///< Negation.
using Add = NaryExpression<std::plus<>, -1>;  ///< Sum operation.
using Sub = NaryExpression<std::minus<>, -1>;  ///< Subtraction from the first.
using Mul = NaryExpression<std::multiplies<>, -1>;  ///< Product.
using Div = NaryExpression<std::divides<>, -1>;  ///< Division of the first.
using Abs = FunctorExpression<&std::abs>;  ///< The absolute value.
using Acos = FunctorExpression<&std::acos>;  ///< Arc cosine.
using Asin = FunctorExpression<&std::asin>;  ///< Arc sine.
using Atan = FunctorExpression<&std::atan>;  ///< Arc tangent.
using Cos = FunctorExpression<&std::cos>;  ///< Cosine.
using Sin = FunctorExpression<&std::sin>;  ///< Sine.
using Tan = FunctorExpression<&std::tan>;  ///< Tangent.
using Cosh = FunctorExpression<&std::cosh>;  ///< Hyperbolic cosine.
using Sinh = FunctorExpression<&std::sinh>;  ///< Hyperbolic sine.
using Tanh = FunctorExpression<&std::tanh>;  ///< Hyperbolic tangent.
using Exp = FunctorExpression<&std::exp>;  ///< Exponential.
using Log = FunctorExpression<&std::log>;  ///< Natural logarithm.
using Log10 = FunctorExpression<&std::log10>;  ///< Decimal logarithm.
using Mod = NaryExpression<std::modulus<int>, 2>;  ///< Modulo (%) operation.
using Pow = BifunctorExpression<&std::pow>;  ///< Base raised to a power.
using Sqrt = FunctorExpression<&std::sqrt>;  ///< Square root.
using Ceil = FunctorExpression<&std::ceil>;  ///< Nearest (>=) integer.
using Floor = FunctorExpression<&std::floor>;  ///< Nearest (<=) integer.
using Min = NaryExpression<Bifunctor<&std::fmin>, -1>;  ///< Minimum value.
using Max = NaryExpression<Bifunctor<&std::fmax>, -1>;  ///< Maximum value.

/// The average of argument expression values.
class Mean : public ExpressionFormula<Mean> {
 public:
   /// Checks the number of provided arguments upon initialization.
   ///
   /// @param[in] args  Arguments of this expression.
   ///
   /// @throws ValidityError  The number of arguments is fewer than 2.
   explicit Mean(std::vector<Expression*> args) : ExpressionFormula(std::move(args)) {
       if (Expression::args().size() < 2)
           throw(ValidityError("Expression requires 2 or more arguments."));
   }

   Interval interval() noexcept override {
       double min_value = 0;
       double max_value = 0;
       for (Expression* arg : Expression::args()) {
           Interval arg_interval = arg->interval();
           min_value += arg_interval.lower();
           max_value += arg_interval.upper();
       }
       min_value /= Expression::args().size();
       max_value /= Expression::args().size();
       return Interval::closed(min_value, max_value);
   }

   /// Computes the expression value with a given argument value extractor.
   template <typename F>
   double Compute(F&& eval) noexcept {
       double sum = 0;
       for (Expression* arg : Expression::args())
           sum += eval(arg);
       return sum / Expression::args().size();
   }
};

/// @cond Doxygen_With_Smart_Using_Declaration
/// Validation specialization for math functions.
/// @{
template <>
void Div::Validate() const {
    auto it = Expression::args().begin();
    for (++it; it != Expression::args().end(); ++it) {
        const auto& expr = *it;
        Interval arg_interval = expr->interval();
        if (expr->value() == 0 || Contains(arg_interval, 0))
            throw(DomainError("Division by 0."));
    }
}

template <>
inline void Acos::Validate() const {
   assert(args().size() == 1);
   EnsureWithin(args().front(), Interval::closed(-1, 1), "Arc cos");
}

template <>
inline void Asin::Validate() const {
   assert(args().size() == 1);
   EnsureWithin(args().front(), Interval::closed(-1, 1), "Arc sin");
}

template <>
inline void Log::Validate() const {
   assert(args().size() == 1);
   EnsurePositive(args().front(), "Natural Logarithm");
}

template <>
inline void Log10::Validate() const {
   assert(args().size() == 1);
   EnsurePositive(args().front(), "Decimal Logarithm");
}

template <>
void Mod::Validate() const {
    assert(args().size() == 2);
    auto* arg_two = args().back();
    int arg_value = arg_two->value();
    if (arg_value == 0)
        throw(DomainError("Modulo second operand must not be 0."));
    Interval interval = arg_two->interval();
    int high = interval.upper();
    int low = interval.lower();
    if (high == 0 || low == 0 || (low < 0 && 0 < high)) {
        throw(DomainError("Modulo second operand sample must not contain 0."));
    }
}

template <>
void Pow::Validate() const {
    assert(args().size() == 2);
    auto* arg_one = args().front();
    auto* arg_two = args().back();
    if (arg_one->value() == 0 && arg_two->value() <= 0)
        throw(DomainError("0 to power 0 or less is undefined."));
    if (Contains(arg_one->interval(), 0) && !IsPositive(arg_two->interval())) {
        throw(
            DomainError("Power expression 'base' sample range contains 0);"
                        " positive exponent is required."));
    }
}

template <>
inline void Sqrt::Validate() const {
   assert(args().size() == 1);
   EnsureNonNegative(args().front(), "Square root");
}
/// @}

/// Interval specialization for math functions.
/// @{
template <>
inline Interval Acos::interval() noexcept {
   return Interval::closed(0, ConstantExpression::kPi.value());
}

template <>
inline Interval Asin::interval() noexcept {
   double half_pi = ConstantExpression::kPi.value() / 2;
   return Interval::closed(-half_pi, half_pi);
}

template <>
inline Interval Atan::interval() noexcept {
   double half_pi = ConstantExpression::kPi.value() / 2;
   return Interval::closed(-half_pi, half_pi);
}

template <>
inline Interval Cos::interval() noexcept {
   return Interval::closed(-1, 1);
}

template <>
inline Interval Sin::interval() noexcept {
   return Interval::closed(-1, 1);
}
/// @}
/// @endcond

}  // namespace scram::mef
