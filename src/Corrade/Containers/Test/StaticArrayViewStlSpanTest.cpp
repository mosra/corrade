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

#if __has_include(<span>)
#include "Corrade/Containers/ArrayViewStlSpan.h"
#endif
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct StaticArrayViewStlSpanTest: TestSuite::Tester {
    explicit StaticArrayViewStlSpanTest();

    void convertFromSpan();
    void convertToSpan();
    void convertToSpanEmpty();
    void convertConstFromSpan();
    void convertToConstSpan();
    void convertToConstSpanEmpty();

    void convertFromSpanDerived();
    void convertConstFromSpanDerived();
    /* So far I don't implement this for the other direction, as the use case
       of people wanting to pass STL things to Corrade is far bigger than the
       use case of people wanting to feed Corrade to APIs taking std::span.
       Plus I'm not sure if such behavior would even be expected / desired in
       the STL world. */

    void convertSpanSized();
    void convertSpanSizedEmpty();
    void convertConstFromSpanSized();
    void convertConstFromSpanSizedEmpty();
    void convertToConstSpanSized();
    void convertToConstSpanSizedEmpty();

    void convertFromSpanSizedDerived();
    void convertConstFromSpanSizedDerived();
    /* So far I don't implement this for the other direction, see above */
};

StaticArrayViewStlSpanTest::StaticArrayViewStlSpanTest() {
    addTests({&StaticArrayViewStlSpanTest::convertFromSpan,
              &StaticArrayViewStlSpanTest::convertToSpan,
              &StaticArrayViewStlSpanTest::convertToSpanEmpty,
              &StaticArrayViewStlSpanTest::convertConstFromSpan,
              &StaticArrayViewStlSpanTest::convertToConstSpan,
              &StaticArrayViewStlSpanTest::convertToConstSpanEmpty,

              &StaticArrayViewStlSpanTest::convertFromSpanDerived,
              &StaticArrayViewStlSpanTest::convertConstFromSpanDerived,

              &StaticArrayViewStlSpanTest::convertSpanSized,
              &StaticArrayViewStlSpanTest::convertSpanSizedEmpty,
              &StaticArrayViewStlSpanTest::convertConstFromSpanSized,
              &StaticArrayViewStlSpanTest::convertConstFromSpanSizedEmpty,
              &StaticArrayViewStlSpanTest::convertToConstSpanSized,
              &StaticArrayViewStlSpanTest::convertToConstSpanSizedEmpty,

              &StaticArrayViewStlSpanTest::convertFromSpanSizedDerived,
              &StaticArrayViewStlSpanTest::convertConstFromSpanSizedDerived});
}

