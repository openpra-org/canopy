/// @file
/// Provides the base class for all expressions
/// and units for expression values.

#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include <boost/icl/continuous_interval.hpp>
#include <boost/noncopyable.hpp>

#include "mef/openpsa/algorithm.h"
#include "mef/openpsa/error.h"

namespace mef::openpsa {

/// Validation domain interval for expression values.
using Interval = boost::icl::continuous_interval<double>;
/// left_open, open, right_open, closed bounds.
/// @todo Interval bound propagation upon operations on boundary values.
using IntervalBounds [[maybe_unused]] = boost::icl::interval_bounds;

/// Returns true if a given interval contains a given value.
inline bool Contains(const Interval& interval, double value) {
   return boost::icl::contains(interval, Interval::closed(value, value));
}

/// Checks if a given interval is within the probability domain.
inline bool IsProbability(const Interval& interval) {
   return boost::icl::within(interval, Interval::closed(0, 1));
}

/// Checks if all values in a given interval are non-negative.
inline bool IsNonNegative(const Interval& interval) {
   return interval.lower() >= 0;
}

/// Checks if all values in a given interval are positive.
inline bool IsPositive(const Interval& interval) {
   return IsNonNegative(interval) && !Contains(interval, 0);
}

/// Abstract base class for all sorts of expressions to describe events.
/// This class also acts like a connector for parameter nodes
/// and may create cycles.
/// Expressions are not expected to be shared
/// except for parameters.
/// In addition, expressions are not expected to be changed
/// after validation phases.
class Expression : private boost::noncopyable {
 public:
   /// Constructor for use by derived classes
   /// to register their arguments.
   ///
   /// @param[in] args  Arguments of this expression.
   explicit Expression(std::vector<Expression*> args = {})
       : args_(std::move(args)), sampled_value_(0), sampled_(false) {}

   virtual ~Expression() = default;

   /// @returns A set of arguments of the expression.
   [[nodiscard]] const std::vector<Expression*>& args() const { return args_; }

   /// Validates the expression.
   /// This late validation is due to parameters that are defined late.
   ///
   /// @throws DomainError  The argument value domains are invalid.
   /// @throws ValidityError  The arguments are invalid for setup.
   [[maybe_unused]] virtual void Validate() const {}

   /// @returns The mean value of this expression.
   virtual double value() noexcept = 0;

   /// @returns The domain interval for validation purposes only.
   virtual Interval interval() noexcept {
       double value = this->value();
       return Interval::closed(value, value);
   }

   /// Determines if the value of the expression contains deviate expressions.
   /// The default logic is to check arguments with uncertainties for sampling.
   /// Derived expression classes must decide
   /// if they don't have arguments,
   /// or if they are random deviates.
   ///
   /// @returns true if the expression's value deviates from its mean.
   /// @returns false if the expression's value does not need sampling.
   ///
   /// @warning Improper registration of arguments
   ///          may yield silent failure.
   virtual bool IsDeviate() noexcept {
       return any_of(args_, [](Expression* arg) { return arg->IsDeviate(); });
   }

   /// @returns A sampled value of this expression.
   double Sample() noexcept {
       if (!sampled_) {
           sampled_ = true;
           sampled_value_ = this->DoSample();
       }
       return sampled_value_;
   }

   /// This routine resets the sampling to get new values.
   /// All the arguments are called to reset themselves.
   /// If this expression was not sampled,
   /// its arguments are not going to get any calls.
   void Reset() noexcept {
       if (!sampled_)
           return;
       sampled_ = false;
       for (Expression* arg : args_) {
           arg->Reset();
       }
   }

 protected:
   /// Registers an additional argument expression.
   ///
   /// @param[in] arg  An argument expression used by this expression.
   [[maybe_unused]] void AddArg(Expression* arg) { args_.push_back(arg); }

 private:
   /// Runs sampling of the expression.
   /// Derived concrete classes must provide the calculation.
   ///
   /// @returns A sampled value of this expression.
   virtual double DoSample() noexcept = 0;

