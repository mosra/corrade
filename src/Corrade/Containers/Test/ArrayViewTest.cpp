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

#include <sstream>

#include "Corrade/Containers/ArrayView.h"
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
    /* Needed only by convertVoidFromExternalView() */
    static ArrayView<int> from(IntView other) {
        return {other.data, other.size};
    }
};

template<> struct ErasedArrayViewConverter<IntView>: ArrayViewConverter<int, IntView> {};
template<> struct ErasedArrayViewConverter<const IntView>: ArrayViewConverter<int, IntView> {};

template<> struct ArrayViewConverter<const int, ConstIntView> {
    constexpr static ArrayView<const int> from(ConstIntView other) {
        return {other.data, other.size};
    }

    constexpr static ConstIntView to(ArrayView<const int> other) {
        return {other.data(), other.size()};
    }
};

template<> struct ErasedArrayViewConverter<ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};
template<> struct ErasedArrayViewConverter<const ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};

/* To keep the ArrayView API in reasonable bounds, the const-adding variants
   have to be implemented explicitly */
template<> struct ArrayViewConverter<const int, IntView> {
    static ArrayView<const int> from(IntView other) {
        return {other.data, other.size};
    }
};
template<> struct ArrayViewConverter<int, ConstIntView> {
    constexpr static ConstIntView to(ArrayView<int> other) {
        return {other.data(), other.size()};
    }
};

}

namespace Test { namespace {

struct ArrayViewTest: TestSuite::Tester {
    explicit ArrayViewTest();

    void constructEmpty();
    void constructEmptyVoid();
    void constructEmptyConstVoid();
    void constructNullptr();
    void constructNullptrVoid();
    void constructNullptrConstVoid();
    void constructNullptrSize();
    void construct();
    void constructVoid();
    void constructConstVoid();
    void constructVoidFrom();
    void constructConstVoidFrom();
    void constructConstVoidFromVoid();
    void constructFixedSize();
    void constructFixedSizeVoid();
    void constructFixedSizeConstVoid();
    void constructFromStatic();
    void constructFromStaticVoid();
    void constructFromStaticConstVoid();
    void constructDerived();
    void constructInitializerList();

    void convertBool();
    void convertPointer();
    void convertConst();
    void convertExternalView();
    void convertConstFromExternalView();
    void convertToConstExternalView();
    void convertVoidFromExternalView();
    void convertConstVoidFromExternalView();
    void convertConstVoidFromConstExternalView();

    void emptyCheck();
    void access();
    void accessConst();
    void accessVoid();
    void accessConstVoid();
    void accessInvalid();
    void rangeBasedFor();

    void sliceInvalid();
    void sliceNullptr();
    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();

