#ifndef WORKING_SET_H
#define WORKING_SET_H

/**
 * @file working_set.h
 * @brief Determine optimal working-set splits for a SYCL device
 *
 * @author Arjun Earthperson
 * @date 11/06/2024
 */

#include <CL/sycl.hpp>
#include <cstddef>


namespace canopy {

    static constexpr size_t TARGET_OCCUPANCY_RATE_OPENCL_CPU(const size_t threads = 1) {
        return static_cast<size_t>(6400.0 * std::pow((128.0 / threads), 4.0/3.0));
    }

    static constexpr size_t TARGET_OCCUPANCY_RATE_OPENMP(const size_t threads = 1) {
        return static_cast<size_t>(2.0 * TARGET_OCCUPANCY_RATE_OPENCL_CPU(threads));
    }

    static constexpr size_t TARGET_OCCUPANCY_RATE_CUDA(const size_t threads = 1) {
        return 204800;
    }

    static constexpr size_t compute_desired_occupancy_rate_heuristic(const hipsycl::rt::backend_id backend, const size_t threads = 1) {
        switch (backend) {
            case hipsycl::rt::backend_id::cuda:
            case hipsycl::rt::backend_id::hip:
                return TARGET_OCCUPANCY_RATE_CUDA(threads);
            case hipsycl::rt::backend_id::ocl:
            case hipsycl::rt::backend_id::level_zero:
                return TARGET_OCCUPANCY_RATE_OPENCL_CPU(threads);
            case hipsycl::rt::backend_id::omp:
                return TARGET_OCCUPANCY_RATE_OPENMP(threads);
        }
    }

/**
 * @brief A structure to represent the working set configuration for SYCL kernels.
 *
 * This structure holds various parameters related to the execution configuration
 * of SYCL kernels, such as the number of work groups, work items, and compute units.
 *
 * @tparam size_type The type used for size-related parameters, defaults to std::size_t.
 *
 * @example
 * @code
 * cl::sycl::queue queue;
 * working_set<> ws = working_set<>::compute(queue, 100, 1000);
 * std::cout << ws << std::endl;
 * @endcode
 */
template <typename size_type = std::size_t>
struct working_set {
    size_type max_num_sub_groups; ///< Maximum number of subgroups.
    size_type max_sub_group_size; ///< Maximum size of a subgroup.
    size_type max_work_item_size_1d; ///< Maximum size of a work item in 1D.
    size_type max_work_group_size; ///< Maximum size of a work group.
    size_type work_group_size; ///< Actual used size of a work group.
    size_type max_compute_units; ///< Maximum number of compute units.
    size_type num_work_groups; ///< Number of work groups.
    size_type global_range; ///< Global range of work items.
    size_type total_work_items = 1; ///< Total number of work items.
    size_type samples_per_work_item; ///< Number of samples per work item.
    size_type F_per_group = 1; ///< Number of F operations per group.
    size_type num_samples = 1 ; ///< Total number of samples
    size_type F_size = 1; ///< Size of the F function block

    /** CUDA **/
    // num_samples = 1e9, num_products = 1e9
    //
    // [GP104]   Tesla P4, 2560 CUDA cores @ 1531 MHz = 25600  : 2.64s
    // [GP104]   Tesla P4, 2560 CUDA cores @ 1531 MHz = 102400 : 1.17s
    // [GP104]   Tesla P4, 2560 CUDA cores @ 1531 MHz = 204800 : 0.95s
    // [GP104]   Tesla P4, 2560 CUDA cores @ 1531 MHz = 256000 : 1.02s
    //
    // [TU116] 1660 Super, 1408 CUDA cores @ 1735 MHz = 102400 : 1.34s
    // [TU116] 1660 Super, 1408 CUDA cores @ 1735 MHz = 112640 : 1.31s
    // [TU116] 1660 Super, 1408 CUDA cores @ 1735 MHz = 204800 : 1.29s
    // [TU116] 1660 Super, 1408 CUDA cores @ 1735 MHz = 307200 : 1.49s
    size_type desired_occupancy = 102400 ; ////< Number of work-groups per compute unit. A higher number can increase
                                        /// parallelism but may also lead to resource contention. Adjust based on
                                        /// performance measurements.


    /** OpenCL **/
    // for OpenCL 64 CPU, 128 threads @ 2.35 GHz, 12800 : 3.28s
    // for OpenCL 64 CPU, 128 threads @ 2.35 GHz, 6400  : 2.85s
    // for OpenCL 64 CPU, 128 threads @ 2.35 GHz, 3200  : 3.99s

    // for OpenCL 8 CPU, 16 threads @ 3.80 GHz = 6400 * 64 = 409600 : 15.0s
    // for OpenCL 8 CPU, 16 threads @ 3.80 GHz = 6400 * 16 = 102400 : 9.55s

