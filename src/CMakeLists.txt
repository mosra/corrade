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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CORRADE_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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
endif()

include(TestBigEndian)
test_big_endian(CMAKE_BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/corradeConfigure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h)

set(Corrade_HEADERS
    Corrade.h
    corradeCompatibility.h)

install(FILES ${Corrade_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/corradeConfigure.h DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

add_subdirectory(Containers)
add_subdirectory(Interconnect)
add_subdirectory(PluginManager)
add_subdirectory(TestSuite)
add_subdirectory(Utility)
