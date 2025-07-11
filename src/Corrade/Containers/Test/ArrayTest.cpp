/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

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

    void resetCounters();

    void constructDefault();
    void constructEmpty();
    void construct();
    void constructZeroSize();
    #ifdef CORRADE_BUILD_DEPRECATED
    void constructDefaultInit();
    void constructDefaultInitZeroSize();
    #endif
    void constructValueInit();
    void constructValueInitZeroSize();
    void constructNoInitTrivial();
    void constructNoInitTrivialZeroSize();
    void constructNoInitNonTrivial();
    void constructNoInitNonTrivialZeroSize();
    void constructNoInitNoDefaultConstructor();
    void constructDirectInit();
    void constructDirectInitZeroSize();
    void constructDirectInitMoveOnly();
    void constructInPlaceInit();
    void constructInPlaceInitZeroSize();
    void constructInPlaceInitMoveOnly();
    void constructFromExisting();
    void constructMove();
    void constructDirectReferences();

    void constructZeroNullPointerAmbiguity();

    void convertBool();
    void convertPointer();
    void convertView();
    void convertViewDerived();
    void convertViewOverload();
    void convertVoid();
    void convertConstVoid();
    void convertToExternalView();
    void convertToConstExternalView();

    void access();
    void accessConst();
    void accessInvalid();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();
    void sliceZeroNullPointerAmbiguity();

    void release();

    void defaultDeleter();
    void customDeleter();
    void customDeleterArrayView();
    void customDeleterNullData();
    void customDeleterZeroSize();
    void customDeleterMovedOutInstance();
    void customDeleterType();
    void customDeleterTypeNoConstructor();
    void customDeleterTypeExplicitDefaultConstructor();
    void customDeleterTypeNullData();
    void customDeleterTypeZeroSize();
    void customDeleterTypeMovedOutInstance();

    void cast();
    void size();

    void constructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();
};

typedef Containers::Array<int> Array;
typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;

ArrayTest::ArrayTest() {
    addTests({&ArrayTest::constructDefault,
              &ArrayTest::constructEmpty,
              &ArrayTest::construct,
              &ArrayTest::constructZeroSize,
              #ifdef CORRADE_BUILD_DEPRECATED
              &ArrayTest::constructDefaultInit,
              &ArrayTest::constructDefaultInitZeroSize,
              #endif
              &ArrayTest::constructValueInit,
              &ArrayTest::constructValueInitZeroSize,
              &ArrayTest::constructNoInitTrivial,
              &ArrayTest::constructNoInitTrivialZeroSize,
              &ArrayTest::constructNoInitNonTrivial,
              &ArrayTest::constructNoInitNonTrivialZeroSize,
              &ArrayTest::constructNoInitNoDefaultConstructor,
              &ArrayTest::constructDirectInit,
              &ArrayTest::constructDirectInitZeroSize});

    addTests({&ArrayTest::constructDirectInitMoveOnly},
        &ArrayTest::resetCounters, &ArrayTest::resetCounters);

    addTests({&ArrayTest::constructInPlaceInit,
              &ArrayTest::constructInPlaceInitZeroSize});

    addTests({&ArrayTest::constructInPlaceInitMoveOnly},
        &ArrayTest::resetCounters, &ArrayTest::resetCounters);

    addTests({&ArrayTest::constructFromExisting,
              &ArrayTest::constructMove,
              &ArrayTest::constructDirectReferences,

              &ArrayTest::constructZeroNullPointerAmbiguity,

              &ArrayTest::convertBool,
              &ArrayTest::convertPointer,
              &ArrayTest::convertView,
              &ArrayTest::convertViewDerived,
              &ArrayTest::convertViewOverload,
              &ArrayTest::convertVoid,
              &ArrayTest::convertConstVoid,
              &ArrayTest::convertToExternalView,
              &ArrayTest::convertToConstExternalView,

              &ArrayTest::access,
              &ArrayTest::accessConst,
              &ArrayTest::accessInvalid,
              &ArrayTest::rvalueArrayAccess,
              &ArrayTest::rangeBasedFor,

              &ArrayTest::slice,
              &ArrayTest::slicePointer,
              &ArrayTest::sliceToStatic,
              &ArrayTest::sliceToStaticPointer,
              &ArrayTest::sliceZeroNullPointerAmbiguity,

              &ArrayTest::release,

              &ArrayTest::defaultDeleter,
              &ArrayTest::customDeleter,
              &ArrayTest::customDeleterArrayView,
              &ArrayTest::customDeleterNullData,
              &ArrayTest::customDeleterZeroSize,
              &ArrayTest::customDeleterMovedOutInstance,
              &ArrayTest::customDeleterType,
              &ArrayTest::customDeleterTypeNoConstructor,
              &ArrayTest::customDeleterTypeExplicitDefaultConstructor,
              &ArrayTest::customDeleterTypeNullData,
              &ArrayTest::customDeleterTypeZeroSize,
              &ArrayTest::customDeleterTypeMovedOutInstance,

              &ArrayTest::cast,
              &ArrayTest::size,

              &ArrayTest::constructorExplicitInCopyInitialization,
              &ArrayTest::copyConstructPlainStruct,
              &ArrayTest::moveConstructPlainStruct});
}

