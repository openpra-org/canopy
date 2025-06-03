#ifndef BOOLEXPR_EXPRESSION_H
#define BOOLEXPR_EXPRESSION_H

#include "boolexpr/node.h"

namespace canopy::boolexpr {

template <Hashable NameType> class BooleanExpression {
  private:
    ExprPtr<NameType> root_{};

    explicit BooleanExpression(ExprPtr<NameType> root) : root_{std::move(root)} {}

  public:
    BooleanExpression(const BooleanExpression &other) = default;
    BooleanExpression &operator=(const BooleanExpression &other) = default;
    BooleanExpression(BooleanExpression &&other) noexcept = default;
    BooleanExpression &operator=(BooleanExpression &&other) noexcept = default;

    constexpr const ExprPtr<NameType>& root() const {
       return root_;
    }

    constexpr ExprPtr<NameType>& root() {
        return root_;
    }

    static constexpr auto constant(const bool value) { return BooleanExpression{Constant::E<NameType>(value)}; }

    static constexpr auto variable(const NameType &value) { return BooleanExpression{Variable<NameType>::E(value)}; }

    friend BooleanExpression operator!(BooleanExpression expr) {
        return BooleanExpression{Not<NameType>::E(std::move(expr.root_))};
    }

    friend BooleanExpression operator&&(BooleanExpression lhs, BooleanExpression rhs) {
        return BooleanExpression{And<NameType>::E({std::move(lhs.root_), std::move(rhs.root_)})};
    }

    friend BooleanExpression operator||(BooleanExpression lhs, BooleanExpression rhs) {
        return BooleanExpression{Or<NameType>::E({std::move(lhs.root_), std::move(rhs.root_)})};
    }

    template <class Visitor> auto visit(Visitor &&visitor) const {
        return root_->visit(std::forward<Visitor>(visitor));
    }
};
} // namespace canopy::boolexpr

#endif
