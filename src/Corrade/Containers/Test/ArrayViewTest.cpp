/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

    void cast();
    void size();
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
              &ArrayViewTest::sliceToStatic,

              &ArrayViewTest::cast,
              &ArrayViewTest::size});
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

    {
        const ArrayView b = {a, 20};
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 20);
    } {
        const auto b = arrayView(a, 20);
        CORRADE_VERIFY((std::is_same<decltype(b), const ArrayView>::value));
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 20);
    }
}

void ArrayViewTest::constructFixedSize() {
    int a[13];

    {
        const ArrayView b = a;
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    } {
        const auto b = arrayView(a);
        CORRADE_VERIFY((std::is_same<decltype(b), const ArrayView>::value));
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    }
}

void ArrayViewTest::constructDerived() {
    struct A { int i; };
    struct B: A {};

    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    B b[5];
    Containers::ArrayView<B> bv{b};

    Containers::ArrayView<A> a{b};
    Containers::ArrayView<A> av{bv};

    CORRADE_VERIFY(a == &b[0]);
    CORRADE_VERIFY(av == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);
}

void ArrayViewTest::constructConst() {
    const int a[] = {3, 4, 7, 12, 0, -15};

    {
        ConstArrayView b = a;
        CORRADE_COMPARE(b.size(), 6);
        CORRADE_COMPARE(b[2], 7);
    } {
        ConstArrayView b = a;
        CORRADE_VERIFY((std::is_same<decltype(b), ConstArrayView>::value));
        CORRADE_COMPARE(b.size(), 6);
        CORRADE_COMPARE(b[2], 7);
    }
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
    const ArrayView cf = a;
    VoidArrayView g = f;
    VoidArrayView cg = cf;
    CORRADE_VERIFY(g == f);
    CORRADE_VERIFY(cg == cf);
    CORRADE_COMPARE(g.size(), f.size()*sizeof(int));
    CORRADE_COMPARE(cg.size(), cf.size()*sizeof(int));
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
    CORRADE_COMPARE(b.cbegin(), b.begin());
    CORRADE_COMPARE(b.cend(), b.end());

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

    CORRADE_COMPARE(out.str(),
        "Containers::ArrayView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n");
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

void ArrayViewTest::cast() {
    std::uint32_t data[6]{};
    Containers::ArrayView<std::uint32_t> a = data;
    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto c = Containers::arrayCast<std::uint16_t>(a);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::ArrayView<std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(c), Containers::ArrayView<std::uint16_t>>::value));
    CORRADE_COMPARE(reinterpret_cast<void*>(b.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<void*>(c.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(a.size(), 6);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(c.size(), 12);
}

void ArrayViewTest::size() {
    int a[6]{};
    ArrayView b{a, 3};
    VoidArrayView c{a};

    CORRADE_COMPARE(Containers::arraySize(a), 6);
    CORRADE_COMPARE(Containers::arraySize(b), 3);
    CORRADE_COMPARE(Containers::arraySize(c), 24);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewTest)
