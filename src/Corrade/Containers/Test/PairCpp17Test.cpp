/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct PairCpp17Test: TestSuite::Tester {
    explicit PairCpp17Test();

    void structuredBindings();
    void structuredBindingsReference();
    void structuredBindingsConstReference();
    void structuredBindingsRvalueReference();
    void structuredBindingsMove();
};

PairCpp17Test::PairCpp17Test() {
    addTests({&PairCpp17Test::structuredBindings,
              &PairCpp17Test::structuredBindingsReference,
              &PairCpp17Test::structuredBindingsConstReference,
              &PairCpp17Test::structuredBindingsRvalueReference,
              &PairCpp17Test::structuredBindingsMove});
}

void PairCpp17Test::structuredBindings() {
    float a = 67.0f;
    Pair<int, float*> pair{13, &a};
    auto [first, second] = pair;
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float*>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, &a);

    /* Constexpr behavior tested for each case (&, const&, &&) below */
}

/* Verifies the & variant behavior with constexpr */
constexpr Pair<int, float> structuredBindingsReferenceConstexpr(int first, float second) {
    Pair<int, float> out;
    auto& [outFirst, outSecond] = out;
    outFirst = first;
    outSecond = second;
    return out;
}

void PairCpp17Test::structuredBindingsReference() {
    Pair<int, float> pair{13, 67.0f};
    auto& [first, second] = pair;
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &pair.first());
    CORRADE_COMPARE(&second, &pair.second());

    constexpr Pair<int, float> cpair = structuredBindingsReferenceConstexpr(13, 67.0f);
    CORRADE_COMPARE(cpair, Containers::pair(13, 67.0f));
}

/* Verifies the const& variant behavior with constexpr */
constexpr Pair<int, float> structuredBindingsConstReferenceConstexpr(const Pair<float, int>& pair) {
    auto& [first, second] = pair;
    return {second, first};
}

void PairCpp17Test::structuredBindingsConstReference() {
    const Pair<int, float> pair{13, 67.0f};
    auto& [first, second] = pair;
    CORRADE_VERIFY(std::is_same<decltype(first), const int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), const float>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &pair.first());
    CORRADE_COMPARE(&second, &pair.second());

    constexpr Pair<int, float> cpair = structuredBindingsConstReferenceConstexpr({67.0f, 13});
    CORRADE_COMPARE(cpair, Containers::pair(13, 67.0f));
}

/* Verifies the && variant behavior with constexpr, although not really well */
constexpr Pair<int, float> structuredBindingsRvalueReferenceConstexpr(int first, float second) {
    Pair<int, float> out;
    auto&& [outFirst, outSecond] = Utility::move(out);
    outFirst = first;
    outSecond = second;
    return out;
}

void PairCpp17Test::structuredBindingsRvalueReference() {
    Pair<int, float> pair{13, 67.0f};
    auto&& [first, second] = Utility::move(pair);
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &pair.first());
    CORRADE_COMPARE(&second, &pair.second());

    /* MSVC 2017 doesn't seem to like rvalues in a constexpr context.
       Everything else works, MSVC 2019 works as well, so just ignore it. */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    constexpr
    #endif
    Pair<int, float> cpair = structuredBindingsRvalueReferenceConstexpr(13, 67.0f);
    CORRADE_COMPARE(cpair, Containers::pair(13, 67.0f));
}

void PairCpp17Test::structuredBindingsMove() {
    auto [a1, b1] = Pair<int, Pointer<float>>{13, Pointer<float>{Corrade::InPlaceInit, 67.0f}};
    auto [b2, a2] = Pair<Pointer<float>, int>{Pointer<float>{Corrade::InPlaceInit, 67.0f}, 13};
    CORRADE_VERIFY(std::is_same<decltype(a1), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(a2), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(b1), Pointer<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(b2), Pointer<float>>::value);
    CORRADE_COMPARE(a1, 13);
    CORRADE_COMPARE(a2, 13);
    CORRADE_COMPARE(*b1, 67.0f);
    CORRADE_COMPARE(*b2, 67.0f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PairCpp17Test)
