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
    corrade_add_plugin(CustomSuffix ${CMAKE_CURRENT_BINARY_DIR} "" CustomSuffix.modconf CustomSuffix.cpp)
    set_target_properties(CustomSuffix CustomSuffix-metadata PROPERTIES
        SUFFIX .mod
        FOLDER "Corrade/PluginManager/Test"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/custom-suffix
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/custom-suffix
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/custom-suffix)

    # DLLs don't allow unresolved symbols; on static builds there's no dynamic
    # plugin manager library to refer to so it has to be linked as well
    if(CORRADE_BUILD_STATIC OR CORRADE_TARGET_WINDOWS)
        target_link_libraries(CustomSuffix PRIVATE CorradePluginManagerTestLib)
    endif()
endif()

corrade_add_static_plugin(CustomSuffixStatic ${CMAKE_CURRENT_BINARY_DIR} CustomSuffixStatic.modconf CustomSuffixStatic.cpp)
set_target_properties(CustomSuffixStatic CustomSuffixStatic-dependencies PROPERTIES
    FOLDER "Corrade/PluginManager/Test"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/custom-suffix
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/custom-suffix
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/custom-suffix)
target_link_libraries(CustomSuffixStatic PRIVATE CorradePluginManagerTestLib)
