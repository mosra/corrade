/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

struct ArrayViewTest: TestSuite::Tester {
    explicit ArrayViewTest();

    void constructEmpty();
    void constructNullptr();
    void constructNullptrSize();
    void construct();
    void constructFixedSize();
    void constructDerived();
    void constructConst();
    void constructVoid();

    void convertBool();
    void convertPointer();
    void convertConst();
    void convertVoid();

    void emptyCheck();
    void access();
    void rangeBasedFor();

    void sliceInvalid();
    void sliceNullptr();
    void slice();
    void sliceToStatic();
};

typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<const void> VoidArrayView;

ArrayViewTest::ArrayViewTest() {
    addTests({&ArrayViewTest::constructEmpty,
              &ArrayViewTest::constructNullptr,
              &ArrayViewTest::constructNullptrSize,
              &ArrayViewTest::construct,
              &ArrayViewTest::constructFixedSize,
              &ArrayViewTest::constructDerived,
              &ArrayViewTest::constructConst,
              &ArrayViewTest::constructVoid,

              &ArrayViewTest::convertBool,
              &ArrayViewTest::convertPointer,
              &ArrayViewTest::convertConst,
              &ArrayViewTest::convertVoid,

              &ArrayViewTest::emptyCheck,
              &ArrayViewTest::access,
              &ArrayViewTest::rangeBasedFor,

              &ArrayViewTest::sliceInvalid,
              &ArrayViewTest::sliceNullptr,
              &ArrayViewTest::slice,
              &ArrayViewTest::sliceToStatic});
}

void ArrayViewTest::constructEmpty() {
    const ArrayView a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayViewTest::constructNullptr() {
    const ArrayView a(nullptr);
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, ArrayView>::value));
}

void ArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */
    const ArrayView a(nullptr, 5);
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 5);
}

void ArrayViewTest::construct() {
    int a[30];

    const ArrayView b = {a, 20};
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 20);
}

void ArrayViewTest::constructFixedSize() {
    int a[13];

    const ArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 13);
}

void ArrayViewTest::constructDerived() {
    struct A { int i; };
    struct B: A {};

    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    CORRADE_VERIFY((std::is_convertible<B(&)[5], Containers::ArrayView<A>>::value));

    {
        CORRADE_EXPECT_FAIL("Intentionally not forbidding construction of base array from larger derived type to stay compatible with raw arrays");

        struct C: A { int b; };

        /* Array of 5 Cs has larger size than array of 5 As so it does not make
           sense to create the view from it, but we are keeping compatibility with
           raw arrays and thus allow the users to shoot themselves in a foot. */

        CORRADE_VERIFY(!(std::is_convertible<C(&)[5], Containers::ArrayView<A>>::value));
    }
}

void ArrayViewTest::constructConst() {
    const int a[] = {3, 4, 7, 12, 0, -15};

    ConstArrayView b = a;
    CORRADE_COMPARE(b.size(), 6);
    CORRADE_COMPARE(b[2], 7);
}

void ArrayViewTest::constructVoid() {
    void* a = reinterpret_cast<void*>(0xdeadbeef);
    VoidArrayView b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(0xdeadbeef);
    VoidArrayView d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_COMPARE(d.size(), 100);
}

void ArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(ArrayView(a));
    CORRADE_VERIFY(!ArrayView());
    CORRADE_VERIFY(VoidArrayView(a));
    CORRADE_VERIFY(!VoidArrayView());
    CORRADE_VERIFY(!(std::is_convertible<ArrayView, int>::value));
    CORRADE_VERIFY(!(std::is_convertible<VoidArrayView, int>::value));
}

void ArrayViewTest::convertPointer() {
    int a[7];
    ArrayView b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const ArrayView c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    const VoidArrayView d = a;
    const void* dp = d;
    CORRADE_COMPARE(dp, static_cast<const void*>(a));

    /* Pointer arithmetic */
    const ArrayView e = a;
    const int* ep = e + 2;
    CORRADE_COMPARE(ep, &e[2]);
}

void ArrayViewTest::convertConst() {
    int a[3];
    ArrayView b = a;
    ConstArrayView c = b;
    CORRADE_VERIFY(c == a);
    CORRADE_COMPARE(c.size(), 3);
}

void ArrayViewTest::convertVoid() {
    int a[] = {3, 4, 7, 12, 0, -15};

    /** @todo C++14: test that all the operations are really constexpr (C++11 doesn't allow void conversions IMHO) */

    /* void reference to compile-time array */
    VoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 6*sizeof(int));

    /* void reference to runtime array */
    VoidArrayView c = {a, 6};
    CORRADE_VERIFY(c == a);
    CORRADE_COMPARE(c.size(), 6*sizeof(int));

    /* void reference to ArrayView */
    ArrayView f = a;
    VoidArrayView g = f;
    CORRADE_VERIFY(g == f);
    CORRADE_COMPARE(g.size(), f.size()*sizeof(int));
}

void ArrayViewTest::emptyCheck() {
    ArrayView a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    int b[5];
    ArrayView c = {b, 5};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());
}

void ArrayViewTest::access() {
    int a[7];
    ArrayView b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), b.size());

    Containers::ArrayView<const int> c = a;
    CORRADE_COMPARE(c.data(), a);
}

void ArrayViewTest::rangeBasedFor() {
    int a[5];
    ArrayView b = a;
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

void ArrayViewTest::sliceInvalid() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    std::ostringstream out;
    Error redirectError{&out};

    a.slice(a - 1, a);
    a.slice(a + 5, a + 6);
    a.slice(a + 2, a + 1);
    a.slice<5>(1);

    CORRADE_COMPARE(out.str(), "Containers::ArrayView::slice(): slice out of range\n"
                               "Containers::ArrayView::slice(): slice out of range\n"
                               "Containers::ArrayView::slice(): slice out of range\n"
                               "Containers::ArrayView::slice(): slice out of range\n");
}

void ArrayViewTest::sliceNullptr() {
    ArrayView a{nullptr, 5};

    ArrayView b = a.prefix(nullptr);
    CORRADE_VERIFY(!b);
    CORRADE_COMPARE(b.size(), 0);

    ArrayView c = a.suffix(nullptr);
    CORRADE_VERIFY(!c);
    CORRADE_COMPARE(c.size(), 5);

    int data[5];
    ArrayView d{data};

    ArrayView e = d.prefix(nullptr);
    CORRADE_VERIFY(!e);
    CORRADE_COMPARE(e.size(), 0);

    ArrayView f = d.suffix(nullptr);
    CORRADE_VERIFY(!f);
    CORRADE_COMPARE(f.size(), 0);
}

void ArrayViewTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    ArrayView b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayView c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);
}

void ArrayViewTest::sliceToStatic() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    StaticArrayView<3, int> b = a.slice<3>(1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewTest)
