if(NOT nlohmann_json_FOUND)
    include(FetchContent)

    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        55f93686c01528224f448c19128836e7df245f72 # v3.12.0
    )
    FetchContent_MakeAvailable(nlohmann_json)
    set(nlohmann_json_FOUND TRUE BOOL "Flag to indicate that nlohmann_json has been configured once using this module")
    #set(nlohmann_json_INCLUDE_DIR "${nlohmann_json_SOURCE_DIR}/include")
    message(DEBUG "nlohmann_json found at: ${nlohmann_json_SOURCE_DIR}")
    message(DEBUG "To use nlohmann_json, add the following to your target:")
    message(DEBUG "  target_link_libraries(your_target PRIVATE json)")
    message(DEBUG "  target_include_directories(your_target PRIVATE \${_json_INCLUDE_DIR})")
endif()




