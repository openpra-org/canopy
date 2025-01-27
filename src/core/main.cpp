#include "kernel/basic_event.h"
#include "working_set.h"

#include <sycl/sycl.hpp>

int impl2() {
    using prob_type = std::float_t;
    using bitpack_type = uint8_t;
    using size_type = std::uint32_t;

    // Define problem size
    constexpr size_type num_basic_events = 2 << 14;

    sycl::queue queue;

    // Allocate Shared USM array of canopy::BasicEvent
    auto *basic_events = sycl::malloc_shared<canopy::basic_event<prob_type, bitpack_type>>(num_basic_events, queue);

    auto sample_shape = canopy::sample_shape<size_type>{
            .batch_size = 4,
            .bitpacks_per_batch = 2 << 14,
    };

    // Initialize probabilities and allocate per-basic-event output buffers
    for (auto i = 0; i < num_basic_events; ++i) {
        // Set probability
        basic_events[i].probability = 0.5f;// Example probability

        // Allocate Device USM buffer for node outputs
        const size_t total_samples = sample_shape.batch_size * sample_shape.bitpacks_per_batch;
        basic_events[i].buffer = sycl::malloc_device<bitpack_type>(total_samples, queue);
    }

    //const auto work_set = canopy::working_set<size_type, bitpack_type>::compute(queue, num_basic_events, sample_shape.batch_size * sample_shape.bitpacks_per_batch);

    // Launch the kernel, batching all basic events at once
    queue.submit([&](sycl::handler &cgh) {

        // Instantiate the kernel
        canopy::kernel::basic_event<prob_type, bitpack_type, size_type> kernel(
                basic_events,
                num_basic_events,
                sample_shape);

        // Define work group sizes
        const size_t work_group_size_z = 1024;// Adjust based on device capabilities
        const size_t local_size_z = work_group_size_z;
        const size_t global_size_z = ((sample_shape.bitpacks_per_batch + local_size_z - 1) / local_size_z) * local_size_z;

        // Define the ranges
        sycl::range<3> global_range(num_basic_events, sample_shape.batch_size, global_size_z);
        sycl::range<3> local_range(1, 1, local_size_z);
        sycl::nd_range<3> ndRange(global_range, local_range);

        // Launch the kernel using parallel_for
        cgh.parallel_for(ndRange, kernel);
    });

    queue.wait_and_throw();

    /** alternatively, submit each basic event job individually **/
    for (auto i = 0; i < num_basic_events; ++i) {
        auto *basic_event = &basic_events[i];
        const auto num_events = 1;

        // Submit the kernel and capture the event
        sycl::event kernel_event = queue.submit([&](sycl::handler &cgh) {
            // Instantiate the kernel
            canopy::kernel::basic_event<prob_type, bitpack_type, size_type> kernel(
                    basic_event,
                    num_events,
                    sample_shape);

            // Define work group sizes
            const size_t work_group_size_z = 1024;// Adjust based on device capabilities
            const size_t local_size_z = work_group_size_z;
            const size_t global_size_z = ((sample_shape.bitpacks_per_batch + local_size_z - 1) / local_size_z) * local_size_z;

            // Define the ranges
            sycl::range<3> global_range(num_events, sample_shape.batch_size, global_size_z);
            sycl::range<3> local_range(1, 1, local_size_z);
            sycl::nd_range<3> ndRange(global_range, local_range);

            // Launch the kernel using parallel_for
            cgh.parallel_for(ndRange, kernel);
        });

        // Submit a host task that depends on the kernel's completion
        queue.submit([&](sycl::handler &cgh) {
            // Add a dependency on the kernel event
            cgh.depends_on(kernel_event);

            // Define a host task that will run on the host after the kernel completes
            cgh.single_task([=]() {
                // Deallocate the per-basic-event output buffer
                sycl::free(basic_event->buffer, queue);
            });
        });
    }
    queue.wait_and_throw();

    // Deallocate per-basic-event output buffers
    for (size_t i = 0; i < num_basic_events; ++i) {
        sycl::free(basic_events[i].buffer, queue);
    }

    // Deallocate the basic_events array
    sycl::free(basic_events, queue);

    return 0;
}

int main() {
    return impl2();
}