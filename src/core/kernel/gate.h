#pragma once

#include "core/node.h"
#include "scram/pdag.h"

#include <sycl/sycl.hpp>

/**
*
* ────────────────────────────────────────────────────────────────
* A. Split the Per‐Bit Accumulation Across Work‐Items
* ────────────────────────────────────────────────────────────────
* Right now, each (gate_id, batch_id, bitpack_id) work‐item processes all “num_inputs” in a for‐loop.
* This is straightforward but can be slow if “num_inputs” is large.
* Instead, you can distribute that workload across multiple threads in the same work‐group or sub‐group.
* For example:
*   1. Each work‐item processes only a subset of inputs (say, 8 or 16, or whatever chunk size is convenient).
*   2. All work‐items in the group store their partial sum of bits into local memory. Each work‐item has an array of eight counters for the eight bits.
*   3. Perform an intra‐group reduce (e.g., sycl::reduce_over_group or a custom parallel reduction in local memory) to combine partial bit‐counts from all items in that group.
*   4. One thread (e.g., the group leader) writes the final eight counters back to global memory or directly performs the threshold comparison and writes to “g.buffer[index].”
*   This transforms a O(num_inputs) loop per kernel thread into a O(num_inputs / (groupSize)) loop plus a modest overhead for group‐based partial sums. If you expect large “num_inputs,” this can be a major speedup.
*/
namespace canopy::kernel {
    template<scram::core::Connective OpType, typename bitpack_t_, typename size_t_>
    class op {
    protected:
        gate<bitpack_t_, size_t_> *gates_;
        const size_t_ num_gates_;
        const sample_shape<size_t_> sample_shape_;

    public:
        op(gate<bitpack_t_, size_t_> *gates, const size_t_ &num_gates, const sample_shape<size_t_> &sample_shape)
            : gates_(gates),
              num_gates_(num_gates),
              sample_shape_(sample_shape) {}

        // get_range method to return the execution range
        static sycl::nd_range<3> get_range(const size_t_ num_gates,
                                           const sycl::range<3> &local_range,
                                           const sample_shape<size_t_> &sample_shape_) {
            // Compute global range
            auto global_size_x = static_cast<size_t>(num_gates);
            auto global_size_y = static_cast<size_t>(sample_shape_.batch_size);
            auto global_size_z = static_cast<size_t>(sample_shape_.bitpacks_per_batch);

            // Adjust global sizes to be multiples of the corresponding local sizes
            global_size_x = ((global_size_x + local_range[0] - 1) / local_range[0]) * local_range[0];
            global_size_y = ((global_size_y + local_range[1] - 1) / local_range[1]) * local_range[1];
            global_size_z = ((global_size_z + local_range[2] - 1) / local_range[2]) * local_range[2];

            sycl::range<3> global_range(global_size_x, global_size_y, global_size_z);

            return {global_range, local_range};
        }

        // ---------------------------------------------------------------------
        // 1) Initialize the bitpack depending on the base operation
        //    (AND-type ops start with all bits=1, OR/XOR-type ops start with 0)
        // ---------------------------------------------------------------------
        static constexpr bitpack_t_ init_bitpack() {
            return (OpType == scram::core::Connective::kAnd || OpType == scram::core::Connective::kNand) ? ~bitpack_t_(0) : 0;
        }

        void operator()(const sycl::nd_item<3> &item) const {
            const auto gate_id = static_cast<size_t_>(item.get_global_id(0));
            const auto batch_id = static_cast<size_t_>(item.get_global_id(1));
            const auto bitpack_idx = static_cast<size_t_>(item.get_global_id(2));

            // Bounds checking
            if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
                return;
            }

            // Compute the linear index into the buffer
            const size_t_ index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;

            // Get gate
            const auto &g = gates_[gate_id];
            const size_t_ num_inputs = g.num_inputs;
            const size_t_ negations_offset = g.negated_inputs_offset;
            // ---------------------------------------------------------------------
            // 1) Initialize, depending on the base operation
            //    (AND-type ops start with all bits=1, OR/XOR-type ops start with 0)
            // ---------------------------------------------------------------------
            bitpack_t_ result = init_bitpack();

