set(API_HEADERS
        mef.h
        io.h
)

add_library(api INTERFACE ${API_HEADERS})
set_target_properties(api PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(api
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # For usage from the build directory
        $<INSTALL_INTERFACE:include>                     # For usage after installation (e.g. /usr/local/include)
)

# self-contain the library headers for downstream usage:
#target_include_directories(api
#        PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:include>
#        PRIVATE
#        ${LIBXML2_INCLUDE_DIR}
#)

install(TARGETS api
        ARCHIVE DESTINATION lib/canopy
        LIBRARY DESTINATION lib/canopy
        RUNTIME DESTINATION bin
)

install(FILES
        ${API_HEADERS}
        DESTINATION include/canopy/api
)