   std::vector<Expression*> args_;  ///< Expression's arguments.
   double sampled_value_;  ///< The sampled value.
   bool sampled_;  ///< Indication if the expression is already sampled.
};

/// CRTP for Expressions with the same formula to evaluate and sample.
///
/// @tparam T  The Expression type with Compute function.
template <class T>
class ExpressionFormula : public Expression {
 public:
   using Expression::Expression;

   /// Computes the expression with argument expression default values.
   double value() noexcept final {
       return static_cast<T*>(this)->Compute(
           [](Expression* arg) { return arg->value(); });
   }

 private:
   /// Computes the expression with argument expression sampled values.
   double DoSample() noexcept final {
       return static_cast<T*>(this)->Compute(
           [](Expression* arg) { return arg->Sample(); });
   }
};

/// n-ary expressions.
///
/// @tparam T  The callable type of operation to apply to the arguments.
/// @tparam N  The arity of the expression (to be specified).
template <typename T, int N>
class NaryExpression;

/// Unary expression.
template <typename T>
class NaryExpression<T, 1> : public ExpressionFormula<NaryExpression<T, 1>> {
 public:
   /// @param[in] expression  The single argument.
   explicit NaryExpression(Expression* expression)
       : ExpressionFormula<NaryExpression<T, 1>>({expression}),
         expression_(*expression) {}

   void Validate() const override {}

   Interval interval() noexcept override {
       Interval arg_interval = expression_.interval();
       double max_value = T()(arg_interval.upper());
       double min_value = T()(arg_interval.lower());
       auto min_max = std::minmax(max_value, min_value);
       return Interval::closed(min_max.first, min_max.second);
   }

   /// Computes the expression value with a given argument value extractor.
   template <typename F>
   double Compute(F&& eval) noexcept {
       return T()(eval(&expression_));
   }

 private:
   Expression& expression_;  ///< The argument expression.
};

/// Binary expression.
template <typename T>
class NaryExpression<T, 2> : public ExpressionFormula<NaryExpression<T, 2>> {
 public:
   /// Two expression argument constructor.
   explicit NaryExpression(Expression* arg_one, Expression* arg_two)
       : ExpressionFormula<NaryExpression<T, 2>>({arg_one, arg_two}) {}

   void Validate() const override {}

   Interval interval() noexcept override {
       Interval interval_one = Expression::args().front()->interval();
       Interval interval_two = Expression::args().back()->interval();
       double max_max = T()(interval_one.upper(), interval_two.upper());
       double max_min = T()(interval_one.upper(), interval_two.lower());
       double min_max = T()(interval_one.lower(), interval_two.upper());
       double min_min = T()(interval_one.lower(), interval_two.lower());
       auto interval_pair = std::minmax({max_max, max_min, min_max, min_min});
       return Interval::closed(interval_pair.first, interval_pair.second);
   }

   /// Computes the expression value with a given argument value extractor.
   template <typename F>
   double Compute(F&& eval) noexcept {
       return T()(eval(Expression::args().front()),
                  eval(Expression::args().back()));
   }
};

namespace detail {

/// Ensures the number of args for multivariate expressions.
///
/// @param[in] args  Argument expressions.
///
/// @throws ValidityError  The number of arguments is fewer than 2.
void EnsureMultivariateArgs(const std::vector<Expression*>& args) {
    if (args.size() < 2)
        throw ValidityError("Expression requires 2 or more arguments: " + std::to_string(args.size()));
}

}  // namespace detail

/// Multivariate expression.
template <typename T>
class NaryExpression<T, -1> : public ExpressionFormula<NaryExpression<T, -1>> {
 public:
   /// Checks the number of provided arguments upon initialization.
   ///
   /// @param[in] args  Arguments of this expression.
   ///
   /// @throws ValidityError  The number of arguments is fewer than 2.
   explicit NaryExpression(std::vector<Expression*> args)
       : ExpressionFormula<NaryExpression<T, -1>>(std::move(args)) {
       detail::EnsureMultivariateArgs(Expression::args());
   }

   void Validate() const override {}

