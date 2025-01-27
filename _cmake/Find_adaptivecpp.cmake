find_package(AdaptiveCpp CONFIG REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT ACPP_DEBUG_LEVEL)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(ACPP_DEBUG_LEVEL 3 CACHE STRING
                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
                FORCE)
    else()
        set(ACPP_DEBUG_LEVEL 0 CACHE STRING
                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
                FORCE)
    endif()
endif()

add_definitions(-DHIPSYCL_DEBUG_LEVEL=${ACPP_DEBUG_LEVEL})

#Use add_definitions for now for older cmake versions
cmake_policy(SET CMP0005 NEW)

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif()

# Using accelerated C++ standard parallelism
#
# @link https://github.com/AdaptiveCpp/AdaptiveCpp/blob/develop/doc/stdpar.md
#
# Offloading of C++ standard parallelism is enabled using `--acpp-stdpar`. This flag does not by itself imply a target
# or compilation flow, which will have to be provided in addition using the normal --acpp-targets argument. C++ standard
# parallelism is expected to work with any of our clang compiler-based compilation flows, such as omp.accelerated, cuda,
# hip or the generic SSCP compiler (--acpp-targets=generic). It is not currently supported in library-only compilation
# flows. The focus of testing currently is the generic SSCP compiler. AdaptiveCpp by default uses some experimental
# heuristics to determine if a problem is worth offloading. These heuristics are currently very simplistic and might not
# work well for you. They can be disabled using --acpp-stdpar-unconditional-offload.
#
# Enable full feature profile for AdaptiveCpp
set(ACPP_COMPILER_FEATURE_PROFILE "full" CACHE STRING "AdaptiveCpp feature profile")
set(ACPP_STDPAR "on" CACHE STRING "enables SYCL offloading of C++ standard parallel algorithms")

add_definitions(-DACPP_COMPILER_FEATURE_PROFILE=full)
add_definitions(-DACPP_STDPAR=on)
