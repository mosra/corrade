/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
#include <utility>

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Implementation/BenchmarkCounters.h"
#include "Corrade/TestSuite/Implementation/BenchmarkStats.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/String.h"

#ifdef CORRADE_TARGET_ANDROID
#include <sstream>
#endif

namespace Corrade { namespace TestSuite {

namespace {
    inline int digitCount(int number) {
        int digits = 0;
        while(number != 0) number /= 10, digits++;
        return digits;
    }

    constexpr const char PaddingString[] = "0000000000";
}

int* Tester::_argc = nullptr;
char** Tester::_argv = nullptr;

void Tester::registerArguments(int& argc, char** const argv) {
    _argc = &argc;
    _argv = argv;
}

Tester::Tester(TesterConfiguration configuration): _logOutput{nullptr}, _errorOutput{nullptr}, _testCaseLine{0}, _checkCount{0}, _expectedFailure{nullptr}, _configuration{std::move(configuration)} {
    CORRADE_ASSERT(_argc, "TestSuite::Tester: command-line arguments not available", );
}

Tester::~Tester() {
    /* Reset argument pointers to avoid accidentally forgotten calls to registerArguments() */
    _argc = nullptr;
    _argv = nullptr;
}

int Tester::exec() { return exec(&std::cout, &std::cerr); }

int Tester::exec(std::ostream* const logOutput, std::ostream* const errorOutput) {
    Utility::Arguments args;
    for(auto&& prefix: _configuration.skippedArgumentPrefixes())
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
        .addOption("benchmark", "wall-time").setHelp("benchmark", "default benchmark type", "TYPE")
            .setFromEnvironment("benchmark", "CORRADE_BENCHMARK")
        .addOption("benchmark-discard", "1").setHelp("benchmark-discard", "discard first N measurements of each benchmark", "N")
            .setFromEnvironment("benchmark-discard", "CORRADE_BENCHMARK_DISCARD")
        .addOption("benchmark-yellow", "0.05").setHelp("benchmark-yellow", "deviation threshold for marking benchmark yellow", "N")
            .setFromEnvironment("benchmark-yellow", "CORRADE_BENCHMARK_YELLOW")
        .addOption("benchmark-red", "0.25").setHelp("benchmark-red", "deviation threshold for marking benchmark red", "N")
            .setFromEnvironment("benchmark-red", "CORRADE_BENCHMARK_RED")
        .setHelp(R"(Corrade TestSuite executable. By default runs test cases in order in which they
were added and exits with non-zero code if any of them failed. Supported
benchmark types:
  wall-time     wall time spent
  cpu-time      CPU time spent
  cpu-cycles    CPU cycles spent (x86 only, gives zero result elsewhere))")
        .parse(*_argc, _argv);

    _logOutput = logOutput;
    _errorOutput = errorOutput;

    /* Decide about color */
    if(args.value("color") == "on" || args.value("color") == "ON")
        _useColor = Debug::Flags{};
    else if(args.value("color") == "off" || args.value("color") == "OFF")
        _useColor = Debug::Flag::DisableColors;
    else _useColor = Debug::isTty(logOutput) && Debug::isTty(errorOutput) ?
            Debug::Flags{} : Debug::Flag::DisableColors;

    /* Decide about default benchmark type */
    TestCaseType defaultBenchmarkType{};
    if(args.value("benchmark") == "wall-time")
        defaultBenchmarkType = TestCaseType::WallTimeBenchmark;
    else if(args.value("benchmark") == "cpu-time")
        defaultBenchmarkType = TestCaseType::CpuTimeBenchmark;
    else if(args.value("benchmark") == "cpu-cycles")
        defaultBenchmarkType = TestCaseType::CpuCyclesBenchmark;
    else Utility::Fatal() << "Unknown benchmark type" << args.value("benchmark");

    std::vector<std::pair<int, TestCase>> usedTestCases;

    /* Disable expected failures, if requested */
    _expectedFailuresDisabled = args.isSet("no-xfail");

    /* Skip test cases, if requested */
    if(args.isSet("skip-tests"))
        for(TestCase& testCase: _testCases)
            if(testCase.type == TestCaseType::Test) testCase.test = nullptr;

    /* Skip benchmarks, if requested */
    if(args.isSet("skip-benchmarks"))
        for(TestCase& testCase: _testCases)
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
            if(index - 1 >= _testCases.size()) continue;
            _testCases[index - 1].test = nullptr;
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
            if(index - 1 >= _testCases.size() || !_testCases[index - 1].test) continue;
            usedTestCases.emplace_back(index, _testCases[index - 1]);
        }

