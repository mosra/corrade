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

namespace {

struct IntView5 {
    explicit IntView5(int* data): data{data} {}

    int* data;
};

struct ConstIntView5 {
    constexpr explicit ConstIntView5(const int* data): data{data} {}

    const int* data;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct StaticArrayViewConverter<5, const int, ConstIntView5> {
    constexpr static StaticArrayView<5, const int> from(ConstIntView5 other) {
        return StaticArrayView<5, const int>{other.data};
    }

    constexpr static ConstIntView5 to(StaticArrayView<5, const int> other) {
        return ConstIntView5{other.data()};
    }
};

template<> struct ErasedStaticArrayViewConverter<ConstIntView5>: StaticArrayViewConverter<5, const int, ConstIntView5> {};
template<> struct ErasedStaticArrayViewConverter<const ConstIntView5>: StaticArrayViewConverter<5, const int, ConstIntView5> {};

/* To keep the ArrayView API in reasonable bounds, the cost-adding variants
   have to be implemented explicitly */
template<> struct StaticArrayViewConverter<5, const int, IntView5> {
    static StaticArrayView<5, const int> from(IntView5 other) {
        return StaticArrayView<5, const int>{other.data};
    }
};
template<> struct StaticArrayViewConverter<5, int, ConstIntView5> {
    static ConstIntView5 to(StaticArrayView<5, int> other) {
        return ConstIntView5{other.data()};
    }
};

}

namespace Test { namespace {

struct StaticArrayViewTest: TestSuite::Tester {
    explicit StaticArrayViewTest();

    void constructDefault();
    void constructNullptr();
    void construct();
    void constructFixedSize();
    void constructDerived();

    void convertBool();
    void convertPointer();
    void convertConst();
    void convertExternalView();
    void convertConstFromExternalView();
    void convertToConstExternalView();

    void access();
    void accessConst();
    void rangeBasedFor();

    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();

    void cast();
    void size();
};

typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
template<std::size_t size> using StaticArrayView = Containers::StaticArrayView<size, int>;
template<std::size_t size> using ConstStaticArrayView = Containers::StaticArrayView<size, const int>;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;

StaticArrayViewTest::StaticArrayViewTest() {
    addTests({&StaticArrayViewTest::constructDefault,
              &StaticArrayViewTest::constructNullptr,
              &StaticArrayViewTest::construct,
              &StaticArrayViewTest::constructFixedSize,
              &StaticArrayViewTest::constructDerived,

              &StaticArrayViewTest::convertBool,
              &StaticArrayViewTest::convertPointer,
              &StaticArrayViewTest::convertConst,
              &StaticArrayViewTest::convertExternalView,
              &StaticArrayViewTest::convertConstFromExternalView,
              &StaticArrayViewTest::convertToConstExternalView,

              &StaticArrayViewTest::access,
              &StaticArrayViewTest::accessConst,
              &StaticArrayViewTest::rangeBasedFor,

              &StaticArrayViewTest::slice,
              &StaticArrayViewTest::slicePointer,
              &StaticArrayViewTest::sliceToStatic,
              &StaticArrayViewTest::sliceToStaticPointer,

              &StaticArrayViewTest::cast,
              &StaticArrayViewTest::size});
}

void StaticArrayViewTest::constructDefault() {
    StaticArrayView<5> a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(!a.empty());
    CORRADE_COMPARE(a.size(), StaticArrayView<5>::Size);
    CORRADE_COMPARE(a.size(), 5);

    constexpr StaticArrayView<5> ca;
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(!ca.empty());
    CORRADE_COMPARE(ca.size(), StaticArrayView<5>::Size);
    CORRADE_COMPARE(ca.size(), 5);
}

void StaticArrayViewTest::constructNullptr() {
    const StaticArrayView<5> a = nullptr;
    CORRADE_VERIFY(a == nullptr);

    constexpr StaticArrayView<5> ca = nullptr;
    CORRADE_VERIFY(ca == nullptr);
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array30[30]{};

void StaticArrayViewTest::construct() {
    int a[30];

    {
        const StaticArrayView<5> b{a};
        CORRADE_VERIFY(b == a);
    } {
        auto b = staticArrayView<5>(a);
        CORRADE_VERIFY((std::is_same<decltype(b), StaticArrayView<5>>::value));
        CORRADE_VERIFY(b == a);

        auto c = staticArrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), StaticArrayView<5>>::value));
        CORRADE_VERIFY(c == a);
    }

    {
        constexpr ConstStaticArrayView<5> b{Array30};
        CORRADE_VERIFY(b == Array30);
    } {
        constexpr auto b = staticArrayView<5>(Array30);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstStaticArrayView<5>>::value));
        CORRADE_VERIFY(b == Array30);

