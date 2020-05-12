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

# Built separately so the error message doesn't affect everything else
corrade_add_plugin(WrongMetadata ${CMAKE_CURRENT_BINARY_DIR} "" WrongMetadata.conf WrongMetadata.cpp)

set_target_properties(
    WrongMetadata WrongMetadata-metadata
    PROPERTIES
        FOLDER "Corrade/PluginManager/Test"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wrong-metadata
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/wrong-metadata
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/wrong-metadata)
target_include_directories(WrongMetadata PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../$<CONFIG>)

# DLLs don't allow unresolved symbols; on static builds there's no dynamic
# plugin manager library to refer to so it has to be linked as well
if(CORRADE_BUILD_STATIC OR CORRADE_TARGET_WINDOWS)
    target_link_libraries(WrongMetadata PRIVATE CorradePluginManagerTestLib)
endif()
