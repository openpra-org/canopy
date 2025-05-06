//
// Created by earthperson on 5/5/25.
//

#pragma once

#include <cstdint>

namespace canopy::expr {
/**
 * @enum OP
 * @brief Basic logic and arithmetic operations.
 *
 * Defines a set of logic and arithmetic operations.
 *
 * Example usage:
 * @code
 * OP operation = OP::XOR;
 * if (operation == OP::XOR) {
 *     // Perform XOR logic
 * }
 * @endcode
 */
enum OP : uint8_t {
    FALSE = 0x00,
    TRUE = 0x01,
    SYMBOL,
    BUFFER, /**< @brief Buffer operation.
                 *  @details Output Z = A. Passes input A directly to output.
                 *  @code
                 *  // Z = A
                 *  @endcode
             */

    MUX, /**< @brief Multiplexer operation.
                 *  @details Output Z = (A • X) | (B • X'). Selects A if X=1, B if X=0.
                 *  @code
                 *  // Z = (A & X) | (B & ~X)
                 *  @endcode
          */

    XOR, /**< @brief Exclusive OR operation.
                 *  @details Output Z = A ⊕ B = (A' • B) | (A • B'). True if A and B differ.
                 *  @code
                 *  // Z = (A ^ B)
                 *  @endcode
          */

    AND, /**< @brief Logical AND operation.
                 *  @details Output Z = A • B.
                 *  @code
                 *  // Z = (A & B)
                 *  @endcode
          */

    OR, /**< @brief Logical OR operation.
                 *  @details Output Z = A | B.
                 *  @code
                 *  // Z = (A | B)
                 *  @endcode
         */

    // negations
    NOT,   /**< @brief Logical NOT operation.
                 *  @details Output Z = A'. Inverts the input.
                 *  @code
                 *  // Z = ~A
                 *  @endcode
            */
    DEMUX, /**< @brief 1-to-2 Demultiplexer operation.
                 *  @details Outputs Z0 = A • X', Z1 = A • X. Routes A to Z0 if X=0, to Z1 if X=1.
                 *  @code
                 *  // Z0 = A & ~X; Z1 = A & X
                 *  @endcode
            */

    XNOR, /**< @brief Exclusive NOR operation.
                 *  @details Output Z = (A • B) | (A' • B'). True if A and B are equal.
                 *  @code
                 *  // Z = ~(A ^ B)
                 *  @endcode
           */

    NAND, /**< @brief Logical NAND operation.
                 *  @details Output Z = (A • B)' = A' | B'. Equivalent to NOT(AND).
                 *  @code
                 *  // Z = ~(A & B)
                 *  @endcode
           */

    NOR, /**< @brief Logical NOR operation.
                 *  @details Output Z = (A | B)' = A' • B'. Equivalent to NOT(OR).
                 *  @code
                 *  // Z = ~(A | B)
                 *  @endcode
          */

    ATLEAST, /**< @brief At-least-k voter operation.
                 *  @details Output Z = 1 if sum(inputs) >= k. True if at least k inputs are true.
                 *  @code
                 *  // Z = (sum(inputs) >= k)
                 *  @endcode
              */

    ATMOST, /**< @brief At-most-k operation.
                 *  @details Output Z = 1 if sum(inputs) <= k. True if at most k inputs are true.
                 *  @code
                 *  // Z = (sum(inputs) <= k)
                 *  @endcode
             */

    EXACTLY, /**< @brief Exactly-k operation.
                 *  @details Output Z = 1 if sum(inputs) == k. True if exactly k inputs are true.
                 *  @code
                 *  // Z = (sum(inputs) == k)
                 *  @endcode
              */

    // multi-output, can be cascaded for higher bit-count

    ADDER, /**< @brief 1-bit Full Adder operation.
                 *  @details SUM = A ⊕ B ⊕ C-IN; C-OUT = (A • B) | (C-IN • (A ⊕ B)).
                 *  Can be cascaded for multi-bit addition.
                 *  @code
                 *  // SUM = A ^ B ^ CIN
                 *  // COUT = (A & B) | (CIN & (A ^ B))
                 *  @endcode
            */

    COMPARATOR, /**< @brief 1-bit Comparator operation.
                 *  @details GT = A • B', EQ = (A ⊕ B)', LT = A' • B.
                 *  For 1-bit: outputs are {GT, EQ, LT} = {A > B, A == B, A < B}.
                 *  @code
                 *  // GT = A & ~B
                 *  // EQ = ~(A ^ B)
                 *  // LT = ~A & B
                 *  @endcode
                 */
};
} // namespace canopy::expr