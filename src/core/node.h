//
// Created by earthperson on 1/5/25.
//

#pragma once

#include <sycl/sycl.hpp>

namespace canopy {

    template<typename bitpack_t_>
    struct node {
        bitpack_t_ *buffer;
    };

    template<typename prob_t_, typename bitpack_t_>
    struct basic_event : node<bitpack_t_> {
        prob_t_ probability;
        int32_t index;
    };

    template<typename bitpack_t_>
    struct tally_event : node<bitpack_t_> {
        std::size_t num_one_bits = 0;
        std::double_t mean = 0.;     // Estimated mean (prob of 1)
        std::double_t std_err = 0.;  // Standard error
        sycl::double4 ci = {0., 0., 0., 0.}; // [ lower_95, upper_95, lower_99, upper_99 ]
    };

    template<typename bitpack_t_>
    tally_event<bitpack_t_> *create_tally_events(const sycl::queue &queue, const std::vector<bitpack_t_ *> &buffers, const std::vector<std::size_t> &initial_values) {
        const auto num_tallies = buffers.size();
        tally_event<bitpack_t_> *tallies = sycl::malloc_shared<tally_event<bitpack_t_>>(num_tallies, queue);
        for (auto i = 0; i < num_tallies; ++i) {
            tallies[i].buffer = buffers[i];
            tallies[i].num_one_bits = initial_values[i];
            tallies[i].mean = 0.0;
            tallies[i].std_err = 0.0;
        }
        return tallies;
    }

    template<typename bitpack_t_>
    void destroy_tally_event(const sycl::queue queue, tally_event<bitpack_t_> *event) {
        sycl::free(event, queue);
    }

    template<typename prob_t_, typename bitpack_t_>
    basic_event<prob_t_, bitpack_t_> *create_basic_events(const sycl::queue &queue, const std::vector<prob_t_> &probabilities, const std::vector<int32_t> &indices, const std::size_t num_bitpacks) {
        const auto num_events = probabilities.size();
        // allocate the basic event objects in a contiguous block
        basic_event<prob_t_, bitpack_t_> *basic_events = sycl::malloc_shared<basic_event<prob_t_, bitpack_t_>>(num_events, queue);
        bitpack_t_* buffers = sycl::malloc_device<bitpack_t_>(num_events * num_bitpacks, queue);
        // allocate basic event buffers separately
        for (auto i = 0; i < num_events; ++i) {
            basic_events[i].probability = probabilities[i];
            basic_events[i].index = indices[i];
            //basic_events[i].buffer = sycl::malloc_device<bitpack_t_>(num_bitpacks, queue);
            basic_events[i].buffer = buffers + i * num_bitpacks;
            //LOG(DEBUG5) <<"building basic event "<<basic_events[i].index<<" with probability "<<basic_events[i].probability;
        }
        return basic_events;
    }

    template<typename prob_t_, typename bitpack_t_>
    void destroy_basic_event(const sycl::queue &queue, basic_event<prob_t_, bitpack_t_> *event) {
        sycl::free(event->buffer, queue);
        sycl::free(event, queue);
    }

    template<typename prob_t_, typename bitpack_t_>
    void destroy_basic_events(const sycl::queue &queue, basic_event<prob_t_, bitpack_t_> *events, const std::size_t count) {
        for (auto i = 0; i < count; ++i) {
            sycl::free(events[i]->buffer, queue);
        }
        sycl::free(events, queue);
    }

    template<typename bitpack_t_, typename size_t_>
    struct gate : node<bitpack_t_> {
        bitpack_t_ **inputs;
        size_t_ num_inputs;
        size_t_ negated_inputs_offset; // first N inputs are positive, the last 'num_inputs - N' inputs are negative
    };

    template<typename bitpack_t_, typename size_t_>
    struct atleast_gate : gate<bitpack_t_, size_t_> {
        std::uint8_t at_least = 0;
    };

    template<typename bitpack_t_, typename size_t_>
    atleast_gate<bitpack_t_, size_t_> *create_atleast_gates(const sycl::queue &queue, const std::vector<std::pair<std::vector<bitpack_t_ *>, size_t_>> &inputs_per_gate, const std::vector<size_t_> &atleast_per_gate, const std::size_t num_bitpacks) {
        const auto num_gates = inputs_per_gate.size();
        // allocate all the gate objects contiguously
        atleast_gate<bitpack_t_, size_t_> *gates = sycl::malloc_shared<atleast_gate<bitpack_t_, size_t_>>(num_gates, queue);
        bitpack_t_* buffers = sycl::malloc_device<bitpack_t_>(num_gates * num_bitpacks, queue);

        for (auto i = 0; i < num_gates; ++i) {
            const auto gate_input_buffers = inputs_per_gate[i].first;
            const auto num_inputs = gate_input_buffers.size();
            const auto num_negated_inputs = inputs_per_gate[i].second;
            gates[i].num_inputs = static_cast<size_t_>(num_inputs);
            gates[i].negated_inputs_offset = static_cast<size_t_>(num_inputs - num_negated_inputs);
            gates[i].inputs = sycl::malloc_shared<bitpack_t_*>(num_inputs, queue);
            gates[i].buffer = buffers + i * num_bitpacks;
            for (auto j = 0; j < num_inputs; ++j) {
                gates[i].inputs[j] = gate_input_buffers[j];
            }
            gates[i].at_least = static_cast<size_t_>(atleast_per_gate[i]);
        }
        return gates;
    }

    template<typename bitpack_t_, typename size_t_>
    gate<bitpack_t_, size_t_> *create_gates(const sycl::queue &queue, const std::vector<std::pair<std::vector<bitpack_t_ *>, size_t_>> &inputs_per_gate, const std::size_t num_bitpacks) {
        const auto num_gates = inputs_per_gate.size();
        // allocate all the gate objects contiguously
        gate<bitpack_t_, size_t_> *gates = sycl::malloc_shared<gate<bitpack_t_, size_t_>>(num_gates, queue);
        bitpack_t_* buffers = sycl::malloc_device<bitpack_t_>(num_gates * num_bitpacks, queue);

        for (auto i = 0; i < num_gates; ++i) {
            const auto gate_input_buffers = inputs_per_gate[i].first;
            const auto num_inputs = gate_input_buffers.size();
            const auto num_negated_inputs = inputs_per_gate[i].second;
            assert(num_negated_inputs <= num_inputs);
            gates[i].num_inputs = static_cast<size_t_>(num_inputs);
            gates[i].negated_inputs_offset = static_cast<size_t_>(num_inputs - num_negated_inputs);
            gates[i].inputs = sycl::malloc_shared<bitpack_t_*>(num_inputs, queue);
            gates[i].buffer = buffers + i * num_bitpacks;
            for (auto j = 0; j < num_inputs; ++j) {
                gates[i].inputs[j] = gate_input_buffers[j];
            }
        }
        return gates;
    }

    template<typename bitpack_t_, typename size_t_>
    void destroy_gate(const sycl::queue &queue, gate<bitpack_t_, size_t_> *gate_ptr) {
        sycl::free(gate_ptr->inputs, queue);// Free the inputs array
        sycl::free(gate_ptr->buffer, queue);// Free the output array
        sycl::free(gate_ptr, queue);        // Free the gate object
    }

    template<typename size_t_>
    struct sample_shape {
        size_t_ batch_size;        // 2nd dimension
        size_t_ bitpacks_per_batch;// 3rd dimension
        size_t_ num_bitpacks() const { return batch_size * bitpacks_per_batch; }
    };
}// namespace scram::canopy
