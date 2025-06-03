#ifndef BOOLEXPR_SIMPLIFY_UTIL_H
#define BOOLEXPR_SIMPLIFY_UTIL_H

#include "boolexpr/node.h"

namespace canopy::boolexpr::transform {

template<Hashable ID>
struct VisitationResult {
    ExprPtr<ID> expr{};
    bool changed{false};
};

} // namespace canopy::boolexpr::simplify

#endif
