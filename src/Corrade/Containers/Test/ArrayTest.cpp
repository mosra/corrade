/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

struct ArrayTest: TestSuite::Tester {
    explicit ArrayTest();

    void constructEmpty();
    void constructNullptr();
    void constructDefaultInit();
    void constructValueInit();
    void constructNoInit();
    void constructDirectInit();
    void construct();
    void constructFromExisting();
    void constructZeroSize();
    void constructMove();
    void constructFrom();
    void constructFromChar();

    void boolConversion();
    void pointerConversion();

    void emptyCheck();
    void access();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void release();

    void customDeleter();
    void customDeleterType();
};

typedef Containers::Array<int> Array;

ArrayTest::ArrayTest() {
    addTests({&ArrayTest::constructEmpty,
              &ArrayTest::constructNullptr,
              &ArrayTest::constructDefaultInit,
              &ArrayTest::constructValueInit,
              &ArrayTest::constructNoInit,
              &ArrayTest::constructDirectInit,
              &ArrayTest::construct,
              &ArrayTest::constructFromExisting,
              &ArrayTest::constructZeroSize,
              &ArrayTest::constructMove,
              &ArrayTest::constructFrom,
              &ArrayTest::constructFromChar,

              &ArrayTest::boolConversion,
              &ArrayTest::pointerConversion,

              &ArrayTest::emptyCheck,
              &ArrayTest::access,
              &ArrayTest::rvalueArrayAccess,
              &ArrayTest::rangeBasedFor,

              &ArrayTest::slice,
              &ArrayTest::release,

              &ArrayTest::customDeleter,
              &ArrayTest::customDeleterType});
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
    const Array c(nullptr);
    CORRADE_VERIFY(c == nullptr);
    CORRADE_COMPARE(c.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, Array>::value));
}

void ArrayTest::construct() {
    const Array a(5);
    CORRADE_VERIFY(a != nullptr);
    CORRADE_COMPARE(a.size(), 5);

    /* Implicit construction from std::size_t is not allowed */
    CORRADE_VERIFY(!(std::is_convertible<std::size_t, Array>::value));
}

void ArrayTest::constructFromExisting() {
    int* a = new int[25];
    Array b{a, 25};
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 25);
}

void ArrayTest::constructDefaultInit() {
    const Array a{DefaultInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
}

void ArrayTest::constructValueInit() {
    const Array a{ValueInit, 2};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
}

namespace {
    struct Foo {
        static int constructorCallCount;
        Foo() { ++constructorCallCount; }
    };

    int Foo::constructorCallCount = 0;
}

void ArrayTest::constructNoInit() {
    const Containers::Array<Foo> a{NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(Foo::constructorCallCount, 0);

    const Containers::Array<Foo> b{DefaultInit, 7};
    CORRADE_COMPARE(Foo::constructorCallCount, 7);
}

void ArrayTest::constructDirectInit() {
    const Array a{DirectInit, 2, -37};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], -37);
    CORRADE_COMPARE(a[1], -37);
}

void ArrayTest::constructZeroSize() {
    const Array a(0);

    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);
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

void ArrayTest::constructFromChar() {
    /* Verify that this compiles without "narrowing from int to char" errors */
    const auto a = Containers::Array<char>::from(0x11, 0x22, 0x33);
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a[1], 0x22);
}

void ArrayTest::boolConversion() {
    CORRADE_VERIFY(Array(2));
    CORRADE_VERIFY(!Array());
    CORRADE_VERIFY(!(std::is_convertible<Array, int>::value));
}

void ArrayTest::pointerConversion() {
    Array a(2);
    int* b = a;
    CORRADE_COMPARE(b, a.begin());

    const Array c(3);
    const int* d = c;
    CORRADE_COMPARE(d, c.begin());

    /* Pointer arithmetic */
    const Array e(3);
    const int* f = e + 2;
    CORRADE_COMPARE(f, &e[2]);

    /* Verify that we can't convert rvalues */
    CORRADE_VERIFY((std::is_convertible<Array&, int*>::value));
    CORRADE_VERIFY((std::is_convertible<const Array&, const int*>::value));
    {
        #ifdef CORRADE_GCC47_COMPATIBILITY
        CORRADE_EXPECT_FAIL("Rvalue references for *this are not supported in GCC < 4.8.1.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<Array, int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<Array&&, int*>::value));
    }

    /* Deleting const&& overload and leaving only const& one will not, in fact,
       disable conversion of const Array&& to pointer, but rather make the
       conversion ambiguous, which is not what we want, as it breaks e.g.
       rvalueArrayAccess() test. */
    {
        CORRADE_EXPECT_FAIL("I don't know how to properly disable conversion of const Array&& to pointer.");
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

    const auto b = Array::from(7, 3, 5, 4);
    CORRADE_COMPARE(b.data(), static_cast<const int*>(b));
    CORRADE_COMPARE(b[2], 5);
}

void ArrayTest::rvalueArrayAccess() {
    CORRADE_COMPARE(Array::from(1, 2, 3, 4)[2], 3);
}

void ArrayTest::rangeBasedFor() {
    Array a(5);
    for(auto& i: a)
        i = 3;

    CORRADE_COMPARE(a[0], 3);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 3);
}

void ArrayTest::slice() {
    Array a = Array::from(1, 2, 3, 4, 5);
    const Array ac = Array::from(1, 2, 3, 4, 5);

    ArrayView<int> b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayView<const int> bc = ac.slice(1, 4);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);

    ArrayView<int> c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView<const int> cc = ac.prefix(3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    ArrayView<int> d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ArrayView<const int> dc = ac.suffix(2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);
}

void ArrayTest::release() {
    Array a(5);
    int* const data = a;
    int* const released = a.release();
    delete[] released;

    CORRADE_COMPARE(data, released);
    CORRADE_COMPARE(a.begin(), nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

namespace {
    int CustomDeleterDeletedCount = 0;
}

void ArrayTest::customDeleter() {
    int data[25]{};

    {
        Array a{data, 25, [](int*, std::size_t size) { CustomDeleterDeletedCount = size; }};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterDeletedCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterDeletedCount, 25);
}

namespace {
    struct CustomDeleter {
        CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
        void operator()(int*, std::size_t size) { deletedCount = size; }
        int& deletedCount;
    };
}

void ArrayTest::customDeleterType() {
    int data[25]{};
    int deletedCount = 0;

    {
        Containers::Array<int, CustomDeleter> a{data, 25, CustomDeleter{deletedCount}};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(deletedCount, 0);
    }

    CORRADE_COMPARE(deletedCount, 25);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayTest)
