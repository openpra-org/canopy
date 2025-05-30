/// @file
/// Contains event classes for fault trees.

#pragma once

#include <cstdint>

#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "mef/openpsa/element.h"
#include "mef/openpsa/expression.h"

namespace mef::openpsa {

/// Abstract base class for general fault tree events.
class Event : public Id, public Usage {
 public:
   static constexpr const char* kTypeString = "event";  ///< For error messages.

   using Id::Id;

   virtual ~Event() = 0;  ///< Abstract class.
};

/// Representation of a house event in a fault tree.
///
/// @note House Events with unset/uninitialized expressions default to False.
class HouseEvent : public Event {
 public:
   static constexpr const char* kTypeString = "house event";  ///< In errors.

   static HouseEvent kTrue;  ///< Literal True event.
   static HouseEvent kFalse;  ///< Literal False event.

   using Event::Event;

   HouseEvent(HouseEvent&&);  ///< For the (N)RVO only (undefined!).

   /// Sets the state for House event.
   ///
   /// @param[in] constant  False or True for the state of this house event.
   void state(bool constant) { state_ = constant; }

   /// @returns The true or false state of this house event.
   [[nodiscard]] bool state() const { return state_; }

 private:
   /// Represents the state of the house event.
   /// Implies On or Off for True or False values of the probability.
   bool state_ = false;
};

class Gate;

/// Representation of a basic event in a fault tree.
class BasicEvent : public Event {
 public:
   static constexpr const char* kTypeString = "basic event";  ///< In errors.

   using Event::Event;

   virtual ~BasicEvent() = default;

   /// @returns true if the probability expression is set.
   [[nodiscard]] bool HasExpression() const { return expression_ != nullptr; }

   /// Sets the expression of this basic event.
   ///
   /// @param[in] expression  The expression to describe this event.
   ///                        nullptr to remove unset the expression.
   void expression(Expression* expression) { expression_ = expression; }

   /// @returns The previously set expression for analysis purposes.
   ///
   /// @pre The expression has been set.
   [[nodiscard]] Expression& expression() const {
       assert(expression_ && "The basic event's expression is not set.");
       return *expression_;
   }

   /// @returns The mean probability of this basic event.
   ///
   /// @pre The expression has been set.
   ///
   /// @note The user of this function should make sure
   ///       that the returned value is acceptable for calculations.
   [[nodiscard]] double p() const noexcept {
       assert(expression_ && "The basic event's expression is not set.");
       return expression_->value();
   }

   /// Validates the probability expressions for the primary event.
   ///
   /// @pre The probability expression is set.
   ///
   /// @throws DomainError  The expression for the basic event is invalid.
   [[maybe_unused]] void Validate() const;

   /// Indicates if this basic event has been set to be in a CCF group.
   ///
   /// @returns true if in a CCF group.
   [[nodiscard]] bool HasCcf() const { return ccf_gate_ != nullptr; }

   /// @returns CCF group gate representing this basic event.
   [[nodiscard]] const Gate& ccf_gate() const {
       assert(ccf_gate_);
       return *ccf_gate_;
   }

   /// Sets the common cause failure group gate
   /// that can represent this basic event
   /// in analysis with common cause information.
   /// This information is expected to be provided by
   /// CCF group application.
   ///
   /// @param[in] gate  CCF group gate.
   void ccf_gate(std::unique_ptr<Gate> gate) {
       assert(!ccf_gate_);
       ccf_gate_ = std::move(gate);
   }

 private:
   /// Expression that describes this basic event
   /// and provides numerical values for probability calculations.
   Expression* expression_ = nullptr;

   /// If this basic event is in a common cause group,
   /// CCF gate can serve as a replacement for the basic event
   /// for common cause analysis.
   std::unique_ptr<Gate> ccf_gate_;
};

class Formula;  // To describe a gate's formula.

/// Logical connectives for formulas.
/// The ordering is the same as analysis connectives in the PDAG.
enum Connective : std::uint8_t {
   kAnd = 0,
   kOr,
   kAtleast,  ///< Combination, K/N, atleast, or Vote gate representation.
   kXor,  ///< Exclusive OR gate with two inputs only.
   kNot,  ///< Boolean negation.
   kNand,  ///< Not AND.
   kNor,  ///< Not OR.
   kNull,  ///< Single argument pass-through without logic.

   // Rarely used connectives specific to the MEF.
   kIff,  ///< Equality with two inputs only.
   kImply,  ///< Implication with two inputs only.
   kCardinality  ///< General quantifier of events.
};

/// The number of connectives in the enum.
static constexpr int kNumConnectives = 11;

/// String representations of the connectives.
/// The ordering is the same as the Connective enum.
const char* const kConnectiveToString[] = {"and", "or",    "atleast",    "xor",
                                          "not", "nand",  "nor",        "null",
                                          "iff", "imply", "cardinality"};

}  // namespace scram::mef
