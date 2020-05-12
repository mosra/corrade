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

corrade_add_test(TestSuiteArgumentsTest ArgumentsTest.cpp
    ARGUMENTS --arguments-value hello)

corrade_add_test(TestSuiteComparatorTest ComparatorTest.cpp)

corrade_add_test(TestSuiteFailingTest FailingTest.cpp)
set_tests_properties(TestSuiteFailingTest PROPERTIES WILL_FAIL ON)

corrade_add_test(TestSuiteTesterTest TesterTest.cpp
    FILES
        TesterTestFiles/abortOnFail.txt
        TesterTestFiles/abortOnFailSkip.txt
        TesterTestFiles/benchmarkCpuClock.txt
        TesterTestFiles/benchmarkCpuCycles.txt
        TesterTestFiles/benchmarkDiscardAll.txt
        TesterTestFiles/benchmarkWallClock.txt
        TesterTestFiles/benchmarkDebugBuildNote.txt
        TesterTestFiles/benchmarkCpuScalingWarning.txt
        TesterTestFiles/benchmarkCpuScalingWarningVerbose.txt
        TesterTestFiles/compareMessageFailed.txt
        TesterTestFiles/compareMessageVerboseDisabled.txt
        TesterTestFiles/compareMessageVerboseEnabled.txt
        TesterTestFiles/compareMessageXfail.txt
        TesterTestFiles/noCatch.txt
        TesterTestFiles/noXfail.txt
        TesterTestFiles/repeatAll.txt
        TesterTestFiles/repeatEvery.txt
        TesterTestFiles/saveDiagnosticAbortOnFail.txt
        TesterTestFiles/saveDiagnosticSucceededDisabled.txt
        TesterTestFiles/saveDiagnosticSucceededEnabled.txt
        TesterTestFiles/saveDiagnosticFailedDisabled.txt
        TesterTestFiles/saveDiagnosticFailedEnabled.txt
        TesterTestFiles/saveDiagnosticVerboseDisabled.txt
        TesterTestFiles/saveDiagnosticVerboseEnabled.txt
        TesterTestFiles/saveDiagnosticXfail.txt
        TesterTestFiles/saveDiagnosticXpassDisabled.txt
        TesterTestFiles/saveDiagnosticXpassEnabled.txt
        TesterTestFiles/shuffleOne.txt
        TesterTestFiles/skipBenchmarks.txt
        TesterTestFiles/skipOnly.txt
        TesterTestFiles/skipTests.txt
        TesterTestFiles/test.txt
        TesterTestFiles/testName.txt

        TesterTestFiles/cpu-governor-performance.txt
        TesterTestFiles/cpu-governor-powersave.txt)
target_include_directories(TestSuiteTesterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(RELATIVE_TEST_DIR ".")
    set(ABSOLUTE_TEST_DIR ".")
    set(TESTER_TEST_DIR "TesterTestFiles")
else()
    set(RELATIVE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(ABSOLUTE_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BundledFilesTestFiles)
    set(TESTER_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TesterTestFiles)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

corrade_add_test(TestSuiteBenchmarkStatsTest BenchmarkStatsTest.cpp)

corrade_add_test(TestSuiteBundledFilesTest BundledFilesTest.cpp
    FILES
        BundledFilesTestFiles/a.txt
        ../Test/BundledFilesTestFiles/b.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/BundledFilesTestFiles/c.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/BundledFilesTestFiles/../BundledFilesTestFiles/d.txt@BundledFilesTestFiles/d.txt)
target_include_directories(TestSuiteBundledFilesTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(
    TestSuiteFailingTest
    TestSuiteTesterTest
    TestSuiteBenchmarkStatsTest
    TestSuiteBundledFilesTest
    PROPERTIES FOLDER "Corrade/TestSuite/Test")
