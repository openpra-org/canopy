set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NBITS_HEADERS
        nbits.h
        bits8.h
        16bits.h
        32bits.h
        64bits.h
)

add_library(nbits STATIC ${NBITS_HEADERS})
set_target_properties(nbits PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS nbits
        RUNTIME DESTINATION lib/canopy)

