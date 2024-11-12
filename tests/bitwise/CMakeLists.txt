set(Boost_USE_STATIC_LIBS OFF)

set(TEST_BITWISE_SOURCES
        test_bitwise.cpp
        bar_tests.cpp
        bit_ops_tests.cpp
        bit_ops_tests.cpp
)

# Locate the Boost library for unit testing
find_package(_boost REQUIRED COMPONENTS unit_test_framework)

# Include directories for the test
include_directories(${Boost_INCLUDE_DIRS})
include_directories("../../src")

add_definitions(-DBOOST_TEST_DYN_LINK)

# Add the test executable
add_executable(test_bitwise ${TEST_BITWISE_SOURCES})

# Link against the Boost Test library and the project's settings library
target_link_libraries(test_bitwise
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        bitwise
)

# Enable testing with CTest
enable_testing()
add_test(NAME BitwiseTests COMMAND test_bitwise)