    void cast();
    void castInvalid();
    void size();
};

typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;

ArrayViewTest::ArrayViewTest() {
    addTests({&ArrayViewTest::constructEmpty,
              &ArrayViewTest::constructEmptyVoid,
              &ArrayViewTest::constructEmptyConstVoid,
              &ArrayViewTest::constructNullptr,
              &ArrayViewTest::constructNullptrVoid,
              &ArrayViewTest::constructNullptrConstVoid,
              &ArrayViewTest::constructNullptrSize,
              &ArrayViewTest::construct,
              &ArrayViewTest::constructVoid,
              &ArrayViewTest::constructConstVoid,
              &ArrayViewTest::constructVoidFrom,
              &ArrayViewTest::constructConstVoidFrom,
              &ArrayViewTest::constructConstVoidFromVoid,
              &ArrayViewTest::constructFixedSize,
              &ArrayViewTest::constructFixedSizeVoid,
              &ArrayViewTest::constructFixedSizeConstVoid,
              &ArrayViewTest::constructFromStatic,
              &ArrayViewTest::constructFromStaticVoid,
              &ArrayViewTest::constructFromStaticConstVoid,
              &ArrayViewTest::constructDerived,
              &ArrayViewTest::constructInitializerList,

              &ArrayViewTest::convertBool,
              &ArrayViewTest::convertPointer,
              &ArrayViewTest::convertConst,
              &ArrayViewTest::convertExternalView,
              &ArrayViewTest::convertConstFromExternalView,
              &ArrayViewTest::convertToConstExternalView,
              &ArrayViewTest::convertVoidFromExternalView,
              &ArrayViewTest::convertConstVoidFromExternalView,
              &ArrayViewTest::convertConstVoidFromConstExternalView,

              &ArrayViewTest::emptyCheck,
              &ArrayViewTest::access,
              &ArrayViewTest::accessConst,
              &ArrayViewTest::accessVoid,
              &ArrayViewTest::accessConstVoid,
              &ArrayViewTest::accessInvalid,
              &ArrayViewTest::rangeBasedFor,

              &ArrayViewTest::sliceInvalid,
              &ArrayViewTest::sliceNullptr,
              &ArrayViewTest::slice,
              &ArrayViewTest::slicePointer,
              &ArrayViewTest::sliceToStatic,
              &ArrayViewTest::sliceToStaticPointer,

              &ArrayViewTest::cast,
              &ArrayViewTest::castInvalid,
              &ArrayViewTest::size});
}

void ArrayViewTest::constructEmpty() {
    ArrayView a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr ArrayView ca;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructEmptyVoid() {
    VoidArrayView a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr VoidArrayView ca;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructEmptyConstVoid() {
    ConstVoidArrayView a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr ConstVoidArrayView ca;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructNullptr() {
    ArrayView a = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr ArrayView ca = nullptr;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructNullptrVoid() {
    VoidArrayView a = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr VoidArrayView ca = nullptr;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructNullptrConstVoid() {
    ConstVoidArrayView a = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(a.empty());
    CORRADE_COMPARE(a.size(), 0);

    constexpr ConstVoidArrayView ca = nullptr;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(ca.empty());
    CORRADE_COMPARE(ca.size(), 0);
}

void ArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::GL::Buffer::setData() without passing any actual data */
    ArrayView a{nullptr, 5};
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(!a.empty());
    CORRADE_COMPARE(a.size(), 5);

    constexpr ArrayView ca{nullptr, 5};
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(!a.empty());
    CORRADE_COMPARE(ca.size(), 5);
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array30[30]{};

void ArrayViewTest::construct() {
    int a[30];

    {
        ArrayView b = {a, 20};
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 20);
    } {
        auto b = arrayView(a, 20);
        CORRADE_VERIFY((std::is_same<decltype(b), ArrayView>::value));
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 20);

        auto c = arrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), ArrayView>::value));
        CORRADE_VERIFY(c == a);
        CORRADE_COMPARE(c.size(), 20);
    }

    {
        constexpr ConstArrayView b = {Array30, 20};
        CORRADE_VERIFY(b == Array30);
        CORRADE_COMPARE(b.size(), 20);
    } {
        constexpr auto b = arrayView(Array30, 20);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstArrayView>::value));
        CORRADE_VERIFY(b == Array30);
        CORRADE_COMPARE(b.size(), 20);

        constexpr auto c = arrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), const ConstArrayView>::value));
        CORRADE_VERIFY(c == Array30);
        CORRADE_COMPARE(c.size(), 20);
    }
}

