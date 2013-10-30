#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
#             Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 2.8.8)
project(Corrade)

include(CMakeDependentOption)
option(WITH_FIND_MODULE "Install FindCorrade.cmake module into CMake's module dir (might require admin privileges)" OFF)
option(GCC44_COMPATIBILITY "Enable compatibility mode for GCC 4.4 (might disable some features)" OFF)
option(GCC45_COMPATIBILITY "Enable compatibility mode for GCC 4.5 (might disable some features)" OFF)
option(GCC46_COMPATIBILITY "Enable compatibility mode for GCC 4.6 (might disable some features)" OFF)
option(GCC47_COMPATIBILITY "Enable compatibility mode for GCC 4.7 (might disable some features)" OFF)

option(BUILD_DEPRECATED "Include deprecated API in the build" ON)
if(BUILD_DEPRECATED)
    set(CORRADE_BUILD_DEPRECATED 1)
endif()

option(BUILD_STATIC "Build static libraries (default are shared)" OFF)
cmake_dependent_option(BUILD_STATIC_PIC "Build static libraries with position-independent code" OFF "BUILD_STATIC" OFF)
option(BUILD_TESTS "Build unit tests." OFF)

if(BUILD_TESTS)
    enable_testing()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL NaCl)
    set(CORRADE_TARGET_NACL 1)
    # Newlib toolchain supports only static linking
    if(NACL_TOOLCHAIN STREQUAL newlib)
        set(CORRADE_TARGET_NACL_NEWLIB 1)
        set(BUILD_STATIC ON)
    else()
        set(CORRADE_TARGET_NACL_GLIBC 1)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CORRADE_TARGET_EMSCRIPTEN 1)
    # It's meaningless to use dynamic libraries with Emscripten
    set(BUILD_STATIC ON)
endif()

if(BUILD_STATIC)
    set(CORRADE_BUILD_STATIC 1)
endif()

# Initialize macros etc.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CORRADE_UTILITY_LIBRARY CorradeUtility)
if(NOT CMAKE_CROSSCOMPILING)
    set(CORRADE_RC_EXECUTABLE corrade-rc)
else()
    find_program(CORRADE_RC_EXECUTABLE corrade-rc)
    if(NOT CORRADE_RC_EXECUTABLE)
        message(FATAL_ERROR "Native `corrade-rc` executable, which is needed when crosscompiling, was not found")
    endif()
endif()
if(BUILD_TESTS)
    set(CORRADE_TESTSUITE_LIBRARIES CorradeTestSuite CorradeUtility)
endif()

if(GCC44_COMPATIBILITY)
    set(CORRADE_GCC44_COMPATIBILITY 1)
    set(GCC45_COMPATIBILITY 1)
endif()

if(GCC45_COMPATIBILITY)
    set(CORRADE_GCC45_COMPATIBILITY 1)
    set(GCC46_COMPATIBILITY 1)
endif()

if(GCC46_COMPATIBILITY)
    set(CORRADE_GCC46_COMPATIBILITY 1)
    set(GCC47_COMPATIBILITY 1)
endif()

if(GCC47_COMPATIBILITY)
    set(CORRADE_GCC47_COMPATIBILITY 1)
endif()

include(UseCorrade)

# Installation paths
include(CorradeLibSuffix)
set(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/cmake/Corrade)
set(CORRADE_CMAKE_FIND_MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)
set(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)
