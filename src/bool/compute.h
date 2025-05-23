#ifndef CANOPY_COMPUTE_H
#define CANOPY_COMPUTE_H
#include "working_set.h"

#include <CL/sycl.hpp>
#include <cstddef>

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

namespace canopy {

template <typename bit_vector_type = std::uint_fast8_t, typename size_type = std::size_t>
size_type eval2(cl::sycl::buffer<bit_vector_type, 1>& F_buf,
               cl::sycl::buffer<bit_vector_type, 1>& sampled_x_buf,
               cl::sycl::queue& queue) {
    // Determine the sizes of F and sampled_x
    const size_type F_size = F_buf.get_count();
    const size_type num_samples = sampled_x_buf.get_count();

    // Allocate device USM memory for F, sampled_x, and result
    bit_vector_type* d_F = cl::sycl::malloc_device<bit_vector_type>(F_size, queue);
    bit_vector_type* d_sampled_x = cl::sycl::malloc_device<bit_vector_type>(num_samples, queue);
    long* d_result = cl::sycl::malloc_device<long>(1, queue);

    // Initialize result to zero on the device
    long zero = 0;
    queue.memcpy(d_result, &zero, sizeof(long));

    // Copy data from host buffers to device USM memory
    {
        // Access host data from buffers
        auto F_host = F_buf.template get_access<cl::sycl::access::mode::read>();
        auto sampled_x_host = sampled_x_buf.template get_access<cl::sycl::access::mode::read>();

        // Copy data to device
        queue.memcpy(d_F, F_host.get_pointer(), F_size * sizeof(bit_vector_type));
        queue.memcpy(d_sampled_x, sampled_x_host.get_pointer(), num_samples * sizeof(bit_vector_type));
    }

    // Wait for the data transfer to complete
    queue.wait();

    // Define the kernel execution
    auto cgf = [&](cl::sycl::handler& cgh) {
        // Compute the working set splits
        const auto splits = working_set<>::compute(queue, F_size, num_samples);

        // Allocate local memory within the kernel
        cl::sycl::local_accessor<bit_vector_type, 1> local_F(splits.F_per_group, cgh);
        cl::sycl::local_accessor<long, 1> local_sums(cl::sycl::range<1>(1), cgh);

        // Define the execution range
        const auto globalSize = cl::sycl::range<1>(splits.global_range);
        const auto localSize = cl::sycl::range<1>(splits.work_group_size);
        const auto executionRange = cl::sycl::nd_range<1>(globalSize, localSize);

        // Launch the kernel
        cgh.parallel_for<class sample_eval_kernel>(
            executionRange,
            [=](cl::sycl::nd_item<1> item) {
                const size_t global_id = item.get_global_linear_id();
                const size_t local_id = item.get_local_linear_id();
                const size_t group_id = item.get_group_linear_id();

                // Each work-group loads its portion of F into local memory
                const size_t F_fragment_start = group_id * splits.F_per_group;
                const size_t F_fragment_end = cl::sycl::min(F_fragment_start + splits.F_per_group, F_size);
                const size_t F_fragment_size = F_fragment_end - F_fragment_start;

                // Load F into local memory in parallel
                for (size_t i = local_id; i < F_fragment_size; i += splits.work_group_size) {
                    local_F[i] = d_F[F_fragment_start + i];
                }

                // Synchronize to ensure all local_F is loaded
                item.barrier(cl::sycl::access::fence_space::local_space);

                // Each work-item processes multiple samples
                size_t sample_start = global_id * splits.samples_per_work_item;
                size_t sample_end = cl::sycl::min(sample_start + splits.samples_per_work_item, num_samples);

                long local_count = 0;

                for (size_t i = sample_start; i < sample_end; ++i) {
                    const auto sample = d_sampled_x[i];
                    auto sample_satisfies_F = 0;

                    // Evaluate the sample over this fragment of F
                    for (size_t j = 0; j < F_fragment_size; ++j) {
                        const bool all_true = (sample | local_F[j]) == 0b11111111;
                        sample_satisfies_F = sample_satisfies_F || all_true;
                    }
                    // Increment local count if sample satisfies F
                    local_count += sample_satisfies_F;
                }

                // Reduce local counts within the work-group
                if (local_id == 0) {
                    local_sums[0] = 0;
                }

                item.barrier(cl::sycl::access::fence_space::local_space);

                // Atomic add local count to local sum
                cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed,
                                     cl::sycl::memory_scope::work_group,
                                     cl::sycl::access::address_space::local_space>
                    local_sum_atomic(local_sums[0]);
                local_sum_atomic.fetch_add(local_count);

                item.barrier(cl::sycl::access::fence_space::local_space);

                // Work-group leader updates the global result
                if (local_id == 0) {
                    cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed,
                                         cl::sycl::memory_scope::device,
                                         cl::sycl::access::address_space::global_space>
                        result_atomic(*d_result);
                    result_atomic.fetch_add(local_sums[0]);
                }
            });
    };

    // Submit the kernel to the queue
    queue.submit(cgf);
    queue.wait();

    // Retrieve the result from the device
    size_type count = 0;
    queue.memcpy(&count, d_result, sizeof(long)).wait();

    // Free device USM memory
    cl::sycl::free(d_F, queue);
    cl::sycl::free(d_sampled_x, queue);
    cl::sycl::free(d_result, queue);

    return count;
}

template <template <typename, typename...> class Container = std::vector,
          typename bit_vector_type = uint_fast8_t, typename... Args>
std::size_t eval2(Container<bit_vector_type, Args...>& F,
                 Container<bit_vector_type, Args...>& sampled_x,
                 cl::sycl::queue& queue) {
    // Determine the sizes
    const std::size_t F_size = F.size();
    const std::size_t num_samples = sampled_x.size();

    // Allocate device USM memory for F and sampled_x
    bit_vector_type* d_F = cl::sycl::malloc_shared<bit_vector_type>(F_size, queue);
    bit_vector_type* d_sampled_x = cl::sycl::malloc_shared<bit_vector_type>(num_samples, queue);

    // Allocate device USM memory for result
    long* d_result = cl::sycl::malloc_shared<long>(1, queue);

    // Initialize result to zero on the device
    long zero = 0;
    queue.memcpy(d_result, &zero, sizeof(long));

    // Copy data from host containers to device USM memory
    queue.memcpy(d_F, F.data(), F_size * sizeof(bit_vector_type));
    queue.memcpy(d_sampled_x, sampled_x.data(), num_samples * sizeof(bit_vector_type));

    // Wait for the data transfer to complete
    queue.wait();

    // Define the kernel execution (same as before but directly using USM pointers)
    auto cgf = [&](cl::sycl::handler& cgh) {
        const auto splits = working_set<>::compute(queue, F_size, num_samples);
        cl::sycl::local_accessor<bit_vector_type, 1> local_F(splits.F_per_group, cgh);
        cl::sycl::local_accessor<long, 1> local_sums(cl::sycl::range<1>(1), cgh);
        const auto globalSize = cl::sycl::range<1>(splits.global_range);
        const auto localSize = cl::sycl::range<1>(splits.work_group_size);
        const auto executionRange = cl::sycl::nd_range<1>(globalSize, localSize);

        cgh.parallel_for<class sample_eval_kernel>(
            executionRange,
            [=](cl::sycl::nd_item<1> item) {
                const size_t global_id = item.get_global_linear_id();
                const size_t local_id = item.get_local_linear_id();
                const size_t group_id = item.get_group_linear_id();

                // Each work-group loads its portion of F into local memory
                const size_t F_fragment_start = group_id * splits.F_per_group;
                const size_t F_fragment_end = cl::sycl::min(F_fragment_start + splits.F_per_group, F_size);
                const size_t F_fragment_size = F_fragment_end - F_fragment_start;

                // Load F into local memory in parallel
                for (size_t i = local_id; i < F_fragment_size; i += splits.work_group_size) {
                    local_F[i] = d_F[F_fragment_start + i];
                }

                // Synchronize to ensure all local_F is loaded
                item.barrier(cl::sycl::access::fence_space::local_space);

                // Each work-item processes multiple samples
                size_t sample_start = global_id * splits.samples_per_work_item;
                size_t sample_end = cl::sycl::min(sample_start + splits.samples_per_work_item, num_samples);

                long local_count = 0;

                for (size_t i = sample_start; i < sample_end; ++i) {
                    const auto sample = d_sampled_x[i];
                    auto sample_satisfies_F = 0;

                    // Evaluate the sample over this fragment of F
                    for (size_t j = 0; j < F_fragment_size; ++j) {
                        const bool all_true = (sample | local_F[j]) == 0b11111111;
                        sample_satisfies_F = sample_satisfies_F || all_true;
                    }
                    // Increment local count if sample satisfies F
                    local_count += sample_satisfies_F;
                }

                // Reduce local counts within the work-group
                if (local_id == 0) {
                    local_sums[0] = 0;
                }

                item.barrier(cl::sycl::access::fence_space::local_space);

                // Atomic add local count to local sum
                cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed,
                                     cl::sycl::memory_scope::work_group,
                                     cl::sycl::access::address_space::local_space>
                    local_sum_atomic(local_sums[0]);
                local_sum_atomic.fetch_add(local_count);

                item.barrier(cl::sycl::access::fence_space::local_space);

                // Work-group leader updates the global result
                if (local_id == 0) {
                    cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed,
                                         cl::sycl::memory_scope::device,
                                         cl::sycl::access::address_space::global_space>
                        result_atomic(*d_result);
                    result_atomic.fetch_add(local_sums[0]);
                }
            });
    };

    // Submit the kernel to the queue
    queue.submit(cgf);
    queue.wait();

    // Retrieve the result from the device
    std::size_t count = 0;
    queue.memcpy(&count, d_result, sizeof(long)).wait();

    // Free device USM memory
    cl::sycl::free(d_F, queue);
    cl::sycl::free(d_sampled_x, queue);
    cl::sycl::free(d_result, queue);

    return count;
}

template <typename bit_vector_type = std::uint_fast8_t, typename size_type = std::size_t>
size_type eval(cl::sycl::buffer<bit_vector_type, 1> &F_buf, cl::sycl::buffer<bit_vector_type, 1> &sampled_x_buf,
               cl::sycl::queue &queue) {

    cl::sycl::buffer<long, 1> result_buf(cl::sycl::range<1>(1));
    // Initialize result to zero
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::write>();
        acc[0] = 0;
    }
    auto cgf = [&](cl::sycl::handler &cgh) {
        // compute the best working set split sizes
        const auto F_size = F_buf.size();
        const auto num_samples = sampled_x_buf.size();
        const auto splits = working_set<>::compute(queue, F_size, num_samples);

        auto F_acc = F_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto sampled_x_acc = sampled_x_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto result_acc = result_buf.template get_access<cl::sycl::access::mode::read_write>(cgh);
        // Local memory for storing a chunk of F
        cl::sycl::local_accessor<bit_vector_type, 1> local_F(splits.F_per_group, cgh);
        cl::sycl::local_accessor<long, 1> local_sums(cl::sycl::range<1>(1), cgh);

        // todo:: use sub-group for warp-level SIMD parallelism

        // todo:: handle case when num_samples is large enough that all the data does not sit in memory
        // todo:: implement batching
        const auto globalSize = cl::sycl::range<1>(splits.global_range);
        const auto localSize = cl::sycl::range<1>(splits.work_group_size);
        const auto executionRange = cl::sycl::nd_range<1>(globalSize, localSize);

        cgh.parallel_for<class sample_eval_kernel>(executionRange, [=](cl::sycl::nd_item<1> item) {
            const size_t global_id = item.get_global_linear_id();
            const size_t local_id = item.get_local_linear_id();
            const size_t group_id = item.get_group_linear_id();

            // Each work-group loads its portion of F into local memory
            const size_t F_fragment_start = group_id * splits.F_per_group;
            const size_t F_fragment_end = cl::sycl::min(F_fragment_start + splits.F_per_group, F_size);
            const size_t F_fragment_size = F_fragment_end - F_fragment_start;

            // Load F into local memory in a parallel fashion
            for (size_t i = local_id; i < F_fragment_size; i += splits.work_group_size) {
                local_F[i] = F_acc[F_fragment_start + i];
                //local_sums[0]
            }

            // Synchronize to ensure all local_F is loaded
            item.barrier(cl::sycl::access::fence_space::local_space);

            // Each work-item processes multiple samples
            size_t sample_start = global_id * splits.samples_per_work_item;
            size_t sample_end = cl::sycl::min(sample_start + splits.samples_per_work_item, num_samples);

            long local_count = 0;

            for (size_t i = sample_start; i < sample_end; ++i) {
                //if (i >= num_samples) break; // Guard against overrun

                const auto sample = sampled_x_acc[i];
                auto sample_satisfies_F = 0;

                // Evaluate the same sample over this chunk/fragment of F
                // if any product in this fragment of F evaluates to true, record that and skip over the remaining fragments
                for (size_t j = 0; j < F_fragment_size; ++j) {
                    const bool all_true = (sample | local_F[j]) == 0b11111111;
                    sample_satisfies_F = sample_satisfies_F || all_true;
                }
                // keep a count for when any sample makes this fragment of F true
                local_count += sample_satisfies_F;
            }

            // Reduce local counts within the work-group
            // Use local memory for reduction
            // Initialize local sum
            if (local_id == 0) {
                local_sums[0] = 0;
            }

            item.barrier(cl::sycl::access::fence_space::local_space);

            // Atomic add local count to local sum
            cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::work_group,
                                 cl::sycl::access::address_space::local_space>
            local_sum_atomic(local_sums[0]);
            local_sum_atomic.fetch_add(local_count);

            item.barrier(cl::sycl::access::fence_space::local_space);

            // Work-group leader updates the global result
            if (local_id == 0) {
                cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::device,
                                     cl::sycl::access::address_space::global_space>
                result_atomic(result_acc[0]);
                result_atomic.fetch_add(local_sums[0]);
            }
        });
    };
    queue.submit(cgf);
    queue.wait_and_throw();

