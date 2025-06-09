#ifndef BOOLEXPR_NODE_H
#define BOOLEXPR_NODE_H

// Utilities
#include <vector>
#include <array>
#include <span>

namespace canopy::boolexpr::node {

enum class NodeType : std::uint8_t {
    Constant,
    Variable,
    NOT,
    OR,
    AND,
};


template<class NodeData>
struct Node {
    NodeType type{};
    NodeData data{};

    friend constexpr bool operator==(const Node &lhs, const Node &rhs) {
        return lhs.type == rhs.type && lhs.data == rhs.data;
    }

    friend constexpr bool operator!=(const Node &lhs, const Node &rhs) {
        return lhs.type != rhs.type || lhs.data != rhs.data;
    }
};


template<class NodeData, std::size_t BASE = 26, std::size_t MOD = 1'000'000'007, std::size_t OFFSET = 1>
class NodeHasher {
  public:
    struct Config {
        std::size_t base{BASE};
        std::size_t mod{MOD};
        std::size_t offset{OFFSET};
    };

    constexpr
    explicit NodeHasher(const Config &config = {}) : config_{config} {}

    [[nodiscard]]
    constexpr std::size_t constant(const bool value) const {
        return unary<NodeType::Constant>(static_cast<std::size_t>(value));
    }

    [[nodiscard]]
    constexpr std::size_t variable(const NodeData& value) const {
        return unary<NodeType::Variable>( value);
    }

    [[nodiscard]]
    constexpr std::size_t unary(const NodeType op, const NodeData data) const {
        std::array<std::size_t, 2> hashes{
            node_type_hasher(op),
            node_data_hasher(data),
        };
        return this->hash(hashes);
    }

    template<NodeType op>
    [[nodiscard]]
    constexpr auto unary(const NodeData data) const {
        return unary(op, static_cast<std::size_t>(data));
    }

    [[nodiscard]]
    constexpr std::size_t nary(const NodeType op, std::span<const NodeData> values) const {
        std::vector<std::size_t> hashes(values.size() + 1);
        hashes[0] = node_type_hasher(op);

        std::transform(values.cbegin(), values.cend(), hashes.begin() + 1,
            [&](auto&& value) { return node_data_hasher(value); }
        );
        return this->hash(hashes);
    }

    template<NodeType op>
    [[nodiscard]]
    constexpr std::size_t nary(std::span<const NodeData> values) const {
        return nary(op, values);
    }

    [[nodiscard]]
    constexpr bool operator==(const NodeHasher &other) const {
        return config_ == other.config_;
    }

  private:
    Config config_{};

    std::hash<NodeType> node_type_hasher{};
    std::hash<NodeData> node_data_hasher{};

    [[nodiscard]]
    constexpr std::size_t hash(const std::span<const std::size_t> &elements) const {
        std::size_t power{1};
        std::size_t hash_val{0};

        for (const auto id : elements) {
            hash_val = (hash_val + (id + config_.offset) * power) % config_.mod;
            power = (power * config_.base) % config_.mod;
        }

        return hash_val;
    }
};
} // namespace canopy::boolexpr::node


#endif
