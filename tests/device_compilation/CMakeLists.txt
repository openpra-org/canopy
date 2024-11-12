set(Boost_USE_STATIC_LIBS OFF)

set(TEST_DEVICE_COMPILATION_SOURCES
        device_compilation_tests.cpp
)

# Locate the Boost library for unit testing
find_package(_boost REQUIRED COMPONENTS unit_test_framework)

# Include directories for the test
include_directories(${Boost_INCLUDE_DIRS})
include_directories("../../src")

add_definitions(-DBOOST_TEST_DYN_LINK)

# Add the test executable
add_executable(test_device_compilation ${TEST_DEVICE_COMPILATION_SOURCES})

add_sycl_to_target(TARGET test_device_compilation ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

# Enable testing with CTest
enable_testing()
add_test(NAME DeviceCompilationTests COMMAND test_device_compilation)
