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

#include "Corrade/Containers/Triple.h"
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct TripleCpp17Test: TestSuite::Tester {
    explicit TripleCpp17Test();

    void structuredBindings();
    void structuredBindingsReference();
    void structuredBindingsConstReference();
    void structuredBindingsRvalueReference();
    void structuredBindingsMove();
};

TripleCpp17Test::TripleCpp17Test() {
    addTests({&TripleCpp17Test::structuredBindings,
              &TripleCpp17Test::structuredBindingsReference,
              &TripleCpp17Test::structuredBindingsConstReference,
              &TripleCpp17Test::structuredBindingsRvalueReference,
              &TripleCpp17Test::structuredBindingsMove});
}

void TripleCpp17Test::structuredBindings() {
    float a = 67.0f;
    Triple<int, float*, bool> triple{13, &a, true};
    auto [first, second, third] = triple;
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float*>::value);
    CORRADE_VERIFY(std::is_same<decltype(third), bool>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, &a);
    CORRADE_COMPARE(third, true);
}

/* Verifies the & variant behavior with constexpr */
constexpr Triple<int, float, bool> structuredBindingsReferenceConstexpr(int first, float second, bool third) {
    Triple<int, float, bool> out;
    auto& [outFirst, outSecond, outThird] = out;
    outFirst = first;
    outSecond = second;
    outThird = third;
    return out;
}

void TripleCpp17Test::structuredBindingsReference() {
    Triple<int, float, bool> triple{13, 67.0f, true};
    auto& [first, second, third] = triple;
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(third), bool>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);
    CORRADE_COMPARE(third, true);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &triple.first());
    CORRADE_COMPARE(&second, &triple.second());
    CORRADE_COMPARE(&third, &triple.third());

    constexpr Triple<int, float, bool> ctriple = structuredBindingsReferenceConstexpr(13, 67.0f, true);
    CORRADE_COMPARE(ctriple, Containers::triple(13, 67.0f, true));
}

/* Verifies the const& variant behavior with constexpr */
constexpr Triple<int, float, bool> structuredBindingsConstReferenceConstexpr(const Triple<bool, float, int>& triple) {
    auto& [first, second, third] = triple;
    return {third, second, first};
}

void TripleCpp17Test::structuredBindingsConstReference() {
    const Triple<int, float, bool> triple{13, 67.0f, true};
    auto& [first, second, third] = triple;
    CORRADE_VERIFY(std::is_same<decltype(first), const int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), const float>::value);
    CORRADE_VERIFY(std::is_same<decltype(third), const bool>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);
    CORRADE_COMPARE(third, true);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &triple.first());
    CORRADE_COMPARE(&second, &triple.second());
    CORRADE_COMPARE(&third, &triple.third());

    constexpr Triple<int, float, bool> ctriple = structuredBindingsConstReferenceConstexpr({true, 67.0f, 13});
    CORRADE_COMPARE(ctriple, Containers::triple(13, 67.0f, true));
}

/* Verifies the && variant behavior with constexpr, although not really well */
constexpr Triple<int, float, bool> structuredBindingsRvalueReferenceConstexpr(int first, float second, bool third) {
    Triple<int, float, bool> out;
    auto&& [outFirst, outSecond, outThird] = Utility::move(out);
    outFirst = first;
    outSecond = second;
    outThird = third;
    return out;
}

void TripleCpp17Test::structuredBindingsRvalueReference() {
    Triple<int, float, bool> triple{13, 67.0f, true};
    auto&& [first, second, third] = Utility::move(triple);
    CORRADE_VERIFY(std::is_same<decltype(first), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(second), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(third), bool>::value);
    CORRADE_COMPARE(first, 13);
    CORRADE_COMPARE(second, 67.0f);
    CORRADE_COMPARE(third, true);

    /* Verify it's indeed references and not a copy bound to a reference */
    CORRADE_COMPARE(&first, &triple.first());
    CORRADE_COMPARE(&second, &triple.second());
    CORRADE_COMPARE(&third, &triple.third());

    constexpr Triple<int, float, bool> ctriple = structuredBindingsRvalueReferenceConstexpr(13, 67.0f, true);
    CORRADE_COMPARE(ctriple, Containers::triple(13, 67.0f, true));
}

void TripleCpp17Test::structuredBindingsMove() {
    auto [a1, b1, c1] = Triple<int, Pointer<float>, bool>{13, Pointer<float>{Corrade::InPlaceInit, 67.0f}, true};
    auto [b2, a2, c2] = Triple<Pointer<float>, int, bool>{Pointer<float>{Corrade::InPlaceInit, 67.0f}, 13, true};
    auto [a3, c3, b3] = Triple<int, bool, Pointer<float>>{13, true, Pointer<float>{Corrade::InPlaceInit, 67.0f}};
    CORRADE_VERIFY(std::is_same<decltype(a1), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(a2), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(a3), int>::value);
    CORRADE_VERIFY(std::is_same<decltype(b1), Pointer<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(b2), Pointer<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(b3), Pointer<float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(c1), bool>::value);
    CORRADE_VERIFY(std::is_same<decltype(c2), bool>::value);
    CORRADE_VERIFY(std::is_same<decltype(c3), bool>::value);
    CORRADE_COMPARE(a1, 13);
    CORRADE_COMPARE(a2, 13);
    CORRADE_COMPARE(a3, 13);
    CORRADE_COMPARE(*b1, 67.0f);
    CORRADE_COMPARE(*b2, 67.0f);
    CORRADE_COMPARE(*b3, 67.0f);
    CORRADE_COMPARE(c1, true);
    CORRADE_COMPARE(c2, true);
    CORRADE_COMPARE(c3, true);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TripleCpp17Test)
