set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BITWISE_SRC
        bar.h
        bit_array.h
        bit_array.cpp
        bit_macros.h
        bit_ops.h
        bit_ops.cpp
)

add_library(bitwise SHARED ${BITWISE_SRC})