    // Retrieve result
    size_type count = 0;
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::read>();
        count = static_cast<size_type>(acc[0]);
    }

    return count;
}

template <typename bit_vector_type = std::uint_fast8_t, typename size_type = std::size_t>
size_type eval_naive(cl::sycl::buffer<bit_vector_type, 1> &F_buf, cl::sycl::buffer<bit_vector_type, 1> &sampled_x_buf,
               cl::sycl::queue &queue) {

    cl::sycl::buffer<long, 1> result_buf(cl::sycl::range<1>(1));
    // Initialize result to zero
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::write>();
        acc[0] = 0;
    }
    queue.submit([&](cl::sycl::handler& cgh) {
        auto F_acc = F_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto sampled_x_acc = sampled_x_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto result_acc = result_buf.template get_access<cl::sycl::access::mode::read_write>(cgh);
        const auto numWorkItems = cl::sycl::range<1>(sampled_x_buf.size());
        const auto num_products = F_buf.size();
        cgh.parallel_for<class sample_eval_kernel>(cl::sycl::range<1>(numWorkItems), [=](cl::sycl::id<1> idx) {
            const size_t i = idx[0];
            const auto sample = sampled_x_acc[i];
            auto sample_satisfies_F = 0;

            // Evaluate the same sample over this chunk/fragment of F
            // if any product in this fragment of F evaluates to true, record that and skip over the remaining fragments
            for (size_t j = 0; j < num_products; ++j) {
                const bool all_true = (sample | F_acc[j]) == 0b11111111;
                sample_satisfies_F = sample_satisfies_F || all_true;
            }
            if (sample_satisfies_F) {
                cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::device,
                                     cl::sycl::access::address_space::global_space>
                result_atomic(result_acc[0]);
                result_atomic.fetch_add(static_cast<long>(1));
            }
        });
    });
    queue.wait_and_throw();

    // Retrieve result
    size_type count = 0;
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::read>();
        count = static_cast<size_type>(acc[0]);
    }

    return count;
}

