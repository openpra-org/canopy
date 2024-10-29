set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BITWISE_SRC
        bar.h
        bar/bit_array.h
        bit_array.cpp
        bit_macros.h
        bit_ops.h
        bit_ops.cpp
        bar/bar_string.h
        bar/bar_shuffle.h
        bar/bar_constants.h
        bar/bar_region.h
        bar/bar_file.h
        bar/bar_shift.h
        bar/bar_compare.h
        bar/bar_getset.h
)

add_library(bitwise SHARED ${BITWISE_SRC})
