/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include <iostream>
#include <utility>

#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/String.h"

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
    args.addOption("skip").setHelp("skip", "skip test cases with given numbers", "\"N1 N2...\"")
        .addOption("only").setHelp("only", "run only test cases with given numbers (in that order)", "\"N1 N2...\"")
        .setHelp("Corrade TestSuite executable. By default runs test cases in order in which they\n"
                 "were added and exits with non-zero code if any of them failed.")
        .parse(argc, argv);

    _logOutput = logOutput;
    _errorOutput = errorOutput;

    std::vector<std::pair<int, TestCase>> usedTestCases;

    /* Remove skipped test cases */
    if(!args.value("skip").empty()) {
        const std::vector<std::string> skip = Utility::String::split(args.value("skip"), ' ');
        for(auto&& n: skip) {
            std::size_t index = std::stoi(n);
            if(index - 1 >= _testCases.size()) continue;
            _testCases[index - 1] = nullptr;
        }
    }

    /* Extract only whitelisted test cases if requested (and skip skipped) */
    if(!args.value("only").empty()) {
        const std::vector<std::string> only = Utility::String::split(args.value("only"), ' ');
        for(auto&& n: only) {
            std::size_t index = std::stoi(n);
            if(index - 1 >= _testCases.size() || !_testCases[index - 1]) continue;
            usedTestCases.emplace_back(index, _testCases[index - 1]);
        }

    /* Otherwise extract all (and skip skipped) */
    } else for(std::size_t i = 0; i != _testCases.size(); ++i) {
        if(!_testCases[i]) continue;
        usedTestCases.emplace_back(i + 1, _testCases[i]);
    }

    unsigned int errorCount = 0,
        noCheckCount = 0;

    /* Fail when we have nothing to test */
    if(usedTestCases.empty()) {
        Utility::Error(errorOutput) << "No tests to run in" << _testName << Debug::nospace << "!";
        return 2;
    }

    Utility::Debug(logOutput) << "Starting" << _testName << "with" << usedTestCases.size() << "test cases...";

    for(auto i: usedTestCases) {
        try {
            /* Reset output to stdout for each test case to prevent debug
               output segfaults */
            /** @todo Drop this when Debug::setOutput() is removed */
            Utility::Debug resetDebugRedirect{&std::cout};
            Utility::Error resetErrorRedirect{&std::cerr};
            Utility::Warning resetWarningRedirect{&std::cerr};

            _testCaseId = i.first;
            _testCaseName.clear();
            (this->*i.second)();
        } catch(Exception) {
            ++errorCount;
            continue;
        } catch(SkipException) {
            continue;
        }

        /* No testing macros called, don't print function name to output */
        if(_testCaseName.empty()) {
            Utility::Debug(logOutput) << "     ? ["
                << Debug::nospace << padding(_testCaseId, _testCases.size())
                << Debug::nospace << _testCaseId << Debug::nospace << "] <unknown>()";

            ++noCheckCount;
            continue;
        }

        Utility::Debug d(logOutput);
        d << (_expectedFailure ? " XFAIL [" : "    OK [")
            << Debug::nospace << padding(_testCaseId, _testCases.size())
            << Debug::nospace << _testCaseId << Debug::nospace << "]" << _testCaseName;
        if(_expectedFailure) d << "\n       " << _expectedFailure->message();
    }

    Utility::Debug d(logOutput);
    d << "Finished" << _testName << "with" << errorCount << "errors out of" << _checkCount << "checks.";
    if(noCheckCount)
        d << noCheckCount << "test cases didn't contain any checks!";

    return errorCount != 0 || noCheckCount != 0;
}

void Tester::verifyInternal(const std::string& expression, bool expressionValue) {
    ++_checkCount;

    /* If the expression is true or the failure is expected, done */
    if(!_expectedFailure) {
        if(expressionValue) return;
    } else if(!expressionValue) {
        Utility::Debug{_logOutput} << " XFAIL ["
            << Debug::nospace << padding(_testCaseId, _testCases.size())
            << Debug::nospace << _testCaseId << Debug::nospace << "]" << _testCaseName
            << "at" << _testFilename << "on line" << _testCaseLine << "\n       " << _expectedFailure->message() << "Expression" << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Utility::Error e(_errorOutput);
    e << (_expectedFailure ? " XPASS [" : "  FAIL [")
        << Debug::nospace << padding(_testCaseId, _testCases.size())
        << Debug::nospace << _testCaseId << Debug::nospace << "]" << _testCaseName
        << "at" << _testFilename << "on line" << _testCaseLine << "\n        Expression" << expression;
    if(!_expectedFailure) e << "failed.";
    else e << "was expected to fail.";
    throw Exception();
}

void Tester::registerTest(std::string filename, std::string name) {
    _testFilename = std::move(filename);
    _testName = std::move(name);
}

void Tester::skip(const std::string& message) {
    Utility::Debug e(_logOutput);
    e << "  SKIP ["
        << Debug::nospace << padding(_testCaseId, _testCases.size())
        << Debug::nospace << _testCaseId << Debug::nospace << "]" << _testCaseName << "\n       " << message;
    throw SkipException();
}

void Tester::registerTestCase(const std::string& name, int line) {
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
