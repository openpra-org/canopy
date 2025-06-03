add_library(boolexpr INTERFACE)

target_include_directories(boolexpr
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # For usage from the build directory
        $<INSTALL_INTERFACE:include>                     # For usage after installation (e.g. /usr/local/include)
)

target_link_libraries(boolexpr INTERFACE fmt::fmt)