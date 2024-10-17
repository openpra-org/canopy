//
// Created by Arjun Earthperson on 10/17/24.
//

#ifndef CANOPY_PLA_H
#define CANOPY_PLA_H

#include <cstddef>
#include <vector>

namespace Canopy::Bool {

    template<typename ChunkType>
    class [[maybe_unused]] PLA {
    public:
        using AndTerm = typename AndPlane<ChunkType>::Term;
        using AndProducts = typename AndPlane<ChunkType>::Products;
        using OrTerm = typename OrPlane<ChunkType>::Term;
        using OrOutputs = typename OrPlane<ChunkType>::Outputs;

        PLA() = default;

        // Initialize the PLA with the number of inputs, product terms, and outputs
        void initialize(size_t num_inputs, size_t num_terms, size_t num_outputs) {
            num_symbols_ = num_inputs;
            and_plane_.initialize(num_terms, num_symbols_);
            or_plane_.initialize(num_outputs, num_terms);
        }

        // Set a product term in the AND plane
        void set_and_product(size_t term_index, const AndTerm& term) {
            and_plane_.set_product(term_index, term);
        }

        // Set an output term in the OR plane
        void set_or_output(size_t output_index, const OrTerm& term) {
            or_plane_.set_output(output_index, term);
        }

        // Optimize the PLA by optimizing both planes
        void optimize() {
            and_plane_.optimize();
            // Additional optimizations for OR plane can be added here
        }

        // Evaluate the PLA given the input vector
        std::vector<bool> evaluate(const std::vector<ChunkType>& inputs) const {
            // Evaluate AND plane
            std::vector<bool> and_results = and_plane_.evaluate(inputs);

            // Evaluate OR plane
            std::vector<bool> or_results = or_plane_.evaluate(and_results);

            return or_results;
        }

    private:
        size_t num_symbols_ = 0;
        AndPlane<ChunkType> and_plane_{};
        OrPlane<ChunkType> or_plane_{};
    };

}


#endif //CANOPY_PLA_H
