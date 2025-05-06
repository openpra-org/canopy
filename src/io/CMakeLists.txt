add_subdirectory(xml)

set(IO_HEADERS
        json.h
        parser.h
        pla.h
)

set(IO_SOURCES

)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)

# Link io against libraries
target_link_libraries(io flatbuffers)

install(TARGETS io
        RUNTIME DESTINATION lib/canopy)