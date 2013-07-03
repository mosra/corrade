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

class ArrayReferenceTest: public TestSuite::Tester {
    public:
        explicit ArrayReferenceTest();

        void constructEmpty();
        void construct();
        void constructFixedSize();
        void constructArray();
        void emptyCheck();
        void access();
        void rangeBasedFor();

        void constReference();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayReference<int> ArrayReference;
typedef Containers::ArrayReference<const int> ConstArrayReference;

ArrayReferenceTest::ArrayReferenceTest() {
    addTests({&ArrayReferenceTest::constructEmpty,
              &ArrayReferenceTest::construct,
              &ArrayReferenceTest::constructFixedSize,
              &ArrayReferenceTest::constructArray,
              &ArrayReferenceTest::emptyCheck,
              &ArrayReferenceTest::access,
              &ArrayReferenceTest::rangeBasedFor});
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

    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), b.size());
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

void ArrayReferenceTest::constReference() {
    const int a[] = {3, 4, 7, 12, 0, -15};

    ConstArrayReference b = a;
    CORRADE_COMPARE(b.size(), 6);
    CORRADE_COMPARE(b[2], 7);

    int c[3];
    ArrayReference d = c;
    ConstArrayReference e = d;
    CORRADE_VERIFY(e == a);
    CORRADE_COMPARE(e.size(), 3);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayReferenceTest)
