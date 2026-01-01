/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#include "Corrade/Containers/Pair.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct PairCpp14Test: TestSuite::Tester {
    explicit PairCpp14Test();

    void accessConstexpr();
    void accessRvalueConstexpr();
};

PairCpp14Test::PairCpp14Test() {
    addTests({&PairCpp14Test::accessConstexpr,
              &PairCpp14Test::accessRvalueConstexpr});
}

constexpr Pair<float, int> populate() {
    Pair<float, int> a;
    a.first() = 3.5f;
    a.second() = 17;
    return a;
}

void PairCpp14Test::accessConstexpr() {
    constexpr Pair<float, int> a = populate();
    CORRADE_COMPARE(a.first(), 3.5f);
    CORRADE_COMPARE(a.second(), 17);
}

struct Movable {
    constexpr explicit Movable(int a = 0) noexcept: a{a} {}
    Movable(const Movable&) = delete;
    constexpr Movable(Movable&& other) noexcept: a(other.a) {}
    Movable& operator=(const Movable&) = delete;
    /* Clang says this is unused. But if I remove it, some other compiler might
       fail. */
    CORRADE_UNUSED constexpr Movable& operator=(Movable&& other) noexcept {
        a = other.a;
        return *this;
    }

    int a;
};

void PairCpp14Test::accessRvalueConstexpr() {
    constexpr Movable a = Pair<Movable, int>{Movable{5}, 3}.first();
    constexpr Movable b = Pair<int, Movable>{5, Movable{3}}.second();
    CORRADE_COMPARE(a.a, 5);
    CORRADE_COMPARE(b.a, 3);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PairCpp14Test)
