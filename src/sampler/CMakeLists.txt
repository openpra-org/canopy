set(SAMPLER_HEADERS
        random.h
        importance.h
        sampler.h
)

set(SAMPLER_SOURCES
)

add_library(sampler STATIC ${SAMPLER_HEADERS} ${SAMPLER_SOURCES})
set_target_properties(sampler PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(sampler PUBLIC ${OpenMP_CXX_INCLUDE_DIRS})
target_link_libraries(sampler PUBLIC ${OpenMP_CXX_LIBRARIES})

install(TARGETS sampler
        RUNTIME DESTINATION lib/canopy)
