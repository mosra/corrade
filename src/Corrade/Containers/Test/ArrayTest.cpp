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

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct IntView {
    IntView(int* data, std::size_t size): data{data}, size{size} {}

    int* data;
    std::size_t size;
};

struct ConstIntView {
    ConstIntView(const int* data, std::size_t size): data{data}, size{size} {}

    const int* data;
    std::size_t size;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct ArrayViewConverter<int, IntView> {
    static IntView to(ArrayView<int> other) {
        return {other.data(), other.size()};
    }
};

template<> struct ArrayViewConverter<const int, ConstIntView> {
    static ConstIntView to(ArrayView<const int> other) {
        return {other.data(), other.size()};
    }
};

}

namespace Test { namespace {

struct ArrayTest: TestSuite::Tester {
    explicit ArrayTest();

    void constructEmpty();
    void constructNullptr();
    void construct();
    void constructDefaultInit();
    void constructValueInit();
    void constructNoInitNonTrivial();
    void constructNoInitTrivial();
    void constructDirectInit();
    void constructInPlaceInit();
    void constructFromExisting();
    void constructZeroSize();
    void constructMove();
    void constructDirectReferences();

    void convertBool();
    void convertPointer();
    void convertView();
    void convertViewDerived();
    void convertViewOverload();
    void convertVoid();
    void convertConstVoid();
    void convertToExternalView();
    void convertToConstExternalView();

    void emptyCheck();
    void access();
    void accessConst();
    void accessInvalid();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();
    void release();

    void defaultDeleter();
    void customDeleter();
    void customDeleterType();
    void customDeleterTypeConstruct();

    void cast();
    void size();

    void emplaceConstructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;

ArrayTest::ArrayTest() {
    addTests({&ArrayTest::constructEmpty,
              &ArrayTest::constructNullptr,
              &ArrayTest::construct,
              &ArrayTest::constructDefaultInit,
              &ArrayTest::constructValueInit,
              &ArrayTest::constructNoInitNonTrivial,
              &ArrayTest::constructNoInitTrivial,
              &ArrayTest::constructDirectInit,
              &ArrayTest::constructInPlaceInit,
              &ArrayTest::constructFromExisting,
              &ArrayTest::constructZeroSize,
              &ArrayTest::constructMove,
              &ArrayTest::constructDirectReferences,

              &ArrayTest::convertBool,
              &ArrayTest::convertPointer,
              &ArrayTest::convertView,
              &ArrayTest::convertViewDerived,
              &ArrayTest::convertViewOverload,
              &ArrayTest::convertVoid,
              &ArrayTest::convertConstVoid,
              &ArrayTest::convertToExternalView,
              &ArrayTest::convertToConstExternalView,

              &ArrayTest::emptyCheck,
              &ArrayTest::access,
              &ArrayTest::accessConst,
              &ArrayTest::accessInvalid,
              &ArrayTest::rvalueArrayAccess,
              &ArrayTest::rangeBasedFor,

              &ArrayTest::slice,
              &ArrayTest::slicePointer,
              &ArrayTest::sliceToStatic,
              &ArrayTest::sliceToStaticPointer,
              &ArrayTest::release,

              &ArrayTest::defaultDeleter,
              &ArrayTest::customDeleter,
              &ArrayTest::customDeleterType,
              &ArrayTest::customDeleterTypeConstruct,

              &ArrayTest::cast,
              &ArrayTest::size,

              &ArrayTest::emplaceConstructorExplicitInCopyInitialization,
              &ArrayTest::copyConstructPlainStruct,
              &ArrayTest::moveConstructPlainStruct});
}

void ArrayTest::constructEmpty() {
    const Array a;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);

