#pragma once

#include "core/node.h"
#include "core/working_set.h"

#include "core/queue/kernel_builder.h"
#include "core/queue/queueable.h"

#include "scram/logger.h"
#include "scram/pdag.h"
#include "scram/preprocessor.h"

#include <sycl/sycl.hpp>

namespace canopy::queue {

    template<typename prob_t_, typename bitpack_t_, typename size_t_>
    class layer_manager {

        using index_t_ = std::int32_t;

        sycl::queue queue_;
        sample_shape<size_t_> sample_shape_;

        std::vector<std::shared_ptr<scram::core::Node>> pdag_nodes_;
        std::unordered_map<index_t_, std::shared_ptr<scram::core::Node>> pdag_nodes_by_index_;
        std::vector<std::vector<std::shared_ptr<scram::core::Node>>> pdag_nodes_by_layer_;

        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> queueables_by_index_;
        std::vector<std::shared_ptr<queueable_base>> queueables_;

        std::unordered_map<index_t_, std::shared_ptr<queueable_base>> tally_queueables_by_index_;

        std::unordered_map<index_t_, tally_event<bitpack_t_> *> allocated_tally_events_by_index_;
        std::unordered_map<index_t_, basic_event<prob_t_, bitpack_t_> *> allocated_basic_events_by_index_;
        std::unordered_map<index_t_, gate<bitpack_t_, size_t_> *> allocated_gates_by_index_;

        std::unordered_map<index_t_, size_t_> accumulated_counts_by_index_;

        static void gather_all_nodes(const std::shared_ptr<scram::core::Gate> &gate, std::vector<std::shared_ptr<scram::core::Node>> &nodes, std::unordered_map<std::int32_t, std::shared_ptr<scram::core::Node>> &nodes_by_index) {
            if (gate->Visited())
                return;
            gate->Visit(1);
            nodes.push_back(gate);
            if (nodes_by_index.contains(gate->index())) {
                LOG(scram::ERROR) << "Found gate with duplicate index while gathering all nodes";
                throw std::runtime_error("gather all nodes failed");
            }
            nodes_by_index[gate->index()] = gate;
            for (const auto &arg: gate->args<scram::core::Gate>()) {
                gather_all_nodes(arg.second, nodes, nodes_by_index);
            }
            for (const auto &arg: gate->args<scram::core::Variable>()) {
                if (!arg.second->Visited()) {
                    arg.second->Visit(1);
                    nodes.push_back(arg.second);
                    if (nodes_by_index.contains(arg.second->index())) {
                        LOG(scram::ERROR) << "Found basic event with duplicate index while gathering all nodes";
                        throw std::runtime_error("gather all nodes failed");
                    }
                    nodes_by_index[arg.second->index()] = arg.second;
                }
            }
        }

        static void layered_toposort(scram::core::Pdag *pdag, std::vector<std::shared_ptr<scram::core::Node>> &nodes, std::unordered_map<index_t_, std::shared_ptr<scram::core::Node>> &nodes_by_index, std::vector<std::vector<std::shared_ptr<scram::core::Node>>> &nodes_by_layer) {
            // Ensure the graph has been topologically sorted, by layer/level
            scram::core::pdag::LayeredTopologicalOrder(pdag);
            // TODO:: Add preprocessing rule for normalizing gates by input count

            // Clear visits for the gathering process
            pdag->Clear<scram::core::Pdag::kVisit>();

            // Collect all nodes
            gather_all_nodes(pdag->root_ptr(), nodes, nodes_by_index);

            // Sort nodes by their order
            std::sort(nodes.begin(), nodes.end(), [](const std::shared_ptr<scram::core::Node> &a, const std::shared_ptr<scram::core::Node> &b) {
                return a->order() < b->order();
            });

            size_t max_layer = nodes.back()->order();// Since nodes are sorted
            nodes_by_layer.resize(max_layer + 1);

            for (auto &node: nodes) {
                nodes_by_layer[node->order()].push_back(node);
            }

            // For each layer, sort so that variables precede gates, and gates are sorted by their Gate::type()
            for (auto &layer: nodes_by_layer) {
                std::sort(layer.begin(), layer.end(), [](const std::shared_ptr<scram::core::Node> &lhs, const std::shared_ptr<scram::core::Node> &rhs) {
                    // Try casting to Variable
                    auto varL = std::dynamic_pointer_cast<scram::core::Variable>(lhs);
                    auto varR = std::dynamic_pointer_cast<scram::core::Variable>(rhs);

                    // If one is a variable and the other is not, variable goes first
                    if (varL && !varR) return true;
                    if (!varL && varR) return false;

                    // If both are variables, treat them as equivalent in this ordering
                    // (no further ordering required among variables)
                    if (varL && varR) return false;

                    // Otherwise, both must be gates. Compare by gate->type()
                    auto gateL = std::dynamic_pointer_cast<scram::core::Gate>(lhs);
                    auto gateR = std::dynamic_pointer_cast<scram::core::Gate>(rhs);
                    return gateL->type() < gateR->type();
                });
            }
            LOG(scram::DEBUG5) << "num_nodes: " << nodes.size();
            LOG(scram::DEBUG5) << "num_layers: " << nodes_by_layer.size();
        }

