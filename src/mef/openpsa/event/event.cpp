/// @file
/// Implementation of Event Class and its derived classes.


#include <limits>

#include <boost/range/algorithm.hpp>

#include "mef/openpsa/event/event.h"
#include "mef/openpsa/error.h"
#include "mef/openpsa/algorithm.h"
#include "mef/openpsa/variant.h"

namespace mef::openpsa {

Event::~Event() = default;

HouseEvent HouseEvent::kTrue = []() {
   HouseEvent house_event("__true__");
   house_event.state(true);
   return house_event;
}();
HouseEvent HouseEvent::kFalse("__false__");

[[maybe_unused]] void BasicEvent::Validate() const {
   assert(expression_ && "The basic event's expression is not set.");
   try {
       EnsureProbability(expression_);
   } catch (DomainError& err) {
       //err << errinfo_element(Event::name(), kTypeString);
       throw;
   }
}

void Formula::ArgSet::Add(ArgEvent event, bool complement) {
   auto* base = variant::as<Event*>(event);
   if (any_of(args_, [&base](const Arg& arg) {
           return variant::as<Event*>(arg.event)->id() == base->id();
       })) {
       throw(DuplicateElementError(std::string(base->id())));
   }
   args_.push_back({complement, event});
   if (!base->usage())
       base->usage(true);
}

[[maybe_unused]] void Formula::ArgSet::Remove(ArgEvent event) {
   auto it = boost::find_if(
       args_, [&event](const Arg& arg) { return arg.event == event; });
   if (it == args_.end())
       throw(LogicError("The event is not in the argument set."));
   args_.erase(it);
}

Formula::Formula(Connective connective, ArgSet args,
                std::optional<int> min_number, std::optional<int> max_number)
   : connective_(connective),
     min_number_(min_number.value_or(0)),
     max_number_(max_number.value_or(0)),
     args_(std::move(args)) {
   ValidateMinMaxNumber(min_number, max_number);

   try {
       ValidateConnective(min_number, max_number);
   } catch (ValidityError& err) {
       //err << errinfo_connective(kConnectiveToString[connective_]);
       throw;
   }

   for (const Arg& arg : args_.data())
       ValidateNesting(arg);
}

[[maybe_unused]] std::optional<int> Formula::min_number() const {
   if (connective_ == kAtleast || connective_ == kCardinality)
       return min_number_;
   return {};
}

[[maybe_unused]] std::optional<int> Formula::max_number() const {
   if (connective_ == kCardinality)
       return max_number_;
   return {};
}

[[maybe_unused]] void Formula::Swap(ArgEvent current, ArgEvent other) {
   auto it = boost::find_if(args_.data(), [&current](const Arg& arg) {
       return arg.event == current;
   });
   if (it == args_.data().end())
       throw(LogicError("The current event is not in the formula."));

   auto* base = variant::as<Event*>(other);
   if (any_of(args_.data(), [&current, &base](const Arg& arg) {
           return arg.event != current &&
                  variant::as<Event*>(arg.event)->id() == base->id();
       })) {
       throw(DuplicateElementError());
   }

   ValidateNesting({it->complement, other});

   if (!base->usage())
       base->usage(true);

   variant::swap(it->event, other);
}

void Formula::ValidateMinMaxNumber(std::optional<int> min_number,
                                  std::optional<int> max_number) {
   assert(!min_number ||
          std::numeric_limits<decltype(min_number_)>::max() >= *min_number);
   assert(!max_number ||
          std::numeric_limits<decltype(max_number_)>::max() >= *max_number);

   if (min_number) {
       if (*min_number < 0)
           throw(LogicError("The min number cannot be negative: "+std::to_string(*min_number)));

       if (connective_ != kAtleast && connective_ != kCardinality) {
           throw(LogicError("The min number can only be defined for 'atleast' or 'cardinality' connective." + std::string(kConnectiveToString[connective_])));
       }
   }

   if (max_number) {
       if (*max_number < 0)
           throw(LogicError("The max number cannot be negative: "+std::to_string(*max_number)));

       if (connective_ != kCardinality) {
           throw(LogicError("The max number can only be defined for 'cardinality' connective: "+std::string(kConnectiveToString[connective_])));
       }

       if (min_number && *min_number > *max_number)
           throw(ValidityError(
               "The connective min number cannot be greater than max number:" + (std::to_string(*min_number) + " > " +
                                                                                 std::to_string(*max_number))));
   }
}

void Formula::ValidateConnective(std::optional<int> min_number,
                                std::optional<int> max_number) {
   switch (connective_) {
   case kAnd:
   case kOr:
   case kNand:
   case kNor:
       if (args_.size() < 2)
           throw(
               ValidityError("The connective must have 2 or more arguments."));
       break;
   case kNot:
   case kNull:
       if (args_.size() != 1)
           throw(
               ValidityError("The connective must have only one argument."));
       break;
   case kXor:
   case kIff:
   case kImply:
       if (args_.size() != 2)
           throw(
               ValidityError("The connective must have exactly 2 arguments."));
       break;
   case kAtleast:
       if (!min_number)
           throw(
               ValidityError("The connective requires min number for its args."));

       if (min_number_ < 2)
           throw(ValidityError("Min number cannot be less than 2: "+std::to_string(min_number_)));

       if (args_.size() <= min_number_) {
           throw(
               ValidityError("The connective must have more arguments than its min number:" + (std::to_string(args_.size()) +
                                                                                               " <= " + std::to_string(min_number_))));
       }
       break;
   case kCardinality:
       if (!min_number || !max_number)
           throw(ValidityError(
               "The connective requires min and max numbers for args."));
       if (args_.empty())
           throw(
               ValidityError("The connective requires one or more arguments."));

       if (args_.size() < max_number_)
           throw(
               ValidityError("The connective max number cannot be greater than the number of arguments: "+(std::to_string(max_number_) + " > " +
                                                                                                             std::to_string(args_.size()))));
   }
}

void Formula::ValidateNesting(const Arg& arg) {
   if (arg.complement) {
       if (connective_ == kNull || connective_ == kNot)
           throw(LogicError("Invalid nesting of a complement arg."));
   }
   if (connective_ == kNot) {
       if (arg.event == ArgEvent(&HouseEvent::kTrue) ||
           arg.event == ArgEvent(&HouseEvent::kFalse)) {
           throw(LogicError("Invalid nesting of a constant arg."));
       }
   }
}

}  // namespace scram::mef
