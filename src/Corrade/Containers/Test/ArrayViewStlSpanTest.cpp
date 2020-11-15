/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

struct ArrayViewStlSpanTest: TestSuite::Tester {
    explicit ArrayViewStlSpanTest();

    void convertSpan();
    void convertSpanEmpty();
    void convertConstFromSpan();
    void convertConstFromSpanEmpty();
    void convertToConstSpan();
    void convertToConstSpanEmpty();
    void convertVoidFromSpan();
    void convertVoidFromSpanEmpty();
    void convertVoidFromConstSpan();
    void convertVoidFromConstSpanEmpty();

    void convertFromSpanSized();
    void convertFromSpanSizedEmpty();
    void convertToSpanSized();
    void convertConstFromSpanSized();
    void convertConstFromSpanSizedEmpty();
    void convertToConstSpanSized();
    void convertVoidFromSpanSized();
    void convertVoidFromSpanSizedEmpty();
    void convertVoidFromConstSpanSized();
    void convertVoidFromConstSpanSizedEmpty();
};

ArrayViewStlSpanTest::ArrayViewStlSpanTest() {
    addTests({&ArrayViewStlSpanTest::convertSpan,
              &ArrayViewStlSpanTest::convertSpanEmpty,
              &ArrayViewStlSpanTest::convertConstFromSpan,
              &ArrayViewStlSpanTest::convertConstFromSpanEmpty,
              &ArrayViewStlSpanTest::convertToConstSpan,
              &ArrayViewStlSpanTest::convertToConstSpanEmpty,
              &ArrayViewStlSpanTest::convertVoidFromSpan,
              &ArrayViewStlSpanTest::convertVoidFromSpanEmpty,
              &ArrayViewStlSpanTest::convertVoidFromConstSpan,
              &ArrayViewStlSpanTest::convertVoidFromConstSpanEmpty,

              &ArrayViewStlSpanTest::convertFromSpanSized,
              &ArrayViewStlSpanTest::convertFromSpanSizedEmpty,
              &ArrayViewStlSpanTest::convertToSpanSized,
              &ArrayViewStlSpanTest::convertConstFromSpanSized,
              &ArrayViewStlSpanTest::convertConstFromSpanSizedEmpty,
              &ArrayViewStlSpanTest::convertToConstSpanSized,
              &ArrayViewStlSpanTest::convertVoidFromSpanSized,
              &ArrayViewStlSpanTest::convertVoidFromSpanSizedEmpty,
              &ArrayViewStlSpanTest::convertVoidFromConstSpanSized,
              &ArrayViewStlSpanTest::convertVoidFromConstSpanSizedEmpty});
}

#if __has_include(<span>)
constexpr float Data[]{42.0f, 13.37f, -25.0f};
#endif

void ArrayViewStlSpanTest::convertSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float> a = data;

    ArrayView<float> b = a;
    CORRADE_COMPARE(b.data(), static_cast<void*>(data));
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    std::span<float> c = b;
    CORRADE_COMPARE(c.data(), static_cast<void*>(data));
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);

    auto d = arrayView(c);
    CORRADE_VERIFY((std::is_same<decltype(d), ArrayView<float>>::value));
    CORRADE_COMPARE(d.data(), static_cast<void*>(data));
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 42.0f);

    constexpr std::span<const float> ca = Data;
    constexpr ArrayView<const float> cb = ca;
    CORRADE_COMPARE(cb.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 42.0f);

    constexpr std::span<const float> cc = cb;
    CORRADE_COMPARE(cc.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 42.0f);

    constexpr auto cd = arrayView(cc);
    CORRADE_VERIFY((std::is_same<decltype(cd), const ArrayView<const float>>::value));
    CORRADE_COMPARE(cd.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cd.size(), 3);
    CORRADE_COMPARE(cd[0], 42.0f);

    /* Conversion from a different type not allowed */
    CORRADE_VERIFY((std::is_convertible<std::span<int>, Containers::ArrayView<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<std::span<int>, Containers::ArrayView<float>>::value));

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different size or type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<const Containers::ArrayView<int>&, std::span<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Containers::ArrayView<int>&, std::span<float>>::value));
    #endif
}

void ArrayViewStlSpanTest::convertSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float> a;
    ArrayView<float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);

    std::span<float> c = b;
    CORRADE_COMPARE(c.data(), nullptr);
    CORRADE_COMPARE(c.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertConstFromSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float> a = data;

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Conversion from a different type not allowed */
    CORRADE_VERIFY((std::is_convertible<std::span<int>, Containers::ArrayView<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<std::span<int>, Containers::ArrayView<const float>>::value));
    #endif
}

