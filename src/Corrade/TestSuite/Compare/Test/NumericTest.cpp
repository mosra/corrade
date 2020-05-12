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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct NumericTest: Tester {
    explicit NumericTest();

    void less();
    void lessOrEqual();
    void greaterOrEqual();
    void greater();
    void around();

    void lessMulti();
    void lessOrEqualMulti();
    void greaterOrEqualMulti();
    void greaterMulti();
    void aroundMulti();

    void explicitBoolConversion();

    void divisible();
    void notDivisible();
};

NumericTest::NumericTest() {
    addTests({&NumericTest::less,
              &NumericTest::lessOrEqual,
              &NumericTest::greaterOrEqual,
              &NumericTest::greater,
              &NumericTest::around,

              &NumericTest::lessMulti,
              &NumericTest::lessOrEqualMulti,
              &NumericTest::greaterOrEqualMulti,
              &NumericTest::greaterMulti,
              &NumericTest::aroundMulti,

              &NumericTest::explicitBoolConversion,

              &NumericTest::divisible,
              &NumericTest::notDivisible});
}

struct BVec2 {
    BVec2(bool a, bool b): a{a}, b{b} {}
    explicit operator bool() const { return a && b; }
    bool a, b;
};

struct Vec2 {
    Vec2(float a, float b): a{a}, b{b} {}
    BVec2 operator<(const Vec2& other) const {
        return {a < other.a, b < other.b};
    }
    BVec2 operator<=(const Vec2& other) const {
        return {a <= other.a, b <= other.b};
    }
    BVec2 operator>=(const Vec2& other) const {
        return {a >= other.a, b >= other.b};
    }
    BVec2 operator>(const Vec2& other) const {
        return {a > other.a, b > other.b};
    }
    Vec2 operator+(const Vec2& other) const {
        return {a + other.a, b + other.b};
    }
    Vec2 operator-(const Vec2& other) const {
        return {a - other.a, b - other.b};
    }
    float a, b;
};

