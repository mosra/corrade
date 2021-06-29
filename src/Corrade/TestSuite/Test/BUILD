#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021
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
load("@corrade//:index.bzl", "corrade_configure_header")

corrade_configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        "RELATIVE_TEST_DIR": "$(rootpath .)",
        "ABSOLUTE_TEST_DIR": "$(rootpath .)/BundledFilesTestFiles",
        "TESTER_TEST_DIR": "$(rootpath .)/TesterTestFiles",
    },
    deps = ["."],
    local = True,
)

cc_test(
    name = "TestSuiteArgumentsTest",
    srcs = ["ArgumentsTest.cpp"],
    args = [
        "--arguments-value",
        "hello",
    ],
    deps = ["@corrade//:TestSuite"],
)

cc_test(
    name = "TestSuiteTesterTest",
    srcs = ["TesterTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        ":configure",
    ],
    data = [
        "TesterTestFiles/abortOnFail.txt",
        "TesterTestFiles/abortOnFailSkip.txt",
        "TesterTestFiles/benchmarkCpuClock.txt",
        "TesterTestFiles/benchmarkCpuCycles.txt",
        "TesterTestFiles/benchmarkDiscardAll.txt",
        "TesterTestFiles/benchmarkWallClock.txt",
        "TesterTestFiles/benchmarkDebugBuildNote.txt",
        "TesterTestFiles/benchmarkCpuScalingWarning.txt",
        "TesterTestFiles/benchmarkCpuScalingWarningVerbose.txt",
        "TesterTestFiles/compareMessageFailed.txt",
        "TesterTestFiles/compareMessageVerboseDisabled.txt",
        "TesterTestFiles/compareMessageVerboseEnabled.txt",
        "TesterTestFiles/compareMessageXfail.txt",
        "TesterTestFiles/noCatch.txt",
        "TesterTestFiles/noXfail.txt",
        "TesterTestFiles/repeatAll.txt",
        "TesterTestFiles/repeatEvery.txt",
        "TesterTestFiles/saveDiagnosticAbortOnFail.txt",
        "TesterTestFiles/saveDiagnosticSucceededDisabled.txt",
        "TesterTestFiles/saveDiagnosticSucceededEnabled.txt",
        "TesterTestFiles/saveDiagnosticFailedDisabled.txt",
        "TesterTestFiles/saveDiagnosticFailedEnabled.txt",
        "TesterTestFiles/saveDiagnosticVerboseDisabled.txt",
        "TesterTestFiles/saveDiagnosticVerboseEnabled.txt",
        "TesterTestFiles/saveDiagnosticXfail.txt",
        "TesterTestFiles/saveDiagnosticXpassDisabled.txt",
        "TesterTestFiles/saveDiagnosticXpassEnabled.txt",
        "TesterTestFiles/shuffleOne.txt",
        "TesterTestFiles/skipBenchmarks.txt",
        "TesterTestFiles/skipOnly.txt",
        "TesterTestFiles/skipTests.txt",
        "TesterTestFiles/test.txt",
        "TesterTestFiles/testName.txt",
        "TesterTestFiles/cpu-governor-performance.txt",
        "TesterTestFiles/cpu-governor-powersave.txt",
    ],
)

cc_test(
    name = "TestSuiteBundledFilesTest",
    srcs = ["BundledFilesTest.cpp"],
    deps = [
        "@corrade//:TestSuite",
        ":configure",
    ],
    data = [
        # bazel has much stricter rules on files location and sourcing
        # so can't do the kind of shenanigans cmake allows doing
        "BundledFilesTestFiles/a.txt",
        "BundledFilesTestFiles/b.txt",
        "BundledFilesTestFiles/c.txt",
        "BundledFilesTestFiles/d.txt",
    ],
)

[cc_test(
    name = "TestSuite%s" % t,
    srcs = ["%s.cpp" % t],
    deps = ["@corrade//:TestSuite"],
)
for t in [
    "ComparatorTest",
    "BenchmarkStatsTest",
    # Bazel doesn't know how to expect failing tests
    # TODO: find a way to fix/wrap?
    #"FailingTest",
]]
