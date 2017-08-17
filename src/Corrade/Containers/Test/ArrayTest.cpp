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

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test {

struct ArrayTest: TestSuite::Tester {
    explicit ArrayTest();

    void constructEmpty();
    void constructNullptr();
    void constructDefaultInit();
    void constructValueInit();
    void constructNoInit();
    void constructDirectInit();
    void constructInPlaceInit();
    void construct();
    void constructFromExisting();
    void constructZeroSize();
    void constructMove();
    void constructDirectReferences();

    void convertBool();
    void convertPointer();
    void convertView();
    void convertViewDerived();
    void convertVoid();

    void emptyCheck();
    void access();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void sliceToStatic();
    void release();

    void defaultDeleter();
    void customDeleter();
    void customDeleterType();
    void customDeleterTypeConstruct();

    void cast();
    void size();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<const void> VoidArrayView;

ArrayTest::ArrayTest() {
    addTests({&ArrayTest::constructEmpty,
              &ArrayTest::constructNullptr,
              &ArrayTest::constructDefaultInit,
              &ArrayTest::constructValueInit,
              &ArrayTest::constructNoInit,
              &ArrayTest::constructDirectInit,
              &ArrayTest::constructInPlaceInit,
              &ArrayTest::construct,
              &ArrayTest::constructFromExisting,
              &ArrayTest::constructZeroSize,
              &ArrayTest::constructMove,
              &ArrayTest::constructDirectReferences,

              &ArrayTest::convertBool,
              &ArrayTest::convertPointer,
              &ArrayTest::convertView,
              &ArrayTest::convertViewDerived,
              &ArrayTest::convertVoid,

              &ArrayTest::emptyCheck,
              &ArrayTest::access,
              &ArrayTest::rvalueArrayAccess,
              &ArrayTest::rangeBasedFor,

              &ArrayTest::slice,
              &ArrayTest::sliceToStatic,
              &ArrayTest::release,

              &ArrayTest::defaultDeleter,
              &ArrayTest::customDeleter,
              &ArrayTest::customDeleterType,
              &ArrayTest::customDeleterTypeConstruct,

              &ArrayTest::cast,
              &ArrayTest::size});
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
}

void ArrayTest::constructValueInit() {
    const Array a{ValueInit, 2};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
}

namespace {
    struct Foo {
        static int constructorCallCount;
        Foo() { ++constructorCallCount; }
    };

    int Foo::constructorCallCount = 0;
}

void ArrayTest::constructNoInit() {
    const Containers::Array<Foo> a{NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
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
    Array a{InPlaceInit, {1, 3, 127, -48}};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 4);
    CORRADE_COMPARE(a[0], 1);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 127);
    CORRADE_COMPARE(a[3], -48);

    Array b{InPlaceInit, {}};
    CORRADE_VERIFY(!b);
}

void ArrayTest::constructZeroSize() {
    const Array a(0);

    CORRADE_VERIFY(a == nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::constructMove() {
    Array a(5);
    CORRADE_VERIFY(a);
    const int* const ptr = a;

    Array b(std::move(a));
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == ptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 5);

    Array c;
    c = std::move(b);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_VERIFY(c == ptr);
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(c.size(), 5);
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
    CORRADE_VERIFY(!(std::is_convertible<Array, int>::value));
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
    {
        #ifdef CORRADE_GCC47_COMPATIBILITY
        CORRADE_EXPECT_FAIL("Rvalue references for *this are not supported in GCC < 4.8.1.");
        #endif
        CORRADE_VERIFY(!(std::is_convertible<Array, int*>::value));
        CORRADE_VERIFY(!(std::is_convertible<Array&&, int*>::value));
    }

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

void ArrayTest::convertVoid() {
    /* void reference to Array */
    Array a(6);
    const Array ca(6);
    VoidArrayView b = a;
    VoidArrayView cb = ca;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(cb == ca);
    CORRADE_COMPARE(b.size(), a.size()*sizeof(int));
    CORRADE_COMPARE(cb.size(), ca.size()*sizeof(int));
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
    CORRADE_COMPARE(*(a.begin()+2), 2);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a.end()-a.begin(), a.size());
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());

    const Array b{InPlaceInit, {7, 3, 5, 4}};
    CORRADE_COMPARE(b.data(), static_cast<const int*>(b));
    CORRADE_COMPARE(b[2], 5);
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

void ArrayTest::sliceToStatic() {
    Array a{InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{InPlaceInit, {1, 2, 3, 4, 5}};

    StaticArrayView<3, int> b = a.slice<3>(1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StaticArrayView<3, const int> bc = ac.slice<3>(1);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);
}

void ArrayTest::release() {
    Array a(5);
    int* const data = a;
    int* const released = a.release();
    delete[] released;

    CORRADE_COMPARE(data, released);
    CORRADE_COMPARE(a.begin(), nullptr);
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::defaultDeleter() {
    Array a{5};
    CORRADE_COMPARE(a.deleter(), nullptr);
}

namespace {
    int CustomDeleterDeletedCount = 0;
}

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

namespace {
    struct CustomDeleter {
        CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
        void operator()(int*, std::size_t size) { deletedCount = size; }
        int& deletedCount;
    };
}

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

    CORRADE_COMPARE(reinterpret_cast<void*>(b.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cb.begin()), reinterpret_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(bc.begin()), reinterpret_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cbc.begin()), reinterpret_cast<const void*>(cac.begin()));

    CORRADE_COMPARE(reinterpret_cast<void*>(d.begin()), reinterpret_cast<void*>(a.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cd.begin()), reinterpret_cast<const void*>(ca.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(dc.begin()), reinterpret_cast<const void*>(ac.begin()));
    CORRADE_COMPARE(reinterpret_cast<const void*>(cdc.begin()), reinterpret_cast<const void*>(cac.begin()));

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

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayTest)
