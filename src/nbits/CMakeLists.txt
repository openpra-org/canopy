set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NBITS_HEADERS
        nbits.h
        bits8.h
        bits16.h
        bits32.h
        bits64.h
)

add_library(nbits STATIC ${NBITS_HEADERS})
set_target_properties(nbits PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS nbits
        RUNTIME DESTINATION lib/canopy)

