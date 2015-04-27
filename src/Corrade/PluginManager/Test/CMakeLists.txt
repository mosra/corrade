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

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

if(NOT CORRADE_TARGET_NACL_NEWLIB AND NOT CORRADE_TARGET_EMSCRIPTEN)
    corrade_add_plugin(Snail ${CMAKE_CURRENT_BINARY_DIR} "" Snail.conf Snail.cpp)
    corrade_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} "" Dog.conf Dog.cpp)
    corrade_add_plugin(Bulldog ${CMAKE_CURRENT_BINARY_DIR} "" Bulldog.conf Bulldog.cpp)
    corrade_add_plugin(Chihuahua ${CMAKE_CURRENT_BINARY_DIR} "" Chihuahua.conf Chihuahua.cpp)
endif()
corrade_add_static_plugin(Canary ${CMAKE_CURRENT_BINARY_DIR} Canary.conf Canary.cpp)

corrade_add_test(PluginManagerTest Test.cpp LIBRARIES CorradePluginManagerTestLib Canary)

target_link_libraries(Canary CorradePluginManagerTestLib)
if(CORRADE_TARGET_WINDOWS)
    target_link_libraries(Snail CorradePluginManagerTestLib)
    target_link_libraries(Dog CorradePluginManagerTestLib)
    target_link_libraries(Bulldog CorradePluginManagerTestLib)
    target_link_libraries(Chihuahua CorradePluginManagerTestLib Dog)
endif()

add_subdirectory(deletable)
add_subdirectory(food)
