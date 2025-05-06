/// @file
/// Constant expressions that cannot have uncertainties.

#pragma once

#include <boost/math/constants/constants.hpp>

#include "mef/openpsa/expression.h"

namespace mef::openpsa {

/// Indicates a constant value.
class ConstantExpression : public Expression {
 public:
   static ConstantExpression kOne;  ///< Constant 1 or True.
   static ConstantExpression kZero;  ///< Constant 0 or False.
   static ConstantExpression kPi;  ///< Constant PI value.

   /// Constructor for constant integer, float, and bool values.
   /// In other words, this constructor is implicitly generic.
   ///
   /// @param[in] value  Numerical value.
   explicit ConstantExpression(double value) : value_(value) {}

   double value() noexcept override { return value_; }
   bool IsDeviate() noexcept override { return false; }

 private:
   double DoSample() noexcept override { return value_; }

   const double value_;  ///< The universal value to represent int, bool, double.
};
ConstantExpression ConstantExpression::kOne(1);
ConstantExpression ConstantExpression::kZero(0);
ConstantExpression ConstantExpression::kPi(boost::math::constants::pi<double>());
}  // namespace scram::mef