void ArrayViewTest::constructVoid() {
    void* a = reinterpret_cast<void*>(0xdeadbeef);
    VoidArrayView b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(0xdeadbeef);
    VoidArrayView d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_VERIFY(!d.empty());
    CORRADE_COMPARE(d.size(), 100);

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::constructConstVoid() {
    void* a = reinterpret_cast<void*>(0xdeadbeef);
    ConstVoidArrayView b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(0xdeadbeef);
    ConstVoidArrayView d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_VERIFY(!d.empty());
    CORRADE_COMPARE(d.size(), 100);

    constexpr ConstVoidArrayView cd{Array30, 25};
    CORRADE_VERIFY(cd == Array30);
    CORRADE_VERIFY(!cd.empty());
    CORRADE_COMPARE(cd.size(), 100);
}

void ArrayViewTest::constructVoidFrom() {
    int a[13];
    const ArrayView b = a;
    VoidArrayView c = b;
    CORRADE_VERIFY(c == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::constructConstVoidFrom() {
    int a[13];
    const ArrayView b = a;
    const ConstArrayView cb = a;
    ConstVoidArrayView c = b;
    ConstVoidArrayView cc = cb;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));

    constexpr ConstArrayView ccb = Array30;
    constexpr ConstVoidArrayView ccc = ccb;
    CORRADE_VERIFY(ccc == Array30);
    CORRADE_COMPARE(ccc.size(), 30*sizeof(int));
}

void ArrayViewTest::constructConstVoidFromVoid() {
    int a[13];
    const ArrayView b = a;
    VoidArrayView c = b;
    ConstVoidArrayView cc = c;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array13[13]{};

void ArrayViewTest::constructFixedSize() {
    int a[13];

    {
        ArrayView b = a;
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    } {
        auto b = arrayView(a);
        CORRADE_VERIFY((std::is_same<decltype(b), ArrayView>::value));
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    }

    {
        constexpr ConstArrayView b = Array13;
        CORRADE_VERIFY(b == Array13);
        CORRADE_COMPARE(b.size(), 13);
    } {
        constexpr auto b = arrayView(Array13);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstArrayView>::value));
        CORRADE_VERIFY(b == Array13);
        CORRADE_COMPARE(b.size(), 13);
    }

    /* Implicit construction from pointer should not be allowed */
    CORRADE_VERIFY(!(std::is_convertible<int*, ArrayView>::value));
}

void ArrayViewTest::constructFixedSizeVoid() {
    int a[13];
    VoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::constructFixedSizeConstVoid() {
    const int a[13]{};
    ConstVoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(b.size(), 13*sizeof(int));

    constexpr ConstVoidArrayView cb = Array30;
    CORRADE_VERIFY(cb == Array30);
    CORRADE_VERIFY(!cb.empty());
    CORRADE_COMPARE(cb.size(), 30*sizeof(int));
}

void ArrayViewTest::constructFromStatic() {
    int a[13];
    StaticArrayView<13, int> av = a;
    constexpr StaticArrayView<13, const int> cav = Array13;

    {
        ArrayView b = av;
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    } {
        auto b = arrayView(av);
        CORRADE_VERIFY((std::is_same<decltype(b), ArrayView>::value));
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    }

    {
        constexpr ConstArrayView b = cav;
        CORRADE_VERIFY(b == cav);
        CORRADE_COMPARE(b.size(), 13);
    } {
        constexpr auto b = arrayView(cav);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstArrayView>::value));
        CORRADE_VERIFY(b == cav);
        CORRADE_COMPARE(b.size(), 13);
    }
}

