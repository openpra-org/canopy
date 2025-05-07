set(CLI_HEADERS
        cmdline.h
        entrypoint.h
)

set(CLI_SOURCES
)

find_package(_boost REQUIRED COMPONENTS program_options serialization)

add_library(cli ${CLI_SOURCES} ${CLI_HEADERS})
set_target_properties(cli PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(cli PRIVATE io utils Boost::program_options Boost::serialization)
target_link_libraries(cli PRIVATE io utils Boost::program_options Boost::serialization)

install(TARGETS cli
        RUNTIME DESTINATION lib/canopy)

add_subdirectory(eval)