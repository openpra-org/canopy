set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(UTILS_HEADERS
        stats.h
        profile.h
        profiler.h
        stopwatch.h
        bits.h
        types.h
        device_info.h
)

add_library(utils STATIC ${UTILS_HEADERS})
set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS utils
        RUNTIME DESTINATION lib/canopy)

