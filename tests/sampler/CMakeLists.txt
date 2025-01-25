set(Boost_USE_STATIC_LIBS OFF)

set(TEST_SAMPLER_SOURCES
        test_sampler.cpp
)


# Add the test executable
add_executable(test_sampler ${TEST_SAMPLER_SOURCES})

# Locate the Boost library for unit testing
find_package(_boost REQUIRED COMPONENTS unit_test_framework)

# Include directories for the test
include_directories(${Boost_INCLUDE_DIRS})
include_directories("../../src")
target_include_directories(test_sampler PRIVATE ${Boost_INCLUDE_DIRS} ${OpenMP_CXX_INCLUDE_DIRS})


add_definitions(-DBOOST_TEST_DYN_LINK)


# Link against the Boost Test library and the project's settings library
target_link_libraries(test_sampler
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${OpenMP_CXX_LIBRARIES}
        sampler
)

# Enable testing with CTest
enable_testing()
add_test(NAME SamplerTests COMMAND test_sampler)