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

#include <sstream>

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct Rectangle {
    constexpr Rectangle(int rows, int cols): rows{rows}, cols{cols} {}

    int rows;
    int cols;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct StridedDimensionsConverter<2, std::size_t, Rectangle> {
    constexpr static Size2D from(const Rectangle& other) {
        return {std::size_t(other.rows), std::size_t(other.cols)};
    }
    constexpr static Rectangle to(const Size2D& to) {
        return {int(to[0]), int(to[1])};
    }
};

}

namespace Test { namespace {

struct StridedDimensionsTest: TestSuite::Tester {
    explicit StridedDimensionsTest();

    void constructDefault();
    void construct();
    void construct3D();
    void constructNoInit();
    void constructCopy();

    void convertScalar();
    void convertScalar3D();
    void convertExternal();
    void convertExternalStaticArrayView();

    void compare();

    void access();
    void accessInvalid();
    void accessRangeFor();
};

StridedDimensionsTest::StridedDimensionsTest() {
    addTests({&StridedDimensionsTest::constructDefault,
              &StridedDimensionsTest::construct,
              &StridedDimensionsTest::construct3D,
              &StridedDimensionsTest::constructNoInit,
              &StridedDimensionsTest::constructCopy,

              &StridedDimensionsTest::convertScalar,
              &StridedDimensionsTest::convertScalar3D,
              &StridedDimensionsTest::convertExternal,
              &StridedDimensionsTest::convertExternalStaticArrayView,

              &StridedDimensionsTest::compare,

              &StridedDimensionsTest::access,
              &StridedDimensionsTest::accessInvalid,
              &StridedDimensionsTest::accessRangeFor});
}

void StridedDimensionsTest::constructDefault() {
    Size3D a1;
    Size3D a2{Corrade::ValueInit};
    CORRADE_COMPARE(a1[0], 0);
    CORRADE_COMPARE(a1[1], 0);
    CORRADE_COMPARE(a1[2], 0);
    CORRADE_COMPARE(a2[0], 0);
    CORRADE_COMPARE(a2[1], 0);
    CORRADE_COMPARE(a2[2], 0);

    constexpr Size3D ca1;
    constexpr Size3D ca2{Corrade::ValueInit};
    CORRADE_COMPARE(ca1[0], 0);
    CORRADE_COMPARE(ca1[1], 0);
    CORRADE_COMPARE(ca1[2], 0);
    CORRADE_COMPARE(ca2[0], 0);
    CORRADE_COMPARE(ca2[1], 0);
    CORRADE_COMPARE(ca2[2], 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<Size3D>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Size3D, Corrade::ValueInitT>::value);

    /* Implicit conversion from ValueInitT not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::ValueInitT, Size3D>::value);
}

void StridedDimensionsTest::construct() {
    Size1D a = 37;
    CORRADE_COMPARE(a[0], 37);

    constexpr Size1D ca = 37;
    CORRADE_COMPARE(ca[0], 37);

    CORRADE_VERIFY(std::is_nothrow_constructible<Size1D, std::size_t>::value);
}

void StridedDimensionsTest::construct3D() {
    Size3D a = {1, 37, 4564};
    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(a[1], 37);
    CORRADE_COMPARE(a[2], 4564);

    constexpr Size3D ca = {1, 37, 4564};
    CORRADE_COMPARE(ca[0], 1);
    CORRADE_COMPARE(ca[1], 37);
    CORRADE_COMPARE(ca[2], 4564);

    CORRADE_VERIFY(std::is_nothrow_constructible<Size3D, std::size_t, std::size_t, std::size_t>::value);
}

void StridedDimensionsTest::constructNoInit() {
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 11 && __OPTIMIZE__
    CORRADE_SKIP("GCC 11+ is stupid and I can't suppress the -W[maybe-]uninitialized warnings in order to test this, skipping to achieve build log silence");
    #endif

    Size3D a{1, 37, 4564};

    /* GCC 11 misoptimizes only if I don't expect it, so I have to check every
       value this stupidly */
    new(&a)Size3D{Corrade::NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 10 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL_IF(a[0] != 1, "GCC 10+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a[0], 1);
    } {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 10 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL_IF(a[1] != 37, "GCC 10+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a[1], 37);
    } {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 10 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL_IF(a[2] != 4564, "GCC 10+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a[2], 4564);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<Size1D, Corrade::NoInitT>::value);

    /* Implicit conversion from NoInitT not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Size1D>::value);
}

void StridedDimensionsTest::constructCopy() {
    Size3D a = {1, 37, 4564};

    Size3D b = a;
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(b[1], 37);
    CORRADE_COMPARE(b[2], 4564);

    Size3D c{2, 5, 6};
    c = b;
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 37);
    CORRADE_COMPARE(c[2], 4564);

    CORRADE_VERIFY(std::is_copy_constructible<Size3D>::value);
    CORRADE_VERIFY(std::is_copy_assignable<Size3D>::value);
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Size3D>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Size3D>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Size3D>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Size3D>::value);
}

void StridedDimensionsTest::convertScalar() {
    Size1D a = 1337;
    std::size_t b = a;
    CORRADE_COMPARE(b, 1337);

    constexpr Size1D ca = 1337;
    constexpr std::size_t cb = ca;
    CORRADE_COMPARE(cb, 1337);
}

void StridedDimensionsTest::convertScalar3D() {
    /* Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<std::size_t, Size1D>::value);
    CORRADE_VERIFY(!std::is_constructible<std::size_t, Size3D>::value);
}

constexpr Size2D Sizes{34, 67};

void StridedDimensionsTest::convertExternal() {
    Size2D a{12, 37};

    Rectangle b = a;
    CORRADE_COMPARE(b.rows, 12);
    CORRADE_COMPARE(b.cols, 37);

    Size2D c = b;
    CORRADE_COMPARE(c[0], 12);
    CORRADE_COMPARE(c[1], 37);

    constexpr Rectangle cb = Sizes;
    CORRADE_COMPARE(cb.rows, 34);
    CORRADE_COMPARE(cb.cols, 67);

    constexpr Size2D cc = cb;
    CORRADE_COMPARE(cc[0], 34);
    CORRADE_COMPARE(cc[1], 67);

    CORRADE_VERIFY(std::is_nothrow_constructible<Size2D, Rectangle>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Rectangle, Size2D>::value);
}

void StridedDimensionsTest::convertExternalStaticArrayView() {
    Size2D a{12, 37};

    Containers::StaticArrayView<2, const std::size_t> b = a;
    CORRADE_COMPARE(b[0], 12);
    CORRADE_COMPARE(b[1], 37);

    Size2D c = b;
    CORRADE_COMPARE(c[0], 12);
    CORRADE_COMPARE(c[1], 37);

    constexpr Containers::StaticArrayView<2, const std::size_t> cb = Sizes;
    CORRADE_COMPARE(cb[0], 34);
    CORRADE_COMPARE(cb[1], 67);

    constexpr Size2D cc = cb;
    CORRADE_COMPARE(cc[0], 34);
    CORRADE_COMPARE(cc[1], 67);

    CORRADE_VERIFY(std::is_nothrow_constructible<Size2D, Containers::StaticArrayView<2, const std::size_t>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::StaticArrayView<2, const std::size_t>, Size2D>::value);
}

void StridedDimensionsTest::compare() {
    Size3D a{1, 37, 4564};
    Size3D b{1, 37, 4564};
    Size3D c{1, 37, 4565};

    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(!(a == c));
    CORRADE_VERIFY(a != c);
}

void StridedDimensionsTest::access() {
    Size3D a{7, 13, 29};

    CORRADE_COMPARE(*a.begin(), 7);
    CORRADE_COMPARE(*a.cbegin(), 7);
    CORRADE_COMPARE(*(a.end() - 1), 29);
    CORRADE_COMPARE(*(a.cend() - 1), 29);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cabegin = *Sizes.begin();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cacbegin = *Sizes.cbegin();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t caend = *(Sizes.end() - 1);
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* ¯\_(ツ)_/¯ */
    #endif
    std::size_t cacend = *(Sizes.cend() - 1);
    CORRADE_COMPARE(cabegin, 34);
    CORRADE_COMPARE(cacbegin, 34);
    CORRADE_COMPARE(caend, 67);
    CORRADE_COMPARE(cacend, 67);
}

void StridedDimensionsTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Size3D a{3, 12, 76};
    const Size3D ca{3, 12, 76};

    std::ostringstream out;
    Error redirectError{&out};

    a[3];
    /* To avoid sanitizers getting angry */
    reinterpret_cast<const Size2D&>(ca)[2];

    CORRADE_COMPARE(out.str(),
        "Containers::StridedDimensions::operator[](): dimension 3 out of range for 3 dimensions\n"
        "Containers::StridedDimensions::operator[](): dimension 2 out of range for 2 dimensions\n");
}

void StridedDimensionsTest::accessRangeFor() {
    Size3D a{6, 12, 28};
    for(std::size_t& i: a) ++i;
    CORRADE_COMPARE(a, (Size3D{7, 13, 29}));

    const Size3D ca = a;
    std::size_t sum = 1;
    for(std::size_t i: ca) sum *= i;
    CORRADE_COMPARE(sum, 29*13*7);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedDimensionsTest)
