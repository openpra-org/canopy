set(Boost_USE_STATIC_LIBS OFF)

set(TEST_BOOL_SOURCES
        test_set_F.cpp
        test_bool.cpp
)


# Add the test executable
add_executable(test_bool ${TEST_BOOL_SOURCES})

# Locate the Boost library for unit testing
find_package(_boost REQUIRED COMPONENTS unit_test_framework)

# Include directories for the test
#include_directories(${Boost_INCLUDE_DIRS})
#include_directories("../../src")
target_include_directories(test_bool PRIVATE ${Boost_INCLUDE_DIRS} ${OpenMP_CXX_INCLUDE_DIRS})


#add_definitions(-DBOOST_TEST_DYN_LINK)


# Link against the Boost Test library and the project's settings library
target_link_libraries(test_bool
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        bool
)

# Enable testing with CTest
enable_testing()
add_test(NAME BoolTests COMMAND test_bool)