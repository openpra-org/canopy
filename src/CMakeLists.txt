set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("${PROJECT_SOURCE_DIR}/src")

#add_subdirectory(api)
#add_subdirectory(cli)
#add_subdirectory(utils)

add_subdirectory(core)
add_subdirectory(scram)
add_subdirectory(scram-cli)
