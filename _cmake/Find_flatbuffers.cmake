if(NOT _flatbuffers_FOUND)
    include(FetchContent)

    # Optionally build flatc
    set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
            flatbuffers
            GIT_REPOSITORY https://github.com/google/flatbuffers.git
            GIT_TAG        1c514626e83c20fffa8557e75641848e1e15cd5e # v25.2.10
    )
    FetchContent_MakeAvailable(flatbuffers)

    set(_flatbuffers_FOUND TRUE BOOL "Flag to indicate that flatbuffers has been configured once using this module")
    set(_flatbuffers_INCLUDE_DIRS "${flatbuffers_SOURCE_DIR}/include")
    message(DEBUG "flatbuffers found at: ${flatbuffers_SOURCE_DIR}")
    message(DEBUG "To use flatbuffers, add the following to your target:")
    message(DEBUG "  target_link_libraries(your_target PRIVATE flatbuffers)")
    message(DEBUG "  target_include_directories(your_target PRIVATE \${_flatbuffers_INCLUDE_DIR})")
endif()