void StaticArrayViewStlSpanTest::convertFromSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else

    /* Uncomment this to see the ErasedStaticArrayViewConverter error
       (which isn't testable with is_constructible below). Should be just a
       single message about "no matching function", not a cascade of errors
       with size too large and such. */
    #if 0
    staticArrayView(std::span<float>{});
    #endif

    /* Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, float>, std::span<float, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, float>, std::span<float>>::value);
    #endif
}

#if __has_include(<span>)
constexpr float Data[]{42.0f, 13.37f, -25.0f};
#endif

void StaticArrayViewStlSpanTest::convertToSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    StaticArrayView<3, float> a = data;
    CORRADE_COMPARE(a.data(), +data);
    CORRADE_COMPARE(a[0], 42.0f);

    std::span<float> b = a;
    CORRADE_COMPARE(b.data(), +data);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    constexpr StaticArrayView<3, const float> ca = Data;
    CORRADE_COMPARE(ca.data(), +Data);
    CORRADE_COMPARE(ca[0], 42.0f);

    constexpr std::span<const float> cb = ca;
    CORRADE_COMPARE(cb.data(), +Data);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different type is correctly not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<std::span<int>, const Containers::StaticArrayView<5, int>&>::value);
    CORRADE_VERIFY(!std::is_constructible<std::span<float>, const Containers::StaticArrayView<5, int>&>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertToSpanEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlSpanTest::convertConstFromSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    /* Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, const float>, std::span<float, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const float>, std::span<float>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertToConstSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    StaticArrayView<3, float> a = data;
    CORRADE_COMPARE(a.data(), +data);
    CORRADE_COMPARE(a[0], 42.0f);

    std::span<const float> b = a;
    CORRADE_COMPARE(b.data(), +data);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different type is correctly not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<std::span<const int>, Containers::StaticArrayView<5, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<std::span<const float>, Containers::StaticArrayView<5, int>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertToConstSpanEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

struct Base {
    float a;
};
struct Derived: Base {};
struct DerivedDifferentSize: Base {
    int b;
};

void StaticArrayViewStlSpanTest::convertFromSpanDerived() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    /* Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, Base>, std::span<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, std::span<Derived>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertConstFromSpanDerived() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    /* Not using is_convertible to catch also accidental explicit
       conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, const Base>, std::span<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const Base>, std::span<Derived>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float, 3> a = data;
    CORRADE_COMPARE(a.data(), +data);
    CORRADE_COMPARE(a[0], 42.0f);

    StaticArrayView<3, float> b = a;
    CORRADE_COMPARE(b.data(), +data);
    CORRADE_COMPARE(b[0], 42.0f);

    std::span<float, 3> c = b;
    CORRADE_COMPARE(c.data(), +data);
    CORRADE_COMPARE(c[0], 42.0f);

    auto d = staticArrayView(c);
    CORRADE_VERIFY(std::is_same<decltype(d), StaticArrayView<3, float>>::value);
    CORRADE_COMPARE(d.data(), +data);
    CORRADE_COMPARE(d[0], 42.0f);

    constexpr std::span<const float, 3> ca = Data;
    CORRADE_COMPARE(ca.data(), +Data);
    CORRADE_COMPARE(ca[0], 42.0f);

    constexpr StaticArrayView<3, const float> cb = ca;
    CORRADE_COMPARE(cb.data(), +Data);
    CORRADE_COMPARE(cb[0], 42.0f);

    constexpr std::span<const float, 3> cc = cb;
    CORRADE_COMPARE(cc.data(), +Data);
    CORRADE_COMPARE(cc[0], 42.0f);

    constexpr auto cd = staticArrayView(cc);
    CORRADE_VERIFY(std::is_same<decltype(cd), const StaticArrayView<3, const float>>::value);
    CORRADE_COMPARE(cd.data(), +Data);
    CORRADE_COMPARE(cd[0], 42.0f);

    /* Conversion from a different size/type not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::StaticArrayView<5, int>, std::span<int, 5>>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StaticArrayView<6, int>, std::span<int, 5>>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StaticArrayView<5, float>, std::span<int, 5>>::value);

    /* Because we're using builtin std::span conversion constructor here, check
       that implicit conversion to a different size or type is correctly not
       allowed. Explicit conversion is allowed however, so is_constructible
       would pass here. */
    CORRADE_VERIFY(std::is_convertible<Containers::StaticArrayView<5, int>, std::span<int, 5>>::value);
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!std::is_convertible<Containers::StaticArrayView<5, int>, std::span<int, 6>>::value);
    }
    CORRADE_VERIFY(!std::is_convertible<Containers::StaticArrayView<5, int>, std::span<float, 5>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertSpanSizedEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlSpanTest::convertConstFromSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float, 3> a = data;
    CORRADE_COMPARE(a.data(), +data);
    CORRADE_COMPARE(a[0], 42.0f);

    StaticArrayView<3, const float> b = a;
    CORRADE_COMPARE(b.data(), +data);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Conversion from a different size/type not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::StaticArrayView<5, const int>, std::span<int, 5>>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StaticArrayView<6, const int>, std::span<int, 5>>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::StaticArrayView<5, const float>, std::span<int, 5>>::value);
    /* Creating a non-const view from a const span should not be possible
       either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<5, int>, std::span<const int, 5>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertConstFromSpanSizedEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlSpanTest::convertToConstSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    StaticArrayView<3, float> a = data;
    CORRADE_COMPARE(a.data(), +data);
    CORRADE_COMPARE(a[0], 42.0f);

    std::span<const float, 3> b = a;
    CORRADE_COMPARE(b.data(), +data);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that implicit conversion to a different size or type is correctly not
       allowed. Explicit conversion is allowed however, so is_constructible
       would pass here. */
    CORRADE_VERIFY(std::is_convertible<Containers::StaticArrayView<5, int>, std::span<const int, 5>>::value);
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!std::is_convertible<Containers::StaticArrayView<5, int>, std::span<const int, 6>>::value);
    }
    CORRADE_VERIFY(!std::is_convertible<Containers::StaticArrayView<5, int>, std::span<const float, 5>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertToConstSpanSizedEmpty() {
    CORRADE_SKIP("Zero-sized StaticArrayView is not implemented yet.");
}

void StaticArrayViewStlSpanTest::convertFromSpanSizedDerived() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Derived data[]{{{42.0f}}, {{13.3f}}, {{-25.0f}}};
    std::span<Derived, 3> a = data;

    StaticArrayView<3, Base> b = a;
    CORRADE_COMPARE(b.data(), static_cast<void*>(data));
    CORRADE_COMPARE(b[0].a, 42.0f);

    /* Conversion the other way not allowed. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, Base>, std::span<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Derived>, std::span<Base, 3>>::value);
    /* Conversion from a derived type that isn't the same size shouldn't be
       allowed either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, std::span<DerivedDifferentSize, 3>>::value);
    #endif
}

void StaticArrayViewStlSpanTest::convertConstFromSpanSizedDerived() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Derived data[]{{{42.0f}}, {{13.3f}}, {{-25.0f}}};
    std::span<Derived, 3> a = data;

    StaticArrayView<3, const Base> b = a;
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(b[0].a, 42.0f);

    /* Conversion the other way not allowed. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<StaticArrayView<3, const Base>, std::span<Derived, 3>>::value);
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const Derived>, std::span<Base, 3>>::value);
    /* Creating a non-const view from a const span should not be possible
       either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, Base>, std::span<const Derived, 3>>::value);
    /* Conversion from a derived type that isn't the same size shouldn't be
       allowed either */
    CORRADE_VERIFY(!std::is_constructible<StaticArrayView<3, const Base>, std::span<DerivedDifferentSize, 3>>::value);
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewStlSpanTest)
