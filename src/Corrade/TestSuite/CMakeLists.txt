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

set(CorradeTestSuite_SRCS
    Comparator.cpp
    Tester.cpp

    Compare/File.cpp
    Compare/FileToString.cpp
    Compare/FloatingPoint.cpp
    Compare/StringToFile.cpp)

set(CorradeTestSuite_HEADERS
    Comparator.h
    Tester.h
    TestSuite.h
    visibility.h)

set(CorradeTestSuite_PRIVATE_HEADERS
    Implementation/BenchmarkCounters.h
    Implementation/BenchmarkStats.h)

# TestSuite library
add_library(CorradeTestSuite ${SHARED_OR_STATIC}
    ${CorradeTestSuite_SRCS}
    ${CorradeTestSuite_HEADERS}
    ${CorradeTestSuite_PRIVATE_HEADERS})
set_target_properties(CorradeTestSuite PROPERTIES
    DEBUG_POSTFIX "-d"
    FOLDER "Corrade/TestSuite")
if(NOT BUILD_STATIC)
    set_target_properties(CorradeTestSuite PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
elseif(BUILD_STATIC_PIC)
    set_target_properties(CorradeTestSuite PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_link_libraries(CorradeTestSuite CorradeUtility)
if(CORRADE_TARGET_EMSCRIPTEN)
    # Since (probably) 1.38.36, the compiler needs to have exceptions enabled
    # as well (not just the linker), and directly for the TestSuite library.
    # Best would be to use the target_link_options() / target_compile_options()
    # together with SHELL, but we don't want to force people to use CMake 3.13
    # so doing it the old way --- and instead of the options being transitively
    # passed to the target, for the test target it's done explicitly inside
    # UseCorrade.cmake. Wasn't needed with 1.38.32.
    set_property(TARGET CorradeTestSuite APPEND_STRING PROPERTY COMPILE_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
endif()

install(TARGETS CorradeTestSuite
        RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
        LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
        ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeTestSuite_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/TestSuite)
if(CORRADE_TESTSUITE_TARGET_XCTEST)
    install(FILES XCTestRunner.mm.in DESTINATION ${CORRADE_DATA_INSTALL_DIR}/TestSuite)
elseif(CORRADE_TARGET_ANDROID)
    install(PROGRAMS AdbRunner.sh DESTINATION ${CORRADE_DATA_INSTALL_DIR}/TestSuite)
elseif(CORRADE_TARGET_EMSCRIPTEN)
    install(FILES EmscriptenRunner.html.in DESTINATION ${CORRADE_DATA_INSTALL_DIR}/TestSuite)
endif()

add_subdirectory(Compare)

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()

# Corrade::TestSuite target alias for superprojects
add_library(Corrade::TestSuite ALIAS CorradeTestSuite)
