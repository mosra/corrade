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

struct MacrosCpp20Test: TestSuite::Tester {
    explicit MacrosCpp20Test();

    void constexpr20();
    void likelyUnlikely();
};

MacrosCpp20Test::MacrosCpp20Test() {
    addTests({&MacrosCpp20Test::constexpr20,
              &MacrosCpp20Test::likelyUnlikely});
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

void MacrosCpp20Test::constexpr20() {
    CORRADE_INFO("CORRADE_CXX_STANDARD is" << CORRADE_CXX_STANDARD << Debug::nospace << ", __cpp_constexpr is" << __cpp_constexpr);

    /* This should pass always */
    CORRADE_CONSTEXPR20 ConstexprNoInit a = constexprNoInit(42);
    CORRADE_COMPARE(a.a, 42);

    /* Using 201907 instead of 202002 as GCC, Clang and MSVC all report it
       strangely, see the macro docs for details. */
    #if defined(CORRADE_TARGET_CLANG) && __clang_major__ < 10
    CORRADE_COMPARE_AS(__cpp_constexpr, 201907,
        TestSuite::Compare::Less);
    CORRADE_SKIP("Clang" << __clang_major__ << "doesn't support all C++20 constexpr features yet");
    #elif !defined(CORRADE_TARGET_CLANG) && defined(CORRADE_TARGET_GCC) && __GNUC__ < 10
    CORRADE_COMPARE_AS(__cpp_constexpr, 201907,
        TestSuite::Compare::Less);
    CORRADE_SKIP("GCC" << __GNUC__ << "doesn't support all C++20 constexpr features yet");
    #elif !defined(CORRADE_TARGET_CLANG) && defined(CORRADE_TARGET_MSVC) && _MSC_VER < 1929
    CORRADE_COMPARE_AS(__cpp_constexpr, 201907,
        TestSuite::Compare::Less);
    CORRADE_SKIP("MSVC" << _MSC_VER << "doesn't support all C++20 constexpr features yet");
    #else
    CORRADE_COMPARE_AS(__cpp_constexpr, 201907,
        TestSuite::Compare::GreaterOrEqual);
    constexpr ConstexprNoInit ca = constexprNoInit(42);
    CORRADE_COMPARE(ca.a, 42);
    #endif
}

void MacrosCpp20Test::likelyUnlikely() {
    /* The LIKELY/UNLIKELY macros are on MSVC only when compiling as C++20, and
       on Clang they use the new syntax only when compiling as C++20 */

    int a = 3;

    /* Test that the macro can handle commas */
    if CORRADE_LIKELY(std::is_same<decltype(a), int>::value && a < 5) {
        a += 1;
    }

    /* Missugestion, but should still go through */
    if CORRADE_UNLIKELY(std::is_same<decltype(a), int>::value && a < 5) {
        a += 1;
    }

    CORRADE_COMPARE(a, 5);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MacrosCpp20Test)
