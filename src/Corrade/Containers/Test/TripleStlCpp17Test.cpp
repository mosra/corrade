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
#include "Corrade/Containers/PairStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct TripleStlCpp17Test: TestSuite::Tester {
    explicit TripleStlCpp17Test();

    void structuredBindings();
};

TripleStlCpp17Test::TripleStlCpp17Test() {
    addTests({&TripleStlCpp17Test::structuredBindings});
}

void TripleStlCpp17Test::structuredBindings()
{
    auto a = Triple<char, Pair<short, int>, long>{(char)42, {(short)37, 50}, 66};
    auto& [a1, a2, a3] = a;
    auto& [a4, a5] = a2;
    CORRADE_COMPARE(a1, 42);
    CORRADE_COMPARE(a4, 37);
    CORRADE_COMPARE(a5, 50);
    CORRADE_COMPARE(a3, 66);

    const auto& [b1, b2, b3] = Triple<char, Pair<short, int>, long>{(char)-42, {(short)-37, -50}, -66};
    const auto& [b4, b5] = b2;
    CORRADE_COMPARE(b1, -42);
    CORRADE_COMPARE(b4, -37);
    CORRADE_COMPARE(b5, -50);
    CORRADE_COMPARE(b3, -66);

    struct NonCopyable {
        int value = 0;
        constexpr NonCopyable() = delete;
        constexpr NonCopyable(int x) noexcept : value{x} {}
        constexpr NonCopyable(NonCopyable&&) = default;
        [[maybe_unused]] constexpr NonCopyable& operator=(NonCopyable&&) = default;
        constexpr NonCopyable(const NonCopyable&) = delete;
        constexpr NonCopyable& operator=(const NonCopyable&) = delete;
    };
    const auto c = Triple<NonCopyable, NonCopyable, NonCopyable>{123, 456, 789};
    const auto& [c1, c2, c3] = c;
    CORRADE_COMPARE(c1.value, 123);
    CORRADE_COMPARE(c2.value, 456);
    CORRADE_COMPARE(c3.value, 789);

    auto d = Triple<NonCopyable, NonCopyable, NonCopyable>{111, 222, 333};
    auto& [d1, d2, d3] = d;
    CORRADE_COMPARE(d1.value, 111);
    CORRADE_COMPARE(d2.value, 222);
    CORRADE_COMPARE(d3.value, 333);
    d1 = 1111; d2 = 2222; d3 = 3333;
    auto& [e1, e2, e3] = d;
    CORRADE_COMPARE(e1.value, 1111);
    CORRADE_COMPARE(e2.value, 2222);
    CORRADE_COMPARE(e3.value, 3333);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TripleStlCpp17Test)
