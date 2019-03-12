#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>
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

# Oldest known CMake currently in use is 3.0.2 on Debian 8 (Jessie), but we
# can't use that since it has a bug in handling imported targets -- a target
# defined in a subdirectory (for example via
# find_package(Corrade REQUIRED PluginManager)) and used as a dependency of
# another target is not propagated correctly upwards, which results in errors
# saying Corrade::PluginManager is not found. Next oldest supported version is
# 3.5.2 (Ubuntu 16.04), but we don't need any features from that so let's stay
# on 3.1.
cmake_minimum_required(VERSION 3.1)

# As reported in https://github.com/mosra/corrade/issues/52 and then tracked
# down to https://gitlab.kitware.com/cmake/cmake/issues/18099, CMake < 3.12 has
# various issues with the new XCode, resulting in weird build failures in
# Corrade. CMake 3.12 fixes that by temporarily switching to the legacy build
# system for Xcode 10. Instead of failing later during the build, fail upfront
# with a clear message.
if(APPLE AND CMAKE_GENERATOR STREQUAL Xcode AND NOT XCODE_VERSION VERSION_LESS 10.0 AND CMAKE_VERSION VERSION_LESS 3.12.0)
    message(FATAL_ERROR "CMake < 3.12 and Xcode 10 have various compatibility issues, breaking the Corrade build. Please upgrade to CMake 3.12 to use Xcode 10.")
endif()

# Configuration fails on < 3.9 if only C++ is enabled (TestBigEndian macro)
if(NOT CMAKE_VERSION VERSION_LESS 3.9.0)
    set(LANG CXX)
endif()
project(Corrade ${LANG})

# Use folders for nice tree in Visual Studio and XCode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CMakeDependentOption)
option(MSVC2017_COMPATIBILITY "Enable compatibility mode for MSVC 2017 (might disable some features)" OFF)
option(MSVC2015_COMPATIBILITY "Enable compatibility mode for MSVC 2015 (might disable some features)" OFF)

option(WITH_INTERCONNECT "Build Interconnect library" ON)
option(WITH_PLUGINMANAGER "Build PluginManager library" ON)
option(WITH_TESTSUITE "Build TestSuite library" ON)
cmake_dependent_option(WITH_UTILITY "Build Utility library" ON "NOT WITH_INTERCONNECT;NOT WITH_PLUGINMANAGER;NOT WITH_TESTSUITE" ON)
cmake_dependent_option(WITH_RC "Build the corrade-rc utility" ON "NOT WITH_UTILITY" ON)

option(BUILD_DEPRECATED "Include deprecated API in the build" ON)
if(BUILD_DEPRECATED)
    set(CORRADE_BUILD_DEPRECATED 1)
endif()

option(BUILD_STATIC "Build static libraries (default are shared)" OFF)
cmake_dependent_option(BUILD_STATIC_PIC "Build static libraries with position-independent code" ON "BUILD_STATIC" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

set(CORRADE_INCLUDE_INSTALL_PREFIX "."
    CACHE STRING "Prefix where to put platform-independent include and other files")

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
if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_WINDOWS_RT OR CORRADE_TARGET_IOS OR CORRADE_TARGET_ANDROID)
    set(CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT 1)
endif()
if(CORRADE_TARGET_APPLE)
    option(TESTSUITE_TARGET_XCTEST "Build TestSuite tests compatible with Xcode XCTest" OFF)
    if(TESTSUITE_TARGET_XCTEST)
        set(CORRADE_TESTSUITE_TARGET_XCTEST 1)
    endif()
endif()
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

# If we're in a CMake subproject, find_package(Corrade) will be looking for
# these, so supply their in-source location to cache
if(CORRADE_TESTSUITE_TARGET_XCTEST)
    set(CORRADE_TESTSUITE_XCTEST_RUNNER ${PROJECT_SOURCE_DIR}/src/Corrade/TestSuite/XCTestRunner.mm.in CACHE INTERNAL "" FORCE)
elseif(CORRADE_TARGET_ANDROID)
    set(CORRADE_TESTSUITE_ADB_RUNNER ${PROJECT_SOURCE_DIR}/src/Corrade/TestSuite/AdbRunner.sh CACHE INTERNAL "" FORCE)
elseif(CORRADE_TARGET_EMSCRIPTEN)
    set(CORRADE_TESTSUITE_EMSCRIPTEN_RUNNER ${PROJECT_SOURCE_DIR}/src/Corrade/TestSuite/EmscriptenRunner.html.in CACHE INTERNAL "" FORCE)
endif()

# Detect and auto-enable compiler compatibility
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # The oldest known and supported GCC currently in use is 4.8.5 on CentOS 7
    # (needed for some server installations)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        message(FATAL_ERROR "Corrade cannot be used with GCC < 4.8.1. Sorry.")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
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
set(CORRADE_DATA_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/share/corrade)
set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/share/cmake/Corrade)
set(CORRADE_INCLUDE_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/include/Corrade)

# Library version
set(CORRADE_LIBRARY_VERSION 2.2)
set(CORRADE_LIBRARY_SOVERSION 2)

add_subdirectory(modules)
add_subdirectory(src)

# Build snippets as part of testing
if(BUILD_TESTS)
    add_subdirectory(doc/snippets)
endif()
