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

set(CorradeInterconnect_SRCS
    Connection.cpp
    Emitter.cpp
    Receiver.cpp)

set(CorradeInterconnect_HEADERS
    Connection.h
    Emitter.h
    Interconnect.h
    Receiver.h
    StateMachine.h
    visibility.h)

set(CorradeInterconnect_PRIVATE_HEADERS
    Implementation/ReceiverConnection.h)

# Interconnect library
add_library(CorradeInterconnect ${SHARED_OR_STATIC}
    ${CorradeInterconnect_SRCS}
    ${CorradeInterconnect_HEADERS}
    ${CorradeInterconnect_PRIVATE_HEADERS})
set_target_properties(CorradeInterconnect PROPERTIES
    DEBUG_POSTFIX "-d"
    FOLDER "Corrade/Interconnect")
if(NOT BUILD_STATIC)
    set_target_properties(CorradeInterconnect PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
elseif(BUILD_STATIC_PIC)
    set_target_properties(CorradeInterconnect PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(CorradeInterconnect PUBLIC CorradeUtility)
# Disable /OPT:ICF on MSVC, which merges functions with identical contents and
# thus breaks signal comparison
if(CORRADE_TARGET_WINDOWS AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_VERSION VERSION_LESS 3.13)
        target_link_libraries(CorradeInterconnect INTERFACE "-OPT:NOICF,REF")
    else()
        target_link_options(CorradeInterconnect INTERFACE "/OPT:NOICF,REF")
    endif()
endif()

install(TARGETS CorradeInterconnect
        RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
        LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
        ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeInterconnect_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Interconnect)

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()

# Corrade::Interconnect target alias for superprojects
add_library(Corrade::Interconnect ALIAS CorradeInterconnect)
