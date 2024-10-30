#ifndef CANOPY_DEVICE_INFO_H
#define CANOPY_DEVICE_INFO_H

#include <CL/sycl.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <string>

class DeviceInfo {
private:
    cl::sycl::device device_;

    // Helper methods to print different categories of information
    void print_general_info(std::ostream& os) const;
    void print_compute_info(std::ostream& os) const;
    void print_memory_info(std::ostream& os) const;
    void print_aspect_info(std::ostream& os) const;
    void print_fp_config_info(std::ostream& os) const;
    void print_execution_capabilities_info(std::ostream& os) const;
    void print_image_info(std::ostream& os) const;
    void print_partition_info(std::ostream& os) const;
    void print_other_info(std::ostream& os) const;

    // Helper functions to convert enums to strings
    static std::string device_type_to_string(cl::sycl::info::device_type type);
    static std::string fp_config_to_string(cl::sycl::info::fp_config config);
    static std::string aspect_to_string(cl::sycl::aspect aspect);
    static std::string execution_capability_to_string(cl::sycl::info::execution_capability cap);
    static std::string local_mem_type_to_string(cl::sycl::info::local_mem_type type);
    static std::string global_mem_cache_type_to_string(cl::sycl::info::global_mem_cache_type type);
    static std::string partition_property_to_string(cl::sycl::info::partition_property prop);
    static std::string partition_affinity_domain_to_string(cl::sycl::info::partition_affinity_domain domain);

public:
    explicit DeviceInfo(cl::sycl::device  dev) : device_(std::move(dev)) {}

    // Overloaded operator<< to print device information
    friend std::ostream& operator<<(std::ostream& os, const DeviceInfo& d);
};

// Implementation of helper methods and functions

