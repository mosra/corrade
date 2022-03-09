/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>

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
#include <algorithm> /* std::shuffle() */
#include <iostream>
#include <random> /* random device for std::shuffle() */
#include <sstream>
#include <typeinfo>
#include <utility>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/TestSuite/Implementation/BenchmarkCounters.h"
#include "Corrade/TestSuite/Implementation/BenchmarkStats.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/String.h"

#ifdef __linux__ /* for getting processor count */
#include <unistd.h>
#endif

namespace Corrade { namespace TestSuite {

using namespace Containers::Literals;

namespace {
    inline int digitCount(int number) {
        int digits = 0;
        while(number != 0) number /= 10, digits++;
        return digits;
    }

    constexpr const char PaddingString[] = "0000000000";

    #ifdef __linux__
    constexpr Containers::StringView DefaultCpuScalingGovernorFile = "/sys/devices/system/cpu/cpu{}/cpufreq/scaling_governor"_s;
    #endif
}

struct Tester::TesterConfiguration::Data {
    explicit Data() = default;

    Data(const Data& other)
        #ifdef __linux__
        : cpuScalingGovernorFile{other.cpuScalingGovernorFile}
        #endif
    {
        arrayAppend(skippedArgumentPrefixes, arrayView(other.skippedArgumentPrefixes));
    }

