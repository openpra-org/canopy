//
// Created by earthperson on 5/5/25.
//

#pragma once

#include "expr/node.h"
#include <string>

namespace canopy::expr {

class builder {

  public:
    // Return an index into the internal node list
    index_t create_symbol(const std::string& symbol_name) {
        node node{};
        node.kind = OperatorKind::Symbol;
        node.id = next_id_++;
        // children not used, etc.
        nodes_.push_back(std::move(node));
        return nodes_.size() - 1;
    }
};

} // namespace canopy::expr
