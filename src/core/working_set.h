#pragma once

/**
 * @file working_set.h
 * @brief Determine optimal working-set splits for a SYCL device
 *
 * @author Arjun Earthperson
 * @date 11/06/2024
 */

#include "core/node.h"

#include "scram/logger.h"


#include <cstddef>
#include <sycl/sycl.hpp>


namespace canopy {

    static constexpr size_t TARGET_OCCUPANCY_RATE_OPENCL_CPU(const size_t threads = 1) {
        return static_cast<size_t>(6400.0 * std::pow((128.0 / static_cast<double_t>(threads)), 4.0 / 3.0));
    }

    static constexpr size_t TARGET_OCCUPANCY_RATE_OPENMP(const size_t threads = 1) {
        return static_cast<size_t>(2 * TARGET_OCCUPANCY_RATE_OPENCL_CPU(threads));
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
            default:
                return TARGET_OCCUPANCY_RATE_OPENMP(threads);
        }
    }

    template<typename size_type, typename bitpack_type>
    struct working_set {
        size_type num_events_;
        size_type samples_per_event_in_bits_;
        size_type samples_per_event_in_bytes_;
        sample_shape<size_type> bitpack_buffer_shape_; ///<
        size_type samples_in_bytes_;

        // raw capabilities
        sycl::info::device_type device_type;
        sycl::detail::u_int max_compute_units;            ///< Maximum number of compute units.
        sycl::detail::u_int max_clock_frequency;

        // work-item
        sycl::detail::u_int max_work_item_dimensions;
        sycl::range<1> max_work_item_sizes_1d;  ///< Maximum sizes for each work item, by dimension, in 1D.
        sycl::range<2> max_work_item_sizes_2d;  ///< Maximum sizes for each work item, by dimension, in 2D.
        sycl::range<3> max_work_item_sizes_3d;  ///< Maximum sizes for each work item, by dimension, in 3D.
        bool work_item_independent_forward_progress;

        // work-group
        std::size_t max_work_group_size;          ///< Maximum size of a work group.

        // sub-group
        sycl::detail::u_int max_num_sub_groups;           ///< Maximum number of subgroups.
        bool sub_group_independent_forward_progress;
        std::vector<std::size_t> sub_group_sizes;
        sycl::detail::u_int preferred_vector_width_char; ///

        // memory allocation
        sycl::detail::u_long max_mem_alloc_size;
        // global memory
        sycl::detail::u_int global_mem_cache_line_size;
        sycl::detail::u_long global_mem_size;
        sycl::detail::u_long global_mem_cache_size;
        sycl::info::global_mem_cache_type global_mem_cache_type;
        // local memory
        sycl::info::local_mem_type local_mem_type;
        sycl::detail::u_long local_mem_size;

        //size_type max_sub_group_size;           ///< Maximum size of a subgroup.
        //size_type work_group_size;              ///< Actual used size of a work group.
        // size_type num_work_groups;              ///< Number of work groups.
        // size_type global_range;                 ///< Global range of work items.
        // size_type total_work_items = 1;         ///< Total number of work items.
        // size_type samples_per_work_item;        ///< Number of samples per work item.
        // size_type F_per_group = 1;              ///< Number of F operations per group.
        // size_type num_samples = 1;              ///< Total number of samples
        // size_type F_size = 1;                   ///< Size of the F function block

        working_set(const sycl::queue &queue, const size_type num_events, const sample_shape<size_type> &requested_shape) {
            const auto device = queue.get_device();
            num_events_ = num_events;
            bitpack_buffer_shape_ = requested_shape;
            samples_per_event_in_bytes_ = bitpack_buffer_shape_.num_bitpacks() * sizeof(bitpack_type);
            samples_per_event_in_bits_ = samples_per_event_in_bytes_ * 8;
            samples_in_bytes_ = samples_per_event_in_bytes_ * num_events_;

            device_type = device.get_info<sycl::info::device::device_type>();
            max_compute_units = device.get_info<sycl::info::device::max_compute_units>();
            max_clock_frequency = device.get_info<sycl::info::device::max_clock_frequency>();
            desired_occupancy = compute_desired_occupancy_rate_heuristic(device.get_backend(), max_compute_units);

            max_work_item_dimensions = device.get_info<sycl::info::device::max_work_item_dimensions>();
            max_work_item_sizes_1d = device.get_info<sycl::info::device::max_work_item_sizes<1>>();
            max_work_item_sizes_2d = device.get_info<sycl::info::device::max_work_item_sizes<2>>();
            max_work_item_sizes_3d = device.get_info<sycl::info::device::max_work_item_sizes<3>>();
            work_item_independent_forward_progress = false;

            max_work_group_size = device.get_info<sycl::info::device::max_work_group_size>();

            max_num_sub_groups = device.get_info<sycl::info::device::max_num_sub_groups>();
            sub_group_sizes = device.get_info<sycl::info::device::sub_group_sizes>();
            preferred_vector_width_char = device.get_info<sycl::info::device::preferred_vector_width_char>();
            sub_group_independent_forward_progress = device.get_info<sycl::info::device::sub_group_independent_forward_progress>();

            // memory allocation
            max_mem_alloc_size = device.get_info<sycl::info::device::max_mem_alloc_size>();
            global_mem_size = device.get_info<sycl::info::device::global_mem_size>();
            global_mem_cache_size = device.get_info<sycl::info::device::global_mem_cache_size>();
            global_mem_cache_line_size = device.get_info<sycl::info::device::global_mem_cache_line_size>();
            global_mem_cache_type = device.get_info<sycl::info::device::global_mem_cache_type>();
            local_mem_type = device.get_info<sycl::info::device::local_mem_type>();
            local_mem_size = device.get_info<sycl::info::device::local_mem_size>();
        }
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
        size_type desired_occupancy = 102400;////< Number of work-groups per compute unit. A higher number can increase
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
        friend std::ostream &operator<<(std::ostream &os, const working_set &ws) {
            os  << "device_type: ";
            switch (ws.device_type) {
                case sycl::info::device_type::cpu: os << "cpu"; break;
                case sycl::info::device_type::gpu: os << "gpu"; break;
                case sycl::info::device_type::all: os << "all"; break;
                case sycl::info::device_type::custom: os << "custom"; break;
                case sycl::info::device_type::automatic: os << "automatic"; break;
                case sycl::info::device_type::accelerator: os << "accelerator";  break;
                case sycl::info::device_type::host: os << "host"; break;
                default: os << "unknown"; break;
            } os << std::endl;
            os  << "max_compute_units: " << ws.max_compute_units << std::endl
                << "max_clock_frequency: " << ws.max_clock_frequency << std::endl
                << "desired_occupancy: " << ws.desired_occupancy << std::endl
                << "------------------------------------------------" << std::endl;
            os  << "max_work_item_dimensions: " << ws.max_work_item_dimensions << std::endl
                << "max_work_item_sizes_1d: " << ws.max_work_item_sizes_1d[0] << std::endl
                << "max_work_item_sizes_2d: " << ws.max_work_item_sizes_2d[0] << ", " << ws.max_work_item_sizes_2d[1] << std::endl
                << "max_work_item_sizes_3d: " << ws.max_work_item_sizes_3d[0] << ", " << ws.max_work_item_sizes_3d[1] << ", " << ws.max_work_item_sizes_3d[2] << std::endl
                << "work_item_independent_forward_progress: " << ws.work_item_independent_forward_progress << std::endl
                << "------------------------------------------------" << std::endl;
            os  << "max_work_group_size: " << ws.max_work_group_size << std::endl
                << "------------------------------------------------" << std::endl;
            os  << "max_num_sub_groups: " << ws.max_num_sub_groups << std::endl
                << "sub_group_sizes: "; for (const auto &size : ws.sub_group_sizes) { os << size <<", "; } os << std::endl
                << "preferred_vector_width_char: "<< ws.preferred_vector_width_char << std::endl
                << "sub_group_independent_forward_progress: " << ws.sub_group_independent_forward_progress << std::endl
                << "------------------------------------------------" << std::endl;
            os  << "max_mem_alloc_size: " << ws.max_mem_alloc_size << std::endl
                << "global_mem_size: " << ws.global_mem_size << std::endl
                << "global_mem_cache_size: " << ws.global_mem_cache_size << std::endl
                << "global_mem_cache_line_size: " << ws.global_mem_cache_line_size << std::endl
                << "global_mem_cache_type: ";
                switch (ws.global_mem_cache_type) {
                    case sycl::info::global_mem_cache_type::none: os << "none"; break;
                    case sycl::info::global_mem_cache_type::read_only: os << "read_only"; break;
                    case sycl::info::global_mem_cache_type::read_write: os << "read_write"; break;
                    default: os << "unknown"; break;
                } os << std::endl
                << "local_mem_type: ";
                switch (ws.local_mem_type) {
                    case sycl::info::local_mem_type::none: os << "none"; break;
                    case sycl::info::local_mem_type::local: os << "local"; break;
                    case sycl::info::local_mem_type::global: os << "global"; break;
                    default: os << "unknown"; break;
                } os << std::endl
                << "local_mem_size: " << ws.local_mem_size << std::endl
                << "------------------------------------------------" << std::endl;
            os  << "num_events_: " << ws.num_events_ << std::endl
                << "buffer_shape_batch_size: " << ws.bitpack_buffer_shape_.batch_size << std::endl
                << "buffer_shape_bitpacks_per_batch: " << ws.bitpack_buffer_shape_.bitpacks_per_batch << std::endl
                << "buffer_samples_per_event_in_bytes: " << ws.samples_per_event_in_bytes_ << std::endl
                << "sample_buffer_in_bytes: " << ws.samples_in_bytes_ << std::endl
                << "sampled_bits_per_event: " << ws.samples_per_event_in_bits_ << std::endl;
            return os;
        }

        /**
         * @brief Computes the optimal nd_range for the tally kernel.
         *
         * This method calculates the optimal local size (workgroup size) for the tally kernel
         * by querying the device capabilities.
         *
         * @param queue The SYCL queue to query device information.
         * @param num_tallies The number of tally events (number of workgroups).
         * @param buffer_size_per_tally The size of the buffer for each tally event (number of bitpack elements).
         * @return A sycl::nd_range<1> object with the optimal global and local sizes.
         */
        static sycl::nd_range<1> compute_optimal_nd_range_for_tally(const sycl::queue &queue, const size_type total_elements) {
            const auto device = queue.get_device();
            size_type max_work_group_size = device.get_info<sycl::info::device::max_work_group_size>();

            // Try to obtain sub_group_sizes
            std::vector<size_t> sub_group_sizes;
            if (device.has(sycl::aspect::gpu)) {
                sub_group_sizes = device.get_info<sycl::info::device::sub_group_sizes>();
            }

            size_type optimal_local_size = 0;

            if (!sub_group_sizes.empty()) {
                // Choose the maximum sub-group size as the base for local size
                size_type max_sub_group_size = *std::max_element(sub_group_sizes.begin(), sub_group_sizes.end());

                // Multiply sub-group size to get an optimal local size, ensuring it's within max work-group size
                size_type multiplier = 1;
                while (max_sub_group_size * multiplier <= max_work_group_size) {
                    multiplier *= 2;// Adjust as needed (e.g., double the multiplier)
                }
                multiplier /= 2;// Step back to the last valid multiplier
                optimal_local_size = max_sub_group_size * multiplier;
            } else {
                // If sub_group_sizes are not available, use other device parameters
                // For CPUs or devices without sub-groups, we can use the preferred vector width
                size_type preferred_vector_width = device.get_info<sycl::info::device::preferred_vector_width_char>();

                if (preferred_vector_width == 0) {
                    // If preferred vector width is not available, default to 1
                    preferred_vector_width = 1;
                }

                // Multiply preferred vector width to get an optimal local size, within constraints
                optimal_local_size = preferred_vector_width;
                while (optimal_local_size * 2 <= max_work_group_size) {
                    optimal_local_size *= 2;
                }
            }

            // Ensure that optimal_local_size is at least 1 and does not exceed max_work_group_size
            optimal_local_size = std::max<size_type>(1, std::min(optimal_local_size, max_work_group_size));

            // Calculate global range, ensuring it's a multiple of local size
            size_type num_work_groups = (total_elements + optimal_local_size - 1) / optimal_local_size;
            size_type global_range = num_work_groups * optimal_local_size;

            // Construct and return the nd_range object
            sycl::range<1> global_range_obj(global_range);
            sycl::range<1> local_range_obj(optimal_local_size);

            return sycl::nd_range<1>(global_range_obj, local_range_obj);
        }

        static sample_shape<size_type> compute_optimal_sample_shape(const sycl::queue &queue, const size_type num_events) {
            const auto device = queue.get_device();
            const size_t max_malloc_size = device.get_info<sycl::info::device::max_mem_alloc_size>();
            static constexpr size_type max_sample_size = 16;
            static constexpr size_type max_batch_size = 16;
            static constexpr size_type one = 1;
            size_type ss = max_sample_size;
            size_type bs = max_batch_size;
            bool found = false;

            for (; ss >= 0; --ss) {
                bs = max_batch_size;// Reinitialize bs for each ss
                for (; bs >= 0; --bs) {
                    uint64_t used_bytes = static_cast<uint64_t>(num_events) * (static_cast<uint64_t>(one) << bs) * (static_cast<uint64_t>(one) << ss) * sizeof(bitpack_type);
                    if (used_bytes <= max_malloc_size) {
                        found = true;
                        break;// Breaks out of the inner loop
                    }
                }
                if (found) {
                    break;// Breaks out of the outer loop
                }
            }
            if (!found) {
                ss = 0;
                bs = 0;
            } else {
                // Adjust ss and bs because they were decremented after finding the valid values
                // (since the for loop decrements before checking the condition)
                ss = ss;
                bs = bs;
            }
            sample_shape<size_type> shape = {
                    .batch_size = one << bs,
                    .bitpacks_per_batch = one << ss,
            };
            return shape;
        }

        static size_type closest_power_of_2(const size_type n) {
            if (n == 0) return 1;  // Edge case: define closest power of 2 to 0 as 1

            size_type min_diff = std::numeric_limits<size_type>::max();  // Initialize minimum difference
            size_type closest = 0;  // To store the closest power of 2

            // Iterate over possible exponents x
            for (size_type x = 0; x < sizeof(size_type) * 8; ++x) {
                const size_type pow2 = static_cast<size_type>(1) << x;  // Compute 2^x
                long diff = pow2 > n ? pow2 - n : n - pow2;
                if (diff < min_diff) {
                    min_diff = diff;
                    closest = pow2;
                } else if (diff == min_diff) {
                    // If there's a tie, choose the smaller power of 2
                    if (pow2 < closest) {
                        closest = pow2;
                    }
                } else {
                    // Since the differences will start increasing after the minimum,
                    // we can break out of the loop early
                    break;
                }
            }

            return static_cast<size_type>(closest);
        }

        [[nodiscard]] sycl::range<3> compute_optimal_local_range_3d_for_cpu(const sycl::range<3> &limits = {0, 0, 0}) const {
            const auto num_bytes_in_dtype = sizeof(bitpack_type); // in bytes
            const auto div_8 = 8 / num_bytes_in_dtype;
            const auto lz = !limits[2] ? div_8 : std::clamp(div_8, static_cast<decltype(limits[2])>(1), limits[2]);
            const auto hw_limited_target_z = std::clamp(lz, lz, max_work_item_sizes_3d[2]);
            return sycl::range<3>{1, 1, hw_limited_target_z};
        }

        [[nodiscard]] sycl::range<3> compute_optimal_local_range_3d_for_gpu(const sycl::range<3> &limits = {0, 0, 0}) const {
            const auto log_2_max_work_group_size = static_cast<std::uint8_t>(std::log2(max_work_group_size));
            auto remaining_budget = log_2_max_work_group_size;

            const auto target_x = !limits[0] ? num_events_ : std::clamp(static_cast<decltype(limits[0])>(num_events_), static_cast<decltype(limits[0])>(1), limits[0]);
            const auto hw_limited_target_x = std::clamp(target_x, target_x, max_work_item_sizes_3d[0]);
            const auto log_2_rounded_x = static_cast<std::uint8_t>(std::log2(closest_power_of_2(hw_limited_target_x)));
            const auto log2_local_x = std::min<std::uint8_t>(log_2_rounded_x, remaining_budget); // between 0 and 10,13

            remaining_budget = remaining_budget - log2_local_x; // between 0 and 10,13

            const auto target_y = !limits[1] ? bitpack_buffer_shape_.batch_size : std::clamp(static_cast<decltype(limits[1])>(bitpack_buffer_shape_.batch_size), static_cast<decltype(limits[1])>(1), limits[1]);
            const auto hw_limited_target_y = std::clamp(target_y, target_y, max_work_item_sizes_3d[1]);
            const auto log_2_rounded_y = static_cast<std::uint8_t>(std::log2(closest_power_of_2(hw_limited_target_y)));
            const auto log2_local_y = std::min<std::uint8_t>(log_2_rounded_y, remaining_budget); // between 0 and 10,13

            remaining_budget = remaining_budget - log2_local_y; // between 0 and 10,13

            const auto target_z = !limits[2] ? bitpack_buffer_shape_.bitpacks_per_batch : std::clamp(static_cast<decltype(limits[2])>(bitpack_buffer_shape_.bitpacks_per_batch), static_cast<decltype(limits[2])>(1), limits[2]);
            const auto hw_limited_target_z = std::clamp(target_z, target_z, max_work_item_sizes_3d[2]);
            const auto log_2_rounded_z = static_cast<std::uint8_t>(std::log2(closest_power_of_2(hw_limited_target_z)));
            const auto log2_local_z = std::min<std::uint8_t>(log_2_rounded_z, remaining_budget); // between 0 and 10,13

            const auto lx = static_cast<std::size_t>(1) << log2_local_x;
            const auto ly = static_cast<std::size_t>(1) << log2_local_y;
            const auto lz = static_cast<std::size_t>(1) << log2_local_z;
            return {lx, ly, lz};
        }


        [[nodiscard]] sycl::range<3> compute_optimal_local_range_3d(const sycl::range<3> &limits = {0, 0, 0}) const {
            sycl::range<3> local_range;
            switch (device_type) {
                case sycl::info::device_type::cpu:
                    local_range = compute_optimal_local_range_3d_for_cpu(limits);
                    //break;
                case sycl::info::device_type::gpu:
                case sycl::info::device_type::accelerator:
                case sycl::info::device_type::custom:
                case sycl::info::device_type::automatic:
                case sycl::info::device_type::host:
                case sycl::info::device_type::all:
                    local_range = compute_optimal_local_range_3d_for_gpu(limits);
                    break;
            }
            LOG(scram::DEBUG3) << "local_range: (events:"<< num_events_ <<", batch_size:"<< bitpack_buffer_shape_.batch_size <<", sample_size:"<<bitpack_buffer_shape_.bitpacks_per_batch<<"): (" << local_range[0] <<", " << local_range[1] <<", " << local_range[2] <<")";
            assert(local_range[0] * local_range[1] * local_range[2] <= max_work_group_size);
            return local_range;
        }

        template<typename dtype>
        static sample_shape<dtype> &rounded(sample_shape<dtype> &shape) {
            shape.batch_size = (shape.batch_size);
            shape.bitpacks_per_batch = (shape.bitpacks_per_batch);
            return shape;
        }

        template<typename dtype>
        static sample_shape<dtype> rounded(const sample_shape<dtype> &shape) {
            sample_shape<dtype> new_shape;
            new_shape.batch_size = (shape.batch_size);
            new_shape.bitpacks_per_batch = (shape.bitpacks_per_batch);
            return new_shape;
        }
    };
}// namespace scram::canopy
