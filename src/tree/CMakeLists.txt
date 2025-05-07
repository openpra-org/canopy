set(BOOL_ET_SOURCES
        kaplan_et.cpp
)

add_executable(tree ${BOOL_ET_SOURCES})

add_sycl_to_target(TARGET tree SOURCES ${BOOL_ET_SOURCES})

install(TARGETS tree
        RUNTIME DESTINATION bin/canopy)