        constexpr auto c = staticArrayView(b);
        CORRADE_VERIFY((std::is_same<decltype(c), const ConstStaticArrayView<5>>::value));
        CORRADE_VERIFY(c == Array30);
    }

    /* Implicit construction from pointer should not be allowed */
    CORRADE_VERIFY(!(std::is_convertible<int*, StaticArrayView<5>>::value));
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array13[13]{};

void StaticArrayViewTest::constructFixedSize() {
    int a[13];

    {
        StaticArrayView<13> b = a;
        CORRADE_VERIFY(b == a);
    } {
        auto b = staticArrayView(a);
        CORRADE_VERIFY((std::is_same<decltype(b), StaticArrayView<13>>::value));
        CORRADE_VERIFY(b == a);
    }

    {
        constexpr ConstStaticArrayView<13> b = Array13;
        CORRADE_VERIFY(b == Array13);
    } {
        constexpr auto b = staticArrayView(Array13);
        CORRADE_VERIFY((std::is_same<decltype(b), const ConstStaticArrayView<13>>::value));
        CORRADE_VERIFY(b == Array13);
    }
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

void StaticArrayViewTest::constructDerived() {
    /* See ArrayViewTest for comments */

    Derived b[5];
    Containers::StaticArrayView<5, Derived> bv{b};
    Containers::StaticArrayView<5, Base> a{b};
    Containers::StaticArrayView<5, Base> av{bv};

    CORRADE_VERIFY(a == &b[0]);
    CORRADE_VERIFY(av == &b[0]);

    constexpr Containers::StaticArrayView<5, const Derived> cbv{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StaticArrayView<5, const Base> ca{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::StaticArrayView<5, const Base> cav{cbv};

    CORRADE_VERIFY(ca == &DerivedArray[0]);
    CORRADE_VERIFY(cav == &DerivedArray[0]);
}

void StaticArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(StaticArrayView<5>{a});
    CORRADE_VERIFY(!StaticArrayView<5>{});

    constexpr ConstStaticArrayView<30> cb = Array30;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(boolCb);

    constexpr ConstStaticArrayView<30> cc;
    constexpr bool boolCc = !!cc;
    CORRADE_VERIFY(!boolCc);

    CORRADE_VERIFY((std::is_constructible<bool, StaticArrayView<5>>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, StaticArrayView<5>>::value));
}

void StaticArrayViewTest::convertPointer() {
    int a[7];
    StaticArrayView<7> b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const StaticArrayView<7> c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    constexpr ConstStaticArrayView<13> cc = Array13;
    constexpr const int* ccp = cc;
    CORRADE_COMPARE(ccp, static_cast<const int*>(Array13));

    /* Pointer arithmetic */
    const StaticArrayView<7> e = a;
    const int* ep = e + 2;
    CORRADE_COMPARE(ep, &e[2]);
}

void StaticArrayViewTest::convertConst() {
    int a[3];
    StaticArrayView<3> b = a;
    ConstArrayView c = b;
    CORRADE_VERIFY(c == a);
}

void StaticArrayViewTest::convertExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView5 a{data};
    CORRADE_COMPARE(a.data, data);

    ConstStaticArrayView<5> b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    ConstIntView5 c = b;
    CORRADE_COMPARE(c.data, data);

    auto d = staticArrayView(c);
    CORRADE_VERIFY((std::is_same<decltype(d), Containers::StaticArrayView<5, const int>>::value));
    CORRADE_COMPARE(d.data(), data);
    CORRADE_COMPARE(d.size(), 5);

    constexpr ConstIntView5 ca{Array13};
    CORRADE_COMPARE(ca.data, Array13);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstStaticArrayView<5> cb = ca;
    CORRADE_COMPARE(cb.data(), Array13);
    CORRADE_COMPARE(cb.size(), 5);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #endif
    ConstIntView5 cc = cb;
    CORRADE_COMPARE(cc.data, Array13);

    /* Broken on Clang 3.8-svn on Apple. The same works with stock Clang 3.8
       (Travis ASan build). ¯\_(ツ)_/¯ */
    #if !defined(CORRADE_TARGET_APPLE_CLANG) || __clang_major__*100 + __clang_minor__ > 703
    constexpr
    #else
    const
    #endif
    auto cd = staticArrayView(cc);
    CORRADE_VERIFY((std::is_same<decltype(cd), const Containers::StaticArrayView<5, const int>>::value));
    CORRADE_COMPARE(cd.data(), Array13);
    CORRADE_COMPARE(cd.size(), 5);

    /* Conversion to a different size or type is not allowed */
    CORRADE_VERIFY((std::is_convertible<ConstIntView5, Containers::StaticArrayView<5, const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<ConstIntView5, Containers::StaticArrayView<6, const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<ConstIntView5, Containers::StaticArrayView<5, const float>>::value));
    CORRADE_VERIFY((std::is_convertible<Containers::StaticArrayView<5, const int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArrayView<6, const int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArrayView<5, const float>, ConstIntView5>::value));
}

void StaticArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView5 a{data};
    CORRADE_COMPARE(a.data, &data[0]);

    ConstStaticArrayView<5> b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion from a different size or type is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntView5, Containers::StaticArrayView<5, const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView5, Containers::StaticArrayView<6, const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntView5, Containers::StaticArrayView<5, const float>>::value));
}

