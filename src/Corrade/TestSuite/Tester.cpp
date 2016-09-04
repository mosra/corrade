/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>
#include <utility>
#include <cstdlib>

#include "Corrade/Containers/Array.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/String.h"

#ifdef CORRADE_TARGET_UNIX
#include <unistd.h>
#endif

#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_UTILITY_USE_ANSI_COLORS)
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <io.h>
#endif

namespace Corrade { namespace TestSuite {

namespace {
    inline int digitCount(int number) {
        int digits = 0;
        while(number != 0) number /= 10, digits++;
        return digits;
    }

    constexpr const char PaddingString[] = "0000000000";

    inline std::string formatTime(std::chrono::nanoseconds ns, std::chrono::nanoseconds max, std::size_t batchSize) {
        std::ostringstream out;
        out << std::right << std::fixed << std::setprecision(2) << std::setw(6);

        if(max >= std::chrono::seconds{1})
            out << std::chrono::duration<float>(ns).count()/batchSize << "  s      ";
        else if(max >= std::chrono::milliseconds{1})
            out << std::chrono::duration<float, std::milli>(ns).count()/batchSize << " ms      ";
        else if(max >= std::chrono::microseconds{1})
            out << std::chrono::duration<float, std::micro>(ns).count()/batchSize << " µs      ";
        else out << std::chrono::duration<float, std::nano>(ns).count()/batchSize << " ns      ";

        return out.str();
    }

    inline std::string formatCount(std::uint64_t count, std::uint64_t max, std::size_t batchSize, const char* unit) {
        std::ostringstream out;
        out << std::right << std::fixed << std::setprecision(2) << std::setw(6);

        if(max >= 1000000000)
            out << float(count)/(1000000000.0f*batchSize) << " G" << unit;
        else if(max >= 1000000)
            out << float(count)/(1000000.0f*batchSize) << " M" << unit;
        else if(max >= 1000)
            out << float(count)/(1000000.0f*batchSize) << " k" << unit;
        else out << float(count)/batchSize << "  " << unit;

        return out.str();
    }

    std::string formatMeasurement(std::uint64_t count, std::uint64_t max, Tester::BenchmarkUnits unit, std::size_t batchSize) {
        switch(unit) {
            case Tester::BenchmarkUnits::Time:
                return formatTime(std::chrono::nanoseconds(count), std::chrono::nanoseconds(max), batchSize);
            case Tester::BenchmarkUnits::Cycles:
                return formatCount(count, max, batchSize, "cycles ");
            case Tester::BenchmarkUnits::Instructions:
                return formatCount(count, max, batchSize, "instrs ");
            case Tester::BenchmarkUnits::Memory:
                return formatCount(count, max, batchSize, "B      ");
            case Tester::BenchmarkUnits::Count:
                return formatCount(count, max, batchSize, "       ");
        }

        CORRADE_ASSERT(false, "TestSuite::Tester: invalid benchmark unit", {}); /* LCOV_EXCL_LINE */
    }
}

Tester::Tester(const TesterConfiguration& configuration): _logOutput{nullptr}, _errorOutput{nullptr}, _testCaseLine{0}, _checkCount{0}, _expectedFailure{nullptr}, _configuration{configuration} {}

int Tester::exec(const int argc, const char** const argv) { return exec(argc, argv, &std::cout, &std::cerr); }

int Tester::exec(const int argc, const char** const argv, std::ostream* const logOutput, std::ostream* const errorOutput) {
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
        .addBooleanOption("abort-on-fail").setHelp("abort after first failure")
            .setFromEnvironment("abort-on-fail", "CORRADE_TEST_ABORT_ON_FAIL")
        .addBooleanOption("no-xfail").setHelp("no-xfail", "disallow expected failures")
            .setFromEnvironment("no-xfail", "CORRADE_TEST_NO_XFAIL")
        .addOption("benchmark", "wall-clock").setHelp("benchmark", "default benchmark type", "TYPE")
        .setHelp(R"(Corrade TestSuite executable. By default runs test cases in order in which they
were added and exits with non-zero code if any of them failed. Supported
benchmark types:
  wall-clock    uses high-precision clock to measure time spent)")
        .parse(argc, argv);

    _logOutput = logOutput;
    _errorOutput = errorOutput;

    /* Decide about color */
    if(args.value("color") == "on" || args.value("color") == "ON")
        _useColor = Debug::Flags{};
    else if(args.value("color") == "off" || args.value("color") == "OFF")
        _useColor = Debug::Flag::DisableColors;
    /* The autodetection is done in Debug class on Windows with WINAPI colors */
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    else _useColor = Debug::Flags{};
    /* We can autodetect via isatty() on Unix-like systems and Windows with
       ANSI colors enabled */
    #elif !defined(CORRADE_TARGET_EMSCRIPTEN)
    else _useColor = logOutput == &std::cout &&
        /* Windows RT projects have C4996 treated as error by default. WHY */
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable: 4996)
        #endif
        errorOutput == &std::cerr && isatty(1) && isatty(2)
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif
        #ifdef CORRADE_TARGET_APPLE
        /* Xcode's console reports that it is a TTY, but it doesn't support
           colors. We have to check for the following undocumented environment
           variable instead. If set, then don't use colors. */
        && !std::getenv("XPC_SERVICE_NAME")
        #endif
        ? Debug::Flags{} : Debug::Flag::DisableColors;
    /* Otherwise can't be autodetected, thus disable by default */
    #else
    else _useColor = Debug::Flag::DisableColors;
    #endif

