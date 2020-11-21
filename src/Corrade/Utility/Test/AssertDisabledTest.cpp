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

#ifndef CORRADE_NO_ASSERT
#define CORRADE_NO_ASSERT
#endif

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace Utility { namespace Test { namespace {

struct AssertDisabledTest: TestSuite::Tester {
    explicit AssertDisabledTest();

    void test();
    void constexprTest();
};

AssertDisabledTest::AssertDisabledTest() {
    addTests({&AssertDisabledTest::test,
              &AssertDisabledTest::constexprTest});

    #ifdef CORRADE_STANDARD_ASSERT
    setTestName("Corrade::Utility::Test::AssertStandardDisabledTest");
    #endif
}

void AssertDisabledTest::test() {
    #ifndef __clang_analyzer__
    std::ostringstream out;
    Error redirectError{&out};

    int a = 0;
    CORRADE_ASSERT(a, "A should be zero", );
    int b = [&](){ CORRADE_ASSERT(a, "A should be zero!", 7); return 3; }();
    CORRADE_INTERNAL_ASSERT(b);

    auto foo = [&](){ ++a; return false; };
    CORRADE_ASSERT_OUTPUT(foo(), "foo() should succeed", );
    int c = [&](){ CORRADE_ASSERT_OUTPUT(foo(), "foo() should succeed!", 7); return 3; }();
    CORRADE_INTERNAL_ASSERT_OUTPUT(foo());

    /* These *still* compile to __builtin_unreachable, so we shouldn't trigger
       them */
    [&](){ if(c != 3) CORRADE_ASSERT_UNREACHABLE("c should be 3", ); }();
    int d = [&](){ if(c != 3) CORRADE_ASSERT_UNREACHABLE("c should be 3!", 7); return 3; }();
    if(c != 3) CORRADE_INTERNAL_ASSERT_UNREACHABLE();

    int e = CORRADE_INTERNAL_ASSERT_EXPRESSION(2 + 4)/2;

    CORRADE_COMPARE(a, 3);
    CORRADE_COMPARE(b, 3);
    CORRADE_COMPARE(c, 3);
    CORRADE_COMPARE(d, 3);
    CORRADE_COMPARE(e, 3);
    CORRADE_COMPARE(out.str(), "");
    #else
    CORRADE_SKIP("With assertions disabled, CORRADE_VERIFY() and CORRADE_COMPARE() cause a lot of false positives in Address Sanitizer.");
    #endif
}

constexpr int divide(int a, int b) {
    return CORRADE_CONSTEXPR_ASSERT(b, "b can't be zero"), a/(b + 5);
}

constexpr int divideInternal(int a, int b) {
    return CORRADE_INTERNAL_CONSTEXPR_ASSERT(b), a/(b + 5);
}

void AssertDisabledTest::constexprTest() {
    #ifndef __clang_analyzer__
    std::ostringstream out;
    Error redirectError{&out};

    {
        constexpr int three = divide(15, 0);
        CORRADE_COMPARE(three, 3);
    } {
        constexpr int three = divideInternal(15, 0);
        CORRADE_COMPARE(three, 3);
    } {
        int three = divide(15, 0);
        CORRADE_COMPARE(three, 3);
    } {
        int three = divideInternal(15, 0);
        CORRADE_COMPARE(three, 3);
    }

    CORRADE_COMPARE(out.str(), "");
    #else
    CORRADE_SKIP("With assertions disabled, CORRADE_VERIFY() and CORRADE_COMPARE() cause a lot of false positives in Address Sanitizer.");
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::AssertDisabledTest)