void StaticArrayViewTest::convertToConstExternalView() {
    int data[]{1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;
    CORRADE_COMPARE(a.data(), &data[0]);
    CORRADE_COMPARE(a.size(), 5);

    ConstIntView5 b = a;
    CORRADE_COMPARE(b.data, data);

    /* Conversion to a different size or type is not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::StaticArrayView<5, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArrayView<6, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArrayView<5, float>, ConstIntView5>::value));
}

/* Needs to be here in order to use it in constexpr */
constexpr int OneToSeven[]{0, 1, 2, 3, 4, 5, 6};

void StaticArrayViewTest::access() {
    int a[7];
    StaticArrayView<7> b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), 7);
    CORRADE_COMPARE(b.cbegin(), b.begin());
    CORRADE_COMPARE(b.cend(), b.end());

    ConstStaticArrayView<7> c = a;
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstStaticArrayView<7> cb = OneToSeven;

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

void StaticArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const StaticArrayView<7> b = a;
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

void StaticArrayViewTest::rangeBasedFor() {
    int a[5];
    StaticArrayView<5> b = a;
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

constexpr int Array5[]{1, 2, 3, 4, 5};

void StaticArrayViewTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

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

    constexpr ConstStaticArrayView<5> ca = Array5;
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

void StaticArrayViewTest::slicePointer() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

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
    constexpr ConstStaticArrayView<5> ca = Array5;
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

void StaticArrayViewTest::sliceToStatic() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

    StaticArrayView<3> b1 = a.slice<3>(1);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    StaticArrayView<3> b2 = a.slice<1, 4>();
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    StaticArrayView<3> c1 = a.prefix<3>();
    CORRADE_COMPARE(c1[0], 1);
    CORRADE_COMPARE(c1[1], 2);
    CORRADE_COMPARE(c1[2], 3);

    StaticArrayView<3> c2 = a.except<2>();
    CORRADE_COMPARE(c2[0], 1);
    CORRADE_COMPARE(c2[1], 2);
    CORRADE_COMPARE(c2[2], 3);

    StaticArrayView<3> d = a.suffix<2>();
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    constexpr ConstStaticArrayView<5> ca = Array5;
    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. Note that the below
       works, as there's no runtime assertion. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstStaticArrayView<3> cb1 = ca.slice<3>(1);
    CORRADE_COMPARE(cb1[0], 2);
    CORRADE_COMPARE(cb1[1], 3);
    CORRADE_COMPARE(cb1[2], 4);
    #endif

    constexpr ConstStaticArrayView<3> cb2 = ca.slice<1, 4>();
    CORRADE_COMPARE(cb2[0], 2);
    CORRADE_COMPARE(cb2[1], 3);
    CORRADE_COMPARE(cb2[2], 4);

    constexpr ConstStaticArrayView<3> cc1 = ca.prefix<3>();
    CORRADE_COMPARE(cc1[0], 1);
    CORRADE_COMPARE(cc1[1], 2);
    CORRADE_COMPARE(cc1[2], 3);

    constexpr ConstStaticArrayView<3> cc2 = ca.except<2>();
    CORRADE_COMPARE(cc2[0], 1);
    CORRADE_COMPARE(cc2[1], 2);
    CORRADE_COMPARE(cc2[2], 3);

    constexpr ConstStaticArrayView<3> cd = ca.suffix<2>();
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
}

void StaticArrayViewTest::sliceToStaticPointer() {
    int data[5] = {1, 2, 3, 4, 5};
    StaticArrayView<5> a = data;

    StaticArrayView<3> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstStaticArrayView<5> ca = Array5;
    constexpr ConstStaticArrayView<3> cb = ca.slice<3>(ca + 1);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);
    #endif
}

void StaticArrayViewTest::cast() {
    std::uint32_t data[6]{};
    Containers::StaticArrayView<6, std::uint32_t> a = data;
    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto c = Containers::arrayCast<std::uint16_t>(a);
    auto d = Containers::arrayCast<std::uint16_t>(data);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::StaticArrayView<3, std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(c), Containers::StaticArrayView<12, std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(d), Containers::StaticArrayView<12, std::uint16_t>>::value));
    CORRADE_COMPARE(reinterpret_cast<void*>(b.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<void*>(c.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<void*>(d.begin()), reinterpret_cast<void*>(a.begin()));
}

void StaticArrayViewTest::size() {
    int a[6]{};
    StaticArrayView<3> b{a};

    CORRADE_COMPARE(Containers::arraySize(b), 3);

    constexpr ConstStaticArrayView<3> cb{Array13};
    constexpr std::size_t size = Containers::arraySize(cb);
    CORRADE_COMPARE(size, 3);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayViewTest)
