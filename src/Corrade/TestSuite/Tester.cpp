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
#include <random>
#include <utility>

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
        .addOption("only").setHelp("only", "run only test cases with given numbers (in that order)", "\"N1 N2...\"")
        .addBooleanOption("shuffle").setHelp("shuffle", "randomly shuffle test case order")
            .setFromEnvironment("shuffle", "CORRADE_TEST_SHUFFLE")
        .addOption("repeat-every", "1").setHelp("repeat-every", "repeat every test case N times", "N")
            .setFromEnvironment("repeat-every", "CORRADE_TEST_REPEAT_EVERY")
        .addOption("repeat-all", "1").setHelp("repeat-all", "repeat all test cases N times", "N")
            .setFromEnvironment("repeat-all", "CORRADE_TEST_REPEAT_ALL")
        .setHelp("Corrade TestSuite executable. By default runs test cases in order in which they\n"
                 "were added and exits with non-zero code if any of them failed.")
        .parse(argc, argv);

    _logOutput = logOutput;
    _errorOutput = errorOutput;

    /* Decide about color */
    if(args.value("color") == "on" || args.value("color") == "ON")
        _useColor = Debug::Flags{};
    else if(args.value("color") == "off" || args.value("color") == "OFF")
        _useColor = Debug::Flag::DisableColors;
    #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    else _useColor = logOutput == &std::cout && errorOutput == &std::cerr && isatty(1) && isatty(2) ?
        Debug::Flags{} : Debug::Flag::DisableColors;
    #endif

    std::vector<std::pair<int, TestCase>> usedTestCases;

    /* Remove skipped test cases */
    if(!args.value("skip").empty()) {
        const std::vector<std::string> skip = Utility::String::split(args.value("skip"), ' ');
        for(auto&& n: skip) {
            std::size_t index = std::stoi(n);
            if(index - 1 >= _testCases.size()) continue;
            _testCases[index - 1].test = nullptr;
        }
    }

    /* Extract only whitelisted test cases if requested (and skip skipped) */
    if(!args.value("only").empty()) {
        const std::vector<std::string> only = Utility::String::split(args.value("only"), ' ');
        for(auto&& n: only) {
            std::size_t index = std::stoi(n);
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

    /* Fail when we have nothing to test */
    if(usedTestCases.empty()) {
        Error(errorOutput, _useColor) << Debug::boldColor(Debug::Color::Red) << "No tests to run in" << _testName << Debug::nospace << "!";
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

        _testCaseId = testCase.first;
        _testCaseInstanceId = testCase.second.instanceId;
        _testCaseDescription = testCase.second.instanceId == ~std::size_t{} ? std::string{} : std::to_string(testCase.second.instanceId);

        /* Final combined repeat count */
        const std::size_t repeatCount = testCase.second.repeatCount*repeatEveryCount;

        bool aborted = false;
        for(std::size_t i = 0; i != repeatCount && !aborted; ++i) {
            if(testCase.second.setup)
                (this->*testCase.second.setup)();

            /* Print the repeat ID only if we are repeating */
            _testCaseRepeatId = repeatCount == 1 ? 0 : i + 1;
            _testCaseLine = 0;
            _testCaseName.clear();
            _testCaseRunning = true;

            try {
                (this->*testCase.second.test)();
            } catch(Exception) {
                ++errorCount;
                aborted = true;
            } catch(SkipException) {
                aborted = true;
            }

            _testCaseRunning = false;

            if(testCase.second.teardown)
                (this->*testCase.second.teardown)();
        }

        /* Print success message if the test case wasn't failed/skipped */
        if(!aborted) {
            /* No testing macros called */
            if(!_testCaseLine) {
                Debug out{logOutput, _useColor};
                printTestCaseLabel(out, "     ?", Debug::Color::Yellow, Debug::Color::Yellow);
                ++noCheckCount;

            /* Common path */
            } else {
                Debug out{logOutput, _useColor};
                printTestCaseLabel(out,
                    _expectedFailure ? " XFAIL" : "    OK",
                    _expectedFailure ? Debug::Color::Yellow : Debug::Color::Default,
                    Debug::Color::Default);
                if(_expectedFailure) out << Debug::newline << "       " << _expectedFailure->message();
            }
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

    if(_testCaseRepeatId)
        out << Debug::nospace << "@" << Debug::nospace << _testCaseRepeatId;

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

void Tester::registerTestCase(std::string&& name, int line) {
    CORRADE_ASSERT(_testCaseRunning,
        "TestSuite::Tester: using verification macros outside of test cases is not allowed", );

    if(_testCaseName.empty()) _testCaseName = std::move(name);
    _testCaseLine = line;
}

Tester::TesterConfiguration::TesterConfiguration() = default;

Tester::ExpectedFailure::ExpectedFailure(Tester& instance, std::string message, const bool enabled): _instance(instance), _message(std::move(message)) {
    if(enabled) _instance._expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    _instance._expectedFailure = nullptr;
}

std::string Tester::ExpectedFailure::message() const { return _message; }

}}
