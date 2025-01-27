#pragma once

#include "core/node.h"
#include "core/working_set.h"

#include "core/kernel/basic_event.h"
#include "core/kernel/gate.h"
#include "core/kernel/tally.h"

#include "core/queue/queueable.h"

#include "scram/logger.h"
#include "scram/pdag.h"
#include "scram/event.h"

#include <sycl/sycl.hpp>

namespace canopy::queue {

    template<typename index_t_, typename prob_t_, typename bitpack_t_, typename size_t_>
    static std::shared_ptr<queueable_base> build_kernel_for_variables(
        const std::vector<std::shared_ptr<scram::core::Variable>> &variables_,
        sycl::queue &queue_,
        const sample_shape<size_t_> &sample_shape_,
        std::vector<std::shared_ptr<queueable_base>> &queueables_,
        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> &queueables_by_index_,
        std::unordered_map<index_t_, basic_event<prob_t_, bitpack_t_>*> &allocated_basic_events_by_index_) {

        if (variables_.empty()) {
            return nullptr;
        }

        // 1) Gather each Variable’s index and probability from the BasicEvent data
        std::vector<index_t_> indices;
        std::vector<prob_t_> probabilities;
        indices.reserve(variables_.size());
        probabilities.reserve(variables_.size());

        for (const auto& variable : variables_) {
            const auto var_unique_index = variable->index();
            const auto var_unique_index_in_pdag_indexmap = var_unique_index - 2;
            indices.push_back(var_unique_index);

            // Retrieve the scram::mef::BasicEvent for this variable
            const scram::core::Pdag::IndexMap<const scram::mef::BasicEvent*>& events = variable->graph().basic_events();
            const scram::mef::BasicEvent* event = events.at(var_unique_index_in_pdag_indexmap);
            const double p = event->expression().value();
            probabilities.push_back(static_cast<prob_t_>(p));
        }

        if (indices.empty()) {
            return nullptr;
        }

        const auto num_events_in_kernel = indices.size();

        // 2) Create the basic_event objects (contiguous array)
        using event_type = basic_event<prob_t_, bitpack_t_>;
        event_type* allocated_basic_events = create_basic_events<prob_t_, bitpack_t_>(queue_, probabilities, indices, sample_shape_.num_bitpacks());

        // 3) Build the basic_event kernel
        using kernel_type = kernel::basic_event<prob_t_, bitpack_t_, size_t_>;
        kernel_type be_kernel(allocated_basic_events, num_events_in_kernel, sample_shape_);

        // 4) Compute the ND-range and create a queueable partition
        const auto local_range = working_set<size_t_, bitpack_t_>(queue_, num_events_in_kernel, sample_shape_).compute_optimal_local_range_3d();
        const auto nd_range = be_kernel.get_range(num_events_in_kernel, local_range, sample_shape_);
        iterable_queueable<kernel_type, 3> be_partition(queue_, be_kernel, nd_range);
        auto queueable_partition = std::make_shared<decltype(be_partition)>(be_partition);

        // 5) Update allocated objects and queueables in the global maps
        for (auto i = 0; i < num_events_in_kernel; ++i) {
            const auto event_index_in_pdag = indices[i];
            allocated_basic_events_by_index_[event_index_in_pdag] = allocated_basic_events + i;
            queueables_by_index_[event_index_in_pdag] = queueable_partition;
        }

        // 6) Enqueue for computation
        queueables_.push_back(queueable_partition);
        return queueable_partition;
    }

