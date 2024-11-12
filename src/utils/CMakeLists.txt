set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(UTILS_HEADERS
        checks.h
        stats.h
        profile.h
        profiler.h
        stopwatch.h
        bits.h
        types.h
        device_info.h
)

find_package(_boost REQUIRED COMPONENTS program_options serialization)

add_library(utils STATIC ${UTILS_HEADERS})
set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(utils boost_program_options boost_serialization)

install(TARGETS utils
        RUNTIME DESTINATION lib/canopy)

