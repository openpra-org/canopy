/// @file
/// Event tree analysis expressions to test functional and initiating events.

#pragma once

#include <string>
#include <unordered_map>

#include "mef/openpsa/algorithm.h"
#include "mef/openpsa/expression.h"
#include "mef/openpsa/find_iterator.h"

namespace mef::openpsa {

/// The context for test-event expressions.
struct Context {
   std::string initiating_event;  ///< The name of the initiating event.
   /// The functional event names and states.
   std::unordered_map<std::string, std::string> functional_events;
};

/// The abstract base class for non-deviate test-event expressions.
class TestEvent : public Expression {
 public:
   /// @param[in] context  The event-tree walk context.
   explicit TestEvent(const Context* context) : context_(*context) {}

   Interval interval() noexcept override { return Interval::closed(0, 1); }
   bool IsDeviate() noexcept override { return false; }

 protected:
   const Context& context_;  ///< The evaluation context.

 private:
   double DoSample() noexcept override { return false; }
};

/// Upon event-tree walk, tests whether an initiating event has occurred.
class TestInitiatingEvent : public TestEvent {
 public:
   /// @copydoc TestEvent::TestEvent
   /// @param[in] name  The public element name of the initiating event to test.
   TestInitiatingEvent(std::string name, const Context* context)
       : TestEvent(context), name_(std::move(name)) {}

   /// @returns true if the initiating event has occurred in the event-tree walk.
   double value() noexcept override {
       return context_.initiating_event == name_;
   }

 private:
   std::string name_;  ///< The name of the initiating event.
};

/// Upon event-tree walk, tests whether a functional event has occurred.
class TestFunctionalEvent : public TestEvent {
 public:
   /// @copydoc TestEvent::TestEvent
   /// @param[in] name  The public element name of the functional event to test.
   /// @param[in] state  One of the valid states of the functional event.
   TestFunctionalEvent(std::string name, std::string state,
                       const Context* context)
       : TestEvent(context), name_(std::move(name)), state_(std::move(state)) {}

   /// @returns true if the functional event has occurred and is in given state.
   double value() noexcept override {
       if (auto it = find(context_.functional_events, name_))
           return it->second == state_;
       return false;
   }

 private:
   std::string name_;  ///< The name of the functional event.
   std::string state_;  ///< The state of the functional event.
};

}  // namespace scram::mef
