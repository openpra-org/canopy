#ifndef BOOLEXPR_TRANSFORM_TO_H
#define BOOLEXPR_TRANSFORM_TO_H

#include "boolexpr/expression.h"

namespace canopy::boolexpr::transform {

template<class BooleanExpressionType, Hashable ID>
BooleanExpressionType to(const BooleanExpression<ID>& expr);


template<class BooleanExpressionType, Hashable ID>
BooleanExpression<ID> to(const BooleanExpressionType& expr);

}

#endif
