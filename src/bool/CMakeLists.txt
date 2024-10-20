set(BOOL_HEADERS
)

set(BOOL_SOURCES
        bool.cpp
)

add_executable(bool ${BOOL_SOURCES} ${BOOL_HEADERS})

target_compile_options(bool PRIVATE --acpp-stdpar --acpp-stdpar-unconditional-offload)
target_compile_definitions(bool PRIVATE -DHIPSYCL_STDPAR_MEMORY_MANAGEMENT_DEFAULT_DISABLED)

add_sycl_to_target(TARGET bool utils SOURCES ${BOOL_SOURCES} ${BOOL_HEADERS})

install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