   Interval interval() noexcept override {
       auto it = Expression::args().begin();
       Interval first_arg_interval = (*it)->interval();
       double max_value = first_arg_interval.upper();
       double min_value = first_arg_interval.lower();
       for (++it; it != Expression::args().end(); ++it) {
           Interval next_arg_interval = (*it)->interval();
           double arg_max = next_arg_interval.upper();
           double arg_min = next_arg_interval.lower();
           double max_max = T()(max_value, arg_max);
           double max_min = T()(max_value, arg_min);
           double min_max = T()(min_value, arg_max);
           double min_min = T()(min_value, arg_min);
           std::tie(min_value, max_value) =
               std::minmax({max_max, max_min, min_max, min_min});
       }
       assert(min_value <= max_value);
       return Interval::closed(min_value, max_value);
   }

   /// Computes the expression value with a given argument value extractor.
   template <typename F>
   double Compute(F&& eval) noexcept {
       auto it = Expression::args().begin();
       double result = eval(*it);
       for (++it; it != Expression::args().end(); ++it) {
           result = T()(result, eval(*it));
       }
       return result;
   }
};

std::string ToString(const Interval& interval) {
    std::stringstream ss;
    ss << interval;
    return ss.str();
}

/// Ensures that expression can be used for probability ([0, 1]).
///
/// @param[in] expression  The expression to be validated.
/// @param[in] type  The type of probability or fraction for error messages.
///
/// @throws DomainError  The expression is not suited for probability.
[[maybe_unused]] void EnsureProbability(Expression* expression,
                      const char* type = "probability") {
    double value = expression->value();
    if (value < 0 || value > 1) {
        throw(DomainError("Invalid " + std::string(type) + " value " + std::to_string(value)));
    }
    Interval interval = expression->interval();
    if (!IsProbability(interval))
        throw(DomainError("Invalid " + std::string(type) + " sample domain " + ToString(interval)));
}

/// Ensures that expression yields positive (> 0) values.
///
/// @param[in] expression  The expression to be validated.
/// @param[in] description  The addition information for error messages.
///
/// @throws DomainError  The expression is not suited for positive values.
[[maybe_unused]] void EnsurePositive(Expression* expression, const char* description) {
    using namespace std::string_literals;  // NOLINT

    double value = expression->value();
    if (value <= 0)
        throw (DomainError(std::string(description) + " argument value must be positive: " + std::to_string(value)));

    Interval interval = expression->interval();
    if (!IsPositive(interval))
        throw(DomainError(std::string(description) + " argument sample domain must be positive " + ToString(interval)));
}

/// Ensures that expression yields non-negative (>= 0) values.
///
/// @param[in] expression  The expression to be validated.
/// @param[in] description  The addition information for error messages.
///
/// @throws DomainError  The expression is not suited for non-negative values.
[[maybe_unused]] void EnsureNonNegative(Expression* expression, const char* description) {
    using namespace std::string_literals;  // NOLINT

    double value = expression->value();
    if (value < 0)
        throw(
            DomainError(std::string(description) + " argument value cannot be negative " + std::to_string(value)));

    Interval interval = expression->interval();
    if (!IsNonNegative(interval))
        throw(DomainError(std::string(description) +
                                " argument sample cannot have negative values " + ToString(interval)));
}

/// Ensures that expression values are within the interval.
///
/// @param[in] expression  The expression to be validated.
/// @param[in] interval  The allowed interval.
/// @param[in] type  The type of expression for error messages.
///
/// @throws DomainError  The expression is not suited for non-negative values.
[[maybe_unused]] void EnsureWithin(Expression* expression, const Interval& interval,
                 const char* type) {
    double arg_value = expression->value();
    if (!Contains(interval, arg_value)) {
        std::stringstream ss;
        ss << type << " argument value must be in " << interval << ".";
        throw(DomainError(ss.str() + std::to_string(arg_value)));
    }
    Interval arg_interval = expression->interval();
    if (!boost::icl::within(arg_interval, interval)) {
        std::stringstream ss;
        ss << type << " argument sample domain must be in " << interval << ".";
        throw(DomainError(ss.str() + ToString(arg_interval)));
    }
}

}  // namespace scram::mef
