#ifndef CANOPY_COMPUTE_H
#define CANOPY_COMPUTE_H

/**
 * @file compute.h
 * @brief Core computations
 *
 * Methods implementing the core computation logic
 *
 * @author Arjun Earthperson
 * @date 10/30/2024
 */

/**
 * @brief Implicants / Products
 * -----------
 * A product (F_acc[j]) is an implicant for F if ANY assignment (sample_x[i] | F_acc[j]) evals to 0b11111111.
 * This is because in the SOP representation, any product evaluating to ⊤ will make F ⊤. This because
 * F, as defined, is in SOP form. So, for F in the form F = p1 + p2 + p3, if any product p evals to ⊤,
 * F evals to ⊤. You can think of operations in (sample_x[i] | F_acc[j]) as the AND-PLANE in an AND-OR
 * PLA (programmable logic array).
 *
 * We expect that a well-formed product will be an implicant. What do I mean by well-formed?
 *  (1) no mutually exclusive events i.e. aa'
 *  (2) no always empty events: abcd, where d was not part of X, so it never gets a truth assignment
 *
 * So what's left now are products that take at-least *some* input from the sampling vector X, which
 * makes the product evaluate to ⊤. of course, during sampling, we might not have encountered such an
 * input, *yet*.
 *
 * In a more straight forward sense, the assignment for X that makes any product ⊤ is the definition
 * of the product itself, i.e. a product a'b'c evals to ⊤ when a=⊥, b=⊥, c=⊤. This is what product is.
 **/

/**
* @brief Prime Implicants
* -----------------
* A prime implicant is an implicant that cannot be further reduced by removing any literals without
* ceasing to be an implicant. That is, it is a minimal implicant in terms of the number of literals.
*
* In the function F(a, b) = ab + a'c,
*
*     - The product term **P = ab** is a prime implicant; removing any literal (either **a** or **b**)
*       results in a term that is not an implicant of F because it would evaluate to true in cases where F
*       is false.
*
*     - However, the term **abc** is not a prime implicant because it can be reduced to **ab**
*       (by eliminating **c**) while still being an implicant of F. Therefore, **abc** is not minimal and
*       thus not prime.
**/

/**
* @brief Essential Prime Implicants
* ---------------------------
* A prime implicant is `essential` if there exists an assignment X for that prime implicant that makes
* F eval to ⊤ while all other products make F eval to ⊥ for that assignment. In other words, An
* essential prime implicant is a prime implicant that covers at least one minterm (truth-table row
* where the function evaluates to true) that is not covered by any other prime implicant. The key is
* that there are outputs that only this prime implicant can account for.
*
* In other words, an essential prime implicant is a prime implicant that covers at least one minterm
* (a combination of variable assignments where F is true) that is not covered by any other prime implicant.
* These minterms are exclusively covered by this prime implicant.
*
* Consider F(a, b, c) with the truth table where F is true for minterms m1, m2, and m5.
*      Let:
*          - **P1** be a prime implicant covering minterms m1 and m5.
*          - **P2** be a prime implicant covering minterms m2 and m5.
*
*      Minterm m1 is only covered by **P1**, so **P1** is an essential prime implicant.
*      Minterm m2 is only covered by **P2**, so **P2** is also an essential prime implicant.
*      Minterm m5 is covered by both **P1** and **P2**, so it does not affect the essentiality of either implicant.
**/

/**
* @brief Minimal Sum-of-Products (SoP)
* ------------------------
*
* Every minimal SoP consists of a sum of prime implicants. This does not imply that a minimal SoP
* contains *all* prime implicants. Rather, a minimal SoP contains *all* essential prime implicants, but
* it may also contain non-essential prime implicants.
*
* Therefore, A minimal SoP expression for F is an expression that represents F using the
* fewest possible product terms (and thus the fewest literals). It consists of all essential prime
* implicants and, if necessary, additional prime implicants to cover all minterms where F is true.
**/

#endif //CANOPY_COMPUTE_H
