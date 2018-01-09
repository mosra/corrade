#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018 Vladimír Vondruš <mosra@centrum.cz>
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
    set_target_properties(Cpp14StandardTest PROPERTIES
        CORRADE_CXX_STANDARD 14
        COMPILE_DEFINITIONS COMPILING_AS_CPP14)
endif()

# Test for C++17:
# - GCC reports proper __cplusplus since 7.0 (see
#   https://gcc.gnu.org/gcc-7/changes.html#cxx)
# - Clang had first C++17 feature in 4.0, which corresponds to AppleClang 9.0
#   (see https://gist.github.com/yamaya/2924292#gistcomment-2289472)
# - MSVC since 2017
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
    corrade_add_test(Cpp17StandardTest CppStandardTest.cpp)
    set_target_properties(Cpp17StandardTest PROPERTIES
        CORRADE_CXX_STANDARD 17
        COMPILE_DEFINITIONS COMPILING_AS_CPP17)
endif()
