#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022
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

# Oldest known CMake currently in use is 3.0.2 on Debian 8 (Jessie). Next
# oldest supported version is 3.5.2 (Ubuntu 16.04), but we don't need any
# features from that, 3.4 is required in order to have RUNTIME_OUTPUT_DIRECTORY
# etc. properties accept generator expressions.
cmake_minimum_required(VERSION 3.4)

# As reported in https://github.com/mosra/corrade/issues/52 and then tracked
# down to https://gitlab.kitware.com/cmake/cmake/issues/18099, CMake < 3.12 has
# various issues with the new XCode, resulting in weird build failures in
# Corrade. CMake 3.12 fixes that by temporarily switching to the legacy build
# system for Xcode 10. Instead of failing later during the build, fail upfront
# with a clear message.
if(APPLE AND CMAKE_GENERATOR STREQUAL Xcode AND NOT XCODE_VERSION VERSION_LESS 10.0 AND CMAKE_VERSION VERSION_LESS 3.12.0)
    message(FATAL_ERROR "CMake < 3.12 and Xcode 10 have various compatibility issues, breaking the Corrade build. Please upgrade to CMake 3.12 to use Xcode 10.")
endif()

# Don't restrict INTERPROCEDURAL_OPTIMIZATION only for icc on Linux
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
# If CMAKE_AUTOMOC is set, all uses of corrade_add_resource() would otherwise
# complain on 3.10 that AUTOMOC is not processing GENERATED files
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()
# Superprojects can use just set(CORRADE_WITH_BLAH ON) without FORCE CACHE on
# 3.13+
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

project(Corrade CXX)

# Use folders for nice tree in Visual Studio and XCode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

include(CMakeDependentOption)

# Options that used to be unprefixed. New options shouldn't be added to this
# list.
set(_CORRADE_DEPRECATED_UNPREFIXED_OPTIONS
    WITH_INTERCONNECT
    WITH_MAIN
    WITH_PLUGINMANAGER
    WITH_TESTSUITE
    WITH_UTILITY
    WITH_RC
    MSVC_COMPATIBILITY
    MSVC2017_COMPATIBILITY
    MSVC2015_COMPATIBILITY
    BUILD_DEPRECATED
    BUILD_MULTITHREADED
    BUILD_STATIC
    BUILD_STATIC_PIC
    BUILD_STATIC_UNIQUE_GLOBALS
    BUILD_TESTS
    TESTSUITE_TARGET_XCTEST
    UTILITY_USE_ANSI_COLORS)
# If during the first run (i.e., when the variable isn't in cache yet), check
# if any of the prefixed options are already set. If so, we assume the user is
# already switched to the prefixed options and won't accept the deprecated
# unprefixed options for backwards compatibility. This way it's possible for
# projects to reuse these variables for other purposes without affecting
# Corrade in any way.
if(NOT DEFINED _CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS)
    set(_CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS ON CACHE INTERNAL "")
    foreach(option ${_CORRADE_DEPRECATED_UNPREFIXED_OPTIONS})
        if(DEFINED CORRADE_${option})
            set(_CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS OFF CACHE INTERNAL "")
            break()
        endif()
    endforeach()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # We want to present the option to the user, but we also want to implicitly
    # enable it during the first run, unless manually overriden. Thus we first
    # remember if it was defined, then define the option() and then below check
    # the remembered state. Also checking the deprecated unprefixed name for
    # backwards compatibility.
    if(NOT DEFINED CORRADE_MSVC_COMPATIBILITY AND NOT DEFINED MSVC_COMPATIBILITY)
        set(_CORRADE_MSVC_COMPATIBILITY_NOT_DEFINED ON)
    endif()
    option(CORRADE_MSVC_COMPATIBILITY "Enable compatibility mode for MSVC 2019+ without /permissive- set (might disable some features)" OFF)

    option(CORRADE_MSVC2017_COMPATIBILITY "Enable compatibility mode for MSVC 2017 (might disable some features)" OFF)
    option(CORRADE_MSVC2015_COMPATIBILITY "Enable compatibility mode for MSVC 2015 (might disable some features)" OFF)
endif()