void ArrayViewTest::constructFromStaticVoid() {
    int a[13];
    const StaticArrayView<13, int> b = a;
    VoidArrayView c = b;
    CORRADE_VERIFY(c == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::constructFromStaticConstVoid() {
    int a[13];
    const StaticArrayView<13, int> b = a;
    const StaticArrayView<13, const int> cb = a;
    ConstVoidArrayView c = b;
    ConstVoidArrayView cc = cb;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));

    constexpr StaticArrayView<13, const int> ccb = Array13;
    constexpr ConstVoidArrayView ccc = ccb;
    CORRADE_VERIFY(ccc == Array13);
    CORRADE_COMPARE(ccc.size(), 13*sizeof(int));
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(): i{} {}
    int i;
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

void ArrayViewTest::constructDerived() {
    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    Derived b[5];
    Containers::ArrayView<Derived> bv{b};
    Containers::ArrayView<Base> a{b};
    Containers::ArrayView<Base> av{bv};

    CORRADE_VERIFY(a == &b[0]);
    CORRADE_VERIFY(av == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);

    constexpr Containers::ArrayView<const Derived> cbv{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::ArrayView<const Base> ca{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::ArrayView<const Base> cav{cbv};

    CORRADE_VERIFY(ca == &DerivedArray[0]);
    CORRADE_VERIFY(cav == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);
}

void ArrayViewTest::constructInitializerList() {
    std::initializer_list<int> a = {3, 5, 7};
    ConstArrayView b = arrayView(a);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b.back(), 7);

    /* R-value init list should work too */
    CORRADE_COMPARE(arrayView<int>({3, 5, 7}).front(), 3);
}

void ArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(ArrayView(a));
    CORRADE_VERIFY(!ArrayView());
    CORRADE_VERIFY(VoidArrayView(a));
    CORRADE_VERIFY(!VoidArrayView());
    CORRADE_VERIFY(ConstVoidArrayView(a));
    CORRADE_VERIFY(!ConstVoidArrayView());

    constexpr ConstArrayView cb = Array30;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(boolCb);

    constexpr ConstArrayView cc;
    constexpr bool boolCc = !!cc;
    CORRADE_VERIFY(!boolCc);

    /** @todo constexpr void but not const? c++14? */

    constexpr ConstVoidArrayView cvb = Array30;
    constexpr bool boolCvb = !!cvb;
    CORRADE_VERIFY(boolCvb);

    constexpr ConstVoidArrayView cvc;
    constexpr bool boolCvc = !!cvc;
    CORRADE_VERIFY(!boolCvc);

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY((std::is_constructible<bool, ArrayView>::value));
    CORRADE_VERIFY((std::is_constructible<bool, VoidArrayView>::value));
    CORRADE_VERIFY((std::is_constructible<bool, ConstVoidArrayView>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, ArrayView>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, VoidArrayView>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, ConstVoidArrayView>::value));
}

void ArrayViewTest::convertPointer() {
    int a[7];
    ArrayView b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const ArrayView c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    constexpr ConstArrayView cc = Array13;
    constexpr const int* ccp = cc;
    CORRADE_COMPARE(ccp, static_cast<const int*>(Array13));

    const ConstVoidArrayView d = a;
    const void* dp = d;
    CORRADE_COMPARE(dp, static_cast<const void*>(a));

    constexpr ConstVoidArrayView cd = Array30;
    constexpr const void* cdp = cd;
    CORRADE_COMPARE(cdp, static_cast<const void*>(Array30));

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

void ArrayViewTest::convertExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView a{data, 5};
    CORRADE_COMPARE(a.data, data);
    CORRADE_COMPARE(a.size, 5);

    ConstArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    ConstIntView c = b;
    CORRADE_COMPARE(c.data, data);
    CORRADE_COMPARE(c.size, 5);

    auto d = arrayView(c);
    CORRADE_VERIFY((std::is_same<decltype(d), Containers::ArrayView<const int>>::value));
    CORRADE_COMPARE(d.data(), data);
    CORRADE_COMPARE(d.size(), 5);

    constexpr ConstIntView ca{Array13, 13};
    CORRADE_COMPARE(ca.data, Array13);
    CORRADE_COMPARE(ca.size, 13);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstArrayView cb = ca;
    CORRADE_COMPARE(cb.data(), Array13);
    CORRADE_COMPARE(cb.size(), 13);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstIntView cc = cb;
    CORRADE_COMPARE(cc.data, Array13);
    CORRADE_COMPARE(cc.size, 13);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #else
    const
    #endif
    auto cd = arrayView(cc);
    CORRADE_VERIFY((std::is_same<decltype(cd), const Containers::ArrayView<const int>>::value));
    CORRADE_COMPARE(cd.data(), Array13);
    CORRADE_COMPARE(cd.size(), 13);

    /* Conversion from/to a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<ConstIntView, Containers::ArrayView<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<ConstIntView, Containers::ArrayView<const float>>::value));
    CORRADE_VERIFY((std::is_convertible<Containers::ArrayView<const int>, ConstIntView>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::ArrayView<const float>, ConstIntView>::value));
}

void ArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView, Containers::ArrayView<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView, Containers::ArrayView<const float>>::value));
}

void ArrayViewTest::convertToConstExternalView() {
    int data[]{1, 2, 3, 4, 5};
    ArrayView a = data;
    CORRADE_COMPARE(a.data(), &data[0]);
    CORRADE_COMPARE(a.size(), 5);

    ConstIntView b = a;
    CORRADE_COMPARE(b.data, data);
    CORRADE_COMPARE(b.size, 5);

    /* Conversion to a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::ArrayView<int>, ConstIntView>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::ArrayView<float>, ConstIntView>::value));
}

void ArrayViewTest::convertVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    VoidArrayView b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5*4);
}

void ArrayViewTest::convertConstVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5*4);
}

void ArrayViewTest::convertConstVoidFromConstExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5*4);
}

void ArrayViewTest::emptyCheck() {
    ArrayView a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    constexpr ConstArrayView ca;
    CORRADE_VERIFY(!ca);
    constexpr bool caEmpty = ca.empty();
    CORRADE_VERIFY(caEmpty);

    int b[5];
    ArrayView c = {b, 5};
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!c.empty());

    constexpr ConstArrayView cc = {Array13, 5};
    CORRADE_VERIFY(cc);
    constexpr bool ccEmpty = cc.empty();
    CORRADE_VERIFY(!ccEmpty);
}

/* Needs to be here in order to use it in constexpr */
constexpr int OneToSeven[]{0, 1, 2, 3, 4, 5, 6};

void ArrayViewTest::access() {
    int a[7];
    ArrayView b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), b.size());
    CORRADE_COMPARE(b.cbegin(), b.begin());
    CORRADE_COMPARE(b.cend(), b.end());

    Containers::ArrayView<const int> c = a;
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstArrayView cb = OneToSeven;

    constexpr const int* data = cb.data();
    CORRADE_VERIFY(data == OneToSeven);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7);

    constexpr const int* begin = cb.begin();
    constexpr const int* cbegin = cb.cbegin();
    CORRADE_COMPARE(begin, OneToSeven);
    CORRADE_COMPARE(cbegin, OneToSeven);

    constexpr const int* end = cb.end();
    constexpr const int* cend = cb.cend();
    CORRADE_COMPARE(end, OneToSeven + 7);
    CORRADE_COMPARE(cend, OneToSeven + 7);
}

void ArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const ArrayView b = a;
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

void ArrayViewTest::accessVoid() {
    int a[7]{};

    VoidArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7*sizeof(int));

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::accessConstVoid() {
    int a[7]{};

    ConstVoidArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7*sizeof(int));

    constexpr ConstVoidArrayView cb = OneToSeven;

    constexpr const void* data = cb.data();
    CORRADE_VERIFY(data == OneToSeven);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7*sizeof(int));
}

void ArrayViewTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    ArrayView a;
    a.front();
    a.back();
    CORRADE_COMPARE(out.str(),
        "Containers::ArrayView::front(): view is empty\n"
        "Containers::ArrayView::back(): view is empty\n");
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    /* Do it this way to avoid (reasonable) warnings about out-of-bounds array
       access with `a - 1` */
    int data[6] = {0, 1, 2, 3, 4, 5};
    ArrayView a{data + 1, 5};

    CORRADE_COMPARE(a.size(), 5);

    std::ostringstream out;
    Error redirectError{&out};

    /* Testing both pointer and size versions */
    a.slice(a - 1, a);
    a.slice(a + 5, a + 6);
    a.slice(5, 6);
    a.slice(a + 2, a + 1);
    a.slice(2, 1);
    /* Testing template size + pointer, template size + size and full template
       version */
    a.slice<1>(a - 1);
    a.slice<5>(a + 1);
    a.slice<5>(1);
    a.slice<1, 6>();

    CORRADE_COMPARE(out.str(),
        "Containers::ArrayView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n"
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

    constexpr ArrayView ca{nullptr, 5};

    constexpr ArrayView cb = ca.prefix(nullptr);
    CORRADE_VERIFY(!cb);
    CORRADE_COMPARE(cb.size(), 0);

    /* constexpr ArrayView cc = ca.suffix(nullptr) won't compile because
       arithmetic on nullptr is not allowed */

    int data[5];
    ArrayView d{data};

    ArrayView e = d.prefix(nullptr);
    CORRADE_VERIFY(!e);
    CORRADE_COMPARE(e.size(), 0);

    ArrayView f = d.suffix(nullptr);
    CORRADE_VERIFY(!f);
    CORRADE_COMPARE(f.size(), 0);

    constexpr ConstArrayView cd = Array13;
    constexpr ConstArrayView ce = cd.prefix(nullptr);
    CORRADE_VERIFY(!ce);
    CORRADE_COMPARE(ce.size(), 0);

    constexpr ConstArrayView cf = cd.suffix(nullptr);
    CORRADE_VERIFY(!cf);
    CORRADE_COMPARE(cf.size(), 0);
}

