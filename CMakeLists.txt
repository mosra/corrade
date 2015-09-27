#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

cmake_minimum_required(VERSION 2.8.9)
project(Corrade)

include(CMakeDependentOption)
option(GCC47_COMPATIBILITY "Enable compatibility mode for GCC 4.7 (might disable some features)" OFF)
option(MSVC2015_COMPATIBILITY "Enable compatibility mode for MSVC 2015 (might disable some features)" OFF)

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
elseif(CMAKE_SYSTEM_NAME STREQUAL Android)
    set(CORRADE_TARGET_ANDROID 1)
    # It's too inconvenient to manually load all shared libs using JNI
    set(BUILD_STATIC ON)
elseif(UNIX)
    # Both APPLE and UNIX are defined on OSX
    if(APPLE)
        set(CORRADE_TARGET_APPLE 1)
    endif()

    # NaCl and Emscripten are Unix too, this selects only the other ones
    set(CORRADE_TARGET_UNIX 1)
elseif(WIN32)
    set(CORRADE_TARGET_WINDOWS 1)

    if(WINDOWS_PHONE OR WINDOWS_STORE)
        set(CORRADE_TARGET_WINDOWS_RT 1)
    endif()
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

# Detect and auto-enable compiler compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0")
        message(FATAL_ERROR "Compatibility branch of Corrade is needed for use with GCC < 4.7. See the documentation for more information.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        if(NOT GCC47_COMPATIBILITY)
            set(GCC47_COMPATIBILITY ON)
            message(WARNING "GCC < 4.8.1 detected, automatically enabling GCC47_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    endif()
elseif(MSVC)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.0")
        message(FATAL_ERROR "Compatibility branch of Corrade is needed for use with MSVC < 2015. See the documentation for more information.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "20.0")
        if(NOT MSVC2015_COMPATIBILITY)
            set(MSVC2015_COMPATIBILITY ON)
            message(WARNING "MSVC 2015 detected, automatically enabling MSVC2015_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    endif()
endif()

if(GCC47_COMPATIBILITY)
    set(CORRADE_GCC47_COMPATIBILITY 1)
endif()

if(MSVC2015_COMPATIBILITY)
    set(CORRADE_MSVC2015_COMPATIBILITY 1)
endif()

include(UseCorrade)

# Installation paths
include(CorradeLibSuffix)
set(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/cmake/Corrade)
set(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)
