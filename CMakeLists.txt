#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017 Vladimír Vondruš <mosra@centrum.cz>
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

cmake_minimum_required(VERSION 2.8.12)

# Configuration fails on < 3.9 if only C++ is enabled (TestBigEndian macro)
if(NOT CMAKE_VERSION VERSION_LESS 3.9.0)
    set(LANG CXX)
endif()
project(Corrade ${LANG})

# CMake policies: enable MACOSX_RPATH by default
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()
# Don't treat imported targets with :: as files
if(POLICY CMP0028)
    cmake_policy(SET CMP0028 NEW)
endif()

# Use folders for nice tree in Visual Studio and XCode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CMakeDependentOption)
option(GCC47_COMPATIBILITY "Enable compatibility mode for GCC 4.7 (might disable some features)" OFF)
option(MSVC2017_COMPATIBILITY "Enable compatibility mode for MSVC 2017 (might disable some features)" OFF)
option(MSVC2015_COMPATIBILITY "Enable compatibility mode for MSVC 2015 (might disable some features)" OFF)

option(WITH_INTERCONNECT "Build Interconnect library" ON)
option(WITH_PLUGINMANAGER "Build PluginManager library" ON)
option(WITH_TESTSUITE "Build TestSuite library" ON)
cmake_dependent_option(WITH_UTILITY "Build Utility library" ON "NOT WITH_INTERCONNECT;NOT WITH_PLUGINMANAGER;NOT WITH_TESTSUITE" ON)

option(BUILD_DEPRECATED "Include deprecated API in the build" ON)
if(BUILD_DEPRECATED)
    set(CORRADE_BUILD_DEPRECATED 1)
endif()

option(BUILD_STATIC "Build static libraries (default are shared)" OFF)
cmake_dependent_option(BUILD_STATIC_PIC "Build static libraries with position-independent code" ON "BUILD_STATIC" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    if(CORRADE_TARGET_IOS)
        set(CORRADE_TESTSUITE_BUNDLE_IDENTIFIER_PREFIX "cz.mosra.corrade")
    endif()
    enable_testing()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CORRADE_TARGET_EMSCRIPTEN 1)
    # It's meaningless to use dynamic libraries with Emscripten
    set(BUILD_STATIC ON)
elseif(UNIX)
    # Both APPLE and UNIX are defined on OSX
    if(APPLE)
        set(CORRADE_TARGET_APPLE 1)

        if(CMAKE_OSX_SYSROOT MATCHES "/iPhoneOS[0-9.]*\\.sdk")
            set(CORRADE_TARGET_IOS 1)
        elseif(CMAKE_OSX_SYSROOT MATCHES "/iPhoneSimulator[0-9.]*\\.sdk")
            set(CORRADE_TARGET_IOS 1)
            set(CORRADE_TARGET_IOS_SIMULATOR 1)
        endif()
    endif()

    # UNIX is also defined on Android
    if(CMAKE_SYSTEM_NAME STREQUAL Android)
        set(CORRADE_TARGET_ANDROID 1)
        # It's too inconvenient to manually load all shared libs using JNI
        set(BUILD_STATIC ON)
    endif()

    # Emscripten is Unix too, this selects only the other ones
    set(CORRADE_TARGET_UNIX 1)
elseif(WIN32)
    set(CORRADE_TARGET_WINDOWS 1)

    if(WINDOWS_PHONE OR WINDOWS_STORE)
        set(CORRADE_TARGET_WINDOWS_RT 1)
    endif()
endif()

# Platform-specific options
if(CORRADE_TARGET_WINDOWS)
    if(CORRADE_TARGET_WINDOWS_RT)
        set(UTILITY_USE_ANSI_COLORS 1)
    else()
        option(UTILITY_USE_ANSI_COLORS "Use ANSI escape sequences for colored Debug output on Windows" OFF)
    endif()
    if(UTILITY_USE_ANSI_COLORS)
        set(CORRADE_UTILITY_USE_ANSI_COLORS 1)
    endif()
