set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CANOPY_API_SOURCES
        canopy.h
)

add_library(canopy ${CANOPY_API_SOURCES})
set_target_properties(canopy PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS canopy
        RUNTIME DESTINATION lib/canopy)