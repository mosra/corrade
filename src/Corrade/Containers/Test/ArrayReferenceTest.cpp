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

#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

class ArrayReferenceTest: public TestSuite::Tester {
    public:
        explicit ArrayReferenceTest();

        void constructEmpty();
        void constructNullptr();
        void constructNullptrSize();
        void construct();
        void constructFixedSize();
        void constructArray();

        void boolConversion();
        void pointerConversion();

        void emptyCheck();
        void access();
        void rangeBasedFor();

        void sliceInvalid();
        void sliceNullptr();
        void slice();

        void constReference();
        void voidConstruction();
        void voidConversion();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayReference<int> ArrayReference;
typedef Containers::ArrayReference<const int> ConstArrayReference;
typedef Containers::ArrayReference<const void> VoidArrayReference;

ArrayReferenceTest::ArrayReferenceTest() {
    addTests({&ArrayReferenceTest::constructEmpty,
              &ArrayReferenceTest::constructNullptr,
              &ArrayReferenceTest::constructNullptrSize,
              &ArrayReferenceTest::construct,
              &ArrayReferenceTest::constructFixedSize,
              &ArrayReferenceTest::constructArray,

              &ArrayReferenceTest::boolConversion,
              &ArrayReferenceTest::pointerConversion,

              &ArrayReferenceTest::emptyCheck,
              &ArrayReferenceTest::access,
              &ArrayReferenceTest::rangeBasedFor,

              &ArrayReferenceTest::sliceInvalid,
              &ArrayReferenceTest::sliceNullptr,
              &ArrayReferenceTest::slice,

              &ArrayReferenceTest::constReference,
              &ArrayReferenceTest::voidConstruction,
              &ArrayReferenceTest::voidConversion});
}

void ArrayReferenceTest::constructEmpty() {
    const ArrayReference a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayReferenceTest::constructNullptr() {
    const ArrayReference a(nullptr);
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, ArrayReference>::value));
}

void ArrayReferenceTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */
    const ArrayReference a(nullptr, 5);
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 5);
}

void ArrayReferenceTest::construct() {
    int a[30];

    const ArrayReference b = {a, 20};
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 20);
}

void ArrayReferenceTest::constructFixedSize() {
    int a[13];

    const ArrayReference b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 13);
}

void ArrayReferenceTest::constructArray() {
    Array a(5);

    const ArrayReference b = a;
    CORRADE_VERIFY(b.begin() == a.begin());
    CORRADE_COMPARE(b.size(), 5);
}

void ArrayReferenceTest::boolConversion() {
    int a[7];
    CORRADE_VERIFY(ArrayReference(a));
    CORRADE_VERIFY(!ArrayReference());
    CORRADE_VERIFY(VoidArrayReference(a));
    CORRADE_VERIFY(!VoidArrayReference());

    /* The conversion is explicit (i.e. no ArrayReference(a) + 7) */
    CORRADE_VERIFY(!(std::is_convertible<ArrayReference, int>::value));
    CORRADE_VERIFY(!(std::is_convertible<VoidArrayReference, int>::value));
}

void ArrayReferenceTest::pointerConversion() {
    int a[7];
    ArrayReference b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const ArrayReference c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    const VoidArrayReference d = a;
    const void* dp = d;
    CORRADE_COMPARE(dp, static_cast<const void*>(a));
}

void ArrayReferenceTest::emptyCheck() {
    ArrayReference a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    int b[5];
    ArrayReference c = {b, 5};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());
}

void ArrayReferenceTest::access() {
    int a[7];
    ArrayReference b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_COMPARE(b.data(), a);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), b.size());

    Containers::ArrayReference<const int> c = a;
    CORRADE_COMPARE(c.data(), a);
}

void ArrayReferenceTest::rangeBasedFor() {
    int a[5];
    ArrayReference b = a;
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

void ArrayReferenceTest::sliceInvalid() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayReference a = data;

    std::ostringstream out;
    Error::setOutput(&out);

    a.slice(a - 1, a);
    a.slice(a + 5, a + 6);
    a.slice(a + 2, a + 1);

    CORRADE_COMPARE(out.str(), "Containers::ArrayReference::slice(): slice out of range\n"
                               "Containers::ArrayReference::slice(): slice out of range\n"
                               "Containers::ArrayReference::slice(): slice out of range\n");
}

void ArrayReferenceTest::sliceNullptr() {
    ArrayReference a{nullptr, 5};

    ArrayReference b = a.prefix(nullptr);
    CORRADE_VERIFY(!b);
    CORRADE_COMPARE(b.size(), 0);

    ArrayReference c = a.suffix(nullptr);
    CORRADE_VERIFY(!c);
    CORRADE_COMPARE(c.size(), 5);

    int data[5];
    ArrayReference d{data};

    ArrayReference e = d.prefix(nullptr);
    CORRADE_VERIFY(!e);
    CORRADE_COMPARE(e.size(), 0);

    ArrayReference f = d.suffix(nullptr);
    CORRADE_VERIFY(!f);
    CORRADE_COMPARE(f.size(), 0);
}

void ArrayReferenceTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayReference a = data;

    ArrayReference b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayReference c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayReference d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);
}

void ArrayReferenceTest::constReference() {
    const int a[] = {3, 4, 7, 12, 0, -15};

    ConstArrayReference b = a;
    CORRADE_COMPARE(b.size(), 6);
    CORRADE_COMPARE(b[2], 7);

    int c[3];
    ArrayReference d = c;
    ConstArrayReference e = d;
    CORRADE_VERIFY(e == c);
    CORRADE_COMPARE(e.size(), 3);
}

void ArrayReferenceTest::voidConstruction() {
    void* a = reinterpret_cast<void*>(0xdeadbeef);
    VoidArrayReference b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(0xdeadbeef);
    VoidArrayReference d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_COMPARE(d.size(), 100);
}

void ArrayReferenceTest::voidConversion() {
    int a[] = {3, 4, 7, 12, 0, -15};

    /** @todo C++14: test that all the operations are really constexpr (C++11 doesn't allow void conversions IMHO) */

    /* void reference to compile-time array */
    VoidArrayReference b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 6*sizeof(int));

    /* void reference to runtime array */
    VoidArrayReference c = {a, 6};
    CORRADE_VERIFY(c == a);
    CORRADE_COMPARE(c.size(), 6*sizeof(int));

    /* void reference to Array */
    Array d(6);
    VoidArrayReference e = d;
    CORRADE_VERIFY(e == d);
    CORRADE_COMPARE(e.size(), d.size()*sizeof(int));

    /* void reference to ArrayReference */
    ArrayReference f = a;
    VoidArrayReference g = f;
    CORRADE_VERIFY(g == f);
    CORRADE_COMPARE(g.size(), f.size()*sizeof(int));
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayReferenceTest)
