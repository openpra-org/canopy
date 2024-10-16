add_executable(bool Bool.cpp)
add_sycl_to_target(TARGET bool SOURCES Bool.cpp)
install(TARGETS bool
        RUNTIME DESTINATION share/canopy/lib/bool)

