//
// Created by earthperson on 5/4/25.
//

#pragma once

#include "expr/op.h"

/**
 * canopy::expr should be all about defining, manipulating, converting, and minimizing boolean expressions, based on
 * general, or very specific constraints.
 * Minimization Objectives:
 *  - Total operation count
 *  - Maximum operation depth
     *  - In other words, level sparsity: Each level should be as dense as possible
 *  - should implement heuristics such as ESPRESSO
 *  - should have clean interfaces for implementing and cascading different heuristics schemes.
 * Conversion Objectives (support conversion to the following forms):
 *  - And-Inverter Graph (AIG) [AND, NOT]
 *  - Disjunctive Normal Form (DNF/SoP)
 *  - Conjunctive Normal Form (CNF/PoS)
 *  - Negation Normal Form (NNF) [AND, OR], no NOT gates, symbols will likely get flipped
 *  - Algebraic/Ring Normal Form (ANF) [XOR], no other gate types.
 *  - Binary Decision Diagram (BDD) and variants
 * Specific Constraints Example:
 *  - Minimize the given graph using all tricks possible, with the following constraints:
 *      - N
 */
namespace canopy::expr {

typedef uint32_t index_t;

/*
 * an operation is the outcome of applying an operator to a set of nodes.
 * as such, two operations are identical we can guarantee that any given operation produces an identical outcome,
 * given the same set of inputs. It would be interesting to (a) create guarantees that input order is preserved, and (b)
 * create a hashing/identity function for an operation based on its inputs and type.
 *
 * Other properties we care about:
 *  1. The depth of the operation in the graph
 *  2. The number and count of each type of immediate parent, sibling, and immediate child operations in the graph (can be helpful for simplification heuristics)
 */
struct node {
    //index id;
    //index_t *inputs;
    //uint32_t num_inputs;
    uint32_t negated_inputs_offset; // first N inputs are positive, the last 'num_inputs - N' inputs are logically negated

    // hash of this struct using unsorted!!! inputs
};
}

namespace canopy {

    enum ProbabilityType {
        PointEstimate           = 0b0000,
        EmpiricalDistribution   = 0b0001,
        BetaDistribution        = 0b0001,
        UniformDistribution     = 0b0010,
    };

    template<typename numeric_t>
    struct probability {
        numeric_t *buffer;
    };

    template<typename index_t, typename numeric_t>
    struct node {
        probability<numeric_t>* probability;
        index_t index;
    };

    template<typename index_t, typename numeric_t>
    struct gate : node<index_t, numeric_t> {
        node<index_t, numeric_t> **inputs;
        index_t num_inputs;
        index_t negated_inputs_offset; // first N inputs are positive, the last 'num_inputs - N' inputs are logically negated
    };

    // compiled graph
    //  |
    //  -> layer / level
    //        |
    //        -> partition
    //              |
    //              -> node

    // one input graph -> potentially multiple queues
    // one device -> multiple queues
    // one queue -> one device

}
