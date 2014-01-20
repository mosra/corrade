/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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

class ArrayReferenceTest: public TestSuite::Tester {
    public:
        explicit ArrayReferenceTest();

        void constructEmpty();
        void construct();
        void constructFixedSize();
        void constructArray();

        void boolConversion();
        void pointerConversion();

        void emptyCheck();
        void access();
        #ifndef CORRADE_GCC45_COMPATIBILITY
        void rangeBasedFor();
        #endif

        void constReference();
        void voidConstruction();
        void voidConversion();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayReference<int> ArrayReference;
typedef Containers::ArrayReference<const int> ConstArrayReference;
typedef Containers::ArrayReference<const void> VoidArrayReference;

ArrayReferenceTest::ArrayReferenceTest() {
    addTests<ArrayReferenceTest>({&ArrayReferenceTest::constructEmpty,
              &ArrayReferenceTest::construct,
              &ArrayReferenceTest::constructFixedSize,
              &ArrayReferenceTest::constructArray,

              &ArrayReferenceTest::boolConversion,
              &ArrayReferenceTest::pointerConversion,

              &ArrayReferenceTest::emptyCheck,
              &ArrayReferenceTest::access,
              #ifndef CORRADE_GCC45_COMPATIBILITY
              &ArrayReferenceTest::rangeBasedFor,
              #endif

              &ArrayReferenceTest::constReference,
              &ArrayReferenceTest::voidConstruction,
              &ArrayReferenceTest::voidConversion});
}

void ArrayReferenceTest::constructEmpty() {
    const ArrayReference a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);

    #ifndef CORRADE_GCC45_COMPATIBILITY
    const ArrayReference b(nullptr);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_COMPARE(b.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, ArrayReference>::value));
    #endif
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
    #ifdef CORRADE_GCC44_COMPATIBILITY
    CORRADE_EXPECT_FAIL("Explicit conversion operators are not supported in GCC 4.4.");
    #endif
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

#ifndef CORRADE_GCC45_COMPATIBILITY
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
#endif

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
    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_VERIFY(e == d);
    #else
    CORRADE_VERIFY(e == static_cast<const void*>(d));
    #endif
    CORRADE_COMPARE(e.size(), d.size()*sizeof(int));

    /* void reference to ArrayReference */
    ArrayReference f = a;
    VoidArrayReference g = f;
    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_VERIFY(g == f);
    #else
    CORRADE_VERIFY(g == static_cast<const void*>(f));
    #endif
    CORRADE_COMPARE(g.size(), f.size()*sizeof(int));
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayReferenceTest)
