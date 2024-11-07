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
    size_type max_work_item_size_1d; ///< Maximum size of a work item in 1D.
    size_type max_work_group_size; ///< Maximum size of a work group.
    size_type max_compute_units; ///< Maximum number of compute units.
    size_type num_work_groups; ///< Number of work groups.
    size_type global_range; ///< Global range of work items.
    size_type total_work_items; ///< Total number of work items.
    size_type samples_per_work_item; ///< Number of samples per work item.
    size_type F_per_group; ///< Number of F operations per group.

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
        os << "max_num_sub_groups: " << ws.max_num_sub_groups
           << " max_work_item_size_1d: " << ws.max_work_item_size_1d
           << " max_work_group_size: " << ws.max_work_group_size
           << " max_compute_units: " << ws.max_compute_units
           << " num_work_groups: " << ws.num_work_groups
           << " global_range: " << ws.global_range
           << " total_work_items: " << ws.total_work_items
           << " samples_per_work_item: " << ws.samples_per_work_item
           << " F_per_group: " << ws.F_per_group;
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
        const auto device = queue.get_device();
        // queried
        set.max_num_sub_groups = device.get_info<cl::sycl::info::device::max_num_sub_groups>();
        set.max_work_item_size_1d = device.get_info<cl::sycl::info::device::max_work_item_sizes<1>>()[0];
        set.max_work_group_size = device.get_info<cl::sycl::info::device::max_work_group_size>();
        set.max_compute_units = device.get_info<cl::sycl::info::device::max_compute_units>();
        // computed
        set.num_work_groups = 54 * set.max_compute_units;
        set.global_range = set.num_work_groups * set.max_work_group_size;
        set.total_work_items = set.global_range;
        set.samples_per_work_item = (num_samples + set.total_work_items - 1) / set.total_work_items;
        set.F_per_group = (F_size + set.num_work_groups - 1) / set.num_work_groups;

        std::cout<<set<<std::endl;
        return set;
    }
};
}

#endif //WORKING_SET_H