            // ---------------------------------------------------------------------
            // 2) Do the base operation, looping over one word from each input
            // ---------------------------------------------------------------------
            for (size_t_ i = 0; i < negations_offset; ++i) {
                const bitpack_t_ val = g.inputs[i][index];
                if constexpr (OpType == scram::core::Connective::kOr || OpType == scram::core::Connective::kNor) {
                    result |= val;
                } else if constexpr (OpType == scram::core::Connective::kAnd || OpType == scram::core::Connective::kNand) {
                    result &= val;
                } else if constexpr (OpType == scram::core::Connective::kXor)// OpType == scram::core::Connective::kXnor
                {
                    result ^= val;
                } else if constexpr (OpType == scram::core::Connective::kNull || OpType == scram::core::Connective::kNot) {
                    result = val;
                }
            }

            for (size_t_ i = negations_offset; i < num_inputs; ++i) {
                const bitpack_t_ val = ~(g.inputs[i][index]);
                if constexpr (OpType == scram::core::Connective::kOr || OpType == scram::core::Connective::kNor) {
                    result |= val;
                } else if constexpr (OpType == scram::core::Connective::kAnd || OpType == scram::core::Connective::kNand) {
                    result &= val;
                } else if constexpr (OpType == scram::core::Connective::kXor)// OpType == scram::core::Connective::kXnor
                {
                    result ^= val;
                } else if constexpr (OpType == scram::core::Connective::kNull || OpType == scram::core::Connective::kNot) {
                    result = val;
                }
            }

            // ---------------------------------------------------------------------
            // 3) If this is a negated op (NOT, NAND, NOR, XNOR), invert the result
            // ---------------------------------------------------------------------
            if constexpr (OpType == scram::core::Connective::kNand || OpType == scram::core::Connective::kNor || OpType == scram::core::Connective::kNot) {
                result = ~result;
            }

