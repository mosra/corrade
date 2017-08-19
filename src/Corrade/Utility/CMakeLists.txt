#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017 Vladimír Vondruš <mosra@centrum.cz>
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

set(CorradeUtility_SRCS
    Debug.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationValue.cpp
    MurmurHash2.cpp
    Sha1.cpp
    String.cpp
    System.cpp)

set(CorradeUtility_GracefulAssert_SRCS
    Arguments.cpp
    ConfigurationGroup.cpp
    Resource.cpp
    Unicode.cpp)

set(CorradeUtility_HEADERS
    Arguments.h
    AbstractHash.h
    Assert.h
    Configuration.h
    ConfigurationGroup.h
    ConfigurationValue.h
    Debug.h
    Directory.h
    Endianness.h
    Macros.h
    MurmurHash2.h
    Resource.h
    Sha1.h
    String.h
    System.h
    TypeTraits.h
    Unicode.h
    utilities.h
    Utility.h
    VisibilityMacros.h
    visibility.h)

# Android-specific functionality
if(CORRADE_TARGET_ANDROID)
    set(CorradeUtility_SRCS ${CorradeUtility_SRCS}
        AndroidStreamBuffer.cpp)
    set(CorradeUtility_HEADERS ${CorradeUtility_HEADERS}
        AndroidStreamBuffer.h)
endif()

# Objects shared between main and test library
add_library(CorradeUtilityObjects OBJECT
    ${CorradeUtility_SRCS}
    ${CorradeUtility_HEADERS})
target_include_directories(CorradeUtilityObjects PUBLIC $<TARGET_PROPERTY:CorradeUtility,INTERFACE_INCLUDE_DIRECTORIES>)
if(NOT BUILD_STATIC)
    target_compile_definitions(CorradeUtilityObjects PRIVATE "-DCorradeUtilityObjects_EXPORTS")
endif()
if(NOT BUILD_STATIC OR BUILD_STATIC_PIC)
    set_target_properties(CorradeUtilityObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
set_target_properties(CorradeUtilityObjects PROPERTIES FOLDER "Corrade/Utility")

# Main Utility library
add_library(CorradeUtility ${SHARED_OR_STATIC}
    $<TARGET_OBJECTS:CorradeUtilityObjects>
    ${CorradeUtility_GracefulAssert_SRCS})
target_include_directories(CorradeUtility PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
# Require (at least) C++11 for users
if(NOT CMAKE_VERSION VERSION_LESS 3.0.0)
    set_property(TARGET CorradeUtility PROPERTY
        INTERFACE_CORRADE_CXX_STANDARD 11)
    set_property(TARGET CorradeUtility APPEND PROPERTY
        COMPATIBLE_INTERFACE_NUMBER_MAX CORRADE_CXX_STANDARD)
else()
    # See FindCorrade.cmake for juicy rant about 2.8.12 and why can't I use
    # single property name to cover all cases. AAAAAAAAARGH.
    set_property(TARGET CorradeUtility PROPERTY
        INTERFACE_CORRADE_CXX_STANDARD_ 11)
    set_property(TARGET CorradeUtility APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING CORRADE_CXX_STANDARD_)
endif()
set_target_properties(CorradeUtility PROPERTIES
    DEBUG_POSTFIX "-d"
    FOLDER "Corrade/Utility")
if(NOT BUILD_STATIC)
    set_target_properties(CorradeUtility PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
elseif(BUILD_STATIC_PIC)
    set_target_properties(CorradeUtility PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# AndroidLogStreamBuffer class needs to be linked to log library
if(CORRADE_TARGET_ANDROID)
    target_link_libraries(CorradeUtility log)
endif()

install(TARGETS CorradeUtility
        RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
        LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
        ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeUtility_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Utility)

# TODO: In the future it might be possible to not require external corrade-rc
# when generating WinRT targets by setting
#  set_target_properties(corrade-rc PROPERTIES VS_WINRT_COMPONENT OFF)
# Then the condition would be if(NOT CMAKE_CROSSCOMPILING OR CORRADE_TARGET_WINDOWS_RT)
# and similar change would be in UseCorrade.cmake. More info in this thread:
#  https://cmake.org/pipermail/cmake-developers/2015-January/024242.html
if(NOT CMAKE_CROSSCOMPILING)
    add_executable(corrade-rc rc.cpp)
    set_target_properties(corrade-rc PROPERTIES FOLDER "Corrade/Utility")
    target_link_libraries(corrade-rc CorradeUtility)
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})
endif()

if(BUILD_TESTS)
    # Library with graceful assert for testing
    add_library(CorradeUtilityTestLib ${SHARED_OR_STATIC} ${CorradeUtility_GracefulAssert_SRCS})
    target_compile_definitions(CorradeUtilityTestLib PRIVATE "CORRADE_GRACEFUL_ASSERT")
    set_target_properties(CorradeUtilityTestLib PROPERTIES
        DEBUG_POSTFIX "-d"
        FOLDER "Corrade/Utility")
    if(BUILD_STATIC_PIC)
        set_target_properties(CorradeUtilityTestLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    target_link_libraries(CorradeUtilityTestLib CorradeUtility)

    # On Windows we need to install first and then run the tests to avoid "DLL
    # not found" hell, thus we need to install this too
    if(CORRADE_TARGET_WINDOWS AND NOT CMAKE_CROSSCOMPILING AND NOT BUILD_STATIC)
        install(TARGETS CorradeUtilityTestLib
            RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
            LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
            ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
    endif()

    add_subdirectory(Test)
endif()

# Corrade::Utility and Corrade::rc target alias for superprojects
add_library(Corrade::Utility ALIAS CorradeUtility)
if(NOT CMAKE_CROSSCOMPILING)
    add_executable(Corrade::rc ALIAS corrade-rc)
endif()
