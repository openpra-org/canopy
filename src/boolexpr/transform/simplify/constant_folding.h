#ifndef BOOLEXPR_SIMPLIFY_CONSTANT_FOLDING_H
#define BOOLEXPR_SIMPLIFY_CONSTANT_FOLDING_H

#include "boolexpr/node.h"
#include "boolexpr/transform/util/visit_result.h"

namespace canopy::boolexpr::transform::simplify {

template <Hashable ID> struct ConstantFolding {
    using Result = VisitationResult<ID>;

    Result dispatch(const ExprPtr<ID>& expr) const {
        if (
            std::holds_alternative<Constant>(expr->node()) ||
            std::holds_alternative<Variable>(expr->node())
        ) {
            return {expr, false};
        }

        return expr->visit(*this);
    }

    // Result operator()(const Constant& node) const {
    //     return {Constant::E<ID>(node.value)};
    // }

    // std::pair<NodePtr, bool> operator()(const ConstantNode& data) const { return {make_val(data.value), false}; }
    // std::pair<NodePtr, bool> operator()(const VariableNode& data) const { return {make_var(data.name), false}; }

    // Result operator()(const Not<ID>& node) const {
    //     const auto result = this->dispatch(node);
    //
    //     if (auto* const_op = std::get_if<ConstantNode>(&new_operand->data)) {
    //         return {make_val(!const_op->value), true};
    //     }
    //     if (operand_changed) {
    //         return {make_not(new_operand), true};
    //     }
    //     // If operand didn't change and no rule applied, return original structure (new NodePtr pointing to same child)
    //     // To signal no change, we'd ideally return the original NodePtr for `data` if available.
    //     // Since std::visit returns by value, we reconstruct and rely on are_structurally_equal.
    //     // The boolean flag is key here.
    //     return {make_not(std::move(new_operand)), false};
    // }

    // std::pair<NodePtr, bool> operator()(const AndNode& data) const {
    //     std::vector<NodePtr> new_operands;
    //     bool children_changed = false;
    //     for(const auto& op : data.operands) {
    //         auto [new_op, child_changed_flag] = std::visit(*this, op->data);
    //         if (child_changed_flag) children_changed = true;
    //         new_operands.push_back(new_op);
    //     }
    //
    //     bool all_const = true;
    //     bool result = AndNode::identity_element;
    //     if (!new_operands.empty()) {
    //         for(const auto& op : new_operands) {
    //             if(auto* const_op = std::get_if<ConstantNode>(&op->data)) {
    //                 result = result && const_op->value;
    //             } else {
    //                 all_const = false; break;
    //             }
    //         }
    //         if (all_const) {
    //             return {make_val(result), true};
    //         }
    //     } else { // Empty AND is true
    //          // If original was empty, no change. If it became empty, it's a change.
    //          // This factory handles empty case to return make_val(true)
    //          bool changed_to_empty = !data.operands.empty();
    //          return {make_and_op({}), changed_to_empty};
    //     }
    //     if (children_changed) return {make_and_op(std::move(new_operands)), true};
    //     return {make_and_op(std::move(new_operands)), false};
    // }
    //
    // std::pair<NodePtr, bool> operator()(const OrNode& data) const {
    //     std::vector<NodePtr> new_operands;
    //     bool children_changed = false;
    //     for(const auto& op : data.operands) {
    //         auto [new_op, child_changed_flag] = std::visit(*this, op->data);
    //         if (child_changed_flag) children_changed = true;
    //         new_operands.push_back(new_op);
    //     }
    //
    //     bool all_const = true;
    //     bool result = OrNode::identity_element;
    //     if (!new_operands.empty()){
    //         for(const auto& op : new_operands) {
    //             if(auto* const_op = std::get_if<ConstantNode>(&op->data)) {
    //                 result = result || const_op->value;
    //             } else {
    //                 all_const = false; break;
    //             }
    //         }
    //         if (all_const) {
    //             return {make_val(result), true};
    //         }
    //     } else {
    //         bool changed_to_empty = !data.operands.empty();
    //         return {make_or_op({}), changed_to_empty}; // make_val(false)
    //     }
    //     if (children_changed) return {make_or_op(std::move(new_operands)), true};
    //     return {make_or_op(std::move(new_operands)), false};
    // }
};


}

#endif
