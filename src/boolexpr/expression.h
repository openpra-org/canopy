#ifndef BOOLEXPR_EXPRESSION_H
#define BOOLEXPR_EXPRESSION_H

#include <algorithm> // reduce
#include <cstdint>   // uint*_t

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "boolexpr/node.h"
#include "boolexpr/util/arena.h"
#include "boolexpr/util/helper_concepts.h"
#include "boolexpr/util/name_indexer.h"

namespace canopy::boolexpr {

template <util::UnsignedInteger NodeID = std::uint16_t, util::UnsignedInteger VarID = std::uint16_t,
          util::UnsignedInteger NumChildrenType = std::uint16_t>
class BooleanExpression {
  public:
    struct Config {
        std::size_t node_capacity{10000};
        std::size_t num_children{1000};
        bool reserve{true};
    };

    struct Node {
        node::NodeType type{};
        NodeID data{};

        friend constexpr bool operator==(const Node &lhs, const Node &rhs) {
            return lhs.type == rhs.type && lhs.data == rhs.data;
        }

        friend constexpr bool operator!=(const Node &lhs, const Node &rhs) {
            return lhs.type != rhs.type || lhs.data != rhs.data;
        }
    };

    using NodeStorage = util::ObjectArena<Node, NodeID>;
    using ChildrenReg = util::SetArena<NodeID, NumChildrenType>;
    using VariableReg = util::NameIndexer<VarID>;

    BooleanExpression() : BooleanExpression(Config{}) {}

    explicit BooleanExpression(const Config &config)
        : nodes{config.node_capacity, config.reserve},
          children{config.node_capacity, config.num_children * config.node_capacity, config.reserve} {
        FALSE_IDX = nodes.create(node::NodeType::Constant, false).value();
        TRUE_IDX = nodes.create(node::NodeType::Constant, true).value();
    }

    NodeID constant(const bool value) const { return value ? TRUE_IDX : FALSE_IDX; }

    auto variable(std::string_view name) {
        const auto var_id = variables.find_or_insert(name);

        // TODO: include of NodeID by var_id
        return nodes.create(node::NodeType::Variable, var_id);
    }

    auto negate(const NodeID operand) { return nodes.create(node::NodeType::NOT, operand); }

    auto conjunct(std::vector<NodeID>&& operands) {
        return apply_nary(node::NodeType::AND, std::move(operands));
    }

    // auto conjunct(std::vector<NodeID> nodes) {
    //     return apply_nary(node::NodeType::AND, std::move(nodes));
    // }

    auto disjunct(std::vector<NodeID>&& operands) {
        return apply_nary(node::NodeType::OR, std::move(operands));
    }

    std::string to_string(const NodeID node) const {
        const auto node_ptr = nodes.at(node);
        if (not node_ptr) {
            return "";
        }

        auto format_nary = [&](const NodeID nary_idx, std::string_view symbol) {
            const auto children_idx = children.get(nary_idx);
            std::vector<std::string> op_strings(children_idx.size());
            std::transform(children_idx.begin(), children_idx.end(), op_strings.begin(),
                           [&](const NodeID child_idx) { return to_string(child_idx); });

            return fmt::format("{}({})", symbol, fmt::join(op_strings, ", "));
        };

        switch (node_ptr->type) {
        case node::NodeType::Constant: {
            return node_ptr->data ? "TRUE" : "FALSE";
        }
        case node::NodeType::Variable: {
            const auto name = variables.get(node_ptr->data);
            if (name.has_value()) {
                return fmt::format("{}", *name);
            }
            return "";
        }
        case node::NodeType::NOT: {
            return fmt::format("NOT({})", to_string(node_ptr->data));
        }
        case node::NodeType::AND: {
            return format_nary(node_ptr->data, "AND");
        }
        case node::NodeType::OR: {
            return format_nary(node_ptr->data, "OR");
        }
        default: {
            std::unreachable();
        }
        }
    }

    constexpr NodeID True() const { return TRUE_IDX; }
    constexpr NodeID False() const { return FALSE_IDX; }

  // private:
    VariableReg variables{};
    NodeStorage nodes{};
    ChildrenReg children{};

    NodeID FALSE_IDX{0};
    NodeID TRUE_IDX{1};

    auto apply_nary(node::NodeType op, std::vector<NodeID>&& operands) {
        // TODO: figure out better input format

        std::sort(std::begin(operands), std::end(operands));

        // TODO: remove duplicates

        return children.store(operands).and_then(
            [&](const auto id) {
                return nodes.create(op, id);
            }
        );
    }
};

} // namespace canopy::boolexpr

#endif
