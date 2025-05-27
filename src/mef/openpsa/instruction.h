/// @file
/// Model and event tree modifier instructions.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "mef/openpsa/element.h"
#include "mef/openpsa/event/event.h"
#include "mef/openpsa/expression.h"

namespace mef::openpsa {

class InstructionVisitor;

/// Instructions and rules for event tree paths.
class Instruction : private boost::noncopyable {
 public:
   virtual ~Instruction() = default;

   /// Applies the visitor to the object.
   virtual void Accept(InstructionVisitor* visitor) const = 0;
};

/// Default visit for instruction type of T.
template <class T>
class Visitable : public Instruction {
 public:
   /// Calls visit with the object pointer T*.
   void Accept(InstructionVisitor* visitor) const final;
};

/// The operation to change house-events.
class SetHouseEvent : public Visitable<SetHouseEvent> {
 public:
   /// @param[in] name  Non-empty public house-event name.
   /// @param[in] state  The new state for the given house-event.
   SetHouseEvent(std::string name, bool state)
       : name_(std::move(name)), state_(state) {}

   /// @returns The name of the house-event to apply this instruction.
   [[nodiscard]] const std::string& name() const { return name_; }

   /// @returns The state of the target house-event to be changed into.
   [[nodiscard]] bool state() const { return state_; }

 private:
   std::string name_;  ///< The public name of the house event.
   bool state_;  ///< The state for the house event.
};

/// The operation of collecting expressions for event tree sequences.
class CollectExpression : public Visitable<CollectExpression> {
 public:
   /// @param[in] expression  The expression to multiply
   ///                        the current sequence probability.
   explicit CollectExpression(Expression* expression)
       : expression_(expression) {}

   /// @returns The collected expression for value extraction.
   Expression& expression() const { return *expression_; }

 private:
   Expression* expression_;  ///< The probability expression to multiply.
};

/// The operation of connecting fault tree events into the event tree.
class CollectFormula : public Visitable<CollectFormula> {
 public:
   /// @param[in] formula  The valid formula to add into the sequence fault tree.
   explicit CollectFormula(std::unique_ptr<Formula> formula)
       : formula_(std::move(formula)) {}

   /// @returns The formula to include into the current product of the path.
   Formula& formula() const { return *formula_; }

 private:
   std::unique_ptr<Formula> formula_;  ///< The valid single collection formula.
};

/// Conditional application of instructions.
class IfThenElse : public Visitable<IfThenElse> {
 public:
   /// @param[in] expression  The expression to evaluate for truth.
   /// @param[in] then_instruction  The required instruction to execute.
   /// @param[in] else_instruction  An optional instruction for the false case.
   IfThenElse(Expression* expression, Instruction* then_instruction,
              Instruction* else_instruction = nullptr)
       : expression_(expression),
         then_instruction_(std::move(then_instruction)),
         else_instruction_(std::move(else_instruction)) {}

   /// @returns The conditional expression of the ternary instruction.
   Expression* expression() const { return expression_; }

   /// @returns The instruction to execute if the expression is true.
   Instruction* then_instruction() const { return then_instruction_; }

   /// @returns The instruction to execute if the expression is false.
   ///          nullptr if the else instruction is optional and not set.
   Instruction* else_instruction() const { return else_instruction_; }

 private:
   Expression* expression_;  ///< The condition source.
   Instruction* then_instruction_;  ///< The mandatory 'truth' instruction.
   Instruction* else_instruction_;  ///< The optional 'false' instruction.
};

/// Compound instructions.
class Block : public Visitable<Block> {
 public:
   /// @param[in] instructions  Instructions to be applied in this block.
   explicit Block(std::vector<Instruction*> instructions)
       : instructions_(std::move(instructions)) {}

   /// @returns The instructions to be applied in the block.
   [[nodiscard]] const std::vector<Instruction*>& instructions() const {
       return instructions_;
   }

 private:
   std::vector<Instruction*> instructions_;  ///< Zero or more instructions.
};

/// A reusable collection of instructions.
class Rule : public Element,
            public Visitable<Rule>,
            public NodeMark,
            public Usage {
 public:
   static constexpr const char* kTypeString = "rule";  ///< For error messages.

   using Element::Element;

   /// @param[in] instructions  One or more instructions for the sequence.
   void instructions(std::vector<Instruction*> instructions) {
       assert(!instructions.empty());
       instructions_ = std::move(instructions);
   }

   /// @returns The instructions to be applied in the rule.
   const std::vector<Instruction*>& instructions() const {
       return instructions_;
   }

 private:
   std::vector<Instruction*> instructions_;  ///< Instructions to execute.
};

class EventTree;  // The target of the Link (avoid dependency cycle).

/// A link to another event tree in end-states only.
class Link : public Visitable<Link>, public NodeMark {
 public:
   /// @param[in] event_tree  The event tree to be linked in the end-sequence.
   explicit Link(const EventTree& event_tree) : event_tree_(event_tree) {}

   /// @returns The referenced event tree in the link.
   const EventTree& event_tree() const { return event_tree_; }

 private:
   const EventTree& event_tree_;  ///< The referenced event tree.
};

/// The base abstract class for instruction visitors.
class InstructionVisitor {
 public:
   virtual ~InstructionVisitor() = default;

   /// A set of required visitation functions for concrete visitors to implement.
   /// @{
   virtual void Visit(const SetHouseEvent*) = 0;
   virtual void Visit(const CollectExpression*) = 0;
   virtual void Visit(const CollectFormula*) = 0;
   virtual void Visit(const Link*) = 0;
   virtual void Visit(const IfThenElse* ite) {
       if (ite->expression()->value()) {
           ite->then_instruction()->Accept(this);
       } else if (ite->else_instruction()) {
           ite->else_instruction()->Accept(this);
       }
   }
   virtual void Visit(const Block* block) {
       for (const Instruction* instruction : block->instructions())
           instruction->Accept(this);
   }
   virtual void Visit(const Rule* rule) {
       for (const Instruction* instruction : rule->instructions())
           instruction->Accept(this);
   }
   /// @}
};

/// Visits only instructions and ignores non-instructions.
class [[maybe_unused]] NullVisitor : public InstructionVisitor {
 public:
   void Visit(const SetHouseEvent*) override {}
   void Visit(const CollectExpression*) override {}
   void Visit(const CollectFormula*) override {}
   void Visit(const Link*) override {}
   void Visit(const IfThenElse* ite) override {
       ite->then_instruction()->Accept(this);
       if (ite->else_instruction())
           ite->else_instruction()->Accept(this);
   }
};

template <class T>
void Visitable<T>::Accept(InstructionVisitor* visitor) const {
   visitor->Visit(static_cast<const T*>(this));
}

}  // namespace scram::mef
