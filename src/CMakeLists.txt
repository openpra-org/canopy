include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(api)
add_subdirectory(mef)
add_subdirectory(io)
add_subdirectory(boolexpr)

#
# libraries
#
add_library(canopy
        ../include/canopy.h
        canopy.cpp
)

target_include_directories(canopy PRIVATE .)
target_include_directories(canopy PUBLIC ../include)

#target_link_libraries(canopy PUBLIC ggml)

if (BUILD_SHARED_LIBS)
    set_target_properties(canopy PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_compile_definitions(canopy PRIVATE CANOPY_BUILD)
    target_compile_definitions(canopy PUBLIC  CANOPY_SHARED)
endif()