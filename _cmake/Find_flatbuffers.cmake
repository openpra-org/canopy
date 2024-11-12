if(NOT _flatbuffers_FOUND)
    #add_subdirectory(flatbuffers)
    if (NOT FLATBUFFERS_SRC_DIR)
        set(FLATBUFFERS_SRC_DIR "${CMAKE_SOURCE_DIR}/external/flatbuffers")
    endif ()
    if (NOT flatbuffers_DIR)
        set(flatbuffers_DIR "${CMAKE_SOURCE_DIR}/external/flatbuffers/CMake")
    endif ()

    # Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
    add_subdirectory(${FLATBUFFERS_SRC_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
            EXCLUDE_FROM_ALL)

    include_directories("${FLATBUFFERS_SRC_DIR}/include")

    # Now simply link against flatbuffers as needed to your already declared target.
    # The flatbuffers target carry header search path automatically if CMake > 2.8.11.
    #target_link_libraries(own_project_target PRIVATE flatbuffers)

    find_package(flatbuffers REQUIRED)
    if(flatbuffers_FOUND)
        set(_flatbuffers_FOUND TRUE BOOL "Flag to indicate that flatbuffers has been configured once using this module")
        message(STATUS "flatbuffers found")
    else()
        message(WARNING "flatbuffers not found. Please make sure it's in your PATH")
    endif()
endif()