    /* Zero-length should not call new */
    const std::size_t size = 0;
    const Array b(size);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayTest::constructNullptr() {
    const Array c(nullptr);
    CORRADE_VERIFY(c == nullptr);
    CORRADE_COMPARE(c.size(), 0);

    /* Implicit construction from nullptr should be allowed */
    CORRADE_VERIFY((std::is_convertible<std::nullptr_t, Array>::value));
}

void ArrayTest::construct() {
    const Array a(5);
    CORRADE_VERIFY(a != nullptr);
    CORRADE_COMPARE(a.size(), 5);

    /* Values should be zero-initialized (same as ValueInit) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);

    /* Implicit construction from std::size_t is not allowed */
    CORRADE_VERIFY(!(std::is_convertible<std::size_t, Array>::value));
}

void ArrayTest::constructFromExisting() {
    int* a = new int[25];
    Array b{a, 25};
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 25);
}

void ArrayTest::constructDefaultInit() {
    const Array a{DefaultInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);

    /* Values are random memory */
}

void ArrayTest::constructValueInit() {
    const Array a{ValueInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);

    /* Values should be zero-initialized (same as the default constructor) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);
}

void ArrayTest::constructNoInitTrivial() {
    const Array a{NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_VERIFY(!a.deleter());
}

struct Foo {
    static int constructorCallCount;
    Foo() { ++constructorCallCount; }
};

int Foo::constructorCallCount = 0;

void ArrayTest::constructNoInitNonTrivial() {
    const Containers::Array<Foo> a{NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_VERIFY(a.deleter());
    CORRADE_COMPARE(Foo::constructorCallCount, 0);

    const Containers::Array<Foo> b{DefaultInit, 7};
    CORRADE_COMPARE(Foo::constructorCallCount, 7);
}

void ArrayTest::constructDirectInit() {
    const Array a{DirectInit, 2, -37};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], -37);
    CORRADE_COMPARE(a[1], -37);
}

void ArrayTest::constructInPlaceInit() {
    Array a1{InPlaceInit, {1, 3, 127, -48}};
    CORRADE_VERIFY(a1);
    CORRADE_COMPARE(a1.size(), 4);
    CORRADE_COMPARE(a1[0], 1);
    CORRADE_COMPARE(a1[1], 3);
    CORRADE_COMPARE(a1[2], 127);
    CORRADE_COMPARE(a1[3], -48);

    Array a2 = array<int>({1, 3, 127, -48});
    CORRADE_VERIFY(a2);
    CORRADE_COMPARE(a2.size(), 4);
    CORRADE_COMPARE(a2[0], 1);
    CORRADE_COMPARE(a2[1], 3);
    CORRADE_COMPARE(a2[2], 127);
    CORRADE_COMPARE(a2[3], -48);

    Array b1{InPlaceInit, {}};
    CORRADE_VERIFY(!b1);

    Array b2 = array<int>({});
    CORRADE_VERIFY(!b2);
}

void ArrayTest::constructZeroSize() {
    const Array a(0);

    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::constructMove() {
    auto myDeleter = [](int* data, std::size_t) { delete[] data; };
    Array a(new int[5], 5, myDeleter);
    CORRADE_VERIFY(a);
    const int* const ptr = a;

    Array b(std::move(a));
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == ptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_VERIFY(a.deleter() == nullptr);
    CORRADE_VERIFY(b.deleter() == myDeleter);

    auto noDeleter = [](int*, std::size_t) {};
    Array c{reinterpret_cast<int*>(0x3), 3, noDeleter};
    c = std::move(b);
    CORRADE_VERIFY(b == reinterpret_cast<int*>(0x3));
    CORRADE_VERIFY(c == ptr);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(c.size(), 5);
    CORRADE_VERIFY(b.deleter() == noDeleter);
    CORRADE_VERIFY(c.deleter() == myDeleter);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Array>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Array>::value);
}

void ArrayTest::constructDirectReferences() {
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

    const Containers::Array<Reference> b{Containers::DirectInit, 5, a};
    CORRADE_COMPARE(b.size(), 5);
}

void ArrayTest::convertBool() {
    CORRADE_VERIFY(Array(2));
    CORRADE_VERIFY(!Array());

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY((std::is_constructible<bool, Array>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, Array>::value));
}

void ArrayTest::convertPointer() {
    Array a(2);
    int* b = a;
    CORRADE_COMPARE(b, a.begin());

    const Array c(3);
    const int* d = c;
    CORRADE_COMPARE(d, c.begin());

    /* Pointer arithmetic */
    const Array e(3);
    const int* f = e + 2;
    CORRADE_COMPARE(f, &e[2]);

    /* Verify that we can't convert rvalues */
    CORRADE_VERIFY((std::is_convertible<Array&, int*>::value));
    CORRADE_VERIFY((std::is_convertible<const Array&, const int*>::value));
    CORRADE_VERIFY(!(std::is_convertible<Array, int*>::value));
    CORRADE_VERIFY(!(std::is_convertible<Array&&, int*>::value));

    /* Deleting const&& overload and leaving only const& one will not, in fact,
       disable conversion of const Array&& to pointer, but rather make the
       conversion ambiguous, which is not what we want, as it breaks e.g.
       rvalueArrayAccess() test. */
    {
        CORRADE_EXPECT_FAIL("I don't know how to properly disable conversion of const Array&& to pointer.");
        CORRADE_VERIFY(!(std::is_convertible<const Array, const int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<const Array&&, const int*>::value));
    }
}

void ArrayTest::convertView() {
    Array a(5);
    const Array ca(5);
    Containers::Array<const int> ac{a.data(), a.size(), [](const int*, std::size_t){}};
    const Containers::Array<const int> cac{a.data(), a.size(), [](const int*, std::size_t){}};

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

        ArrayView c = Array{3};
        CORRADE_COMPARE(c.size(), 3);
        /* The rest is a dangling pointer, can't test */
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

        auto c = arrayView(Array{3});
        CORRADE_VERIFY((std::is_same<decltype(c), ArrayView>::value));
        CORRADE_COMPARE(c.size(), 3);
        /* The rest is a dangling pointer, can't test */
    }
}
void ArrayTest::convertViewDerived() {
    struct A { int i; };
    struct B: A {};

    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    Containers::Array<B> b{5};
    Containers::ArrayView<A> a = b;

    CORRADE_VERIFY(a == b);
    CORRADE_COMPARE(a.size(), 5);
}

bool takesAView(Containers::ArrayView<int>) { return true; }
bool takesAConstView(Containers::ArrayView<const int>) { return true; }
CORRADE_UNUSED bool takesAView(Containers::ArrayView<std::pair<int, int>>) { return false; }
CORRADE_UNUSED bool takesAConstView(Containers::ArrayView<const std::pair<int, int>>) { return false; }

void ArrayTest::convertViewOverload() {
    Array a(5);
    const Array ca(5);

    /* It should pick the correct one and not fail, assert or be ambiguous */
    CORRADE_VERIFY(takesAView(a));
    CORRADE_VERIFY(takesAConstView(a));
    CORRADE_VERIFY(takesAConstView(ca));
}

void ArrayTest::convertVoid() {
    Array a(6);
    VoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), a.size()*sizeof(int));
}

