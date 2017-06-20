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

if(NOT BUILD_STATIC)
    set(SHARED_OR_STATIC SHARED)
else()
    set(SHARED_OR_STATIC STATIC)
endif()

# Assume Emscripten and iOS are little-endian (TODO: better check?)
if(NOT CORRADE_TARGET_EMSCRIPTEN AND NOT CORRADE_TARGET_IOS)
    include(TestBigEndian)
    test_big_endian(CORRADE_BIG_ENDIAN)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

set(Corrade_HEADERS
    Corrade.h)

# Force IDEs to display all header files in project view
add_custom_target(Corrade SOURCES ${Corrade_HEADERS})
set_target_properties(Corrade PROPERTIES FOLDER "Corrade")

install(FILES ${Corrade_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/configure.h DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR})

if(WITH_UTILITY) # Cyclic dependency of Containers and Utility
    add_subdirectory(Containers)
endif()

if(WITH_INTERCONNECT)
    add_subdirectory(Interconnect)
endif()

if(WITH_PLUGINMANAGER)
    add_subdirectory(PluginManager)
endif()

if(WITH_TESTSUITE)
    add_subdirectory(TestSuite)
endif()

if(WITH_UTILITY)
    add_subdirectory(Utility)
endif()

# Corrade configure file for superprojects
set(_CORRADE_CONFIGURE_FILE ${CMAKE_CURRENT_BINARY_DIR}/configure.h CACHE INTERNAL "")
