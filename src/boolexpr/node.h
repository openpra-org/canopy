#ifndef BOOLEXPR_NODE_H
#define BOOLEXPR_NODE_H

#include <memory>  // shared_ptr
#include <variant> // variant

// Utilities
#include "boolexpr/util.h" // Hashable

#include <vector>

namespace canopy::boolexpr {

template <Hashable NameType>
struct Expression;

template <Hashable NameType>
using ExprPtr = std::shared_ptr<Expression<NameType>>;

struct Constant {
    bool value{};

    static constexpr auto T() {
        return Constant{true};
    }

    static constexpr auto F() {
        return Constant{false};
    }

    template<Hashable NameType>
    static constexpr auto E(const bool value) {
        return std::make_shared<Expression<NameType>>(Constant{value});
    }
};


template <Hashable NameType>
struct Variable {
    NameType name{};

    static constexpr auto E(const NameType& name) {
        return std::make_shared<Expression<NameType>>(Variable{name});
    }
};


template <Hashable NameType> struct Not {
    static constexpr bool primary{true};

    ExprPtr<NameType> operand{};

    static constexpr auto E(ExprPtr<NameType> operand) {
        if (operand) {
            // Rule: !!A -> A
            if (auto* not_op_ptr = std::get_if<Not>(&operand->node_)) {
                return not_op_ptr->operand;
            }
        }
        return std::make_shared<Expression<NameType>>(Not{std::move(operand)});
    }
};


template <Hashable NameType> struct Or {
    static constexpr bool primary{true};
    static constexpr bool identity{false};
    static constexpr bool annihilator{true};

    std::vector<ExprPtr<NameType>> operands{};

    static constexpr auto E(std::vector<ExprPtr<NameType>>&& operands) {
        if (operands.empty()) {
            return Constant::E<NameType>(identity);
        }

        // OR(A) -> A
        if (operands.size() == 1) {
            return operands.front();
        }

        return std::make_shared<Expression<NameType>>(Or{std::move(operands)});
    }

    // void normalize() {
    //     // TODO: Build Normalization tool
    //     // 1. Partition operands: [Constants, Variables, Expressions]
    //     // 2. Apply Associativity to any OR-subexpressions: (A || B) || C <=> ||(A, B, C)
    //     // 3. Remove ConstantNodes:
    //     //      A || True <=> True,
    //     //      A || False <=> A,
    //     // 4. Apply Idempotence Rules: A || A <=> A
    //     // 5. Sort variables and Expressions
    // }
};

template <Hashable NameType> struct And {
    static constexpr bool primary{true};
    static constexpr bool identity{true};
    static constexpr bool annihilator{false};

    std::vector<ExprPtr<NameType>> operands{};

    static constexpr auto E(std::vector<ExprPtr<NameType>>&& operands) {
        // Identity for AND
        if (operands.empty()) {
            return Constant::E<NameType>(identity);
        }

        // AND(A) -> A
        if (operands.size() == 1) {
            return operands.front();
        }

        return std::make_shared<Expression<NameType>>(And{std::move(operands)});
    }
};

template <Hashable NameType>
using Node = std::variant<
    Constant,
    Variable<NameType>,

    // Gates
    Not<NameType>,
    Or<NameType>,
    And<NameType>
>;

template <Hashable NameType> struct Expression {
    Node<NameType> node_{};

    explicit Expression(Node<NameType>&& node) : node_{std::move(node)} {}

    template<class Visitor>
    auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), node_);
    }

    constexpr const auto& node() const {
        return node_;
    }

    constexpr auto& node() {
        return node_;
    }
};

} // namespace canopy::boolexpr

#endif
