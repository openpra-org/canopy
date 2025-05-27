/// @file
/// Functional containers for basic events
/// grouped by common cause failure.
/// Common cause failure can be modeled
/// with alpha, beta, MGL,
/// or direct parameter assignment in phi model.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <cmath>
#include <utility>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include "mef/openpsa/expr/constant.h"
#include "mef/openpsa/expr/numerical.h"
#include "mef/openpsa/element.h"
#include "mef/openpsa/event/event.h"
#include "mef/openpsa/expression.h"

namespace mef::openpsa {

class CcfGroup;  // CCF Events know their own groups.

/// A basic event that represents a multiple failure of
/// a group of events due to a common cause.
/// This event is generated out of a common cause group.
/// This class is a helper to report correctly the CCF events.
class CcfEvent : public BasicEvent {
 public:
   /// Constructs CCF event with specific name
   /// that is used for internal purposes.
   /// This name is formatted with the CcfGroup.
   /// The creator CCF group and the member events of this specific CCF event
   /// are saved for reporting.
   ///
   /// @param[in] members  The members that this CCF event
   ///                     represents as multiple failure.
   /// @param[in] ccf_group  The CCF group that created this event.
   CcfEvent(std::vector<Gate*> members, const CcfGroup* ccf_group);

   /// @returns The CCF group that created this CCF event.
   const CcfGroup& ccf_group() const { return ccf_group_; }

   /// @returns Members of this CCF event.
   ///          The members also own this CCF event through parentship.
   const std::vector<Gate*>& members() const { return members_; }

 private:
   /// Creates a mangled name
   /// that is specific to CCF events and unique per model.
   ///
   /// @param[in] members  The members that this CCF event represents.
   ///
   /// @returns The name string valid only for internal uses.
   static std::string MakeName(const std::vector<Gate*>& members) {
       return "[" +
              boost::join(members | boost::adaptors::transformed(
                                        [](const Gate* gate) -> decltype(auto) {
                                            return gate->name();
                                        }),
                          " ") +
              "]";
   }

   const CcfGroup& ccf_group_;  ///< The originating CCF group.
   std::vector<Gate*> members_;  ///< Member parent gates of this CCF event.
};

/// Abstract base class for all common cause failure models.
class CcfGroup : public Id {
 public:
   /// Type string for error messages.
   static constexpr const char* kTypeString = "CCF group";

   using Id::Id;

   virtual ~CcfGroup() = default;

   /// @returns Members of the CCF group with original names as keys.
   const std::vector<BasicEvent*>& members() const { return members_; }

   /// Adds a basic event into this CCF group.
   /// This function asserts that each basic event has unique string id.
   ///
   /// @param[in] basic_event  A member basic event.
   ///
   /// @throws DuplicateElementError  The basic event is already in the group.
   /// @throws LogicError  The probability distribution or factors
   ///                     for this CCF group are already defined.
   ///                     No more members are accepted.
   void AddMember(BasicEvent* basic_event) {
       if (distribution_ || !factors_.empty()) {
           throw(LogicError("No more members accepted. The distribution for " +
                                  Element::name() +
                                  " CCF group has already been defined."));
       }
       if (any_of(members_, [&basic_event](BasicEvent* member) {
               return member->name() == basic_event->name();
           })) {
           throw(DuplicateElementError(basic_event->name() + " CCF group event"));
       }
       members_.push_back(basic_event);
   }

   /// Adds the distribution that describes the probability of
   /// basic events in this CCF group.
   /// All basic events should be added as members
   /// before defining their probabilities.
   /// No more basic events can be added after this function.
   ///
   /// @param[in] distr  The probability distribution of this group.
   ///
   /// @throws ValidityError  Not enough members.
   /// @throws LogicError  The distribution has already been defined.
   void AddDistribution(Expression* distr) {
       if (distribution_)
           throw(LogicError("CCF distribution is already defined."));
       if (members_.size() < 2) {
           throw(ValidityError("CCF group must have at least 2 members: "+Element::name()+" "+std::string(kTypeString)));
       }
       distribution_ = distr;
       // Define probabilities of all basic events.
       for (BasicEvent* member : members_)
           member->expression(distribution_);
   }