    /* Otherwise extract all (and skip skipped) */
    } else for(std::size_t i = 0; i != _testCases.size(); ++i) {
        if(!_testCases[i].test) continue;
        usedTestCases.emplace_back(i + 1, _testCases[i]);
    }

    const std::size_t repeatAllCount = args.value<std::size_t>("repeat-all");
    const std::size_t repeatEveryCount = args.value<std::size_t>("repeat-every");
    if(!repeatAllCount || !repeatEveryCount)
        Utility::Fatal() << "You have to repeat at least once";

    /* Repeat the test cases, if requested */
    const std::size_t originalTestCaseCount = usedTestCases.size();
    usedTestCases.reserve(usedTestCases.size()*repeatAllCount);
    for(std::size_t i = 0; i != repeatAllCount - 1; ++i)
        usedTestCases.insert(usedTestCases.end(), usedTestCases.begin(), usedTestCases.begin() + originalTestCaseCount);

    /* Shuffle the test cases, if requested */
    if(args.isSet("shuffle"))
        std::shuffle(usedTestCases.begin(), usedTestCases.end(), std::minstd_rand{std::random_device{}()});

    unsigned int errorCount = 0,
        noCheckCount = 0;

    /* Nothing to test */
    if(usedTestCases.empty()) {
        /* Not an error if we're skipping either tests or benchmarks (but not
           both) */
        if(args.isSet("skip-tests") && !args.isSet("skip-benchmarks")) {
            Debug(logOutput, _useColor)
                << Debug::boldColor(Debug::Color::Default) << "No remaining benchmarks to run in"
                << _testName << Debug::nospace << ".";
            return 0;
        }

        if(!args.isSet("skip-tests") && args.isSet("skip-benchmarks")) {
            Debug(logOutput, _useColor)
                << Debug::boldColor(Debug::Color::Default) << "No remaining tests to run in"
                << _testName << Debug::nospace << ".";
            return 0;
        }

        Error(errorOutput, _useColor) << Debug::boldColor(Debug::Color::Red) << "No test cases to run in" << _testName << Debug::nospace << "!";
        return 2;
    }

