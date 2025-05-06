/// @file
/// Conditional (if-then-else, switch-case) expressions.

#pragma once

#include <vector>

#include "mef/openpsa/expression.h"

namespace mef::openpsa {

/// If-Then-Else ternary expression.
class Ite : public ExpressionFormula<Ite> {
 public:
   /// @param[in] condition  The Boolean expression to be tested.
   /// @param[in] then_arm  The expression if the Boolean is true.
   /// @param[in] else_arm  The expression if the Boolean is false.
   Ite(Expression* condition, Expression* then_arm, Expression* else_arm)
       : ExpressionFormula<Ite>({condition, then_arm, else_arm}) {}

   Interval interval() noexcept override {
       assert(args().size() == 3);
       Interval then_interval = args()[1]->interval();
       Interval else_interval = args()[2]->interval();
       return Interval::closed(
           std::min(then_interval.lower(), else_interval.lower()),
           std::max(then_interval.upper(), else_interval.upper()));
   }

   /// Computes the if-then-else expression with the given evaluator.
   template <typename F>
   double Compute(F&& eval) noexcept {
       assert(args().size() == 3);
       return eval(args()[0]) ? eval(args()[1]) : eval(args()[2]);
   }
};

/// Switch-Case conditional operations.
class Switch : public ExpressionFormula<Switch> {
 public:
   /// Individual cases in the switch-case operation.
   struct Case {
       Expression& condition;  ///< The case condition.
       Expression& value;  ///< The value to evaluated if the condition is true.
   };

   /// @param[in] cases  The collection of cases to evaluate.
   /// @param[in] default_value  The default value if all cases are false.
   Switch(std::vector<Case> cases, Expression* default_value) : ExpressionFormula({default_value}),
                                                                cases_(std::move(cases)),
                                                                default_value_(*default_value) {
       for (auto& case_arm : cases_) {
           Expression::AddArg(&case_arm.condition);
           Expression::AddArg(&case_arm.value);
       }
   }

   Interval interval() noexcept override {
       Interval default_interval = default_value_.interval();
       double min_value = default_interval.lower();
       double max_value = default_interval.upper();
       for (auto& case_arm : cases_) {
           Interval case_interval = case_arm.value.interval();
           min_value = std::min(min_value, case_interval.lower());
           max_value = std::max(max_value, case_interval.upper());
       }
       return Interval::closed(min_value, max_value);
   }

   /// Computes the switch-case expression with the given evaluator.
   template <typename F>
   double Compute(F&& eval) noexcept {
       for (Case& case_arm : cases_) {
           if (eval(&case_arm.condition))
               return eval(&case_arm.value);
       }
       return eval(&default_value_);
   }

 private:
   std::vector<Case> cases_;  ///< Ordered collection of cases.
   Expression& default_value_;  ///< The default case value.
};

}
