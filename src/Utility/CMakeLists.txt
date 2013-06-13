#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
    ConfigurationGroup.cpp
    ConfigurationValue.cpp
    MurmurHash2.cpp
    Sha1.cpp
    String.cpp
    Translator.cpp
    TypeTraits.cpp
    Unicode.cpp
    utilities.cpp)

set(CorradeUtility_GracefulAssert_SRCS
    Resource.cpp)

set(CorradeUtility_HEADERS
    AbstractHash.h
    Assert.h
    Configuration.h
    ConfigurationGroup.h
    ConfigurationValue.h
    Debug.h
    Directory.h
    Endianness.h
    MurmurHash2.h
    Resource.h
    Sha1.h
    String.h
    Translator.h
    TypeTraits.h
    Unicode.h
    utilities.h
    Utility.h
    Visibility.h

    corradeUtilityVisibility.h)

# NaCl-specific functionality
if(CORRADE_TARGET_NACL)
    set(CorradeUtility_SRCS ${CorradeUtility_SRCS}
        NaClStreamBuffer.cpp)
    set(CorradeUtility_HEADERS ${CorradeUtility_HEADERS}
        NaClStreamBuffer.h)
endif()

add_library(CorradeUtilityObjects OBJECT ${CorradeUtility_SRCS})
add_library(CorradeUtility ${SHARED_OR_STATIC} $<TARGET_OBJECTS:CorradeUtilityObjects> ${CorradeUtility_GracefulAssert_SRCS})

if(NOT BUILD_STATIC)
    set_target_properties(CorradeUtilityObjects CorradeUtility PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
    set_target_properties(CorradeUtilityObjects PROPERTIES COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
elseif(BUILD_STATIC_PIC)
    # TODO: CMake 2.8.9 has this as POSITION_INDEPENDENT_CODE property
    set_target_properties(CorradeUtility PROPERTIES COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
endif()
install(TARGETS CorradeUtility DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeUtility_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Utility)

if(NOT CMAKE_CROSSCOMPILING)
    add_executable(corrade-rc rc.cpp)
    target_link_libraries(corrade-rc CorradeUtility)
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})
endif()

if(BUILD_TESTS)
    add_library(CorradeUtilityTestLib ${SHARED_OR_STATIC} $<TARGET_OBJECTS:CorradeUtilityObjects> ${CorradeUtility_GracefulAssert_SRCS})
    set_target_properties(CorradeUtilityTestLib PROPERTIES COMPILE_FLAGS "-DCORRADE_GRACEFUL_ASSERT")
    if(BUILD_STATIC_PIC)
        # TODO: CMake 2.8.9 has this as POSITION_INDEPENDENT_CODE property
        set_target_properties(CorradeUtilityTestLib PROPERTIES COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
    endif()

    add_subdirectory(Test)
endif()