        static void gather_layer_nodes(
                const std::vector<std::shared_ptr<scram::core::Node>> &layer_nodes,
                std::vector<std::shared_ptr<scram::core::Variable>> &out_variables,
                std::unordered_map<scram::core::Connective, std::vector<std::shared_ptr<scram::core::Gate>>> &out_gates_by_type) {
            out_variables.clear();
            out_gates_by_type.clear();

            for (auto &node: layer_nodes) {
                // If the node is a Variable, store it
                if (auto var = std::dynamic_pointer_cast<scram::core::Variable>(node)) {
                    out_variables.push_back(var);
                }
                // Else if the node is a Gate, group it by Connective type
                else if (auto gate = std::dynamic_pointer_cast<scram::core::Gate>(node)) {
                    out_gates_by_type[gate->type()].push_back(gate);
                } else {
                    LOG(scram::WARNING) << "gather_layer_nodes: Node "
                                 << node->index()
                                 << " was neither a Variable nor a Gate.";
                }
            }
        }

        void build_kernels_for_layer(const std::vector<std::shared_ptr<scram::core::Node>> &layer_nodes) {
            // Step (1): Partition layer_nodes into Variables and gates_by_type
            std::vector<std::shared_ptr<scram::core::Variable>> variables;
            std::unordered_map<scram::core::Connective, std::vector<std::shared_ptr<scram::core::Gate>>> gates_by_type;
            gather_layer_nodes(layer_nodes, variables, gates_by_type);

            // Step (2): Build a single kernel for all variables in this layer (if any)
            auto be_kernel = build_kernel_for_variables<index_t_, prob_t_, bitpack_t_, size_t_>(variables, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_);
            // We could store or log “be_kernel” if we want direct reference, or just rely
            // on the global queueables_ list.

            // Step (3): Build one kernel per gate->type() in this layer
            auto gate_kernels = build_kernels_for_gates<index_t_, prob_t_, bitpack_t_, size_t_>(gates_by_type, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_);

            // Optionally do something with (be_kernel) and the (gate_kernels) vector.
            // The queueables_ container is updated in each subfunction, so
            // they are already “registered” for execution.
        }

        void map_nodes_by_layer(const std::vector<std::vector<std::shared_ptr<scram::core::Node>>> &nodes_by_layer) {
            for (const auto &nodes_in_layer: nodes_by_layer) {
                build_kernels_for_layer(nodes_in_layer);
                //build_tallies_for_layer<index_t_, prob_t_, bitpack_t_, size_t_>(nodes_in_layer, queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_, allocated_tally_events_by_index_);
            }
            // last layer gets tallied
            build_tallies_for_layer<index_t_, prob_t_, bitpack_t_, size_t_>(nodes_by_layer.back(), queue_, sample_shape_, queueables_, queueables_by_index_, allocated_basic_events_by_index_, allocated_gates_by_index_, allocated_tally_events_by_index_);
        }

        void fetch_all_tallies() {
            submit_all().wait_and_throw();
            for (auto &pair: allocated_tally_events_by_index_) {
                const index_t_ index = pair.first;
                const tally_event<bitpack_t_> *tally = pair.second;
                LOG(scram::DEBUG1) << "tally[" << index << "]["<<pdag_nodes_by_index_[index].get()->index()<<"] :: [std_err] :: [p05, mean, p95] :: " << "[" << tally->std_err << "] :: " << "[" << tally->ci[0] << ", "<< tally->mean << ", " << tally->ci[1] << "]";
            }
        }

    public:
        layer_manager(scram::core::Pdag *pdag, const size_t_ batch_size, const size_t_ bitpacks_per_batch) {
            sample_shape_.batch_size = batch_size;
            sample_shape_.bitpacks_per_batch = bitpacks_per_batch;
            sample_shape_ = working_set<size_t_, bitpack_t_>::rounded(sample_shape_);
            layered_toposort(pdag, pdag_nodes_, pdag_nodes_by_index_, pdag_nodes_by_layer_);
            LOG(scram::DEBUG2) << working_set<size_t_, bitpack_t_>(queue_, pdag_nodes_.size(), sample_shape_);
            map_nodes_by_layer(pdag_nodes_by_layer_);
        }

        sycl::queue submit_all() {
            for (const auto &queueable: queueables_) {
                queueable->submit();
            }
            return queue_;
        }

        tally_event<bitpack_t_> tally(const index_t_ evt_idx, const std::size_t count) {
            tally_event<bitpack_t_> to_tally;
            if (!allocated_tally_events_by_index_.contains(evt_idx)) {
                LOG(scram::ERROR) << "Unable to tally probability for unknown event with index "<< evt_idx;
                return std::move(to_tally);
            }
            LOG(scram::DEBUG1) << "Counting "<< count <<" tallies for event with index " << evt_idx;

            for (auto i = 0; i < count; i++) {
                fetch_all_tallies();
            }
            const tally_event<bitpack_t_>* computed_tally = allocated_tally_events_by_index_[evt_idx];
            to_tally.num_one_bits = computed_tally->num_one_bits;
            to_tally.mean = computed_tally->mean;
            to_tally.std_err = computed_tally->std_err;
            to_tally.ci = computed_tally->ci;
            return to_tally;
        }

        ~layer_manager() {
            // Free allocated basic events
            for (auto &pair: allocated_basic_events_by_index_) {
                basic_event<prob_t_, bitpack_t_> *event = pair.second;
                auto buffer = event->buffer;
                auto probability = event->probability;
                auto index = event->index;
                //destroy_basic_event(queue_, event);
            }

            // Free allocated gates
            for (auto &pair: allocated_gates_by_index_) {
                gate<bitpack_t_, size_t_> *event = pair.second;
                //destroy_gate(queue_, event);
            }

            // Free allocated tally events
            for (auto &pair: allocated_tally_events_by_index_) {
                tally_event<bitpack_t_> *event = pair.second;
                //destroy_tally_event(queue_, event);
            }
        }
    };
}// namespace scram::canopy::queue