void DeviceInfo::print_general_info(std::ostream& os) const {
    os << "General Information:\n";
    try {
        os << "  Device Name: " << device_.get_info<cl::sycl::info::device::name>() << "\n";
        os << "  Vendor Name: " << device_.get_info<cl::sycl::info::device::vendor>() << "\n";
        os << "  Driver Version: " << device_.get_info<cl::sycl::info::device::driver_version>() << "\n";
        os << "  Profile: " << device_.get_info<cl::sycl::info::device::profile>() << "\n";
        os << "  Version: " << device_.get_info<cl::sycl::info::device::version>() << "\n";
        os << "  OpenCL C Version: " << device_.get_info<cl::sycl::info::device::opencl_c_version>() << "\n";
        os << "  Device Type: " << device_type_to_string(device_.get_info<cl::sycl::info::device::device_type>()) << "\n";
        os << "  Vendor ID: " << device_.get_info<cl::sycl::info::device::vendor_id>() << "\n";
        os << "  Platform: " << device_.get_info<cl::sycl::info::device::platform>().get_info<cl::sycl::info::platform::name>() << "\n";
        os << "  Is Available: " << std::boolalpha << device_.get_info<cl::sycl::info::device::is_available>() << "\n";
        os << "  Is Compiler Available: " << std::boolalpha << device_.get_info<cl::sycl::info::device::is_compiler_available>() << "\n";
        os << "  Is Linker Available: " << std::boolalpha << device_.get_info<cl::sycl::info::device::is_linker_available>() << "\n";
        os << "  Reference Count: " << device_.get_info<cl::sycl::info::device::reference_count>() << "\n";

        os << "  Available Extensions:\n";
        for (const auto& ext : device_.get_info<cl::sycl::info::device::extensions>()) {
            os << "    " << ext << "\n";
        }
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving general information: " << e.what() << "]\n";
    }
}
void DeviceInfo::print_compute_info(std::ostream& os) const {
    os << "Compute Unit Information:\n";
    try {
        os << "  Max Compute Units: " << device_.get_info<cl::sycl::info::device::max_compute_units>() << "\n";
        os << "  Max Work Item Dimensions: " << device_.get_info<cl::sycl::info::device::max_work_item_dimensions>() << "\n";

        auto max_work_item_sizes = device_.get_info<cl::sycl::info::device::max_work_item_sizes<3>>();
        os << "  Max Work Item Sizes: ";
        for (auto i=0; i<3; i++) {
            os << max_work_item_sizes[i] << " ";
        }
        os << "\n";

        os << "  Max Work Group Size: " << device_.get_info<cl::sycl::info::device::max_work_group_size>() << "\n";
        os << "  Preferred Vector Widths:\n";
        os << "    char: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_char>() << "\n";
        os << "    short: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_short>() << "\n";
        os << "    int: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_int>() << "\n";
        os << "    long: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_long>() << "\n";
        os << "    float: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_float>() << "\n";
        os << "    double: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_double>() << "\n";
        os << "    half: " << device_.get_info<cl::sycl::info::device::preferred_vector_width_half>() << "\n";

        os << "  Native Vector Widths:\n";
        os << "    char: " << device_.get_info<cl::sycl::info::device::native_vector_width_char>() << "\n";
        os << "    short: " << device_.get_info<cl::sycl::info::device::native_vector_width_short>() << "\n";
        os << "    int: " << device_.get_info<cl::sycl::info::device::native_vector_width_int>() << "\n";
        os << "    long: " << device_.get_info<cl::sycl::info::device::native_vector_width_long>() << "\n";
        os << "    float: " << device_.get_info<cl::sycl::info::device::native_vector_width_float>() << "\n";
        os << "    double: " << device_.get_info<cl::sycl::info::device::native_vector_width_double>() << "\n";
        os << "    half: " << device_.get_info<cl::sycl::info::device::native_vector_width_half>() << "\n";

        os << "  Max Clock Frequency: " << device_.get_info<cl::sycl::info::device::max_clock_frequency>() << " MHz\n";
        os << "  Address Bits: " << device_.get_info<cl::sycl::info::device::address_bits>() << "\n";
        os << "  Max Samplers: " << device_.get_info<cl::sycl::info::device::max_samplers>() << "\n";
        os << "  Max Parameter Size: " << device_.get_info<cl::sycl::info::device::max_parameter_size>() << "\n";
        os << "  Mem Base Address Align: " << device_.get_info<cl::sycl::info::device::mem_base_addr_align>() << " bits\n";
        os << "  Profiling Timer Resolution: " << device_.get_info<cl::sycl::info::device::profiling_timer_resolution>() << "\n";
        os << "  Endian Little: " << std::boolalpha << device_.get_info<cl::sycl::info::device::is_endian_little>() << "\n";
        os << "  Sub-group Independent Forward Progress: " << std::boolalpha
           << device_.get_info<cl::sycl::info::device::sub_group_independent_forward_progress>() << "\n";

        auto sub_group_sizes = device_.get_info<cl::sycl::info::device::sub_group_sizes>();
        os << "  Sub-group Sizes: ";
        for (const auto& size : sub_group_sizes) {
            os << size << " ";
        }
        os << "\n";

    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving compute information: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_memory_info(std::ostream& os) const {
    os << "Memory Information:\n";
    try {
        os << "  Global Memory Size: " << device_.get_info<cl::sycl::info::device::global_mem_size>() / (1024 * 1024) << " MB\n";
        os << "  Max Memory Allocation Size: " << device_.get_info<cl::sycl::info::device::max_mem_alloc_size>() / (1024 * 1024) << " MB\n";
        os << "  Local Memory Size: " << device_.get_info<cl::sycl::info::device::local_mem_size>() / 1024 << " KB\n";
        os << "  Local Memory Type: " << local_mem_type_to_string(device_.get_info<cl::sycl::info::device::local_mem_type>()) << "\n";
        os << "  Error Correction Support: " << std::boolalpha << device_.get_info<cl::sycl::info::device::error_correction_support>() << "\n";
        os << "  Host Unified Memory: " << std::boolalpha << device_.get_info<cl::sycl::info::device::host_unified_memory>() << "\n";

        os << "  Global Memory Cache Type: " << global_mem_cache_type_to_string(device_.get_info<cl::sycl::info::device::global_mem_cache_type>()) << "\n";
        os << "  Global Memory Cache Line Size: " << device_.get_info<cl::sycl::info::device::global_mem_cache_line_size>() << "\n";
        os << "  Global Memory Cache Size: " << device_.get_info<cl::sycl::info::device::global_mem_cache_size>() / 1024 << " KB\n";
        os << "  Max Constant Buffer Size: " << device_.get_info<cl::sycl::info::device::max_constant_buffer_size>() / 1024 << " KB\n";
        os << "  Max Constant Args: " << device_.get_info<cl::sycl::info::device::max_constant_args>() << "\n";
        os << "  Printf Buffer Size: " << device_.get_info<cl::sycl::info::device::printf_buffer_size>() << "\n";
        os << "  Preferred Interop User Sync: " << std::boolalpha << device_.get_info<cl::sycl::info::device::preferred_interop_user_sync>() << "\n";
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving memory information: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_image_info(std::ostream& os) const {
    os << "Image Support Information:\n";
    try {
        os << "  Image Support: " << std::boolalpha << device_.get_info<cl::sycl::info::device::image_support>() << "\n";
        if (device_.get_info<cl::sycl::info::device::image_support>()) {
            os << "  Max Read Image Args: " << device_.get_info<cl::sycl::info::device::max_read_image_args>() << "\n";
            os << "  Max Write Image Args: " << device_.get_info<cl::sycl::info::device::max_write_image_args>() << "\n";
            os << "  Image2D Max Width: " << device_.get_info<cl::sycl::info::device::image2d_max_width>() << "\n";
            os << "  Image2D Max Height: " << device_.get_info<cl::sycl::info::device::image2d_max_height>() << "\n";
            os << "  Image3D Max Width: " << device_.get_info<cl::sycl::info::device::image3d_max_width>() << "\n";
            os << "  Image3D Max Height: " << device_.get_info<cl::sycl::info::device::image3d_max_height>() << "\n";
            os << "  Image3D Max Depth: " << device_.get_info<cl::sycl::info::device::image3d_max_depth>() << "\n";
            os << "  Image Max Buffer Size: " << device_.get_info<cl::sycl::info::device::image_max_buffer_size>() << "\n";
            os << "  Image Max Array Size: " << device_.get_info<cl::sycl::info::device::image_max_array_size>() << "\n";
        }
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving image information: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_fp_config_info(std::ostream& os) const {
    os << "Floating Point Configurations:\n";
    try {
        os << "  Half Precision FP Configurations:\n";
        for (const auto& config : device_.get_info<cl::sycl::info::device::half_fp_config>()) {
            os << "    " << fp_config_to_string(config) << "\n";
        }
    } catch (...) {
        os << "  [Half precision FP configurations not supported]\n";
    }

    try {
        os << "  Single Precision FP Configurations:\n";
        for (const auto& config : device_.get_info<cl::sycl::info::device::single_fp_config>()) {
            os << "    " << fp_config_to_string(config) << "\n";
        }
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving single precision FP configurations: " << e.what() << "]\n";
    }

    try {
        os << "  Double Precision FP Configurations:\n";
        for (const auto& config : device_.get_info<cl::sycl::info::device::double_fp_config>()) {
            os << "    " << fp_config_to_string(config) << "\n";
        }
    } catch (...) {
        os << "  [Double precision FP configurations not supported]\n";
    }
}

void DeviceInfo::print_aspect_info(std::ostream& os) const {
    os << "Aspects Supported:\n";
    try {
        for (const auto& aspect : device_.get_info<cl::sycl::info::device::aspects>()) {
            os << "  " << aspect_to_string(aspect) << "\n";
        }
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving aspect information: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_execution_capabilities_info(std::ostream& os) const {
    os << "Execution Capabilities:\n";
    try {
        for (const auto& cap : device_.get_info<cl::sycl::info::device::execution_capabilities>()) {
            os << "  " << execution_capability_to_string(cap) << "\n";
        }
        os << "  Queue Profiling: " << std::boolalpha << device_.get_info<cl::sycl::info::device::queue_profiling>() << "\n";
        os << "  Built-in Kernels:\n";
        for (const auto& kernel : device_.get_info<cl::sycl::info::device::built_in_kernels>()) {
            os << "    " << kernel << "\n";
        }
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving execution capabilities: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_partition_info(std::ostream& os) const {
    os << "Partition Information:\n";
    try {
        os << "  Partition Max Sub-devices: " << device_.get_info<cl::sycl::info::device::partition_max_sub_devices>() << "\n";

        os << "  Partition Properties:\n";
        for (const auto& prop : device_.get_info<cl::sycl::info::device::partition_properties>()) {
            os << "    " << partition_property_to_string(prop) << "\n";
        }

        os << "  Partition Affinity Domains:\n";
        for (const auto& domain : device_.get_info<cl::sycl::info::device::partition_affinity_domains>()) {
            os << "    " << partition_affinity_domain_to_string(domain) << "\n";
        }

        os << "  Partition Type Property: " << partition_property_to_string(device_.get_info<cl::sycl::info::device::partition_type_property>()) << "\n";
        os << "  Partition Type Affinity Domain: " << partition_affinity_domain_to_string(device_.get_info<cl::sycl::info::device::partition_type_affinity_domain>()) << "\n";
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving partition information: " << e.what() << "]\n";
    }
}

void DeviceInfo::print_other_info(std::ostream& os) const {
    os << "Other Information:\n";
    try {
        // Query parent device if applicable
        auto parent_device = device_.get_info<cl::sycl::info::device::parent_device>();
        os << "  Parent Device: " << parent_device.get_info<cl::sycl::info::device::name>() << "\n";
    } catch (const cl::sycl::exception& e) {
        os << "  [Error retrieving other information: " << e.what() << "]\n";
    }
}


// Helper functions to convert enums to strings
std::string DeviceInfo::device_type_to_string(cl::sycl::info::device_type type) {
    switch (type) {
        case cl::sycl::info::device_type::cpu: return "CPU";
        case cl::sycl::info::device_type::gpu: return "GPU";
        case cl::sycl::info::device_type::accelerator: return "Accelerator";
        case cl::sycl::info::device_type::custom: return "Custom";
        case cl::sycl::info::device_type::automatic: return "Automatic";
        case cl::sycl::info::device_type::host: return "Host";
        default: return "Unknown";
    }
}

std::string DeviceInfo::fp_config_to_string(cl::sycl::info::fp_config config) {
    switch (config) {
        case cl::sycl::info::fp_config::denorm: return "denorm";
        case cl::sycl::info::fp_config::inf_nan: return "inf_nan";
        case cl::sycl::info::fp_config::round_to_nearest: return "round_to_nearest";
        case cl::sycl::info::fp_config::round_to_zero: return "round_to_zero";
        case cl::sycl::info::fp_config::round_to_inf: return "round_to_inf";
        case cl::sycl::info::fp_config::fma: return "fma";
        case cl::sycl::info::fp_config::correctly_rounded_divide_sqrt: return "correctly_rounded_divide_sqrt";
        case cl::sycl::info::fp_config::soft_float: return "soft_float";
        default: return "unknown_fp_config";
    }
}

std::string DeviceInfo::aspect_to_string(cl::sycl::aspect aspect) {
    switch (aspect) {
        case cl::sycl::aspect::cpu: return "cpu";
        case cl::sycl::aspect::gpu: return "gpu";
        case cl::sycl::aspect::accelerator: return "accelerator";
        case cl::sycl::aspect::custom: return "custom";
        case cl::sycl::aspect::emulated: return "emulated";
        case cl::sycl::aspect::host_debuggable: return "host_debuggable";
        case cl::sycl::aspect::fp16: return "fp16";
        case cl::sycl::aspect::fp64: return "fp64";
        case cl::sycl::aspect::atomic64: return "atomic64";
        case cl::sycl::aspect::image: return "image";
        case cl::sycl::aspect::online_compiler: return "online_compiler";
        case cl::sycl::aspect::online_linker: return "online_linker";
        case cl::sycl::aspect::queue_profiling: return "queue_profiling";
        case cl::sycl::aspect::usm_device_allocations: return "usm_device_allocations";
        case cl::sycl::aspect::usm_host_allocations: return "usm_host_allocations";
        case cl::sycl::aspect::usm_atomic_host_allocations: return "usm_atomic_host_allocations";
        case cl::sycl::aspect::usm_shared_allocations: return "usm_shared_allocations";
        case cl::sycl::aspect::usm_atomic_shared_allocations: return "usm_atomic_shared_allocations";
        case cl::sycl::aspect::usm_system_allocations: return "usm_system_allocations";
        default: return "unknown_aspect";
    }
}

std::string DeviceInfo::execution_capability_to_string(cl::sycl::info::execution_capability cap) {
    switch (cap) {
        case cl::sycl::info::execution_capability::exec_kernel: return "exec_kernel";
        case cl::sycl::info::execution_capability::exec_native_kernel: return "exec_native_kernel";
        default: return "unknown_execution_capability";
    }
}

std::string DeviceInfo::local_mem_type_to_string(cl::sycl::info::local_mem_type type) {
    switch (type) {
        case cl::sycl::info::local_mem_type::none: return "none";
        case cl::sycl::info::local_mem_type::local: return "local";
        case cl::sycl::info::local_mem_type::global: return "global";
        default: return "unknown_local_mem_type";
    }
}

std::string DeviceInfo::global_mem_cache_type_to_string(cl::sycl::info::global_mem_cache_type type) {
    switch (type) {
        case cl::sycl::info::global_mem_cache_type::none: return "none";
        case cl::sycl::info::global_mem_cache_type::read_only: return "read_only";
        case cl::sycl::info::global_mem_cache_type::read_write: return "read_write";
        default: return "unknown_global_mem_cache_type";
    }
}

std::string DeviceInfo::partition_property_to_string(cl::sycl::info::partition_property prop) {
    switch (prop) {
        case cl::sycl::info::partition_property::no_partition: return "no_partition";
        case cl::sycl::info::partition_property::partition_equally: return "partition_equally";
        case cl::sycl::info::partition_property::partition_by_counts: return "partition_by_counts";
        case cl::sycl::info::partition_property::partition_by_affinity_domain: return "partition_by_affinity_domain";
        default: return "unknown_partition_property";
    }
}

std::string DeviceInfo::partition_affinity_domain_to_string(cl::sycl::info::partition_affinity_domain domain) {
    switch (domain) {
        case cl::sycl::info::partition_affinity_domain::not_applicable: return "not_applicable";
        case cl::sycl::info::partition_affinity_domain::numa: return "numa";
        case cl::sycl::info::partition_affinity_domain::L1_cache: return "L1_cache";
        case cl::sycl::info::partition_affinity_domain::L2_cache: return "L2_cache";
        case cl::sycl::info::partition_affinity_domain::L3_cache: return "L3_cache";
        case cl::sycl::info::partition_affinity_domain::L4_cache: return "L4_cache";
        case cl::sycl::info::partition_affinity_domain::next_partitionable: return "next_partitionable";
        default: return "unknown_partition_affinity_domain";
    }
}

// Overloaded operator<< implementation
std::ostream& operator<<(std::ostream& os, const DeviceInfo& d) {
    os << "Device Information:\n";
    d.print_general_info(os);
    os << "\n";
    d.print_compute_info(os);
    os << "\n";
    d.print_memory_info(os);
    os << "\n";
    d.print_image_info(os);
    os << "\n";
    d.print_fp_config_info(os);
    os << "\n";
    d.print_execution_capabilities_info(os);
    os << "\n";
    d.print_partition_info(os);
    os << "\n";
    d.print_aspect_info(os);
    os << "\n";
    d.print_other_info(os);
    return os;
}

#endif //CANOPY_DEVICE_INFO_H
