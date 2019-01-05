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

# CMake before 3.8 has broken $<TARGET_FILE*> expressions for iOS (see
# https://gitlab.kitware.com/cmake/cmake/merge_requests/404) and since Corrade
# doesn't support dynamic plugins on iOS, this sorta works around that. Should
# be revisited when updating Travis to newer Xcode (current has CMake 3.6).
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    corrade_add_plugin(Snail ${CMAKE_CURRENT_BINARY_DIR} "" Snail.conf Snail.cpp)
    corrade_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} "" Dog.conf Dog.cpp)
    corrade_add_plugin(Bulldog ${CMAKE_CURRENT_BINARY_DIR} "" Bulldog.conf Bulldog.cpp)
    corrade_add_plugin(PitBull ${CMAKE_CURRENT_BINARY_DIR} "" PitBull.conf PitBull.cpp)

    set_target_properties(
        Snail Snail-metadata
        Dog Dog-metadata
        Bulldog Bulldog-metadata
        PitBull PitBull-metadata
        PROPERTIES FOLDER "Corrade/PluginManager/Test")

    set(PLUGINS_DIR $<TARGET_FILE_DIR:PluginManagerManagerTest>)
    set(DOG_PLUGIN_FILENAME $<TARGET_FILE:Dog>)
    set(DOGGO_PLUGIN_FILENAME $<TARGET_FILE:Doggo>)
    set(PITBULL_PLUGIN_FILENAME $<TARGET_FILE:PitBull>)

    # First replace ${} variables, then $<> generator expressions
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
    file(GENERATE OUTPUT $<TARGET_FILE_DIR:PluginManagerManagerTest>/configure.h
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.h)
endif()

corrade_add_static_plugin(Canary ${CMAKE_CURRENT_BINARY_DIR} Canary.conf Canary.cpp)
set_target_properties(Canary Canary-dependencies PROPERTIES FOLDER "Corrade/PluginManager/Test")

corrade_add_test(PluginManagerManagerTest
    ManagerTest.cpp
    AbstractAnimal.cpp
    AbstractDeletable.cpp
    AbstractFood.cpp
    LIBRARIES CorradePluginManagerTestLib Canary)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    target_include_directories(PluginManagerManagerTest PRIVATE $<TARGET_FILE_DIR:PluginManagerManagerTest>)
else()
    target_include_directories(PluginManagerManagerTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

corrade_add_test(PluginManagerManagerInitFiniTest
    ManagerInitFiniTest.cpp
    LIBRARIES InitFiniStatic CorradePluginManager)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    target_include_directories(PluginManagerManagerInitFiniTest PRIVATE $<TARGET_FILE_DIR:PluginManagerManagerTest>)
endif()

set_target_properties(
    PluginManagerManagerTest
    PluginManagerManagerInitFiniTest
    PROPERTIES FOLDER "Corrade/PluginManager/Test")

target_link_libraries(Canary PRIVATE CorradePluginManagerTestLib)
if(CORRADE_TARGET_WINDOWS)
    target_link_libraries(Snail PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(Dog PUBLIC CorradePluginManagerTestLib)
    target_link_libraries(Bulldog PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(PitBull PRIVATE Dog)
endif()

add_subdirectory(init-fini)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    add_subdirectory(doggo)
    add_subdirectory(deletable)
    add_subdirectory(food)
    add_subdirectory(wrong)
    add_subdirectory(wrong-metadata)
endif()