   /// Adds a CCF factor for the specified model.
   /// All basic events should be added as members
   /// before defining the CCF factors.
   /// No more basic events can be added after this function.
   ///
   /// @param[in] factor  A factor for the CCF model.
   /// @param[in] level  The level of the passed factor.
   ///
   /// @throws ValidityError  The level is invalid.
   /// @throws ValidityError  The factor for the level already exists.
   /// @throws LogicError  The level is not positive,
   ///                     or the CCF group members are undefined.
   void AddFactor(Expression* factor, std::optional<int> level = {}) {
       int min_level = this->min_level();
       if (!level)
           level = prev_level_ ? (prev_level_ + 1) : min_level;

       if (*level <= 0 || members_.empty())
           throw(LogicError("Invalid CCF group factor setup."));

       if (*level < min_level) {
           throw(ValidityError("The CCF factor level (" +
                                     std::to_string(*level) +
                                     ") is less than the minimum level (" +
                                     std::to_string(min_level) + "). "+Element::name()+ " "+std::string(kTypeString)));
       }
       if (members_.size() < *level) {
           throw(ValidityError("The CCF factor level " + std::to_string(*level) +
                                     " is more than the number of members (" +
                                     std::to_string(members_.size()) + ")"+Element::name()+ " "+std::string(kTypeString)));
       }

       int index = *level - min_level;
       if (index < factors_.size() && factors_[index].second != nullptr) {
           throw(ValidityError("Redefinition of CCF factor for level " +
                                     std::to_string(*level) +Element::name()+ " "+std::string(kTypeString)));
       }
       if (index >= factors_.size())
           factors_.resize(index + 1);

       factors_[index] = {*level, factor};
       prev_level_ = *level;
   }

   /// Validates the setup for the CCF model and group.
   /// Checks if the provided distribution is between 0 and 1.
   ///
   /// This check must be performed before validating basic events
   /// that are members of this CCF group
   /// to give more precise error messages.
   ///
   /// @throws DomainError  The numeric values are invalid.
   /// @throws ValidityError  There is an issue with the setup.
   /// @throws LogicError  The primary distribution, event, factors are not set.
   void Validate() const {
       try {
           if (!distribution_ || members_.empty() || factors_.empty())
               throw(LogicError("CCF group is not initialized."));

           EnsureProbability(distribution_, "CCF group distribution");

           for (const std::pair<int, Expression*>& f : factors_) {
               if (!f.second)
                   throw(ValidityError("Missing some CCF factors"));

               EnsureProbability(f.second, "CCF group factor");
           }

           this->DoValidate();

       } catch (Error& err) {
           //err << errinfo_element(Element::name(), kTypeString);
           throw;
       }
   }

   /// Processes the given factors and members
   /// to create common cause failure probabilities and new events
   /// that can replace the members in a fault tree.
   ///
   /// @pre The CCF is validated.
   void ApplyModel() {
       // Construct replacement proxy gates for member basic events.
       std::vector<std::pair<Gate*, Formula::ArgSet>> proxy_gates;
       for (BasicEvent* member : members_) {
           auto new_gate = std::make_unique<Gate>(member->name(), member->base_path(),
                                                  member->role());
           assert(member->id() == new_gate->id());
           proxy_gates.push_back({new_gate.get(), {}});
           member->ccf_gate(std::move(new_gate));
       }

       ExpressionMap probabilities = this->CalculateProbabilities();
       assert(probabilities.size() > 1);

       for (const auto& level_prob_pair : probabilities) {
           const auto& level = level_prob_pair.first;
           const auto& prob = level_prob_pair.second;

           using Iterator = decltype(proxy_gates)::iterator;
           auto combination_visitor = [this, &level, &prob](Iterator it_begin, Iterator it_end)
           {
               std::vector<Gate*> combination;
               for (auto it = it_begin; it != it_end; ++it)
                   combination.push_back(it->first);

               auto ccf_event = std::make_unique<CcfEvent>(std::move(combination), this);

               for (auto it = it_begin; it != it_end; ++it)
                   it->second.Add(ccf_event.get());

               ccf_event->expression(prob);
               ccf_events_.emplace_back(std::move(ccf_event));

               return false;
           };
           for_each_combination(proxy_gates.begin(),
                                     std::next(proxy_gates.begin(), level),
                                     proxy_gates.end(), combination_visitor);
       }

       // Assign formulas to the proxy gates.
       for (std::pair<Gate*, Formula::ArgSet>& gate : proxy_gates) {
           assert(gate.second.size() >= 2);
           gate.first->formula(std::make_unique<Formula>(kOr, std::move(gate.second)));
       }
   }