struct Movable {
    static int constructed;
    static int destructed;
    static int moved;

    /*implicit*/ Movable(int a = 0) noexcept: a{a} { ++constructed; }
    Movable(const Movable&) = delete;
    Movable(Movable&& other) noexcept: a(other.a) {
        ++constructed;
        ++moved;
    }
    ~Movable() { ++destructed; }
    Movable& operator=(const Movable&) = delete;
    /* Clang complains this one is unused. I want it to record moves, *if* it
       ever gets used, tho. */
    CORRADE_UNUSED Movable& operator=(Movable&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    int a;
};

int Movable::constructed = 0;
int Movable::destructed = 0;
int Movable::moved = 0;

void ArrayTest::resetCounters() {
    Movable::constructed = Movable::destructed = Movable::moved = 0;
}

void ArrayTest::constructDefault() {
    /* Should be implicitly constructible */
    const Array a1 = {};
    /* GCC 4.8 tries to use the deleted Array copy constructor with = nullptr,
       probably due to the workaround to avoid Array{0} being ambiguous between
       a std::size_t and a nullptr constructor */
    /** @todo drop this once the single-argument size constructor is
        deprecated in favor of explicit ValueInit / NoInit */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
    const Array a2{nullptr};
    #else
    const Array a2 = nullptr;
    #endif
    CORRADE_VERIFY(a1 == nullptr);
    CORRADE_VERIFY(a2 == nullptr);
    CORRADE_VERIFY(a1.isEmpty());
    CORRADE_VERIFY(a2.isEmpty());
    CORRADE_COMPARE(a1.size(), 0);
    CORRADE_COMPARE(a2.size(), 0);
}

void ArrayTest::constructEmpty() {
    /* Zero-length should not call new */
    const std::size_t size = 0;
    const Array b(size);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_COMPARE(b.size(), 0);
}

void ArrayTest::construct() {
    const Array a(5);
    CORRADE_VERIFY(a != nullptr);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);

    /* Values should be zero-initialized (same as ValueInit) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);

    /* Implicit construction from std::size_t is not allowed */
    CORRADE_VERIFY(!std::is_convertible<std::size_t, Array>::value);
}

void ArrayTest::constructZeroSize() {
    Array a{0};
    CORRADE_VERIFY(!a.data());
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::constructFromExisting() {
    int* a = new int[25];
    Array b{a, 25};
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 25);
}

#ifdef CORRADE_BUILD_DEPRECATED
void ArrayTest::constructDefaultInit() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    const Array a{Corrade::DefaultInit, 5};
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);

    /* Values are random memory */
}

void ArrayTest::constructDefaultInitZeroSize() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    Array a{Corrade::DefaultInit, 0};
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(a.size(), 0);
}
#endif