    /** OpenMP **/
    // for OpenMP 64 CPU, 128 threads @ 2.35 GHz, 6400 * 32 = 204800 : 8.90s
    // for OpenMP 64 CPU, 128 threads @ 2.35 GHz, 6400 * 16 = 102400 : 7.86s
    // for OpenMP 64 CPU, 128 threads @ 2.35 GHz,           = 32768  : 8.61s
    // for OpenMP 64 CPU, 128 threads @ 2.35 GHz,           = 20480  : 9.00s
    // for OpenMP 64 CPU, 128 threads @ 2.35 GHz, 6400 * 2  = 12800  : 10.7s

    // for OpenMP 8 CPU, 16 threads @ 3.80 GHz, 6400 * 64 = 409600 : 22.9s
    // for OpenMP 8 CPU, 16 threads @ 3.80 GHz, 6400 * 32 = 204800 : 21.6s
    // for OpenMP 8 CPU, 16 threads @ 3.80 GHz, 6400 * 16 = 102400 : 30.8s
    // for OpenMP 8 CPU, 16 threads @ 3.80 GHz, 6400 * 8 = 51200 : 54.9s

    /**
     * @brief Overloaded stream insertion operator for working_set.
     *
     * This operator allows for easy printing of the working_set configuration.
     *
     * @param os The output stream.
     * @param ws The working_set instance to be printed.
     * @return A reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const working_set& ws) {
        os << "max_num_sub_groups: " << ws.max_num_sub_groups << std::endl
           << "max_sub_group_size: " << ws.max_sub_group_size << std::endl
           << "max_work_item_size_1d: " << ws.max_work_item_size_1d << std::endl
           << "max_work_group_size: " << ws.max_work_group_size << std::endl
           << "max_compute_units: " << ws.max_compute_units << std::endl
           << "num_work_groups: " << ws.num_work_groups << std::endl
           << "work_group_size: " << ws.work_group_size << std::endl
           << "global_range: " << ws.global_range << std::endl
           << "total_work_items: " << ws.total_work_items << std::endl
           << "samples_per_work_item: " << ws.samples_per_work_item << std::endl
           << "F_per_group: " << ws.F_per_group << std::endl
           << "F_size: " << ws.F_size << std::endl
           << "num_samples: " << ws.num_samples << std::endl
           << "desired_occupancy: " << ws.desired_occupancy << std::endl;
        return os;
    }

    /**
     * @brief Computes the working set configuration based on the given SYCL queue.
     *
     * This static method calculates the working set parameters using the device
     * information from the provided SYCL queue. It considers the size of F operations
     * and the number of samples to determine the configuration.
     *
     * @param queue The SYCL queue to query device information.
     * @param F_size The size of F operations, defaults to 1.
     * @param num_samples The number of samples, defaults to 1.
     * @return A working_set instance with computed parameters.
     */
    static working_set compute(const cl::sycl::queue& queue, const size_type F_size = 1, const size_type num_samples = 1) {
        working_set set;
        set.F_size = F_size;
        set.num_samples = num_samples;
        const auto device = queue.get_device();
        // queried
        set.max_num_sub_groups = device.get_info<cl::sycl::info::device::max_num_sub_groups>();
        set.max_sub_group_size = 0;
        set.max_work_item_size_1d = device.get_info<cl::sycl::info::device::max_work_item_sizes<1>>()[0];
        set.max_work_group_size = device.get_info<cl::sycl::info::device::max_work_group_size>();
        set.max_compute_units = device.get_info<cl::sycl::info::device::max_compute_units>();

        set.desired_occupancy = compute_desired_occupancy_rate_heuristic(device.get_backend(), set.max_compute_units);

        // Desired parameters (can be adjusted)
        set.work_group_size = set.max_work_group_size; // Use the maximum work group size

        // Compute the initial number of work-groups based on desired occupancy
        set.num_work_groups = set.max_compute_units * set.desired_occupancy;
        set.num_work_groups = std::max<size_t>(1UL, std::min(set.num_work_groups, F_size));

        // Calculate F_per_group with ceiling division to cover all F elements
        set.F_per_group = (F_size + set.num_work_groups - 1) / set.num_work_groups;

        // Recalculate num_work_groups based on adjusted F_per_group
        set.num_work_groups = (F_size + set.F_per_group - 1) / set.F_per_group;

        // Calculate the total number of work-items
        set.total_work_items = set.num_work_groups * set.work_group_size;

        // Determine samples_per_work_item to cover all samples
        set.samples_per_work_item = (num_samples + set.total_work_items - 1) / set.total_work_items;

        // compute the global range
        set.global_range = set.total_work_items;
        std::cout<<set<<std::endl;
        return set;
    }
};
}

#endif //WORKING_SET_H
