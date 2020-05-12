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

#include "Corrade/Containers/Array.h"
#if __has_include(<span>)
#include "Corrade/Containers/ArrayViewStlSpan.h"
#endif
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct ArrayStlSpanTest: TestSuite::Tester {
    explicit ArrayStlSpanTest();

    void toSpan();
    void toSpanEmpty();
    void toSpanConst();
    void toSpanConstEmpty();

    void toSpanSized();
    void toSpanSizedConst();
};

ArrayStlSpanTest::ArrayStlSpanTest() {
    addTests({&ArrayStlSpanTest::toSpan,
              &ArrayStlSpanTest::toSpanEmpty,
              &ArrayStlSpanTest::toSpanConst,
              &ArrayStlSpanTest::toSpanConstEmpty,

              &ArrayStlSpanTest::toSpanSized,
              &ArrayStlSpanTest::toSpanSizedConst});
}

void ArrayStlSpanTest::toSpan() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Array<float> a{InPlaceInit, {42.0f, 13.37f, -25.0f}};

    std::span<float> b = a;
    CORRADE_COMPARE(b.data(), a.data());
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::Array<int>&, std::span<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::Array<int>&, std::span<float>>::value));
    #endif
}

void ArrayStlSpanTest::toSpanEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Array<float> a;

    std::span<float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayStlSpanTest::toSpanConst() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Array<float> a{InPlaceInit, {42.0f, 13.37f, -25.0f}};

    std::span<const float> b = a;
    CORRADE_COMPARE(b.data(), a.data());
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::Array<int>&, std::span<const int>>::value));
    CORRADE_VERIFY((std::is_convertible<const Containers::Array<int>&, std::span<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::Array<int>&, std::span<const float>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Containers::Array<int>&, std::span<const float>>::value));
    #endif
}

void ArrayStlSpanTest::toSpanConstEmpty() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    Array<float> a;

    std::span<const float> b = a;
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.size(), 0);
    #endif
}

void ArrayStlSpanTest::toSpanSized() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different size or type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<Array<float>&, std::span<float>>::value));
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<Array<float>&, std::span<float, 3>>::value));
    }
    #endif
}

void ArrayStlSpanTest::toSpanSizedConst() {
    #if !__has_include(<span>)
    CORRADE_SKIP("The <span> header is not available on this platform.");
    #else
    /* Because we're using builtin std::span conversion constructor here, check
       that conversion to a different size or type is correctly not allowed */
    CORRADE_VERIFY((std::is_convertible<Array<float>&, std::span<const float>>::value));
    CORRADE_VERIFY((std::is_convertible<const Array<float>&, std::span<const float>>::value));
    {
        #if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
        CORRADE_EXPECT_FAIL("The implicit all-catching span(Container&) constructor in libc++ < 9 causes this to be an UB instead of giving me a possibility to catch this at compile time.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<Array<float>&, std::span<const float, 3>>::value));
        CORRADE_VERIFY(!(std::is_convertible<const Array<float>&, std::span<const float, 3>>::value));
    }
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayStlSpanTest)
