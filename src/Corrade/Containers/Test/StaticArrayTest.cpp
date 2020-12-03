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

#include "Corrade/Containers/StaticArray.h"
#include "Corrade/TestSuite/Tester.h"

namespace {

struct IntView5 {
    explicit IntView5(int* data): data{data} {}

    int* data;
};

struct ConstIntView5 {
    explicit ConstIntView5(const int* data): data{data} {}

    const int* data;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct StaticArrayViewConverter<5, int, IntView5> {
    static IntView5 to(StaticArrayView<5, int> other) {
        return IntView5{other.data()};
    }
};

template<> struct StaticArrayViewConverter<5, const int, ConstIntView5> {
    static ConstIntView5 to(StaticArrayView<5, const int> other) {
        return ConstIntView5{other.data()};
    }
};

}

namespace Test { namespace {

struct StaticArrayTest: TestSuite::Tester {
    explicit StaticArrayTest();

    void resetCounters();

    void construct();
    void constructDefaultInit();
    void constructValueInit();
    void constructNoInit();
    void constructInPlaceInit();
    void constructInPlaceInitOneArgument();
    void constructDirectInit();
    void constructNonCopyable();
    void constructNoImplicitConstructor();
    void constructDirectReferences();

    void copy();
    void move();

    void convertBool();
    void convertPointer();
    void convertView();
    void convertViewDerived();
    void convertViewOverload();
    void convertStaticView();
    void convertStaticViewDerived();
    void convertStaticViewOverload();
    void convertVoid();
    void convertConstVoid();
    void convertToExternalView();
    void convertToConstExternalView();

    void access();
    void accessConst();
    void rvalueArrayAccess();
    void rangeBasedFor();

    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();

    void cast();
    void size();

