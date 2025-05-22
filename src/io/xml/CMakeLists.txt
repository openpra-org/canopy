find_package(LibXml2 REQUIRED)
find_package(_boost REQUIRED)

set(IO_XML_HEADERS
        xml.h
        xml_error.h
)

set(IO_XML_SOURCES
        xml.cpp
)

add_library(io_xml STATIC ${IO_XML_SOURCES} ${IO_XML_HEADERS})
set_target_properties(io_xml PROPERTIES LINKER_LANGUAGE CXX)

# Link io_xml against libraries
target_include_directories(io_xml PRIVATE ${LIBXML2_INCLUDE_DIR})
target_link_libraries(io_xml PRIVATE Boost::exception Boost::iterator Boost::range LibXml2::LibXml2)

install(TARGETS io_xml
        RUNTIME DESTINATION lib/canopy)