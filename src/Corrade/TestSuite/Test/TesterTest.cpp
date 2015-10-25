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

#include <cstdlib>
#include <sstream>

#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace TestSuite {

class StringLength;

template<> class Comparator<StringLength> {
    public:
        Comparator(int epsilon = 0): epsilon(epsilon) {}

        bool operator()(const std::string& actual, const std::string& expected) {
            return std::abs(int(actual.size()) - int(expected.size())) <= epsilon;
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Length of actual" << actual << "doesn't match length of expected" << expected << "with epsilon" << epsilon;
        }

    private:
        int epsilon;
};

class StringLength {
    public:
        StringLength(int epsilon = 0): c(epsilon) {}

        Comparator<StringLength> comparator() { return c; }

    private:
        Comparator<StringLength> c;
};

namespace Test {

struct Test: Tester {
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
    void compareImplicitConversionFail();

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
              &Test::compareImplicitConversionFail,

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

    {
        CORRADE_EXPECT_FAIL_IF("This is not our universe", 6*7 == 49);
        CORRADE_VERIFY(true); // #8
    }
}

void Test::unexpectedPassExpression() {
    CORRADE_EXPECT_FAIL("Not yet implemented.");
    CORRADE_VERIFY(true == true); // #9
}

void Test::unexpectedPassEqual() {
    CORRADE_EXPECT_FAIL("Cannot get it right.");
    CORRADE_COMPARE(2 + 2, 4); // #10
}

void Test::compareAs() {
    CORRADE_COMPARE_AS("kill!", "hello", StringLength); // #11
}

void Test::compareAsFail() {
    CORRADE_COMPARE_AS("meh", "hello", StringLength); // #12
}

void Test::compareWith() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10)); // #13
}

void Test::compareWithFail() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(9)); // #14
}

void Test::compareImplicitConversionFail() {
    std::string hello{"hello"};
    CORRADE_COMPARE("holla", hello); // #15
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
        void compareAsVarargs();
        void compareNonCopyable();
        void verifyExplicitBool();
};

class EmptyTest: public Tester {};

TesterTest::TesterTest() {
    addTests({&TesterTest::test,
              &TesterTest::emptyTest,

              &TesterTest::compareAsOverload,
              &TesterTest::compareAsVarargs,
              &TesterTest::compareNonCopyable,
              &TesterTest::verifyExplicitBool});
}

void TesterTest::test() {
    std::stringstream out;

    Test t;
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 1);

    std::string expected =
        "Starting TesterTest::Test with 14 test cases...\n"
        "     ?: <unknown>()\n"
        "    OK: trueExpression()\n"
        "  FAIL: falseExpression() at here.cpp on line 112 \n"
        "        Expression 5 != 5 failed.\n"
        "    OK: equal()\n"
        "  FAIL: nonEqual() at here.cpp on line 122 \n"
        "        Values a and b are not the same, actual is\n"
        "        5 \n"
        "        but expected\n"
        "        3\n"
        " XFAIL: expectFail() at here.cpp on line 128 \n"
        "        The world is not mad yet. 2 + 2 and 5 are not equal.\n"
        " XFAIL: expectFail() at here.cpp on line 129 \n"
        "        The world is not mad yet. Expression false == true failed.\n"
        "    OK: expectFail()\n"
        " XPASS: unexpectedPassExpression() at here.cpp on line 142 \n"
        "        Expression true == true was expected to fail.\n"
        " XPASS: unexpectedPassEqual() at here.cpp on line 147 \n"
        "        2 + 2 and 4 are not expected to be equal.\n"
        "    OK: compareAs()\n"
        "  FAIL: compareAsFail() at here.cpp on line 155 \n"
        "        Length of actual \"meh\" doesn't match length of expected \"hello\" with epsilon 0\n"
        "    OK: compareWith()\n"
        "  FAIL: compareWithFail() at here.cpp on line 163 \n"
        "        Length of actual \"You rather GTFO\" doesn't match length of expected \"hello\" with epsilon 9\n"
        "  FAIL: compareImplicitConversionFail() at here.cpp on line 168 \n"
        "        Values \"holla\" and hello are not the same, actual is\n"
        "        holla \n"
        "        but expected\n"
        "        hello\n"
        "  SKIP: skip() \n"
        "        This testcase is skipped.\n"
        "Finished TesterTest::Test with 7 errors out of 15 checks. 1 test cases didn't contain any checks!\n";

    //CORRADE_COMPARE(out.str().length(), expected.length());
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

void TesterTest::compareAsVarargs() {
    const std::pair<int, int> a(3, 5);
    const std::pair<float, float> b(3.2f, 5.7f);
    CORRADE_COMPARE_AS(a, b, std::pair<int, int>);
}

namespace {
    struct NonCopyable {
        explicit NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = delete;
    };

    inline bool operator==(const NonCopyable&, const NonCopyable&) { return true; }
    inline Utility::Debug& operator<<(Utility::Debug& debug, const NonCopyable&) {
        return debug << "NonCopyable";
    }
}

void TesterTest::compareNonCopyable() {
    /* Just to verify that there is no need to copy anything anywhere */
    NonCopyable a, b;
    CORRADE_COMPARE(a, b);
}

void TesterTest::verifyExplicitBool() {
    struct ExplicitTrue { explicit operator bool() const { return true; } };
    ExplicitTrue t;
    CORRADE_VERIFY(t);
    CORRADE_VERIFY(ExplicitTrue());

    struct ExplicitTrueNonConst { explicit operator bool() { return true; } };
    ExplicitTrueNonConst tc;
    CORRADE_VERIFY(tc);
    CORRADE_VERIFY(ExplicitTrueNonConst());

    struct ExplicitFalse { explicit operator bool() const { return false; } };
    ExplicitFalse f;
    CORRADE_VERIFY(!f);
}

}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
