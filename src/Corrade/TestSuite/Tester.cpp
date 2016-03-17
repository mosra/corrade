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

#ifdef CORRADE_TARGET_WINDOWS
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
    else _useColor = logOutput == &std::cout && errorOutput == &std::cerr && isatty(1) && isatty(2) ?
        Debug::Flags{} : Debug::Flag::DisableColors;

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

    for(auto i: usedTestCases) {
        /* Reset output to stdout for each test case to prevent debug
            output segfaults */
        /** @todo Drop this when Debug::setOutput() is removed */
        Debug resetDebugRedirect{&std::cout};
        Error resetErrorRedirect{&std::cerr};
        Utility::Warning resetWarningRedirect{&std::cerr};

        _testCaseId = i.first;

        if(i.second.setup) {
            _testCaseName = "<setup>()";
            (this->*i.second.setup)();
        }

        try {
            _testCaseName.clear();
            (this->*i.second.test)();

            /* No testing macros called, don't print function name to output */
            if(_testCaseName.empty()) {
                Debug(logOutput, _useColor)
                    << Debug::boldColor(Debug::Color::Yellow) << "     ?"
                    << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
                    << Debug::boldColor(Debug::Color::Cyan) << padding(_testCaseId, _testCases.size())
                    << Debug::nospace << _testCaseId << Debug::nospace
                    << Debug::color(Debug::Color::Blue) << "]"
                    << Debug::boldColor(Debug::Color::Yellow) << "<unknown>()"
                    << Debug::resetColor;

                ++noCheckCount;
                continue;
            }

            Debug d(logOutput, _useColor);
            if(_expectedFailure) d << Debug::boldColor(Debug::Color::Yellow) << " XFAIL";
            else d << Debug::boldColor(Debug::Color::Default) << "    OK";
            d << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
                << Debug::boldColor(Debug::Color::Cyan) << padding(_testCaseId, _testCases.size())
                << Debug::nospace << _testCaseId << Debug::nospace
                << Debug::color(Debug::Color::Blue) << "]"
                << Debug::boldColor(Debug::Color::Default) << _testCaseName
                << Debug::resetColor;
            if(_expectedFailure) d << "\n       " << _expectedFailure->message();

        } catch(Exception) {
            ++errorCount;
        } catch(SkipException) {}

        if(i.second.teardown) {
            _testCaseName = "<teardown>()";
            (this->*i.second.teardown)();
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

void Tester::verifyInternal(const std::string& expression, bool expressionValue) {
    ++_checkCount;

    /* If the expression is true or the failure is expected, done */
    if(!_expectedFailure) {
        if(expressionValue) return;
    } else if(!expressionValue) {
        Debug{_logOutput, _useColor} << Debug::boldColor(Debug::Color::Yellow)
            << " XFAIL" << Debug::color(Debug::Color::Blue) << "["
            << Debug::nospace << Debug::boldColor(Debug::Color::Cyan)
            << padding(_testCaseId, _testCases.size()) << Debug::nospace
            << _testCaseId << Debug::nospace << Debug::color(Debug::Color::Blue)
            << "]" << Debug::boldColor(Debug::Color::Default) << _testCaseName
            << Debug::resetColor << "at" << _testFilename << "on line"
            << _testCaseLine << "\n       " << _expectedFailure->message()
            << "Expression" << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Error e{_errorOutput, _useColor};
    e << Debug::boldColor(Debug::Color::Red) << (_expectedFailure ? " XPASS" : "  FAIL")
        << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
        << Debug::boldColor(Debug::Color::Cyan) << padding(_testCaseId, _testCases.size())
        << Debug::nospace << _testCaseId << Debug::nospace
        << Debug::color(Debug::Color::Blue) << "]"
        << Debug::boldColor(Debug::Color::Default) << _testCaseName
        << Debug::resetColor << "at" << _testFilename << "on line"
        << _testCaseLine << "\n        Expression" << expression;
    if(!_expectedFailure) e << "failed.";
    else e << "was expected to fail.";
    throw Exception();
}

void Tester::registerTest(std::string filename, std::string name) {
    _testFilename = std::move(filename);
    _testName = std::move(name);
}

void Tester::skip(const std::string& message) {
    Debug e(_logOutput, _useColor);
    e << Debug::boldColor(Debug::Color::Default) << "  SKIP"
        << Debug::color(Debug::Color::Blue) << "[" << Debug::nospace
        << Debug::boldColor(Debug::Color::Cyan) << padding(_testCaseId, _testCases.size())
        << Debug::nospace << _testCaseId << Debug::nospace
        << Debug::color(Debug::Color::Blue) << "]"
        << Debug::boldColor(Debug::Color::Default) << _testCaseName
        << Debug::resetColor << "\n       " << message;
    throw SkipException();
}

void Tester::registerTestCase(const std::string& name, int line) {
    CORRADE_ASSERT(_testCaseName != "<setup>()" && _testCaseName != "<teardown>()",
        "TestSuite::Tester: using verification macros inside setup or teardown functions is not allowed", );

    if(_testCaseName.empty()) _testCaseName = name + "()";
    _testCaseLine = line;
}

const char* Tester::padding(const int number, const int max) {
    return PaddingString + sizeof(PaddingString) - digitCount(max) + digitCount(number) - 1;
}

Tester::TesterConfiguration::TesterConfiguration() = default;

Tester::ExpectedFailure::ExpectedFailure(Tester* const instance, std::string message, const bool enabled): _instance(instance), _message(std::move(message)) {
    if(enabled) _instance->_expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    _instance->_expectedFailure = nullptr;
}

std::string Tester::ExpectedFailure::message() const { return _message; }

}}
