# Find_boost.cmake
# lodash wrapper module that locates the Boost library and associated include directories.

# CMake 3.30 - Address CMP0167 policy warning
# @link https://cmake.org/cmake/help/latest/policy/CMP0167.html#policy:CMP0167
#
# The FindBoost module is removed.
#
# CMake 3.29 and below provide a FindBoost module, but it needs constant updates to keep up with upstream Boost
# releases. Upstream Boost 1.70 and above provide a BoostConfig.cmake package configuration file.
# find_package(Boost CONFIG) finds the upstream package directly, without the find module.
#
# CMake 3.30 and above prefer to not provide the FindBoost module so that find_package(Boost) calls, without the CONFIG
# or NO_MODULE options, find the upstream BoostConfig.cmake directly. This policy provides compatibility for projects
# that have not been ported to use the upstream Boost package.
#
# The OLD behavior of this policy is for find_package(Boost) to load CMake's FindBoost module. The NEW behavior is for
# find_package(Boost) to search for the upstream BoostConfig.cmake.
#
# This policy was introduced in CMake version 3.30. It may be set by cmake_policy() or cmake_minimum_required().
# If it is not set, CMake warns, and uses OLD behavior.
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

if(NOT _boost_FOUND)
#    message(STATUS "Downloading and extracting boost library sources. This will take some time...")

    set(BOOST_INCLUDE_LIBRARIES icl dll exception math multi_index algorithm filesystem)
    set(BOOST_ENABLE_CMAKE ON)
    set(FETCHCONTENT_QUIET ON)
    include(FetchContent)
    FetchContent_Declare(
            Boost
            URL "https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz"
            URL_HASH SHA256=dcea50f40ba1ecfc448fdf886c0165cf3e525fef2c9e3e080b9804e8117b9694
            USES_TERMINAL_DOWNLOAD TRUE
            DOWNLOAD_NO_EXTRACT FALSE
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            GIT_SHALLOW TRUE
            GIT_PROGRESS ON
            OVERRIDE_FIND_PACKAGE TRUE # needed to find correct Boost
            EXCLUDE_FROM_ALL # compile only what you need
    )
    FetchContent_MakeAvailable(Boost)

    find_package(Boost 1.88.0 EXACT REQUIRED)

    if(Boost_FOUND)
        set(_boost_FOUND TRUE BOOL "Flag to indicate that Boost has been configured using this module")
        message(DEBUG "Boost found")
        set(Boost_USE_STATIC_LIBS   ON)
        set(Boost_USE_MULTITHREADED ON)
        message(DEBUG "Boost Include directory: ${Boost_INCLUDE_DIR}")
        message(DEBUG "Boost Library directories: ${Boost_LIBRARY_DIRS}")
    else()
        message(FATAL_ERROR "Boost not found. Please install Boost.")
    endif()

    if(NOT WIN32)
        set(Boost_USE_MULTITHREADED OFF)
    endif()
endif()
