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

#include "Corrade/Containers/OptionalStl.h"
#include "Corrade/Containers/Pointer.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct OptionalStlTest: TestSuite::Tester {
    explicit OptionalStlTest();

    void convertCopy();
    void convertCopyNull();
    void convertMove();
    void convertMoveNull();
};

OptionalStlTest::OptionalStlTest() {
    addTests({&OptionalStlTest::convertCopy,
              &OptionalStlTest::convertCopyNull,
              &OptionalStlTest::convertMove,
              &OptionalStlTest::convertMoveNull});
}

void OptionalStlTest::convertCopy() {
    std::optional<int> a = 5;
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(*a, 5);

    Optional<int> b{a};
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(*b, 5);

    std::optional<int> c{b};
    CORRADE_VERIFY(c);
    CORRADE_COMPARE(*c, 5);

    auto d = optional(std::optional<int>{13});
    CORRADE_VERIFY((std::is_same<decltype(d), Optional<int>>::value));
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(*d, 13);
}

void OptionalStlTest::convertCopyNull() {
    std::optional<int> a;
    CORRADE_VERIFY(!a);

    Optional<int> b(a);
    CORRADE_VERIFY(!b);

    std::optional<int> c(b);
    CORRADE_VERIFY(!c);
}

void OptionalStlTest::convertMove() {
    std::optional<Pointer<int>> a{pointer(new int{15})};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(**a, 15);

    Optional<Pointer<int>> b(std::move(a));
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!*a);
    CORRADE_COMPARE(**b, 15);

    std::optional<Pointer<int>> c(std::move(b));
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!*b);
    CORRADE_COMPARE(**c, 15);

    auto d = optional(std::optional<Pointer<int>>{new int{13}});
    CORRADE_VERIFY((std::is_same<decltype(d), Optional<Pointer<int>>>::value));
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(**d, 13);
}

void OptionalStlTest::convertMoveNull() {
    std::optional<Pointer<int>> a;
    CORRADE_VERIFY(!a);

    Optional<Pointer<int>> b(std::move(a));
    CORRADE_VERIFY(!b);

    std::optional<Pointer<int>> c(std::move(b));
    CORRADE_VERIFY(!c);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::OptionalStlTest)
