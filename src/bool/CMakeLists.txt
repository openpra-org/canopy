set(BOOL_HEADERS
        compute.h
        events.h
        working_set.h
        F.h
)

set(BOOL_SOURCES
        bool.cpp
)

add_executable(bool ${BOOL_SOURCES} ${BOOL_HEADERS})
target_link_libraries(bool PRIVATE utils)
add_sycl_to_target(TARGET bool utils SOURCES ${BOOL_SOURCES} ${BOOL_HEADERS})

install(TARGETS bool
        RUNTIME DESTINATION bin/canopy)