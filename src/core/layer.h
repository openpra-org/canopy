//
// Created by earthperson on 1/5/25.
//

#pragma once

#include <cinttypes>
#include <array>
#include <sycl/sycl.hpp>

namespace scram::canopy {

    template<typename index_t_, index_t_ num_events>
    struct flattened_layer {
        std::array<int32_t, num_events> buffer_to_pdag_index_map; ///< map each event to an externally referenced PDAG's indices
        std::array<index_t_, num_events> sample_buffer_indices; ///< indices to read/write the samples from/to the externally allocated sample buffer
    };

    template<typename index_t_, index_t_ num_outputs>
    struct flattened_basic_event_layer : flattened_layer<index_t_, num_outputs> {
        std::array<index_t_, num_outputs> probability_buffer_indices; ///< indices to access the externally allocated probability buffer
    };

    template<typename index_t_, index_t_ num_inputs>
    struct flattened_tally_event_layer : flattened_layer<index_t_, num_inputs> {
        std::array<index_t_, num_inputs> tally_buffer_indices; ///< indices to access the externally allocated tally/count buffer
    };

    template<typename index_t_>
    struct node_fragment {
        int32_t pdag_index;
        index_t_ sample_buffer_index;
    };

    template<typename index_t_>
    struct basic_event_fragment : node_fragment<index_t_> {
        index_t_ probabilities_buffer_index;
    };

    template<typename index_t_>
    struct tally_event_fragment : node_fragment<index_t_> {
        index_t_ tallies_buffer_index;
    };

    template<typename index_t_>
    struct gate_fragment : node_fragment<index_t_> {
        index_t_ sample_buffer_input_index_start_offset;
        index_t_ num_inputs;
    };



//     template<typename index_t_, index_t_ num_events>
// struct flattened_layer {
//         std::array<int32_t, num_events> buffer_to_pdag_index_map; ///< map each event to an externally referenced PDAG's indices
//         std::array<index_t_, num_events> sample_buffer_indices; ///< indices to read/write the samples from/to the externally allocated sample buffer
//     };
//
//     template<typename index_t_, index_t_ num_outputs>
//     struct flattened_basic_event_layer : flattened_layer<index_t_, num_outputs> {
//         std::array<index_t_, num_outputs> probability_buffer_indices; ///< indices to access the externally allocated probability buffer
//     };
//
//     template<typename index_t_, index_t_ num_inputs>
//     struct flattened_tally_event_layer : flattened_layer<index_t_, num_inputs> {
//         std::array<index_t_, num_inputs> tally_buffer_indices; ///< indices to access the externally allocated tally/count buffer
//     };
//
//     template<typename index_t_>
//     struct node_fragment {
//         int32_t pdag_index;
//         index_t_ sample_buffer_index;
//     };
//
//     template<typename index_t_>
//     struct basic_event_fragment : node_fragment<index_t_> {
//         index_t_ probabilities_buffer_index;
//     };
//
//     template<typename index_t_>
//     struct tally_event_fragment : node_fragment<index_t_> {
//         index_t_ tallies_buffer_index;
//     };
//
//     template<typename index_t_>
//     struct gate_fragment : node_fragment<index_t_> {
//         index_t_ sample_buffer_input_index_start_offset;
//         index_t_ num_inputs;
//     };

}// namespace scram::canopy

//
