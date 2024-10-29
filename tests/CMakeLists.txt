# for boost tests
find_package(_boost COMPONENTS unit_test_framework REQUIRED)

add_subdirectory(bitwise)
add_subdirectory(device_compilation)