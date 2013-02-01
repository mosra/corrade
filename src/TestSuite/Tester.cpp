/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Tester.h"

#include <iostream>

namespace Corrade { namespace TestSuite {

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
        } catch(Exception e) {
            ++errorCount;
            continue;
        }

        /* No testing macros called, don't print function name to output */
        if(testCaseName.empty()) {
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

#ifndef DOXYGEN_GENERATING_OUTPUT
void Tester::verify(const std::string& expression, bool expressionValue) {
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
#endif

void Tester::registerTestCase(const std::string& name, int line) {
    if(testCaseName.empty()) testCaseName = name + "()";
    testCaseLine = line;
    ++checkCount;
}

}}
