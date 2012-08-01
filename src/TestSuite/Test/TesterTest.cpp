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

#include "TesterTest.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace Corrade::Utility;

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)

namespace Corrade { namespace TestSuite { namespace Test {

TesterTest::TesterTest() {
    addTests(&TesterTest::test);
}

TesterTest::Test::Test() {
    addTests(&Test::noChecks,
             &Test::trueExpression,
             &Test::falseExpression,
             &Test::equal,
             &Test::nonEqual);
}

void TesterTest::Test::noChecks() {
    return;
}

void TesterTest::Test::trueExpression() {
    CORRADE_VERIFY(true);
}

void TesterTest::Test::falseExpression() {
    CORRADE_VERIFY(5 != 5);
}

void TesterTest::Test::equal() {
    CORRADE_COMPARE(3, 3);
}

void TesterTest::Test::nonEqual() {
    int a = 5;
    int b = 3;
    CORRADE_COMPARE(a, b);
}

void TesterTest::test() {
    stringstream out;
    Debug::setOutput(&out);
    Error::setOutput(&out);

    Test t;
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec();

    Debug::setOutput(&cout);
    Error::setOutput(&cerr);

    CORRADE_VERIFY(result == 1);

    string expected = "Starting TesterTest::Test with 5 test cases...\n"
        "    OK: trueExpression()\n"
        "  FAIL: falseExpression() at here.cpp on line 50 \n"
        "        Expression 5 != 5 failed.\n"
        "    OK: equal()\n"
        "  FAIL: nonEqual() at here.cpp on line 60 \n"
        "        Values a and b are not the same, actual: 5 vs. expected: 3\n"
        "Finished TesterTest::Test with 2 errors. 1 test cases didn't contain any checks!\n";

    CORRADE_COMPARE(out.str(), expected);
}

}}}
