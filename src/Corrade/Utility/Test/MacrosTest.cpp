/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Corrade { namespace Utility { namespace Test {

struct MacrosTest: TestSuite::Tester {
    explicit MacrosTest();

    void alignAs();
    void deprecated();
    void noreturn();
};

MacrosTest::MacrosTest() {
    addTests({&MacrosTest::alignAs,
              &MacrosTest::deprecated});
}

void MacrosTest::alignAs() {
    CORRADE_ALIGNAS(16) const char aligned[3]{};
    CORRADE_COMPARE(reinterpret_cast<std::uintptr_t>(aligned) % 16, 0);
}

namespace {

/* Declarations on their own shouldn't produce any compiler diagnostics */
CORRADE_DEPRECATED("use function() instead") int deprecatedFunction() { return 1; }
struct CORRADE_DEPRECATED("use Struct instead") DeprecatedStruct { enum: int { Value = 1 }; };
struct Struct { enum: int { Value = 1 }; };
using DeprecatedAlias CORRADE_DEPRECATED_ALIAS("use Struct instead") = Struct;
enum class CORRADE_DEPRECATED_ENUM("use Enum instead") DeprecatedEnum { Value = 1 };
enum class Foo { DeprecatedEnumValue CORRADE_DEPRECATED_ENUM("use Foo::Value instead") = 1 };
/* Not sure how to test CORRADE_DEPRECATED_FILE() though */

}

/* Uncomment to test deprecation warnings */
// #define ENABLE_DEPRECATION_WARNINGS

#ifndef ENABLE_DEPRECATION_WARNINGS
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#endif
void MacrosTest::deprecated() {
    CORRADE_VERIFY(deprecatedFunction());
    CORRADE_VERIFY(DeprecatedStruct::Value);
    CORRADE_VERIFY(DeprecatedAlias::Value);
    CORRADE_VERIFY(int(DeprecatedEnum::Value));
    CORRADE_VERIFY(int(Foo::DeprecatedEnumValue));
}
#ifndef ENABLE_DEPRECATION_WARNINGS
#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif

namespace {
    CORRADE_NORETURN void foo() { std::exit(42); }
}

void MacrosTest::noreturn() {
    if(false) foo();
    CORRADE_VERIFY(true);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MacrosTest)
