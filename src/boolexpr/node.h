#ifndef BOOLEXPR_NODE_H
#define BOOLEXPR_NODE_H

// Utilities

namespace canopy::boolexpr::node {

enum class NodeType : std::uint8_t {
    Constant,
    Variable,
    NOT,
    OR,
    AND,
};

} // namespace canopy::boolexpr::node

#endif
