set(BOOL_HEADERS
        compute.h
        events.h
)

set(BOOL_SOURCES
        bool.cpp
)

add_executable(bool ${BOOL_SOURCES} ${BOOL_HEADERS})

#target_compile_options(bool PRIVATE --acpp-stdpar --acpp-stdpar-unconditional-offload)
#target_compile_definitions(bool PRIVATE -DHIPSYCL_STDPAR_MEMORY_MANAGEMENT_DEFAULT_DISABLED)

add_sycl_to_target(TARGET bool utils SOURCES ${BOOL_SOURCES} ${BOOL_HEADERS})

install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

set(BOOL_ET_SOURCES
        kaplan_et.cpp
)

add_executable(bool_et ${BOOL_ET_SOURCES} ${BOOL_HEADERS})

add_sycl_to_target(TARGET bool_et utils SOURCES ${BOOL_ET_SOURCES} ${BOOL_HEADERS})

install(TARGETS bool_et
        RUNTIME DESTINATION share/canopy/lib/bool_et)