    /* Decide about default benchmark type */
    TestCaseType defaultBenchmarkType;
    if(args.value("benchmark") == "wall-clock")
        defaultBenchmarkType = TestCaseType::WallClockBenchmark;
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

            case TestCaseType::WallClockBenchmark:
                testCase.second.benchmarkBegin = &Tester::wallClockBenchmarkBegin;
                testCase.second.benchmarkEnd = &Tester::wallClockBenchmarkEnd;
                benchmarkUnits = BenchmarkUnits::Time;
                break;

            /* These have begin/end provided by the user */
            case TestCaseType::CustomTimeBenchmark:
            case TestCaseType::CustomCycleBenchmark:
            case TestCaseType::CustomInstructionBenchmark:
            case TestCaseType::CustomMemoryBenchmark:
            case TestCaseType::CustomCountBenchmark:
                benchmarkUnits = BenchmarkUnits(int(testCase.second.type));
                _benchmarkName = "Custom benchmark";
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

        bool aborted = false;
        for(std::size_t i = 0; i != repeatCount && !aborted; ++i) {
            if(testCase.second.setup)
                (this->*testCase.second.setup)();

            /* Print the repeat ID only if we are repeating */
            _testCaseRepeatId = repeatCount == 1 ? ~std::size_t{} : i;
            _testCaseLine = 0;
            _testCaseName.clear();
            _testCase = &testCase.second;
            _benchmarkResult = 0;

            try {
                (this->*testCase.second.test)();
            } catch(Exception) {
                ++errorCount;
                aborted = true;
            } catch(SkipException) {
                aborted = true;
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

            /* Benchmark */
            } else {
                Debug out{logOutput, _useColor};
                printTestCaseLabel(out, " BENCH", Debug::Color::Default, Debug::Color::Default);

                const std::uint64_t min = *std::min_element(measurements.begin(), measurements.end());
                const std::uint64_t max = *std::max_element(measurements.begin(), measurements.end());
                std::uint64_t avg{};
                for(std::uint64_t v: measurements) avg += v;
                avg /= measurements.size();

                out << Debug::newline << "       "
                    << Debug::boldColor(Debug::Color::Default)
                    << _benchmarkBatchSize << "iterations per repeat." << _benchmarkName << "per iteration:"
                    << Debug::newline << "        Min:"
                    << Debug::resetColor << formatMeasurement(min, max, benchmarkUnits, _benchmarkBatchSize)
                    << Debug::boldColor(Debug::Color::Default) << "Max:"
                    << Debug::resetColor << formatMeasurement(max, max, benchmarkUnits, _benchmarkBatchSize)
                    << Debug::boldColor(Debug::Color::Default) << "Avg:"
                    << Debug::resetColor << formatMeasurement(avg, max, benchmarkUnits, _benchmarkBatchSize);
            }

        /* Abort on first failure */
        } else if(args.isSet("abort-on-fail")) {
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

void Tester::wallClockBenchmarkBegin() {
    _benchmarkName = "Wall clock time";
    _wallClockBenchmarkBegin = std::chrono::high_resolution_clock::now();
}

std::uint64_t Tester::wallClockBenchmarkEnd() {
    return (std::chrono::high_resolution_clock::now() - _wallClockBenchmarkBegin).count();
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
