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
};

StaticArrayViewStlTest::StaticArrayViewStlTest() {
    addTests({&StaticArrayViewStlTest::convertFromArray,
              &StaticArrayViewStlTest::convertFromArrayEmpty,
              &StaticArrayViewStlTest::convertFromConstArray,
              &StaticArrayViewStlTest::convertFromConstArrayEmpty,
              &StaticArrayViewStlTest::convertConstFromArray,
              &StaticArrayViewStlTest::convertConstFromArrayEmpty});
}

void StaticArrayViewStlTest::convertFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = staticArrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(c), StaticArrayView<3, float>>::value));
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);
}

void StaticArrayViewStlTest::convertFromArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlTest::convertFromConstArray() {
    const std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);
}

void StaticArrayViewStlTest::convertFromConstArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlTest::convertConstFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    StaticArrayView<3, const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);
}

void StaticArrayViewStlTest::convertConstFromArrayEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewStlTest)