void ArrayTest::constructValueInit() {
    const Array a{Corrade::ValueInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);

    /* Values should be zero-initialized (same as the default constructor) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);
}

void ArrayTest::constructValueInitZeroSize() {
    Array a{Corrade::ValueInit, 0};
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::constructNoInitTrivial() {
    const Array a{Corrade::NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_VERIFY(!a.deleter());
}

void ArrayTest::constructNoInitTrivialZeroSize() {
    Array a{Corrade::NoInit, 0};
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(a.size(), 0);
}

struct Foo {
    static int constructorCallCount;
    Foo() { ++constructorCallCount; }
};

int Foo::constructorCallCount = 0;

void ArrayTest::constructNoInitNonTrivial() {
    const Containers::Array<Foo> a{Corrade::NoInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_VERIFY(a.deleter());
    CORRADE_COMPARE(Foo::constructorCallCount, 0);

    /* Just to verify that the variable gets updated when calling a regular
       constructor */
    const Containers::Array<Foo> b{Corrade::ValueInit, 7};
    CORRADE_COMPARE(Foo::constructorCallCount, 7);
}

void ArrayTest::constructNoInitNonTrivialZeroSize() {
    Containers::Array<Foo> a{Corrade::NoInit, 0};
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(a.size(), 0);
}

/* A variant of these is used in StaticArrayTest, PairTest and TripleTest */
struct NoDefaultConstructor {
    /* Clang complains this one is unused. Well, yes, it's here to make the
       struct non-default-constructible. */
    CORRADE_UNUSED /*implicit*/ NoDefaultConstructor(int a): a{a} {}
    int a;
};
template<class T> struct Wrapped {
    /* This works only if T is default-constructible */
    /*implicit*/ Wrapped(): a{} {}
    T a;
};

void ArrayTest::constructNoInitNoDefaultConstructor() {
    /* In libstdc++ before version 8 std::is_trivially_constructible<T> doesn't
       work with (template) types where the default constructor isn't usable,
       failing compilation instead of producing std::false_type; in version 4.8
       this trait isn't available at all. std::is_trivial is used instead,
       verify that it compiles correctly everywhere. */

    Containers::Array<Wrapped<NoDefaultConstructor>> a{Corrade::NoInit, 4};
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.size(), 4);
}

void ArrayTest::constructDirectInit() {
    const Array a{Corrade::DirectInit, 2, -37};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a.size(), 2);
    CORRADE_COMPARE(a[0], -37);
    CORRADE_COMPARE(a[1], -37);
}

void ArrayTest::constructDirectInitZeroSize() {
    Array a{Corrade::DirectInit, 0, -37};
    CORRADE_VERIFY(!a.data());
    CORRADE_COMPARE(a.size(), 0);
}

void ArrayTest::constructDirectInitMoveOnly() {
    {
        /* This one is weird as it moves one argument 3 times, but should work
           nevertheless */
        Containers::Array<Movable> a{Corrade::DirectInit, 3, Movable{-37}};
        CORRADE_COMPARE(a[0].a, -37);
        CORRADE_COMPARE(a[1].a, -37);
        CORRADE_COMPARE(a[2].a, -37);

        /* 1 temporary that was moved to the concrete places 3 times */
        CORRADE_COMPARE(Movable::constructed, 1 + 3);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 1 + 3);
    CORRADE_COMPARE(Movable::destructed, 1 + 3);
    CORRADE_COMPARE(Movable::moved, 3);
}

void ArrayTest::constructInPlaceInit() {
    Array a1{Corrade::InPlaceInit, {1, 3, 127, -48}};
    CORRADE_VERIFY(a1);
    CORRADE_COMPARE(a1.size(), 4);
    CORRADE_COMPARE(a1[0], 1);
    CORRADE_COMPARE(a1[1], 3);
    CORRADE_COMPARE(a1[2], 127);
    CORRADE_COMPARE(a1[3], -48);

    int data[]{1, 3, 127, -48};
    Array a2{Corrade::InPlaceInit, data};
    CORRADE_VERIFY(a2);
    CORRADE_COMPARE(a2.size(), 4);
    CORRADE_COMPARE(a2[0], 1);
    CORRADE_COMPARE(a2[1], 3);
    CORRADE_COMPARE(a2[2], 127);
    CORRADE_COMPARE(a2[3], -48);

    Array a3 = array<int>({1, 3, 127, -48});
    CORRADE_VERIFY(a3);
    CORRADE_COMPARE(a3.size(), 4);
    CORRADE_COMPARE(a3[0], 1);
    CORRADE_COMPARE(a3[1], 3);
    CORRADE_COMPARE(a3[2], 127);
    CORRADE_COMPARE(a3[3], -48);

    Array a4 = array<int>(data);
    CORRADE_VERIFY(a4);
    CORRADE_COMPARE(a4.size(), 4);
    CORRADE_COMPARE(a4[0], 1);
    CORRADE_COMPARE(a4[1], 3);
    CORRADE_COMPARE(a4[2], 127);
    CORRADE_COMPARE(a4[3], -48);
}

