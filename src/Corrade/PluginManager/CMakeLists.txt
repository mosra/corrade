#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "Corrade/PluginManager")

if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Implementation/configure.h.cmake
                   ${CMAKE_CURRENT_BINARY_DIR}/Implementation/configure.h)
endif()

set(CorradePluginManager_SRCS
    PluginMetadata.cpp)

set(CorradePluginManager_GracefulAssert_SRCS
    AbstractPlugin.cpp
    AbstractManager.cpp)

set(CorradePluginManager_HEADERS
    AbstractPlugin.h
    AbstractManager.h
    AbstractManagingPlugin.h
    Manager.h
    Manager.hpp
    PluginManager.h
    PluginMetadata.h
    visibility.h)

set(CorradePluginManager_PRIVATE_HEADERS
    Implementation/Plugin.h)

# Objects shared between main and test library
add_library(CorradePluginManagerObjects OBJECT
    ${CorradePluginManager_SRCS}
    ${CorradePluginManager_HEADERS}
    ${CorradePluginManager_PRIVATE_HEADERS})
target_include_directories(CorradePluginManagerObjects PUBLIC $<TARGET_PROPERTY:CorradeUtility,INTERFACE_INCLUDE_DIRECTORIES>)
if(NOT CORRADE_BUILD_STATIC)
    target_compile_definitions(CorradePluginManagerObjects PRIVATE "CorradePluginManagerObjects_EXPORTS")
endif()
if(NOT CORRADE_BUILD_STATIC OR CORRADE_BUILD_STATIC_PIC)
    set_target_properties(CorradePluginManagerObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Main PluginManager library
add_library(CorradePluginManager ${SHARED_OR_STATIC}
    $<TARGET_OBJECTS:CorradePluginManagerObjects>
    ${CorradePluginManager_GracefulAssert_SRCS})
target_include_directories(CorradePluginManager PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(CorradePluginManager PROPERTIES DEBUG_POSTFIX "-d")
if(NOT CORRADE_BUILD_STATIC)
    set_target_properties(CorradePluginManager PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
elseif(CORRADE_BUILD_STATIC_PIC)
    set_target_properties(CorradePluginManager PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
# Utility also does -ldl now
target_link_libraries(CorradePluginManager PUBLIC CorradeUtility)

install(TARGETS CorradePluginManager
        RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
        LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
        ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradePluginManager_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/PluginManager)

if(CORRADE_BUILD_TESTS)
    # Library with graceful assert for testing
    add_library(CorradePluginManagerTestLib ${SHARED_OR_STATIC} ${EXCLUDE_FROM_ALL_IF_TEST_TARGET}
        $<TARGET_OBJECTS:CorradePluginManagerObjects>
        ${CorradePluginManager_GracefulAssert_SRCS})
    target_include_directories(CorradePluginManagerTestLib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(CorradePluginManagerTestLib PRIVATE "CORRADE_GRACEFUL_ASSERT")
    set_target_properties(CorradePluginManagerTestLib PROPERTIES DEBUG_POSTFIX "-d")
    if(CORRADE_BUILD_STATIC_PIC)
        set_target_properties(CorradePluginManagerTestLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
    target_link_libraries(CorradePluginManagerTestLib PUBLIC CorradeUtility)
    if(CORRADE_TARGET_UNIX)
        target_link_libraries(CorradePluginManagerTestLib PUBLIC ${CMAKE_DL_LIBS})
    endif()

    add_subdirectory(Test ${EXCLUDE_FROM_ALL_IF_TEST_TARGET})
endif()

# Corrade::PluginManager target alias for superprojects
add_library(Corrade::PluginManager ALIAS CorradePluginManager)
