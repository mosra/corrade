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

#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StaticArrayViewCpp17Test: TestSuite::Tester {
    explicit StaticArrayViewCpp17Test();

    void structuredBindings();
    void structuredBindingsReference();
    void structuredBindingsConstReference();
    void structuredBindingsRvalueReference();
    /* As the view is non-owning, a rvalue doesn't imply that its contents are
       able to be moved out. Thus, unlike StaticArray or Pair/Triple, it has no
       difference in behavior depending on whether the input is T&, const T& or
       T&&, it always returns a T& */
};

StaticArrayViewCpp17Test::StaticArrayViewCpp17Test() {
    addTests({&StaticArrayViewCpp17Test::structuredBindings,
              &StaticArrayViewCpp17Test::structuredBindingsReference,
              &StaticArrayViewCpp17Test::structuredBindingsConstReference,
              &StaticArrayViewCpp17Test::structuredBindingsRvalueReference});
}

void StaticArrayViewCpp17Test::structuredBindings() {
    float data[]{32.5f, -2.25f};
    ArrayView2<float> array = data;
    auto [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Constexpr behavior tested for each case (&, const&, &&) below */
}

/* Verifies the & variant behavior with constexpr */
constexpr float structuredBindingsReferenceConstexpr(float a0, float a1) {
    float data[2]{};
    ArrayView2<float> out = data;
    auto& [outA0, outA1] = out;
    outA0 = a0;
    outA1 = a1;
    return data[0] - data[1];
}

void StaticArrayViewCpp17Test::structuredBindingsReference() {
    float data[]{32.5f, -2.25f};
    ArrayView2<float> array = data;
    auto& [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &data[0]);
    CORRADE_COMPARE(&a1, &data[1]);

    constexpr float carray = structuredBindingsReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(carray, 34.75f);
}

/* Verifies the const& variant behavior with constexpr */
constexpr float structuredBindingsConstReferenceConstexpr(const ArrayView2<const float>& size) {
    auto& [a1, a0] = size;
    return a0 - a1;
}

void StaticArrayViewCpp17Test::structuredBindingsConstReference() {
    const float data[]{32.5f, -2.25f};
    ArrayView2<const float> array = data;
    auto& [a0, a1] = array;
    CORRADE_VERIFY(std::is_same<decltype(a0), const float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), const float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &data[0]);
    CORRADE_COMPARE(&a1, &data[1]);

    constexpr float cdata[]{-2.25f, 32.5f};
    constexpr float carray = structuredBindingsConstReferenceConstexpr(cdata);
    CORRADE_COMPARE(carray, 34.75f);
}

/* Verifies the && variant behavior with constexpr, although not really well */
constexpr float structuredBindingsRvalueReferenceConstexpr(float a0, float a1) {
    float data[2]{};
    ArrayView2<float> out = data;
    auto&& [outA0, outA1] = Utility::move(out);
    outA0 = a0;
    outA1 = a1;
    return data[0] - data[1];
}

void StaticArrayViewCpp17Test::structuredBindingsRvalueReference() {
    float data[]{32.5f, -2.25f};
    ArrayView2<float> array = data;
    /* It's actually still float& */
    auto&& [a0, a1] = Utility::move(array);
    CORRADE_VERIFY(std::is_same<decltype(a0), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(a1), float>::value);
    CORRADE_COMPARE(a0, 32.5f);
    CORRADE_COMPARE(a1, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&a0, &data[0]);
    CORRADE_COMPARE(&a1, &data[1]);

    /* MSVC 2017 doesn't seem to like the move in a constexpr context.
       Everything else works, MSVC 2019 works as well, so just ignore it. */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    constexpr
    #endif
    float carray = structuredBindingsRvalueReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(carray, 34.75f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewCpp17Test)
