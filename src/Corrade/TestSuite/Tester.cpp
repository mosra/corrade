/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "Tester.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <typeinfo>
#include <utility>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/TestSuite/Implementation/BenchmarkCounters.h"
#include "Corrade/TestSuite/Implementation/BenchmarkStats.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/String.h"

#ifdef __linux__ /* for getting processor count */
#include <unistd.h>
#endif

namespace Corrade { namespace TestSuite {

namespace {
    inline int digitCount(int number) {
        int digits = 0;
        while(number != 0) number /= 10, digits++;
        return digits;
    }

    constexpr const char PaddingString[] = "0000000000";

    #ifdef __linux__
    constexpr const char DefaultCpuScalingGovernorFile[] = "/sys/devices/system/cpu/cpu{}/cpufreq/scaling_governor";
    #endif
}

struct Tester::TesterConfiguration::Data {
    std::vector<std::string> skippedArgumentPrefixes;
    #ifdef __linux__
    std::string cpuScalingGovernorFile = DefaultCpuScalingGovernorFile;
    #endif
};

Tester::TesterConfiguration::TesterConfiguration() noexcept = default;

Tester::TesterConfiguration::TesterConfiguration(const TesterConfiguration& other):
    /* GCC 4.8 doesn't like {} here */
    _data{other._data ? new Data(*other._data) : nullptr} {}

Tester::TesterConfiguration::TesterConfiguration(TesterConfiguration&&) noexcept = default;

Tester::TesterConfiguration& Tester::TesterConfiguration::operator=(const TesterConfiguration& other) {
    /* GCC 4.8 doesn't like {} here */
    _data.reset(other._data ? new Data(*other._data) : nullptr);
    return *this;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::operator=(TesterConfiguration&&) noexcept = default;

Tester::TesterConfiguration::~TesterConfiguration() = default;

Containers::ArrayView<const std::string> Tester::TesterConfiguration::skippedArgumentPrefixes() const {
    return _data ? Containers::arrayView(&_data->skippedArgumentPrefixes[0], _data->skippedArgumentPrefixes.size()) : nullptr;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::setSkippedArgumentPrefixes(std::initializer_list<std::string> prefixes) {
    if(!_data) _data.reset(new Data);
    _data->skippedArgumentPrefixes.insert(_data->skippedArgumentPrefixes.end(), prefixes);
    return *this;
}

#ifdef __linux__
std::string Tester::TesterConfiguration::cpuScalingGovernorFile() const {
    return _data ? _data->cpuScalingGovernorFile : DefaultCpuScalingGovernorFile;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::setCpuScalingGovernorFile(const std::string& filename) {
    if(!_data) _data.reset(new Data);
    _data->cpuScalingGovernorFile = filename;
    return *this;
}
#endif

struct Tester::IterationPrinter::IterationPrinter::Data {
    std::ostringstream out;
    IterationPrinter* parent;
};

struct Tester::TesterState {
    explicit TesterState(const TesterConfiguration& configuration): configuration{std::move(configuration)} {}

    std::string formattedTestCaseName() const {
        if(testCaseName.empty()) return "<unknown>";
        if(testCaseTemplateName.empty()) return testCaseName;
        return Utility::formatString("{}<{}>", testCaseName, testCaseTemplateName);
    }

    Debug::Flags useColor;
    std::ostream *logOutput{}, *errorOutput{};
    std::vector<TestCase> testCases;
    std::string testFilename, testName, testCaseName, testCaseTemplateName,
        testCaseDescription, benchmarkName;
    std::size_t testCaseId{~std::size_t{}}, testCaseInstanceId{~std::size_t{}},
        testCaseRepeatId{~std::size_t{}}, benchmarkBatchSize{}, testCaseLine{},
        checkCount{}, diagnosticCount{};

    std::uint64_t benchmarkBegin{};
    std::uint64_t benchmarkResult{};
    TestCase* testCase{};
    /* When there's one more bool, this should become flags instead. Right now
       this only fill all holes in the struct layout. */
    bool expectedFailuresDisabled{};
    bool verbose{};
    bool testCaseLabelPrinted{};
    bool isDebugBuild{};
    ExpectedFailure* expectedFailure{};
    std::string expectedFailureMessage;
    IterationPrinter* iterationPrinter{};
    TesterConfiguration configuration;

    std::string saveDiagnosticPath;
};

int* Tester::_argc = nullptr;
char** Tester::_argv = nullptr;

void Tester::registerArguments(int& argc, char** const argv) {
    _argc = &argc;
    _argv = argv;
}

namespace {
    /* Unlike with Debug not making this unique across static libs as I don't
       expect any use case that would need this yet */
    Tester* currentTester = nullptr;
}

Tester::Tester(const TesterConfiguration& configuration): _state{new TesterState{configuration}} {
    CORRADE_ASSERT(_argc, "TestSuite::Tester: command-line arguments not available", );
}

Tester::~Tester() {
    /* Reset argument pointers to avoid accidentally forgotten calls to registerArguments() */
    _argc = nullptr;
    _argv = nullptr;
}

Tester& Tester::instance() {
    CORRADE_ASSERT(currentTester, "TestSuite: attempting to call CORRADE_*() macros from outside a test case", *currentTester);
    return *currentTester;
}

int Tester::exec() { return exec(nullptr, &std::cout, &std::cerr); }

int Tester::exec(Tester* const previousTester, std::ostream* const logOutput, std::ostream* const errorOutput) {
    /* Set up the global pointer for the time during which tests are run, then
       reset it back again. The `previousTester` is needed for testing where
       there *are* two nested Tester instances. */
    CORRADE_ASSERT(currentTester == previousTester, "TestSuite::Tester: only one Tester instance can be active at a time", {});
    currentTester = this;
    Containers::ScopeGuard resetCurrentTester{previousTester, [](Tester* t) {
        currentTester = t;
    }};

    Utility::Arguments args;
    for(auto&& prefix: _state->configuration.skippedArgumentPrefixes())
        args.addSkippedPrefix(prefix);
    args.addOption('c', "color", "auto").setHelp("color", "colored output", "on|off|auto")
            .setFromEnvironment("color", "CORRADE_TEST_COLOR")
        .addOption("skip").setHelp("skip", "skip test cases with given numbers", "\"N1 N2...\"")
        .addBooleanOption("skip-tests").setHelp("skip-tests", "skip all tests")
            .setFromEnvironment("skip-tests", "CORRADE_TEST_SKIP_TESTS")
        .addBooleanOption("skip-benchmarks").setHelp("skip-benchmarks", "skip all benchmarks")
            .setFromEnvironment("skip-benchmarks", "CORRADE_TEST_SKIP_BENCHMARKS")
        .addOption("only").setHelp("only", "run only test cases with given numbers", "\"N1 N2...\"")
        .addBooleanOption("shuffle").setHelp("shuffle", "randomly shuffle test case order")
            .setFromEnvironment("shuffle", "CORRADE_TEST_SHUFFLE")
        .addOption("repeat-every", "1").setHelp("repeat-every", "repeat every test case N times", "N")
            .setFromEnvironment("repeat-every", "CORRADE_TEST_REPEAT_EVERY")
        .addOption("repeat-all", "1").setHelp("repeat-all", "repeat all test cases N times", "N")
            .setFromEnvironment("repeat-all", "CORRADE_TEST_REPEAT_ALL")
        .addBooleanOption("abort-on-fail").setHelp("abort-on-fail", "abort after first failure")
            .setFromEnvironment("abort-on-fail", "CORRADE_TEST_ABORT_ON_FAIL")
        .addBooleanOption("no-xfail").setHelp("no-xfail", "disallow expected failures")
            .setFromEnvironment("no-xfail", "CORRADE_TEST_NO_XFAIL")
        .addBooleanOption("no-catch").setHelp("no-catch", "don't catch standard exceptions")
            .setFromEnvironment("no-catch", "CORRADE_TEST_NO_CATCH")
        .addOption("save-diagnostic", "").setHelp("save-diagnostic", "save diagnostic files to given path", "PATH")
            .setFromEnvironment("save-diagnostic", "CORRADE_TEST_SAVE_DIAGNOSTIC")
        .addBooleanOption('v', "verbose").setHelp("verbose", "enable verbose output")
            .setFromEnvironment("verbose", "CORRADE_TEST_VERBOSE")
        .addOption("benchmark", "wall-time").setHelp("benchmark", "default benchmark type", "TYPE")
            .setFromEnvironment("benchmark", "CORRADE_TEST_BENCHMARK")
        .addOption("benchmark-discard", "1").setHelp("benchmark-discard", "discard first N measurements of each benchmark", "N")
            .setFromEnvironment("benchmark-discard", "CORRADE_TEST_BENCHMARK_DISCARD")
        .addOption("benchmark-yellow", "0.05").setHelp("benchmark-yellow", "deviation threshold for marking benchmark yellow", "N")
            .setFromEnvironment("benchmark-yellow", "CORRADE_TEST_BENCHMARK_YELLOW")
        .addOption("benchmark-red", "0.25").setHelp("benchmark-red", "deviation threshold for marking benchmark red", "N")
            .setFromEnvironment("benchmark-red", "CORRADE_TEST_BENCHMARK_RED")
        .setGlobalHelp(R"(Corrade TestSuite executable. By default runs test cases in order in which they
were added and exits with non-zero code if any of them failed. Supported
benchmark types:
  wall-time     wall time spent
  cpu-time      CPU time spent
  cpu-cycles    CPU cycles spent (x86 only, gives zero result elsewhere))")
        .parse(*_argc, _argv);

    _state->logOutput = logOutput;
    _state->errorOutput = errorOutput;

    /* Decide about color */
    if(args.value("color") == "on" || args.value("color") == "ON")
        _state->useColor = Debug::Flags{};
    else if(args.value("color") == "off" || args.value("color") == "OFF")
        _state->useColor = Debug::Flag::DisableColors;
    /* LCOV_EXCL_START */ /* Can't reliably test this */
    else _state->useColor = Debug::isTty(logOutput) && Debug::isTty(errorOutput) ?
            Debug::Flags{} : Debug::Flag::DisableColors;
    /* LCOV_EXCL_STOP */

    /* Decide about default benchmark type */
    TestCaseType defaultBenchmarkType{};
    if(args.value("benchmark") == "wall-time")
        defaultBenchmarkType = TestCaseType::WallTimeBenchmark;
    else if(args.value("benchmark") == "cpu-time")
        defaultBenchmarkType = TestCaseType::CpuTimeBenchmark;
    else if(args.value("benchmark") == "cpu-cycles")
        defaultBenchmarkType = TestCaseType::CpuCyclesBenchmark;
    /* LCOV_EXCL_START */ /* Can't test stuff that aborts the app */
    else Utility::Fatal{} << "Unknown benchmark type" << args.value("benchmark")
        << Utility::Debug::nospace << ", use one of wall-time, cpu-time or cpu-cycles";
    /* LCOV_EXCL_STOP */

    std::vector<std::pair<int, TestCase>> usedTestCases;

    /* Disable expected failures, if requested */
    _state->expectedFailuresDisabled = args.isSet("no-xfail");

    /* Skip test cases, if requested */
    if(args.isSet("skip-tests"))
        for(TestCase& testCase: _state->testCases)
            if(testCase.type == TestCaseType::Test) testCase.test = nullptr;

    /* Skip benchmarks, if requested */
    if(args.isSet("skip-benchmarks"))
        for(TestCase& testCase: _state->testCases)
            if(testCase.type != TestCaseType::Test) testCase.test = nullptr;

    /* Remove skipped test cases */
    if(!args.value("skip").empty()) {
        const std::vector<std::string> skip = Utility::String::split(args.value("skip"), ' ');
        for(auto&& n: skip) {
            #ifndef CORRADE_TARGET_ANDROID
            const std::size_t index = std::stoi(n);
            #else
            const std::size_t index = std::strtoul(n.data(), nullptr, 10);
            #endif
            if(index - 1 >= _state->testCases.size()) continue;
            _state->testCases[index - 1].test = nullptr;
        }
    }

    /* Extract only whitelisted test cases if requested (and skip skipped) */
    if(!args.value("only").empty()) {
        const std::vector<std::string> only = Utility::String::split(args.value("only"), ' ');
        for(auto&& n: only) {
            #ifndef CORRADE_TARGET_ANDROID
            const std::size_t index = std::stoi(n);
            #else
            const std::size_t index = std::strtoul(n.data(), nullptr, 10);
            #endif
            if(index - 1 >= _state->testCases.size() || !_state->testCases[index - 1].test) continue;
            usedTestCases.emplace_back(index, _state->testCases[index - 1]);
        }

    /* Otherwise extract all (and skip skipped) */
    } else for(std::size_t i = 0; i != _state->testCases.size(); ++i) {
        if(!_state->testCases[i].test) continue;
        usedTestCases.emplace_back(i + 1, _state->testCases[i]);
    }

    const std::size_t repeatAllCount = args.value<std::size_t>("repeat-all");
    const std::size_t repeatEveryCount = args.value<std::size_t>("repeat-every");
    /* LCOV_EXCL_START */ /* Can't test stuff that aborts the app */
    if(!repeatAllCount || !repeatEveryCount)
        Utility::Fatal() << "You have to repeat at least once";
    /* LCOV_EXCL_STOP */

    /* Repeat the test cases, if requested */
    const std::size_t originalTestCaseCount = usedTestCases.size();
    usedTestCases.reserve(usedTestCases.size()*repeatAllCount);
    for(std::size_t i = 0; i != repeatAllCount - 1; ++i)
        usedTestCases.insert(usedTestCases.end(), usedTestCases.begin(), usedTestCases.begin() + originalTestCaseCount);

    /* Shuffle the test cases, if requested */
    if(args.isSet("shuffle"))
        std::shuffle(usedTestCases.begin(), usedTestCases.end(), std::minstd_rand{std::random_device{}()});

    /* Save the path for diagnostic files, if set; remember verbosity */
    _state->saveDiagnosticPath = args.value("save-diagnostic");
    _state->verbose = args.isSet("verbose");

    unsigned int errorCount = 0,
        noCheckCount = 0;

    /* Nothing to test */
    if(usedTestCases.empty()) {
        /* Not an error if we're skipping either tests or benchmarks (but not
           both) */
        if(args.isSet("skip-tests") && !args.isSet("skip-benchmarks")) {
            Debug(logOutput, _state->useColor)
                << Debug::boldColor(Debug::Color::Default) << "No remaining benchmarks to run in"
                << _state->testName << Debug::nospace << ".";
            return 0;
        }

        if(!args.isSet("skip-tests") && args.isSet("skip-benchmarks")) {
            Debug(logOutput, _state->useColor)
                << Debug::boldColor(Debug::Color::Default) << "No remaining tests to run in"
                << _state->testName << Debug::nospace << ".";
            return 0;
        }

        Error(errorOutput, _state->useColor) << Debug::boldColor(Debug::Color::Red) << "No test cases to run in" << _state->testName << Debug::nospace << "!";
        return 2;
    }

    Debug(logOutput, _state->useColor) << Debug::boldColor(Debug::Color::Default) << "Starting" << _state->testName << "with" << usedTestCases.size() << "test cases...";

    /* If we are running a benchmark, print helpful messages in case the
       benchmark results might be skewed. Inspiration taken from:
       https://github.com/google/benchmark/blob/0ae233ab23c560547bf85ce1346580966e799861/src/sysinfo.cc#L209-L224
       I doubt the code there works on macOS, so enabling it for Linux only. */
    for(std::pair<int, TestCase> testCase: usedTestCases) {
        if(testCase.second.type == TestCaseType::Test) continue;

        if(_state->verbose && _state->isDebugBuild) {
            Debug(logOutput, _state->useColor) << Debug::boldColor(Debug::Color::White) << "  INFO" << Debug::resetColor << "Benchmarking a debug build.";
        }
        #ifdef __linux__
        for(std::size_t i = 0, count = sysconf(_SC_NPROCESSORS_ONLN); i != count; ++i) {
            const std::string file = Utility::formatString(_state->configuration.cpuScalingGovernorFile().data(), i);
            if(!Utility::Directory::exists(file)) break;
            const std::string governor = Utility::String::trim(Utility::Directory::readString(file));
            if(governor != "performance") {
                Warning out{errorOutput, _state->useColor};

                out << Debug::boldColor(Debug::Color::Yellow) << "  WARN"
                    << Debug::resetColor << "CPU" << governor
                    << "scaling detected, benchmark measurements may be noisy.";
                if(_state->verbose) out << "Use\n         sudo cpupower frequency-set --governor performance\n       to get more stable results.";
                break;
            }
        }
        #endif
        break;
    }

    /* Ensure the test case IDs are valid only during the test run */
    Containers::ScopeGuard testCaseIdReset{&*_state, [](TesterState* state) {
        state->testCaseId = ~std::size_t{};
        state->testCaseRepeatId = ~std::size_t{};
        state->testCaseInstanceId = ~std::size_t{};
    }};

    bool abortedOnFail = false;
    for(std::pair<int, TestCase> testCase: usedTestCases) {
        /* Reset output to stdout for each test case to prevent debug
           output segfaults */
        /** @todo Drop this when Debug::setOutput() is removed */
        Debug resetDebugRedirect{&std::cout};
        Error resetErrorRedirect{&std::cerr};
        Utility::Warning resetWarningRedirect{&std::cerr};

        /* Select default benchmark */
        if(testCase.second.type == TestCaseType::DefaultBenchmark)
            testCase.second.type = defaultBenchmarkType;

        /* Select benchmark function */
        BenchmarkUnits benchmarkUnits = BenchmarkUnits::Count;
        switch(testCase.second.type) {
            /* LCOV_EXCL_START */
            case TestCaseType::DefaultBenchmark:
                CORRADE_INTERNAL_ASSERT_UNREACHABLE();
            /* LCOV_EXCL_STOP */

            case TestCaseType::Test:
                break;

            case TestCaseType::WallTimeBenchmark:
                testCase.second.benchmarkBegin = &Tester::wallTimeBenchmarkBegin;
                testCase.second.benchmarkEnd = &Tester::wallTimeBenchmarkEnd;
                benchmarkUnits = BenchmarkUnits::Nanoseconds;
                break;

            case TestCaseType::CpuTimeBenchmark:
                testCase.second.benchmarkBegin = &Tester::cpuTimeBenchmarkBegin;
                testCase.second.benchmarkEnd = &Tester::cpuTimeBenchmarkEnd;
                benchmarkUnits = BenchmarkUnits::Nanoseconds;
                break;

            case TestCaseType::CpuCyclesBenchmark:
                testCase.second.benchmarkBegin = &Tester::cpuCyclesBenchmarkBegin;
                testCase.second.benchmarkEnd = &Tester::cpuCyclesBenchmarkEnd;
                benchmarkUnits = BenchmarkUnits::Cycles;
                break;

            /* These have begin/end provided by the user */
            case TestCaseType::CustomTimeBenchmark:
            case TestCaseType::CustomCycleBenchmark:
            case TestCaseType::CustomInstructionBenchmark:
            case TestCaseType::CustomMemoryBenchmark:
            case TestCaseType::CustomCountBenchmark:
                benchmarkUnits = BenchmarkUnits(int(testCase.second.type));
                _state->benchmarkName = "";
                break;
        }

        _state->testCaseId = testCase.first;
        _state->testCaseInstanceId = testCase.second.instanceId;
        _state->testCaseLabelPrinted = false;
        if(testCase.second.instanceId == ~std::size_t{})
            _state->testCaseDescription = {};
        else
            _state->testCaseDescription = std::to_string(testCase.second.instanceId);

        /* Final combined repeat count */
        const std::size_t repeatCount = testCase.second.repeatCount*repeatEveryCount;

        /* Array with benchmark measurements */
        Containers::Array<std::uint64_t> measurements{testCase.second.type != TestCaseType::Test ? repeatCount : 0};

        bool aborted = false, skipped = false;
        for(std::size_t i = 0; i != repeatCount && !aborted; ++i) {
            if(testCase.second.setup)
                (this->*testCase.second.setup)();

            /* Print the repeat ID only if we are repeating */
            _state->testCaseRepeatId = repeatCount == 1 ? ~std::size_t{} : i;
            _state->testCaseLine = 0;
            _state->testCaseName.clear();
            _state->testCaseTemplateName.clear();
            _state->testCase = &testCase.second;
            _state->benchmarkBatchSize = 0;
            _state->benchmarkResult = 0;

            try {
                (this->*testCase.second.test)();
            } catch(const Exception&) {
                ++errorCount;
                aborted = true;
            } catch(const SkipException&) {
                aborted = true;
                skipped = true;
            } catch(const std::exception& e) {
                /* Conditionally rethrow to let the standard exception abort
                   the process -- useful for debugging */
                if(args.isSet("no-catch")) throw;

                ++errorCount;
                aborted = true;
                Error out{_state->errorOutput, _state->useColor};
                printTestCaseLabel(out, " THROW", Debug::Color::Red,
                    _state->testCaseLine ? Debug::Color::Default : Debug::Color::Yellow);
                /* The file/line info is available but useless because the
                   exception definitely doesn't come from there, thus not
                   printing it. Also not doing ++noCheckCount because the
                   checks could still be there, only after the exception
                   happened. */
                out << Debug::newline << "       " << typeid(e).name() << Debug::nospace << ":" << e.what();
            }

            /* Not catching ... exceptions because those could obscure critical
               problems: https://stackoverflow.com/a/2183971 */

            _state->testCase = nullptr;

            if(testCase.second.teardown)
                (this->*testCase.second.teardown)();

            if(testCase.second.benchmarkEnd)
                measurements[i] = _state->benchmarkResult;

            /* There shouldn't be any stale expected failure after the test
               case exists. If this fires for user code, they did something
               VERY WRONG. (Or I have a serious bug.) */
            CORRADE_INTERNAL_ASSERT(!_state->expectedFailure);
        }

        /* Print success message if the test case wasn't failed/skipped */
        if(!aborted) {
            /* No testing/benchmark macros called */
            if(!_state->testCaseLine) {
                Debug out{logOutput, _state->useColor};
                printTestCaseLabel(out, "     ?", Debug::Color::Yellow, Debug::Color::Yellow);
                ++noCheckCount;

            /* A successful test case. Print the OK only if there wasn't some
               other message (INFO, WARN, XFAIL or SAVED) before, as it would
               otherwise make the output confusing ("is it OK or WARN?!") */
            } else if(testCase.second.type == TestCaseType::Test) {
                if(!_state->testCaseLabelPrinted) {
                    Debug out{logOutput, _state->useColor};
                    printTestCaseLabel(out, "    OK", Debug::Color::Default, Debug::Color::Default);
                }

            /* Benchmark. Completely custom printing. */
            } else {
                /* All other types are benchmarks */
                CORRADE_INTERNAL_ASSERT(testCase.second.type != TestCaseType::Test);

                Debug out{logOutput, _state->useColor};

                const char* padding = PaddingString + sizeof(PaddingString) - digitCount(_state->testCases.size()) + digitCount(_state->testCaseId) - 1;

                out << Debug::boldColor(Debug::Color::Default) << " BENCH"
                    << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
                    << Debug::boldColor(Debug::Color::Cyan) << padding
                    << Debug::nospace << _state->testCaseId << Debug::nospace
                    << Debug::color(Debug::Color::Blue) << "]";

                /* Gather measurements. There needs to be at least one
                   measurememnt left even if the discard count says otherwise. */
                const std::size_t discardMeasurements = measurements.empty() ? 0 :
                        std::min(measurements.size() - 1, args.value<std::size_t>("benchmark-discard"));

                double mean, stddev;
                Utility::Debug::Color color;
                std::tie(mean, stddev, color) = Implementation::calculateStats(measurements.suffix(discardMeasurements), _state->benchmarkBatchSize, args.value<double>("benchmark-yellow"), args.value<double>("benchmark-red"));

                Implementation::printStats(out, mean, stddev, color, benchmarkUnits);

                out << Debug::boldColor(Debug::Color::Default)
                    << _state->formattedTestCaseName() << Debug::nospace;

                /* Optional test case description */
                if(!_state->testCaseDescription.empty()) {
                    out << "("
                        << Debug::nospace
                        << Debug::resetColor << _state->testCaseDescription
                        << Debug::nospace << Debug::boldColor(Debug::Color::Default)
                        << ")";
                } else out << "()";

                out << Debug::nospace << "@" << Debug::nospace
                    << measurements.size() - discardMeasurements
                    << Debug::nospace << "x" << Debug::nospace << _state->benchmarkBatchSize
                    << Debug::resetColor;
                if(!_state->benchmarkName.empty())
                    out << "(" << Utility::Debug::nospace << _state->benchmarkName
                        << Utility::Debug::nospace << ")";
            }

        /* Abort on first failure */
        } else if(args.isSet("abort-on-fail") && !skipped) {
            abortedOnFail = true;
            break;
        }
    }

    /* Print the final wrap-up */
    Debug out(logOutput, _state->useColor);
    if(abortedOnFail) {
        out << Debug::boldColor(Debug::Color::Red) << "Aborted"
            << Debug::boldColor(Debug::Color::Default) << _state->testName
            << Debug::boldColor(Debug::Color::Red) << "after first failure"
            << Debug::boldColor(Debug::Color::Default) << "out of"
            << _state->checkCount << "checks so far.";
    } else {
        out << Debug::boldColor(Debug::Color::Default) << "Finished"
            << _state->testName << "with";
        if(errorCount) out << Debug::boldColor(Debug::Color::Red);
        out << errorCount << "errors";
        if(errorCount) out << Debug::boldColor(Debug::Color::Default);
        out << "out of" << _state->checkCount << "checks.";
    }
    if(_state->diagnosticCount) {
        /* If --save-diagnostic was not enabled but failed checks indicated
           that they *could* save diagnostic files, hint that to the user. */
        if(!_state->saveDiagnosticPath.empty()) {
            out << Debug::boldColor(Debug::Color::Green) << _state->diagnosticCount
                << "checks saved diagnostic files.";
        } else {
            out << Debug::boldColor(Debug::Color::Green) << _state->diagnosticCount
                << "failed checks are able to save diagnostic files, enable "
                   "--save-diagnostic to get them.";
        }
    }
    if(noCheckCount)
        out << Debug::boldColor(Debug::Color::Yellow) << noCheckCount
            << "test cases didn't contain any checks!";

    return errorCount != 0 || noCheckCount != 0;
}

void Tester::printTestCaseLabel(Debug& out, const char* const status, const Debug::Color statusColor, const Debug::Color labelColor) {
    _state->testCaseLabelPrinted = true;

    const char* padding = PaddingString + sizeof(PaddingString) - digitCount(_state->testCases.size()) + digitCount(_state->testCaseId) - 1;

    out << Debug::boldColor(statusColor) << status
        << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
        << Debug::boldColor(Debug::Color::Cyan) << padding
        << Debug::nospace << _state->testCaseId << Debug::nospace
        << Debug::color(Debug::Color::Blue) << "]"
        << Debug::boldColor(labelColor) << _state->formattedTestCaseName()
        << Debug::nospace;

    /* Optional test case description */
    if(!_state->testCaseDescription.empty()) {
        out << "("
            << Debug::nospace
            << Debug::resetColor << _state->testCaseDescription
            << Debug::nospace << Debug::boldColor(labelColor)
            << ")";
    } else out << "()";

    if(_state->testCaseRepeatId != ~std::size_t{})
        out << Debug::nospace << "@" << Debug::nospace << _state->testCaseRepeatId + 1;

    out << Debug::resetColor;
}

void Tester::printFileLineInfo(Debug& out) {
    out << "at" << _state->testFilename << Debug::nospace << ":" << Debug::nospace << _state->testCaseLine;

    /* If we have checks annotated with an iteration macro, print those. These
       are linked in reverse order so we have to reverse the vector before
       printing. */
    if(_state->iterationPrinter) {
        std::vector<std::string> iterations;
        for(IterationPrinter* iterationPrinter = _state->iterationPrinter; iterationPrinter; iterationPrinter = iterationPrinter->_data->parent) {
            iterations.push_back(iterationPrinter->_data->out.str());
        }
        std::reverse(iterations.begin(), iterations.end());
        out << "(iteration" << Utility::String::join(iterations, ", ") << Debug::nospace << ")";
    }

    out << Debug::newline;
}

void Tester::verifyInternal(const char* expression, bool expressionValue) {
    ++_state->checkCount;

    /* If the expression is true or the failure is expected, done */
    if(!_state->expectedFailure) {
        if(expressionValue) return;
    } else if(!expressionValue) {
        Debug out{_state->logOutput, _state->useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        printFileLineInfo(out);
        out << "       " << _state->expectedFailureMessage << "Expression"
            << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Error out{_state->errorOutput, _state->useColor};
    printTestCaseLabel(out, _state->expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
    printFileLineInfo(out);
    out << "        Expression" << expression;
    if(!_state->expectedFailure) out << "failed.";
    else out << "was expected to fail.";
    throw Exception();
}

void Tester::printComparisonMessageInternal(ComparisonStatusFlags flags, const char* actual, const char* expected, void(*printer)(void*, ComparisonStatusFlags, Debug&, const char*, const char*), void(*saver)(void*, ComparisonStatusFlags, Debug&, const std::string&), void* comparator) {
    ++_state->checkCount;

    /* If verbose output is not enabled, remove verbose stuff from comparison
       status flags */
    if(!_state->verbose) flags &= ~(ComparisonStatusFlag::Verbose|ComparisonStatusFlag::VerboseDiagnostic);

    /* In case of an expected failure, print a static message */
    if(_state->expectedFailure && (flags & ComparisonStatusFlag::Failed)) {
        Debug out{_state->logOutput, _state->useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        printFileLineInfo(out);
        out << "       " << _state->expectedFailureMessage << actual << "and"
            << expected << "failed the comparison.";

    /* Otherwise, in case of an unexpected failure or an unexpected pass, print
       an error message */
    } else if(bool(_state->expectedFailure) != bool(flags & ComparisonStatusFlag::Failed)) {
        Error out{_state->errorOutput, _state->useColor};
        printTestCaseLabel(out, _state->expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
        printFileLineInfo(out);
        out << "       ";
        if(!_state->expectedFailure) printer(comparator, flags, out, actual, expected);
        else out << actual << "and" << expected << "were expected to fail the comparison.";

    /* Otherwise, if the comparison succeeded but the comparator wants to print
       a message, let it do that as well */
    /** @todo print also in case of XFAIL or XPASS? those currently get just a
        static message and printer is never called */
    } else if(flags & (ComparisonStatusFlag::Warning|ComparisonStatusFlag::Message|ComparisonStatusFlag::Verbose)) {
        Debug out{_state->logOutput, _state->useColor};
        printTestCaseLabel(out,
            flags & ComparisonStatusFlag::Warning ? "  WARN" : "  INFO",
            flags & ComparisonStatusFlag::Warning ? Debug::Color::Yellow : Debug::Color::Default,
            Debug::Color::Default);
        printFileLineInfo(out);
        out << "       ";
        printer(comparator, flags, out, actual, expected);
    }

    /* Save diagnostic file(s) if the comparator wants to, it's not
       in an XFAIL (because XFAIL should be silent, OTOH XPASS should do the
       same as FAIL), ... */
    if((flags & (ComparisonStatusFlag::Diagnostic|ComparisonStatusFlag::VerboseDiagnostic)) && !(_state->expectedFailure && flags & ComparisonStatusFlag::Failed)) {
        /* ... and the user allowed that. */
        if(!_state->saveDiagnosticPath.empty()) {
            CORRADE_ASSERT(saver, "TestSuite::Comparator: comparator returning ComparisonStatusFlag::[Verbose]Diagnostic has to implement saveDiagnostic() as well", );

            Debug out{_state->logOutput, _state->useColor};
            printTestCaseLabel(out, " SAVED", Debug::Color::Green, Debug::Color::Default);
            saver(comparator, flags, out, _state->saveDiagnosticPath);
            ++_state->diagnosticCount;

        /* If the user didn't allow, count all failure diagnostics in order to
           hint to the user that there's --save-diagnostic in the final output */
        } else if(bool(_state->expectedFailure) != bool(flags & ComparisonStatusFlag::Failed)) {
            ++_state->diagnosticCount;
        }
    }

    /* Throw an exception if this is an error */
    if(bool(_state->expectedFailure) != bool(flags & ComparisonStatusFlag::Failed))
        throw Exception();
}

void Tester::registerTest(const char* filename, const char* name, bool isDebugBuild) {
    _state->testFilename = std::move(filename);
    if(_state->testName.empty()) _state->testName = std::move(name);
    _state->isDebugBuild = isDebugBuild;
}

void Tester::skip(const char* message) {
    skip(std::string{message});
}

void Tester::skip(const std::string& message) {
    Debug out{_state->logOutput, _state->useColor};
    printTestCaseLabel(out, "  SKIP", Debug::Color::Default, Debug::Color::Default);
    out << Debug::newline << "       " << message;
    throw SkipException();
}

std::size_t Tester::testCaseId() const {
    CORRADE_ASSERT(_state->testCaseId != ~std::size_t{},
        "TestSuite::Tester::testCaseId(): can be called only from within a test case", {});
    return _state->testCaseId;
}

std::size_t Tester::testCaseInstanceId() const {
    CORRADE_ASSERT(_state->testCaseInstanceId != ~std::size_t{},
        "TestSuite::Tester::testCaseInstanceId(): can be called only from within an instanced test case", {});
    return _state->testCaseInstanceId;
}

std::size_t Tester::testCaseRepeatId() const {
    CORRADE_ASSERT(_state->testCaseRepeatId != ~std::size_t{},
        "TestSuite::Tester::testCaseRepeatId(): can be called only from within a repeated test case", {});
    return _state->testCaseRepeatId;
}

Containers::StringView Tester::testName() const {
    return _state->testName;
}

void Tester::setTestName(const std::string& name) {
    _state->testName = name;
}

void Tester::setTestName(std::string&& name) {
    _state->testName = std::move(name);
}

void Tester::setTestName(const char* name) {
    _state->testName = name;
}

void Tester::setTestCaseName(const std::string& name) {
    _state->testCaseName = name;
}

void Tester::setTestCaseName(std::string&& name) {
    _state->testCaseName = std::move(name);
}

void Tester::setTestCaseName(const char* name) {
    _state->testCaseName = name;
}

void Tester::setTestCaseTemplateName(const std::string& name) {
    _state->testCaseTemplateName = name;
}

void Tester::setTestCaseTemplateName(std::string&& name) {
    _state->testCaseTemplateName = std::move(name);
}

void Tester::setTestCaseTemplateName(const char* name) {
    _state->testCaseTemplateName = name;
}

void Tester::setTestCaseTemplateName(const std::initializer_list<Containers::StringView> names) {
    _state->testCaseTemplateName = Utility::String::join({names.begin(), names.end()}, ", ");
}

void Tester::setTestCaseTemplateName(const std::initializer_list<const char*> names) {
    _state->testCaseTemplateName = Utility::String::join({names.begin(), names.end()}, ", ");
}

void Tester::setTestCaseDescription(const std::string& description) {
    _state->testCaseDescription = description;
}

void Tester::setTestCaseDescription(std::string&& description) {
    _state->testCaseDescription = std::move(description);
}

void Tester::setTestCaseDescription(const char* description) {
    _state->testCaseDescription = description;
}

void Tester::setBenchmarkName(const std::string& name) {
    _state->benchmarkName = name;
}

void Tester::setBenchmarkName(std::string&& name) {
    _state->benchmarkName = std::move(name);
}

void Tester::setBenchmarkName(const char* name) {
    _state->benchmarkName = name;
}

void Tester::registerTestCase(const char* name, int line) {
    CORRADE_ASSERT(_state->testCase,
        "TestSuite::Tester: using verification macros outside of test cases is not allowed", );

    if(_state->testCaseName.empty()) _state->testCaseName = std::move(name);
    _state->testCaseLine = line;
}

Tester::BenchmarkRunner Tester::createBenchmarkRunner(const std::size_t batchSize) {
    CORRADE_ASSERT(_state->testCase,
        "TestSuite::Tester: using benchmark macros outside of test cases is not allowed",
        (BenchmarkRunner{nullptr, nullptr}));

    _state->benchmarkBatchSize = batchSize;
    return BenchmarkRunner{_state->testCase->benchmarkBegin, _state->testCase->benchmarkEnd};
}

void Tester::wallTimeBenchmarkBegin() {
    _state->benchmarkName = "wall time";
    _state->benchmarkBegin = Implementation::wallTime();
}

std::uint64_t Tester::wallTimeBenchmarkEnd() {
    return Implementation::wallTime() - _state->benchmarkBegin;
}

void Tester::cpuTimeBenchmarkBegin() {
    _state->benchmarkName = "CPU time";
    _state->benchmarkBegin = Implementation::cpuTime();
}

std::uint64_t Tester::cpuTimeBenchmarkEnd() {
    return Implementation::cpuTime() - _state->benchmarkBegin;
}

void Tester::cpuCyclesBenchmarkBegin() {
    _state->benchmarkName = "CPU cycles";
    _state->benchmarkBegin = Implementation::rdtsc();
}

std::uint64_t Tester::cpuCyclesBenchmarkEnd() {
    return Implementation::rdtsc() - _state->benchmarkBegin;
}

void Tester::addTestCaseInternal(const TestCase& testCase) {
    _state->testCases.push_back(testCase);
}

Tester::ExpectedFailure::ExpectedFailure(std::string&& message, const bool enabled) {
    Tester& instance = Tester::instance();
    if(!enabled || instance._state->expectedFailuresDisabled) return;
    instance._state->expectedFailureMessage = message;
    instance._state->expectedFailure = this;
}

Tester::ExpectedFailure::ExpectedFailure(const std::string& message, const bool enabled): ExpectedFailure{std::string{message}, enabled} {}

Tester::ExpectedFailure::ExpectedFailure(const char* message, const bool enabled): ExpectedFailure{std::string{message}, enabled} {}

Tester::ExpectedFailure::~ExpectedFailure() {
    instance()._state->expectedFailure = nullptr;
}

Tester::IterationPrinter::IterationPrinter() {
    Tester& instance = Tester::instance();
    /* Insert itself into the list of iteration printers */
    _data.emplace().parent = instance._state->iterationPrinter;
    instance._state->iterationPrinter = this;
}

Tester::IterationPrinter::~IterationPrinter() {
    /* Remove itself from the list of iteration printers (assuming destruction
       of those goes in inverse order) */
    CORRADE_INTERNAL_ASSERT(instance()._state->iterationPrinter == this);
    instance()._state->iterationPrinter = _data->parent;
}

Utility::Debug Tester::IterationPrinter::debug() {
    return Debug{&_data->out, Debug::Flag::NoNewlineAtTheEnd};
}

Tester::BenchmarkRunner::~BenchmarkRunner() {
    _instance._state->benchmarkResult = (_instance.*_end)();
}

const char* Tester::BenchmarkRunner::end() const {
     return reinterpret_cast<char*>(_instance._state->benchmarkBatchSize);
}

}}