void ArrayTest::constructInPlaceInitZeroSize() {
    Array a1{Corrade::InPlaceInit, {}};
    CORRADE_VERIFY(!a1);
    CORRADE_VERIFY(!a1.data());
    CORRADE_COMPARE(a1.size(), 0);

    Array a2 = array<int>({});
    CORRADE_VERIFY(!a2);
    CORRADE_VERIFY(!a2.data());
    CORRADE_COMPARE(a2.size(), 0);
}

void ArrayTest::constructInPlaceInitMoveOnly() {
    #if 0
    {
        /* This one is weird as it moves one argument 3 times, but should work
           nevertheless */
        Containers::Array<Movable> a{Corrade::InPlaceInit, {Movable{1}, Movable{2}, Movable{3}}};
        CORRADE_COMPARE(a[0].a, -37);
        CORRADE_COMPARE(a[1].a, -37);
        CORRADE_COMPARE(a[2].a, -37);

        /* 6 temporaries that were moved to the concrete places 6 times */
        CORRADE_COMPARE(Movable::constructed, 6 + 6);
        CORRADE_COMPARE(Movable::destructed, 6);
        CORRADE_COMPARE(Movable::moved, 6);
    }

    CORRADE_COMPARE(Movable::constructed, 6 + 6);
    CORRADE_COMPARE(Movable::destructed, 6 + 6);
    CORRADE_COMPARE(Movable::moved, 6);
    #else
    CORRADE_SKIP("Impossible with the std::initializer_list argument, needs to wait until InPlaceInit with T(&&)[size] exists.");
    #endif
}

void ArrayTest::constructMove() {
    auto myDeleter = [](int* data, std::size_t) { delete[] data; };
    Array a(new int[5], 5, myDeleter);
    CORRADE_VERIFY(a);
    const int* const ptr = a;

    Array b(Utility::move(a));
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == ptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_VERIFY(a.deleter() == nullptr);
    CORRADE_VERIFY(b.deleter() == myDeleter);

    auto noDeleter = [](int*, std::size_t) {};
    Array c{reinterpret_cast<int*>(0x3), 3, noDeleter};
    c = Utility::move(b);
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

    const Containers::Array<Reference> b{Corrade::DirectInit, 5, a};
    CORRADE_COMPARE(b.size(), 5);
}

/* Without a corresponding SFINAE check in the std::nullptr_t constructor, this
   is ambiguous, but *only* if the size_t overload has a second 64-bit
   argument. If both would be the same, it wouldn't be ambigous, if the size_t
   overload second argument was 32-bit and the other 16-bit it wouldn't be
   either. */
int integerArrayOverload(std::size_t, long long) {
    return 76;
}
int integerArrayOverload(const Array&, int) {
    return 39;
}

void ArrayTest::constructZeroNullPointerAmbiguity() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverload(25, 2), 76);
    CORRADE_COMPARE(integerArrayOverload(nullptr, 2), 39);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverload(0, 3), 76);
}

