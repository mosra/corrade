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

#include "Corrade/Containers/StridedDimensions.h"
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StridedDimensionsCpp17Test: TestSuite::Tester {
    explicit StridedDimensionsCpp17Test();

    void structuredBindings();
    void structuredBindingsReference();
    void structuredBindingsConstReference();
    void structuredBindingsRvalueReference();
    /* There's no point in having non-trivial / move-only types in
       StridedDimensions, so unlike Pair and Triple, move behavior isn't tested
       here even though a r-value overload needs to be present */
};

StridedDimensionsCpp17Test::StridedDimensionsCpp17Test() {
    addTests({&StridedDimensionsCpp17Test::structuredBindings,
              &StridedDimensionsCpp17Test::structuredBindingsReference,
              &StridedDimensionsCpp17Test::structuredBindingsConstReference,
              &StridedDimensionsCpp17Test::structuredBindingsRvalueReference});
}

void StridedDimensionsCpp17Test::structuredBindings() {
    /* Deliberately checking with a type that's used neither for SizeND nor for
       StrideND to verify there's no accidental type assumption anywhere */
    StridedDimensions<3, float> size{16.0f, 32.5f, -2.25f};
    auto [z, y, x] = size;
    CORRADE_VERIFY(std::is_same<decltype(z), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(y), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(x), float>::value);
    CORRADE_COMPARE(z, 16.0f);
    CORRADE_COMPARE(y, 32.5f);
    CORRADE_COMPARE(x, -2.25f);
}

/* Verifies the & variant behavior with constexpr */
constexpr StridedDimensions<2, float> structuredBindingsReferenceConstexpr(float y, float x) {
    StridedDimensions<2, float> out;
    auto& [outY, outX] = out;
    outY = y;
    outX = x;
    return out;
}

void StridedDimensionsCpp17Test::structuredBindingsReference() {
    StridedDimensions<2, float> size{32.5f, -2.25f};
    auto& [y, x] = size;
    CORRADE_VERIFY(std::is_same<decltype(y), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(x), float>::value);
    CORRADE_COMPARE(y, 32.5f);
    CORRADE_COMPARE(x, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference. The
       const operator[] returns a copy so we compare to begin() instead. */
    CORRADE_COMPARE(&y, size.begin());
    CORRADE_COMPARE(&x, size.begin() + 1);

    constexpr StridedDimensions<2, float> csize = structuredBindingsReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(csize, (StridedDimensions<2, float>{32.5f, -2.25f}));
}

/* Verifies the const& variant behavior with constexpr */
constexpr StridedDimensions<2, float> structuredBindingsConstReferenceConstexpr(const StridedDimensions<2, float>& size) {
    auto& [y, x] = size;
    return {x, y};
}

void StridedDimensionsCpp17Test::structuredBindingsConstReference() {
    const StridedDimensions<2, float> size{32.5f, -2.25f};
    auto& [y, x] = size;
    CORRADE_VERIFY(std::is_same<decltype(y), const float>::value);
    CORRADE_VERIFY(std::is_same<decltype(x), const float>::value);
    CORRADE_COMPARE(y, 32.5f);
    CORRADE_COMPARE(x, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference. The
       const operator[] returns a copy so we compare to begin() instead. */
    CORRADE_COMPARE(&y, size.begin());
    CORRADE_COMPARE(&x, size.begin() + 1);

    constexpr StridedDimensions<2, float> csize = structuredBindingsConstReferenceConstexpr({-2.25f, 32.5f});
    CORRADE_COMPARE(csize, (StridedDimensions<2, float>{32.5f, -2.25f}));
}

/* Verifies the && variant behavior with constexpr, although not really well */
constexpr StridedDimensions<2, float> structuredBindingsRvalueReferenceConstexpr(float y, float x) {
    StridedDimensions<2, float> out;
    auto&& [outY, outX] = Utility::move(out);
    outY = y;
    outX = x;
    return out;
}

void StridedDimensionsCpp17Test::structuredBindingsRvalueReference() {
    StridedDimensions<2, float> size{32.5f, -2.25f};
    auto&& [y, x] = Utility::move(size);
    CORRADE_VERIFY(std::is_same<decltype(y), float>::value);
    CORRADE_VERIFY(std::is_same<decltype(x), float>::value);
    CORRADE_COMPARE(y, 32.5f);
    CORRADE_COMPARE(x, -2.25f);

    /* Verify it's indeed references and not a copy bound to a reference. The
       const operator[] returns a copy so we compare to begin() instead. */
    CORRADE_COMPARE(&y, size.begin());
    CORRADE_COMPARE(&x, size.begin() + 1);

    /* MSVC 2017 doesn't seem to like the move in a constexpr context.
       Everything else works, MSVC 2019 works as well, so just ignore it. */
    #ifndef CORRADE_MSVC2017_COMPATIBILITY
    constexpr
    #endif
    StridedDimensions<2, float> csize = structuredBindingsRvalueReferenceConstexpr(32.5f, -2.25f);
    CORRADE_COMPARE(csize, (StridedDimensions<2, float>{32.5f, -2.25f}));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedDimensionsCpp17Test)
