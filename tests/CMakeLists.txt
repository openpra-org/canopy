# for boost tests
find_package(_boost COMPONENTS unit_test_framework REQUIRED)

add_subdirectory(bitwise)
add_subdirectory(bool)
add_subdirectory(nbits)
add_subdirectory(io)
add_subdirectory(device_compilation)
add_subdirectory(sampler)