void ArrayTest::convertBool() {
    CORRADE_VERIFY(Array(2));
    CORRADE_VERIFY(!Array());

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY(std::is_constructible<bool, Array>::value);
    CORRADE_VERIFY(!std::is_constructible<int, Array>::value);
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

    /* Verify that we can't convert rvalues. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<int*, Array&>::value);
    CORRADE_VERIFY(std::is_constructible<const int*, const Array&>::value);
    CORRADE_VERIFY(!std::is_constructible<int*, Array>::value);
    CORRADE_VERIFY(!std::is_constructible<int*, Array&&>::value);

    /* Deleting const&& overload and leaving only const& one will not, in fact,
       disable conversion of const Array&& to pointer, but rather make the
       conversion ambiguous, which is not what we want, as it breaks e.g.
       rvalueArrayAccess() test. Not using is_convertible to catch also
       accidental explicit conversions. */
    {
        CORRADE_EXPECT_FAIL("I don't know how to properly disable conversion of const Array&& to pointer.");
        CORRADE_VERIFY(!std::is_constructible<const int*, const Array>::value);
        CORRADE_VERIFY(!std::is_constructible<const int*, const Array&&>::value);
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
        CORRADE_VERIFY(std::is_same<decltype(b), const ArrayView>::value);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ConstArrayView>::value);
        CORRADE_VERIFY(std::is_same<decltype(bc), const ConstArrayView>::value);
        CORRADE_VERIFY(std::is_same<decltype(cbc), const ConstArrayView>::value);
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);

        auto c = arrayView(Array{3});
        CORRADE_VERIFY(std::is_same<decltype(c), ArrayView>::value);
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
CORRADE_UNUSED bool takesAView(Containers::ArrayView<float>) { return false; }
CORRADE_UNUSED bool takesAConstView(Containers::ArrayView<const float>) { return false; }

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
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    IntView b = a;
    CORRADE_COMPARE(b.data, a);
    CORRADE_COMPARE(b.size, a.size());

    ConstIntView cb = a;
    CORRADE_COMPARE(cb.data, a);
    CORRADE_COMPARE(cb.size, a.size());

    /* Conversion to a different type is not allowed. Not using is_convertible
       to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<IntView, Containers::Array<int>>::value);
    CORRADE_VERIFY(std::is_constructible<ConstIntView, Containers::Array<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<IntView, Containers::Array<float>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstIntView, Containers::Array<float>>::value);
}

void ArrayTest::convertToConstExternalView() {
    const Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    ConstIntView b = a;
    CORRADE_COMPARE(b.data, a);
    CORRADE_COMPARE(b.size, a.size());

    /* Conversion to a different type is not allowed. Not using is_convertible
       to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstIntView, const Containers::Array<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstIntView, const Containers::Array<float>>::value);
}

void ArrayTest::access() {
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(a.front(), 1);
    CORRADE_COMPARE(a.back(), 5);
    CORRADE_COMPARE(*(a.begin() + 2), 3);
    CORRADE_COMPARE(a[4], 5);
    CORRADE_COMPARE(a.end() - a.begin(), a.size());
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());

    /* Mutable access */
    a.front() += 100;
    a.back() *= 10;
    *(a.begin() + 1) -= 10;
    *(a.end() - 3) += 1000;
    ++a[3];
    CORRADE_COMPARE(a[0], 101);
    CORRADE_COMPARE(a[1], -8);
    CORRADE_COMPARE(a[2], 1003);
    CORRADE_COMPARE(a[3], 5);
    CORRADE_COMPARE(a[4], 50);
}

void ArrayTest::accessConst() {
    const Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    CORRADE_COMPARE(a.data(), static_cast<const int*>(a));
    CORRADE_COMPARE(a.front(), 1);
    CORRADE_COMPARE(a.back(), 5);
    CORRADE_COMPARE(*(a.begin() + 2), 3);
    CORRADE_COMPARE(a[4], 5);
    CORRADE_COMPARE(a.end() - a.begin(), a.size());
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());
}

void ArrayTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    Array a;
    Array b{5};
    a.front();
    a.back();
    b[5];
    CORRADE_COMPARE(out,
        "Containers::Array::front(): array is empty\n"
        "Containers::Array::back(): array is empty\n"
        "Containers::Array::operator[](): index 5 out of range for 5 elements\n");
}

