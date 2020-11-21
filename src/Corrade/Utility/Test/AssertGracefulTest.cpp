/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#define CORRADE_GRACEFUL_ASSERT

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Utility { namespace Test { namespace {

struct AssertGracefulTest: TestSuite::Tester {
    explicit AssertGracefulTest();

    void test();
    void constexprTest();
};

AssertGracefulTest::AssertGracefulTest() {
    addTests({&AssertGracefulTest::test,
              &AssertGracefulTest::constexprTest});
}

void AssertGracefulTest::test() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test graceful assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    int a = 5;
    [&](){ CORRADE_ASSERT(!a, "A should be zero", ); }();
    int b = [&](){ CORRADE_ASSERT(!a, "A should be zero!", 7); return 3; }();

    auto foo = [&](){ ++a; return false; };
    [&](){ CORRADE_ASSERT_OUTPUT(foo(), "foo() should succeed", ); }();
    int c = [&](){ CORRADE_ASSERT_OUTPUT(foo(), "foo() should succeed!", 7); return 3; }();

    [&](){ if(c != 3) CORRADE_ASSERT_UNREACHABLE("C should be 3", ); }();
    int d = [&](){ if(a) CORRADE_ASSERT_UNREACHABLE("C should be 3!", 7); return 3; }();

    /* CORRADE_INTERNAL_ASSERT(), CORRADE_INTERNAL_ASSERT_OUTPUT(),
       CORRADE_INTERNAL_ASSERT_EXPRESSION() and
       CORRADE_INTERNAL_ASSERT_UNREACHABLE() do not have a graceful version */

    CORRADE_COMPARE(a, 7);
    CORRADE_COMPARE(b, 7);
    CORRADE_COMPARE(c, 7);
    CORRADE_COMPARE(d, 7);
    CORRADE_COMPARE(out.str(),
        "A should be zero\n"
        "A should be zero!\n"
        "foo() should succeed\n"
        "foo() should succeed!\n"
        "C should be 3\n"
        "C should be 3!\n");
}

constexpr int divide(int a, int b) {
    return CORRADE_CONSTEXPR_ASSERT(b, "b can't be zero"), a/(b + 5);
}

void AssertGracefulTest::constexprTest() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test graceful assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    {
        int three = divide(15, 0);
        CORRADE_COMPARE(three, 3);
    }

    /* CORRADE_INTERNAL_CONSTEXPR_ASSERT() doesn't have a graceful version */

    CORRADE_COMPARE(out.str(),
        "b can't be zero\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::AssertGracefulTest)
