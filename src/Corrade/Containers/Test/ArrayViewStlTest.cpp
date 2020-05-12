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

#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct ArrayViewStlTest: TestSuite::Tester {
    explicit ArrayViewStlTest();

    void convertFromArray();
    void convertFromArrayEmpty();
    void convertFromConstArray();
    void convertFromConstArrayEmpty();
    void convertConstFromArray();
    void convertConstFromArrayEmpty();
    void convertVoidFromArray();
    void convertVoidFromArrayEmpty();
    void convertVoidFromConstArray();
    void convertVoidFromConstArrayEmpty();

    void convertFromVector();
    void convertFromVectorEmpty();
    void convertFromConstVector();
    void convertFromConstVectorEmpty();
    void convertConstFromVector();
    void convertConstFromVectorEmpty();
    void convertVoidFromVector();
    void convertVoidFromVectorEmpty();
    void convertVoidFromConstVector();
    void convertVoidFromConstVectorEmpty();
};

ArrayViewStlTest::ArrayViewStlTest() {
    addTests({&ArrayViewStlTest::convertFromArray,
              &ArrayViewStlTest::convertFromArrayEmpty,
              &ArrayViewStlTest::convertFromConstArray,
              &ArrayViewStlTest::convertFromConstArrayEmpty,
              &ArrayViewStlTest::convertConstFromArray,
              &ArrayViewStlTest::convertConstFromArrayEmpty,
              &ArrayViewStlTest::convertVoidFromArray,
              &ArrayViewStlTest::convertVoidFromArrayEmpty,
              &ArrayViewStlTest::convertVoidFromConstArray,
              &ArrayViewStlTest::convertVoidFromConstArrayEmpty,

              &ArrayViewStlTest::convertFromVector,
              &ArrayViewStlTest::convertFromVectorEmpty,
              &ArrayViewStlTest::convertFromConstVector,
              &ArrayViewStlTest::convertFromConstVectorEmpty,
              &ArrayViewStlTest::convertConstFromVector,
              &ArrayViewStlTest::convertConstFromVectorEmpty,
              &ArrayViewStlTest::convertVoidFromVector,
              &ArrayViewStlTest::convertVoidFromVectorEmpty,
              &ArrayViewStlTest::convertVoidFromConstVector,
              &ArrayViewStlTest::convertVoidFromConstVectorEmpty});
}

void ArrayViewStlTest::convertFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    ArrayView<float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = arrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(c), ArrayView<float>>::value));
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);

    auto d = arrayView(std::array<float, 3>{});
    CORRADE_VERIFY((std::is_same<decltype(d), ArrayView<float>>::value));
    CORRADE_COMPARE(c.size(), 3);
    /* The rest is a dangling pointer, can't test */
}

void ArrayViewStlTest::convertFromArrayEmpty() {
    /* GCC 4.8 complains about missing initializers when {} is used here */
    std::array<float, 0> a;
    ArrayView<float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertFromConstArray() {
    const std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = arrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(c), ArrayView<const float>>::value));
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);
}

void ArrayViewStlTest::convertFromConstArrayEmpty() {
    /* GCC 4.8 complains about missing initializers when {} is used here,
       however Clang on libc++ doesn't allow the opposite. */
    const std::array<float, 0> a
        #if !(defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5)
        {}
        #endif
        ;
    ArrayView<const float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertConstFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);
}

void ArrayViewStlTest::convertConstFromArrayEmpty() {
    /* GCC 4.8 complains about missing initializers when {} is used here */
    std::array<float, 0> a;
    ArrayView<const float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertVoidFromArray() {
    std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};
    const std::array<float, 3> ca{{42.0f, 13.37f, -25.0f}};

    ArrayView<void> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b.size(), 3*4);

    ArrayView<const void> cb = a;
    CORRADE_COMPARE(cb.data(), &a[0]);
    CORRADE_COMPARE(cb.size(), 3*4);

    ArrayView<const void> c = ca;
    CORRADE_COMPARE(c.data(), &ca[0]);
    CORRADE_COMPARE(c.size(), 3*4);
}

void ArrayViewStlTest::convertVoidFromArrayEmpty() {
    /* GCC 4.8 complains about missing initializers when {} is used here */
    std::array<float, 0> a;
    ArrayView<const void> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertVoidFromConstArray() {
    const std::array<float, 3> a{{42.0f, 13.37f, -25.0f}};

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b.size(), 3*4);
}

void ArrayViewStlTest::convertVoidFromConstArrayEmpty() {
    /* GCC 4.8 complains about missing initializers when {} is used here,
       however Clang on libc++ doesn't allow the opposite. */
    const std::array<float, 0> a
        #if !(defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5)
        {}
        #endif
        ;
    ArrayView<const void> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertFromVector() {
    std::vector<float> a{42.0f, 13.37f, -25.0f};

    ArrayView<float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = arrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(c), ArrayView<float>>::value));
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);

    auto d = arrayView(std::vector<float>(3));
    CORRADE_VERIFY((std::is_same<decltype(d), ArrayView<float>>::value));
    CORRADE_COMPARE(c.size(), 3);
    /* The rest is a dangling pointer, can't test */
}

void ArrayViewStlTest::convertFromVectorEmpty() {
    std::vector<float> a;
    ArrayView<float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertFromConstVector() {
    const std::vector<float> a{42.0f, 13.37f, -25.0f};

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);

    auto c = arrayView(a);
    CORRADE_VERIFY((std::is_same<decltype(c), ArrayView<const float>>::value));
    CORRADE_COMPARE(c, &a[0]);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 42.0f);
}

void ArrayViewStlTest::convertFromConstVectorEmpty() {
    const std::vector<float> a;
    ArrayView<const float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertConstFromVector() {
    std::vector<float> a{42.0f, 13.37f, -25.0f};

    ArrayView<const float> b = a;
    CORRADE_COMPARE(b, &a[0]);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 42.0f);
}

void ArrayViewStlTest::convertConstFromVectorEmpty() {
    std::vector<float> a;
    ArrayView<const float> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayViewStlTest::convertVoidFromVector() {
    std::vector<float> a{42.0f, 13.37f, -25.0f};

    ArrayView<void> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b.size(), 3*4);

    ArrayView<const void> cb = a;
    CORRADE_COMPARE(cb.data(), &a[0]);
    CORRADE_COMPARE(cb.size(), 3*4);
}

void ArrayViewStlTest::convertVoidFromVectorEmpty() {
    std::vector<float> a;

    ArrayView<const void> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);

    ArrayView<const void> cb = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(cb.size(), 0);
}

void ArrayViewStlTest::convertVoidFromConstVector() {
    const std::vector<float> a{42.0f, 13.37f, -25.0f};

    ArrayView<const void> b = a;
    CORRADE_COMPARE(b.data(), &a[0]);
    CORRADE_COMPARE(b.size(), 3*4);
}

void ArrayViewStlTest::convertVoidFromConstVectorEmpty() {
    std::vector<float> a;

    ArrayView<const void> b = a;
    /* If size() is 0, data() may or may not return a null pointer. So can't
       test shit. FFS, C++. */
    CORRADE_COMPARE(b.size(), 0);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewStlTest)
