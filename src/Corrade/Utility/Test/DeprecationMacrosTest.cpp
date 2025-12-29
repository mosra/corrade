/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
#include "Corrade/Utility/DeprecationMacros.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct DeprecationMacrosTest: TestSuite::Tester {
    explicit DeprecationMacrosTest();

    void deprecated();
    void deprecatedAlias();
    void deprecatedEnum();
    void deprecatedNamespace();
    void deprecatedMacro();
    void deprecatedFile();
};

DeprecationMacrosTest::DeprecationMacrosTest() {
    addTests({&DeprecationMacrosTest::deprecated,
              &DeprecationMacrosTest::deprecatedAlias,
              &DeprecationMacrosTest::deprecatedEnum,
              &DeprecationMacrosTest::deprecatedNamespace,
              &DeprecationMacrosTest::deprecatedMacro,
              &DeprecationMacrosTest::deprecatedFile});
}

/* Declarations on their own shouldn't produce any compiler diagnostics */
CORRADE_DEPRECATED("use Variable instead") constexpr int DeprecatedVariable = 3;
CORRADE_DEPRECATED("use function() instead") int deprecatedFunction() { return 1; }
typedef CORRADE_DEPRECATED("use int instead") int DeprecatedTypedef;
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

/* Uncomment to test various subsets of deprecation warnings */
// #define ENABLE_DEPRECATION_WARNINGS
// #define ENABLE_ALIAS_DEPRECATION_WARNINGS
// #define ENABLE_ENUM_DEPRECATION_WARNINGS
// #define ENABLE_NAMESPACE_DEPRECATION_WARNINGS
// #define ENABLE_MACRO_DEPRECATION_WARNINGS
// #define ENABLE_FILE_DEPRECATION_WARNINGS

#ifndef ENABLE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecated() {
    CORRADE_COMPARE(DeprecatedVariable, 3);

    CORRADE_VERIFY(deprecatedFunction()); /* Warning on MSVC, GCC, Clang */

    DeprecatedTypedef a = 5; /* Warning on MSVC, GCC, Clang */
    CORRADE_COMPARE(a, 5);

    DeprecatedStruct s; /* Warning on MSVC, GCC, Clang */
    CORRADE_VERIFY(s.value); /* This too warns on MSVC */
    /* Doesn't fire a warning on MSVC or GCC 9 and older, only instantiating
       the struct above does. Works on Clang. */
    CORRADE_VERIFY(DeprecatedStruct::Value);
}
#ifndef ENABLE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

#ifndef ENABLE_ALIAS_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecatedAlias() {
    DeprecatedAlias a; /* Warning on MSVC 2017 (2015 unsupported), GCC, Clang */
    CORRADE_VERIFY(a.value);
    /* Doesn't fire a warning on MSVC or GCC 9 and older, only instantiating
       the struct above does. Works on Clang. */
    CORRADE_VERIFY(DeprecatedAlias::Value);
}
#ifndef ENABLE_ALIAS_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

#ifndef ENABLE_ENUM_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecatedEnum() {
    DeprecatedEnum e{}; /* Warning on MSVC 2017 (2015 ignores it), GCC, Clang */
    CORRADE_VERIFY(!int(e));
    /* Doesn't fire a warning on MSVC or GCC 9 and older, only instantiating
       the enum above does. Works on Clang and GCC 10+. */
    CORRADE_VERIFY(int(DeprecatedEnum::Value));

    /* Doesn't fire a warning on MSVC. Works on GCC and Clang. */
    CORRADE_VERIFY(int(Foo::DeprecatedEnumValue));
}
#ifndef ENABLE_ENUM_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

#ifndef ENABLE_NAMESPACE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecatedNamespace() {
    /* Warning on MSVC, Clang. Doesn't fire on GCC (because it's broken
       and thus disabled there -- see CORRADE_DEPRECATED_NAMESPACE() docs). */
    CORRADE_VERIFY(int(DeprecatedNamespace::Value));
}
#ifndef ENABLE_NAMESPACE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

#ifndef ENABLE_MACRO_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecatedMacro() {
    /* Warning on MSVC, GCC, Clang. Can only be disabled with
       CORRADE_IGNORE_DEPRECATED_PUSH on Clang, not on GCC or MSVC. */
    DEPRECATED_MACRO(hello?);

    CORRADE_VERIFY(true);
}
#ifndef ENABLE_MACRO_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

#ifndef ENABLE_FILE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_PUSH
#endif
void DeprecationMacrosTest::deprecatedFile() {
    /* Warning on MSVC, GCC, Clang, directly at the point where it's called.
       Can only be disabled with CORRADE_IGNORE_DEPRECATED_PUSH on Clang, not
       on GCC or MSVC. */
    CORRADE_DEPRECATED_FILE(
        "ignore me, I'm just testing the CORRADE_DEPRECATED_FILE() macro")

    CORRADE_VERIFY(true);
}
#ifndef ENABLE_FILE_DEPRECATION_WARNINGS
CORRADE_IGNORE_DEPRECATED_POP
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DeprecationMacrosTest)
