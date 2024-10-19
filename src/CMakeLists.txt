add_subdirectory(bool)
target_include_directories(bool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(sampler)
add_subdirectory(utils)

include_directories("${PROJECT_SOURCE_DIR}/src")