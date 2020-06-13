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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/DebugStl.h"

#ifndef CORRADE_TARGET_EMSCRIPTEN
#include <thread>
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

/* Putting this as early as possible so it doesn't get changed every time */
constexpr const char* ThisIsLine37 = CORRADE_LINE_STRING;

struct MacrosTest: TestSuite::Tester {
    explicit MacrosTest();

    void alignAs();
    void deprecated();
    void noreturn();
    void fallthrough();
    void cxxStandard();
    void alwaysNeverInline();
    void function();
    void lineString();

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    void threadLocal();
    #endif
};

MacrosTest::MacrosTest() {
    addTests({&MacrosTest::alignAs,
              &MacrosTest::deprecated,
              &MacrosTest::noreturn,
              &MacrosTest::fallthrough,
              &MacrosTest::cxxStandard,
              &MacrosTest::alwaysNeverInline,
              &MacrosTest::function,
              &MacrosTest::lineString,

              #ifndef CORRADE_TARGET_EMSCRIPTEN
              &MacrosTest::threadLocal
              #endif
              });
}

void MacrosTest::alignAs() {
    CORRADE_ALIGNAS(16) const char aligned[3]{};
    CORRADE_COMPARE(reinterpret_cast<std::uintptr_t>(aligned) % 16, 0);
}

/* Declarations on their own shouldn't produce any compiler diagnostics */
CORRADE_DEPRECATED("use Variable instead") constexpr int DeprecatedVariable = 3;
CORRADE_DEPRECATED("use function() instead") int deprecatedFunction() { return 1; }
struct CORRADE_DEPRECATED("use Struct instead") DeprecatedStruct { enum: int { Value = 1 }; int value = 1; };
struct Struct { enum: int { Value = 1 }; int value = 1; };
using DeprecatedAlias CORRADE_DEPRECATED_ALIAS("use Struct instead") = Struct;
enum class CORRADE_DEPRECATED_ENUM("use Enum instead") DeprecatedEnum { Value = 1 };
enum class Foo { DeprecatedEnumValue CORRADE_DEPRECATED_ENUM("use Foo::Value instead") = 1 };
namespace CORRADE_DEPRECATED_NAMESPACE("use Namespace instead") DeprecatedNamespace {
    enum: int { Value = 1 };
}

#define MACRO(foo) do {} while(false)
#define DEPRECATED_MACRO(foo) \
    CORRADE_DEPRECATED_MACRO(DEPRECATED_MACRO(),"ignore me, I'm just testing the CORRADE_DEPRECATED_MACRO() macro") MACRO(foo)

/* Uncomment to test deprecation warnings */
// #define ENABLE_DEPRECATION_WARNINGS

#ifndef ENABLE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
CORRADE_DEPRECATED_FILE( /* Warning on MSVC, GCC, Clang */
    "ignore me, I'm just testing the CORRADE_DEPRECATED_FILE() macro")

void MacrosTest::deprecated() {
    DEPRECATED_MACRO(hello?); /* Warning on MSVC, GCC, Clang */

    CORRADE_COMPARE(DeprecatedVariable, 3);

    CORRADE_VERIFY(deprecatedFunction()); /* Warning on MSVC, GCC, Clang */

    DeprecatedStruct s; /* Warning on MSVC, GCC, Clang */
    CORRADE_VERIFY(s.value); /* This too warns on MSVC */
    /* Doesn't fire a warning on MSVC or GCC, only instantiating the struct
       above does. Works on Clang. */
    CORRADE_VERIFY(DeprecatedStruct::Value);

    DeprecatedAlias a; /* Warning on MSVC 2017 (2015 unsupported), GCC, Clang */
    CORRADE_VERIFY(a.value);
    /* Doesn't fire a warning on MSVC or GCC, only instantiating the struct
       above does. Works on Clang. */
    CORRADE_VERIFY(DeprecatedAlias::Value);

    DeprecatedEnum e{}; /* Warning on MSVC 2017 (2015 ignores it), GCC, Clang */
    CORRADE_VERIFY(!int(e));
    /* Doesn't fire a warning on MSVC or GCC, only instantiating the enum above
       does. Works on Clang. */
    CORRADE_VERIFY(int(DeprecatedEnum::Value));

    /* Doesn't fire a warning on MSVC. Works on GCC and Clang. */
    CORRADE_VERIFY(int(Foo::DeprecatedEnumValue));

    /* Warning on MSVC, Clang. Doesn't fire on GCC (because it's broken
       and thus disabled there -- see CORRADE_DEPRECATED_NAMESPACE() docs). */
    CORRADE_VERIFY(int(DeprecatedNamespace::Value));
}
#ifndef ENABLE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

CORRADE_NORETURN void foo() { std::exit(42); }

void MacrosTest::noreturn() {
    if(false) foo();
    CORRADE_VERIFY(true);
}

void MacrosTest::fallthrough() {
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

void MacrosTest::cxxStandard() {
    CORRADE_COMPARE_AS(CORRADE_CXX_STANDARD, 201103, TestSuite::Compare::GreaterOrEqual);
}

CORRADE_ALWAYS_INLINE int alwaysInline() { return 5; }
CORRADE_NEVER_INLINE int neverInline() { return 37; }

void MacrosTest::alwaysNeverInline() {
    CORRADE_COMPARE(alwaysInline() + neverInline(), 42);
}

/* Needs another inner anonymous namespace otherwise Clang complains about a
   missing prototype (UGH) */
namespace SubNamespace { namespace {
    const char* thisIsAFunction(int, float) {
        return CORRADE_FUNCTION;
    }
}}

void MacrosTest::function() {
    /* Should be really just a function name, with no mangled signature or
       surrounding namespace. Compare as a string to avoid comparing
       pointers -- they are equal on some compilers, but not always. */
    CORRADE_COMPARE(SubNamespace::thisIsAFunction(1, 0.0f), std::string{"thisIsAFunction"});
}

void MacrosTest::lineString() {
    CORRADE_COMPARE(ThisIsLine37, std::string{"37"});
}

#ifndef CORRADE_TARGET_EMSCRIPTEN
CORRADE_THREAD_LOCAL int threadLocalVar = 3;
int globalVar = 3;

void MacrosTest::threadLocal() {
    threadLocalVar = 5;
    globalVar = 15;

    CORRADE_COMPARE(threadLocalVar, 5);
    CORRADE_COMPARE(globalVar, 15);

    std::thread t{[]() {
        threadLocalVar = 7;
        globalVar = 17;
    }};

    t.join();

    CORRADE_COMPARE(threadLocalVar, 5);
    CORRADE_COMPARE(globalVar, 17);
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MacrosTest)
