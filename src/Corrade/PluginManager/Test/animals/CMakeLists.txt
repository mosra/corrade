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
        PROPERTIES
            FOLDER "Corrade/PluginManager/Test"
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/animals
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/animals
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/animals)
endif()

corrade_add_static_plugin(Canary ${CMAKE_CURRENT_BINARY_DIR} Canary.conf Canary.cpp)
target_link_libraries(Canary PRIVATE CorradePluginManagerTestLib)
if(CORRADE_BUILD_STATIC AND NOT CORRADE_TARGET_EMSCRIPTEN AND NOT CORRADE_TARGET_IOS AND NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_WINDOWS_RT AND BUILD_STATIC)
    # The plugin gets linked to a shared test lib in a static build
    set_target_properties(Canary PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

corrade_add_static_plugin(Dird ${CMAKE_CURRENT_BINARY_DIR} Dird.conf Dird.cpp)
target_link_libraries(Dird PRIVATE CorradePluginManagerTestLib)

set_target_properties(
    Canary Canary-dependencies
    Dird Dird-dependencies
    PROPERTIES
        FOLDER "Corrade/PluginManager/Test"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/animals
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/animals
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/animals)

# DLLs don't allow unresolved symbols; on static builds there's no dynamic
# plugin manager library to refer to so it has to be linked as well
if((CORRADE_BUILD_STATIC OR CORRADE_TARGET_WINDOWS) AND NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    target_link_libraries(Snail PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(Dog PUBLIC CorradePluginManagerTestLib)
    target_link_libraries(Bulldog PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(PitBull PRIVATE CorradePluginManagerTestLib)
endif()

# Additionally the plugin dependencies have to be linked as well
if(CORRADE_TARGET_WINDOWS)
    target_link_libraries(PitBull PRIVATE Dog)
endif()
