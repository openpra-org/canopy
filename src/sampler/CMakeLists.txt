set(SAMPLER_HEADERS
        random.h
        importance.h
        sampler.h
)

set(SAMPLER_SOURCES
        sampler.cpp
)

add_library(sampler STATIC ${SAMPLER_HEADERS} ${SAMPLER_SOURCES})
#target_compile_options(sampler PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_COMPILE_OPTIONS>)
#target_include_directories(sampler PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_INCLUDE_DIRECTORIES>)
#target_link_libraries(sampler PUBLIC $<LINK_ONLY:MKL::MKL>)
#add_sycl_to_target(TARGET sampler SOURCES ${SAMPLER_HEADERS} ${SAMPLER_SOURCES})

install(TARGETS sampler
        RUNTIME DESTINATION lib/canopy)