void ArrayTest::convertConstVoid() {
    Array a(6);
    const Array ca(6);
    ConstVoidArrayView b = a;
    ConstVoidArrayView cb = ca;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(cb == ca);
    CORRADE_COMPARE(b.size(), a.size()*sizeof(int));
    CORRADE_COMPARE(cb.size(), ca.size()*sizeof(int));
}

void ArrayTest::convertToExternalView() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};

    IntView b = a;
    CORRADE_COMPARE(b.data, a);
    CORRADE_COMPARE(b.size, a.size());

    ConstIntView cb = a;
    CORRADE_COMPARE(cb.data, a);
    CORRADE_COMPARE(cb.size, a.size());

    /* Conversion to a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::Array<int>, IntView>::value));
    CORRADE_VERIFY((std::is_convertible<Containers::Array<int>, ConstIntView>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::Array<float>, IntView>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::Array<float>, ConstIntView>::value));
}

void ArrayTest::convertToConstExternalView() {
    const Array a{InPlaceInit, {1, 2, 3, 4, 5}};

    ConstIntView b = a;
    CORRADE_COMPARE(b.data, a);
    CORRADE_COMPARE(b.size, a.size());

    /* Conversion to a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<const Containers::Array<int>, ConstIntView>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Containers::Array<float>, ConstIntView>::value));
}

void ArrayTest::emptyCheck() {
    Array a;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(a.empty());

    Array b(5);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!b.empty());
}

void ArrayTest::access() {
    Array a(7);
    for(std::size_t i = 0; i != 7; ++i)
        a[i] = i;

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(a.front(), 0);
    CORRADE_COMPARE(a.back(), 6);
    CORRADE_COMPARE(*(a.begin()+2), 2);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a.end()-a.begin(), a.size());
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());

    const Array b{InPlaceInit, {7, 3, 5, 4}};
    CORRADE_COMPARE(b.data(), static_cast<const int*>(b));
    CORRADE_COMPARE(b[2], 5);
}

void ArrayTest::accessConst() {
    Array a(7);
    for(std::size_t i = 0; i != 7; ++i)
        a[i] = i;

    const Array& ca = a;
    CORRADE_COMPARE(ca.data(), static_cast<int*>(a));
    CORRADE_COMPARE(ca.front(), 0);
    CORRADE_COMPARE(ca.back(), 6);
    CORRADE_COMPARE(*(ca.begin()+2), 2);
    CORRADE_COMPARE(ca[4], 4);
    CORRADE_COMPARE(ca.end() - ca.begin(), ca.size());
    CORRADE_COMPARE(ca.cbegin(), ca.begin());
    CORRADE_COMPARE(ca.cend(), ca.end());
}

void ArrayTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::stringstream out;
    Error redirectError{&out};

    Array a;
    a.front();
    a.back();
    CORRADE_COMPARE(out.str(),
        "Containers::Array::front(): array is empty\n"
        "Containers::Array::back(): array is empty\n");
}

void ArrayTest::rvalueArrayAccess() {
    CORRADE_COMPARE((Array{InPlaceInit, {1, 2, 3, 4}}[2]), 3);
}

void ArrayTest::rangeBasedFor() {
    Array a(5);
    for(auto& i: a)
        i = 3;

    CORRADE_COMPARE(a[0], 3);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 3);

    /* To verify the constant begin()/end() accessors */
    const Array& ca = a;
    for(auto&& i: ca)
        CORRADE_COMPARE(i, 3);
}

