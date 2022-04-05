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

#include <utility>

#include "Corrade/Containers/TripleStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct TripleStlTest: TestSuite::Tester {
    explicit TripleStlTest();

    void convertCopy();
    void convertMove();
};

TripleStlTest::TripleStlTest() {
    addTests({&TripleStlTest::convertCopy,
              &TripleStlTest::convertMove});
}

void TripleStlTest::convertCopy() {
    std::tuple<float, int, bool> a{35.0f, 4, true};

    Triple<float, int, bool> b = a;
    CORRADE_COMPARE(b.first(), 35.0f);
    CORRADE_COMPARE(b.second(), 4);
    CORRADE_COMPARE(b.third(), true);

    std::tuple<float, int, bool> c = b;
    CORRADE_COMPARE(std::get<0>(c), 35.0f);
    CORRADE_COMPARE(std::get<1>(c), 4);
    CORRADE_COMPARE(std::get<2>(c), true);

    auto d = triple(c);
    CORRADE_VERIFY(std::is_same<decltype(d), Triple<float, int, bool>>::value);
    CORRADE_COMPARE(d.first(), 35.0f);
    CORRADE_COMPARE(d.second(), 4);
    CORRADE_COMPARE(d.third(), true);
}

void TripleStlTest::convertMove() {
    std::tuple<Pointer<float>, Pointer<int>, Pointer<bool>> a{pointer(new float{35.0f}), pointer(new int{4}), Pointer<bool>{Corrade::InPlaceInit, true}};

    Triple<Pointer<float>, Pointer<int>, Pointer<bool>> b = std::move(a);
    CORRADE_COMPARE(*b.first(), 35.0f);
    CORRADE_COMPARE(*b.second(), 4);
    CORRADE_COMPARE(*b.third(), true);
    CORRADE_VERIFY(!std::get<0>(a));
    CORRADE_VERIFY(!std::get<1>(a));
    CORRADE_VERIFY(!std::get<2>(a));

    std::tuple<Pointer<float>, Pointer<int>, Pointer<bool>> c = std::move(b);
    CORRADE_COMPARE(*std::get<0>(c), 35.0f);
    CORRADE_COMPARE(*std::get<1>(c), 4);
    CORRADE_COMPARE(*std::get<2>(c), true);
    CORRADE_VERIFY(!b.first());
    CORRADE_VERIFY(!b.second());
    CORRADE_VERIFY(!b.third());

    auto d = triple(std::move(c));
    CORRADE_VERIFY(std::is_same<decltype(d), Triple<Pointer<float>, Pointer<int>, Pointer<bool>>>::value);
    CORRADE_COMPARE(*d.first(), 35.0f);
    CORRADE_COMPARE(*d.second(), 4);
    CORRADE_COMPARE(*d.third(), true);
    CORRADE_VERIFY(!std::get<0>(c));
    CORRADE_VERIFY(!std::get<1>(c));
    CORRADE_VERIFY(!std::get<2>(c));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TripleStlTest)
