#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CorradeUtility_SRCS
    Debug.cpp
    Directory.cpp
    Configuration.cpp
    ConfigurationValue.cpp
    MurmurHash2.cpp
    Sha1.cpp
    String.cpp
    utilities.cpp)

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
    TypeTraits.h
    Unicode.h
    utilities.h
    Utility.h
    VisibilityMacros.h
    visibility.h)

# NaCl-specific functionality
if(CORRADE_TARGET_NACL)
    set(CorradeUtility_SRCS ${CorradeUtility_SRCS}
        NaClStreamBuffer.cpp)
    set(CorradeUtility_HEADERS ${CorradeUtility_HEADERS}
        NaClStreamBuffer.h)

# Android-specific functionality
elseif(CORRADE_TARGET_ANDROID)
    set(CorradeUtility_SRCS ${CorradeUtility_SRCS}
        AndroidStreamBuffer.cpp)
    set(CorradeUtility_HEADERS ${CorradeUtility_HEADERS}
        AndroidStreamBuffer.h)
endif()

# Objects shared between main and test library
add_library(CorradeUtilityObjects OBJECT
    ${CorradeUtility_SRCS}
    ${CorradeUtility_HEADERS})
if(NOT BUILD_STATIC)
    set_target_properties(CorradeUtilityObjects PROPERTIES COMPILE_FLAGS "-DCorradeUtilityObjects_EXPORTS")
endif()
if(NOT BUILD_STATIC OR BUILD_STATIC_PIC)
    set_target_properties(CorradeUtilityObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Main Utility library
add_library(CorradeUtility ${SHARED_OR_STATIC}
    $<TARGET_OBJECTS:CorradeUtilityObjects>
    ${CorradeUtility_GracefulAssert_SRCS})
set_target_properties(CorradeUtility PROPERTIES DEBUG_POSTFIX "-d")
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
    target_link_libraries(corrade-rc CorradeUtility)
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})
endif()

if(BUILD_TESTS)
    # Library with graceful assert for testing
    add_library(CorradeUtilityTestLib ${SHARED_OR_STATIC} ${CorradeUtility_GracefulAssert_SRCS})
    set_target_properties(CorradeUtilityTestLib PROPERTIES
        COMPILE_FLAGS "-DCORRADE_GRACEFUL_ASSERT"
        DEBUG_POSTFIX "-d")
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