    template<const scram::core::Connective gate_type_, typename index_t_, typename prob_t_, typename bitpack_t_, typename size_t_>
    static std::shared_ptr<queueable_base> build_kernel_for_gates_of_type(
        const std::vector<std::shared_ptr<scram::core::Gate>> &gates_,
        sycl::queue &queue_,
        const sample_shape<size_t_> &sample_shape_,
        std::vector<std::shared_ptr<queueable_base>> &queueables_,
        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> &queueables_by_index_,
        const std::unordered_map<index_t_, basic_event<prob_t_, bitpack_t_>*> &allocated_basic_events_by_index_,
        std::unordered_map<index_t_, gate<bitpack_t_, size_t_>*> &allocated_gates_by_index_) {

        if (gates_.empty()) {
            return nullptr;
        }

        std::vector<index_t_> indices;
        std::vector<std::pair<std::vector<bitpack_t_*>, size_t_>> inputs_by_gate_with_negated_offset; // positive and negated inputs, with a value for the offset at which the negatives begin
        std::vector<size_t_> atleast_args_by_gate;
        std::set<std::shared_ptr<queueable_base>> layer_dependencies;

        indices.reserve(gates_.size());
        inputs_by_gate_with_negated_offset.reserve(gates_.size());
        atleast_args_by_gate.reserve(gates_.size());

        // 1) For each gate, gather child buffers (from both Variables and child Gates)
        for (const auto& gate_event : gates_)
        {
            const auto gate_index = gate_event->index();
            indices.push_back(gate_index);

            std::vector<bitpack_t_*> positive_gate_inputs;
            std::vector<bitpack_t_*> negative_gate_inputs;
            positive_gate_inputs.reserve(gate_event->args().size());
            negative_gate_inputs.reserve(gate_event->args().size());

            // non-zero only for atleast gates
            atleast_args_by_gate.push_back(gate_event->min_number());

            // For all Variable args
            for (const auto& arg_pair : gate_event->args<scram::core::Variable>()) {
                const auto arg_index = arg_pair.second->index();
                if (!queueables_by_index_.count(arg_index)) {
                    LOG(scram::ERROR) << "Unknown BasicEvent " << arg_index << " in gate " << gate_index;
                    std::exit(EXIT_FAILURE);
                }
                layer_dependencies.insert(queueables_by_index_[arg_index]);
                const auto* basic_ev_ptr = allocated_basic_events_by_index_.at(arg_index);

                // this is a negation of the event, store it in the negations vector
                if (arg_pair.first < 0) {
                    negative_gate_inputs.push_back(basic_ev_ptr->buffer);
                } else {
                    positive_gate_inputs.push_back(basic_ev_ptr->buffer);
                }
            }

            // For all Gate args
            for (auto& arg_pair : gate_event->args<scram::core::Gate>()) {
                const auto arg_index = arg_pair.second->index();
                if (!queueables_by_index_.count(arg_index))
                {
                    LOG(scram::ERROR) << "Unknown Gate " << arg_index << " in gate " << gate_index;
                    std::exit(EXIT_FAILURE);
                }
                layer_dependencies.insert(queueables_by_index_[arg_index]);
                const auto* child_gate_ptr = allocated_gates_by_index_.at(arg_index);

                // this is a negation of the event, store it in the negations vector
                if (arg_pair.first < 0) {
                    negative_gate_inputs.push_back(child_gate_ptr->buffer);
                } else {
                    positive_gate_inputs.push_back(child_gate_ptr->buffer);
                }
            }

            const auto num_negated_events = negative_gate_inputs.size();
            // merge these two vectors
            positive_gate_inputs.insert(positive_gate_inputs.end(), negative_gate_inputs.begin(), negative_gate_inputs.end());

            std::pair<std::vector<bitpack_t_*>, size_t_> gate_inputs(positive_gate_inputs, num_negated_events);
            inputs_by_gate_with_negated_offset.push_back(gate_inputs);
        }

        // 2) Create a contiguous array for these gates
        const size_t num_events_in_layer = indices.size();
        const auto local_range = working_set<size_t_, bitpack_t_>(queue_, num_events_in_layer, sample_shape_).compute_optimal_local_range_3d();

        // 3) Instantiate the appropriate kernel for this gate type, along with the partition
        using kernel_type = kernel::op<gate_type_, bitpack_t_, size_t_>;
        std::shared_ptr<queueable_base> queueable_partition;

        if constexpr (gate_type_ == scram::core::Connective::kAtleast) {
            atleast_gate<bitpack_t_, size_t_>* allocated_gates = create_atleast_gates<bitpack_t_, size_t_>(queue_, inputs_by_gate_with_negated_offset, atleast_args_by_gate, sample_shape_.num_bitpacks());
            kernel_type typed_kernel(allocated_gates, num_events_in_layer, sample_shape_);
            const auto nd_range = typed_kernel.get_range(num_events_in_layer, local_range, sample_shape_);
            queueable<kernel_type, 3> partition(queue_, typed_kernel, nd_range, layer_dependencies);
            queueable_partition = std::make_shared<decltype(partition)>(partition);

            // 4) Record the newly allocated gate pointers and queueables
            for (std::size_t i = 0; i < num_events_in_layer; ++i)
            {
                allocated_gates_by_index_[indices[i]] = (allocated_gates + i);
                queueables_by_index_[indices[i]] = queueable_partition;
            }
        } else {
            gate<bitpack_t_, size_t_>* allocated_gates = create_gates<bitpack_t_, size_t_>(queue_, inputs_by_gate_with_negated_offset, sample_shape_.num_bitpacks());
            kernel_type typed_kernel(allocated_gates, num_events_in_layer, sample_shape_);
            const auto nd_range = typed_kernel.get_range(num_events_in_layer, local_range, sample_shape_);
            queueable<kernel_type, 3> partition(queue_, typed_kernel, nd_range, layer_dependencies);
            queueable_partition = std::make_shared<decltype(partition)>(partition);

            // 4) Record the newly allocated gate pointers and queueables
            for (std::size_t i = 0; i < num_events_in_layer; ++i)
            {
                allocated_gates_by_index_[indices[i]] = (allocated_gates + i);
                queueables_by_index_[indices[i]] = queueable_partition;
            }
        }

        // 5) Queue for execution
        queueables_.push_back(queueable_partition);

        return queueable_partition;
    }

