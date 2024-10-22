set(SAMPLER_HEADERS
        random.h
        importance.h
)

set(SAMPLER_SOURCES
        sampler.cpp
)

add_library(sampler STATIC ${SAMPLER_SOURCES} ${SAMPLER_HEADERS})

install(TARGETS sampler
        RUNTIME DESTINATION share/canopy/lib/sampler)
