add_executable(bool Bool.cpp
        term.h
        pla/LogicBlock.h
        pla/PLA.h
        OpPlane.h
        AndPlane.h
        OrPlane.h
        pla/OpPlane.h)

target_compile_options(bool PRIVATE --acpp-stdpar --acpp-stdpar-unconditional-offload)
target_compile_definitions(bool PRIVATE -DHIPSYCL_STDPAR_MEMORY_MANAGEMENT_DEFAULT_DISABLED)

add_sycl_to_target(TARGET bool sampler SOURCES Bool.cpp)

install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

