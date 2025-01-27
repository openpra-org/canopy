cmake_minimum_required (VERSION 3.22)

set(CANOPY_VERSION_MAJOR 0)
set(CANOPY_VERSION_MINOR 0)
set(CANOPY_VERSION_PATCH 1)

set(CMAKE_C_COMPILER /usr/bin/clang-18)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(canopy VERSION ${CANOPY_VERSION_MAJOR}.${CANOPY_VERSION_MINOR}.${CANOPY_VERSION_PATCH} LANGUAGES CXX)

# Tell CMake where the modules are.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/_cmake")
message(DEBUG "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# Print compiler information
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

######################### Options ################################################## {{{
option(BUILD_TESTS "Build tests" ON)  # Influences CTest.
set(ALLOWED_MALLOC_TYPES "tcmalloc" "jemalloc" "malloc" CACHE STRING "Allowed memory allocator types: tcmalloc, jemalloc, malloc")
set(MALLOC_TYPE "malloc" CACHE STRING "Select the memory allocator type (tcmalloc, jemalloc, malloc)")
option(WITH_COVERAGE "Instrument for coverage analysis" OFF)
option(WITH_PROFILE "Instrument for performance profiling" OFF)
option(OPTIMIZE_FOR_NATIVE "Build with -march=native" ON)

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

#Use add_definitions for now for older cmake versions
cmake_policy(SET CMP0005 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Needed for Clang Tooling.

find_package(_adaptivecpp)
find_package(_openmp)
find_package(_boost)


function(CHECK_COMPILER_VERSION MIN_VERSION)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_VERSION)
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} Compiler version too old. Required minimum version: ${MIN_VERSION}")
    endif()
endfunction()

add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")  # Needed to print file paths.

# Proactively disable warnings in case Wall/Wextra are enabled outside.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-missing-field-initializers -Wno-string-plus-int")

# Strict debug flags for SCRAM targets (opt-in, must subscribe to quality checks explicitly).
# NOTE: This is a list unlike CMAKE_CXX_FLAGS.
set(SCRAM_CXX_FLAGS_DEBUG -Wall -Wextra -Werror -Wnon-virtual-dtor -Wold-style-cast)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-new-ttp-matching")  # TODO: Boost ICL failure.
    CHECK_COMPILER_VERSION("7.1")
    list(APPEND SCRAM_CXX_FLAGS_DEBUG
            -Wredundant-decls -Wcast-align -Wlogical-op -Wvla -Wuseless-cast -Wunreachable-code
            -Wshadow -Wpedantic -Wmissing-declarations
            -Wimplicit-fallthrough=0  # TODO: Consider explicit fallthrough.
    )

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    CHECK_COMPILER_VERSION("5.0")
    list(APPEND SCRAM_CXX_FLAGS_DEBUG -Wno-missing-braces -Wshadow -Wunused-exception-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-string-plus-int -Wno-unused-lambda-capture)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-enum-constexpr-conversion")

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    CHECK_COMPILER_VERSION("9.0")

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    CHECK_COMPILER_VERSION("18.0.1")
    # TODO: Warning with overload of private override.
    list(APPEND SCRAM_CXX_FLAGS_DEBUG -diag-disable=1125)
endif()

if(WIN32)
    list(APPEND SCRAM_CXX_FLAGS_DEBUG -Wno-error)
endif()

if(WITH_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

if(WITH_PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -fno-omit-frame-pointer")
endif()

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif()

if(OPTIMIZE_FOR_NATIVE)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
endif()

## set the memory allocator
# profiling on
if(WITH_PROFILE)
    if(NOT MALLOC_TYPE STREQUAL "malloc")
        message(WARNING "memory allocator: Type ${MALLOC_TYPE} disabled during profiling, using malloc")
    endif()
    message(STATUS "memory allocator: malloc")
    set(MALLOC "System Malloc")
    # profiling off
else()
    # non-system allocator requested
    if(NOT MALLOC_TYPE STREQUAL "malloc")
        # requested tcmalloc
        if(MALLOC_TYPE STREQUAL "tcmalloc")
            find_package(Tcmalloc)
            if(Tcmalloc_FOUND)
                list(APPEND LIBS ${Tcmalloc_LIBRARIES})
                set(MALLOC "TCMalloc")
            else ()
                message(FATAL_ERROR "memory allocator: Requested type tcmalloc not found")
            endif ()
            # requested jemalloc
        elseif (MALLOC_TYPE STREQUAL "jemalloc")
            find_package(JeMalloc)
            if(JEMALLOC_FOUND)
                list(APPEND LIBS ${JEMALLOC_LIBRARIES})
                set(MALLOC "JEMalloc")
            else ()
                message(FATAL_ERROR "memory allocator: Requested type jemalloc not found")
            endif ()
            # requested unsupported allocator
        else()
            message(FATAL_ERROR "memory allocator: Requested type ${MALLOC_TYPE} is invalid")
        endif ()
        # system allocator requested
    else()
        set(MALLOC "System Malloc")
    endif()
    message(STATUS "memory allocator: ${MALLOC_TYPE}")
endif()

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_BINARY_DIR}/include")

if(WIN32)
    set(CMAKE_SKIP_RPATH TRUE)
else()
    # Use, i.e. don't skip the full RPATH for the build tree.
    set(CMAKE_SKIP_BUILD_RPATH FALSE)

    # When building,
    # don't use the install RPATH already
    # (but later on when installing).
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    #set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/scram")
    #set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib/scram")

    # Add the automatically determined parts of the RPATH,
    # which point to directories outside the build tree
    # to the install RPATH.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

list(APPEND LIBS ${Boost_LIBRARIES})
find_package(LibXml2 REQUIRED)
list(APPEND LIBS ${LIBXML2_LIBRARIES})
list(APPEND LIBS ${CMAKE_DL_LIBS})

message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Libraries: ${LIBS}")
########################## End of find libraries ######################## }}}
########################## Begin includes ############################### {{{

include_directories(SYSTEM "${LIBXML2_INCLUDE_DIR}")
#
#include_directories("${CMAKE_SOURCE_DIR}")  # Include the core headers via "src".
#include_directories("src")  # Include the core headers via "src".
include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_BINARY_DIR}/include")

add_subdirectory(src)

if(BUILD_TESTS)
    include(CTest)
    find_package(_boost COMPONENTS unit_test_framework REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif()
