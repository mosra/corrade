cmake_minimum_required(VERSION 2.6)

project(Corrade)

option(BUILD_TESTS "Build unit tests (requires Qt4)." OFF)

if(BUILD_TESTS)
    find_package(Qt4)

    if(NOT QT4_FOUND)
        message(FATAL_ERROR "Qt4, required for building unit tests, was not found. Set BUILD_TESTS to OFF to skip building them.")
    endif()

    enable_testing()
endif()

set(CMAKE_MODULE_PATH "${Corrade_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

set(CORRADE_RC_EXECUTABLE corrade-rc)
include(CorradeMacros)
include(CorradeLibSuffix)

set(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)
set(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

# On 64bit Linux with GCC flag -fPIC is needed for building some static libraries
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCC AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(USE_FPIC ON)
else()
    set(USE_FPIC OFF)
endif()

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)

install(DIRECTORY src/ DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*/Test" EXCLUDE
        PATTERN "*/doc" EXCLUDE)
