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

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(FILETEST_DIR FileTestFiles)
    set(FILETEST_SAVE_DIR FileTestSaveFiles)
else()
    set(FILETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FileTestFiles)
    set(FILETEST_SAVE_DIR ${CMAKE_CURRENT_BINARY_DIR}/FileTestFiles)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

corrade_add_test(TestSuiteCompareContainerTest ContainerTest.cpp)
corrade_add_test(TestSuiteCompareSortedContainerTest SortedContainerTest.cpp)
corrade_add_test(TestSuiteCompareFileTest FileTest.cpp
    FILES
        FileTestFiles/base.txt
        FileTestFiles/different.txt
        FileTestFiles/empty.txt
        FileTestFiles/hýždě.txt
        FileTestFiles/smaller.txt)
target_include_directories(TestSuiteCompareFileTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
corrade_add_test(TestSuiteCompareFileToStringTest FileToStringTest.cpp
    FILES
        FileTestFiles/base.txt
        FileTestFiles/different.txt
        FileTestFiles/empty.txt
        FileTestFiles/hýždě.txt
        FileTestFiles/smaller.txt)
target_include_directories(TestSuiteCompareFileToStringTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
corrade_add_test(TestSuiteCompareFloatingPointTest FloatingPointTest.cpp)
corrade_add_test(TestSuiteCompareNumericTest NumericTest.cpp)
corrade_add_test(TestSuiteCompareStringTest StringTest.cpp)
corrade_add_test(TestSuiteCompareStringToFileTest StringToFileTest.cpp
    FILES
        FileTestFiles/base.txt
        FileTestFiles/different.txt
        FileTestFiles/empty.txt
        FileTestFiles/hýždě.txt
        FileTestFiles/smaller.txt)
target_include_directories(TestSuiteCompareStringToFileTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(
    TestSuiteCompareContainerTest
    TestSuiteCompareFileTest
    TestSuiteCompareFileToStringTest
    TestSuiteCompareFloatingPointTest
    TestSuiteCompareNumericTest
    TestSuiteCompareStringTest
    TestSuiteCompareStringToFileTest
    PROPERTIES FOLDER "Corrade/TestSuite/Compare/Test")