 protected:
   /// Mapping expressions and their application levels.
   using ExpressionMap = std::vector<std::pair<int, Expression*>>;

   /// @returns The probability distribution of the events.
   Expression* distribution() const { return distribution_; }

   /// @returns CCF factors of the model.
   const ExpressionMap& factors() const { return factors_; }

   /// Registers a new expression for ownership by the group.
   /// @{
   template <class T, typename... Ts>
   Expression* Register(Ts&&... args) {
       expressions_.emplace_back(std::make_unique<T>(std::forward<Ts>(args)...));
       return expressions_.back().get();
   }
   template <class T>
   Expression* Register(std::initializer_list<Expression*> args) {
       expressions_.emplace_back(std::make_unique<T>(args));
       return expressions_.back().get();
   }
   /// @}

 private:
   /// @returns The minimum level for CCF factors for the specific model.
   virtual int min_level() const { return 1; }

   /// Runs any additional validation specific to the CCF models.
   /// All the general validation is done in the base class Validate function.
   /// The derived classes should only provided additional logic if any.
   ///
   /// @throws ValidityError  The model is invalid.
   virtual void DoValidate() const {}

   /// Calculates probabilities for new basic events
   /// representing failures due to common cause.
   /// Each derived common cause failure model
   /// must implement this function
   /// with its own specific formulas and assumptions.
   ///
   /// @returns  Expressions representing probabilities
   ///           for each level of groupings for CCF events.
   virtual ExpressionMap CalculateProbabilities() = 0;

   int prev_level_ = 0;  ///< To deduce optional levels from the previous level.
   Expression* distribution_ = nullptr;  ///< The group probability distribution.
   std::vector<BasicEvent*> members_;  ///< Members of CCF groups.
   ExpressionMap factors_;  ///< CCF factors for models to get CCF probabilities.
   /// Collection of expressions created specifically for this group.
   std::vector<std::unique_ptr<Expression>> expressions_;
   /// CCF events created by the group.
   std::vector<std::unique_ptr<CcfEvent>> ccf_events_;
};

/// Common cause failure model that assumes,
/// if common cause failure occurs,
/// then all components or members fail simultaneously or within short time.
class BetaFactorModel : public CcfGroup {
 public:
   using CcfGroup::CcfGroup;

 private:
   int min_level() const override { return CcfGroup::members().size(); }

   ExpressionMap CalculateProbabilities() override {
       assert(CcfGroup::factors().size() == 1);
       assert(CcfGroup::members().size() == CcfGroup::factors().front().first);

       ExpressionMap probabilities;

       Expression* beta = CcfGroup::factors().begin()->second;
       probabilities.emplace_back(  // (1 - beta) * Q
           1, CcfGroup::Register<Mul>(
                  {CcfGroup::Register<Sub>({&ConstantExpression::kOne, beta}),
                   CcfGroup::distribution()}));

       probabilities.emplace_back(  // beta * Q
           CcfGroup::factors().front().first,
           CcfGroup::Register<Mul>({beta, CcfGroup::distribution()}));
       return probabilities;
   }
};

/// Multiple Greek Letters model characterizes failure of
/// sub-groups of the group due to common cause.
/// The factor for k-component group defines
/// fraction of failure k or more members
/// given that (k-1) members failed.
class MglModel : public CcfGroup {
 public:
   using CcfGroup::CcfGroup;

 private:
   int min_level() const override { return 2; }

