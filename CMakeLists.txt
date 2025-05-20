cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project("canopy" LANGUAGES C CXX)
include(CheckIncludeFileCXX)

# Add path to modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Required for relocatable CMake package
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build-info.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/common-flags.cmake)

######################### Options ################################################## {{{
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)  # Influences CTest.
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
######################### Options ################################################## }}}

#
# build the library
#
add_subdirectory(src)

if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

#
# install
#
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CANOPY_BUILD_NUMBER        ${BUILD_NUMBER})
set(CANOPY_BUILD_COMMIT        ${BUILD_COMMIT})
set(CANOPY_INSTALL_VERSION 0.0.${BUILD_NUMBER})

set(CANOPY_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Location of header  files")
set(CANOPY_LIB_INSTALL_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Location of library files")
set(CANOPY_BIN_INSTALL_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Location of binary  files")

set(CANOPY_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/canopy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/canopy-cpp.h)

set_target_properties(canopy
        PROPERTIES
        PUBLIC_HEADER "${CANOPY_PUBLIC_HEADERS}")

install(TARGETS canopy LIBRARY PUBLIC_HEADER)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/canopy-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/canopy-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/canopy
        PATH_VARS CANOPY_INCLUDE_INSTALL_DIR
        CANOPY_LIB_INSTALL_DIR
        CANOPY_BIN_INSTALL_DIR )

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/canopy-version.cmake
        VERSION ${CANOPY_INSTALL_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/canopy-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/canopy-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/canopy)

configure_file(cmake/canopy.pc.in
        "${CMAKE_CURRENT_BINARY_DIR}/canopy.pc"
        @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/canopy.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)