void ArrayTest::slice() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{InPlaceInit, {1, 2, 3, 4, 5}};

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

    ArrayView c1 = a.prefix(3);
    CORRADE_COMPARE(c1.size(), 3);
    CORRADE_COMPARE(c1[0], 1);
    CORRADE_COMPARE(c1[1], 2);
    CORRADE_COMPARE(c1[2], 3);

    ConstArrayView cc1 = ac.prefix(3);
    CORRADE_COMPARE(cc1.size(), 3);
    CORRADE_COMPARE(cc1[0], 1);
    CORRADE_COMPARE(cc1[1], 2);
    CORRADE_COMPARE(cc1[2], 3);

    ArrayView c2 = a.except(2);
    CORRADE_COMPARE(c2.size(), 3);
    CORRADE_COMPARE(c2[0], 1);
    CORRADE_COMPARE(c2[1], 2);
    CORRADE_COMPARE(c2[2], 3);

    ConstArrayView cc2 = ac.except(2);
    CORRADE_COMPARE(cc2.size(), 3);
    CORRADE_COMPARE(cc2[0], 1);
    CORRADE_COMPARE(cc2[1], 2);
    CORRADE_COMPARE(cc2[2], 3);

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

void ArrayTest::slicePointer() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{InPlaceInit, {1, 2, 3, 4, 5}};

    ArrayView b = a.slice(a + 1, a + 4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    ConstArrayView bc = ac.slice(ac + 1, ac + 4);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);

    ArrayView c = a.prefix(a + 3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ConstArrayView cc = ac.prefix(ac + 3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    ArrayView d = a.suffix(a + 2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ConstArrayView dc = ac.suffix(ac + 2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);
}

void ArrayTest::sliceToStatic() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{InPlaceInit, {1, 2, 3, 4, 5}};

    StaticArrayView<3, int> b1 = a.slice<3>(1);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    StaticArrayView<3, const int> bc1 = ac.slice<3>(1);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    StaticArrayView<3, int> b2 = a.slice<1, 4>();
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    StaticArrayView<3, const int> bc2 = ac.slice<1, 4>();
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

    StaticArrayView<3, int> c = a.prefix<3>();
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    StaticArrayView<3, const int> cc = ac.prefix<3>();
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);
}

void ArrayTest::sliceToStaticPointer() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{InPlaceInit, {1, 2, 3, 4, 5}};

    StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StaticArrayView<3, const int> bc = ac.slice<3>(ac + 1);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);
}

void ArrayTest::release() {
    auto myDeleter = [](int* data, std::size_t) { delete[] data; };
    Array a(new int[5], 5, myDeleter);
    int* const data = a;
    int* const released = a.release();
    delete[] released;

    /* Not comparing pointers directly because then Clang Analyzer complains
       that printing the value of `released` is use-after-free. Um. */
    CORRADE_COMPARE(reinterpret_cast<std::intptr_t>(data), reinterpret_cast<std::intptr_t>(released));
    CORRADE_COMPARE(a.begin(), nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.deleter() == nullptr);
}

void ArrayTest::defaultDeleter() {
    Array a{5};
    CORRADE_VERIFY(a.deleter() == nullptr);
}

int CustomDeleterDeletedCount = 0;

void ArrayTest::customDeleter() {
    int data[25]{};

    {
        Array a{data, 25, [](int*, std::size_t size) { CustomDeleterDeletedCount = size; }};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterDeletedCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterDeletedCount, 25);
}

struct CustomDeleter {
    CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
    void operator()(int*, std::size_t size) { deletedCount = size; }
    int& deletedCount;
};

void ArrayTest::customDeleterType() {
    int data[25]{};
    int deletedCount = 0;

    {
        Containers::Array<int, CustomDeleter> a{data, 25, CustomDeleter{deletedCount}};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(deletedCount, 0);
    }

    CORRADE_COMPARE(deletedCount, 25);
}

