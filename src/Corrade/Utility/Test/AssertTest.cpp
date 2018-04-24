/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include <sstream>

#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Utility { namespace Test {

struct AssertTest: TestSuite::Tester {
    explicit AssertTest();

    void test();

    void constexprTest();

    bool _failAssert, _failConstexprAssert, _failInternalAssert,
        _failInternalConstexprAssert, _failAssertOutput,
        _failInternalAssertOutput, _failAssertUnreachable;
};

AssertTest::AssertTest(): TestSuite::Tester{TesterConfiguration{}.setSkippedArgumentPrefixes({"fail-on"})} {
    addTests({&AssertTest::test,
              &AssertTest::constexprTest});

    Arguments args{"fail-on"};
    args.addOption("assert", "false").setHelp("assert", "fail on CORRADE_ASSERT()", "BOOL")
        .addOption("constexpr-assert", "false").setHelp("constexpr-assert", "fail on CORRADE_CONSTEXPR_ASSERT()", "BOOL")
        .addOption("internal-assert", "false").setHelp("internal-assert", "fail on CORRADE_INTERNAL_ASSERT()", "BOOL")
        .addOption("internal-constexpr-assert", "false").setHelp("internal-constexpr-assert", "fail on CORRADE_INTERNAL_CONSTEXPR_ASSERT()", "BOOL")
        .addOption("assert-output", "false").setHelp("assert-output", "fail on CORRADE_ASSERT_OUTPUT()", "BOOL")
        .addOption("internal-assert-output", "false").setHelp("internal-assert-output", "fail on CORRADE_INTERNAL_ASSERT_OUTPUT()", "BOOL")
        .addOption("assert-unreachable", "false").setHelp("assert-unreachable", "fail on CORRADE_ASSERT_UNREACHABLE()", "BOOL")
        .parse(arguments().first, arguments().second);

    _failAssert = args.value<bool>("assert");
    _failConstexprAssert = args.value<bool>("constexpr-assert");
    _failInternalAssert = args.value<bool>("internal-assert");
    _failInternalConstexprAssert = args.value<bool>("internal-constexpr-assert");
    _failAssertOutput = args.value<bool>("assert-output");
    _failInternalAssertOutput = args.value<bool>("internal-assert-output");
    _failAssertUnreachable = args.value<bool>("assert-unreachable");
}

void AssertTest::test() {
    std::ostringstream out;
    /* Redirect output only if no failures are expected */
    Error redirectError{_failAssert || _failInternalAssert || _failAssertOutput || _failInternalAssertOutput || _failAssertUnreachable ? Error::output() : &out};

    int a = 0;
    CORRADE_ASSERT(!a && !_failAssert, "A should be zero", );
    int b = [&](){ CORRADE_ASSERT(!a, "A should be zero!", 7); return 3; }();
    CORRADE_INTERNAL_ASSERT(b && !_failInternalAssert);

    auto foo = [&](){ ++a; return true; };
    CORRADE_ASSERT_OUTPUT(foo() && !_failAssertOutput, "foo() should succeed", );
    int c = [&](){ CORRADE_ASSERT_OUTPUT(foo(), "foo() should succeed!", 7); return 3; }();
    CORRADE_INTERNAL_ASSERT_OUTPUT(foo() && !_failInternalAssertOutput);

    if(!a || _failAssertUnreachable) CORRADE_ASSERT_UNREACHABLE();

    CORRADE_COMPARE(a, 3);
    CORRADE_COMPARE(b, 3);
    CORRADE_COMPARE(c, 3);
    CORRADE_COMPARE(out.str(), "");
}

namespace {

constexpr int divide(int a, int b) {
    return CORRADE_CONSTEXPR_ASSERT(b, "b can't be zero"), a/b;
}

constexpr int divideInternal(int a, int b) {
    return CORRADE_INTERNAL_CONSTEXPR_ASSERT(b), a/b;
}

}

void AssertTest::constexprTest() {
    std::ostringstream out;
    Error redirectError{_failConstexprAssert || _failInternalConstexprAssert ? Error::output() : &out};

    /* Change divisor to 0 for compile-time failure. */

    {
        constexpr int three = divide(15, 5);
        CORRADE_COMPARE(three, 3);
    } {
        constexpr int three = divideInternal(15, 5);
        CORRADE_COMPARE(three, 3);
    } {
        int three = divide(15, _failConstexprAssert ? 0 : 5);
        CORRADE_COMPARE(three, 3);
    } {
        int three = divideInternal(15, _failInternalConstexprAssert ? 0 : 5);
        CORRADE_COMPARE(three, 3);
    }

    CORRADE_COMPARE(out.str(), "");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::AssertTest)
