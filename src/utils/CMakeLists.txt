set(UTILS_HEADERS
        json.h
)

set(UTILS_SOURCES
        stub.cpp
)

add_library(utils STATIC ${UTILS_HEADERS} ${UTILS_SOURCES})

install(TARGETS utils
        RUNTIME DESTINATION share/canopy/lib/utils)
