/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct IntView {
    IntView(int* data, std::size_t size): data{data}, size{size} {}

    int* data;
    std::size_t size;
};

struct ConstIntView {
    constexpr ConstIntView(const int* data, std::size_t size): data{data}, size{size} {}

    const int* data;
    std::size_t size;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct ArrayViewConverter<int, IntView> {
    static ArrayView<int> from(IntView other) {
        return {other.data, other.size};
    }
};

template<> struct ArrayViewConverter<const int, ConstIntView> {
    constexpr static ArrayView<const int> from(ConstIntView other) {
        return {other.data, other.size};
    }
};

/* To keep the (Strided)ArrayView API in reasonable bounds, the cost-adding
   variants have to be implemented explicitly */
template<> struct ArrayViewConverter<const int, IntView> {
    static ArrayView<const int> from(IntView other) {
        return {other.data, other.size};
    }
};

}

namespace Test { namespace {

struct StridedArrayViewTest: TestSuite::Tester {
    explicit StridedArrayViewTest();

    void constructEmpty();
    void constructNullptr();
    void constructNullptrSize();
    void construct();
    void constructFixedSize();
    void constructDerived();
    void constructView();
    void constructStaticView();

    void convertBool();
    void convertConst();
    void convertFromExternalView();
    void convertConstFromExternalView();

    void emptyCheck();
    void access();
    void accessConst();
    void accessInvalid();
    void iterator();
    void rangeBasedFor();

    void sliceInvalid();
    void slice();

    void cast();
    void castInvalid();
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
              &StridedArrayViewTest::constructView,
              &StridedArrayViewTest::constructStaticView,

              &StridedArrayViewTest::convertBool,
              &StridedArrayViewTest::convertConst,
              &StridedArrayViewTest::convertFromExternalView,
              &StridedArrayViewTest::convertConstFromExternalView,

              &StridedArrayViewTest::emptyCheck,
              &StridedArrayViewTest::access,
              &StridedArrayViewTest::accessConst,
              &StridedArrayViewTest::accessInvalid,
              &StridedArrayViewTest::iterator,
              &StridedArrayViewTest::rangeBasedFor,

              &StridedArrayViewTest::sliceInvalid,
              &StridedArrayViewTest::slice,

              &StridedArrayViewTest::cast,
              &StridedArrayViewTest::castInvalid});
}

void StridedArrayViewTest::constructEmpty() {
    StridedArrayView a;
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr StridedArrayView ca;
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);
}

void StridedArrayViewTest::constructNullptr() {
    StridedArrayView a(nullptr);
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(a.stride(), 0);

    constexpr StridedArrayView ca(nullptr);
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), 0);
    CORRADE_COMPARE(ca.stride(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, StridedArrayView>::value));
}

void StridedArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::Buffer::setData() without passing any actual data */
    StridedArrayView a(nullptr, 5, 8);
    CORRADE_VERIFY(a.data() == nullptr);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);

    constexpr StridedArrayView ca(nullptr, 5, 8);
    CORRADE_VERIFY(ca.data() == nullptr);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(ca.stride(), 8);
}

/* Needs to be here in order to use it in constexpr */
constexpr const struct {
    int value;
    int other;
} Struct[10]{
    {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
    /* Otherwise GCC 4.8 loudly complains about missing initializers */
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
};

void StridedArrayViewTest::construct() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView b = {&a[0].value, 5, 8};
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr ConstStridedArrayView cb = {&Struct[0].value, 5, 8};
    CORRADE_VERIFY(cb.data() == Struct);
    CORRADE_COMPARE(cb.size(), 5);
    CORRADE_COMPARE(cb.stride(), 8);
    CORRADE_COMPARE(cb[2], 7853268);
    CORRADE_COMPARE(cb[4], 234810);
}

/* Needs to be here in order to use it in constexpr */
constexpr const int Array[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

void StridedArrayViewTest::constructFixedSize() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };

    const StridedArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr ConstStridedArrayView cb = Array;
    CORRADE_VERIFY(cb.data() == Array);
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 4);
    CORRADE_COMPARE(cb[2], 7853268);
    CORRADE_COMPARE(cb[4], 234810);
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(): i{} {}
    short i;
};
struct Derived: Base {
    constexpr Derived() {}
};
constexpr Derived DerivedArray[5]
    /* This missing makes MSVC2015 complain it's not constexpr, but if present
       then GCC 4.8 fails to build. Eh. ¯\_(ツ)_/¯ */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    {}
    #endif
    ;