    template<typename index_t_, typename prob_t_, typename bitpack_t_, typename size_t_>
    static std::vector<std::shared_ptr<queueable_base>> build_kernels_for_gates(
        const std::unordered_map<scram::core::Connective, std::vector<std::shared_ptr<scram::core::Gate>>> &gates_by_type,
        sycl::queue &queue_,
        const sample_shape<size_t_> &sample_shape_,
        std::vector<std::shared_ptr<queueable_base>> &queueables_,
        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> &queueables_by_index_,
        const std::unordered_map<index_t_, basic_event<prob_t_, bitpack_t_>*> &allocated_basic_events_by_index_,
        std::unordered_map<index_t_, gate<bitpack_t_, size_t_>*> &allocated_gates_by_index_
        ) {
        std::vector<std::shared_ptr<queueable_base>> kernels;
        kernels.reserve(gates_by_type.size());

        for (const auto &[gate_type, gates_] : gates_by_type)
        {
            if (gates_.empty()) {
                continue;
            }

            std::shared_ptr<queueable_base> kernel = nullptr;

            switch (gate_type) {
                case scram::core::kAnd:
                    kernel = build_kernel_for_gates_of_type<scram::core::kAnd, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kOr:
                    kernel = build_kernel_for_gates_of_type<scram::core::kOr, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kAtleast:
                    kernel = build_kernel_for_gates_of_type<scram::core::kAtleast, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kXor:
                    kernel = build_kernel_for_gates_of_type<scram::core::kXor, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kNot:
                    kernel = build_kernel_for_gates_of_type<scram::core::kNot, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kNand:
                    kernel = build_kernel_for_gates_of_type<scram::core::kNand, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kNor:
                    kernel = build_kernel_for_gates_of_type<scram::core::kNor, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
                case scram::core::kNull:
                    kernel = build_kernel_for_gates_of_type<scram::core::kNull, index_t_, prob_t_, bitpack_t_, size_t_>(gates_, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);
                    break;
            }
            // Build exactly one kernel for gates of this type in the layer
            if (kernel) {
                kernels.push_back(kernel);
            }
        }

        return kernels;
    }

