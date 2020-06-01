#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
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

corrade_add_test(MainTest MainTest.cpp
    ARGUMENTS --arg-utf hýždě --arg-another šňůra)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)
corrade_add_test(TargetTest TargetTest.cpp)
target_include_directories(TargetTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(VersionTest VersionTest.cpp)

# Baseline, C++11, is everywhere
corrade_add_test(Cpp11StandardTest CppStandardTest.cpp)
set_target_properties(Cpp11StandardTest PROPERTIES
    COMPILE_DEFINITIONS COMPILING_AS_CPP11)

# Test for C++14:
# - GCC reports proper __cplusplus since 5.0 (had to test manually, thought
#   it's since 4.9, but nope, that one returns 201103, at least on Android)
# - should be in Clang since before 3.6 (i.e., always), but breaks with old
#   libstdc++, so I'm expecting at least 3.9 to exclude Travis sanitizer build
#   that uses 3.8
# - AppleClang 7.0 is equivalent to 3.6 and has libc++, so it should work on it
# - should be in MSVC since 2015 Update 3 (assume people patch their things)
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.9") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
    corrade_add_test(Cpp14StandardTest CppStandardTest.cpp)
    set_target_properties(Cpp14StandardTest PROPERTIES CORRADE_CXX_STANDARD 14)
    target_compile_definitions(Cpp14StandardTest PRIVATE
        COMPILING_AS_CPP14 "TEST_NAME=\"Cpp14StandardTest\"")

    # Test also clashes of corrade's own setter and CMake one
    corrade_add_test(Cpp14StandardTestCMakeStandard CppStandardTest.cpp)
    set_target_properties(Cpp14StandardTestCMakeStandard PROPERTIES
        CORRADE_CXX_STANDARD 17 # This should get ignored even tho it's higher
        CXX_STANDARD 14)
    target_compile_definitions(Cpp14StandardTestCMakeStandard PRIVATE
        COMPILING_AS_CPP14 "TEST_NAME=\"Cpp14StandardTestCMakeStandard\"")

    # cxx_std_* features are only since CMake 3.8
    if(NOT CMAKE_VERSION VERSION_LESS 3.8)
        corrade_add_test(Cpp14StandardTestCMakeFeatures CppStandardTest.cpp)
         # This should get ignored even tho it's higher
        set_target_properties(Cpp14StandardTestCMakeFeatures PROPERTIES CORRADE_CXX_STANDARD 17)
        target_compile_features(Cpp14StandardTestCMakeFeatures PRIVATE cxx_std_14)
        target_compile_definitions(Cpp14StandardTestCMakeFeatures PRIVATE
            COMPILING_AS_CPP14 "TEST_NAME=\"Cpp14StandardTestCMakeFeatures\"")
    endif()
endif()

# Test for C++17:
# - GCC reports proper __cplusplus since 7.0 (see
#   https://gcc.gnu.org/gcc-7/changes.html#cxx)
# - Clang reports proper __cplusplus since 5.0 (https://releases.llvm.org/5.0.0/tools/clang/docs/ReleaseNotes.html#id1)
# - AppleClang 9.3 is equivalent to 5.0.2 (https://en.wikipedia.org/wiki/Xcode#Latest_versions)
# - MSVC since 2017
# This same expression is used in doc/snippets/CMakeLists.txt,
# src/Corrade/Containers/Test/CMakeLists.txt and
# src/Corrade/Utility/Test/CMakeLists.txt, please keep in sync.
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
    corrade_add_test(Cpp17StandardTest CppStandardTest.cpp)
    set_target_properties(Cpp17StandardTest PROPERTIES
        CORRADE_CXX_STANDARD 17
        COMPILE_DEFINITIONS COMPILING_AS_CPP17)
endif()

# Test for C++2a (__cplusplus larger than for C++17):
# - GCC knows -std=c++2a since 8.0 (see
#   https://gcc.gnu.org/gcc-8/changes.html#cxx)
# - Clang knows -std=c++2a since 6.0 (https://releases.llvm.org/6.0.0/tools/clang/docs/ReleaseNotes.html#c-language-changes-in-clang)
# - AppleClang 10.0 is equivalent to 6.0 (https://en.wikipedia.org/wiki/Xcode#Latest_versions)
# - MSVC uses /std:c++latest for C++20 since 2017 19.15 (source?)
# This same expression is used in doc/snippets/CMakeLists.txt and
# src/Corrade/Containers/Test/CMakeLists.txt, please keep in sync.
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
    corrade_add_test(Cpp2aStandardTest CppStandardTest.cpp)
    set_target_properties(Cpp2aStandardTest PROPERTIES
        CORRADE_CXX_STANDARD 20
        COMPILE_DEFINITIONS COMPILING_AS_CPP2A)
endif()
