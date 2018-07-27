/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

struct StridedArrayViewTest: TestSuite::Tester {
    explicit StridedArrayViewTest();

    void constructEmpty();
    void constructNullptr();
    void constructNullptrSize();
    void construct();
    void constructFixedSize();
    void constructDerived();
    void constructConst();
    void constructView();
    void constructStaticView();

    void convertBool();
    void convertConst();

    void emptyCheck();
    void access();
    void accessConst();
    void accessInvalid();
    void iterator();
    void rangeBasedFor();

    void sliceInvalid();
    void slice();
};

typedef Containers::StridedArrayView<int> StridedArrayView;
typedef Containers::StridedArrayView<const int> ConstStridedArrayView;

StridedArrayViewTest::StridedArrayViewTest() {
    addTests({&StridedArrayViewTest::constructEmpty,
              &StridedArrayViewTest::constructNullptr,
              &StridedArrayViewTest::constructNullptrSize,
              &StridedArrayViewTest::construct,
              &StridedArrayViewTest::constructFixedSize,
              &StridedArrayViewTest::constructDerived,
              &StridedArrayViewTest::constructConst,
              &StridedArrayViewTest::constructView,
              &StridedArrayViewTest::constructStaticView,

              &StridedArrayViewTest::convertBool,
              &StridedArrayViewTest::convertConst,

              &StridedArrayViewTest::emptyCheck,
              &StridedArrayViewTest::access,
              &StridedArrayViewTest::accessConst,
              &StridedArrayViewTest::accessInvalid,
              &StridedArrayViewTest::iterator,
              &StridedArrayViewTest::rangeBasedFor,

              &StridedArrayViewTest::sliceInvalid,
              &StridedArrayViewTest::slice});
}

void StridedArrayViewTest::constructEmpty() {
    const StridedArrayView a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);
}

void StridedArrayViewTest::constructNullptr() {
    const StridedArrayView a(nullptr);
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, StridedArrayView>::value));
}

void StridedArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */
    const StridedArrayView a(nullptr, 5, 8);
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);
}

void StridedArrayViewTest::construct() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1}, {}, {}, {}, {}, {}
    };

    const StridedArrayView b = {&a[0].value, 5, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);
}

void StridedArrayViewTest::constructFixedSize() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

    const StridedArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);
}

void StridedArrayViewTest::constructDerived() {
    struct A { short i; };
    struct B: A {};

    /* Valid use case: constructing Containers::StridedArrayView<Math::Vector<3, Float>>
       from Containers::StridedArrayView<Color3> because the data have the same size
       and data layout */

    B b[5];
    Containers::StridedArrayView<B> bv{b};

    Containers::StridedArrayView<A> a{b};
    Containers::StridedArrayView<A> av{bv};

    CORRADE_VERIFY(a.data() == &b[0]);
    CORRADE_VERIFY(av.data() == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 2);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(av.stride(), 2);
}

void StridedArrayViewTest::constructConst() {
    const int a[]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

    ConstStridedArrayView b = a;
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);
}

void StridedArrayViewTest::constructView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    const ArrayView<int> view = a;

    const StridedArrayView b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);
}

void StridedArrayViewTest::constructStaticView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    const StaticArrayView<10, int> view = a;

    const StridedArrayView b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);
}

void StridedArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(StridedArrayView(a));
    CORRADE_VERIFY(!StridedArrayView());
    CORRADE_VERIFY(!(std::is_convertible<StridedArrayView, int>::value));
}

void StridedArrayViewTest::convertConst() {
    int a[3];
    StridedArrayView b = a;
    ConstStridedArrayView c = b;
    CORRADE_VERIFY(c.data() == a);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c.stride(), 4);
}

void StridedArrayViewTest::emptyCheck() {
    StridedArrayView a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    int b[5];
    StridedArrayView c = {b, 5, 4};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());
}

void StridedArrayViewTest::access() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1}, {}, {}, {}, {}, {}
    };

    StridedArrayView b{&a[0].value, 7, 8};
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    /* Data access */
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(b[4], 4);

    ConstStridedArrayView c = {&a[0].value, 7, 8};
    CORRADE_COMPARE(c.data(), a);
}

void StridedArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const StridedArrayView b = a;
    b.front() = 0;
    *(b.begin()+1) = 1;
    *(b.cbegin()+2) = 2;
    b[3] = 3;
    *(b.end()-3) = 4;
    *(b.cend()-2) = 5;
    b.back() = 6;

    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a[5], 5);
    CORRADE_COMPARE(a[6], 6);
}

void StridedArrayViewTest::accessInvalid() {
    std::stringstream out;
    Error redirectError{&out};

    StridedArrayView a;
    a.front();
    a.back();
    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView::front(): view is empty\n"
        "Containers::StridedArrayView::back(): view is empty\n");
}

void StridedArrayViewTest::iterator() {
    struct {
        int value;
        int:32;
    } a[10]{{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}};

    StridedArrayView b{&a[0].value, 7, 8};

    CORRADE_VERIFY(b.begin() == b.begin());
    CORRADE_VERIFY(b.begin() != b.begin() + 1);

    CORRADE_VERIFY(b.begin() < b.begin() + 1);
    CORRADE_VERIFY(!(b.begin() < b.begin()));
    CORRADE_VERIFY(b.begin() <= b.begin());
    CORRADE_VERIFY(!(b.begin() + 1 <= b.begin()));

    CORRADE_VERIFY(b.begin() + 1 > b.begin());
    CORRADE_VERIFY(!(b.begin() > b.begin()));
    CORRADE_VERIFY(b.begin() >= b.begin());
    CORRADE_VERIFY(!(b.begin() >= b.begin() + 1));

    CORRADE_VERIFY(b.cbegin() == b.begin());
    CORRADE_VERIFY(b.cend() == b.end());

    CORRADE_COMPARE(*(b.begin() + 2), 2);
    CORRADE_COMPARE(*(2 + b.begin()), 2);
    CORRADE_COMPARE(*(b.end() - 2), 5);
    CORRADE_COMPARE(b.end() - b.begin(), b.size());

    CORRADE_COMPARE(*(++b.begin()), 1);
    CORRADE_COMPARE(*(--b.end()), 6);
}

void StridedArrayViewTest::rangeBasedFor() {
    struct {
        int value;
        int other;
    } a[5];
    StridedArrayView b{&a[0].value, 5, 8};
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

void StridedArrayViewTest::sliceInvalid() {
    int data[5] = {1, 2, 3, 4, 5};
    StridedArrayView a = data;

    std::ostringstream out;
    Error redirectError{&out};

    a.slice(5, 6);
    a.slice(2, 1);

    CORRADE_COMPARE(out.str(),
        "Containers::StridedArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::StridedArrayView::slice(): slice [2:1] out of range for 5 elements\n");
}

void StridedArrayViewTest::slice() {
    struct {
        int value;
        float other;
    } data[5]{{1, 0.0f}, {2, 5.0f}, {3, -1.0f}, {4, 0.5f}, {5, -0.1f}};
    StridedArrayView a{&data[0].value, 5, 8};

    StridedArrayView b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StridedArrayView c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    StridedArrayView d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedArrayViewTest)