    template<typename index_t_, typename prob_t_, typename bitpack_t_, typename size_t_>
    static std::shared_ptr<queueable_base> build_tallies_for_layer(
        const std::vector<std::shared_ptr<scram::core::Node>> &nodes,
        sycl::queue &queue_,
        const sample_shape<size_t_> &sample_shape_,
        std::vector<std::shared_ptr<queueable_base>> &queueables_,
        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> &queueables_by_index_,
        const std::unordered_map<index_t_, basic_event<prob_t_, bitpack_t_>*> &allocated_basic_events_by_index_,
        const std::unordered_map<index_t_, gate<bitpack_t_, size_t_>*> &allocated_gates_by_index_,
        std::unordered_map<index_t_, tally_event<bitpack_t_> *> &allocated_tally_events_by_index_) {
        // collect all events in this layer
        std::vector<index_t_> indices;
        std::vector<bitpack_t_ *> node_buffers;
        std::vector<std::size_t> initial_tallies;
        std::set<std::shared_ptr<queueable_base>> layer_dependencies;// it is the union of all the tally dependencies
        for (const auto &node: nodes) {
            const auto event_index = node->index();
            auto does_not_exist = queueables_by_index_.find(event_index);
            if (does_not_exist == queueables_by_index_.end()) {
                LOG(scram::ERROR) << "Attempting to build tally for unknown event " << event_index;
                exit(EXIT_FAILURE);
            }
            layer_dependencies.insert(queueables_by_index_[event_index]);

            bitpack_t_ *buffer;
            auto is_not_a_gate = allocated_gates_by_index_.find(event_index);
            if (is_not_a_gate == allocated_gates_by_index_.end()) {
                auto is_not_a_basic_event = allocated_basic_events_by_index_.find(event_index);
                if (is_not_a_basic_event == allocated_basic_events_by_index_.end()) {
                    LOG(scram::ERROR) << "Attempting to build tally for unknown event " << event_index;
                    exit(EXIT_FAILURE);
                } else {
                    // is a basic event
                    buffer = allocated_basic_events_by_index_.at(event_index)->buffer;
                }
            } else {
                buffer = allocated_gates_by_index_.at(event_index)->buffer;
            }
            // store the data for this tally
            node_buffers.push_back(buffer);
            indices.push_back(event_index);
            initial_tallies.push_back(0);
        }

        if (indices.empty()) {
            return nullptr;
        }

        // allocate basic_events object
        const auto num_events_in_layer = indices.size();
        using event_type = tally_event<bitpack_t_>;
        event_type *allocated_tallies = create_tally_events<bitpack_t_>(queue_, node_buffers, initial_tallies);

        // build the kernel
        using kernel_type = kernel::tally<prob_t_, bitpack_t_, size_t_>;
        kernel_type tally_kernel(allocated_tallies, num_events_in_layer, sample_shape_);
        const sycl::range<3> local_limits(1, 0, 0); // first dimension always needs to have just one
        const auto local_range = working_set<size_t_, bitpack_t_>(queue_, num_events_in_layer, sample_shape_).compute_optimal_local_range_3d(local_limits);
        const auto nd_range = tally_kernel.get_range(num_events_in_layer, local_range, sample_shape_);
        iterable_queueable<kernel_type, 3> tally_partition(queue_, tally_kernel, nd_range, layer_dependencies);
        auto queueable_partition = std::make_shared<decltype(tally_partition)>(tally_partition);

        // store each event raw pointer in the identity/index map
        for (auto i = 0; i < num_events_in_layer; i++) {
            const auto event_index_in_pdag = indices[i];
            // store each event raw pointer in the identity/index map
            allocated_tally_events_by_index_[event_index_in_pdag] = allocated_tallies + i;
            // the queueable where this event is computed can be found from the global queueables_by_index_ map
            queueables_by_index_[event_index_in_pdag] = queueable_partition;
        }

        // actually queue this layer for computation
        queueables_.push_back(queueable_partition);

        return queueable_partition;
    }
}