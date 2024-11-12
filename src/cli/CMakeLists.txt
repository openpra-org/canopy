set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CLI_HEADERS
        cmdline.h
        entrypoint.h
)

set(CLI_SOURCES
)

find_package(_boost REQUIRED COMPONENTS program_options)

# Include directories for the test
include_directories(${Boost_INCLUDE_DIRS})

# Add the test executable
add_library(cli ${CLI_SOURCES} ${CLI_HEADERS})
set_target_properties(cli PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(cli io utils boost_program_options boost_serialization)

install(TARGETS cli
        RUNTIME DESTINATION lib/canopy)

add_subdirectory(eval)