endif()
if(CORRADE_TARGET_APPLE)
    option(TESTSUITE_TARGET_XCTEST "Build TestSuite tests compatible with Xcode XCTest" OFF)
    if(TESTSUITE_TARGET_XCTEST)
        set(CORRADE_TESTSUITE_TARGET_XCTEST 1)
    endif()
endif()

if(BUILD_STATIC)
    set(CORRADE_BUILD_STATIC 1)
endif()

# Initialize macros etc.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
if(CMAKE_CROSSCOMPILING)
    find_program(CORRADE_RC_EXECUTABLE corrade-rc)
    if(NOT CORRADE_RC_EXECUTABLE)
        message(FATAL_ERROR "Native `corrade-rc` executable, which is needed when crosscompiling, was not found")
    endif()
    add_executable(Corrade::rc IMPORTED)
    set_property(TARGET Corrade::rc PROPERTY IMPORTED_LOCATION ${CORRADE_RC_EXECUTABLE})
endif()
if(CORRADE_TESTSUITE_TARGET_XCTEST)
    set(CORRADE_TESTSUITE_XCTEST_RUNNER ${PROJECT_SOURCE_DIR}/src/Corrade/TestSuite/XCTestRunner.mm.in)
endif()
if(CORRADE_TARGET_ANDROID)
    set(CORRADE_TESTSUITE_ADB_RUNNER ${PROJECT_SOURCE_DIR}/src/Corrade/TestSuite/AdbRunner.sh)
endif()

# Detect and auto-enable compiler compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0")
        message(FATAL_ERROR "Corrade cannot be used with GCC < 4.7. Sorry.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        if(NOT GCC47_COMPATIBILITY)
            set(GCC47_COMPATIBILITY ON)
            message(WARNING "GCC < 4.8.1 detected, automatically enabling GCC47_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    endif()
elseif(MSVC)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.0")
        message(FATAL_ERROR "Corrade cannot be used with MSVC < 2015. Sorry.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10")
        if(NOT MSVC2015_COMPATIBILITY)
            set(MSVC2015_COMPATIBILITY ON)
            message(WARNING "MSVC 2015 detected, automatically enabling MSVC2015_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.20")
        if(NOT MSVC2017_COMPATIBILITY)
            set(MSVC2017_COMPATIBILITY ON)
            message(WARNING "MSVC 2017 detected, automatically enabling MSVC2017_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    endif()
endif()

if(GCC47_COMPATIBILITY)
    set(CORRADE_GCC47_COMPATIBILITY 1)
endif()

if(MSVC2017_COMPATIBILITY)
    set(CORRADE_MSVC2017_COMPATIBILITY 1)
endif()

if(MSVC2015_COMPATIBILITY)
    set(CORRADE_MSVC2015_COMPATIBILITY 1)
    set(CORRADE_MSVC2017_COMPATIBILITY 1)
endif()

if(TESTSUITE_TARGET_XCTEST)
    set(CORRADE_TESTSUITE_TARGET_XCTEST 1)
endif()

include(UseCorrade)
if(CORRADE_TARGET_EMSCRIPTEN)
    include(UseEmscripten)
endif()

# Installation paths
include(CorradeLibSuffix)
set(CORRADE_BINARY_INSTALL_DIR bin)
set(CORRADE_LIBRARY_INSTALL_DIR lib${LIB_SUFFIX})
set(CORRADE_DATA_INSTALL_DIR share/corrade)
set(CORRADE_CMAKE_MODULE_INSTALL_DIR share/cmake/Corrade)
set(CORRADE_INCLUDE_INSTALL_DIR include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 0.2)
set(CORRADE_LIBRARY_SOVERSION 0)

add_subdirectory(modules)
add_subdirectory(src)

# Build snippets as part of testing
if(BUILD_TESTS)
    add_subdirectory(doc/snippets)
endif()