void ArrayTest::customDeleterTypeConstruct() {
    struct CustomImplicitDeleter {
        void operator()(int*, std::size_t) {}
    };

    /* Just verify that this compiles */
    Containers::Array<int, CustomImplicitDeleter> a;
    Containers::Array<int, CustomImplicitDeleter> b{nullptr};
    int c;
    Containers::Array<int, CustomImplicitDeleter> d{&c, 1};
    CORRADE_VERIFY(true);
}

void ArrayTest::cast() {
    Containers::Array<std::uint32_t> a{6};
    const Containers::Array<std::uint32_t> ca{6};
    Containers::Array<const std::uint32_t> ac{a.data(), a.size(), [](const std::uint32_t*, std::size_t){}};
    const Containers::Array<const std::uint32_t> cac{a.data(), a.size(), [](const std::uint32_t*, std::size_t){}};

    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto bc = Containers::arrayCast<const std::uint64_t>(ac);
    auto cb = Containers::arrayCast<const std::uint64_t>(ca);
    auto cbc = Containers::arrayCast<const std::uint64_t>(cac);

    auto d = Containers::arrayCast<std::uint16_t>(a);
    auto dc = Containers::arrayCast<const std::uint16_t>(ac);
    auto cd = Containers::arrayCast<const std::uint16_t>(ca);
    auto cdc = Containers::arrayCast<const std::uint16_t>(cac);

    CORRADE_VERIFY((std::is_same<decltype(b), Containers::ArrayView<std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(bc), Containers::ArrayView<const std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cb), Containers::ArrayView<const std::uint64_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cbc), Containers::ArrayView<const std::uint64_t>>::value));

    CORRADE_VERIFY((std::is_same<decltype(d), Containers::ArrayView<std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cd), Containers::ArrayView<const std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(dc), Containers::ArrayView<const std::uint16_t>>::value));
    CORRADE_VERIFY((std::is_same<decltype(cdc), Containers::ArrayView<const std::uint16_t>>::value));

    CORRADE_COMPARE(static_cast<void*>(b.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cb.begin()), static_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(static_cast<const void*>(bc.begin()), static_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cbc.begin()), static_cast<const void*>(cac.begin()));

    CORRADE_COMPARE(static_cast<void*>(d.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cd.begin()), static_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(static_cast<const void*>(dc.begin()), static_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cdc.begin()), static_cast<const void*>(cac.begin()));

    CORRADE_COMPARE(a.size(), 6);
    CORRADE_COMPARE(ca.size(), 6);
    CORRADE_COMPARE(ac.size(), 6);
    CORRADE_COMPARE(cac.size(), 6);

    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(cbc.size(), 3);

    CORRADE_COMPARE(d.size(), 12);
    CORRADE_COMPARE(cd.size(), 12);
    CORRADE_COMPARE(dc.size(), 12);
    CORRADE_COMPARE(cdc.size(), 12);
}

void ArrayTest::size() {
    Array a{3};

    CORRADE_COMPARE(Containers::arraySize(a), 3);
}

void ArrayTest::emplaceConstructorExplicitInCopyInitialization() {
    /* See constructHelpers.h for details about this compiler-specific issue */
    struct ExplicitDefault {
        explicit ExplicitDefault() = default;
    };

    struct ContainingExplicitDefaultWithImplicitConstructor {
        ExplicitDefault a;
    };

    /* This alone works */
    ContainingExplicitDefaultWithImplicitConstructor a;
    static_cast<void>(a);

    /* So this should too */
    Containers::Array<ContainingExplicitDefaultWithImplicitConstructor> b{DirectInit, 5};
    CORRADE_COMPARE(b.size(), 5);
}

void ArrayTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    Containers::Array<ExtremelyTrivial> a{DirectInit, 2, 3, 'a'};
    CORRADE_COMPARE(a.size(), 2);

    /* This copy-constructs the new values */
    Containers::Array<ExtremelyTrivial> b{InPlaceInit, {
        {4, 'b'},
        {5, 'c'},
        {6, 'd'}
    }};
    CORRADE_COMPARE(b.size(), 3);
}

void ArrayTest::moveConstructPlainStruct() {
    struct MoveOnlyStruct {
        int a;
        char c;
        Array b;
    };

    /* This needs special handling on GCC 4.8, where T{std::move(b)} attempts
       to convert MoveOnlyStruct to int to initialize the first argument and
       fails miserably. */
    Containers::Array<MoveOnlyStruct> a{DirectInit, 2, 3, 'a', nullptr};
    CORRADE_COMPARE(a.size(), 2);

    /* Unlike with copyConstructPlainStruct(), the InPlaceInit doesn't use
       move-construction, so that's not affected */
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayTest)
