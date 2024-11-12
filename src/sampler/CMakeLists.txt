set(SAMPLER_HEADERS
        random.h
        importance.h
)

set(SAMPLER_SOURCES
)

add_library(sampler STATIC ${SAMPLER_HEADERS})
set_target_properties(sampler PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS sampler
        RUNTIME DESTINATION lib/canopy)