void ArrayTest::rvalueArrayAccess() {
    CORRADE_COMPARE((Array{Corrade::InPlaceInit, {1, 2, 3, 4}}[2]), 3);
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
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    ArrayView b1 = a.slice(1, 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ConstArrayView bc1 = ac.slice(1, 4);
    CORRADE_COMPARE(bc1.size(), 3);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    ArrayView b2 = a.sliceSize(1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ConstArrayView bc2 = ac.sliceSize(1, 3);
    CORRADE_COMPARE(bc2.size(), 3);
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

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

    ArrayView d = a.exceptPrefix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ConstArrayView dc = ac.exceptPrefix(2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);

    ArrayView e = a.exceptSuffix(2);
    CORRADE_COMPARE(e.size(), 3);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    ConstArrayView ce = ac.exceptSuffix(2);
    CORRADE_COMPARE(ce.size(), 3);
    CORRADE_COMPARE(ce[0], 1);
    CORRADE_COMPARE(ce[1], 2);
    CORRADE_COMPARE(ce[2], 3);
}

void ArrayTest::slicePointer() {
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    ArrayView b1 = a.slice(a + 1, a + 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ConstArrayView bc1 = ac.slice(ac + 1, ac + 4);
    CORRADE_COMPARE(bc1.size(), 3);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    ArrayView b2 = a.sliceSize(a + 1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ConstArrayView bc2 = ac.sliceSize(ac + 1, 3);
    CORRADE_COMPARE(bc2.size(), 3);
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

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
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

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

    StaticArrayView<3, int> b3 = a.sliceSize<1, 3>();
    CORRADE_COMPARE(b3[0], 2);
    CORRADE_COMPARE(b3[1], 3);
    CORRADE_COMPARE(b3[2], 4);

    StaticArrayView<3, const int> bc3 = ac.sliceSize<1, 3>();
    CORRADE_COMPARE(bc3[0], 2);
    CORRADE_COMPARE(bc3[1], 3);
    CORRADE_COMPARE(bc3[2], 4);

    StaticArrayView<3, int> c = a.prefix<3>();
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    StaticArrayView<3, const int> cc = ac.prefix<3>();
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    StaticArrayView<3, int> d = a.suffix<3>();
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    StaticArrayView<3, const int> dc = ac.suffix<3>();
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);
}

void ArrayTest::sliceToStaticPointer() {
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StaticArrayView<3, const int> bc = ac.slice<3>(ac + 1);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);
}

void ArrayTest::sliceZeroNullPointerAmbiguity() {
    Array a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    const Array ac{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    /* These should all unambigously pick the std::size_t overloads, not the
       T* overloads */

    ArrayView b = a.sliceSize(0, 3);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(b[1], 2);
    CORRADE_COMPARE(b[2], 3);

    ConstArrayView bc = ac.sliceSize(0, 3);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(bc[0], 1);
    CORRADE_COMPARE(bc[1], 2);
    CORRADE_COMPARE(bc[2], 3);

    ArrayView c = a.prefix(0);
    CORRADE_COMPARE(c.size(), 0);
    CORRADE_COMPARE(c.data(), static_cast<void*>(a.data()));

    ConstArrayView cc = ac.prefix(0);
    CORRADE_COMPARE(cc.size(), 0);
    CORRADE_COMPARE(cc.data(), static_cast<const void*>(ac.data()));

    /** @todo suffix(0), once the non-deprecated suffix(std::size_t size) is a
        thing */

    StaticArrayView<3, int> e = a.slice<3>(0);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    StaticArrayView<3, const int> ec = ac.slice<3>(0);
    CORRADE_COMPARE(ec[0], 1);
    CORRADE_COMPARE(ec[1], 2);
    CORRADE_COMPARE(ec[2], 3);
}

void ArrayTest::release() {
    auto myDeleter = [](int* data, std::size_t) { delete[] data; };
    Array a(new int[5], 5, myDeleter);
    int* const data = a;
    int* const released = a.release();
    delete[] released;

    /* Not comparing pointers directly because then Clang Analyzer complains
       that printing the value of `released` is use-after-free. Um.

       GCC 13+ now also crashes in, breathing heavily, trying to be helpful
       with "warning: pointer may be used after ‘void operator delete [](void*)’".
       Well, no shit? I'm looking at the value. If I'd be comparing the value
       before a delete instead, you'd complain that I might have a potential
       leak. Happens only in a Release build. Possibly related is the
       following, however this didn't happen on GCC 12:
       https://gcc.gnu.org/bugzilla//show_bug.cgi?id=106119 */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 13
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuse-after-free"
    #endif
    CORRADE_COMPARE(reinterpret_cast<std::intptr_t>(data), reinterpret_cast<std::intptr_t>(released));
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 13
    #pragma GCC diagnostic pop
    #endif
    CORRADE_COMPARE(a.begin(), nullptr);
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_VERIFY(a.deleter() == nullptr);
}

void ArrayTest::defaultDeleter() {
    Array a{5};
    CORRADE_VERIFY(a.deleter() == nullptr);
}

int CustomDeleterCallCount = 0;

void ArrayTest::customDeleter() {
    CustomDeleterCallCount = 0;
    int data[25]{1337};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        Array a{data, 25, [](int* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], 1337);
            CORRADE_COMPARE(size, 25);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void ArrayTest::customDeleterArrayView() {
    CustomDeleterCallCount = 0;
    int data[25]{1337};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        Array a{ArrayView{data, 25}, [](int* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], 1337);
            CORRADE_COMPARE(size, 25);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void ArrayTest::customDeleterNullData() {
    CustomDeleterCallCount = 0;
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        Array a{nullptr, 25, [](int* data, std::size_t size) {
            CORRADE_VERIFY(!data);
            CORRADE_COMPARE(size, 25);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a == nullptr);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter should be called even in case the data is null. This is to
       have correct behavior e.g. in Array<char, Utility::Path::MapDeleter>
       where the data can be null for an empty file, but the fd should still
       get properly closed after. */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void ArrayTest::customDeleterZeroSize() {
    CustomDeleterCallCount = 0;
    int data[25]{1337};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        Array a{data, 0, [](int* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], 1337);
            CORRADE_COMPARE(size, 0);
            ++CustomDeleterCallCount;
        }};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* Variant of the above, while not as common, the deleter should
       unconditionally get called here as well */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void ArrayTest::customDeleterMovedOutInstance() {
    CustomDeleterCallCount = 0;
    int data[25]{};
    CORRADE_VERIFY(true); /* to register proper function name */

    {
        Array a{data, 25, [](int*, std::size_t) {
            ++CustomDeleterCallCount;
        }};
        CORRADE_COMPARE(CustomDeleterCallCount, 0);

        Array b = Utility::move(a);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter got reset to nullptr in a, which means the function gets
       called only once */
    CORRADE_COMPARE(CustomDeleterCallCount, 1);
}

void ArrayTest::customDeleterType() {
    int data[25]{1337};
    int deletedCount = 0;
    CORRADE_VERIFY(true); /* to register proper function name */

    struct CustomDeleter {
        CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
        void operator()(int* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], 1337);
            CORRADE_COMPARE(size, 25);
            ++deletedCount;
        }
        int& deletedCount;
    };

    {
        Containers::Array<int, CustomDeleter> a{data, 25, CustomDeleter{deletedCount}};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(deletedCount, 0);
    }

    CORRADE_COMPARE(deletedCount, 1);
}

void ArrayTest::customDeleterTypeNoConstructor() {
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

void ArrayTest::customDeleterTypeExplicitDefaultConstructor() {
    struct CustomExplicitDeleter {
        explicit CustomExplicitDeleter() = default;

        void operator()(int*, std::size_t) {}
    };

    /* Just verify that this compiles */
    Containers::Array<int, CustomExplicitDeleter> a;
    Containers::Array<int, CustomExplicitDeleter> b{nullptr};
    int c;
    Containers::Array<int, CustomExplicitDeleter> d{&c, 1, CustomExplicitDeleter{}};
    CORRADE_VERIFY(true);
}

void ArrayTest::customDeleterTypeNullData() {
    int deletedCount = 0;
    CORRADE_VERIFY(true); /* to register proper function name */

    struct CustomDeleter {
        CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
        void operator()(int* data, std::size_t size) {
            CORRADE_VERIFY(!data);
            CORRADE_COMPARE(size, 25);
            ++deletedCount;
        }
        int& deletedCount;
    };

    {
        Containers::Array<int, CustomDeleter> a{nullptr, 25, CustomDeleter{deletedCount}};
        CORRADE_VERIFY(a == nullptr);
        CORRADE_COMPARE(a.size(), 25);
        CORRADE_COMPARE(deletedCount, 0);
    }

    /* Just to check that the behavior is the same as with plain deleter
       pointers -- see comment in customDeleterNullData() for details */
    CORRADE_COMPARE(deletedCount, 1);
}

void ArrayTest::customDeleterTypeZeroSize() {
    int deletedCount = 0;
    int data[25]{1337};
    CORRADE_VERIFY(true); /* to register proper function name */

    struct CustomDeleter {
        CustomDeleter(int& deletedCountOutput): deletedCount{deletedCountOutput} {}
        void operator()(int* data, std::size_t size) {
            CORRADE_VERIFY(data);
            CORRADE_COMPARE(data[0], 1337);
            CORRADE_COMPARE(size, 0);
            ++deletedCount;
        }
        int& deletedCount;
    };

    {
        Containers::Array<int, CustomDeleter> a{data, 0, CustomDeleter{deletedCount}};
        CORRADE_VERIFY(a == data);
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(deletedCount, 0);
    }

    /* Just to check that the behavior is the same as with plain deleter
       pointers -- see comment in customDeleterZeroSize() for details */
    CORRADE_COMPARE(deletedCount, 1);
}

void ArrayTest::customDeleterTypeMovedOutInstance() {
    CustomDeleterCallCount = 0;
    int deletedCount = 0;
    int data[25]{};
    CORRADE_VERIFY(true); /* to register proper function name */

    struct CustomDeleter {
        CustomDeleter(): deletedCount{} {}
        CustomDeleter(int& deletedCountOutput): deletedCount{&deletedCountOutput} {}
        void operator()(int*, std::size_t) {
            if(deletedCount) ++*deletedCount;
            ++CustomDeleterCallCount;
        }
        int* deletedCount;
    };

    {
        Containers::Array<int, CustomDeleter> a{data, 0, CustomDeleter{deletedCount}};
        CORRADE_COMPARE(deletedCount, 0);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);

        Containers::Array<int, CustomDeleter> b = Utility::move(a);
        CORRADE_COMPARE(deletedCount, 0);
        CORRADE_COMPARE(CustomDeleterCallCount, 0);
    }

    /* The deleter got reset to a default-constructed state in a, which means
       the deletedCount pointer gets reset -- but the default-constructed
       instance still gets called for it, so the global counter is 2 */
    CORRADE_COMPARE(deletedCount, 1);
    CORRADE_COMPARE(CustomDeleterCallCount, 2);
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

    CORRADE_VERIFY(std::is_same<decltype(b), Containers::ArrayView<std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(bc), Containers::ArrayView<const std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cb), Containers::ArrayView<const std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cbc), Containers::ArrayView<const std::uint64_t>>::value);

    CORRADE_VERIFY(std::is_same<decltype(d), Containers::ArrayView<std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cd), Containers::ArrayView<const std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(dc), Containers::ArrayView<const std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cdc), Containers::ArrayView<const std::uint16_t>>::value);

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

void ArrayTest::constructorExplicitInCopyInitialization() {
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
    Containers::Array<ContainingExplicitDefaultWithImplicitConstructor> c{Corrade::ValueInit, 5};
    Containers::Array<ContainingExplicitDefaultWithImplicitConstructor> b{Corrade::DirectInit, 5};
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(c.size(), 5);
}

void ArrayTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    Containers::Array<ExtremelyTrivial> a{Corrade::DirectInit, 2, 3, 'a'};
    CORRADE_COMPARE(a.size(), 2);

    /* This copy-constructs the new values */
    Containers::Array<ExtremelyTrivial> b{Corrade::InPlaceInit, {
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

    /* This needs special handling on GCC 4.8, where T{Utility::move(b)}
       attempts to convert MoveOnlyStruct to int to initialize the first
       argument and fails miserably. */
    Containers::Array<MoveOnlyStruct> a{Corrade::DirectInit, 2, 3, 'a', nullptr};
    CORRADE_COMPARE(a.size(), 2);

    /* Unlike with copyConstructPlainStruct(), the InPlaceInit doesn't use
       move-construction, so that's not affected */
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayTest)
