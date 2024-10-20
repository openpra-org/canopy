add_executable(bool bool.cpp
        pla/LogicBlock.h
        pla/PLA.h
        stats/stats.h
)

target_compile_options(bool PRIVATE --acpp-stdpar --acpp-stdpar-unconditional-offload)
target_compile_definitions(bool PRIVATE -DHIPSYCL_STDPAR_MEMORY_MANAGEMENT_DEFAULT_DISABLED)

add_sycl_to_target(TARGET bool sampler SOURCES bool.cpp)

install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

