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

#include <cstdlib>
#include <sstream>

#include "TestSuite/Tester.h"

namespace Corrade { namespace TestSuite {

class StringLength;

template<> class Comparator<StringLength> {
    public:
        inline Comparator(int epsilon = 0): epsilon(epsilon) {}

        inline bool operator()(const std::string& actual, const std::string& expected) {
            return std::abs(int(actual.size()) - int(expected.size())) <= epsilon;
        }

        inline void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Length of actual" << actual << "doesn't match length of expected" << expected << "with epsilon" << epsilon;
        }

    private:
        int epsilon;
};

class StringLength {
    public:
        StringLength(int epsilon = 0): c(epsilon) {}

        inline Comparator<StringLength> comparator() { return c; }

    private:
        Comparator<StringLength> c;
};

namespace Test {

class Test: public Tester {
    public:
        Test();

        void noChecks();
        void trueExpression();
        void falseExpression();
        void equal();
        void nonEqual();
        void expectFail();
        void unexpectedPassExpression();
        void unexpectedPassEqual();

        void compareAs();
        void compareAsFail();
        void compareWith();
        void compareWithFail();

        void skip();
};

Test::Test() {
    addTests({&Test::noChecks,
              &Test::trueExpression,
              &Test::falseExpression,
              &Test::equal,
              &Test::nonEqual,
              &Test::expectFail,
              &Test::unexpectedPassExpression,
              &Test::unexpectedPassEqual,

              &Test::compareAs,
              &Test::compareAsFail,
              &Test::compareWith,
              &Test::compareWithFail,

              &Test::skip});
}

void Test::noChecks() {
    return;
}

void Test::trueExpression() {
    CORRADE_VERIFY(true); // #1
}

void Test::falseExpression() {
    CORRADE_VERIFY(5 != 5); // #2
}

void Test::equal() {
    CORRADE_COMPARE(3, 3); // #3
}

void Test::nonEqual() {
    int a = 5;
    int b = 3;
    CORRADE_COMPARE(a, b); // #4
}

void Test::expectFail() {
    {
        CORRADE_EXPECT_FAIL("The world is not mad yet.");
        CORRADE_COMPARE(2 + 2, 5); // #5
        CORRADE_VERIFY(false == true); // #6
    }

    CORRADE_VERIFY(true); // #7
}

void Test::unexpectedPassExpression() {
    CORRADE_EXPECT_FAIL("Not yet implemented.");
    CORRADE_VERIFY(true == true); // #8
}

void Test::unexpectedPassEqual() {
    CORRADE_EXPECT_FAIL("Cannot get it right.");
    CORRADE_COMPARE(2 + 2, 4); // #9
}

void Test::compareAs() {
    CORRADE_COMPARE_AS("kill!", "hello", StringLength); // #10
}

void Test::compareAsFail() {
    CORRADE_COMPARE_AS("meh", "hello", StringLength); // #11
}

void Test::compareWith() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10)); // #12
}

void Test::compareWithFail() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(9)); // #13
}

void Test::skip() {
    CORRADE_SKIP("This testcase is skipped.");
    CORRADE_VERIFY(false); // (not called)
}

class TesterTest: public Tester {
    public:
        TesterTest();

        void test();
        void emptyTest();

        void compareAsOverload();
};

class EmptyTest: public Tester {};

TesterTest::TesterTest() {
    addTests({&TesterTest::test,
              &TesterTest::emptyTest,
              &TesterTest::compareAsOverload});
}

void TesterTest::test() {
    std::stringstream out;

    Test t;
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 1);

    std::string expected =
        "Starting TesterTest::Test with 13 test cases...\n"
        "    OK: trueExpression()\n"
        "  FAIL: falseExpression() at here.cpp on line 102 \n"
        "        Expression 5 != 5 failed.\n"
        "    OK: equal()\n"
        "  FAIL: nonEqual() at here.cpp on line 112 \n"
        "        Values a and b are not the same, actual is\n"
        "        5 \n"
        "        but expected\n"
        "        3\n"
        " XFAIL: expectFail() at here.cpp on line 118 \n"
        "        The world is not mad yet. 2 + 2 and 5 are not equal.\n"
        " XFAIL: expectFail() at here.cpp on line 119 \n"
        "        The world is not mad yet. Expression false == true failed.\n"
        "    OK: expectFail()\n"
        " XPASS: unexpectedPassExpression() at here.cpp on line 127 \n"
        "        Expression true == true was expected to fail.\n"
        " XPASS: unexpectedPassEqual() at here.cpp on line 132 \n"
        "        2 + 2 and 4 are not expected to be equal.\n"
        "    OK: compareAs()\n"
        "  FAIL: compareAsFail() at here.cpp on line 140 \n"
        "        Length of actual \"meh\" doesn't match length of expected \"hello\" with epsilon 0\n"
        "    OK: compareWith()\n"
        "  FAIL: compareWithFail() at here.cpp on line 148 \n"
        "        Length of actual \"You rather GTFO\" doesn't match length of expected \"hello\" with epsilon 9\n"
        "  SKIP: skip() at here.cpp on line 152 \n"
        "        This testcase is skipped.\n"
        "Finished TesterTest::Test with 6 errors out of 13 checks. 1 test cases didn't contain any checks!\n";

    CORRADE_COMPARE(out.str().length(), expected.length());
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::emptyTest() {
    std::stringstream out;

    EmptyTest t;
    t.registerTest("here.cpp", "TesterTest::EmptyTest");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 2);

    CORRADE_COMPARE(out.str(), "In TesterTest::EmptyTest weren't found any test cases!\n");
}

void TesterTest::compareAsOverload() {
    /* Just test that this compiles well */
    float a = 3.0f;
    double b = 3.0f;
    CORRADE_COMPARE_AS(a, b, float);
    CORRADE_COMPARE_AS(a, b, double);
}

}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
