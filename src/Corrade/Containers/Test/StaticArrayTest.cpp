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

#include "Corrade/Containers/StaticArray.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

struct StaticArrayTest: TestSuite::Tester {
    explicit StaticArrayTest();

    void construct();
    void constructInPlace();
    void constructInPlaceOneArgument();
    void constructValueInit();
    void constructNoInit();
    void constructDirectInit();
    void constructNonCopyable();
    void constructNoImplicitConstructor();
    void constructDirectReferences();

    void convertBool();
    void convertPointer();
    void convertView();
    void convertViewDerived();
    void convertStaticView();
    void convertStaticViewDerived();
    void convertVoid();

    void access();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void sliceToStatic();

    void cast();
    void size();
};

typedef Containers::StaticArray<5, int> StaticArray;
typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<const void> VoidArrayView;
typedef Containers::StaticArrayView<5, int> StaticArrayView;
typedef Containers::StaticArrayView<5, const int> ConstStaticArrayView;

StaticArrayTest::StaticArrayTest() {
    addTests({&StaticArrayTest::construct,
              &StaticArrayTest::constructInPlace,
              &StaticArrayTest::constructInPlaceOneArgument,
              &StaticArrayTest::constructValueInit,
              &StaticArrayTest::constructNoInit,
              &StaticArrayTest::constructDirectInit,
              &StaticArrayTest::constructNonCopyable,
              &StaticArrayTest::constructNoImplicitConstructor,
              &StaticArrayTest::constructDirectReferences,

              &StaticArrayTest::convertBool,
              &StaticArrayTest::convertPointer,
              &StaticArrayTest::convertView,
              &StaticArrayTest::convertViewDerived,
              &StaticArrayTest::convertStaticView,
              &StaticArrayTest::convertStaticViewDerived,
              &StaticArrayTest::convertVoid,

              &StaticArrayTest::access,
              &StaticArrayTest::rvalueArrayAccess,
              &StaticArrayTest::rangeBasedFor,

              &StaticArrayTest::slice,
              &StaticArrayTest::sliceToStatic,

              &StaticArrayTest::cast,
              &StaticArrayTest::size});
}

void StaticArrayTest::construct() {
    const StaticArray a;
    const StaticArray b{DefaultInit};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!a.empty());
    CORRADE_VERIFY(!b.empty());
    CORRADE_COMPARE(a.size(), StaticArray::Size);
    CORRADE_COMPARE(b.size(), StaticArray::Size);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(b.size(), 5);
}

void StaticArrayTest::constructInPlace() {
    const StaticArray a{1, 2, 3, 4, 5};
    const StaticArray b{InPlaceInit, 1, 2, 3, 4, 5};

    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(a[1], 2);
    CORRADE_COMPARE(b[1], 2);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(a[3], 4);
    CORRADE_COMPARE(b[3], 4);
    CORRADE_COMPARE(a[4], 5);
    CORRADE_COMPARE(b[4], 5);
}

void StaticArrayTest::constructInPlaceOneArgument() {
    const Containers::StaticArray<1, int> a{17};
    CORRADE_COMPARE(a[0], 17);
}

void StaticArrayTest::constructValueInit() {
    const StaticArray a{ValueInit};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);
}

namespace {
    struct Foo {
        static int constructorCallCount;
        static int destructorCallCount;
        Foo() { ++constructorCallCount; }
        ~Foo() { ++destructorCallCount; }
    };

    int Foo::constructorCallCount = 0;
    int Foo::destructorCallCount = 0;
}

void StaticArrayTest::constructNoInit() {
    {
        const Containers::StaticArray<5, Foo> a{NoInit};
        CORRADE_COMPARE(Foo::constructorCallCount, 0);

        const Containers::StaticArray<5, Foo> b{DefaultInit};
        CORRADE_COMPARE(Foo::constructorCallCount, 5);
    }

    CORRADE_COMPARE(Foo::destructorCallCount, 10);
}

void StaticArrayTest::constructDirectInit() {
    const StaticArray a{DirectInit, -37};
    CORRADE_COMPARE(a[0], -37);
    CORRADE_COMPARE(a[1], -37);
    CORRADE_COMPARE(a[2], -37);
    CORRADE_COMPARE(a[3], -37);
    CORRADE_COMPARE(a[4], -37);
}

