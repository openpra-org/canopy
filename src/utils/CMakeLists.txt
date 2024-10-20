set(UTILS_HEADERS
        random.h
        stats.h
        profile.h
        profiler.h
        stopwatch.h
)

set(UTILS_SOURCES
        stub.cpp
)

find_package(_boost)

add_library(utils STATIC ${UTILS_SOURCES} ${UTILS_HEADERS})

# Link utils against Boost libraries
target_link_libraries(utils boost_distributions)

install(TARGETS utils
        RUNTIME DESTINATION share/canopy/lib/utils)