constexpr int Array5[]{1, 2, 3, 4, 5};

void ArrayViewTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    ArrayView b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayView c1 = a.prefix(3);
    CORRADE_COMPARE(c1.size(), 3);
    CORRADE_COMPARE(c1[0], 1);
    CORRADE_COMPARE(c1[1], 2);
    CORRADE_COMPARE(c1[2], 3);

    ArrayView c2 = a.except(2);
    CORRADE_COMPARE(c2.size(), 3);
    CORRADE_COMPARE(c2[0], 1);
    CORRADE_COMPARE(c2[1], 2);
    CORRADE_COMPARE(c2[2], 3);

    ArrayView d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    constexpr ConstArrayView ca = Array5;
    constexpr ConstArrayView cb = ca.slice(1, 4);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);

    constexpr ConstArrayView cc1 = ca.prefix(3);
    CORRADE_COMPARE(cc1.size(), 3);
    CORRADE_COMPARE(cc1[0], 1);
    CORRADE_COMPARE(cc1[1], 2);
    CORRADE_COMPARE(cc1[2], 3);

    constexpr ConstArrayView cc2 = ca.except(2);
    CORRADE_COMPARE(cc2.size(), 3);
    CORRADE_COMPARE(cc2[0], 1);
    CORRADE_COMPARE(cc2[1], 2);
    CORRADE_COMPARE(cc2[2], 3);

    constexpr ConstArrayView cd = ca.suffix(2);
    CORRADE_COMPARE(cd.size(), 3);
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
}

void ArrayViewTest::slicePointer() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    ArrayView b = a.slice(data + 1, data + 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ArrayView c = a.prefix(data + 3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView d = a.suffix(data + 2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    /* MSVC 2015 chokes on all these due to (I assume) the assertion doing
       pointer arithmetic on the _data member. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr ConstArrayView cb = ca.slice(Array5 + 1, Array5 + 4);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);

    /* The slice function checks for validity of the pointers, taking one
       pointer from _data and the second pointer from the prefix() argument.
       GCC <= 5 chokes on that, because for it doing pointer arithmetic on
       _data is apparently not constexpr. Note that the above slice() call
       worked correctly on it (both pointers treated as constexpr?). */
    #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 5
    constexpr ConstArrayView cc = ca.prefix(Array5 + 3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    constexpr ConstArrayView cd = ca.suffix(Array5 + 2);
    CORRADE_COMPARE(cd.size(), 3);
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
    #endif
    #endif
}

void ArrayViewTest::sliceToStatic() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    StaticArrayView<3, int> b1 = a.slice<3>(1);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    StaticArrayView<3, int> b2 = a.slice<1, 4>();
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    StaticArrayView<3, int> c = a.prefix<3>();
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr StaticArrayView<3, const int> cb1 = ca.slice<3>(1);
    CORRADE_COMPARE(cb1[0], 2);
    CORRADE_COMPARE(cb1[1], 3);
    CORRADE_COMPARE(cb1[2], 4);

    constexpr StaticArrayView<3, const int> cb2 = ca.slice<1, 4>();
    CORRADE_COMPARE(cb2[0], 2);
    CORRADE_COMPARE(cb2[1], 3);
    CORRADE_COMPARE(cb2[2], 4);

    constexpr StaticArrayView<3, const int> cc = ca.prefix<3>();
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);
    #endif
}

void ArrayViewTest::sliceToStaticPointer() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr StaticArrayView<3, const int> cb = ca.slice<3>(ca + 1);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);
    #endif
}