    Debug(logOutput, _useColor) << Debug::boldColor(Debug::Color::Default) << "Starting" << _testName << "with" << usedTestCases.size() << "test cases...";

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
            case TestCaseType::DefaultBenchmark:
                CORRADE_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */

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
                _benchmarkName = "";
                break;
        }

        _testCaseId = testCase.first;
        _testCaseInstanceId = testCase.second.instanceId;
        if(testCase.second.instanceId == ~std::size_t{})
            _testCaseDescription = {};
        else {
            #ifndef CORRADE_TARGET_ANDROID
            _testCaseDescription = std::to_string(testCase.second.instanceId);
            #else
            std::ostringstream out;
            out << testCase.second.instanceId;
            _testCaseDescription = out.str();
            #endif
        }

        /* Final combined repeat count */
        const std::size_t repeatCount = testCase.second.repeatCount*repeatEveryCount;

        /* Array with benchmark measurements */
        Containers::Array<std::uint64_t> measurements{testCase.second.type != TestCaseType::Test ? repeatCount : 0};

        bool aborted = false, skipped = false;
        for(std::size_t i = 0; i != repeatCount && !aborted; ++i) {
            if(testCase.second.setup)
                (this->*testCase.second.setup)();

            /* Print the repeat ID only if we are repeating */
            _testCaseRepeatId = repeatCount == 1 ? ~std::size_t{} : i;
            _testCaseLine = 0;
            _testCaseName.clear();
            _testCase = &testCase.second;
            _benchmarkBatchSize = 0;
            _benchmarkResult = 0;

            try {
                (this->*testCase.second.test)();
            } catch(Exception) {
                ++errorCount;
                aborted = true;
            } catch(SkipException) {
                aborted = true;
                skipped = true;
            }

            _testCase = nullptr;

            if(testCase.second.teardown)
                (this->*testCase.second.teardown)();

            if(testCase.second.benchmarkEnd)
                measurements[i] = _benchmarkResult;
        }

        /* Print success message if the test case wasn't failed/skipped */
        if(!aborted) {
            /* No testing/benchmark macros called */
            if(!_testCaseLine) {
                Debug out{logOutput, _useColor};
                printTestCaseLabel(out, "     ?", Debug::Color::Yellow, Debug::Color::Yellow);
                ++noCheckCount;

            /* Test case or benchmark with expected failure inside */
            } else if(testCase.second.type == TestCaseType::Test || _expectedFailure) {
                Debug out{logOutput, _useColor};
                printTestCaseLabel(out,
                    _expectedFailure ? " XFAIL" : "    OK",
                    _expectedFailure ? Debug::Color::Yellow : Debug::Color::Default,
                    Debug::Color::Default);
                if(_expectedFailure) out << Debug::newline << "       " << _expectedFailure->message();

            /* Benchmark. Completely custom printing. */
            } else {
                Debug out{logOutput, _useColor};

                const char* padding = PaddingString + sizeof(PaddingString) - digitCount(_testCases.size()) + digitCount(_testCaseId) - 1;

                out << Debug::boldColor(Debug::Color::Default) << " BENCH"
                    << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
                    << Debug::boldColor(Debug::Color::Cyan) << padding
                    << Debug::nospace << _testCaseId << Debug::nospace
                    << Debug::color(Debug::Color::Blue) << "]";

                /* Gather measurements. There needs to be at least one
                   measurememnt left even if the discard count says otherwise. */
                const std::size_t discardMeasurements = measurements.empty() ? 0 :
                        std::min(measurements.size() - 1, args.value<std::size_t>("benchmark-discard"));

                double mean, stddev;
                Utility::Debug::Color color;
                std::tie(mean, stddev, color) = Implementation::calculateStats(measurements.suffix(discardMeasurements), _benchmarkBatchSize, args.value<double>("benchmark-yellow"), args.value<double>("benchmark-red"));

                Implementation::printStats(out, mean, stddev, color, benchmarkUnits);

                out << Debug::boldColor(Debug::Color::Default)
                    << (_testCaseName.empty() ? "<unknown>" : _testCaseName)
                    << Debug::nospace;

                /* Optional test case description */
                if(!_testCaseDescription.empty()) {
                    out << "("
                        << Debug::nospace
                        << Debug::resetColor << _testCaseDescription
                        << Debug::nospace << Debug::boldColor(Debug::Color::Default)
                        << ")";
                } else out << "()";

                out << Debug::nospace << "@" << Debug::nospace
                    << measurements.size() - discardMeasurements
                    << Debug::nospace << "x" << Debug::nospace << _benchmarkBatchSize
                    << Debug::resetColor;
                if(!_benchmarkName.empty())
                    out << "(" << Utility::Debug::nospace << _benchmarkName
                        << Utility::Debug::nospace << ")";
            }

        /* Abort on first failure */
        } else if(args.isSet("abort-on-fail") && !skipped) {
            Debug out{logOutput, _useColor};
            out << Debug::boldColor(Debug::Color::Red) << "Aborted"
                << Debug::boldColor(Debug::Color::Default) << _testName
                << Debug::boldColor(Debug::Color::Red) << "after first failure"
                << Debug::boldColor(Debug::Color::Default) << "out of"
                << _checkCount << "checks so far.";
            if(noCheckCount)
                out << Debug::boldColor(Debug::Color::Yellow) << noCheckCount << "test cases didn't contain any checks!";

            return 1;
        }
    }

    Debug d(logOutput, _useColor);
    d << Debug::boldColor(Debug::Color::Default) << "Finished" << _testName << "with";
    if(errorCount) d << Debug::boldColor(Debug::Color::Red);
    d << errorCount << "errors";
    if(errorCount) d << Debug::boldColor(Debug::Color::Default);
    d << "out of" << _checkCount << "checks.";
    if(noCheckCount)
        d << Debug::boldColor(Debug::Color::Yellow) << noCheckCount << "test cases didn't contain any checks!";

    return errorCount != 0 || noCheckCount != 0;
}

void Tester::printTestCaseLabel(Debug& out, const char* const status, const Debug::Color statusColor, const Debug::Color labelColor) {
    const char* padding = PaddingString + sizeof(PaddingString) - digitCount(_testCases.size()) + digitCount(_testCaseId) - 1;

    out << Debug::boldColor(statusColor) << status
        << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
        << Debug::boldColor(Debug::Color::Cyan) << padding
        << Debug::nospace << _testCaseId << Debug::nospace
        << Debug::color(Debug::Color::Blue) << "]"
        << Debug::boldColor(labelColor)
        << (_testCaseName.empty() ? "<unknown>" : _testCaseName)
        << Debug::nospace;

    /* Optional test case description */
    if(!_testCaseDescription.empty()) {
        out << "("
            << Debug::nospace
            << Debug::resetColor << _testCaseDescription
            << Debug::nospace << Debug::boldColor(labelColor)
            << ")";
    } else out << "()";

    if(_testCaseRepeatId != ~std::size_t{})
        out << Debug::nospace << "@" << Debug::nospace << _testCaseRepeatId + 1;

    out << Debug::resetColor;
}

