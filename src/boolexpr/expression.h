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

template <util::UnsignedInteger NodeID = std::uint16_t,
          util::UnsignedInteger VarID = std::uint16_t,
          util::UnsignedInteger NumChildrenType = std::uint16_t>
class BooleanExpression {
  public:
    struct Config {
        std::size_t node_capacity{10000};
        std::size_t num_children{1000};
        bool reserve{true};

        typename node::NodeHasher<NodeID>::Config hasher_config{};
    };

    using NodeData = NodeID;
    using Node = node::Node<NodeData>;
    using NodeStorage = util::ObjectArena<Node, NodeID>;
    using ChildrenReg = util::SetArena<NodeID, NumChildrenType>;
    using VariableReg = util::NameIndexer<VarID>;

    BooleanExpression() : BooleanExpression(Config{}) {}

    explicit BooleanExpression(const Config &config)
        : nodes{config.node_capacity, config.reserve},
          children{config.node_capacity, config.num_children * config.node_capacity, config.reserve},
          node_hasher{config.hasher_config}
    {}

    auto constant(const bool value){
        return find_or_insert_unary<node::NodeType::Constant>(value);
    }

    auto variable(std::string_view name) {
        const auto var_id = variables.find_or_insert(name);
        return find_or_insert_unary<node::NodeType::Variable>(var_id);
    }

    std::optional<NodeID> negate(const NodeID operand) {
        if (not exists(operand)) {
            return std::nullopt;
        }
        return find_or_insert_unary<node::NodeType::NOT>(operand);
    }

    auto negate(const std::optional<NodeID> &operand) {
        return operand.and_then(
            [&](const NodeID id) {
                return negate(id);
            }
        );
    }

    auto conjunct(std::vector<NodeID>&& operands) {
        return find_or_insert_nary<node::NodeType::AND>(std::move(operands));
    }

    auto disjunct(std::vector<NodeID>&& operands) {
        return find_or_insert_nary<node::NodeType::OR>(std::move(operands));
    }

    auto exists(const NodeID node) const {
        return nodes.occupied(node);
    }

    auto get_children(const NodeID node) const {
        const auto node_ptr = nodes.at(node);
        if (node_ptr == nullptr) {
            return std::span<const NodeID>{};
        }

        switch (node_ptr->type) {
            case node::NodeType::Constant:
            case node::NodeType::Variable: {
                return std::span<const NodeID>{};
            }
            case node::NodeType::NOT: {
                return std::span<const NodeID>(&(node_ptr->data), 1);
            }
            case node::NodeType::OR:
            case node::NodeType::AND: {
                return children.get(node_ptr->data);
            }
            default: {
                std::unreachable();
            }
        }
    }

    node::NodeType get_type(const NodeID node) const {
        const auto node_ptr = nodes.at(node);
        if (node_ptr == nullptr) {
            std::unreachable();
        }
        return node_ptr->type;
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
            if (const auto name = variables.at(node_ptr->data); name.has_value()) {
                return fmt::format("{}", name.value());
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

  private:
    VariableReg variables{};
    NodeStorage nodes{};
    ChildrenReg children{};

    node::NodeHasher<NodeID> node_hasher{};
    std::unordered_map<std::size_t, NodeID> unique_nodes{};

    template<node::NodeType node_type>
    [[nodiscard]]
    std::optional<NodeID> find_or_insert_unary(const NodeData data) {
        const auto node_hash = node_hasher.template unary<node_type>(data);

        if (const auto node_id = find_node_by_hash(node_hash); node_id.has_value()) {
            return node_id;
        }

        const auto node_id = nodes.create(node_type, data);

        if (node_id.has_value()) {
            add_node_by_hash(node_hash, node_id.value());
        }
        return node_id;
    }

    std::optional<NodeID> find_node_by_hash(const std::size_t hash) const {
        auto it = unique_nodes.find(hash);
        if (it != unique_nodes.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void add_node_by_hash(const std::size_t hash, NodeID idx) {
        unique_nodes.emplace(hash, idx);
    }

    template<node::NodeType node_type>
    std::optional<NodeID> find_or_insert_nary(std::vector<NodeID>&& operands) {
        // TODO: figure out better input format

        std::sort(std::begin(operands), std::end(operands));

        // TODO: remove duplicates

        if (std::any_of(operands.cbegin(), operands.cend(),
            [&](const NodeID id) {
                return not exists(id);
            }))
        {
            return std::nullopt;
        }

        const auto node_hash = node_hasher.template nary<node_type>(operands);
        if (const auto node_id = find_node_by_hash(node_hash); node_id.has_value()) {
            return node_id;
        }

        const auto node_id = children.store(operands).and_then(
            [&](const auto id) {
                return nodes.create(node_type, id);
            }
        );

        if (node_id.has_value()) {
            add_node_by_hash(node_hash, node_id.value());
        }
        return node_id;
    }
};

} // namespace canopy::boolexpr

#endif
