#ifndef BOOLEXPR_OPERATOR_H
#define BOOLEXPR_OPERATOR_H

namespace canopy::boolexpr {

enum class UnaryOperatorType {
    NOT,
    // IDENTITY,
};


enum class BinaryOperatorType {
    AND,
    OR,
    XOR,     // P XOR Q is equivalent to (-P && Q) || (P && -Q)
    IMPLIES, // P -> Q  is equivalent to !P || Q
    IFF      // P <-> Q is equivalent to (P -> Q) && (Q -> P)
};


enum class TernaryOperatorType {
    IF_THEN_ELSE // IF condition THEN success_branch ELSE failure_branch
};


} // namespace canopy::boolexpr

#endif
