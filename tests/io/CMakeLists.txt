set(TEST_IO_SOURCES
        test_io.cpp
        test_io_pla.cpp
)

# Add the test executable
add_executable(test_io ${TEST_IO_SOURCES})

# Locate the Boost library for unit testing
find_package(_boost REQUIRED COMPONENTS unit_test_framework)
find_package(_flatbuffers REQUIRED)

# Include directories for the test
target_include_directories(test_io PRIVATE ${_flatbuffers_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenMP_CXX_INCLUDE_DIRS})

# Link against the Boost Test library and the project's settings library
target_link_libraries(test_io
        PRIVATE
        Boost::unit_test_framework
        flatbuffers
        io
)

# Enable testing with CTest
enable_testing()
add_test(NAME IOTests COMMAND test_io)