void StaticArrayTest::constructNonCopyable() {
    struct NonCopyable {
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = delete;

        NonCopyable() {}

        /* to make it non-trivial to hit
           https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70395 */
        ~NonCopyable() {}
    };

    const Containers::StaticArray<5, NonCopyable> a;
    CORRADE_VERIFY(a);
}

void StaticArrayTest::constructNoImplicitConstructor() {
    struct NoImplicitConstructor {
        NoImplicitConstructor(int i): i{i} {}

        int i;
    };

    const Containers::StaticArray<5, NoImplicitConstructor> a{Containers::DirectInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a[0].i, 5);
    CORRADE_COMPARE(a[1].i, 5);
    CORRADE_COMPARE(a[2].i, 5);
    CORRADE_COMPARE(a[3].i, 5);
    CORRADE_COMPARE(a[4].i, 5);

    const Containers::StaticArray<5, NoImplicitConstructor> b{Containers::InPlaceInit, 1, 2, 3, 4, 5};
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b[0].i, 1);
    CORRADE_COMPARE(b[1].i, 2);
    CORRADE_COMPARE(b[2].i, 3);
    CORRADE_COMPARE(b[3].i, 4);
    CORRADE_COMPARE(b[4].i, 5);
}

void StaticArrayTest::constructDirectReferences() {
    struct NonCopyable {
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = delete;
        NonCopyable() = default;
    } a;

    struct Reference {
        Reference(NonCopyable&) {}
    };

    const Containers::StaticArray<5, Reference> b{Containers::DirectInit, a};
    CORRADE_VERIFY(b);
}

void StaticArrayTest::convertBool() {
    CORRADE_VERIFY(StaticArray{});
    CORRADE_VERIFY(!(std::is_convertible<StaticArray, int>::value));
}

void StaticArrayTest::convertPointer() {
    StaticArray a;
    int* b = a;
    CORRADE_COMPARE(b, a.begin());

    const StaticArray c;
    const int* d = c;
    CORRADE_COMPARE(d, c.begin());

    /* Pointer arithmetic */
    const StaticArray e;
    const int* f = e + 2;
    CORRADE_COMPARE(f, &e[2]);

    /* Verify that we can't convert rvalues */
    CORRADE_VERIFY((std::is_convertible<StaticArray&, int*>::value));
    CORRADE_VERIFY((std::is_convertible<const StaticArray&, const int*>::value));
    {
        #ifdef CORRADE_GCC47_COMPATIBILITY
        CORRADE_EXPECT_FAIL("Rvalue references for *this are not supported in GCC < 4.8.1.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<StaticArray, int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<StaticArray&&, int*>::value));
    }

    /* Deleting const&& overload and leaving only const& one will not, in fact,
       disable conversion of const Array&& to pointer, but rather make the
       conversion ambiguous, which is not what we want, as it breaks e.g.
       rvalueArrayAccess() test. */
    {
        CORRADE_EXPECT_FAIL("I don't know how to properly disable conversion of const Array&& to pointer.");
        CORRADE_VERIFY(!(std::is_convertible<const StaticArray, const int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<const StaticArray&&, const int*>::value));
    }
}

void StaticArrayTest::convertView() {
    StaticArray a;
    const StaticArray ca;
    Containers::StaticArray<5, const int> ac;
    const Containers::StaticArray<5, const int> cac;

    {
        const ArrayView b = a;
        const ConstArrayView cb = ca;
        const ConstArrayView bc = ac;
        const ConstArrayView cbc = cac;
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        const auto b = arrayView(a);
        const auto cb = arrayView(ca);
        const auto bc = arrayView(ac);
        const auto cbc = arrayView(cac);
        CORRADE_VERIFY((std::is_same<decltype(b), const ArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(bc), const ConstArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(cbc), const ConstArrayView>::value));
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    }
}

void StaticArrayTest::convertViewDerived() {
    struct A { int i; };
    struct B: A {};

    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    Containers::StaticArray<5, B> b;
    Containers::ArrayView<A> a = b;

    CORRADE_VERIFY(a == b);
    CORRADE_COMPARE(a.size(), 5);
}

void StaticArrayTest::convertStaticView() {
    StaticArray a;
    const StaticArray ca;
    Containers::StaticArray<5, const int> ac;
    const Containers::StaticArray<5, const int> cac;

    {
        const StaticArrayView b = a;
        const ConstStaticArrayView cb = ca;
        const ConstStaticArrayView bc = ac;
        const ConstStaticArrayView cbc = cac;
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        const auto b = staticArrayView(a);
        const auto cb = staticArrayView(ca);
        const auto bc = staticArrayView(ac);
        const auto cbc = staticArrayView(cac);
        CORRADE_VERIFY((std::is_same<decltype(b), const StaticArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(cb), const ConstStaticArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(bc), const ConstStaticArrayView>::value));
        CORRADE_VERIFY((std::is_same<decltype(cbc), const ConstStaticArrayView>::value));
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    }
}

void StaticArrayTest::convertStaticViewDerived() {
    struct A { int i; };
    struct B: A {};

    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    Containers::StaticArray<5, B> b;
    Containers::StaticArrayView<5, A> a = b;

    CORRADE_VERIFY(a == b);
    CORRADE_COMPARE(a.size(), 5);
}

void StaticArrayTest::convertVoid() {
    /* void reference to Array */
    StaticArray a;
    const StaticArray ca;
    VoidArrayView b = a;
    VoidArrayView cb = ca;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(cb == ca);
    CORRADE_COMPARE(b.size(), 5*sizeof(int));
    CORRADE_COMPARE(cb.size(), 5*sizeof(int));
}

void StaticArrayTest::access() {
    StaticArray a;
    for(std::size_t i = 0; i != 5; ++i)
        a[i] = i;

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(*(a.begin()+2), 2);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a.end()-a.begin(), 5);
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());
}

