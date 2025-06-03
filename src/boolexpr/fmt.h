#ifndef BOOLEXPR_FMT_H
#define BOOLEXPR_FMT_H

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "boolexpr/node.h"

namespace canopy::boolexpr {

template <Hashable NameType>
struct FormattingVisitor {
    std::string operator()(const Constant& node) const {
        return fmt::format("{}", node.value);
    }

    std::string operator()(const Variable<NameType>& node) const {
        return node.name;
    }

    std::string operator()(const Not<NameType>& node) const {
        return fmt::format("NOT({})", node.operand->visit(*this));
    }

    template<typename NaryNodeType>
    std::string format_nary(const NaryNodeType& node, std::string_view op_symbol) const {
        if (node.operands.empty()) {
            return fmt::format("{}", NaryNodeType::identity);
        }

        std::vector<std::string> op_strings(node.operands.size());
        std::transform(
            node.operands.cbegin(),
            node.operands.cend(),
            op_strings.begin(),
            [&](const ExprPtr<NameType>& expr) {
                return expr->visit(*this);
            }
        );

        return fmt::format("{}({})", op_symbol, fmt::join(op_strings, ", "));
    }

    std::string operator()(const And<NameType>& node) const {
        return format_nary(node, "AND");
    }

    std::string operator()(const Or<NameType>& node) const {
        return format_nary(node, "OR");
    }
};


template <Hashable NameType>
auto format_as(const BooleanExpression<NameType>& boolexpr) {
    FormattingVisitor<NameType> visitor{};
    return boolexpr.visit(visitor);
}

}


#endif
