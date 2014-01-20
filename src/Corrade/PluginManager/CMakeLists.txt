#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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

set(CorradePluginManager_SRCS
    AbstractPlugin.cpp
    AbstractManager.cpp
    PluginMetadata.cpp)

set(CorradePluginManager_HEADERS
    AbstractPlugin.h
    AbstractManager.h
    Manager.h
    PluginManager.h
    PluginMetadata.h
    visibility.h)

add_library(CorradePluginManager ${SHARED_OR_STATIC} ${CorradePluginManager_SRCS})
if(NOT BUILD_STATIC)
    set_target_properties(CorradePluginManager PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
elseif(BUILD_STATIC_PIC)
    # TODO: CMake 2.8.9 has this as POSITION_INDEPENDENT_CODE property
    set_target_properties(CorradePluginManager PROPERTIES COMPILE_FLAGS "${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")
endif()
target_link_libraries(CorradePluginManager CorradeUtility)
if(CORRADE_TARGET_UNIX OR CORRADE_TARGET_NACL_GLIBC)
    target_link_libraries(CorradePluginManager ${CMAKE_DL_LIBS})
endif()

install(TARGETS CorradePluginManager
        RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
        LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
        ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradePluginManager_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/PluginManager)

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
