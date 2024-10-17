add_executable(bool Bool.cpp
        term.h
        pla/LogicBlock.h
        pla/PLA.h
        OpPlane.h
        AndPlane.h
        OrPlane.h
        pla/OpPlane.h)
add_sycl_to_target(TARGET bool SOURCES Bool.cpp)
install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

