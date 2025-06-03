#ifndef BOOLEXPR_TRANSFORM_SUBSTITUTE_H
#define BOOLEXPR_TRANSFORM_SUBSTITUTE_H

#include <map>

#include "boolexpr/expression.h"

namespace canopy::boolexpr::transform {

template<Hashable ID>
using Context = std::map<Variable<ID>, bool>;


template <Hashable ID>
BooleanExpression<ID> substitute(BooleanExpression<ID> &expr, const Context<ID>& ctx);


}



#endif
