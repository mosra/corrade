#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

# On MSVC remove /W3, as we are replacing it with /W4. Could be removed as of
# 3.15 with this: https://cmake.org/cmake/help/latest/policy/CMP0092.html
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set_directory_properties(PROPERTIES
    CORRADE_CXX_STANDARD 11
    CORRADE_USE_PEDANTIC_FLAGS ON)

# On Windows enable UNICODE/_UNICODE macros to avoid accidentally passing UTF-8
# values to ANSI functions
if(CORRADE_TARGET_WINDOWS)
    add_definitions("-DUNICODE" "-D_UNICODE")
endif()

# Xxode-generated project files have a limitation that you can't create a
# library target from OBJECT sources only and you have to provide at least one
# file. Otherwise the generated library gets replaced with self-pointing
# symlink and the build then obviously fails with "File not found" error.
# Related: https://cmake.org/pipermail/cmake/2016-April/063178.html
#
# To fix this, the dummy.cpp is added to all targets which have just OBJECT
# sources. But only when using the Xcode generator, because on certain
# compilers such as Emscripten even compiling an empty file is a heavy
# operation due to Node.js, Java, Python and whatever other bloat running
# underneath.
if(CMAKE_GENERATOR STREQUAL Xcode)
    set(_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP ${PROJECT_SOURCE_DIR}/src/dummy.cpp)
else()
    set(_CORRADE_OBJECT_ONLY_TARGET_DUMMY_CPP )
endif()

add_subdirectory(Corrade)

# Corrade include dir for superprojects
set(CORRADE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

add_subdirectory(debuggers)