void ArrayViewStlSpanTest::convertConstFromSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float> a;

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertToConstSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    ArrayView<float> a = data;

    std::span<const float> b = a;
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different size or type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::ArrayView<int>, std::span<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::ArrayView<int>, std::span<const float>>::value));
    #endif
}

void ArrayViewStlSpanTest::convertToConstSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    ArrayView<float> a;

    std::span<const float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float> a = data;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3*4);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float> a;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromConstSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    const float data[]{42.0f, 13.37f, -25.0f};
    std::span<const float> a = data;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3*4);

    constexpr std::span<const float> ca = Data;
    constexpr ArrayView<const void> cb = ca;
    CORRADE_COMPARE(cb.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cb.size(), 3*4);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromConstSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<const float> a;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertFromSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float, 3> a = data;

    ArrayView<float> b = a;
    CORRADE_COMPARE(b.data(), static_cast<void*>(data));
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = arrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(b), ArrayView<float>>::value));
    CORRADE_COMPARE(c.data(), static_cast<void*>(data));
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);

    constexpr std::span<const float, 3> ca = Data;
    constexpr ArrayView<const float> cb = ca;
    CORRADE_COMPARE(cb.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 42.0f);

    constexpr auto cc = arrayView(ca);
    CORRADE_VERIFY((std::is_same<decltype(cb), const ArrayView<const float>>::value));
    CORRADE_COMPARE(cc.data(), static_cast<const void*>(Data));
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 42.0f);

    /* Conversion from a different type not allowed */
    CORRADE_VERIFY((std::is_convertible<std::span<int, 37>, Containers::ArrayView<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<std::span<int, 37>, Containers::ArrayView<float>>::value));
    #endif
}

void ArrayViewStlSpanTest::convertFromSpanSizedEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float, 0> a;

    ArrayView<float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertToSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    CORRADE_VERIFY((std::is_convertible<ArrayView<float>&, std::span<float>>::value));
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<ArrayView<float>&, std::span<float, 3>>::value));
    }
    #endif
}

void ArrayViewStlSpanTest::convertConstFromSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float, 3> a = data;

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Conversion from a different type not allowed */
    CORRADE_VERIFY((std::is_convertible<std::span<int, 37>, Containers::ArrayView<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<std::span<int, 37>, Containers::ArrayView<const float>>::value));
    #endif
}

void ArrayViewStlSpanTest::convertConstFromSpanSizedEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float, 0> a;

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertToConstSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    CORRADE_VERIFY((std::is_convertible<ArrayView<float>&, std::span<const float>>::value));
    CORRADE_VERIFY((std::is_convertible<ArrayView<const float>&, std::span<const float>>::value));
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<ArrayView<float>&, std::span<const float, 3>>::value));
        CORRADE_VERIFY(!(std::is_convertible<ArrayView<const float>&, std::span<const float, 3>>::value));
    }
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    float data[]{42.0f, 13.37f, -25.0f};
    std::span<float, 3> a = data;

    ArrayView<void> b = a;
    CORRADE_COMPARE(b, static_cast<void*>(data));
    CORRADE_COMPARE(b.size(), 3*4);

    ArrayView<const void> cb = a;
    CORRADE_COMPARE(cb, static_cast<const void*>(data));
    CORRADE_COMPARE(cb.size(), 3*4);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromSpanSizedEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<float, 0> a;

    ArrayView<void> b = a;
    CORRADE_COMPARE(b, nullptr);
    CORRADE_COMPARE(b.size(), 0);

    ArrayView<const void> cb = a;
    CORRADE_COMPARE(cb, nullptr);
    CORRADE_COMPARE(cb.size(), 0);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromConstSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    const float data[]{42.0f, 13.37f, -25.0f};
    std::span<const float, 3> a = data;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b, static_cast<const void*>(data));
    CORRADE_COMPARE(b.size(), 3*4);

    constexpr std::span<const float, 3> ca = Data;
    constexpr ArrayView<const void> cb = ca;
    CORRADE_COMPARE(cb, static_cast<const void*>(Data));
    CORRADE_COMPARE(cb.size(), 3*4);
    #endif
}

void ArrayViewStlSpanTest::convertVoidFromConstSpanSizedEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    std::span<const float, 0> a;

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b, nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewStlSpanTest)
