set(UTILS_HEADERS
        checks.h
        stats.h
        profile.h
        profiler.h
        stopwatch.h
        types.h
)

set(UTILS_SOURCES
        stats.cpp
)

find_package(nlohmann_json REQUIRED)
find_package(_boost REQUIRED COMPONENTS program_options serialization)

add_library(utils STATIC ${UTILS_HEADERS} ${UTILS_SOURCES})
set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils PUBLIC nlohmann_json Boost::program_options Boost::serialization)
target_link_libraries(utils PUBLIC nlohmann_json Boost::program_options Boost::serialization)

install(TARGETS utils
        RUNTIME DESTINATION lib/canopy)