void StaticArrayTest::rvalueArrayAccess() {
    CORRADE_COMPARE((StaticArray{DirectInit, 3})[2], 3);
}

void StaticArrayTest::rangeBasedFor() {
    StaticArray a;
    for(auto& i: a)
        i = 3;

    CORRADE_COMPARE(a[0], 3);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 3);

    /* To verify the constant begin()/end() accessors */
    const StaticArray& ca = a;
    for(auto&& i: ca)
        CORRADE_COMPARE(i, 3);
}

void StaticArrayTest::slice() {
    StaticArray a{InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray ac{InPlaceInit, 1, 2, 3, 4, 5};

    ArrayView b = a.slice(1, 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ConstArrayView bc = ac.slice(1, 4);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);

    ArrayView c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ConstArrayView cc = ac.prefix(3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    ArrayView d = a.suffix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ConstArrayView dc = ac.suffix(2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);
}

void StaticArrayTest::sliceToStatic() {
    StaticArray a{InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray ac{InPlaceInit, 1, 2, 3, 4, 5};

    Containers::StaticArrayView<3, int> b = a.slice<3>(1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    Containers::StaticArrayView<3, const int> bc = ac.slice<3>(1);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);
}

void StaticArrayTest::cast() {
    Containers::StaticArray<6, std::uint32_t> a;
    const Containers::StaticArray<6, std::uint32_t> ca;
    Containers::StaticArray<6, const std::uint32_t> ac;
    const Containers::StaticArray<6, const std::uint32_t> cac;

    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto bc = Containers::arrayCast<const std::uint64_t>(ac);
    auto cb = Containers::arrayCast<const std::uint64_t>(ca);
    auto cbc = Containers::arrayCast<const std::uint64_t>(cac);

    auto d = Containers::arrayCast<std::uint16_t>(a);
    auto dc = Containers::arrayCast<const std::uint16_t>(ac);
    auto cd = Containers::arrayCast<const std::uint16_t>(ca);
    auto cdc = Containers::arrayCast<const std::uint16_t>(cac);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::StaticArrayView<3, std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(bc), Containers::StaticArrayView<3, const std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cb), Containers::StaticArrayView<3, const std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cbc), Containers::StaticArrayView<3, const std::uint64_t>>::value));

    CORRADE_VERIFY((std::is_same<decltype(d), Containers::StaticArrayView<12,  std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cd), Containers::StaticArrayView<12, const std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(dc), Containers::StaticArrayView<12, const std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cdc), Containers::StaticArrayView<12, const std::uint16_t>>::value));

    CORRADE_COMPARE(reinterpret_cast<void*>(b.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cb.begin()), reinterpret_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(bc.begin()), reinterpret_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cbc.begin()), reinterpret_cast<const void*>(cac.begin()));

    CORRADE_COMPARE(reinterpret_cast<void*>(d.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cd.begin()), reinterpret_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(dc.begin()), reinterpret_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cdc.begin()), reinterpret_cast<const void*>(cac.begin()));
}

void StaticArrayTest::size() {
    StaticArray a;

    CORRADE_COMPARE(Containers::arraySize(a), 5);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayTest)
