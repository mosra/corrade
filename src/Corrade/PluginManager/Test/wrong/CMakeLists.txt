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

corrade_add_plugin(NoPluginVersion ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf NoPluginVersion.cpp)
corrade_add_plugin(NoPluginInterface ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf NoPluginInterface.cpp)
corrade_add_plugin(NoPluginInitializer ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf NoPluginInitializer.cpp)
corrade_add_plugin(NoPluginFinalizer ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf NoPluginFinalizer.cpp)
corrade_add_plugin(NoPluginInstancer ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf NoPluginInstancer.cpp)

set_target_properties(
    NoPluginVersion NoPluginVersion-metadata
    NoPluginInterface NoPluginInterface-metadata
    NoPluginInitializer NoPluginInitializer-metadata
    NoPluginFinalizer NoPluginFinalizer-metadata
    NoPluginInstancer NoPluginInstancer-metadata
    PROPERTIES
        FOLDER "Corrade/PluginManager/Test"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wrong
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wrong
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/wrong)

if(NOT CORRADE_TARGET_WINDOWS)
    # On Windows it's SHARED, not MODULE, so this doesn't really work
    corrade_add_plugin(UnresolvedReference ${CMAKE_CURRENT_BINARY_DIR} "" ../dummy.conf UnresolvedReference.cpp)
    set_target_properties(UnresolvedReference UnresolvedReference-metadata
        PROPERTIES
            FOLDER "Corrade/PluginManager/Test"
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wrong
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wrong
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/wrong)
endif()