            // 4) Write final result into the gate's output buffer
            g.buffer[index] = result;
        }
    };


    template<typename bitpack_t_, typename size_t_>
    class op<scram::core::Connective::kAtleast, bitpack_t_, size_t_> {
    protected:
        atleast_gate<bitpack_t_, size_t_> *gates_;
        const size_t_ num_gates_;
        const sample_shape<size_t_> sample_shape_;

    public:
        op(atleast_gate<bitpack_t_, size_t_> *gates, const size_t_ &num_gates, const sample_shape<size_t_> &sample_shape)
            : gates_(gates),
              num_gates_(num_gates),
              sample_shape_(sample_shape) {}

        // get_range method to return the execution range
        static sycl::nd_range<3> get_range(const size_t_ num_gates,
                                           const sycl::range<3> &local_range,
                                           const sample_shape<size_t_> &sample_shape_) {
            // Compute global range
            auto global_size_x = static_cast<size_t>(num_gates);
            auto global_size_y = static_cast<size_t>(sample_shape_.batch_size);
            auto global_size_z = static_cast<size_t>(sample_shape_.bitpacks_per_batch);

            // Adjust global sizes to be multiples of the corresponding local sizes
            global_size_x = ((global_size_x + local_range[0] - 1) / local_range[0]) * local_range[0];
            global_size_y = ((global_size_y + local_range[1] - 1) / local_range[1]) * local_range[1];
            global_size_z = ((global_size_z + local_range[2] - 1) / local_range[2]) * local_range[2];

            sycl::range<3> global_range(global_size_x, global_size_y, global_size_z);

            return {global_range, local_range};
        }

        void operator()(const sycl::nd_item<3> &item) const {
            const auto gate_id = static_cast<std::uint32_t>(item.get_global_id(0));
            const auto batch_id = static_cast<std::uint32_t>(item.get_global_id(1));
            const auto bitpack_idx = static_cast<std::uint32_t>(item.get_global_id(2));

            // Bounds checking
            if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
                return;
            }

            // Compute the linear index into the buffer
            const std::uint32_t index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;

            // Get gate
            const auto &g = gates_[gate_id];
            const auto num_inputs = g.num_inputs;
            const auto negations_offset = g.negated_inputs_offset;

            static constexpr bitpack_t_ NUM_BITS = sizeof(bitpack_t_) * 8;
            sycl::marray<bitpack_t_, NUM_BITS> accumulated_counts(0);

            // for each input, accumulate the counts for each bit-position
            for (auto i = 0; i < negations_offset; ++i) {
                const bitpack_t_ val = g.inputs[i][index];
                #pragma unroll
                for (auto idx = 0; idx < NUM_BITS; ++idx) {
                    accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
                }
            }

            for (auto i = negations_offset; i < num_inputs; ++i) {
                const bitpack_t_ val = ~(g.inputs[i][index]);
                #pragma unroll
                for (auto idx = 0; idx < NUM_BITS; ++idx) {
                    accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
                }
            }

            // at_least = 0   -> always one
            // at_least = 1   -> or gate
            // at_least = k   -> k of n
            // at_least = n   -> and gate
            // at_least = n+1 -> always zero
            const auto threshold = g.at_least;

            bitpack_t_ result = 0;

            #pragma unroll
            for (auto idx = 0; idx < NUM_BITS; ++idx) {
                result |= ((accumulated_counts[idx] >= threshold ? 1 : 0) << idx);
            }

            g.buffer[index] = result;
        }
    };
}
            //     const auto gate_id     = static_cast<size_t_>(item.get_global_id(0));
            //     const auto batch_id    = static_cast<size_t_>(item.get_global_id(1));
            //     const auto bitpack_idx = static_cast<size_t_>(item.get_global_id(2));
            //
            //     // Bounds checking
            //     if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
            //         return;
            //     }
            //
            //     // Compute the linear index into the buffer
            //     const size_t_ index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;
            //
            //     // This single gate might have many input buffers to combine ("k-of-n" logic)
            //     const auto& g = gates_[gate_id];
            //     const auto num_inputs = g.num_inputs;
            //     const auto threshold  = g.at_least;
            //
            //
            //     const auto grp        = item.get_group();         // sycl::group<3>
            //     const auto local_id   = item.get_local_linear_id();
            //     const auto group_size = grp.get_local_range().size();
            //
            //     // We step over the inputs in increments of group_size.
            //     // e.g. thread local_id processes i, i+group_size, i+2group_size, ...
            //     // until i >= num_inputs
            //     std::uint8_t private_counts[64];
            //     for(int b = 0; b < 64; b++)
            //         private_counts[b] = 0;
            //
            //     for(std::uint32_t i = local_id; i < num_inputs; i += group_size)
            //     {
            //         // Read input
            //         const std::uint64_t val = g.inputs[i][index];
            //         // Accumulate bits
            //         for(int b = 0; b < 64; b++) {
            //             private_counts[b] += (val >> b) & 1ULL;
            //         }
            //     }
            //
            //     // Now use sycl::reduce_over_group() to sum each bit's counts across the group
            //     for(int b = 0; b < 64; b++) {
            //         private_counts[b] = sycl::reduce_over_group(grp, private_counts[b], sycl::plus<>());
            //     }
            //     // Only one thread in each work-group should do the threshold check + write
            //     if(item.get_local_linear_id() == 0)
            //     {
            //         std::uint64_t final_result = 0ULL;
            //         for(int b = 0; b < 64; b++) {
            //             if(private_counts[b] >= threshold) {
            //                 final_result |= (1ULL << b);
            //             }
            //         }
            //         g.buffer[index] = final_result;
            //     }
            // }
        // };
        //
        // template<>
        // inline void op<scram::core::Connective::kAtleast, std::uint8_t, std::uint32_t>::operator()(const sycl::nd_item<3> &item) const {
        //     const auto gate_id     = static_cast<std::uint32_t>(item.get_global_id(0));
        //     const auto batch_id    = static_cast<std::uint32_t>(item.get_global_id(1));
        //     const auto bitpack_idx = static_cast<std::uint32_t>(item.get_global_id(2));
        //
        //     // Bounds checking
        //     if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
        //         return;
        //     }
        //
        //     // Compute the linear index into the buffer
        //     const std::uint32_t index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;
        //
        //     // Get gate
        //     const auto& g = gates_[gate_id];
        //     const auto num_inputs = g.num_inputs;
        //     const auto negations_offset = g.negated_inputs_offset;
        //     //sycl::marray<std::uint8_t, 8> accumulated_counts = {0, 0, 0, 0, 0, 0, 0, 0};
        //     sycl::uchar8 accumulated_counts = {0, 0, 0, 0, 0, 0, 0, 0};
        //
        //     // for each input, accumulate the counts for each bit-position
        //     for (auto i = 0; i < negations_offset; ++i) {
        //
        //         const std::uint8_t val = g.inputs[i][index];
        //
        //         accumulated_counts[0] += (val & 0b00000001 ? 1 : 0);
        //         accumulated_counts[1] += (val & 0b00000010 ? 1 : 0);
        //         accumulated_counts[2] += (val & 0b00000100 ? 1 : 0);
        //         accumulated_counts[3] += (val & 0b00001000 ? 1 : 0);
        //         accumulated_counts[4] += (val & 0b00010000 ? 1 : 0);
        //         accumulated_counts[5] += (val & 0b00100000 ? 1 : 0);
        //         accumulated_counts[6] += (val & 0b01000000 ? 1 : 0);
        //         accumulated_counts[7] += (val & 0b10000000 ? 1 : 0);
        //     }
        //
        //     for (auto i = negations_offset; i < num_inputs; ++i) {
        //
        //         const std::uint8_t val = ~(g.inputs[i][index]);
        //
        //         accumulated_counts[0] += (val & 0b00000001 ? 1 : 0);
        //         accumulated_counts[1] += (val & 0b00000010 ? 1 : 0);
        //         accumulated_counts[2] += (val & 0b00000100 ? 1 : 0);
        //         accumulated_counts[3] += (val & 0b00001000 ? 1 : 0);
        //         accumulated_counts[4] += (val & 0b00010000 ? 1 : 0);
        //         accumulated_counts[5] += (val & 0b00100000 ? 1 : 0);
        //         accumulated_counts[6] += (val & 0b01000000 ? 1 : 0);
        //         accumulated_counts[7] += (val & 0b10000000 ? 1 : 0);
        //     }
        //
        //     // at_least = 0   -> always one
        //     // at_least = 1   -> or gate
        //     // at_least = k   -> k of n
        //     // at_least = n   -> and gate
        //     // at_least = n+1 -> always zero
        //     const auto threshold = g.at_least;
        //
        //     std::uint8_t result = accumulated_counts[0] >= threshold ? 1 : 0;
        //     result |= (accumulated_counts[1] >= threshold ? 1 : 0) << 1;
        //     result |= (accumulated_counts[2] >= threshold ? 1 : 0) << 2;
        //     result |= (accumulated_counts[3] >= threshold ? 1 : 0) << 3;
        //     result |= (accumulated_counts[4] >= threshold ? 1 : 0) << 4;
        //     result |= (accumulated_counts[5] >= threshold ? 1 : 0) << 5;
        //     result |= (accumulated_counts[6] >= threshold ? 1 : 0) << 6;
        //     result |= (accumulated_counts[7] >= threshold ? 1 : 0) << 7;
        //
        //     g.buffer[index] = result;
        // }

        // template<>
        // inline void op<scram::core::Connective::kAtleast, std::uint8_t, std::uint32_t>::operator()(const sycl::nd_item<3> &item) const {
        //     const auto gate_id     = static_cast<std::uint32_t>(item.get_global_id(0));
        //     const auto batch_id    = static_cast<std::uint32_t>(item.get_global_id(1));
        //     const auto bitpack_idx = static_cast<std::uint32_t>(item.get_global_id(2));
        //
        //     // Bounds checking
        //     if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
        //         return;
        //     }
        //
        //     // Compute the linear index into the buffer
        //     const std::uint32_t index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;
        //
        //     // Get gate
        //     const auto& g = gates_[gate_id];
        //     const auto num_inputs = g.num_inputs;
        //     const auto negations_offset = g.negated_inputs_offset;
        //
        //     using bitpack_t_ = std::uint8_t;
        //     static constexpr bitpack_t_ NUM_BITS = 8;
        //     sycl::marray<bitpack_t_, NUM_BITS> accumulated_counts(0);
        //
        //     // for each input, accumulate the counts for each bit-position
        //     for (auto i = 0; i < negations_offset; ++i) {
        //         const bitpack_t_ val = g.inputs[i][index];
        //         #pragma unroll
        //         for (auto idx = 0; idx < NUM_BITS; ++idx) {
        //             accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
        //         }
        //     }
        //
        //     for (auto i = negations_offset; i < num_inputs; ++i) {
        //         const bitpack_t_ val = ~(g.inputs[i][index]);
        //         #pragma unroll
        //         for (auto idx = 0; idx < NUM_BITS; ++idx) {
        //             accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
        //         }
        //     }
        //
        //     // at_least = 0   -> always one
        //     // at_least = 1   -> or gate
        //     // at_least = k   -> k of n
        //     // at_least = n   -> and gate
        //     // at_least = n+1 -> always zero
        //     const auto threshold = g.at_least;
        //
        //     bitpack_t_ result = 0;
        //
        //     #pragma unroll
        //     for (auto idx = 0; idx < NUM_BITS; ++idx) {
        //         result |= ((accumulated_counts[idx] >= threshold ? 1 : 0) << idx);
        //     }
        //
        //     g.buffer[index] = result;
        // }
        //
        // template<>
        // inline void op<scram::core::Connective::kAtleast, std::uint64_t, std::uint32_t>::operator()(const sycl::nd_item<3> &item) const {
        //     const auto gate_id     = static_cast<std::uint32_t>(item.get_global_id(0));
        //     const auto batch_id    = static_cast<std::uint32_t>(item.get_global_id(1));
        //     const auto bitpack_idx = static_cast<std::uint32_t>(item.get_global_id(2));
        //
        //     // Bounds checking
        //     if (gate_id >= this->num_gates_ || batch_id >= this->sample_shape_.batch_size || bitpack_idx >= this->sample_shape_.bitpacks_per_batch) {
        //         return;
        //     }
        //
        //     // Compute the linear index into the buffer
        //     const std::uint32_t index = batch_id * sample_shape_.bitpacks_per_batch + bitpack_idx;
        //
        //     // Get gate
        //     const auto& g = gates_[gate_id];
        //     const auto num_inputs = g.num_inputs;
        //     const auto negations_offset = g.negated_inputs_offset;
        //
        //     sycl::marray<std::uint8_t, 64> accumulated_counts(0);
        //
        //     // for each input, accumulate the counts for each bit-position
        //     for (auto i = 0; i < negations_offset; ++i) {
        //         const std::uint64_t val = g.inputs[i][index];
        //         #pragma unroll
        //         for (auto idx = 0; idx < 64; ++idx) {
        //             accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
        //         }
        //     }
        //
        //     for (auto i = negations_offset; i < num_inputs; ++i) {
        //         const std::uint64_t val = ~(g.inputs[i][index]);
        //         #pragma unroll
        //         for (auto idx = 0; idx < 64; ++idx) {
        //             accumulated_counts[idx] += (val & (1 << i) ? 1 : 0);
        //         }
        //     }
        //
        //     // at_least = 0   -> always one
        //     // at_least = 1   -> or gate
        //     // at_least = k   -> k of n
        //     // at_least = n   -> and gate
        //     // at_least = n+1 -> always zero
        //     const auto threshold = g.at_least;
        //
        //     std::uint64_t result = 0;
        //
        //     #pragma unroll
        //     for (auto idx = 0; idx < 64; ++idx) {
        //         result |= ((accumulated_counts[idx] >= threshold ? 1 : 0) << idx);
        //     }
        //
        //     g.buffer[index] = result;
        // }
    //}
    //
    // // We will accumulate partial sums for each bit in a 64-bit block:
    // // partial_counters[b] (for b=0..63)
    // // Then reduce them across the group.
    // // We'll store each thread’s partial sums in a private array,
    // // then do a group-level reduction in local memory.
    //
    // // 1) Each thread accumulates partial sums for a subset of the inputs:
    // //    We'll divide "num_inputs" among all threads in this group.
    // // ------------------------------------------------------------------------
    // const size_t group_size = item.get_local_range(0) *
    //                           item.get_local_range(1) *
    //                           item.get_local_range(2);
    // const size_t local_id   = item.get_local_linear_id();
    //
    // // Compute chunk bounds
    // const size_t chunk      = (num_inputs + group_size - 1) / group_size;
    // const size_t start      = local_id * chunk;
    // const size_t end        = sycl::min(start + chunk, static_cast<size_t>(num_inputs));
    //
    // // Private counters for each bit of a 64-bit pack
    // // Using 32-bit or 16-bit counters depends on max num_inputs
    // // Here, 32-bit is safer if num_inputs can be large
    // std::uint8_t local_counters[64];
    // for (int b = 0; b < 64; b++) {
    //     local_counters[b] = 0;
    // }
    //
    // // Accumulate partial sums
    // for (size_t i = start; i < end; ++i)
    // {
    //     // Read one 64-bit word from input i
    //     const std::uint64_t val = g.inputs[i][index];
    //
    //     // For each bit, add 1 if set
    //     // A common optimization is to enumerate set bits, but we’ll keep it direct:
    //     for (int b = 0; b < 64; b++) {
    //         local_counters[b] += static_cast<std::uint8_t>((val >> b) & 1ULL);
    //     }
    // }
    //
    // // 2) Store partial sums in local memory, then reduce them across the group
    // // ------------------------------------------------------------------------
    // // We'll store each thread’s 64 counters in local memory at an offset
    // // of local_id*64. Then do a parallel reduction.
    // sycl::group<3>  grp = item.get_group();
    // sycl::range<3>  lrange = item.get_local_range();
    //
    // // A 2D local_accessor: [group_size, 64]
    // //   local_sums[lid][bit_index]
    // sycl::local_accessor<std::uint32_t, 2> local_sums(sycl::range<2>(group_size, 64), grp);
    //
    // // Write private counters to local memory
    // for (int b = 0; b < 64; b++) {
    //     local_sums[local_id][b] = local_counters[b];
    // }
    // item.barrier(sycl::access::fence_space::local_space);
    //
    // // Parallel reduction in local memory.
    // // Double‐tree approach: stride = group_size/2 down to 1
    // // At each step, only threads with local_id < stride add in values from local_id+stride
    // for (size_t stride = group_size / 2; stride > 0; stride /= 2)
    // {
    //     if (local_id < stride)
    //     {
    //         for (int b = 0; b < 64; b++)
    //         {
    //             local_sums[local_id][b] += local_sums[local_id + stride][b];
    //         }
    //     }
    //     // Barrier after each pass
    //     item.barrier(sycl::access::fence_space::local_space);
    // }
    //
    // // Now local_sums[0][b] holds the sum for all threads in group for bit b.
    // // 3) Compare sums to threshold, produce final 64-bit mask
    // //    We do this for exactly one thread (group leader, local_id=0).
    // if (local_id == 0)
    // {
    //     std::uint64_t final_mask = 0ULL;
    //     for (int b = 0; b < 64; b++)
    //     {
    //         if (local_sums[0][b] >= threshold) {
    //             final_mask |= (1ULL << b);
    //         }
    //     }
    //     g.buffer[index] = final_mask;
    // }
