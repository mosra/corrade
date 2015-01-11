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

namespace Corrade { namespace TestSuite {

Tester::Tester(): logOutput(nullptr), errorOutput(nullptr), testCaseLine(0), checkCount(0), expectedFailure(nullptr) {}

int Tester::exec() { return exec(&std::cout, &std::cerr); }

int Tester::exec(std::ostream* logOutput, std::ostream* errorOutput) {
    this->logOutput = logOutput;
    this->errorOutput = errorOutput;

    /* Fail when we have nothing to test */
    if(testCases.empty()) {
        Utility::Error(errorOutput) << "In" << testName << "weren't found any test cases!";
        return 2;
    }

    Utility::Debug(logOutput) << "Starting" << testName << "with" << testCases.size() << "test cases...";

    unsigned int errorCount = 0,
        noCheckCount = 0;

    for(auto i: testCases) {
        /* Reset output to stdout for each test case to prevent debug
            output segfaults */
        /** @todo Drop this when Debug has proper output scoping */
        Utility::Debug::setOutput(&std::cout);
        Utility::Error::setOutput(&std::cerr);
        Utility::Warning::setOutput(&std::cerr);

        try {
            testCaseName.clear();
            (this->*i)();
        } catch(Exception) {
            ++errorCount;
            continue;
        } catch(SkipException) {
            continue;
        }

        /* No testing macros called, don't print function name to output */
        if(testCaseName.empty()) {
            Utility::Debug(logOutput) << "     ?: <unknown>()";

            ++noCheckCount;
            continue;
        }

        Utility::Debug d(logOutput);
        d << (expectedFailure ? " XFAIL:" : "    OK:") << testCaseName;
        if(expectedFailure) d << "\n       " << expectedFailure->message();
    }

    Utility::Debug d(logOutput);
    d << "Finished" << testName << "with" << errorCount << "errors out of" << checkCount << "checks.";
    if(noCheckCount)
        d << noCheckCount << "test cases didn't contain any checks!";

    return errorCount != 0 || noCheckCount != 0;
}

void Tester::verifyInternal(const std::string& expression, bool expressionValue) {
    ++checkCount;

    /* If the expression is true or the failure is expected, done */
    if(!expectedFailure) {
        if(expressionValue) return;
    } else if(!expressionValue) {
        Utility::Debug(logOutput) << " XFAIL:" << testCaseName << "at" << testFilename << "on line" << testCaseLine << "\n       " << expectedFailure->message() << "Expression" << expression << "failed.";
        return;
    }

    /* Otherwise print message to error output and throw exception */
    Utility::Error e(errorOutput);
    e << (expectedFailure ? " XPASS:" : "  FAIL:") << testCaseName << "at" << testFilename << "on line" << testCaseLine << "\n        Expression" << expression;
    if(!expectedFailure) e << "failed.";
    else e << "was expected to fail.";
    throw Exception();
}

void Tester::registerTest(std::string filename, std::string name) {
    testFilename = std::move(filename);
    testName = std::move(name);
}

void Tester::skip(const std::string& message) {
    Utility::Debug e(logOutput);
    e << "  SKIP:" << testCaseName << "\n       " << message;
    throw SkipException();
}

void Tester::registerTestCase(const std::string& name, int line) {
    if(testCaseName.empty()) testCaseName = name + "()";
    testCaseLine = line;
}

Tester::ExpectedFailure::ExpectedFailure(Tester* instance, std::string message): instance(instance), _message(std::move(message)) {
    instance->expectedFailure = this;
}

Tester::ExpectedFailure::~ExpectedFailure() {
    instance->expectedFailure = nullptr;
}

std::string Tester::ExpectedFailure::message() const { return _message; }

}}
