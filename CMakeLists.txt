cmake_minimum_required (VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(canopy)

# Tell CMake where the modules are.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/_cmake")
message(DEBUG "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# Print compiler information
message(DEBUG "C compiler: ${CMAKE_C_COMPILER}")
message(DEBUG "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(DEBUG "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

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
        set(ACPP_DEBUG_LEVEL 2 CACHE STRING
                "Choose the debug level, options are: 0 (no debug), 1 (print errors), 2 (also print warnings), 3 (also print general information)"
                FORCE)
    endif()
endif()

#add_compile_definitions(HIPSYCL_DEBUG_LEVEL="${HIPSYCL_DEBUG_LEVEL}")
#Use add_definitions for now for older cmake versions
cmake_policy(SET CMP0005 NEW)
add_definitions(-DHIPSYCL_DEBUG_LEVEL=${ACPP_DEBUG_LEVEL})

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif()

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

add_subdirectory(examples)