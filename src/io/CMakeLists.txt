set(IO_HEADERS
        json.h
        parser.h
        pla.h
)

set(IO_SOURCES
        stub.cpp
)

find_package(_boost)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})

# Link io against Boost libraries
target_link_libraries(io flatbuffers)

install(TARGETS io
        RUNTIME DESTINATION share/canopy/lib/io)