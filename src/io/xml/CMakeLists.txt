find_package(LibXml2 REQUIRED)

set(IO_XML_HEADERS
        document.h
        error.h
        element.h
        helpers.h
        range.h
        validator.h
)

set(IO_XML_SOURCES
        document.cpp
        element.cpp
        error.cpp
        range.cpp
        validator.cpp
)

add_library(io_xml STATIC ${IO_XML_SOURCES} ${IO_XML_HEADERS})

# Link io_xml against libraries
## libxml2 usage is strictly internal, keep it PRIVATE
target_link_libraries(io_xml PRIVATE LibXml2::LibXml2)

# self-contain the library headers for downstream usage:
target_include_directories(io_xml
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${LIBXML2_INCLUDE_DIR}
)

install(TARGETS io_xml
        ARCHIVE DESTINATION lib/canopy
        LIBRARY DESTINATION lib/canopy
        RUNTIME DESTINATION bin
)

install(FILES
        ${IO_XML_HEADERS}
        DESTINATION include/canopy/io/xml
)