void Tester::verifyInternal(const std::string& expression, bool expressionValue) {
    ++_checkCount;

    /* If the expression is true or the failure is expected, done */
    if(!_expectedFailure) {
        if(expressionValue) return;
    } else if(!expressionValue) {
        Debug out{_logOutput, _useColor};
        printTestCaseLabel(out, " XFAIL", Debug::Color::Yellow, Debug::Color::Default);
        out << "at" << _testFilename << "on line" << _testCaseLine
            << Debug::newline << "       " << _expectedFailure->message()
            << "Expression" << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Error out{_errorOutput, _useColor};
    printTestCaseLabel(out, _expectedFailure ? " XPASS" : "  FAIL", Debug::Color::Red, Debug::Color::Default);
    out << "at" << _testFilename << "on line" << _testCaseLine
        << Debug::newline << "        Expression" << expression;
    if(!_expectedFailure) out << "failed.";
    else out << "was expected to fail.";
    throw Exception();
}

void Tester::registerTest(std::string filename, std::string name) {
    _testFilename = std::move(filename);
    _testName = std::move(name);
}

void Tester::skip(const std::string& message) {
    Debug out{_logOutput, _useColor};
    printTestCaseLabel(out, "  SKIP", Debug::Color::Default, Debug::Color::Default);
    out << Debug::newline << "       " << message;
    throw SkipException();
}

void Tester::setTestCaseName(const std::string& name) {
    _testCaseName = name;
}

void Tester::setTestCaseName(std::string&& name) {
    _testCaseName = std::move(name);
}

void Tester::setTestCaseDescription(const std::string& description) {
    _testCaseDescription = description;
}

void Tester::setTestCaseDescription(std::string&& description) {
    _testCaseDescription = std::move(description);
}

void Tester::setBenchmarkName(const std::string& name) {
    _benchmarkName = name;
}

void Tester::setBenchmarkName(std::string&& name) {
    _benchmarkName = std::move(name);
}

void Tester::registerTestCase(std::string&& name, int line) {
    CORRADE_ASSERT(_testCase,
        "TestSuite::Tester: using verification macros outside of test cases is not allowed", );

    if(_testCaseName.empty()) _testCaseName = std::move(name);
    _testCaseLine = line;
}

Tester::BenchmarkRunner Tester::createBenchmarkRunner(const std::size_t batchSize) {
    CORRADE_ASSERT(_testCase,
        "TestSuite::Tester: using benchmark macros outside of test cases is not allowed",
        (BenchmarkRunner{*this, nullptr, nullptr}));

    _benchmarkBatchSize = batchSize;
    return BenchmarkRunner{*this, _testCase->benchmarkBegin, _testCase->benchmarkEnd};
}

void Tester::wallTimeBenchmarkBegin() {
    _benchmarkName = "wall time";
    _benchmarkBegin = Implementation::wallTime();
}

std::uint64_t Tester::wallTimeBenchmarkEnd() {
    return Implementation::wallTime() - _benchmarkBegin;
}

void Tester::cpuTimeBenchmarkBegin() {
    _benchmarkName = "CPU time";
    _benchmarkBegin = Implementation::cpuTime();
}

std::uint64_t Tester::cpuTimeBenchmarkEnd() {
    return Implementation::cpuTime() - _benchmarkBegin;
}

void Tester::cpuCyclesBenchmarkBegin() {
    _benchmarkName = "CPU cycles";
    _benchmarkBegin = Implementation::rdtsc();
}

std::uint64_t Tester::cpuCyclesBenchmarkEnd() {
    return Implementation::rdtsc() - _benchmarkBegin;
}

Tester::TesterConfiguration::TesterConfiguration() = default;

Tester::ExpectedFailure::ExpectedFailure(Tester& instance, std::string message, const bool enabled): _instance(instance), _message(std::move(message)) {
    if(enabled && !instance._expectedFailuresDisabled) _instance._expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    _instance._expectedFailure = nullptr;
}

std::string Tester::ExpectedFailure::message() const { return _message; }

}}
