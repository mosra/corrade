/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <utility>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Move.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct MoveTest: TestSuite::Tester {
    explicit MoveTest();

    void forward();
    void move();
};

MoveTest::MoveTest() {
    addTests({&MoveTest::forward,
              &MoveTest::move});
}

struct Foo {
    int a;
};

template<class L, class CL, class R, class CR, class V> void check(L&& l, CL&& cl, R&& r, CR&& cr, V&& v) {
    /* It makes sense to take std::forward() as a ground truth */
    CORRADE_VERIFY(std::is_same<decltype(std::forward<L>(l)), Foo&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<L>(l)), Foo&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<CL>(cl)), const Foo&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<CL>(cl)), const Foo&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<R>(r)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<R>(r)), Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<CR>(cr)), const Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<CR>(cr)), const Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::forward<V>(v)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::forward<V>(v)), Foo&&>::value);
}

constexpr Foo ca{5};

void MoveTest::forward() {
    CORRADE_VERIFY(true); /* to register correct function name */

    Foo a{1};

    /* Using std::move() here to avoid accumulating bugs in case
       Utility::move() would be broken */
    check(a, ca, std::move(a), std::move(ca), Foo{3});

    /* Verify the utility can be used in a constexpr context */
    constexpr Foo cb = Utility::forward<const Foo>(ca);
    constexpr Foo cc = Utility::forward<Foo>(Foo{7});
    CORRADE_COMPARE(cb.a, 5);
    CORRADE_COMPARE(cc.a, 7);
}

void MoveTest::move() {
    Foo a;

    /* It makes sense to take std::move() as a ground truth */
    CORRADE_VERIFY(std::is_same<decltype(std::move(a)), Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::move(a)), Foo&&>::value);

    CORRADE_VERIFY(std::is_same<decltype(std::move(ca)), const Foo&&>::value);
    CORRADE_VERIFY(std::is_same<decltype(Utility::move(ca)), const Foo&&>::value);

    constexpr Foo cb = Utility::move(ca);
    CORRADE_COMPARE(cb.a, 5);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MoveTest)
