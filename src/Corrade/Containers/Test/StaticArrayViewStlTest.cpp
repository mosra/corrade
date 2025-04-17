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

#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StaticArrayViewStlTest: TestSuite::Tester {
    explicit StaticArrayViewStlTest();

    void convertFromArray();
    void convertFromArrayEmpty();
    void convertFromConstArray();
    void convertFromConstArrayEmpty();
    void convertConstFromArray();
    void convertConstFromArrayEmpty();

    void convertFromArrayDerived();
    void convertConstFromArrayDerived();
};

StaticArrayViewStlTest::StaticArrayViewStlTest() {
    addTests({&StaticArrayViewStlTest::convertFromArray,
              &StaticArrayViewStlTest::convertFromArrayEmpty,
              &StaticArrayViewStlTest::convertFromConstArray,
              &StaticArrayViewStlTest::convertFromConstArrayEmpty,
              &StaticArrayViewStlTest::convertConstFromArray,
              &StaticArrayViewStlTest::convertConstFromArrayEmpty,

              &StaticArrayViewStlTest::convertFromArrayDerived,
              &StaticArrayViewStlTest::convertConstFromArrayDerived});
}

void StaticArrayViewStlTest::convertFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = staticArrayView(a);
    CORRADE_VERIFY(std::is_same<decltype(c), StaticArrayView<3, float>>::value);
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c[0], 42.0f);
}

void StaticArrayViewStlTest::convertFromArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlTest::convertFromConstArray() {
    const std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b[0], 42.0f);
}

void StaticArrayViewStlTest::convertFromConstArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlTest::convertConstFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Creating a non-const view from a const array should not be possible. Not
       using is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, const float>, std::array<float, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, float>, const std::array<float, 3>>::value);
}

void StaticArrayViewStlTest::convertConstFromArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

struct Base {
    float a;
};
struct Derived: Base {
    /* FFS why is this needed?! */
    /*implicit*/ Derived(float a): Base{a} {}
};
struct DerivedDifferentSize: Base {
    int b;
};

void StaticArrayViewStlTest::convertFromArrayDerived() {
    std::array<Derived, 3> a{{{42.0f}, {13.3f}, {-25.0f}}};

    StaticArrayView<3, Base> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b[0].a, 42.0f);

    /* Conversion the other way not allowed. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, Base>, std::array<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Derived>, std::array<Base, 3>>::value);
    /* Conversion from a derived type that isn't the same size shouldn't be
       allowed either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, std::array<DerivedDifferentSize, 3>>::value);
}

void StaticArrayViewStlTest::convertConstFromArrayDerived() {
    std::array<Derived, 3> a{{{42.0f}, {13.3f}, {-25.0f}}};

    StaticArrayView<3, const Base> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b[0].a, 42.0f);

    /* Conversion the other way not allowed. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, const Base>, std::array<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const Derived>, std::array<Base, 3>>::value);
    /* Creating a non-const view from a const array should not be possible
       either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, std::array<const Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, const std::array<Derived, 3>>::value);
    /* Conversion from a derived type that isn't the same size shouldn't be
       allowed either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const Base>, std::array<DerivedDifferentSize, 3>>::value);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewStlTest)
