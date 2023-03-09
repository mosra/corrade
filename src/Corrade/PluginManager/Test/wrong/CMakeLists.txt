#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022
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

corrade_add_plugin(NoPluginVersion ${CMAKE_CURRENT_BINARY_DIR} "" "" NoPluginVersion.cpp)
corrade_add_plugin(NoPluginInterface ${CMAKE_CURRENT_BINARY_DIR} "" "" NoPluginInterface.cpp)
corrade_add_plugin(NoPluginInitializer ${CMAKE_CURRENT_BINARY_DIR} "" "" NoPluginInitializer.cpp)
corrade_add_plugin(NoPluginFinalizer ${CMAKE_CURRENT_BINARY_DIR} "" "" NoPluginFinalizer.cpp)
corrade_add_plugin(NoPluginInstancer ${CMAKE_CURRENT_BINARY_DIR} "" "" NoPluginInstancer.cpp)

add_library(MissingLibraryDependencyLibrary SHARED MissingLibraryDependencyLibrary.cpp)
# For the VisibilityMacros include
target_link_libraries(MissingLibraryDependencyLibrary PRIVATE Corrade::Utility)
corrade_add_plugin(MissingLibraryDependency ${CMAKE_CURRENT_BINARY_DIR} "" "" MissingLibraryDependency.cpp)

set_target_properties(
    NoPluginVersion
    NoPluginInterface
    NoPluginInitializer
    NoPluginFinalizer
    NoPluginInstancer
    MissingLibraryDependency
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wrong
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wrong
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/wrong)

# On Windows the plugin dependencies have to be linked, but if we don't put the
# DLL next to the executable (and don't load it explicitly before either), it
# still won't work
if(CORRADE_TARGET_WINDOWS)
    set_target_properties(MissingLibraryDependencyLibrary
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wrong/dependency
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wrong/dependency
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/wrong/dependency)
    target_link_libraries(MissingLibraryDependency PRIVATE MissingLibraryDependencyLibrary CorradePluginManagerTestLib)
endif()
