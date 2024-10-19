set(SAMPLER_HEADERS
#        Sampler.h
        Stats.h
)

set(SAMPLER_SOURCES
        stub.cpp
        RunningStats.cpp
)

find_package(_boost)

add_library(sampler STATIC ${SAMPLER_SOURCES} ${SAMPLER_HEADERS})

# Link utils against Boost libraries
target_link_libraries(sampler utils boost_accumulators)

install(TARGETS sampler
        RUNTIME DESTINATION share/canopy/lib/sampler)