void ArrayViewTest::cast() {
    std::uint32_t data[6]{};
    Containers::ArrayView<std::uint32_t> a = data;
    Containers::ArrayView<void> av = data;
    Containers::ArrayView<const void> cav = data;
    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto bv = Containers::arrayCast<std::uint64_t>(av);
    auto cbv = Containers::arrayCast<const std::uint64_t>(cav);
    auto c = Containers::arrayCast<std::uint16_t>(a);
    auto cv = Containers::arrayCast<std::uint16_t>(av);
    auto ccv = Containers::arrayCast<const std::uint16_t>(cav);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::ArrayView<std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(bv), Containers::ArrayView<std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cbv), Containers::ArrayView<const std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(c), Containers::ArrayView<std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cv), Containers::ArrayView<std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(ccv), Containers::ArrayView<const std::uint16_t>>::value));
    CORRADE_COMPARE(static_cast<void*>(b.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(bv.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cbv.begin()), static_cast<const void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(c.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(cv.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(ccv.begin()), static_cast<const void*>(a.begin()));
    CORRADE_COMPARE(a.size(), 6);
    CORRADE_COMPARE(av.size(), 6*4);
    CORRADE_COMPARE(cav.size(), 6*4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(bv.size(), 3);
    CORRADE_COMPARE(cbv.size(), 3);
    CORRADE_COMPARE(c.size(), 12);
    CORRADE_COMPARE(cv.size(), 12);
    CORRADE_COMPARE(ccv.size(), 12);
}

void ArrayViewTest::castInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    char data[10]{};
    Containers::ArrayView<char> a = data;
    Containers::ArrayView<void> av = data;
    Containers::ArrayView<const void> cav = data;

    auto b = Containers::arrayCast<std::uint16_t>(a);
    auto bv = Containers::arrayCast<std::uint16_t>(av);
    auto cbv = Containers::arrayCast<const std::uint16_t>(cav);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(cbv.size(), 5);

    {
        std::ostringstream out;
        Error redirectError{&out};
        Containers::arrayCast<std::uint32_t>(a);
        Containers::arrayCast<std::uint32_t>(av);
        Containers::arrayCast<const std::uint32_t>(cav);
        CORRADE_COMPARE(out.str(),
            "Containers::arrayCast(): can't reinterpret 10 1-byte items into a 4-byte type\n"
            "Containers::arrayCast(): can't reinterpret 10 bytes into a 4-byte type\n"
            "Containers::arrayCast(): can't reinterpret 10 bytes into a 4-byte type\n");
    }
}

void ArrayViewTest::size() {
    int a[6]{};
    ArrayView b{a, 3};
    ConstVoidArrayView c{a};

    CORRADE_COMPARE(Containers::arraySize(a), 6);
    CORRADE_COMPARE(Containers::arraySize(b), 3);
    CORRADE_COMPARE(Containers::arraySize(c), 24);

    constexpr ConstArrayView cb{Array13, 3};
    constexpr ConstVoidArrayView cc{Array13};
    constexpr std::size_t sizeA = Containers::arraySize(Array13);
    constexpr std::size_t sizeB = Containers::arraySize(cb);
    constexpr std::size_t sizeC = Containers::arraySize(cc);
    CORRADE_COMPARE(sizeA, 13);
    CORRADE_COMPARE(sizeB, 3);
    CORRADE_COMPARE(sizeC, 52);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewTest)