    Containers::Array<Containers::String> skippedArgumentPrefixes;
    #ifdef __linux__
    Containers::String cpuScalingGovernorFile = Containers::String::nullTerminatedGlobalView(DefaultCpuScalingGovernorFile);
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

Containers::Array<Containers::StringView> Tester::TesterConfiguration::skippedArgumentPrefixes() const {
    if(!_data) return nullptr;

    Containers::Array<Containers::StringView> out{NoInit, _data->skippedArgumentPrefixes.size()};
    for(std::size_t i = 0; i != out.size(); ++i)
        new(&out[i]) Containers::StringView{_data->skippedArgumentPrefixes[i]};
    return out;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::setSkippedArgumentPrefixes(std::initializer_list<Containers::StringView> prefixes) {
    if(!_data) _data.reset(new Data);
    const Containers::ArrayView<const Containers::StringView> in = Containers::arrayView(prefixes);
    const Containers::ArrayView<Containers::String> out = arrayAppend(_data->skippedArgumentPrefixes, NoInit, prefixes.size());
    for(std::size_t i = 0; i != prefixes.size(); ++i) {
        new(&out[i]) Containers::String{Containers::String::nullTerminatedGlobalView(in[i])};
    }

    return *this;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::setSkippedArgumentPrefixes(std::initializer_list<const char*> prefixes) {
    if(!_data) _data.reset(new Data);
    const Containers::ArrayView<const char* const> in = Containers::arrayView(prefixes);
    const Containers::ArrayView<Containers::String> out = arrayAppend(_data->skippedArgumentPrefixes, NoInit, prefixes.size());
    for(std::size_t i = 0; i != prefixes.size(); ++i) {
        /* We can't make any assumptions about globality of the char* literals here */
        new(&out[i]) Containers::String{in[i]};
    }

    return *this;
}

#ifdef __linux__
Containers::StringView Tester::TesterConfiguration::cpuScalingGovernorFile() const {
    return _data ? Containers::StringView{_data->cpuScalingGovernorFile} : DefaultCpuScalingGovernorFile;
}

Tester::TesterConfiguration& Tester::TesterConfiguration::setCpuScalingGovernorFile(const Containers::StringView filename) {
    if(!_data) _data.reset(new Data);
    _data->cpuScalingGovernorFile = Containers::String::nullTerminatedGlobalView(filename);
    return *this;
}
#endif

struct Tester::Printer::Printer::Data {
    std::ostringstream out;
};

struct Tester::TesterState {
    explicit TesterState(const TesterConfiguration& configuration): configuration{std::move(configuration)} {}

    void printFormattedTestCaseName(Debug& out) const {
        if(!testCaseName) {
            out << "<unknown>";
            return;
        }

        out << testCaseName;
        if(testCaseTemplateName)
            out << Debug::nospace << "<" << Debug::nospace << testCaseTemplateName << Debug::nospace << ">";
    }

    Debug::Flags useColor;
    std::ostream *logOutput{}, *errorOutput{};
    std::vector<TestCase> testCases;
    /* Assuming this always comes from a global string literals (__FILE__ in
       CORRADE_TEST_MAIN()) */
    Containers::StringView testFilename;
    /* These implicitly come from a stringified class name or CORRADE_FUNCTION
       but can also be overriden, either with a global string literal but also
       from a formatted string or whatever else, so have a possibility to own
       them */
    Containers::String testName, testCaseName, testCaseTemplateName,
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
    IterationPrinter* iterationPrinter{};
    TesterConfiguration configuration;

    /* This could practically be a StringView always as it's only used inside
       exec(), taken from args that are alive until the end, but let's play it
       safe */
    Containers::String saveDiagnosticPath;
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
        .addOption("skip").setHelp("skip", "skip test cases with given numbers", "N1,N2-N3…")
        .addBooleanOption("skip-tests").setHelp("skip-tests", "skip all tests")
            .setFromEnvironment("skip-tests", "CORRADE_TEST_SKIP_TESTS")
        .addBooleanOption("skip-benchmarks").setHelp("skip-benchmarks", "skip all benchmarks")
            .setFromEnvironment("skip-benchmarks", "CORRADE_TEST_SKIP_BENCHMARKS")
        .addOption("only").setHelp("only", "run only test cases with given numbers", "N1,N2-N3…")
        .addBooleanOption("shuffle").setHelp("shuffle", "randomly shuffle test case order")
            .setFromEnvironment("shuffle", "CORRADE_TEST_SHUFFLE")
        .addOption("repeat-every", "1").setHelp("repeat-every", "repeat every test case N times", "N")
            .setFromEnvironment("repeat-every", "CORRADE_TEST_REPEAT_EVERY")
        .addOption("repeat-all", "1").setHelp("repeat-all", "repeat all test cases N times", "N")
            .setFromEnvironment("repeat-all", "CORRADE_TEST_REPEAT_ALL")
        .addBooleanOption('X', "abort-on-fail").setHelp("abort-on-fail", "abort after first failure")
            .setFromEnvironment("abort-on-fail", "CORRADE_TEST_ABORT_ON_FAIL")
        .addBooleanOption("no-xfail").setHelp("no-xfail", "disallow expected failures")
            .setFromEnvironment("no-xfail", "CORRADE_TEST_NO_XFAIL")
        .addBooleanOption("no-catch").setHelp("no-catch", "don't catch standard exceptions")
            .setFromEnvironment("no-catch", "CORRADE_TEST_NO_CATCH")
        .addOption('S', "save-diagnostic", "").setHelp("save-diagnostic", "save diagnostic files to given path", "PATH")
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
    else {
        Utility::Error{} << "TestSuite::Tester::exec(): unknown benchmark type" << args.value("benchmark")
        << Utility::Debug::nospace << ", use one of wall-time, cpu-time or cpu-cycles";
        return 1;
    }

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
        /* The test case numbering is 1-based. The max argument ensures IDs out
           of bounds are skipped. If the parsing fails due to strange
           characters, bail. The error should be already printed by the
           utility. */
        const Containers::Optional<Containers::Array<std::uint32_t>> range = Utility::String::parseNumberSequence(args.value<Containers::StringView>("skip"), 1, _state->testCases.size() + 1);
        if(!range) return 2;
        for(std::uint32_t index: *range)
            _state->testCases[index - 1].test = nullptr;
    }

    /* Extract only whitelisted test cases if requested (and skip skipped) */
    if(!args.value("only").empty()) {
        /* The test case numbering is 1-based. The max argument ensures IDs out
           of bounds are skipped. If the parsing fails due to strange
           characters, bail. The error should be already printed by the
           utility. */
        const Containers::Optional<Containers::Array<std::uint32_t>> range = Utility::String::parseNumberSequence(args.value<Containers::StringView>("only"), 1, _state->testCases.size() + 1);
        if(!range) return 2;
        for(std::uint32_t index: *range)
            if(_state->testCases[index - 1].test)
                usedTestCases.emplace_back(index, _state->testCases[index - 1]);

    /* Otherwise extract all (and skip skipped) */
    } else for(std::size_t i = 0; i != _state->testCases.size(); ++i) {
        if(!_state->testCases[i].test) continue;
        usedTestCases.emplace_back(i + 1, _state->testCases[i]);
    }

    const std::size_t repeatAllCount = args.value<std::size_t>("repeat-all");
    const std::size_t repeatEveryCount = args.value<std::size_t>("repeat-every");
    if(!repeatAllCount || !repeatEveryCount) {
        Utility::Error{} << "TestSuite::Tester::exec(): you have to repeat at least once";
        return 1;
    }

    /* Repeat the test cases, if requested */
    const std::size_t originalTestCaseCount = usedTestCases.size();
    usedTestCases.reserve(usedTestCases.size()*repeatAllCount);
    for(std::size_t i = 0; i != repeatAllCount - 1; ++i)
        usedTestCases.insert(usedTestCases.end(), usedTestCases.begin(), usedTestCases.begin() + originalTestCaseCount);

    /* Shuffle the test cases, if requested */
    if(args.isSet("shuffle"))
        std::shuffle(usedTestCases.begin(), usedTestCases.end(), std::minstd_rand{std::random_device{}()});

    /* Save the path for diagnostic files, if set. This could practically be
       a StringView always as it's only used inside this function and args is
       alive until the end, but let's play it safe. */
    _state->saveDiagnosticPath = Containers::String::nullTerminatedGlobalView(args.value<Containers::StringView>("save-diagnostic"));

    /* Remember verbosity */
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
            /* Try reading the CPU scaling governor file, if it exists. It
               doesn't exist on some but not all Androids, and it doesn't make
               sense to print a "No such file" error from read() when running
               any tests on such systems. */
            const Containers::String filename = Utility::format(_state->configuration.cpuScalingGovernorFile().data(), i);
            Containers::Optional<Containers::String> file;
            if(!Utility::Path::exists(filename) || !(file = Utility::Path::readString(filename)))
                break;
            const Containers::StringView governor = file->trimmed();
            if(governor != "performance"_s) {
                Warning out{errorOutput, _state->useColor};

                out << Debug::boldColor(Debug::Color::Yellow) << "  WARN"
                    << Debug::resetColor << "CPU" << governor
                    << "scaling detected, benchmark measurements may be noisy.";
                if(_state->verbose) {
                    #ifndef CORRADE_TARGET_ANDROID
                    out << "Use\n"
                        "         sudo cpupower frequency-set --governor performance\n"
                        "       to get more stable results.";
                    #else
                    out << "Use\n"
                        "         echo performance | tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor\n"
                        "       on a rooted device to get more stable results.";
                    #endif
                }
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
            _state->testCaseName = {};
            _state->testCaseTemplateName = {};
            _state->testCase = &testCase.second;
            _state->benchmarkBatchSize = 0;
            _state->benchmarkResult = 0;

            try {
                /* If --no-catch is specified, let the standard exception
                   abort the process -- useful for debugging. Can't handle this
                   in the parent try/catch block and conditionally rethrow
                   because that would cause the backtrace to point here and not
                   to the original exception location. */
                if(args.isSet("no-catch")) {
                    (this->*testCase.second.test)();
                } else try {
                    (this->*testCase.second.test)();
                } catch(const std::exception& e) {
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
            } catch(const Exception&) {
                ++errorCount;
                aborted = true;
            } catch(const SkipException&) {
                aborted = true;
                skipped = true;
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
                const std::size_t discardMeasurements = measurements.isEmpty() ? 0 :
                        Utility::min(measurements.size() - 1, args.value<std::size_t>("benchmark-discard"));

                double mean, stddev;
                Utility::Debug::Color color;
                std::tie(mean, stddev, color) = Implementation::calculateStats(measurements.exceptPrefix(discardMeasurements), _state->benchmarkBatchSize, args.value<double>("benchmark-yellow"), args.value<double>("benchmark-red"));

                Implementation::printStats(out, mean, stddev, color, benchmarkUnits);

                out << Debug::boldColor(Debug::Color::Default);
                _state->printFormattedTestCaseName(out);
                out << Debug::nospace;

                /* Optional test case description */
                if(_state->testCaseDescription) {
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
                if(_state->benchmarkName)
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
        if(_state->saveDiagnosticPath) {
            out << Debug::boldColor(Debug::Color::Green) << _state->diagnosticCount
                << "checks saved diagnostic files.";
        } else {
            out << Debug::boldColor(Debug::Color::Green) << _state->diagnosticCount
                << "failed checks are able to save diagnostic files, enable "
                   "-S / --save-diagnostic to get them.";
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
        << Debug::boldColor(labelColor);
    _state->printFormattedTestCaseName(out);
    out << Debug::nospace;

    /* Optional test case description */
    if(_state->testCaseDescription) {
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

void Tester::printFileLineInfo(Debug& out, std::size_t line) {
    out << "at" << _state->testFilename << Debug::nospace << ":" << Debug::nospace << line;

    /* If we have checks annotated with an iteration macro, print those. These
       are linked in reverse order so we have to reverse the vector before
       printing. */
    if(_state->iterationPrinter) {
        /** @todo remove std::string once Debug doesn't rely on streams */
        std::vector<std::string> iterations;
        for(IterationPrinter* iterationPrinter = _state->iterationPrinter; iterationPrinter; iterationPrinter = iterationPrinter->_parent) {
            iterations.push_back(iterationPrinter->_data->out.str());
        }
        std::reverse(iterations.begin(), iterations.end());
        out << "(iteration" << Utility::String::join(iterations, ", ") << Debug::nospace << ")";
    }

    out << Debug::newline;
}

void Tester::printFileLineInfo(Debug& out) {
    printFileLineInfo(out, _state->testCaseLine);
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
        out << "       " << _state->expectedFailure->_data->out.str() << "Expression"
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

void Tester::printComparisonMessageInternal(ComparisonStatusFlags flags, const char* actual, const char* expected, void(*printer)(void*, ComparisonStatusFlags, Debug&, const char*, const char*), void(*saver)(void*, ComparisonStatusFlags, Debug&, const Containers::StringView&), void* comparator) {
    ++_state->checkCount;

    /* If verbose output is not enabled, remove verbose stuff from comparison
       status flags */
    if(!_state->verbose) flags &= ~(ComparisonStatusFlag::Verbose|ComparisonStatusFlag::VerboseDiagnostic);

    /* In case of an expected failure, print a static message */
    if(_state->expectedFailure && (flags & ComparisonStatusFlag::Failed)) {
        Debug out{_state->logOutput, _state->useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        printFileLineInfo(out);
        out << "       " << _state->expectedFailure->_data->out.str() << actual << "and"
            << expected << "failed the comparison.";

    /* Otherwise, in case of an unexpected failure or an unexpected pass, print
       an error message */
    } else if(bool(_state->expectedFailure) != bool(flags & ComparisonStatusFlag::Failed)) {
        /** @todo deduplicate this with fail()? */
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
        /** @todo deduplicate this with infoOrWarn()? */
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
        if(_state->saveDiagnosticPath) {
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
    /* The file is __FILE__, thus assumed to be global */
    _state->testFilename = Containers::StringView{filename, Containers::StringViewFlag::Global};
    /* The name is a stringified class name, thus also assumed to be global */
    if(!_state->testName) _state->testName = Containers::String::nullTerminatedGlobalView(Containers::StringView{name, Containers::StringViewFlag::Global});
    _state->isDebugBuild = isDebugBuild;
}

void Tester::infoOrWarn(const Printer& printer, std::size_t line, bool warn) {
    Debug out{_state->logOutput, _state->useColor};
    printTestCaseLabel(out,
        warn ? "  WARN" : "  INFO",
        warn ? Debug::Color::Yellow : Debug::Color::Default,
        Debug::Color::Default);
    printFileLineInfo(out, line);
    out << "       " << printer._data->out.str();
}

void Tester::failIf(const Printer& printer, const bool fail) {
    if(_state->expectedFailure && fail) {
        Debug out{_state->logOutput, _state->useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        printFileLineInfo(out);
        /** @todo this is extremely uninformative, implement the verbose output
            for XFAIL/XPASS at least, or figure out a better way to report
            this */
        out << "       " << _state->expectedFailure->_data->out.str() << "Condition failed.";
        return;
    }

    if(bool(_state->expectedFailure) != fail) {
        Error out{_state->errorOutput, _state->useColor};
        printTestCaseLabel(out, _state->expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
        printFileLineInfo(out);
        out << "       ";
        if(!_state->expectedFailure) out << printer._data->out.str();
        else out << "Failure was expected to happen.";
        throw Exception{};
    }
}

void Tester::skip(const Printer& printer) {
    Debug out{_state->logOutput, _state->useColor};
    printTestCaseLabel(out, "  SKIP", Debug::Color::Default, Debug::Color::Default);
    out << Debug::newline << "       " << printer._data->out.str();
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

void Tester::setTestName(const Containers::StringView name) {
    /* If the name comes from a global string literal, avoid a needless copy */
    _state->testName = Containers::String::nullTerminatedGlobalView(name);
}

void Tester::setTestName(const char* const name) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestName(Containers::StringView{name});
}

Containers::StringView Tester::testCaseName() const {
    return _state->testCaseName;
}

void Tester::setTestCaseName(const Containers::StringView name) {
    /* If the name comes from a global string literal, avoid a needless copy */
    _state->testCaseName = Containers::String::nullTerminatedGlobalView(name);
}

void Tester::setTestCaseName(const char* const name) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseName(Containers::StringView{name});
}

Containers::StringView Tester::testCaseTemplateName() const {
    return _state->testCaseTemplateName;
}

void Tester::setTestCaseTemplateName(const Containers::StringView name) {
    /* If the name comes from a global string literal, avoid a needless copy */
    _state->testCaseTemplateName = Containers::String::nullTerminatedGlobalView(name);
}

void Tester::setTestCaseTemplateName(const char* const name) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseTemplateName(Containers::StringView{name});
}

void Tester::setTestCaseTemplateName(const Containers::ArrayView<const Containers::StringView> names) {
    _state->testCaseTemplateName = ", "_s.join(names);
}

void Tester::setTestCaseTemplateName(const std::initializer_list<Containers::StringView> names) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseTemplateName(Containers::arrayView(names));
}

void Tester::setTestCaseTemplateName(const Containers::ArrayView<const char* const> names) {
    Containers::Array<Containers::StringView> out{NoInit, names.size()};
    for(std::size_t i = 0; i != names.size(); ++i)
        new(&out[i]) Containers::StringView{names[i]};
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseTemplateName(out);
}

void Tester::setTestCaseTemplateName(const std::initializer_list<const char*> names) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseTemplateName(Containers::arrayView(names));
}

Containers::StringView Tester::testCaseDescription() const {
    return _state->testCaseDescription;
}

void Tester::setTestCaseDescription(const Containers::StringView description) {
    /* If the name comes from a global string literal, avoid a needless copy */
    _state->testCaseDescription = Containers::String::nullTerminatedGlobalView(description);
}

void Tester::setTestCaseDescription(const char* const description) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setTestCaseDescription(Containers::StringView{description});
}

Containers::StringView Tester::benchmarkName() const {
    return _state->benchmarkName;
}

void Tester::setBenchmarkName(const Containers::StringView name) {
    /* If the name comes from a global string literal, avoid a needless copy */
    _state->benchmarkName = Containers::String::nullTerminatedGlobalView(name);
}

void Tester::setBenchmarkName(const char* const name) {
    /* Delegating to the above overload instead of going directly to have both
       implicitly covered by the tests */
    setBenchmarkName(Containers::StringView{name});
}

void Tester::registerTestCase(const char* name) {
    CORRADE_ASSERT(_state->testCase,
        "TestSuite::Tester: using verification macros outside of test cases is not allowed", );

    /* The name is CORRADE_FUNCTION, thus assumed to be global */
    if(!_state->testCaseName) _state->testCaseName = Containers::String::nullTerminatedGlobalView(Containers::StringView{name, Containers::StringViewFlag::Global});
}

void Tester::registerTestCase(const char* name, int line) {
    registerTestCase(name);
    _state->testCaseLine = line;
}

Tester::BenchmarkRunner Tester::createBenchmarkRunner(const std::size_t batchSize) {
    CORRADE_ASSERT(_state->testCase,
        "TestSuite::Tester: using benchmark macros outside of test cases is not allowed",
        (BenchmarkRunner{nullptr, nullptr}));
    CORRADE_ASSERT(_state->testCase->benchmarkBegin && _state->testCase->benchmarkEnd,
        "TestSuite::Tester: CORRADE_BENCHMARK() can be called only inside a benchmark",
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

Utility::Debug Tester::Printer::debug() {
    return Debug{&_data->out, Debug::Flag::NoNewlineAtTheEnd};
}

Tester::Printer::Printer(): _data{InPlaceInit} {}

Tester::Printer::~Printer() = default;

Tester::ExpectedFailure::ExpectedFailure(const bool enabled) {
    Tester& instance = Tester::instance();
    if(!enabled || instance._state->expectedFailuresDisabled) return;
    /** @todo some assert to avoid multiple active expected failures at the same time */
    instance._state->expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    Tester::instance()._state->expectedFailure = nullptr;
}

Tester::IterationPrinter::IterationPrinter() {
    Tester& instance = Tester::instance();
    /* Insert itself into the list of iteration printers */
    _parent = instance._state->iterationPrinter;
    instance._state->iterationPrinter = this;
}

Tester::IterationPrinter::~IterationPrinter() {
    /* Remove itself from the list of iteration printers (assuming destruction
       of those goes in inverse order) */
    CORRADE_INTERNAL_ASSERT(instance()._state->iterationPrinter == this);
    instance()._state->iterationPrinter = _parent;
}

Tester::BenchmarkRunner::~BenchmarkRunner() {
    _instance._state->benchmarkResult = (_instance.*_end)();
}

const char* Tester::BenchmarkRunner::end() const {
     return reinterpret_cast<char*>(_instance._state->benchmarkBatchSize);
}

}}
