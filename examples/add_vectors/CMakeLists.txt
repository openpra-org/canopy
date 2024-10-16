add_executable(add_vectors add_vectors.cpp)
add_sycl_to_target(TARGET add_vectors SOURCES add_vectors.cpp)
install(TARGETS add_vectors
        RUNTIME DESTINATION share/canopy/examples/)

