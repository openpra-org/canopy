/// @file
/// The MEF Substitution constructs.

#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "mef/openpsa/algorithm.h"
#include "mef/openpsa/element.h"
#include "mef/openpsa/event/event.h"

namespace mef::openpsa {

/// The general representation for
/// Delete Terms, Recovery Rules, and Exchange Events.
class Substitution : public Element {
 public:
   /// Type string for error messages.
   static constexpr const char* kTypeString = "substitution";

   using Target = std::variant<BasicEvent*, bool>;  ///< The target type.

   /// The "traditional" substitution types.
   enum Type { kDeleteTerms, kRecoveryRule, kExchangeEvent };

   using Element::Element;

   /// @returns The formula hypothesis of the substitution.
   ///
   /// @pre The required hypothesis formula has been set.
   const Formula& hypothesis() const {
       assert(hypothesis_ && "Substitution hypothesis is not set.");
       return *hypothesis_;
   }

   /// Sets the substitution hypothesis formula.
   ///
   /// @param[in] formula  Simple Boolean formula built over basic events only.
   void hypothesis(std::unique_ptr<Formula> formula) {
       assert(formula && "Cannot unset the hypothesis of substitution.");
       hypothesis_ = std::move(formula);
   }

   /// @returns The target of the substitution.
   ///
   /// @pre The target has been set.
   const Target& target() const { return target_; }

   /// Sets the target of the substitution.
   ///
   /// @param[in] target_event  Basic event or Boolean constant.
   void target(Target target_event) { target_ = std::move(target_event); }

   /// @returns The source events of the substitution.
   const std::vector<BasicEvent*>& source() const { return source_; }

   /// @returns true if the substitution is declarative.
   bool declarative() const { return source_.empty(); }

   /// Adds a source event to the substitution container.
   ///
   /// @param[in] source_event  The event to be replaced by the target event.
   ///
   /// @throws DuplicateElementError  The source event is duplicate.
   void Add(BasicEvent* source_event) {
       if (any_of(source_, [source_event](BasicEvent* arg) {
               return arg->id() == source_event->id();
           })) {
           throw(DuplicateElementError("source event: "+source_event->id()));
       }
       source_.push_back(source_event);
   }

   /// Checks if the substitution is setup correctly.
   ///
   /// @pre The substitution has its hypothesis and target.
   ///
   /// @throws ValidityError  Problems with the substitution setup.
   ///
   /// @note Non-declarative substitutions need to be validated further
   ///       for idempotency across substitutions before analysis.
   void Validate() const  {
       assert(hypothesis_ && "Missing substitution hypothesis.");
       if (any_of(hypothesis_->args(), [](const Formula::Arg& arg) {
               return !std::holds_alternative<BasicEvent*>(arg.event);
           })) {
           throw(ValidityError(
               "Substitution hypothesis must be built over basic events only. "+Element::name()+" "+kTypeString));
       }

       if (any_of(hypothesis_->args(),
                       [](const Formula::Arg& arg) { return arg.complement; })) {
           throw(ValidityError("Substitution hypotheses must be coherent."+Element::name()+" "+kTypeString));
       }

       if (declarative()) {
           switch (hypothesis_->connective()) {
           case kNull:
           case kAnd:
           case kAtleast:
           case kOr:
               break;
           default:
               throw(ValidityError("Substitution hypotheses must be coherent. "+Element::name()+" "+kTypeString + " "+kConnectiveToString[hypothesis_->connective()]));
           }
           const bool* constant = std::get_if<bool>(&target_);
           if (constant && *constant)
               throw(ValidityError("Substitution has no effect."+Element::name()+" "+kTypeString));
       } else {  // Non-declarative.
           switch (hypothesis_->connective()) {
           case kNull:
           case kAnd:
           case kOr:
               break;
           default:
               throw(
                   ValidityError("Non-declarative substitution hypotheses only allow "
                                 "AND/OR/NULL connectives."+Element::name()+" "+kTypeString + " "+kConnectiveToString[hypothesis_->connective()]));
           }
           const bool* constant = std::get_if<bool>(&target_);
           if (constant && !*constant)
               throw(ValidityError("Substitution source set is irrelevant."+Element::name()+" "+kTypeString));
       }
   }

   /// @returns The equivalent "traditional" substitution type if any.
   ///
   /// @pre The hypothesis, target, and source are all defined and valid.
   std::optional<Type> type() const  {
       auto in_hypothesis = [this](const BasicEvent* source_arg) {
           return any_of(hypothesis_->args(),
                              [source_arg](const Formula::Arg& arg) {
                                  return std::get<BasicEvent*>(arg.event) == source_arg;
                              });
       };
       
       auto is_mutually_exclusive = [](const Formula& formula) {
           switch (formula.connective()) {
           case kAtleast:
               return formula.min_number() == 2;
           case kAnd:
               return formula.args().size() == 2;
           default:
               return false;
           }
       };

       if (source_.empty()) {
           if (const bool* constant = std::get_if<bool>(&target_)) {
               assert(!*constant && "Substitution has no effect.");
               if (is_mutually_exclusive(*hypothesis_))
                   return kDeleteTerms;
           } else if (std::holds_alternative<BasicEvent*>(target_)) {
               if (hypothesis_->connective() == kAnd)
                   return kRecoveryRule;
           }
           return {};
       }
       if (!std::holds_alternative<BasicEvent*>(target_))
           return {};
       if (hypothesis_->connective() != kAnd && hypothesis_->connective() != kNull)
           return {};

       if (source_.size() == hypothesis_->args().size()) {
           if (all_of(source_, in_hypothesis))
               return kRecoveryRule;
       } else if (source_.size() == 1) {
           if (in_hypothesis(source_.front()))
               return kExchangeEvent;
       }
       return {};
   }

 private:
   std::unique_ptr<Formula> hypothesis_;  ///< The formula to be satisfied.
   std::vector<BasicEvent*> source_;  ///< The source events to be replaced.
   Target target_;  ///< The target event to replace the source events.
};

/// String representations of the "traditional" substitution types in the MEF.
const char* const kSubstitutionTypeToString[] = {"delete-terms", "recovery-rule", "exchange-event"};

}  // namespace scram::mef