void NumericTest::less() {
    /* In random order to assure that we don't accidentally compare pointer
       values instead of float values inside (yes I forgot to dereference at
       first and *only* Clang failed the tests, both MSVC and GCC happily
       passed) */
    float b = 9.28f;
    float a = 9.27f;
    float c = 9.29f;
    CORRADE_COMPARE(Comparator<Compare::Less<float>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Less<float>>{}(b, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Less<float>>{}(c, b), ComparisonStatusFlag::Failed);

    std::stringstream out;

    {
        Error e(&out);
        Comparator<Less<float>> compare;
        ComparisonStatusFlags flags = compare(c, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "c", "b");
    }

    CORRADE_COMPARE(out.str(), "Value c is not less than b, actual is 9.29 but expected < 9.28\n");
}

void NumericTest::lessOrEqual() {
    float a = 9.27f;
    float c = 9.29f;
    float b = 9.28f;
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<float>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<float>>{}(b, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<float>>{}(c, b), ComparisonStatusFlag::Failed);

    std::stringstream out;

    {
        Error e(&out);
        Comparator<LessOrEqual<float>> compare;
        ComparisonStatusFlags flags = compare(c, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "c", "b");
    }

    CORRADE_COMPARE(out.str(), "Value c is not less than or equal to b, actual is 9.29 but expected <= 9.28\n");
}

void NumericTest::greaterOrEqual() {
    float c = 9.29f;
    float b = 9.28f;
    float a = 9.27f;
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<float>>{}(a, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<float>>{}(b, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<float>>{}(c, b), ComparisonStatusFlags{});

    std::stringstream out;

    {
        Error e(&out);
        Comparator<GreaterOrEqual<float>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Value a is not greater than or equal to b, actual is 9.27 but expected >= 9.28\n");
}

void NumericTest::greater() {
    float b = 9.28f;
    float c = 9.29f;
    float a = 9.27f;
    CORRADE_COMPARE(Comparator<Compare::Greater<float>>{}(a, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Greater<float>>{}(b, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Greater<float>>{}(c, b), ComparisonStatusFlags{});

    std::stringstream out;

    {
        Error e(&out);
        Comparator<Greater<float>> compare;
        ComparisonStatusFlags flags = compare(a, b);
        compare.printMessage(flags, e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Value a is not greater than b, actual is 9.27 but expected > 9.28\n");
}

void NumericTest::around() {
    float b = 9.28f;
    float a = 9.25f;
    float c = 9.31f;
    float d = 9.29f;
    float e = 9.27f;
    CORRADE_COMPARE(Comparator<Compare::Around<float>>{0.02f}(a, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Around<float>>{0.02f}(c, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Around<float>>{0.02f}(d, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Around<float>>{0.02f}(e, b), ComparisonStatusFlags{});

    std::stringstream out;

    {
        Error err{&out};
        Comparator<Around<float>> compare{0.02f};
        ComparisonStatusFlags flags = compare(a, b);
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, err, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Value a is not around b, actual is 9.25 but 9.26 <= expected <= 9.3\n");

    /* Verify that the helper works */
    CORRADE_COMPARE_WITH(d, b, Compare::around(0.02f));
}

void NumericTest::lessMulti() {
    Vec2 a{9.27f, 3.11f};
    Vec2 b{9.28f, 3.12f};
    Vec2 c{9.28f, 3.10f};
    CORRADE_COMPARE(Comparator<Compare::Less<Vec2>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Less<Vec2>>{}(b, b), ComparisonStatusFlag::Failed);

    /* b is neither less nor greater/equal than c */
    CORRADE_COMPARE(Comparator<Compare::Less<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
}

void NumericTest::lessOrEqualMulti() {
    Vec2 a{9.27f, 3.11f};
    Vec2 b{9.27f, 3.12f};
    Vec2 c{9.28f, 3.10f};
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<Vec2>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<Vec2>>{}(b, b), ComparisonStatusFlags{});

    /* b is neither less/equal nor greater than c */
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Greater<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
}

void NumericTest::greaterOrEqualMulti() {
    Vec2 a{9.27f, 3.12f};
    Vec2 b{9.27f, 3.11f};
    Vec2 c{9.28f, 3.10f};
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<Vec2>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<Vec2>>{}(b, b), ComparisonStatusFlags{});

    /* b is neither greater/equal nor less than c */
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Less<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
}

void NumericTest::greaterMulti() {
    Vec2 a{9.28f, 3.12f};
    Vec2 b{9.27f, 3.11f};
    Vec2 c{9.28f, 3.10f};
    CORRADE_COMPARE(Comparator<Compare::Greater<Vec2>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Greater<Vec2>>{}(b, b), ComparisonStatusFlag::Failed);

    /* b is neither greater nor less/equal than c */
    CORRADE_COMPARE(Comparator<Compare::Greater<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<Vec2>>{}(c, b), ComparisonStatusFlag::Failed);
}

void NumericTest::aroundMulti() {
    Vec2 epsilon{0.02f, 0.02f};

    Vec2 a{9.25f, 3.08f};
    Vec2 b{9.28f, 3.11f};
    Vec2 c{9.31f, 3.14f};
    Vec2 d{9.29f, 3.10f};
    Vec2 e{9.25f, 3.14f};

    /* Too below for both / too above for both */
    CORRADE_COMPARE(Comparator<Compare::Around<Vec2>>{epsilon}(a, b), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Around<Vec2>>{epsilon}(c, b), ComparisonStatusFlag::Failed);

    /* Slightly above for one and slightly below for the other */
    CORRADE_COMPARE(Comparator<Compare::Around<Vec2>>{epsilon}(d, b), ComparisonStatusFlags{});

    /* Too below for one and too above for the other */
    CORRADE_COMPARE(Comparator<Compare::Around<Vec2>>{epsilon}(e, b), ComparisonStatusFlag::Failed);
}

void NumericTest::explicitBoolConversion() {
    struct ExplicitBool {
        explicit operator bool() const { return true; }
    };

    struct Foo {
        ExplicitBool operator<(const Foo&) const { return {}; }
        ExplicitBool operator<=(const Foo&) const { return {}; }
        ExplicitBool operator>=(const Foo&) const { return {}; }
        ExplicitBool operator>(const Foo&) const { return {}; }
    };

    struct Bar {
        ExplicitBool operator>=(const Bar&) const { return {}; }
        Bar operator-(const Bar&) const { return {}; }
        Bar operator+(const Bar&) const { return {}; }
    };

    CORRADE_COMPARE(Comparator<Compare::Less<Foo>>{}({}, {}), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::LessOrEqual<Foo>>{}({}, {}), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::GreaterOrEqual<Foo>>{}({}, {}), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Greater<Foo>>{}({}, {}), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Around<Bar>>{{}}({}, {}), ComparisonStatusFlags{});
}

void NumericTest::divisible() {
    int b = 4;
    int c = 8;
    int a = 20;
    CORRADE_COMPARE(Comparator<Compare::Divisible<int>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::Divisible<int>>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::Divisible<int>>{}(b, a), ComparisonStatusFlag::Failed);

    std::stringstream out;

    {
        Error e(&out);
        Comparator<Divisible<int>> compare;
        ComparisonStatusFlags flags = compare(a, c);
        compare.printMessage(flags, e, "a", "c");
    }

    CORRADE_COMPARE(out.str(), "Value a is not divisible by c, 20 % 8 was not expected to be 4\n");
}

void NumericTest::notDivisible() {
    int b = 8;
    int c = 4;
    int a = 20;
    CORRADE_COMPARE(Comparator<Compare::NotDivisible<int>>{}(a, b), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<Compare::NotDivisible<int>>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<Compare::NotDivisible<int>>{}(b, a), ComparisonStatusFlags{});

    std::stringstream out;

    {
        Error e(&out);
        Comparator<NotDivisible<int>> compare;
        ComparisonStatusFlags flags = compare(a, c);
        compare.printMessage(flags, e, "a", "c");
    }

    CORRADE_COMPARE(out.str(), "Value a is divisible by c, 20 % 4 was not expected to be 0\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::NumericTest)
