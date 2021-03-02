load("//bazel:configure_header.bzl", "configure_header")

configure_header(
    name = "configure",
    src = "configure.h.cmake",
    output = "configure.h",
    defines = {
        "RELATIVE_TEST_DIR": "$(execpath .)",
        "ABSOLUTE_TEST_DIR": "$(execpath .)/BundledFilesTestFiles",
        "TESTER_TEST_DIR": "$(execpath .)/TesterTestFiles",
    },
    deps = ["."],
)

cc_test(
    name = "TestSuiteArgumentsTest",
    srcs = ["ArgumentsTest.cpp"],
    args = [
        "--arguments-value",
        "hello",
    ],
    copts = ["-Isrc"],
    deps = ["//:TestSuite"],
)

cc_test(
    name = "TestSuiteTesterTest",
    srcs = ["TesterTest.cpp"],
    copts = ["-Isrc"],
    deps = [
        "//:TestSuite",
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
    copts = ["-Isrc"],
    deps = [
        "//:TestSuite",
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
    copts = ["-Isrc"],
    deps = ["//:TestSuite"],
)
for t in [
    "ComparatorTest",
    "BenchmarkStatsTest",
    # Bazel doesn't know how to expect failing tests
    # TODO: find a way to fix/wrap?
    #"FailingTest",
]]

#cc_test(
#    name = "TestSuiteFailingTest",
#    srcs = ["FailingTest.cpp"],
#    copts = ["-Isrc"],
#    deps = ["//:TestSuite"],
#)
