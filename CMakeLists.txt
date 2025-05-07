cmake_minimum_required (VERSION 3.22)

set(CANOPY_VERSION_MAJOR 0)
set(CANOPY_VERSION_MINOR 1)
set(CANOPY_VERSION_PATCH 0)

set(CMAKE_C_COMPILER /usr/bin/clang-18)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(canopy VERSION ${CANOPY_VERSION_MAJOR}.${CANOPY_VERSION_MINOR}.${CANOPY_VERSION_PATCH})

set(CMAKE_DISABLE_SOURCE_CHANGES ON) # In-source build prevention.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Tell CMake where the modules are.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/_cmake")
message(DEBUG "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
include("${CMAKE_MODULE_PATH}/helpers.cmake") # Include helper methods

# Print compiler information
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

######################### Options ################################################## {{{
option(BUILD_TESTS "Build tests" ON)  # Influences CTest.
option(BUILD_EXAMPLES "Build examples" ON)
option(CODE_COVERAGE "Perform code coverage analysis" OFF)
option(PORTABLE "Build without CPU specific optimizations" OFF)
option(ENABLE_LTO "Enable Link-Time Optimization (LTO)" ON)
option(ENABLE_PGO "Enable Profile-Guided Optimization (PGO)" OFF)
######################### Options ################################################## }}}

######################## Link-Time Optimization ############## {{{
set(CMAKE_POSITION_INDEPENDENT_CODE OFF) # OFF is ok for static libs
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO})
######################## Link-Time Optimization ############## }}}

######################## Profile-Guided Optimization ######### {{{
if(ENABLE_PGO)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-fprofile-generate)
        add_link_options(-fprofile-generate)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-fprofile-use -fprofile-correction)
        add_link_options(-fprofile-use -fprofile-correction)
    endif()
endif()
######################## Profile-Guided Optimization ######### }}}

######################## Non-Portable Optimizations ########## {{{
set(NON_PORTABLE_CXX_FLAGS)
mark_as_advanced(NON_PORTABLE_CXX_FLAGS)
CHECK_COMPILER_FLAG_SUPPORT("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE NON_PORTABLE_CXX_FLAGS)
CHECK_COMPILER_FLAG_SUPPORT("-xHost" COMPILER_SUPPORTS_XHOST NON_PORTABLE_CXX_FLAGS)

if (NOT PORTABLE)
    add_compile_options(${NON_PORTABLE_CXX_FLAGS})
endif ()
######################## Non-Portable Optimizations ########## }}}

# CMake 3.27 - Address CMP0144 policy warning
# @link https://cmake.org/cmake/help/latest/policy/CMP0167.html#policy:CMP0144
#
# find_package() uses upper-case <PACKAGENAME>_ROOT variables.
#
# In CMake 3.27 and above the find_package(<PackageName>) command now searches prefixes specified by the upper-case
# <PACKAGENAME>_ROOT CMake variable and the <PACKAGENAME>_ROOT environment variable in addition to the case-preserved
# <PackageName>_ROOT and <PackageName>_ROOT variables used since policy CMP0074. This policy provides compatibility with
# projects that have not been updated to avoid using <PACKAGENAME>_ROOT variables for other purposes.
#
# The OLD behavior for this policy is to ignore <PACKAGENAME>_ROOT variables if the original <PackageName> has
# lower-case characters. The NEW behavior for this policy is to use <PACKAGENAME>_ROOT variables.
#
# This policy was introduced in CMake version 3.27. It may be set by cmake_policy() or cmake_minimum_required(). If it
# is not set, CMake warns, and uses OLD behavior.
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

find_package(_openmp)
#list(APPEND LIBS ${LIBXML2_LIBRARIES})
#find_package(_mkl)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake)

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

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_BINARY_DIR}/include")

if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

add_subdirectory(src)