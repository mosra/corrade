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

corrade_add_plugin(HotDog ${CMAKE_CURRENT_BINARY_DIR} "" HotDog.conf HotDog.cpp)
corrade_add_plugin(HotDogWithSnail ${CMAKE_CURRENT_BINARY_DIR} "" HotDogWithSnail.conf HotDog.cpp)
corrade_add_plugin(OldBread ${CMAKE_CURRENT_BINARY_DIR} "" OldBread.conf OldBread.cpp)
corrade_add_plugin(RottenTomato ${CMAKE_CURRENT_BINARY_DIR} "" RottenTomato.conf RottenTomato.cpp)

set_target_properties(
    HotDog HotDog-metadata
    HotDogWithSnail HotDogWithSnail-metadata
    OldBread OldBread-metadata
    RottenTomato RottenTomato-metadata
    PROPERTIES
        FOLDER "Corrade/PluginManager/Test"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/food
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/food
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/food)

# DLLs don't allow unresolved symbols; on static builds there's no dynamic
# plugin manager library to refer to so it has to be linked as well
if(CORRADE_BUILD_STATIC OR CORRADE_TARGET_WINDOWS)
    target_link_libraries(HotDog PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(HotDogWithSnail PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(OldBread PRIVATE CorradePluginManagerTestLib)
    target_link_libraries(RottenTomato PRIVATE CorradePluginManagerTestLib)
endif()

# Additionally the plugin dependencies have to be linked as well
if(CORRADE_TARGET_WINDOWS)
    target_link_libraries(HotDog PUBLIC Dog)
    target_link_libraries(HotDogWithSnail PRIVATE HotDog)
endif()
