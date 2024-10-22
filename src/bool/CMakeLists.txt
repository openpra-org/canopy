set(BOOL_HEADERS
)

set(BOOL_SOURCES
        bool.cpp
)

add_executable(bool ${BOOL_SOURCES} ${BOOL_HEADERS})

add_sycl_to_target(TARGET bool utils SOURCES ${BOOL_SOURCES} ${BOOL_HEADERS})

install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