   ExpressionMap CalculateProbabilities() override {
       ExpressionMap probabilities;
       int max_level = CcfGroup::factors().back().first;
       assert(CcfGroup::factors().size() == max_level - 1);

       int num_members = CcfGroup::members().size();
       for (int i = 0; i < max_level; ++i) {
           double mult = CalculateCombinationReciprocal(num_members - 1, i);
           std::vector<Expression*> args;
           args.push_back(CcfGroup::Register<ConstantExpression>(mult));
           for (int j = 0; j < i; ++j) {
               args.push_back(CcfGroup::factors()[j].second);
           }
           if (i < max_level - 1) {
               args.push_back(CcfGroup::Register<Sub>(
                   {&ConstantExpression::kOne, CcfGroup::factors()[i].second}));
           }
           args.push_back(CcfGroup::distribution());
           probabilities.emplace_back(i + 1, CcfGroup::Register<Mul>(std::move(args)));
       }
       assert(probabilities.size() == max_level);
       return probabilities;
   }
};

/// Alpha factor model characterizes
/// failure of exactly k members of
/// the group due to common cause.
class AlphaFactorModel : public CcfGroup {
 public:
   using CcfGroup::CcfGroup;

 private:
   ExpressionMap CalculateProbabilities() override {
       ExpressionMap probabilities;
       int max_level = CcfGroup::factors().back().first;
       assert(CcfGroup::factors().size() == max_level);
       std::vector<Expression*> sum_args;
       for (const std::pair<int, Expression*>& factor : CcfGroup::factors()) {
           sum_args.emplace_back(CcfGroup::Register<Mul>(
               {CcfGroup::Register<ConstantExpression>(factor.first), factor.second}));
       }
       Expression* sum = CcfGroup::Register<Add>(std::move(sum_args));
       int num_members = CcfGroup::members().size();

       for (int i = 0; i < max_level; ++i) {
           double mult = CalculateCombinationReciprocal(num_members - 1, i);
           Expression* level = CcfGroup::Register<ConstantExpression>(i + 1);
           Expression* fraction =
               CcfGroup::Register<Div>({CcfGroup::factors()[i].second, sum});
           Expression* prob = CcfGroup::Register<Mul>(
               {level, CcfGroup::Register<ConstantExpression>(mult), fraction,
                CcfGroup::distribution()});
           probabilities.emplace_back(i + 1, prob);
       }
       assert(probabilities.size() == max_level);
       return probabilities;
   }
};

/// Phi factor model is a simplification,
/// where fractions of k-member group failure is given directly.
/// Thus, Q_k = phi_k * Q_total.
/// This model is described in the Open-PSA Model Exchange Format.
class PhiFactorModel : public CcfGroup {
 public:
   using CcfGroup::CcfGroup;

 private:
   /// In addition to the default validation of CcfGroup,
   /// checks if the given factors' sum is 1.
   ///
   /// @throws ValidityError  There is an issue with the setup.
   void DoValidate() const override {
       double sum = 0;
       double sum_min = 0;
       double sum_max = 0;
       for (const std::pair<int, Expression*>& factor : CcfGroup::factors()) {
           sum += factor.second->value();
           Interval interval = factor.second->interval();
           sum_min += interval.lower();
           sum_max += interval.upper();
       }
       if (!is_close(1, sum, 1e-4) || !is_close(1, sum_min, 1e-4) ||
           !is_close(1, sum_max, 1e-4)) {
           throw(ValidityError("The factors for Phi model CCF must sum to 1."));
       }
   }

   ExpressionMap CalculateProbabilities() override {
       ExpressionMap probabilities;
       int max_level = CcfGroup::factors().back().first;
       for (const std::pair<int, Expression*>& factor : CcfGroup::factors()) {
           Expression* prob =
               CcfGroup::Register<Mul>({factor.second, CcfGroup::distribution()});
           probabilities.emplace_back(factor.first, prob);
       }
       assert(probabilities.size() == max_level);
       return probabilities;
   }
};

CcfEvent::CcfEvent(std::vector<Gate*> members, const CcfGroup* ccf_group)
    : BasicEvent(MakeName(members), ccf_group->base_path(), ccf_group->role()),
      ccf_group_(*ccf_group),
      members_(std::move(members)) {}

}  // namespace scram::mef
