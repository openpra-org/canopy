#ifndef CANOPY_LOGICBLOCK_H
#define CANOPY_LOGICBLOCK_H

#include <cstddef>
#include <vector>
#include <cstdint>
#include <bitset>
#include <CL/sycl.hpp> // Include SYCL headers

/**
 * @file LogicBlock.h
 * @brief Defines the templated base class for logic blocks optimized for SYCL.
 */

/**
 * @namespace Canopy::Bool
 * @brief Namespace for canopy's boolean project components.
 */

namespace Canopy::Bool {

    template<typename Wires, typename Derived>
    class LogicBlock {
    public:
        /**
         * @brief The number of input wires.
         *
         * This member variable defines how many inputs the logic block accepts.
         */
        const size_t input_width_;

        /**
         * @brief The number of output wires.
         *
         * This member variable defines how many outputs the logic block produces.
         */
        const size_t output_width_;

        /**
         * @brief Constructs a LogicBlock with specified input and output widths.
         *
         * @param input_width The number of input wires.
         * @param output_width The number of output wires.
         *
         * @note This constructor initializes the input and output widths but does not perform
         * any additional logic block setup.
         */
        explicit LogicBlock(const size_t input_width, const size_t output_width)
                : input_width_(input_width), output_width_(output_width) {}

        // Default constructor is deleted to prevent uninitialized LogicBlock.
        LogicBlock() = delete;

        // Default destructor
        ~LogicBlock() = default;

        /**
         * @brief Computes the output based on the input using the derived class's implementation.
         *
         * @param input A SYCL buffer containing the input wires.
         * @param output A SYCL buffer to store the output wires.
         * @param q The SYCL queue to enqueue the kernel.
         */
        void compute(cl::sycl::queue& q, const Wires& input, Wires& output) const {
            // Forward the call to the derived class's implementation
            static_cast<const Derived*>(this)->compute_impl(q, input, output);
        }
    };

} // namespace Canopy::Bool

#endif // CANOPY_LOGICBLOCK_H