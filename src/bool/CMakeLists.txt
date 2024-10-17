add_executable(bool Bool.cpp
        term.h
        pla/PLA.h
        OpPlane.h
        AndPlane.h
        OrPlane.h
        LogicBlock.h)
add_sycl_to_target(TARGET bool SOURCES Bool.cpp)
install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

