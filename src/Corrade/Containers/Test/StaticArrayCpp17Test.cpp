/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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

#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StaticArrayCpp17Test: TestSuite::Tester {
    explicit StaticArrayCpp17Test();

    void structuredBindings();
    void structuredBindingsReference();
    void structuredBindingsConstReference();
    void structuredBindingsRvalueReference();
    void structuredBindingsMove();
};

StaticArrayCpp17Test::StaticArrayCpp17Test() {
    addTests({&StaticArrayCpp17Test::structuredBindings,
              &StaticArrayCpp17Test::structuredBindingsReference,
              &StaticArrayCpp17Test::structuredBindingsConstReference,
              &StaticArrayCpp17Test::structuredBindingsRvalueReference,
              &StaticArrayCpp17Test::structuredBindingsMove});
}

void StaticArrayCpp17Test::structuredBindings() {
    Array2<float> array{32.5f, -2.25f};
    auto [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Constexpr behavior tested for each case (&, const&, &&) below */
}

/* Verifies the & variant behavior with constexpr */
constexpr Array2<float> structuredBindingsReferenceConstexpr(float first, float second) {
    Array2<float> out;
    auto& [outSecond, outFirst] = out;
    outFirst = first;
    outSecond = second;
    return out;
}

void StaticArrayCpp17Test::structuredBindingsReference() {
    Array2<float> array{32.5f, -2.25f};
    auto& [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &array[0]);
    CORRADE_COMPARE(&a1, &array[1]);

    constexpr Array2<float> carray = structuredBindingsReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(carray[0], -2.25f);
    CORRADE_COMPARE(carray[1], 32.5f);
}

/* Verifies the const& variant behavior with constexpr */
constexpr Array2<float> structuredBindingsConstReferenceConstexpr(const Array2<float>& pair) {
    auto& [first, second] = pair;
    return {second, first};
}

void StaticArrayCpp17Test::structuredBindingsConstReference() {
    const Array2<float> array{32.5f, -2.25f};
    auto& [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), const float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), const float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &array[0]);
    CORRADE_COMPARE(&a1, &array[1]);

    constexpr Array2<float> carray = structuredBindingsConstReferenceConstexpr({32.5f, -2.25f});
    CORRADE_COMPARE(carray[0], -2.25f);
    CORRADE_COMPARE(carray[1], 32.5f);
}

/* Verifies the && variant behavior with constexpr, although not really well */
constexpr Array2<float> structuredBindingsRvalueReferenceConstexpr(float first, float second) {
    Array2<float> out;
    auto&& [outSecond, outFirst] = Utility::move(out);
    outFirst = first;
    outSecond = second;
    return out;
}

void StaticArrayCpp17Test::structuredBindingsRvalueReference() {
    Array2<float> array{32.5f, -2.25f};
    auto&& [a0, a1] = Utility::move(array);
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &array[0]);
    CORRADE_COMPARE(&a1, &array[1]);

    /* MSVC 2017 doesn't seem to like rvalues in a constexpr context.
       Everything else works, MSVC 2019 works as well, so just ignore it. */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    constexpr
    #endif
    Array2<float> carray = structuredBindingsRvalueReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(carray[0], -2.25f);
    CORRADE_COMPARE(carray[1], 32.5f);
}

void StaticArrayCpp17Test::structuredBindingsMove() {
    auto [a0, a1] = Array2<Pointer<float>>{Pointer<float>{Corrade::InPlaceInit, 32.5f}, Pointer<float>{Corrade::InPlaceInit, -2.25f}};
    CORRADE_VERIFY(std::is_same<decltype(a0), Pointer<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), Pointer<float>>::value);
    CORRADE_COMPARE(*a0, 32.5f);
    CORRADE_COMPARE(*a1, -2.25f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayCpp17Test)
