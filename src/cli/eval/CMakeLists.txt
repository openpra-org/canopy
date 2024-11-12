set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CLI_EVAL_HEADERS
        argparse.h
        args.h
)

set(CLI_EVAL_SOURCES
        main.cpp
)

find_package(_boost REQUIRED COMPONENTS program_options)

# Include directories for the test
include_directories(${Boost_INCLUDE_DIRS})

# Add the test executable
add_executable(canopy-eval-cli ${CLI_EVAL_SOURCES} ${CLI_EVAL_HEADERS})
target_link_libraries(canopy-eval-cli PRIVATE cli io utils boost_program_options)
add_sycl_to_target(TARGET canopy-eval-cli cli io utils boost_program_options)



install(TARGETS canopy-eval-cli
        RUNTIME DESTINATION bin/canopy)