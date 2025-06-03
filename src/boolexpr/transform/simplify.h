#ifndef BOOLEXPR_TRANSFORM_SIMPLIFY_H
#define BOOLEXPR_TRANSFORM_SIMPLIFY_H

#include "boolexpr/expression.h"

#include "boolexpr/transform/simplify/constant_folding.h"
#include "boolexpr/transform/simplify/double_negation.h"

namespace canopy::boolexpr::transform {

template <Hashable ID>
BooleanExpression<ID> simplify(BooleanExpression<ID> &expr, const int max_iters = 10);

}

#endif