template <typename bit_vector_type = std::uint_fast8_t, typename size_type = std::size_t>
size_type eval_naive_2D(cl::sycl::buffer<bit_vector_type, 2> &F_buf, cl::sycl::buffer<bit_vector_type, 2> &sampled_x_buf,
               cl::sycl::queue &queue) {

    cl::sycl::buffer<long, 1> result_buf(cl::sycl::range<1>(1));
    // Initialize result to zero
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::write>();
        acc[0] = 0;
    }
    queue.submit([&](cl::sycl::handler& cgh) {
        auto F_acc = F_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto sampled_x_acc = sampled_x_buf.template get_access<cl::sycl::access::mode::read>(cgh);
        auto result_acc = result_buf.template get_access<cl::sycl::access::mode::read_write>(cgh);
        const auto numWorkItems = cl::sycl::range<2>(sampled_x_buf.size());
        const auto num_products = F_buf.size();
        cgh.parallel_for<class sample_eval_kernel>(cl::sycl::range<2>(numWorkItems), [=](cl::sycl::id<2> idx) {
            const size_t i = idx[0];
            const auto sample = sampled_x_acc[i];
            auto sample_satisfies_F = 0;

            // Evaluate the same sample over this chunk/fragment of F
            // if any product in this fragment of F evaluates to true, record that and skip over the remaining fragments
            for (size_t j = 0; j < num_products; ++j) {
                const bool all_true = (sample | F_acc[j]) == 0b11111111;
                sample_satisfies_F = sample_satisfies_F || all_true;
            }
            if (sample_satisfies_F) {
                cl::sycl::atomic_ref<long, cl::sycl::memory_order::relaxed, cl::sycl::memory_scope::device,
                                     cl::sycl::access::address_space::global_space>
                result_atomic(result_acc[0]);
                result_atomic.fetch_add(static_cast<long>(1));
            }
        });
    });
    queue.wait_and_throw();

    // Retrieve result
    size_type count = 0;
    {
        auto acc = result_buf.get_access<cl::sycl::access::mode::read>();
        count = static_cast<size_type>(acc[0]);
    }

    return count;
}

template <template <typename, typename...> class Container = std::vector, typename bit_vector_type = uint_fast8_t, typename... Args>
std::size_t eval(Container<bit_vector_type, Args...>& F, Container<bit_vector_type, Args...>& sampled_x, cl::sycl::queue &queue) {
    // https://www.intel.com/content/www/us/en/docs/oneapi/optimization-guide-gpu/2024-2/sub-groups-and-simd-vectorization.html#DATA-SHARING
    cl::sycl::buffer<bit_vector_type, 1> F_buf(F.data(), cl::sycl::range<1>(F.size()));
    cl::sycl::buffer<bit_vector_type, 1> sampled_x_buf(sampled_x.data(), cl::sycl::range<1>(sampled_x.size()));
    return eval(F_buf, sampled_x_buf, queue);
}


} // namespace canopy
#endif //CANOPY_COMPUTE_H
