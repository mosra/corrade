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

#include "Corrade/Containers/Triple.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct TripleCpp14Test: TestSuite::Tester {
    explicit TripleCpp14Test();

    void accessConstexpr();
    void accessRvalueConstexpr();
};

TripleCpp14Test::TripleCpp14Test() {
    addTests({&TripleCpp14Test::accessConstexpr,
              &TripleCpp14Test::accessRvalueConstexpr});
}

constexpr Triple<float, int, double> populate() {
    Triple<float, int, double> a;
    a.first() = 3.5f;
    a.second() = 17;
    a.third() = 0.007;
    return a;
}

void TripleCpp14Test::accessConstexpr() {
    constexpr Triple<float, int, double> a = populate();
    CORRADE_COMPARE(a.first(), 3.5f);
    CORRADE_COMPARE(a.second(), 17);
    CORRADE_COMPARE(a.third(), 0.007);
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

void TripleCpp14Test::accessRvalueConstexpr() {
    constexpr Movable a = Triple<Movable, int, int>{Movable{5}, 3, 2}.first();
    constexpr Movable b = Triple<int, Movable, int>{5, Movable{3}, 2}.second();
    constexpr Movable c = Triple<int, int, Movable>{5, 3, Movable{2}}.third();
    CORRADE_COMPARE(a.a, 5);
    CORRADE_COMPARE(b.a, 3);
    CORRADE_COMPARE(c.a, 2);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TripleCpp14Test)