void StridedArrayViewTest::constructDerived() {
    /* Valid use case: constructing Containers::StridedArrayView<Math::Vector<3, Float>>
       from Containers::StridedArrayView<Color3> because the data have the same size
       and data layout */

    Derived b[5];
    Containers::StridedArrayView<Derived> bv{b};
    Containers::StridedArrayView<Base> a{b};
    Containers::StridedArrayView<Base> av{bv};

    CORRADE_VERIFY(a.data() == &b[0]);
    CORRADE_VERIFY(av.data() == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 2);
    CORRADE_COMPARE(av.size(), 5);
    CORRADE_COMPARE(av.stride(), 2);

    constexpr Containers::StridedArrayView<const Derived> cbv{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StridedArrayView<const Base> ca{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StridedArrayView<const Base> cav{cbv};

    CORRADE_VERIFY(ca.data() == &DerivedArray[0]);
    CORRADE_VERIFY(cav.data() == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(ca.stride(), 2);
    CORRADE_COMPARE(cav.size(), 5);
    CORRADE_COMPARE(cav.stride(), 2);
}

void StridedArrayViewTest::constructView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    ArrayView<int> view = a;
    StridedArrayView b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr ArrayView<const int> cview = Array;
    constexpr ConstStridedArrayView cb = cview;
    CORRADE_VERIFY(cb.data() == Array);
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 4);
    CORRADE_COMPARE(cb[2], 7853268);
    CORRADE_COMPARE(cb[4], 234810);
}

void StridedArrayViewTest::constructStaticView() {
    int a[10]{ 2, 16, 7853268, -100, 234810, 0, 0, 0, 0, 0 };
    StaticArrayView<10, int> view = a;
    StridedArrayView b = view;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 10);
    CORRADE_COMPARE(b.stride(), 4);
    CORRADE_COMPARE(b[2], 7853268);
    CORRADE_COMPARE(b[4], 234810);

    constexpr StaticArrayView<10, const int> cview = Array;
    constexpr ConstStridedArrayView cb = cview;
    CORRADE_VERIFY(cb.data() == Array);
    CORRADE_COMPARE(cb.size(), 10);
    CORRADE_COMPARE(cb.stride(), 4);
    CORRADE_COMPARE(cb[2], 7853268);
    CORRADE_COMPARE(cb[4], 234810);
}

void StridedArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(StridedArrayView(a));
    CORRADE_VERIFY(!StridedArrayView());

    constexpr ConstStridedArrayView cb = Array;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(boolCb);

    constexpr ConstStridedArrayView cc;
    constexpr bool boolCc = !!cc;
    CORRADE_VERIFY(!boolCc);

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

void StridedArrayViewTest::convertFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    StridedArrayView b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5);

    constexpr ConstIntView ca{Array, 10};
    CORRADE_COMPARE(ca.data, Array);
    CORRADE_COMPARE(ca.size, 10);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if defined(CORRADE_TARGET_APPLE) && __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstStridedArrayView cb = ca;
    CORRADE_COMPARE(cb.data(), Array);
    CORRADE_COMPARE(cb.size(), 10);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, Containers::StridedArrayView<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, Containers::StridedArrayView<float>>::value));
}

void StridedArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstStridedArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion to a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, Containers::StridedArrayView<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, Containers::StridedArrayView<const float>>::value));
}

void StridedArrayViewTest::emptyCheck() {
    StridedArrayView a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    constexpr StridedArrayView ca;
    CORRADE_VERIFY(!ca);
    constexpr bool caEmpty = ca.empty();
    CORRADE_VERIFY(caEmpty);

    int b[5];
    StridedArrayView c = {b, 5, 4};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());

    constexpr ConstStridedArrayView cb = {Array, 5, 4};
    CORRADE_VERIFY(cb);
    constexpr bool cbEmpty = cb.empty();
    CORRADE_VERIFY(!cbEmpty);
}

void StridedArrayViewTest::access() {
    struct {
        int value;
        int other;
    } a[10]{
        {2, 23125}, {16, 1}, {7853268, -2}, {-100, 5}, {234810, 1},
        /* Otherwise GCC 4.8 loudly complains about missing initializers */
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    StridedArrayView b{&a[0].value, 7, 8};
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.stride(), 8);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(b[4], 4);

    ConstStridedArrayView c = {&a[0].value, 7, 8};
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstStridedArrayView cb = {&Struct[0].value, 7, 8};

    constexpr const void* data = cb.data();
    CORRADE_VERIFY(data == Struct);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7);

    constexpr std::size_t stride = cb.stride();
    CORRADE_COMPARE(stride, 8);
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

void StridedArrayViewTest::cast() {
    struct {
        short a;
        short b;
        int c;
    } data[5]{{1, 10, 0}, {2, 20, 0}, {3, 30, 0}, {4, 40, 0}, {5, 50, 0}};
    Containers::StridedArrayView<short> a{&data[0].a, 5, 8};
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 8);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 4);

    auto b = Containers::arrayCast<int>(a);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(b.stride(), 8);
    #ifndef CORRADE_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (30 << 16) | 3); /* 1966083 on LE */
    CORRADE_COMPARE(b[3], (40 << 16) | 4); /* 2621444 on LE */
    #else
    CORRADE_COMPARE(b[2], (3 << 16) | 30); /* 196638 on BE */
    CORRADE_COMPARE(b[3], (4 << 16) | 40); /* 262184 on BE */
    #endif
}

void StridedArrayViewTest::castInvalid() {
     struct {
        char a;
        char b;
    } data[5] CORRADE_ALIGNAS(2) {{1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}};
    Containers::StridedArrayView<char> a{&data[0].a, 5, 2};
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(a.stride(), 2);

    /* Check the alignment to avoid unaligned reads on platforms where it
       matters (such as Emscripten) */
    CORRADE_VERIFY(reinterpret_cast<std::uintptr_t>(data)%2 == 0);

    auto b = Containers::arrayCast<short>(a);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(b.stride(), 2);
    #ifndef CORRADE_BIG_ENDIAN
    CORRADE_COMPARE(b[2], (30 << 8) | 3); /* 7683 on LE */
    CORRADE_COMPARE(b[3], (40 << 8) | 4); /* 10244 on LE */
    #else
    CORRADE_COMPARE(b[2], (3 << 8) | 30); /* 798 on BE */
    CORRADE_COMPARE(b[3], (4 << 8) | 40); /* 1064 on BE */
    #endif

    {
        std::ostringstream out;
        Error redirectError{&out};
        Containers::arrayCast<int>(a);
        CORRADE_COMPARE(out.str(), "Containers::arrayCast(): can't fit a 4-byte type into a stride of 2\n");
    }
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StridedArrayViewTest)
