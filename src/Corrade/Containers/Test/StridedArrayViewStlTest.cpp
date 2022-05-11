/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include <algorithm> /* std::lower_bound() */

#include "Corrade/Containers/StridedArrayViewStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StridedArrayViewStlTest: TestSuite::Tester {
    explicit StridedArrayViewStlTest();

    void lowerBound();
    void unique();
};

StridedArrayViewStlTest::StridedArrayViewStlTest() {
    addTests({&StridedArrayViewStlTest::lowerBound,
              &StridedArrayViewStlTest::unique});
}

void StridedArrayViewStlTest::lowerBound() {
    struct Foo {
        int key;
        double value;
    };

    const Foo foos[]{
        {2, 0.1},
        {7, 5.6},
        {7, 7.8},
        {16, 2.2},
        {54, 0.3}
    };
    Containers::StridedArrayView1D<const int> keys = Containers::stridedArrayView(foos).slice(&Foo::key);

    {
        auto it = std::lower_bound(keys.begin(), keys.end(), 0);
        CORRADE_VERIFY(it != keys.end());
        CORRADE_COMPARE(*it, 2);
    } {
        auto it = std::lower_bound(keys.begin(), keys.end(), 7);
        CORRADE_VERIFY(it != keys.end());
        CORRADE_COMPARE(*it, 7);
    } {
        auto it = std::lower_bound(keys.begin(), keys.end(), 55);
        CORRADE_VERIFY(it == keys.end());
    }
}

void StridedArrayViewStlTest::unique() {
    struct Foo {
        int key;
        double value;
    };

    Foo foos[]{
        {2, 0.1},
        {7, 5.6},
        {7, 7.8},
        {16, 2.2},
        {16, 0.3}
    };
    Containers::StridedArrayView1D<int> keys = Containers::stridedArrayView(foos).slice(&Foo::key);

    std::size_t count = std::unique(keys.begin(), keys.end()) - keys.begin();
    CORRADE_COMPARE(count, 3);
    CORRADE_COMPARE_AS(keys.prefix(count),
        Containers::stridedArrayView({2, 7, 16}),
        TestSuite::Compare::Container);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedArrayViewStlTest)