option(CORRADE_WITH_INTERCONNECT "Build Interconnect library" ON)
option(CORRADE_WITH_PLUGINMANAGER "Build PluginManager library" ON)
option(CORRADE_WITH_TESTSUITE "Build TestSuite library" ON)
cmake_dependent_option(CORRADE_WITH_UTILITY "Build Utility library" ON "NOT CORRADE_WITH_INTERCONNECT;NOT CORRADE_WITH_PLUGINMANAGER;NOT CORRADE_WITH_TESTSUITE" ON)
cmake_dependent_option(CORRADE_WITH_MAIN "Build Main library" ON "NOT CORRADE_WITH_TESTSUITE" ON)
cmake_dependent_option(CORRADE_WITH_RC "Build the corrade-rc utility" ON "NOT CORRADE_WITH_UTILITY" ON)

option(CORRADE_BUILD_DEPRECATED "Include deprecated API in the build" ON)

option(CORRADE_BUILD_MULTITHREADED "Build in a way that makes it possible to safely use certain Corrade features simultaneously in multiple threads" ON)

option(CORRADE_BUILD_STATIC "Build static libraries (default are shared)" OFF)
# Disable PIC on Emscripten by default (but still allow it to be enabled
# explicitly if one so desired). Currently causes linker errors related to
# __memory_base etc.: https://github.com/emscripten-core/emscripten/issues/8761
if(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(ON_EXCEPT_EMSCRIPTEN OFF)
else()
    set(ON_EXCEPT_EMSCRIPTEN ON)
endif()
cmake_dependent_option(CORRADE_BUILD_STATIC_PIC "Build static libraries with position-independent code" ${ON_EXCEPT_EMSCRIPTEN} "CORRADE_BUILD_STATIC" OFF)
cmake_dependent_option(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS "Build static libraries with globals unique across shared libraries" ${ON_EXCEPT_EMSCRIPTEN} "CORRADE_BUILD_STATIC" OFF)
option(CORRADE_BUILD_TESTS "Build unit tests" OFF)

# Option-independent platform discovery
if(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CORRADE_TARGET_EMSCRIPTEN 1)
    # It's meaningless to use dynamic libraries with Emscripten
    set(CORRADE_BUILD_STATIC ON)
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
        set(CORRADE_BUILD_STATIC ON)
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
    option(CORRADE_TESTSUITE_TARGET_XCTEST "Build TestSuite tests compatible with Xcode XCTest" OFF)
endif()
if(CORRADE_TARGET_WINDOWS)
    if(CORRADE_TARGET_WINDOWS_RT)
        set(CORRADE_UTILITY_USE_ANSI_COLORS ON)
    else()
        option(CORRADE_UTILITY_USE_ANSI_COLORS "Use ANSI escape sequences for colored Debug output on Windows" OFF)
    endif()
endif()

# Check if we can use IFUNC for CPU dispatch. Linux with glibc and Android with
# API 18+ has it, but e.g. Alpine Linux with musl doesn't, and on Android with
# API < 30 we don't get AT_HWCAP passed into the resolver and can't call
# getauxval() ourselves because it's too early at that point, which makes it
# pretty useless. Plus it also needs a certain binutils version and a capable
# compiler, so it's easiest to just verify the whole thing.
if(CORRADE_TARGET_UNIX)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("\
int fooImplementation() { return 42; }
#if defined(__ANDROID_API__) && __ANDROID_API__ < 30
#error need Android API 30+ to have AT_HWCAP passed into the resolver
#endif
extern \"C\" int(*fooDispatcher())() {
    return fooImplementation;
}
int foo() __attribute__((ifunc(\"fooDispatcher\")));
int main() { return foo() - 42; }\
        " _CORRADE_CPU_CAN_USE_IFUNC)
    # In cases where ifunc is known to be broken, disable it by default --
    # users can still force it to be enabled, but the initial state should not
    # cause crashes or other strange behavior.
    if(_CORRADE_CPU_CAN_USE_IFUNC)
        set(_CORRADE_CPU_USE_IFUNC_DEFAULT ON)
        # On GCC 4.8, if --coverage or -fprofile-arcs is enabled, the ifunc
        # dispatchers cause a segfault. On Ubuntu 20.04 at least. Not the case
        # with GCC 5 there, not the case with GCC 4.8 on Arch. Can't find any
        # upstream bug report or commit that would be related to this.
        if(CMAKE_CXX_COMPILER_ID AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9" AND CMAKE_CXX_FLAGS MATCHES "(--coverage|-fprofile-arcs)")
            if(NOT DEFINED CORRADE_CPU_USE_IFUNC)
                message(WARNING "Disabling CORRADE_CPU_USE_IFUNC by default as it may crash when used together with --coverage on GCC 4.8.")
            endif()
            set(_CORRADE_CPU_USE_IFUNC_DEFAULT OFF)
        endif()

        # If sanitizers are enabled, call into the dispatch function crashes.
        # Upstream bugreport https://github.com/google/sanitizers/issues/342
        # suggests using __attribute__((no_sanitize_address)), but that doesn't
        # work / can't be used because it would mean marking basically
        # everything including the actual implementation that's being
        # dispatched to.
        #
        # Sanitizers can be also enabled any other way such as with
        # CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER or with per-target properties
        # which isn't easy to detect, so as a secondary measure there's a kill
        # switch in Corrade/configure.h that will fail with an error if a
        # sanitizer is detected and CORRADE_CPU_USE_IFUNC is enabled.
        #
        # While Clang and GCC use -fsanitize=whatever, MSVC allows also
        # /fsanitize=, so catch both.
        if(CMAKE_CXX_FLAGS MATCHES "[-/]fsanitize=")
            if(NOT DEFINED CORRADE_CPU_USE_IFUNC)
                message(WARNING "Disabling CORRADE_CPU_USE_IFUNC by default as it crashes when used together with sanitizers. See https://github.com/google/sanitizers/issues/342 for more information.")
            endif()
            set(_CORRADE_CPU_USE_IFUNC_DEFAULT OFF)
        endif()
    else()
        set(_CORRADE_CPU_USE_IFUNC_DEFAULT OFF)
    endif()
else()
    set(_CORRADE_CPU_CAN_USE_IFUNC OFF)
    set(_CORRADE_CPU_USE_IFUNC_DEFAULT OFF)
endif()
cmake_dependent_option(CORRADE_CPU_USE_IFUNC "Allow using GNU IFUNC for runtime CPU dispatch" ${_CORRADE_CPU_USE_IFUNC_DEFAULT} "_CORRADE_CPU_CAN_USE_IFUNC" OFF)

# Runtime CPU dispatch. Because going through a function pointer may have
# negative perf consequences, enable it by default only on platforms that have
# IFUNC, and thus can avoid the function pointer indirection.
option(CORRADE_BUILD_CPU_RUNTIME_DISPATCH "Build with runtime dispatch for CPU-dependent functionality" ${_CORRADE_CPU_CAN_USE_IFUNC})
# Enabled by default and independent of CORRADE_BUILD_CPU_RUNTIME_DISPATCH as
# correctness is important. It can however negatively affect performance
# compared to IFUNC or compile-time dispatch, so make it possible to disable
# this for benchmarks.
cmake_dependent_option(CORRADE_BUILD_TESTS_FORCE_CPU_POINTER_DISPATCH "Force pointer-based dispatch for unit tests to verify all variants of CPU-dependent functionality" ON "CORRADE_BUILD_TESTS" OFF)

# Pass -msimd128 to tests to make it possible to verify also the WASM SIMD
# functionality even if the actual library is built without. I thought it would
# be a no-brainer, however *actual* finalized SIMD128 requires final Clang 13
# (such version is reported since emsdk 2.0.13, but the actual final version
# containing all intrinsics is only in emsdk 2.0.18), and Node.js 15+, which
# contains V8 8.6:
#   https://github.com/nodejs/node/blob/main/doc/changelogs/CHANGELOG_V15.md#2020-10-20-version-1500-current-bethgriggs
# Final bitmask instructions were added in 8.5, Node.js 14 has 8.4:
#   https://github.com/v8/v8/commit/aa5bcc09bf7d5d77056e6033918d79c30aa3f49a
# However, even now, in the midst of 2022 wildfires, emsdk still bundles
# Node.js 14.18 and there's no hope of it being upgraded any time soon:
#   https://github.com/emscripten-core/emsdk/pull/877
#   https://github.com/emscripten-core/emsdk/issues/947
#   https://github.com/emscripten-core/emsdk/issues/1064
# So instead of having -msimd128 always, I only enable it implicitly if Node.js
# 15 is found, and then CORRADE_ENABLE_SIMD128 / CORRADE_TARGET_SIMD128 gets
# enabled only if there's Clang 13 but also Emscripten 2.0.18+. What a mess.
if(CORRADE_TARGET_EMSCRIPTEN)
    set(ENABLE_IF_NODEJS_15 OFF)
    if(CORRADE_BUILD_TESTS)
        find_package(NodeJs REQUIRED QUIET)
        if(NOT NodeJs_VERSION VERSION_LESS 15.0)
            set(ENABLE_IF_NODEJS_15 ON)
        elseif(NOT DEFINED CORRADE_BUILD_TESTS_FORCE_WASM_SIMD128)
            message(WARNING "Disabling CORRADE_BUILD_TESTS_FORCE_WASM_SIMD128 by default as finalized WebAssembly SIMD support is only since Node.js 15 but found version ${NodeJs_VERSION}")
        endif()
    endif()

    cmake_dependent_option(CORRADE_BUILD_TESTS_FORCE_WASM_SIMD128 "Force -msimd128 for unit tests to verify also WASM SIMD variants of CPU-dependent functionality" ${ENABLE_IF_NODEJS_15} "CORRADE_BUILD_TESTS_FORCE_CPU_POINTER_DISPATCH" OFF)
endif()

# Backwards compatibility for unprefixed CMake options. If the user isn't
# explicitly using prefixed options in the first run already, accept the
# unprefixed options, and remember this decision for subsequent runs
if(NOT DEFINED _CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS)
    set(_CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS ON CACHE INTERNAL "")
endif()
# If the user wasn't explicitly using prefixed options in the first run and the
# CORRADE_BUILD_DEPRECATED option is not currently disabled (which can get
# changed subsequently), accept the unprefixed options and print a warning if
# they're different from the prefixed ones.
if(_CORRADE_ACCEPT_DEPRECATED_UNPREFIXED_OPTIONS AND CORRADE_BUILD_DEPRECATED)
    # BUILD_STATIC_PIC and BUILD_STATIC_UNIQUE_GLOBALS need extra care, as they
    # should be implicitly set to ON if BUILD_STATIC is set. Doing this before
    # propagating the unprefixed options to avoid a false-positive warning
    # when e.g. CORRADE_BUILD_STATIC_PIC is implicitly ON but BUILD_STATIC_PIC
    # not yet.
    if(BUILD_STATIC)
        if(NOT CORRADE_TARGET_EMSCRIPTEN AND NOT DEFINED BUILD_STATIC_PIC)
            set(BUILD_STATIC_PIC ON)
        endif()
        if(NOT CORRADE_TARGET_EMSCRIPTEN AND NOT DEFINED BUILD_STATIC_UNIQUE_GLOBALS)
            set(BUILD_STATIC_UNIQUE_GLOBALS ON)
        endif()
    endif()

    set(_CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION )
    foreach(option ${_CORRADE_DEPRECATED_UNPREFIXED_OPTIONS})
        if(DEFINED ${option})
            # CMake has no comparison of boolean values (EQUAL returns false if
            # comparing ON and 1 or OFF and FALSE, STREQUAL also), so we have
            # to do it this way. Also warn only on the first encountered
            # variable so people can fix it, reconfigure and go to the next one
            # that warns.
            if((${option} AND NOT CORRADE_${option}) OR
               (NOT ${option} AND CORRADE_${option}) AND NOT _CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION)
                set(_CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION ${option})
            endif()
            set(CORRADE_${option} ${${option}})
            # If variables specified on the command line don't match any
            # options, they're kept in cache but set as UNINITIALIZED, meaning
            # they don't appear in cmake-gui or ccmake, so there's no way to
            # fix the warning apart from hand-enditing the CMakeCache.txt or
            # recreating the build dir. Update their cached type to be BOOL to
            # make them appear.
            set(${option} ${${option}} CACHE BOOL "Deprecated, use CORRADE_${option} instead" FORCE)
        endif()
    endforeach()

    if(_CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION)
        # CMake 3.5+ has deprecation warnings enabled by default (which makes
        # sense), 3.4 not. Use a warning there instead.
        # TODO: drop when 3.4 is not supported anymore
        if(CMAKE_VERSION VERSION_LESS 3.5)
            set(DEPRECATION_OR_WARNING WARNING)
        else()
            set(DEPRECATION_OR_WARNING DEPRECATION)
        endif()
        message(${DEPRECATION_OR_WARNING} "Unprefixed options such as ${_CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION} are deprecated, use CORRADE_${_CORRADE_WARN_DEPRECATED_UNPREFIXED_OPTION} instead. Delete the unprefixed variable from CMake cache or set both to the same value to silence this warning.")
    endif()
endif()

# Initialize variables, macros etc.
if(CORRADE_BUILD_TESTS)
    if(CORRADE_TARGET_IOS)
        set(CORRADE_TESTSUITE_BUNDLE_IDENTIFIER_PREFIX "cz.mosra.corrade")
    endif()
    enable_testing()

    # If CORRADE_TESTSUITE_TEST_TARGET is set, tests aren't built by default
    # (in the ALL target) but instead set as dependencies of a target named
    # after the value of CORRADE_TESTSUITE_TEST_TARGET. This is a copy of
    # what's done in corrade_add_test(), because we also build various test
    # libraries and plugins in addition to the test executables.
    if(CORRADE_TESTSUITE_TEST_TARGET)
        if(NOT TARGET ${CORRADE_TESTSUITE_TEST_TARGET})
            add_custom_target(${CORRADE_TESTSUITE_TEST_TARGET})
        endif()
        set(EXCLUDE_FROM_ALL_IF_TEST_TARGET EXCLUDE_FROM_ALL)
    endif()
endif()

if(CMAKE_CROSSCOMPILING)
    find_program(CORRADE_RC_EXECUTABLE corrade-rc)
    if(NOT CORRADE_RC_EXECUTABLE)
        message(FATAL_ERROR "Native `corrade-rc` executable, which is needed when crosscompiling, was not found")
    endif()
    # Strictly speaking the GLOBAL isn't needed, but when crosscompiling with
    # Corrade as a CMake subproject and find_package(Corrade REQUIRED rc) (or
    # Utility) isn't present for whatever reason, corrade_add_resource() called
    # outside of Corrade itself would fail on the rc target not existing. We
    # could require everyone to call find_package() to fix this one particular
    # issue but that would be kinda mean. Don't be mean. The find_package()
    # call is still recommended as it unifies the external and subproject
    # workflows but it shouldn't be *required*.
    add_executable(Corrade::rc IMPORTED GLOBAL)
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
        if(NOT CORRADE_MSVC2015_COMPATIBILITY)
            set(CORRADE_MSVC2015_COMPATIBILITY ON)
            message(WARNING "MSVC 2015 detected, automatically enabling CORRADE_MSVC2015_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.20")
        if(NOT CORRADE_MSVC2017_COMPATIBILITY)
            set(CORRADE_MSVC2017_COMPATIBILITY ON)
            message(WARNING "MSVC 2017 detected, automatically enabling CORRADE_MSVC2017_COMPATIBILITY. Note that some features may not be available with this compiler.")
        endif()
    # CORRADE_MSVC_COMPATIBILITY is enforced by CORRADE_MSVC2017_COMPATIBILITY
    # and CORRADE_MSVC2015_COMPATIBILITY below, but can be disabled for 2019+.
    elseif(_CORRADE_MSVC_COMPATIBILITY_NOT_DEFINED)
        set(CORRADE_MSVC_COMPATIBILITY ON CACHE BOOL "Enable compatibility mode for MSVC 2019+ without /permissive- set (might disable some features)" FORCE)
        message(WARNING "MSVC 2019+ detected, automatically enabling CORRADE_MSVC_COMPATIBILITY which may cause some features to not be available. You can disable this option if you pass /permissive- to the compiler to enable a standards-conforming mode.")
    endif()

    if(CORRADE_MSVC2017_COMPATIBILITY)
        set(CORRADE_MSVC_COMPATIBILITY ON)
    endif()

    if(CORRADE_MSVC2015_COMPATIBILITY)
        set(CORRADE_MSVC2017_COMPATIBILITY ON)
        set(CORRADE_MSVC_COMPATIBILITY ON)
    endif()
endif()

include(UseCorrade)

# Installation paths
include(CorradeLibSuffix)
set(CORRADE_BINARY_INSTALL_DIR bin)
set(CORRADE_LIBRARY_INSTALL_DIR lib${LIB_SUFFIX})
set(CORRADE_DATA_INSTALL_DIR share/corrade)
set(CORRADE_CMAKE_MODULE_INSTALL_DIR share/cmake/Corrade)
set(CORRADE_INCLUDE_INSTALL_DIR include/Corrade)
# Prefix the non-binary paths with the obsolete CORRADE_INCLUDE_INSTALL_PREFIX,
# if set
if(CORRADE_BUILD_DEPRECATED AND CORRADE_INCLUDE_INSTALL_PREFIX AND NOT CORRADE_INCLUDE_INSTALL_PREFIX STREQUAL ".")
    message(DEPRECATION "CORRADE_INCLUDE_INSTALL_PREFIX is obsolete as its primary use was for old Android NDK versions. Please switch to the NDK r19+ layout instead of using this variable and recreate your build directory to get rid of this warning.")
    set(CORRADE_DATA_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/${CORRADE_DATA_INSTALL_DIR})
    set(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/${CORRADE_CMAKE_MODULE_INSTALL_DIR})
    set(CORRADE_INCLUDE_INSTALL_DIR ${CORRADE_INCLUDE_INSTALL_PREFIX}/${CORRADE_INCLUDE_INSTALL_DIR})
endif()

# Library version. CORRADE_VERSION_YEAR/MONTH is used in
# src/Corrade/CMakeLists.txt to generate the version.h header.
set(CORRADE_LIBRARY_VERSION 2.4)
set(CORRADE_LIBRARY_SOVERSION 2)
set(CORRADE_VERSION_YEAR 2020)
set(CORRADE_VERSION_MONTH 6)

# A single output location. After a decade of saying NO THIS IS A NON-SOLUTION
# TO A NON-PROBLEM I reconsidered my views and enabled this, because:
#
# - On Windows (which don't have RPATH), this makes test execution finally
#   possible without having to install all the stuff first (including the
#   test-only libs, which is ugh).
# - With CMake subprojects, this makes it finally possible to use dynamic
#   plugins directly from the build dir (again without installing anything) ---
#   all plugins are put into the same place, so PluginManager has a single
#   place to look into; and thanks to the dynamic libraries being there as
#   well, this location can be automagically detected as relative to
#   Utility::Path::libraryLocation().
# - Thanks to the $<CONFIG> being part of the output path, you are always sure
#   you never accidentally mix up debug/release libraries when switching
#   CMAKE_BUILD_TYPE in an existing build dir.
#
# The runtime location is set to CMAKE_BINARY_DIR and not PROJECT_BINARY_DIR
# because have one runtime location per CMake subproject would not solve much
# either. If the user already provides CMAKE_RUNTIME_OUTPUT_DIRECTORY (even
# empty), it's respected and nothing is being done.
#
# Explicitly using a generator expression to ensure plugins are added to e.g.
# <CONFIG>/lib/corrade/fs/ instead of lib/corrade/fs/<CONFIG>. Also adding this
# to cache, making superprojects pick that up implicitly as well, without
# forcing them to explicitly mirror this setting.
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin CACHE PATH "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib CACHE PATH "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib CACHE PATH "" FORCE)
endif()

# Implicitly, corrade_add_test(), corrade_add_plugin() and other macros check
# for existence of Corrade::TestSuite, Corrade::PluginManager and other targets
# to avoid cryptic errors later during the build. However, when building
# Corrade itself, those may not be defined yet (for example Containers tests
# are added before the TestSuite subdirectory), however it will work correctly
# during the actual build and thus such errors would be wrong.
set(_CORRADE_USE_NO_TARGET_CHECKS 1)

add_subdirectory(modules)
add_subdirectory(src)

# Build snippets as part of testing. Unlike all other Test/ directories, this
# one isn't added with ${EXCLUDE_FROM_ALL_IF_TEST_TARGET} because the targets
# from there aren't referenced by CTest and thus it'd likely happen that they
# accidentally don't get added as a dependency to that target, causing them to
# be never built. Instead, each target there is handled separately, to minimize
# the chance of an accident.
if(CORRADE_BUILD_TESTS)
    add_subdirectory(doc/snippets)
endif()
