/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/Macros.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct MacrosCpp17Test: TestSuite::Tester {
    explicit MacrosCpp17Test();

    void nodiscard();
    void constexpr20();
    void fallthrough();
};

MacrosCpp17Test::MacrosCpp17Test() {
    addTests({&MacrosCpp17Test::nodiscard,
              &MacrosCpp17Test::constexpr20,
              &MacrosCpp17Test::fallthrough});
}

CORRADE_NODISCARD("this message will not be printed until C++20") int nodiscardReturn(int a) { return a + 1; }

void MacrosCpp17Test::nodiscard() {
    /* CORRADE_NODISCARD has different implementation with C++17 (and then with
       C++20), other than that the test case is equivalent to
       MacrosTest::nodiscard() */

    int a = 2;
    #if 1 /* Set to 0 to produce a warning */
    a +=
    #endif
    nodiscardReturn(3);

    CORRADE_COMPARE_AS(a, 2, TestSuite::Compare::GreaterOrEqual);
}

struct ConstexprNoInit {
    CORRADE_CONSTEXPR20 explicit ConstexprNoInit(NoInitT) {}

    int a;
};

CORRADE_CONSTEXPR20 ConstexprNoInit constexprNoInit(int a) {
    ConstexprNoInit s{NoInit};
    s.a = a;
    return s;
}

void MacrosCpp17Test::constexpr20() {
    /* Compared to MacrosCpp20Test::constexpr20(), here the macro should
       compile to nothing, making it behave like a regular function */
    CORRADE_CONSTEXPR20 ConstexprNoInit a = constexprNoInit(42);
    CORRADE_COMPARE(a.a, 42);
}

void MacrosCpp17Test::fallthrough() {
    /* The CORRADE_FALLTHROUGH macro is non-empty on MSVC only since C++17 */

    int a = 2;
    int d[5]{};
    int e[5]{5, 4, 3, 2, 1};
    int *b = d, *c = e;
    switch(a) {
        case 2:
            *b++ = *c++;
            CORRADE_FALLTHROUGH
        case 1:
            *b++ = *c++;
    };

    CORRADE_COMPARE(d[0], 5);
    CORRADE_COMPARE(d[1], 4);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MacrosCpp17Test)
