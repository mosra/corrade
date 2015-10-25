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

namespace Corrade { namespace TestSuite {

Tester::Tester(): _logOutput{nullptr}, _errorOutput{nullptr}, _testCaseLine{0}, _checkCount{0}, _expectedFailure{nullptr} {}

int Tester::exec() { return exec(&std::cout, &std::cerr); }

int Tester::exec(std::ostream* logOutput, std::ostream* errorOutput) {
    _logOutput = logOutput;
    _errorOutput = errorOutput;

    /* Fail when we have nothing to test */
    if(_testCases.empty()) {
        Utility::Error(errorOutput) << "In" << _testName << "weren't found any test cases!";
        return 2;
    }

    Utility::Debug(logOutput) << "Starting" << _testName << "with" << _testCases.size() << "test cases...";

    unsigned int errorCount = 0,
        noCheckCount = 0;

    for(auto i: _testCases) {
        /* Reset output to stdout for each test case to prevent debug
            output segfaults */
        /** @todo Drop this when Debug has proper output scoping */
        Utility::Debug::setOutput(&std::cout);
        Utility::Error::setOutput(&std::cerr);
        Utility::Warning::setOutput(&std::cerr);

        try {
            _testCaseName.clear();
            (this->*i)();
        } catch(Exception) {
            ++errorCount;
            continue;
        } catch(SkipException) {
            continue;
        }

        /* No testing macros called, don't print function name to output */
        if(_testCaseName.empty()) {
            Utility::Debug(logOutput) << "     ?: <unknown>()";

            ++noCheckCount;
            continue;
        }

        Utility::Debug d(logOutput);
        d << (_expectedFailure ? " XFAIL:" : "    OK:") << _testCaseName;
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
        Utility::Debug{_logOutput} << " XFAIL:" << _testCaseName << "at" << _testFilename << "on line" << _testCaseLine << "\n       " << _expectedFailure->message() << "Expression" << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Utility::Error e(_errorOutput);
    e << (_expectedFailure ? " XPASS:" : "  FAIL:") << _testCaseName << "at" << _testFilename << "on line" << _testCaseLine << "\n        Expression" << expression;
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
    e << "  SKIP:" << _testCaseName << "\n       " << message;
    throw SkipException();
}

void Tester::registerTestCase(const std::string& name, int line) {
    if(_testCaseName.empty()) _testCaseName = name + "()";
    _testCaseLine = line;
}

Tester::ExpectedFailure::ExpectedFailure(Tester* const instance, std::string message, const bool enabled): _instance(instance), _message(std::move(message)) {
    if(enabled) _instance->_expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    _instance->_expectedFailure = nullptr;
}

std::string Tester::ExpectedFailure::message() const { return _message; }

}}
