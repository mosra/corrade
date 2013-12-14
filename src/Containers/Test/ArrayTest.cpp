/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "Containers/Array.h"
#include "TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

class ArrayTest: public TestSuite::Tester {
    public:
        explicit ArrayTest();

        void constructEmpty();
        void constructNullptr();
        void construct();
        void constructMove();
        void constructFrom();
        void constructZeroInitialized();

        void boolConversion();
        void pointerConversion();

        void emptyCheck();
        void access();
        void rangeBasedFor();
        void release();
};

typedef Containers::Array<int> Array;

ArrayTest::ArrayTest() {
    addTests<ArrayTest>({&ArrayTest::constructEmpty,
              &ArrayTest::constructNullptr,
              &ArrayTest::construct,
              &ArrayTest::constructMove,
              &ArrayTest::constructFrom,
              &ArrayTest::constructZeroInitialized,

              &ArrayTest::boolConversion,
              &ArrayTest::pointerConversion,

              &ArrayTest::emptyCheck,
              &ArrayTest::access,
              &ArrayTest::rangeBasedFor,
              &ArrayTest::release});
}

void ArrayTest::constructEmpty() {
    const Array a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);

    /* Zero-length should not call new */
    const std::size_t size = 0;
    const Array b(size);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayTest::constructNullptr() {
    #if defined(CORRADE_GCC45_COMPATIBILITY) || defined(CORRADE_MSVC2013_COMPATIBILITY)
    CORRADE_SKIP("Nullptr is not supported on this compiler.");
    #else
    const Array c(nullptr);
    CORRADE_VERIFY(c == nullptr);
    CORRADE_COMPARE(c.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, Array>::value));
    #endif
}

void ArrayTest::construct() {
    const Array a(5);
    CORRADE_VERIFY(a != nullptr);
    CORRADE_COMPARE(a.size(), 5);

    /* Implicit construction from std::size_t is not allowed */
    CORRADE_VERIFY(!(std::is_convertible<std::size_t, Array>::value));
}

void ArrayTest::constructMove() {
    Array a(5);
    CORRADE_VERIFY(a);
    const int* const ptr = a;

    Array b(std::move(a));
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == ptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 5);

    Array c;
    c = std::move(b);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_VERIFY(c == ptr);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(c.size(), 5);
}

void ArrayTest::constructFrom() {
    Array a = Array::from(1, 3, 127, -48);
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 4);
    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 127);
    CORRADE_COMPARE(a[3], -48);

    Array b = Array::from();
    CORRADE_VERIFY(!b);
}

void ArrayTest::constructZeroInitialized() {
    Array a = Array::zeroInitialized(2);
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
}

void ArrayTest::boolConversion() {
    CORRADE_VERIFY(Array(2));
    CORRADE_VERIFY(!Array());

    /* The conversion is explicit (i.e. no Array(2) + 7) */
    #ifdef CORRADE_GCC44_COMPATIBILITY
    CORRADE_EXPECT_FAIL("Explicit conversion operators are not supported in GCC 4.4.");
    #endif
    CORRADE_VERIFY(!(std::is_convertible<Array, int>::value));
}

void ArrayTest::pointerConversion() {
    Array a(2);
    int* b = a;
    CORRADE_COMPARE(b, a.begin());

    const Array c(3);
    const int* d = c;
    CORRADE_COMPARE(d, c.begin());

    /* Verify that we can't convert rvalues */
    CORRADE_VERIFY((std::is_convertible<Array&, int*>::value));
    CORRADE_VERIFY((std::is_convertible<const Array&, const int*>::value));
    {
        #if defined(CORRADE_GCC47_COMPATIBILITY) || defined(CORRADE_MSVC2013_COMPATIBILITY)
        CORRADE_EXPECT_FAIL("Rvalue references for *this are not supported in GCC < 4.8.1.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<Array, int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<Array&&, int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<const Array, const int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<const Array&&, const int*>::value));
    }
}

void ArrayTest::emptyCheck() {
    Array a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    Array b(5);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!b.empty());
}

void ArrayTest::access() {
    Array a(7);
    for(std::size_t i = 0; i != 7; ++i)
        a[i] = i;

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(*(a.begin()+2), 2);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a.end()-a.begin(), a.size());

    const Array b(7);
    CORRADE_COMPARE(b.data(), static_cast<const int*>(b));
}

void ArrayTest::rangeBasedFor() {
    #ifndef CORRADE_GCC45_COMPATIBILITY
    Array a(5);
    for(auto& i: a)
        i = 3;

    CORRADE_COMPARE(a[0], 3);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 3);
    #else
    CORRADE_SKIP("Range-based for is not available in GCC 4.5");
    #endif
}

void ArrayTest::release() {
    Array a(5);
    int* const data = a;
    int* const released = a.release();
    delete[] released;

    CORRADE_COMPARE(data, released);
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_COMPARE(a.begin(), nullptr);
    #else
    CORRADE_VERIFY(a.begin() == 0);
    #endif
    CORRADE_COMPARE(a.size(), 0);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayTest)
