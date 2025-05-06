/// @file
/// Parameter expressions that act like a shareable variable.

#pragma once

#include <cstdint>

#include "mef/openpsa/element.h"
#include "mef/openpsa/expression.h"

namespace mef::openpsa {

/// Provides units for parameters.
enum Units : std::uint8_t {
   kUnitless = 0,
   kBool,
   kInt,
   kFloat,
   kHours,
   kInverseHours,
   kYears,
   kInverseYears,
   kFit,
   kDemands
};

const int kNumUnits = 10;  ///< The number of elements in the Units enum.

/// String representations of the Units in the same order as the enum.
const char* const kUnitsToString[] = {"unitless", "bool",    "int",   "float",
                                     "hours",    "hours-1", "years", "years-1",
                                     "fit",      "demands"};

/// The special parameter for system mission time.
class MissionTime : public Expression {
 public:
   /// @param[in] time  The mission time.
   /// @param[in] unit  The unit of the given ``time`` argument.
   ///
   /// @throws LogicError  The time value is negative.
   explicit MissionTime(double time = 0, Units unit = kHours) : unit_(unit) { value(time); }

   /// @returns The unit of the system mission time.
   Units unit() const { return unit_; }

   /// Changes the mission time value.
   ///
   /// @param[in] time  The mission time in hours.
   ///
   /// @throws LogicError  The time value is negative.
   void value(double time) {
       if (time < 0)
           throw(LogicError("Mission time cannot be negative."));
       value_ = time;
   }

   double value() noexcept override { return value_; }
   Interval interval() noexcept override { return Interval::closed(0, value_); }
   bool IsDeviate() noexcept override { return false; }

 private:
   double DoSample() noexcept override { return value_; }

   Units unit_;  ///< Units of this parameter.
   double value_;  ///< The universal value to represent int, bool, double.
};

/// This class provides a representation of a variable
/// in basic event description.
/// It is both expression and element description.
class Parameter : public Expression, public Id, public NodeMark, public Usage {
 public:
   /// Type string for errors.
   static constexpr const char* kTypeString = "parameter";

   using Id::Id;

   /// Sets the expression of this parameter.
   ///
   /// @param[in] expression  The expression to describe this parameter.
   ///
   /// @throws LogicError  The parameter expression is already set.
   void expression(Expression* expression) {
       if (expression_)
           throw(LogicError("Parameter expression is already set."));
       expression_ = expression;
       Expression::AddArg(expression);
   }

   /// @returns The unit of this parameter.
   Units unit() const { return unit_; }

   /// Sets the unit of this parameter.
   ///
   /// @param[in] unit  A valid unit.
   void unit(Units unit) { unit_ = unit; }

   double value() noexcept override { return expression_->value(); }
   Interval interval() noexcept override { return expression_->interval(); }

 private:
   double DoSample() noexcept override { return expression_->Sample(); }

   Units unit_ = kUnitless;  ///< Units of this parameter.
   Expression* expression_ = nullptr;  ///< Expression for this parameter.
};

}  // namespace scram::mef
