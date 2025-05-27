#ifndef BOOLEXPR_NODE_H
#define BOOLEXPR_NODE_H

#include <memory>  // shared_ptr
#include <variant>  // variant

// Operator Types
#include "./operator.h"

// Utilities
#include "./util.h"  // Hashable


namespace canopy::boolexpr {

template<Hashable NameType>
struct ExpressionNode;

template<Hashable NameType>
using NodePtr = std::shared_ptr<ExpressionNode<NameType>>;


struct ConstantNode {
    bool value{};
};


template<Hashable NameType>
struct VariableNode {
    NameType name{};
};


template<Hashable NameType>
struct UnaryOperatorNode {
    UnaryOperatorType op{};

    NodePtr<NameType> operand{};
};


template<Hashable NameType>
struct BinaryOperatorNode {
    BinaryOperatorType op{};

    NodePtr<NameType> lhs{};
    NodePtr<NameType> rhs{};
};


template<Hashable NameType>
struct TernaryOperatorNode {
    TernaryOperatorType op{};

    NodePtr<NameType> condition{};
    NodePtr<NameType> success_branch{};
    NodePtr<NameType> failure_branch{};
};


template<Hashable NameType>
using NodeData = std::variant<
    ConstantNode,
    VariableNode<NameType>,
    UnaryOperatorNode<NameType>,
    BinaryOperatorNode<NameType>,
    TernaryOperatorNode<NameType>
>;


template<Hashable NameType>
struct ExpressionNode {
    NodeData<NameType> data{};

    explicit ExpressionNode(NodeData<NameType>&& data_)
        : data{std::move(data_)}
    {}
};

}

#endif
