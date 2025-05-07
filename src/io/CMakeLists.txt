set(IO_HEADERS
        parser.h
)

set(IO_SOURCES
        parser.cpp
)

find_package(nlohmann_json)

add_library(io STATIC ${IO_SOURCES} ${IO_HEADERS})
set_target_properties(io PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(io PRIVATE nlohmann_json)

# Link io against libraries
target_link_libraries(io PRIVATE nlohmann_json)

install(TARGETS io
        RUNTIME DESTINATION lib/canopy)

add_subdirectory(xml)