    void emplaceConstructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();
};

typedef Containers::StaticArray<5, int> StaticArray;
typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;
typedef Containers::StaticArrayView<5, int> StaticArrayView;
typedef Containers::StaticArrayView<5, const int> ConstStaticArrayView;

StaticArrayTest::StaticArrayTest() {
    addTests({&StaticArrayTest::construct,
              &StaticArrayTest::constructDefaultInit,
              &StaticArrayTest::constructValueInit});

    addTests({&StaticArrayTest::constructNoInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests({&StaticArrayTest::constructInPlaceInit,
              &StaticArrayTest::constructInPlaceInitOneArgument,
              &StaticArrayTest::constructDirectInit});

    addTests({&StaticArrayTest::constructNonCopyable},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests({&StaticArrayTest::constructNoImplicitConstructor,
              &StaticArrayTest::constructDirectReferences});

    addTests({&StaticArrayTest::copy,
              &StaticArrayTest::move},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests({&StaticArrayTest::convertBool,
              &StaticArrayTest::convertPointer,
              &StaticArrayTest::convertView,
              &StaticArrayTest::convertViewDerived,
              &StaticArrayTest::convertViewOverload,
              &StaticArrayTest::convertStaticView,
              &StaticArrayTest::convertStaticViewDerived,
              &StaticArrayTest::convertStaticViewOverload,
              &StaticArrayTest::convertVoid,
              &StaticArrayTest::convertConstVoid,
              &StaticArrayTest::convertToExternalView,
              &StaticArrayTest::convertToConstExternalView,

              &StaticArrayTest::access,
              &StaticArrayTest::accessConst,
              &StaticArrayTest::rvalueArrayAccess,
              &StaticArrayTest::rangeBasedFor,

              &StaticArrayTest::slice,
              &StaticArrayTest::slicePointer,
              &StaticArrayTest::sliceToStatic,
              &StaticArrayTest::sliceToStaticPointer,

              &StaticArrayTest::cast,
              &StaticArrayTest::size,

              &StaticArrayTest::emplaceConstructorExplicitInCopyInitialization,
              &StaticArrayTest::copyConstructPlainStruct,
              &StaticArrayTest::moveConstructPlainStruct});
}

void StaticArrayTest::construct() {
    const StaticArray a;
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!a.empty());
    CORRADE_COMPARE(a.size(), StaticArray::Size);
    CORRADE_COMPARE(a.size(), 5);

    /* Values should be zero-initialized (same as ValueInit) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);
}

void StaticArrayTest::constructDefaultInit() {
    const StaticArray a{DefaultInit};
    CORRADE_VERIFY(a);

    /* Values are random memory */
}

void StaticArrayTest::constructValueInit() {
    const StaticArray a{ValueInit};
    CORRADE_VERIFY(a);

    /* Values should be zero-initialized (same as the default constructor) */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);
}

struct Throwable {
    explicit Throwable(int) {}
    Throwable(const Throwable&) {}
    Throwable(Throwable&&) {}
    Throwable& operator=(const Throwable&) { return *this; }
    Throwable& operator=(Throwable&&) { return *this; }
};

struct Copyable {
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;

    /*implicit*/ Copyable(int a = 0) noexcept: a{a} { ++constructed; }
    Copyable(const Copyable& other) noexcept: a{other.a} {
        ++constructed;
        ++copied;
    }
    Copyable(Copyable&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }
    ~Copyable() { ++destructed; }
    Copyable& operator=(const Copyable& other) noexcept {
        a = other.a;
        ++copied;
        return *this;
    }
    Copyable& operator=(Copyable&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    int a;
};

int Copyable::constructed = 0;
int Copyable::destructed = 0;
int Copyable::copied = 0;
int Copyable::moved = 0;

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
    Movable& operator=(Movable&& other) noexcept {
        a = other.a;
        ++moved;
        return *this;
    }

    int a;
};

int Movable::constructed = 0;
int Movable::destructed = 0;
int Movable::moved = 0;

void swap(Movable& a, Movable& b) {
    /* Swap these without copying the parent class */
    std::swap(a.a, b.a);
}

struct Immovable {
    static int constructed;
    static int destructed;

    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
    /*implicit*/ Immovable(int a = 0) noexcept: a{a} { ++constructed; }
    ~Immovable() { ++destructed; }
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;

    int a;
};

int Immovable::constructed = 0;
int Immovable::destructed = 0;

void StaticArrayTest::resetCounters() {
    Copyable::constructed = Copyable::destructed = Copyable::copied = Copyable::moved =
        Movable::constructed = Movable::destructed = Movable::moved =
            Immovable::constructed = Immovable::destructed = 0;
}

void StaticArrayTest::constructNoInit() {
    {
        const Containers::StaticArray<5, Copyable> a{NoInit};
        CORRADE_COMPARE(Copyable::constructed, 0);

        const Containers::StaticArray<5, Copyable> b{DefaultInit};
        CORRADE_COMPARE(Copyable::constructed, 5);
    }

    CORRADE_COMPARE(Copyable::destructed, 10);
}

void StaticArrayTest::constructInPlaceInit() {
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

void StaticArrayTest::constructInPlaceInitOneArgument() {
    const Containers::StaticArray<1, int> a{17};
    CORRADE_COMPARE(a[0], 17);
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
    /* Can't use ValueInit because that apparently copy-constructs the array
       elements (huh?) */
    const Containers::StaticArray<5, Immovable> a{DefaultInit};
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

void StaticArrayTest::copy() {
    {
        Containers::StaticArray<3, Copyable> a{Containers::InPlaceInit, 1, 2, 3};

        Containers::StaticArray<3, Copyable> b{a};
        CORRADE_COMPARE(b[0].a, 1);
        CORRADE_COMPARE(b[1].a, 2);
        CORRADE_COMPARE(b[2].a, 3);

        Containers::StaticArray<3, Copyable> c;
        c = b;
        CORRADE_COMPARE(c[0].a, 1);
        CORRADE_COMPARE(c[1].a, 2);
        CORRADE_COMPARE(c[2].a, 3);
    }

    CORRADE_COMPARE(Copyable::constructed, 9);
    CORRADE_COMPARE(Copyable::destructed, 9);
    CORRADE_COMPARE(Copyable::copied, 6);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Copyable>::value);
    CORRADE_VERIFY((std::is_nothrow_copy_constructible<Containers::StaticArray<3, Copyable>>::value));
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Copyable>::value);
    CORRADE_VERIFY((std::is_nothrow_copy_assignable<Containers::StaticArray<3, Copyable>>::value));

    CORRADE_VERIFY(std::is_copy_constructible<Throwable>::value);
    CORRADE_VERIFY(!(std::is_nothrow_copy_constructible<Throwable>::value));
    CORRADE_VERIFY(!(std::is_nothrow_copy_constructible<Containers::StaticArray<3, Throwable>>::value));
    CORRADE_VERIFY(std::is_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!(std::is_nothrow_copy_assignable<Containers::StaticArray<3, Throwable>>::value));
}


void StaticArrayTest::move() {
    {
        Containers::StaticArray<3, Movable> a{Containers::InPlaceInit, 1, 2, 3};

        Containers::StaticArray<3, Movable> b{std::move(a)};
        CORRADE_COMPARE(b[0].a, 1);
        CORRADE_COMPARE(b[1].a, 2);
        CORRADE_COMPARE(b[2].a, 3);

        Containers::StaticArray<3, Movable> c;
        c = std::move(b); /* this uses the swap() specialization -> no move */
        CORRADE_COMPARE(c[0].a, 1);
        CORRADE_COMPARE(c[1].a, 2);
        CORRADE_COMPARE(c[2].a, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 9);
    CORRADE_COMPARE(Movable::destructed, 9);
    CORRADE_COMPARE(Movable::moved, 3);

    CORRADE_VERIFY(!std::is_copy_constructible<Movable>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Movable>::value);
    {
        CORRADE_EXPECT_FAIL("StaticArray currently doesn't propagate deleted copy constructor/assignment correctly.");
        CORRADE_VERIFY(!(std::is_copy_constructible<Containers::StaticArray<3, Movable>>::value));
        CORRADE_VERIFY(!(std::is_copy_assignable<Containers::StaticArray<3, Movable>>::value));
    }

    CORRADE_VERIFY((std::is_move_constructible<Containers::StaticArray<3, Movable>>::value));
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Movable>::value);
    CORRADE_VERIFY((std::is_nothrow_move_constructible<Containers::StaticArray<3, Movable>>::value));
    CORRADE_VERIFY((std::is_move_assignable<Containers::StaticArray<3, Movable>>::value));
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Movable>::value);
    CORRADE_VERIFY((std::is_nothrow_move_assignable<Containers::StaticArray<3, Movable>>::value));

    CORRADE_VERIFY(std::is_move_constructible<Throwable>::value);
    CORRADE_VERIFY(!(std::is_nothrow_move_constructible<Throwable>::value));
    CORRADE_VERIFY(!(std::is_nothrow_move_constructible<Containers::StaticArray<3, Throwable>>::value));
    CORRADE_VERIFY(std::is_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!(std::is_nothrow_move_assignable<Containers::StaticArray<3, Throwable>>::value));
}

void StaticArrayTest::convertBool() {
    CORRADE_VERIFY(StaticArray{});

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY((std::is_constructible<bool, StaticArray>::value));
    CORRADE_VERIFY(!(std::is_constructible<int, StaticArray>::value));
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
    CORRADE_VERIFY(!(std::is_convertible<StaticArray, int*>::value));
    CORRADE_VERIFY(!(std::is_convertible<StaticArray&&, int*>::value));

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

bool takesAView(Containers::ArrayView<int>) { return true; }
bool takesAConstView(Containers::ArrayView<const int>) { return true; }
CORRADE_UNUSED bool takesAView(Containers::ArrayView<std::pair<int, int>>) { return false; }
CORRADE_UNUSED bool takesAConstView(Containers::ArrayView<const std::pair<int, int>>) { return false; }

void StaticArrayTest::convertViewOverload() {
    StaticArray a;
    const StaticArray ca;

    /* It should pick the correct one and not fail, assert or be ambiguous */
    CORRADE_VERIFY(takesAView(a));
    CORRADE_VERIFY(takesAConstView(a));
    CORRADE_VERIFY(takesAConstView(ca));
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

bool takesAStaticView(Containers::StaticArrayView<5, int>) { return true; }
bool takesAStaticConstView(Containers::StaticArrayView<5, const int>) { return true; }
CORRADE_UNUSED bool takesAStaticView(Containers::StaticArrayView<5, std::pair<int, int>>) { return false; }
CORRADE_UNUSED bool takesAStaticConstView(Containers::StaticArrayView<5, const std::pair<int, int>>) { return false; }

void StaticArrayTest::convertStaticViewOverload() {
    StaticArray a;
    const StaticArray ca;

    /* It should pick the correct one and not fail, assert or be ambiguous */
    CORRADE_VERIFY(takesAStaticView(a));
    CORRADE_VERIFY(takesAStaticConstView(a));
    CORRADE_VERIFY(takesAStaticConstView(ca));
}

void StaticArrayTest::convertVoid() {
    StaticArray a;
    VoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 5*sizeof(int));
}

void StaticArrayTest::convertConstVoid() {
    StaticArray a;
    const StaticArray ca;
    ConstVoidArrayView b = a;
    ConstVoidArrayView cb = ca;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(cb == ca);
    CORRADE_COMPARE(b.size(), 5*sizeof(int));
    CORRADE_COMPARE(cb.size(), 5*sizeof(int));
}

void StaticArrayTest::convertToExternalView() {
    StaticArray a{1, 2, 3, 4, 5};

    IntView5 b = a;
    CORRADE_COMPARE(b.data, a.data());

    ConstIntView5 cb = a;
    CORRADE_COMPARE(cb.data, a.data());

    /* Conversion to a different size or type is not allowed */
    CORRADE_VERIFY((std::is_convertible<Containers::StaticArray<5, int>, IntView5>::value));
    CORRADE_VERIFY((std::is_convertible<Containers::StaticArray<5, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArray<6, int>, IntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArray<6, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArray<5, float>, IntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<Containers::StaticArray<5, float>, ConstIntView5>::value));
}

void StaticArrayTest::convertToConstExternalView() {
    const StaticArray a{1, 2, 3, 4, 5};

    ConstIntView5 b = a;
    CORRADE_COMPARE(b.data, a.data());

    /* Conversion to a different size or type is not allowed */
    CORRADE_VERIFY((std::is_convertible<const Containers::StaticArray<5, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Containers::StaticArray<6, int>, ConstIntView5>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Containers::StaticArray<5, float>, ConstIntView5>::value));
}

void StaticArrayTest::access() {
    StaticArray a;
    for(std::size_t i = 0; i != 5; ++i)
        a[i] = i;

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(a.front(), 0);
    CORRADE_COMPARE(a.back(), 4);
    CORRADE_COMPARE(*(a.begin()+2), 2);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a.end()-a.begin(), 5);
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());
}

void StaticArrayTest::accessConst() {
    StaticArray a;
    for(std::size_t i = 0; i != 5; ++i)
        a[i] = i;

    const StaticArray& ca = a;
    CORRADE_COMPARE(ca.data(), static_cast<int*>(a));
    CORRADE_COMPARE(ca.front(), 0);
    CORRADE_COMPARE(ca.back(), 4);
    CORRADE_COMPARE(*(ca.begin()+2), 2);
    CORRADE_COMPARE(ca[4], 4);
    CORRADE_COMPARE(ca.end() - ca.begin(), 5);
    CORRADE_COMPARE(ca.cbegin(), ca.begin());
    CORRADE_COMPARE(ca.cend(), ca.end());
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

void StaticArrayTest::slicePointer() {
    StaticArray a{InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray ac{InPlaceInit, 1, 2, 3, 4, 5};

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

void StaticArrayTest::sliceToStatic() {
    StaticArray a{InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray ac{InPlaceInit, 1, 2, 3, 4, 5};

    Containers::StaticArrayView<3, int> b1 = a.slice<3>(1);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    Containers::StaticArrayView<3, const int> bc1 = ac.slice<3>(1);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    Containers::StaticArrayView<3, int> b2 = a.slice<1, 4>();
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    Containers::StaticArrayView<3, const int> bc2 = ac.slice<1, 4>();
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

    Containers::StaticArrayView<3, int> c1 = a.prefix<3>();
    CORRADE_COMPARE(c1[0], 1);
    CORRADE_COMPARE(c1[1], 2);
    CORRADE_COMPARE(c1[2], 3);

    Containers::StaticArrayView<3, const int> cc1 = ac.prefix<3>();
    CORRADE_COMPARE(cc1[0], 1);
    CORRADE_COMPARE(cc1[1], 2);
    CORRADE_COMPARE(cc1[2], 3);

    Containers::StaticArrayView<3, int> c2 = a.except<2>();
    CORRADE_COMPARE(c2[0], 1);
    CORRADE_COMPARE(c2[1], 2);
    CORRADE_COMPARE(c2[2], 3);

    Containers::StaticArrayView<3, const int> cc2 = ac.except<2>();
    CORRADE_COMPARE(cc2[0], 1);
    CORRADE_COMPARE(cc2[1], 2);
    CORRADE_COMPARE(cc2[2], 3);

    Containers::StaticArrayView<3, int> d = a.suffix<2>();
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    Containers::StaticArrayView<3, const int> cd = ac.suffix<2>();
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
}

void StaticArrayTest::sliceToStaticPointer() {
    StaticArray a{InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray ac{InPlaceInit, 1, 2, 3, 4, 5};

    Containers::StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    Containers::StaticArrayView<3, const int> bc = ac.slice<3>(ac + 1);
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

void StaticArrayTest::emplaceConstructorExplicitInCopyInitialization() {
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
    Containers::StaticArray<3, ContainingExplicitDefaultWithImplicitConstructor> b{DirectInit};
    CORRADE_COMPARE(b.size(), 3);
}

void StaticArrayTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    Containers::StaticArray<3, ExtremelyTrivial> a{DirectInit, 3, 'a'};
    CORRADE_COMPARE(a.front().a, 3);

    /* This copy-constructs new values */
    Containers::StaticArray<3, ExtremelyTrivial> b{a};
    CORRADE_COMPARE(b.front().a, 3);
}

void StaticArrayTest::moveConstructPlainStruct() {
    /* Can't make MoveOnlyStruct directly non-copyable because then we'd hit
       another GCC 4.8 bug where it can't be constructed using {} anymore. In
       other tests I simply add a (move-only) Array or Pointer member, but here
       I don't want to avoid a needless header dependency. */
    struct MoveOnlyPointer {
        MoveOnlyPointer(std::nullptr_t) {}
        MoveOnlyPointer(const MoveOnlyPointer&) = delete;
        MoveOnlyPointer(MoveOnlyPointer&&) = default;
        MoveOnlyPointer& operator=(const MoveOnlyPointer&) = delete;
        MoveOnlyPointer& operator=(MoveOnlyPointer&&) = default;

        std::nullptr_t a;
    };

    struct MoveOnlyStruct {
        int a;
        char c;
        MoveOnlyPointer b;
    };

    /* This needs special handling on GCC 4.8, where T{std::move(b)} attempts
       to convert MoveOnlyStruct to int to initialize the first argument and
       fails miserably. */
    Containers::StaticArray<3, MoveOnlyStruct> a{DirectInit, 3, 'a', nullptr};
    CORRADE_COMPARE(a.front().a, 3);

    /* This move-constructs new values */
    Containers::StaticArray<3, MoveOnlyStruct> b{std::move(a)};
    CORRADE_COMPARE(b.front().a, 3);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayTest)
