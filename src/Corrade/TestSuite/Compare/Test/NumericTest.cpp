/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

struct NumericTest: Tester {
    explicit NumericTest();

    void less();
    void lessOrEqual();
    void greaterOrEqual();
    void greater();

    void explicitBoolConversion();
};

NumericTest::NumericTest() {
    addTests({&NumericTest::less,
              &NumericTest::lessOrEqual,
              &NumericTest::greaterOrEqual,
              &NumericTest::greater,

              &NumericTest::explicitBoolConversion});
}

void NumericTest::less() {
    /* In random order to assure that we don't accidentally compare pointer
       values instead of float values inside (yes I forgot to dereference at
       first and *only* Clang failed the tests, both MSVC and GCC happily
       passed) */
    float b = 9.28f;
    float a = 9.27f;
    float c = 9.29f;
    CORRADE_VERIFY(Comparator<Compare::Less<float>>{}(a, b));
    CORRADE_VERIFY(!Comparator<Compare::Less<float>>{}(b, b));
    CORRADE_VERIFY(!Comparator<Compare::Less<float>>{}(c, b));

    std::stringstream out;

    {
        Error e(&out);
        Comparator<Less<float>> compare;
        CORRADE_VERIFY(!compare(c, b));
        compare.printErrorMessage(e, "c", "b");
    }

    CORRADE_COMPARE(out.str(), "Value c is not less than b, actual is 9.29 but expected < 9.28\n");
}

void NumericTest::lessOrEqual() {
    float a = 9.27f;
    float c = 9.29f;
    float b = 9.28f;
    CORRADE_VERIFY(Comparator<Compare::LessOrEqual<float>>{}(a, b));
    CORRADE_VERIFY(Comparator<Compare::LessOrEqual<float>>{}(b, b));
    CORRADE_VERIFY(!Comparator<Compare::LessOrEqual<float>>{}(c, b));

    std::stringstream out;

    {
        Error e(&out);
        Comparator<LessOrEqual<float>> compare;
        CORRADE_VERIFY(!compare(c, b));
        compare.printErrorMessage(e, "c", "b");
    }

    CORRADE_COMPARE(out.str(), "Value c is not less than or equal to b, actual is 9.29 but expected <= 9.28\n");
}

void NumericTest::greaterOrEqual() {
    float c = 9.29f;
    float b = 9.28f;
    float a = 9.27f;
    CORRADE_VERIFY(!Comparator<Compare::GreaterOrEqual<float>>{}(a, b));
    CORRADE_VERIFY(Comparator<Compare::GreaterOrEqual<float>>{}(b, b));
    CORRADE_VERIFY(Comparator<Compare::GreaterOrEqual<float>>{}(c, b));

    std::stringstream out;

    {
        Error e(&out);
        Comparator<GreaterOrEqual<float>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Value a is not greater than or equal to b, actual is 9.27 but expected >= 9.28\n");
}

void NumericTest::greater() {
    float b = 9.28f;
    float c = 9.29f;
    float a = 9.27f;
    CORRADE_VERIFY(!Comparator<Compare::Greater<float>>{}(a, b));
    CORRADE_VERIFY(!Comparator<Compare::Greater<float>>{}(b, b));
    CORRADE_VERIFY(Comparator<Compare::Greater<float>>{}(c, b));

    std::stringstream out;

    {
        Error e(&out);
        Comparator<Greater<float>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Value a is not greater than b, actual is 9.27 but expected > 9.28\n");
}

void NumericTest::explicitBoolConversion() {
    struct ExplicitBool {
        explicit operator bool() const { return true; }
    };

    struct Foo {
        ExplicitBool operator<(const Foo&) const { return ExplicitBool{}; }
        ExplicitBool operator<=(const Foo&) const { return ExplicitBool{}; }
        ExplicitBool operator>=(const Foo&) const { return ExplicitBool{}; }
        ExplicitBool operator>(const Foo&) const { return ExplicitBool{}; }
    };

    CORRADE_VERIFY(Comparator<Compare::Less<Foo>>{}({}, {}));
    CORRADE_VERIFY(Comparator<Compare::LessOrEqual<Foo>>{}({}, {}));
    CORRADE_VERIFY(Comparator<Compare::GreaterOrEqual<Foo>>{}({}, {}));
    CORRADE_VERIFY(Comparator<Compare::Greater<Foo>>{}({}, {}));
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::NumericTest)

