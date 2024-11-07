add_subdirectory(bool)
target_include_directories(bool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(bitwise)
add_subdirectory(utils)
add_subdirectory(io)
add_subdirectory(sampler)

include_directories("${PROJECT_SOURCE_DIR}/src")