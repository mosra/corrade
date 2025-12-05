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

    void constructValueInit();
    template<class T> void constructValueInitTrivial();
    #ifdef CORRADE_BUILD_DEPRECATED
    void constructDefaultInit();
    void constructDefaultInitTrivialConstructor();
    void constructDefaultInitDefaultConstructor();
    #endif
    void constructNoInit();
    template<class T> void constructNoInitTrivial();
    void constructNoInitNoDefaultConstructor();
    void constructInPlaceInit();
    template<class T> void constructInPlaceInitTrivial();
    void constructInPlaceInitOneArgument();
    template<class T> void constructInPlaceInitOneArgumentTrivial();
    void constructInPlaceInitMoveOnly();
    void constructDirectInit();
    template<class T> void constructDirectInitTrivial();
    void constructDirectInitMoveOnly();
    void constructImmovable();
    void constructNoImplicitConstructor();
    void constructDirectReferences();
    void constructArray();
    template<class T> void constructArrayTrivial();
    void constructArrayRvalue();
    void constructArrayMove();

    /* No constructZeroNullPointerAmbiguity() here as the StaticArray is never
       empty, thus never null, thus std::nullptr_t constructor makes no sense */

    void copy();
    template<class T> void copyTrivial();
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
    void sliceZeroNullPointerAmbiguity();

    void cast();
    void size();

    void constructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();
};

struct NoInitConstructible {
    constexpr NoInitConstructible(int a = 0): a{a} {}
    explicit NoInitConstructible(Corrade::NoInitT) {}
    operator int() const { return a; }
    int a;
};

StaticArrayTest::StaticArrayTest() {
    addTests({&StaticArrayTest::constructValueInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructValueInitTrivial<int>,
        &StaticArrayTest::constructValueInitTrivial<NoInitConstructible>});

    #ifdef CORRADE_BUILD_DEPRECATED
    addTests({&StaticArrayTest::constructDefaultInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests({&StaticArrayTest::constructDefaultInitTrivialConstructor,
              &StaticArrayTest::constructDefaultInitDefaultConstructor});
    #endif

    addTests({&StaticArrayTest::constructNoInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructNoInitTrivial<int>,
        &StaticArrayTest::constructNoInitTrivial<NoInitConstructible>,
        &StaticArrayTest::constructNoInitNoDefaultConstructor});

    addTests({&StaticArrayTest::constructInPlaceInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructInPlaceInitTrivial<int>,
        &StaticArrayTest::constructInPlaceInitTrivial<NoInitConstructible>});

    addTests({&StaticArrayTest::constructInPlaceInitOneArgument},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructInPlaceInitOneArgumentTrivial<int>,
        &StaticArrayTest::constructInPlaceInitOneArgumentTrivial<NoInitConstructible>});

    addTests({&StaticArrayTest::constructInPlaceInitMoveOnly,
              &StaticArrayTest::constructDirectInit},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructDirectInitTrivial<int>,
        &StaticArrayTest::constructDirectInitTrivial<NoInitConstructible>});

    addTests({&StaticArrayTest::constructDirectInitMoveOnly,
              &StaticArrayTest::constructImmovable},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests({&StaticArrayTest::constructNoImplicitConstructor,
              &StaticArrayTest::constructDirectReferences});

    addTests({&StaticArrayTest::constructArray},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::constructArrayTrivial<int>,
        &StaticArrayTest::constructArrayTrivial<NoInitConstructible>});

    addTests({&StaticArrayTest::constructArrayRvalue,
              &StaticArrayTest::constructArrayMove});

    addTests({&StaticArrayTest::copy},
        &StaticArrayTest::resetCounters, &StaticArrayTest::resetCounters);

    addTests<StaticArrayTest>({
        &StaticArrayTest::copyTrivial<int>,
        &StaticArrayTest::copyTrivial<NoInitConstructible>});

    addTests({&StaticArrayTest::move},
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
              &StaticArrayTest::sliceZeroNullPointerAmbiguity,

              &StaticArrayTest::cast,
              &StaticArrayTest::size,

              &StaticArrayTest::constructorExplicitInCopyInitialization,
              &StaticArrayTest::copyConstructPlainStruct,
              &StaticArrayTest::moveConstructPlainStruct});
}

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
    /* With the guaranteed copy/move elision in C++17 this one might be unused
       as well */
    #ifdef CORRADE_TARGET_CXX17
    CORRADE_UNUSED
    #endif
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
    /* Clang complains this function is unused. But removing it may have
       unintended consequences, so don't. */
    CORRADE_UNUSED Copyable& operator=(Copyable&& other) noexcept {
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

struct Throwable {
    /* Clang complains this function is unused. But removing it may have
       unintended consequences, so don't. */
    CORRADE_UNUSED explicit Throwable(int) {}
    Throwable(const Throwable&) {}
    Throwable(Throwable&&) {}
    Throwable& operator=(const Throwable&) { return *this; }
    Throwable& operator=(Throwable&&) { return *this; }
};

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
    Corrade::Utility::swap(a.a, b.a);
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

void StaticArrayTest::constructValueInit() {
    {
        /* Without the tag it should be implicitly constructible */
        const StaticArray<5, Copyable> a1 = {};
        const StaticArray<5, Copyable> a2{Corrade::ValueInit};
        CORRADE_VERIFY(a1);
        CORRADE_VERIFY(a2);
        CORRADE_VERIFY(!a1.isEmpty());
        CORRADE_VERIFY(!a2.isEmpty());
        CORRADE_COMPARE(a1.size(), (StaticArray<5, Copyable>::Size));
        CORRADE_COMPARE(a2.size(), (StaticArray<5, Copyable>::Size));
        CORRADE_COMPARE(a1.size(), 5);
        CORRADE_COMPARE(a2.size(), 5);

        /* Values should be zero-initialized (same as ValueInit) */
        CORRADE_COMPARE(a1[0].a, 0);
        CORRADE_COMPARE(a2[0].a, 0);
        CORRADE_COMPARE(a1[1].a, 0);
        CORRADE_COMPARE(a2[1].a, 0);
        CORRADE_COMPARE(a1[2].a, 0);
        CORRADE_COMPARE(a2[2].a, 0);
        CORRADE_COMPARE(a1[3].a, 0);
        CORRADE_COMPARE(a2[3].a, 0);
        CORRADE_COMPARE(a1[4].a, 0);
        CORRADE_COMPARE(a2[4].a, 0);

        CORRADE_COMPARE(Copyable::constructed, 10);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 10);
    CORRADE_COMPARE(Copyable::destructed, 10);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Size should be the same as a plain array, even with all the extra craze
       with base classes */
    CORRADE_COMPARE(sizeof(StaticArray<5, Copyable>), 5*sizeof(Copyable));

    /* Implicit construction with the tag is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::ValueInitT, StaticArray<5, Copyable>>::value);
}

template<class> struct TrivialTraits;
template<> struct TrivialTraits<int> {
    static const char* name() { return "int"; }
};
template<> struct TrivialTraits<NoInitConstructible> {
    static const char* name() { return "NoInitConstructible"; }
};

template<class T> void StaticArrayTest::constructValueInitTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    /* Without the tag it should be implicitly constructible */
    const StaticArray<5, T> a1 = {};
    const StaticArray<5, T> a2{Corrade::ValueInit};
    CORRADE_VERIFY(a1);
    CORRADE_VERIFY(a2);
    CORRADE_VERIFY(!a1.isEmpty());
    CORRADE_VERIFY(!a2.isEmpty());
    CORRADE_COMPARE(a1.size(), (StaticArray<5, T>::Size));
    CORRADE_COMPARE(a2.size(), (StaticArray<5, T>::Size));
    CORRADE_COMPARE(a1.size(), 5);
    CORRADE_COMPARE(a2.size(), 5);

    /* Values should be zero-initialized (same as ValueInit) */
    CORRADE_COMPARE(a1[0], 0);
    CORRADE_COMPARE(a2[0], 0);
    CORRADE_COMPARE(a1[1], 0);
    CORRADE_COMPARE(a2[1], 0);
    CORRADE_COMPARE(a1[2], 0);
    CORRADE_COMPARE(a2[2], 0);
    CORRADE_COMPARE(a1[3], 0);
    CORRADE_COMPARE(a2[3], 0);
    CORRADE_COMPARE(a1[4], 0);
    CORRADE_COMPARE(a2[4], 0);

    constexpr StaticArray<5, T> ca1;
    constexpr StaticArray<5, T> ca2{Corrade::ValueInit};
    constexpr bool bool1 = !!ca1;
    constexpr bool bool2 = !!ca2;
    constexpr bool empty1 = ca1.isEmpty();
    constexpr bool empty2 = ca2.isEmpty();
    constexpr std::size_t size1 = ca1.size();
    constexpr std::size_t size2 = ca2.size();
    CORRADE_VERIFY(bool1);
    CORRADE_VERIFY(bool2);
    CORRADE_VERIFY(!empty1);
    CORRADE_VERIFY(!empty2);
    CORRADE_COMPARE(size1, 5);
    CORRADE_COMPARE(size2, 5);

    /* Values should be zero-initialized (same as ValueInit) */
    CORRADE_COMPARE(ca1[0], 0);
    CORRADE_COMPARE(ca2[0], 0);
    CORRADE_COMPARE(ca1[1], 0);
    CORRADE_COMPARE(ca2[1], 0);
    CORRADE_COMPARE(ca1[2], 0);
    CORRADE_COMPARE(ca2[2], 0);
    CORRADE_COMPARE(ca1[3], 0);
    CORRADE_COMPARE(ca2[3], 0);
    CORRADE_COMPARE(ca1[4], 0);
    CORRADE_COMPARE(ca2[4], 0);

    /* Size should be the same as a plain array, even with all the extra craze
       with base classes */
    CORRADE_COMPARE(sizeof(StaticArray<5, T>), 5*sizeof(T));

    /* Implicit construction with the tag is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::ValueInitT, StaticArray<5, T>>::value);
}

#ifdef CORRADE_BUILD_DEPRECATED
void StaticArrayTest::constructDefaultInit() {
    {
        CORRADE_IGNORE_DEPRECATED_PUSH
        const StaticArray<5, Copyable> a{Corrade::DefaultInit};
        CORRADE_IGNORE_DEPRECATED_POP

        /* Values should be default-constructed for non-trivial types */
        CORRADE_COMPARE(a[0].a, 0);
        CORRADE_COMPARE(a[1].a, 0);
        CORRADE_COMPARE(a[2].a, 0);
        CORRADE_COMPARE(a[3].a, 0);
        CORRADE_COMPARE(a[4].a, 0);

        CORRADE_COMPARE(Copyable::constructed, 5);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 5);
    CORRADE_COMPARE(Copyable::destructed, 5);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DefaultInitT, StaticArray<5, Copyable>>::value);
}

void StaticArrayTest::constructDefaultInitTrivialConstructor() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    const StaticArray<5, int> a{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP

    /* Values are random memory */

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DefaultInitT, StaticArray<5, int>>::value);
}

void StaticArrayTest::constructDefaultInitDefaultConstructor() {
    CORRADE_IGNORE_DEPRECATED_PUSH
    const StaticArray<5, NoInitConstructible> a{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP

    /* Values are default-constructed */
    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 0);
    CORRADE_COMPARE(a[2], 0);
    CORRADE_COMPARE(a[3], 0);
    CORRADE_COMPARE(a[4], 0);

    CORRADE_IGNORE_DEPRECATED_PUSH
    constexpr StaticArray<5, NoInitConstructible> ca{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_COMPARE(ca[0], 0);
    CORRADE_COMPARE(ca[1], 0);
    CORRADE_COMPARE(ca[2], 0);
    CORRADE_COMPARE(ca[3], 0);
    CORRADE_COMPARE(ca[4], 0);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DefaultInitT, StaticArray<5, NoInitConstructible>>::value);
}
#endif

void StaticArrayTest::constructNoInit() {
    {
        StaticArray<3, Copyable> a{Corrade::InPlaceInit, 57, 39, 78};
        CORRADE_COMPARE(Copyable::constructed, 3);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);

        new(&a) StaticArray<3, Copyable>{Corrade::NoInit};
        CORRADE_COMPARE(Copyable::constructed, 3);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 3);
    CORRADE_COMPARE(Copyable::destructed, 3);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, StaticArray<5, Copyable>>::value);
}

template<class T> void StaticArrayTest::constructNoInitTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    StaticArray<3, T> a{Corrade::InPlaceInit, 57, 39, 78};
    new(&a) StaticArray<3, T>{Corrade::NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a[0], 57);
        CORRADE_COMPARE(a[1], 39);
        CORRADE_COMPARE(a[2], 78);
    }

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, StaticArray<5, T>>::value);
}

/* A variant of these is used in ArrayTest, PairTest and TripleTest */
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

void StaticArrayTest::constructNoInitNoDefaultConstructor() {
    /* In libstdc++ before version 8 std::is_trivially_constructible<T> doesn't
       work with (template) types where the default constructor isn't usable,
       failing compilation instead of producing std::false_type; in version 4.8
       this trait isn't available at all. std::is_trivial is used instead,
       verify that it compiles correctly everywhere. */

    StaticArray<3, Wrapped<NoDefaultConstructor>> a{Corrade::NoInit};
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.size(), 3);
}

void StaticArrayTest::constructInPlaceInit() {
    {
        /* Without the tag it should be implicitly constructible */
        const StaticArray<5, Copyable> a = {10, 20, 30, 40, 50};
        const StaticArray<5, Copyable> b{Corrade::InPlaceInit, 10, 20, 30, 40, 50};

        CORRADE_COMPARE(a[0].a, 10);
        CORRADE_COMPARE(b[0].a, 10);
        CORRADE_COMPARE(a[1].a, 20);
        CORRADE_COMPARE(b[1].a, 20);
        CORRADE_COMPARE(a[2].a, 30);
        CORRADE_COMPARE(b[2].a, 30);
        CORRADE_COMPARE(a[3].a, 40);
        CORRADE_COMPARE(b[3].a, 40);
        CORRADE_COMPARE(a[4].a, 50);
        CORRADE_COMPARE(b[4].a, 50);

        CORRADE_COMPARE(Copyable::constructed, 10);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 10);
    CORRADE_COMPARE(Copyable::destructed, 10);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Implicit construction with the tag is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::InPlaceInitT, StaticArray<5, Copyable>>::value);
}

template<class T> void StaticArrayTest::constructInPlaceInitTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    /* Without the tag it should be implicitly constructible */
    const StaticArray<5, T> a = {10, 20, 30, 40, 50};
    const StaticArray<5, T> b{Corrade::InPlaceInit, 10, 20, 30, 40, 50};
    CORRADE_COMPARE(a[0], 10);
    CORRADE_COMPARE(b[0], 10);
    CORRADE_COMPARE(a[1], 20);
    CORRADE_COMPARE(b[1], 20);
    CORRADE_COMPARE(a[2], 30);
    CORRADE_COMPARE(b[2], 30);
    CORRADE_COMPARE(a[3], 40);
    CORRADE_COMPARE(b[3], 40);
    CORRADE_COMPARE(a[4], 50);
    CORRADE_COMPARE(b[4], 50);

    constexpr StaticArray<5, T> ca = {10, 20, 30, 40, 50};
    constexpr StaticArray<5, T> cb{Corrade::InPlaceInit, 10, 20, 30, 40, 50};
    CORRADE_COMPARE(ca[0], 10);
    CORRADE_COMPARE(cb[0], 10);
    CORRADE_COMPARE(ca[1], 20);
    CORRADE_COMPARE(cb[1], 20);
    CORRADE_COMPARE(ca[2], 30);
    CORRADE_COMPARE(cb[2], 30);
    CORRADE_COMPARE(ca[3], 40);
    CORRADE_COMPARE(cb[3], 40);
    CORRADE_COMPARE(ca[4], 50);
    CORRADE_COMPARE(cb[4], 50);

    /* It should always be constructible only with exactly the matching number
       of elements. As that's checked with a static_assert(), it's impossible
       to verify with std::is_constructible unfortunately and the only way to
       test that is manually, thus uncomment the code below to test the error
       behavior.

       Additionally, to avoid noise in the compiler output, the first two
       should only produce "excess elements in array initializer" and a static
       assert, the second two just a static assert, no other compiler error. */
    #if 0
    StaticArray<3, T> a3{1, 2, 3, 4};
    StaticArray<3, T> a4{Corrade::InPlaceInit, 1, 2, 3, 4};
    StaticArray<3, T> a5{1, 2};
    StaticArray<3, T> a6{Corrade::InPlaceInit, 1, 2};
    #endif

    /* Implicit construction with the tag is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::InPlaceInitT, StaticArray<5, T>>::value);
}

void StaticArrayTest::constructInPlaceInitOneArgument() {
    {
        /* Should be implicitly constructible */
        const StaticArray<1, Copyable> a = 17;
        CORRADE_COMPARE(a[0].a, 17);

        CORRADE_COMPARE(Copyable::constructed, 1);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 1);
    CORRADE_COMPARE(Copyable::destructed, 1);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);
}

template<class T> void StaticArrayTest::constructInPlaceInitOneArgumentTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    /* Should be implicitly constructible */
    const StaticArray<1, T> a = 17;
    CORRADE_COMPARE(a[0], 17);

    constexpr StaticArray<1, T> ca = 17;
    CORRADE_COMPARE(ca[0], 17);
}

void StaticArrayTest::constructInPlaceInitMoveOnly() {
    {
        const StaticArray<3, Movable> a{Movable{1}, Movable{2}, Movable{3}};
        const StaticArray<3, Movable> b{Corrade::InPlaceInit, Movable{1}, Movable{2}, Movable{3}};

        CORRADE_COMPARE(a[0].a, 1);
        CORRADE_COMPARE(b[0].a, 1);
        CORRADE_COMPARE(a[1].a, 2);
        CORRADE_COMPARE(b[1].a, 2);
        CORRADE_COMPARE(a[2].a, 3);
        CORRADE_COMPARE(b[2].a, 3);

        /* 6 temporaries that were moved to the concrete places 6 times */
        CORRADE_COMPARE(Movable::constructed, 6 + 6);
        CORRADE_COMPARE(Movable::destructed, 6);
        CORRADE_COMPARE(Movable::moved, 6);
    }

    CORRADE_COMPARE(Movable::constructed, 6 + 6);
    CORRADE_COMPARE(Movable::destructed, 6 + 6);
    CORRADE_COMPARE(Movable::moved, 6);
}

void StaticArrayTest::constructDirectInit() {
    {
        const StaticArray<5, Copyable> a{Corrade::DirectInit, -37};
        CORRADE_COMPARE(a[0].a, -37);
        CORRADE_COMPARE(a[1].a, -37);
        CORRADE_COMPARE(a[2].a, -37);
        CORRADE_COMPARE(a[3].a, -37);
        CORRADE_COMPARE(a[4].a, -37);

        CORRADE_COMPARE(Copyable::constructed, 5);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 5);
    CORRADE_COMPARE(Copyable::destructed, 5);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DirectInitT, StaticArray<5, Copyable>>::value);
}

template<class T> void StaticArrayTest::constructDirectInitTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    const StaticArray<5, T> a{Corrade::DirectInit, -37};
    CORRADE_COMPARE(a[0], -37);
    CORRADE_COMPARE(a[1], -37);
    CORRADE_COMPARE(a[2], -37);
    CORRADE_COMPARE(a[3], -37);
    CORRADE_COMPARE(a[4], -37);

    /* DirectInit delegates to NoInit, so it can't be constexpr */

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DirectInitT, StaticArray<5, T>>::value);
}

void StaticArrayTest::constructDirectInitMoveOnly() {
    {
        /* This one is weird as it moves one argument 3 times, but should work
           nevertheless */
        const StaticArray<3, Movable> a{Corrade::DirectInit, Movable{-37}};
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

void StaticArrayTest::constructImmovable() {
    #ifdef CORRADE_BUILD_DEPRECATED
    CORRADE_IGNORE_DEPRECATED_PUSH
    StaticArray<5, Immovable> a{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP
    #endif
    StaticArray<5, Immovable> b{Corrade::ValueInit};
    StaticArray<5, Immovable> c;
    #ifdef CORRADE_BUILD_DEPRECATED
    CORRADE_VERIFY(a);
    #endif
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(c);
}

void StaticArrayTest::constructNoImplicitConstructor() {
    struct NoImplicitConstructor {
        NoImplicitConstructor(int i): i{i} {}

        int i;
    };

    const StaticArray<5, NoImplicitConstructor> a{Corrade::DirectInit, 5};
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(a[0].i, 5);
    CORRADE_COMPARE(a[1].i, 5);
    CORRADE_COMPARE(a[2].i, 5);
    CORRADE_COMPARE(a[3].i, 5);
    CORRADE_COMPARE(a[4].i, 5);

    const StaticArray<5, NoImplicitConstructor> b{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
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

    const StaticArray<5, Reference> b{Corrade::DirectInit, a};
    CORRADE_VERIFY(b);
}

void StaticArrayTest::constructArray() {
    struct PairOfInts {
        Copyable a, b;
    };

    const PairOfInts data[]{
        {1, 2},
        {3, 4},
        {5, 6}
    };

    CORRADE_COMPARE(Copyable::constructed, 6);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    {
        /* Should be implicitly constructible */
        StaticArray<3, PairOfInts> a1 = data;
        StaticArray<3, PairOfInts> a2 = {Corrade::InPlaceInit, data};
        CORRADE_COMPARE(a1[0].a.a, 1);
        CORRADE_COMPARE(a2[0].a.a, 1);
        CORRADE_COMPARE(a1[0].b.a, 2);
        CORRADE_COMPARE(a2[0].b.a, 2);
        CORRADE_COMPARE(a1[1].a.a, 3);
        CORRADE_COMPARE(a2[1].a.a, 3);
        CORRADE_COMPARE(a1[1].b.a, 4);
        CORRADE_COMPARE(a2[1].b.a, 4);
        CORRADE_COMPARE(a1[2].a.a, 5);
        CORRADE_COMPARE(a2[2].a.a, 5);
        CORRADE_COMPARE(a1[2].b.a, 6);
        CORRADE_COMPARE(a2[2].b.a, 6);

        CORRADE_COMPARE(Copyable::constructed, 6 + 12);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 12);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 6 + 12);
    CORRADE_COMPARE(Copyable::destructed, 12);
    CORRADE_COMPARE(Copyable::copied, 12);
    CORRADE_COMPARE(Copyable::moved, 0);
}

template<class> struct PairOfInts;
template<> struct PairOfInts<int> {
    int a, b;
};
template<> struct PairOfInts<NoInitConstructible> {
    constexpr PairOfInts(int a = 0, int b = 0): a{a}, b{b} {}
    PairOfInts(Corrade::NoInitT): a{Corrade::NoInit}, b{Corrade::NoInit} {}

    NoInitConstructible a, b;
};

template<class T> void StaticArrayTest::constructArrayTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    const PairOfInts<T> data[]{
        {1, 2},
        {3, 4},
        {5, 6}
    };
    /* Should be implicitly constructible */
    StaticArray<3, PairOfInts<T>> a1 = data;
    StaticArray<3, PairOfInts<T>> a2 = {Corrade::InPlaceInit, data};
    CORRADE_COMPARE(a1[0].a, 1);
    CORRADE_COMPARE(a2[0].a, 1);
    CORRADE_COMPARE(a1[0].b, 2);
    CORRADE_COMPARE(a2[0].b, 2);
    CORRADE_COMPARE(a1[1].a, 3);
    CORRADE_COMPARE(a2[1].a, 3);
    CORRADE_COMPARE(a1[1].b, 4);
    CORRADE_COMPARE(a2[1].b, 4);
    CORRADE_COMPARE(a1[2].a, 5);
    CORRADE_COMPARE(a2[2].a, 5);
    CORRADE_COMPARE(a1[2].b, 6);
    CORRADE_COMPARE(a2[2].b, 6);

    constexpr PairOfInts<T> cdata[]{
        {1, 2},
        {3, 4},
        {5, 6}
    };
    constexpr StaticArray<3, PairOfInts<T>> ca1 = cdata;
    constexpr StaticArray<3, PairOfInts<T>> ca2 = {Corrade::InPlaceInit, cdata};
    CORRADE_COMPARE(ca1[0].a, 1);
    CORRADE_COMPARE(ca2[0].a, 1);
    CORRADE_COMPARE(ca1[0].b, 2);
    CORRADE_COMPARE(ca2[0].b, 2);
    CORRADE_COMPARE(ca1[1].a, 3);
    CORRADE_COMPARE(ca2[1].a, 3);
    CORRADE_COMPARE(ca1[1].b, 4);
    CORRADE_COMPARE(ca2[1].b, 4);
    CORRADE_COMPARE(ca1[2].a, 5);
    CORRADE_COMPARE(ca2[2].a, 5);
    CORRADE_COMPARE(ca1[2].b, 6);
    CORRADE_COMPARE(ca2[2].b, 6);

    /* It should always be constructible only with exactly the matching number
       of elements. As that's checked with a static_assert(), it's impossible
       to verify with std::is_constructible unfortunately and the only way to
       test that is manually, thus uncomment the code below to test the error
       behavior.

       Additionally, to avoid noise in the compiler output, the first two
       should only produce "excess elements in array initializer" and a static
       assert, the second two just a static assert, no other compiler error.

       Unlike the rvalue case below, the second two cases don't compile on GCC
       4.8 --- it picks the variadic constructor instead, failing in a
       different way. */
    #if 0
    StaticArray<2, PairOfInts<T>> a3{data};
    StaticArray<2, PairOfInts<T>> a4{Corrade::InPlaceInit, data};
    StaticArray<4, PairOfInts<T>> a5{data};
    StaticArray<4, PairOfInts<T>> a6{Corrade::InPlaceInit, data};
    #endif
}

void StaticArrayTest::constructArrayRvalue() {
    struct PairOfInts {
        int a, b;
    };

    /* Should be implicitly constructible. Not sure why the second {{}}s are
       needed in the first case, though. */
    StaticArray<3, PairOfInts> a1 = {{
        {1, 2},
        {3, 4},
        {5, 6}
    }};
    StaticArray<3, PairOfInts> a2 = {Corrade::InPlaceInit, {
        {1, 2},
        {3, 4},
        {5, 6}
    }};
    CORRADE_COMPARE(a1[0].a, 1);
    CORRADE_COMPARE(a2[0].a, 1);
    CORRADE_COMPARE(a1[0].b, 2);
    CORRADE_COMPARE(a2[0].b, 2);
    CORRADE_COMPARE(a1[1].a, 3);
    CORRADE_COMPARE(a2[1].a, 3);
    CORRADE_COMPARE(a1[1].b, 4);
    CORRADE_COMPARE(a2[1].b, 4);
    CORRADE_COMPARE(a1[2].a, 5);
    CORRADE_COMPARE(a2[2].a, 5);
    CORRADE_COMPARE(a1[2].b, 6);
    CORRADE_COMPARE(a2[2].b, 6);

    /* It should always be constructible only with exactly the matching number
       of elements. As that's checked with a static_assert(), it's impossible
       to verify with std::is_constructible unfortunately and the only way to
       test that is manually, thus uncomment the code below to test the error
       behavior.

       Additionally, to avoid noise in the compiler output, the first two
       should only produce "excess elements in array initializer" and a static
       assert, the second two just a static assert, no other compiler error. */
    #if 0
    StaticArray<3, PairOfInts> a3{{
        {1, 2},
        {3, 4},
        {5, 6},
        {7, 8}
    }};
    StaticArray<3, PairOfInts> a4{Corrade::InPlaceInit, {
        {1, 2},
        {3, 4},
        {5, 6},
        {7, 8}
    }};
    #endif
    #if 0 || (defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5)
    CORRADE_WARN("Creating a StaticArray from a smaller array isn't an error on GCC 4.8.");
    StaticArray<3, PairOfInts> a5{{
        {1, 2},
        {3, 4}
    }};
    StaticArray<3, PairOfInts> a6{Corrade::InPlaceInit, {
        {1, 2},
        {3, 4}
    }};
    #endif
}

void StaticArrayTest::constructArrayMove() {
    #ifdef CORRADE_MSVC2017_COMPATIBILITY
    /* MSVC 2015 and 2017 fails with
        error C2440: 'return': cannot convert from 'T [3]' to 'T (&&)[3]'
        Corrade/Utility/Move.h(88): note: You cannot bind an lvalue to an rvalue reference
       on the Utility::move() call inside the r-value constructor, std::move()
       behaves the same. Because of that, only the copying constructor can be
       enabled, as otherwise it would pick it for the constructArrayRvalue()
       above as well and fail even in the case where nothing needs to be
       moved. */
    CORRADE_SKIP("MSVC 2015 and 2017 isn't able to move arrays.");
    #elif defined(CORRADE_TARGET_GCC) && __GNUC__ >= 10 && __GNUC__ < 12
    /* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104996, unfortunately
       there doesn't seem to be any clear workaround */
    CORRADE_SKIP("GCC 11 and 12 isn't able to move arrays due to a regression.");
    #else
    struct MovableInt {
        Movable a;
        int b;
    };

    {
        /* Should be implicitly constructible. Not sure why the second {{}}s
           are needed in the first case, though. */
        StaticArray<3, MovableInt> a1 = {{
            {Movable{1}, 2},
            {Movable{3}, 4},
            {Movable{5}, 6}
        }};
        StaticArray<3, MovableInt> a2 = {Corrade::InPlaceInit, {
            {Movable{1}, 2},
            {Movable{3}, 4},
            {Movable{5}, 6}
        }};
        CORRADE_COMPARE(a1[0].a.a, 1);
        CORRADE_COMPARE(a2[0].a.a, 1);
        CORRADE_COMPARE(a1[0].b, 2);
        CORRADE_COMPARE(a2[0].b, 2);
        CORRADE_COMPARE(a1[1].a.a, 3);
        CORRADE_COMPARE(a2[1].a.a, 3);
        CORRADE_COMPARE(a1[1].b, 4);
        CORRADE_COMPARE(a2[1].b, 4);
        CORRADE_COMPARE(a1[2].a.a, 5);
        CORRADE_COMPARE(a2[2].a.a, 5);
        CORRADE_COMPARE(a1[2].b, 6);
        CORRADE_COMPARE(a2[2].b, 6);

        /* 6 temporaries that were moved to the concrete places 6 times */
        CORRADE_COMPARE(Movable::constructed, 6 + 6);
        CORRADE_COMPARE(Movable::destructed, 6);
        CORRADE_COMPARE(Movable::moved, 6);
    }

    CORRADE_COMPARE(Movable::constructed, 6 + 6);
    CORRADE_COMPARE(Movable::destructed, 6 + 6);
    CORRADE_COMPARE(Movable::moved, 6);
    #endif
}

void StaticArrayTest::copy() {
    {
        StaticArray<3, Copyable> a{Corrade::InPlaceInit, 1, 2, 3};
        StaticArray<3, Copyable> b = a;
        CORRADE_COMPARE(b[0].a, 1);
        CORRADE_COMPARE(b[1].a, 2);
        CORRADE_COMPARE(b[2].a, 3);

        StaticArray<3, Copyable> c;
        c = b;
        CORRADE_COMPARE(c[0].a, 1);
        CORRADE_COMPARE(c[1].a, 2);
        CORRADE_COMPARE(c[2].a, 3);

        CORRADE_COMPARE(Copyable::constructed, 9);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 6);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 9);
    CORRADE_COMPARE(Copyable::destructed, 9);
    CORRADE_COMPARE(Copyable::copied, 6);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<StaticArray<3, Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<StaticArray<3, Copyable>>::value);

    CORRADE_VERIFY(std::is_copy_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<StaticArray<3, Throwable>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<StaticArray<3, Throwable>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!std::is_trivially_copy_constructible<StaticArray<3, Copyable>>::value);
    CORRADE_VERIFY(!std::is_trivially_copy_assignable<StaticArray<3, Copyable>>::value);
    CORRADE_VERIFY(!std::is_trivially_copyable<StaticArray<3, Copyable>>::value);
    CORRADE_VERIFY(!std::is_trivially_copy_constructible<StaticArray<3, Throwable>>::value);
    CORRADE_VERIFY(!std::is_trivially_copy_assignable<StaticArray<3, Throwable>>::value);
    CORRADE_VERIFY(!std::is_trivially_copyable<StaticArray<3, Throwable>>::value);
    #endif
}

template<class T> void StaticArrayTest::copyTrivial() {
    setTestCaseTemplateName(TrivialTraits<T>::name());

    StaticArray<3, T> a{Corrade::InPlaceInit, 1, 2, 3};
    StaticArray<3, T> b = a;
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(b[1], 2);
    CORRADE_COMPARE(b[2], 3);

    StaticArray<3, T> c;
    c = b;
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    constexpr StaticArray<3, T> ca{Corrade::InPlaceInit, 1, 2, 3};
    constexpr StaticArray<3, T> cb = ca;
    CORRADE_COMPARE(cb[0], 1);
    CORRADE_COMPARE(cb[1], 2);
    CORRADE_COMPARE(cb[2], 3);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<T>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<StaticArray<3, T>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<T>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<StaticArray<3, T>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<StaticArray<3, T>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<StaticArray<3, T>>::value);
    CORRADE_VERIFY(std::is_trivially_copyable<StaticArray<3, T>>::value);
    #endif
}

void StaticArrayTest::move() {
    {
        StaticArray<3, Movable> a{Corrade::InPlaceInit, 1, 2, 3};

        StaticArray<3, Movable> b{Utility::move(a)};
        CORRADE_COMPARE(b[0].a, 1);
        CORRADE_COMPARE(b[1].a, 2);
        CORRADE_COMPARE(b[2].a, 3);

        StaticArray<3, Movable> c;
        c = Utility::move(b); /* this uses the swap() specialization -> no move */
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
        CORRADE_VERIFY(!std::is_copy_constructible<StaticArray<3, Movable>>::value);
        CORRADE_VERIFY(!std::is_copy_assignable<StaticArray<3, Movable>>::value);
    }

    CORRADE_VERIFY(std::is_move_constructible<StaticArray<3, Movable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<StaticArray<3, Movable>>::value);
    CORRADE_VERIFY(std::is_move_assignable<StaticArray<3, Movable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<StaticArray<3, Movable>>::value);

    CORRADE_VERIFY(std::is_move_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<StaticArray<3, Throwable>>::value);
    CORRADE_VERIFY(std::is_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<StaticArray<3, Throwable>>::value);
}

void StaticArrayTest::convertBool() {
    CORRADE_VERIFY(StaticArray<5, int>{});

    constexpr StaticArray<5, int> ca;
    CORRADE_VERIFY(ca);

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY(std::is_constructible<bool, StaticArray<5, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<int, StaticArray<5, int>>::value);
}

constexpr StaticArray<5, int> Array5{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

void StaticArrayTest::convertPointer() {
    StaticArray<5, int> a;
    int* b = a;
    CORRADE_COMPARE(b, a.begin());

    const StaticArray<5, int> c;
    const int* d = c;
    CORRADE_COMPARE(d, c.begin());

    /* Pointer arithmetic */
    const StaticArray<5, int> e;
    const int* f = e + 2;
    CORRADE_COMPARE(f, &e[2]);

    constexpr const int* cd = Array5;
    CORRADE_COMPARE(cd, Array5.begin());

    /* Verify that we can't convert rvalues. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<int*, StaticArray<5, int>&>::value);
    CORRADE_VERIFY(std::is_constructible<const int*, const StaticArray<5, int>&>::value);
    CORRADE_VERIFY(!std::is_constructible<int*, StaticArray<5, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<int*, StaticArray<5, int>&&>::value);

    /* Deleting const&& overload and leaving only const& one will not, in fact,
       disable conversion of const Array&& to pointer, but rather make the
       conversion ambiguous, which is not what we want, as it breaks e.g.
       rvalueArrayAccess() test. Not using is_convertible to catch also
       accidental explicit conversions. */
    {
        CORRADE_EXPECT_FAIL("I don't know how to properly disable conversion of const Array&& to pointer.");
        CORRADE_VERIFY(!std::is_constructible<const int*, const StaticArray<5, int>>::value);
        CORRADE_VERIFY(!std::is_constructible<const int*, const StaticArray<5, int>&&>::value);
    }
}

constexpr StaticArray<5, const int> Array5C;

void StaticArrayTest::convertView() {
    StaticArray<5, int> a;
    const StaticArray<5, int> ca;
    StaticArray<5, const int> ac;
    const StaticArray<5, const int> cac;

    {
        const ArrayView<int> b = a;
        const ArrayView<const int> cb = ca;
        const ArrayView<const int> bc = ac;
        const ArrayView<const int> cbc = cac;
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        constexpr ArrayView<const int> cb = Array5;
        constexpr ArrayView<const int> cbc = Array5C;
        CORRADE_VERIFY(cb.begin() == Array5.begin());
        CORRADE_VERIFY(cbc.begin() == Array5C.begin());
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        const auto b = arrayView(a);
        const auto cb = arrayView(ca);
        const auto bc = arrayView(ac);
        const auto cbc = arrayView(cac);
        CORRADE_VERIFY(std::is_same<decltype(b), const ArrayView<int>>::value);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ArrayView<const int>>::value);
        CORRADE_VERIFY(std::is_same<decltype(bc), const ArrayView<const int>>::value);
        CORRADE_VERIFY(std::is_same<decltype(cbc), const ArrayView<const int>>::value);
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        constexpr auto cb = arrayView(Array5);
        constexpr auto cbc = arrayView(Array5C);
        CORRADE_VERIFY(std::is_same<decltype(cb), const ArrayView<const int>>::value);
        CORRADE_VERIFY(std::is_same<decltype(cbc), const ArrayView<const int>>::value);
        CORRADE_VERIFY(cb.begin() == Array5.begin());
        CORRADE_VERIFY(cbc.begin() == Array5C.begin());
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    }
}

struct A { int i; };
struct B: A {
    explicit B(Corrade::NoInitT) {}
    constexpr B(int i = 0): A{i} {}
};
constexpr StaticArray<5, B> ArrayB5;

void StaticArrayTest::convertViewDerived() {
    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    StaticArray<5, B> b;
    ArrayView<A> a = b;
    CORRADE_VERIFY(a == b);
    CORRADE_COMPARE(a.size(), 5);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const A> ca = ArrayB5;
    CORRADE_VERIFY(ca == ArrayB5);
    CORRADE_COMPARE(ca.size(), 5);
}

bool takesAView(ArrayView<int>) { return true; }
bool takesAConstView(ArrayView<const int>) { return true; }
CORRADE_UNUSED bool takesAView(ArrayView<float>) { return false; }
CORRADE_UNUSED bool takesAConstView(ArrayView<const float>) { return false; }

void StaticArrayTest::convertViewOverload() {
    StaticArray<5, int> a;
    const StaticArray<5, int> ca;

    /* It should pick the correct one and not fail, assert or be ambiguous */
    CORRADE_VERIFY(takesAView(a));
    CORRADE_VERIFY(takesAConstView(a));
    CORRADE_VERIFY(takesAConstView(ca));
}

void StaticArrayTest::convertStaticView() {
    StaticArray<5, int> a;
    const StaticArray<5, int> ca;
    StaticArray<5, const int> ac;
    const StaticArray<5, const int> cac;

    {
        const StaticArrayView<5, int> b = a;
        const StaticArrayView<5, const int> cb = ca;
        const StaticArrayView<5, const int> bc = ac;
        const StaticArrayView<5, const int> cbc = cac;
        CORRADE_VERIFY(b.begin() == a.begin());
        CORRADE_VERIFY(bc.begin() == ac.begin());
        CORRADE_VERIFY(cb.begin() == ca.begin());
        CORRADE_VERIFY(cbc.begin() == cac.begin());
        CORRADE_COMPARE(b.size(), 5);
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(bc.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        constexpr StaticArrayView<5, const int> cb = Array5;
        constexpr StaticArrayView<5, const int> cbc = Array5C;
        CORRADE_VERIFY(cb.begin() == Array5.begin());
        CORRADE_VERIFY(cbc.begin() == Array5C.begin());
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    } {
        constexpr auto cb = staticArrayView(Array5);
        constexpr auto cbc = staticArrayView(Array5C);
        CORRADE_VERIFY(std::is_same<decltype(cb), const StaticArrayView<5, const int>>::value);
        CORRADE_VERIFY(std::is_same<decltype(cbc), const StaticArrayView<5, const int>>::value);
        CORRADE_VERIFY(cb.begin() == Array5.begin());
        CORRADE_VERIFY(cbc.begin() == Array5C.begin());
        CORRADE_COMPARE(cb.size(), 5);
        CORRADE_COMPARE(cbc.size(), 5);
    }
}

void StaticArrayTest::convertStaticViewDerived() {
    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    StaticArray<5, B> b;
    StaticArrayView<5, A> a = b;
    CORRADE_VERIFY(a == b);
    CORRADE_COMPARE(a.size(), 5);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<5, const A> ca = ArrayB5;
    CORRADE_VERIFY(ca == ArrayB5);
    CORRADE_COMPARE(ca.size(), 5);
}

bool takesAStaticView(StaticArrayView<5, int>) { return true; }
bool takesAStaticConstView(StaticArrayView<5, const int>) { return true; }
CORRADE_UNUSED bool takesAStaticView(StaticArrayView<5, float>) { return false; }
CORRADE_UNUSED bool takesAStaticConstView(StaticArrayView<5, const float>) { return false; }

void StaticArrayTest::convertStaticViewOverload() {
    StaticArray<5, int> a;
    const StaticArray<5, int> ca;

    /* It should pick the correct one and not fail, assert or be ambiguous */
    CORRADE_VERIFY(takesAStaticView(a));
    CORRADE_VERIFY(takesAStaticConstView(a));
    CORRADE_VERIFY(takesAStaticConstView(ca));
}

void StaticArrayTest::convertVoid() {
    StaticArray<5, int> a;
    ArrayView<void> b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_COMPARE(b.size(), 5*sizeof(int));
}

void StaticArrayTest::convertConstVoid() {
    StaticArray<5, int> a;
    constexpr StaticArray<5, int> ca;

    {
        ArrayView<const void> b = a;
        ArrayView<const void> cb = ca;
        CORRADE_VERIFY(b == a);
        CORRADE_VERIFY(cb == ca);
        CORRADE_COMPARE(b.size(), 5*sizeof(int));
        CORRADE_COMPARE(cb.size(), 5*sizeof(int));
    } {
        constexpr ArrayView<const void> cb = Array5;
        CORRADE_VERIFY(cb == Array5);
        CORRADE_COMPARE(cb.size(), 5*sizeof(int));
    }
}

void StaticArrayTest::convertToExternalView() {
    StaticArray<5, int> a{1, 2, 3, 4, 5};

    IntView5 b = a;
    CORRADE_COMPARE(b.data, a.data());

    ConstIntView5 cb = a;
    CORRADE_COMPARE(cb.data, a.data());

    /* Conversion to a different size or type is not allowed */
    /** @todo For some reason I can't use is_constructible here because it
       doesn't consider conversion operators in this case. In others (such as
       with ArrayView) it does, why? */
    CORRADE_VERIFY(std::is_convertible<StaticArray<5, int>, IntView5>::value);
    CORRADE_VERIFY(std::is_convertible<StaticArray<5, int>, ConstIntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<StaticArray<6, int>, IntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<StaticArray<6, int>, ConstIntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<StaticArray<5, float>, IntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<StaticArray<5, float>, ConstIntView5>::value);
}

void StaticArrayTest::convertToConstExternalView() {
    const StaticArray<5, int> a{1, 2, 3, 4, 5};

    ConstIntView5 b = a;
    CORRADE_COMPARE(b.data, a.data());

    constexpr StaticArray<5, int> ca{1, 2, 3, 4, 5};

    ConstIntView5 cb = ca;
    CORRADE_COMPARE(cb.data, ca.data());

    /* Conversion to a different size or type is not allowed */
    /** @todo For some reason I can't use is_constructible here because it
       doesn't consider conversion operators in this case. In others (such as
       with ArrayView) it does, why? */
    CORRADE_VERIFY(std::is_convertible<const StaticArray<5, int>, ConstIntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<const StaticArray<6, int>, ConstIntView5>::value);
    CORRADE_VERIFY(!std::is_convertible<const StaticArray<5, float>, ConstIntView5>::value);
}

void StaticArrayTest::access() {
    StaticArray<5, int> a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};

    CORRADE_COMPARE(a.data(), static_cast<int*>(a));
    CORRADE_COMPARE(a.front(), 1);
    CORRADE_COMPARE(a.back(), 5);
    CORRADE_COMPARE(*(a.begin() + 2), 3);
    CORRADE_COMPARE(a[4], 5);
    CORRADE_COMPARE(a.end() - a.begin(), 5);
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

void StaticArrayTest::accessConst() {
    const StaticArray<5, int> a{Corrade::InPlaceInit, {1, 2, 3, 4, 5}};
    CORRADE_COMPARE(a.data(), static_cast<const int*>(a));
    CORRADE_COMPARE(a.front(), 1);
    CORRADE_COMPARE(a.back(), 5);
    CORRADE_COMPARE(*(a.begin() + 2), 3);
    CORRADE_COMPARE(a[4], 5);
    CORRADE_COMPARE(a.end() - a.begin(), 5);
    CORRADE_COMPARE(a.cbegin(), a.begin());
    CORRADE_COMPARE(a.cend(), a.end());

    constexpr const int* data = Array5.data();
    constexpr const int& front = Array5.front();
    constexpr const int& back = Array5.back();
    constexpr const int* begin = Array5.begin();
    constexpr const int* cbegin = Array5.cbegin();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* This causes an ICE */
    #endif
    const int* end = Array5.end();
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* This causes an ICE */
    #endif
    const int* cend = Array5.cend();
    constexpr const int& fourth = Array5[4];
    CORRADE_COMPARE(data, static_cast<const int*>(Array5));
    CORRADE_COMPARE(front, 1);
    CORRADE_COMPARE(back, 5);
    CORRADE_COMPARE(*(begin + 2), 3);
    CORRADE_COMPARE(fourth, 5);
    CORRADE_COMPARE(end - begin, 5);
    CORRADE_COMPARE(cbegin, begin);
    CORRADE_COMPARE(cend, end);
}

void StaticArrayTest::rvalueArrayAccess() {
    CORRADE_COMPARE((StaticArray<5, int>{Corrade::DirectInit, 3})[2], 3);
}

void StaticArrayTest::rangeBasedFor() {
    StaticArray<5, int> a;
    for(auto& i: a)
        i = 3;

    CORRADE_COMPARE(a[0], 3);
    CORRADE_COMPARE(a[1], 3);
    CORRADE_COMPARE(a[2], 3);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 3);

    /* To verify the constant begin()/end() accessors */
    const StaticArray<5, int>& ca = a;
    for(auto&& i: ca)
        CORRADE_COMPARE(i, 3);
}

void StaticArrayTest::slice() {
    StaticArray<5, int> a{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray<5, int> ac{Corrade::InPlaceInit, 1, 2, 3, 4, 5};

    ArrayView<int> b1 = a.slice(1, 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ArrayView<const int> bc1 = ac.slice(1, 4);
    CORRADE_COMPARE(bc1.size(), 3);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    ArrayView<int> b2 = a.sliceSize(1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ArrayView<const int> bc2 = ac.sliceSize(1, 3);
    CORRADE_COMPARE(bc2.size(), 3);
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

    ArrayView<int> c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView<const int> cc = ac.prefix(3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    ArrayView<int> d = a.exceptPrefix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ArrayView<const int> dc = ac.exceptPrefix(2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);

    ArrayView<int> e = a.exceptSuffix(2);
    CORRADE_COMPARE(e.size(), 3);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    ArrayView<const int> ec = ac.exceptSuffix(2);
    CORRADE_COMPARE(ec.size(), 3);
    CORRADE_COMPARE(ec[0], 1);
    CORRADE_COMPARE(ec[1], 2);
    CORRADE_COMPARE(ec[2], 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cbc1 = Array5.slice(1, 4);
    CORRADE_COMPARE(cbc1.size(), 3);
    CORRADE_COMPARE(cbc1[0], 2);
    CORRADE_COMPARE(cbc1[1], 3);
    CORRADE_COMPARE(cbc1[2], 4);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cbc2 = Array5.sliceSize(1, 3);
    CORRADE_COMPARE(cbc2.size(), 3);
    CORRADE_COMPARE(cbc2[0], 2);
    CORRADE_COMPARE(cbc2[1], 3);
    CORRADE_COMPARE(cbc2[2], 4);

    constexpr ArrayView<const int> ccc = Array5.prefix(3);
    CORRADE_COMPARE(ccc.size(), 3);
    CORRADE_COMPARE(ccc[0], 1);
    CORRADE_COMPARE(ccc[1], 2);
    CORRADE_COMPARE(ccc[2], 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cdc = Array5.exceptPrefix(2);
    CORRADE_COMPARE(cdc.size(), 3);
    CORRADE_COMPARE(cdc[0], 3);
    CORRADE_COMPARE(cdc[1], 4);
    CORRADE_COMPARE(cdc[2], 5);

    constexpr ArrayView<const int> cec = Array5.exceptSuffix(2);
    CORRADE_COMPARE(cec.size(), 3);
    CORRADE_COMPARE(cec[0], 1);
    CORRADE_COMPARE(cec[1], 2);
    CORRADE_COMPARE(cec[2], 3);
}

void StaticArrayTest::slicePointer() {
    StaticArray<5, int> a{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray<5, int> ac{Corrade::InPlaceInit, 1, 2, 3, 4, 5};

    ArrayView<int> b1 = a.slice(a + 1, a + 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ArrayView<const int> bc1 = ac.slice(ac + 1, ac + 4);
    CORRADE_COMPARE(bc1.size(), 3);
    CORRADE_COMPARE(bc1[0], 2);
    CORRADE_COMPARE(bc1[1], 3);
    CORRADE_COMPARE(bc1[2], 4);

    ArrayView<int> b2 = a.sliceSize(a + 1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ArrayView<const int> bc2 = ac.sliceSize(ac + 1, 3);
    CORRADE_COMPARE(bc2.size(), 3);
    CORRADE_COMPARE(bc2[0], 2);
    CORRADE_COMPARE(bc2[1], 3);
    CORRADE_COMPARE(bc2[2], 4);

    ArrayView<int> c = a.prefix(a + 3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView<const int> cc = ac.prefix(ac + 3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    ArrayView<int> d = a.suffix(a + 2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ArrayView<const int> dc = ac.suffix(ac + 2);
    CORRADE_COMPARE(dc.size(), 3);
    CORRADE_COMPARE(dc[0], 3);
    CORRADE_COMPARE(dc[1], 4);
    CORRADE_COMPARE(dc[2], 5);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cbc1 = Array5.slice(Array5 + 1, Array5 + 4);
    CORRADE_COMPARE(cbc1.size(), 3);
    CORRADE_COMPARE(cbc1[0], 2);
    CORRADE_COMPARE(cbc1[1], 3);
    CORRADE_COMPARE(cbc1[2], 4);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cbc2 = Array5.sliceSize(Array5 + 1, 3);
    CORRADE_COMPARE(cbc2.size(), 3);
    CORRADE_COMPARE(cbc2[0], 2);
    CORRADE_COMPARE(cbc2[1], 3);
    CORRADE_COMPARE(cbc2[2], 4);

    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ == 5
    CORRADE_WARN("prefix() with a calculated pointer isn't constexpr on GCC 5");
    #else
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> ccc = Array5.prefix(Array5 + 3);
    CORRADE_COMPARE(ccc.size(), 3);
    CORRADE_COMPARE(ccc[0], 1);
    CORRADE_COMPARE(ccc[1], 2);
    CORRADE_COMPARE(ccc[2], 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    ArrayView<const int> cdc = Array5.suffix(Array5 + 2);
    CORRADE_COMPARE(cdc.size(), 3);
    CORRADE_COMPARE(cdc[0], 3);
    CORRADE_COMPARE(cdc[1], 4);
    CORRADE_COMPARE(cdc[2], 5);
    #endif
}

void StaticArrayTest::sliceToStatic() {
    StaticArray<5, int> a{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray<5, int> ac{Corrade::InPlaceInit, 1, 2, 3, 4, 5};

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

    StaticArrayView<3, int> d = a.exceptPrefix<2>();
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    StaticArrayView<3, const int> cd = ac.exceptPrefix<2>();
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);

    StaticArrayView<3, int> e = a.exceptSuffix<2>();
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    StaticArrayView<3, const int> ce = ac.exceptSuffix<2>();
    CORRADE_COMPARE(ce[0], 1);
    CORRADE_COMPARE(ce[1], 2);
    CORRADE_COMPARE(ce[2], 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<3, const int> cbc1 = Array5.slice<3>(1);
    CORRADE_COMPARE(cbc1[0], 2);
    CORRADE_COMPARE(cbc1[1], 3);
    CORRADE_COMPARE(cbc1[2], 4);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<3, const int> cbc2 = Array5.slice<1, 4>();
    CORRADE_COMPARE(cbc2[0], 2);
    CORRADE_COMPARE(cbc2[1], 3);
    CORRADE_COMPARE(cbc2[2], 4);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<3, const int> cbc3 = Array5.sliceSize<1, 3>();
    CORRADE_COMPARE(cbc3[0], 2);
    CORRADE_COMPARE(cbc3[1], 3);
    CORRADE_COMPARE(cbc3[2], 4);

    constexpr StaticArrayView<3, const int> ccc = Array5.prefix<3>();
    CORRADE_COMPARE(ccc[0], 1);
    CORRADE_COMPARE(ccc[1], 2);
    CORRADE_COMPARE(ccc[2], 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<3, const int> ccd = Array5.exceptPrefix<2>();
    CORRADE_COMPARE(ccd[0], 3);
    CORRADE_COMPARE(ccd[1], 4);
    CORRADE_COMPARE(ccd[2], 5);

    constexpr StaticArrayView<3, const int> cce = Array5.exceptSuffix<2>();
    CORRADE_COMPARE(cce[0], 1);
    CORRADE_COMPARE(cce[1], 2);
    CORRADE_COMPARE(cce[2], 3);
}

void StaticArrayTest::sliceToStaticPointer() {
    StaticArray<5, int> a{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray<5, int> ac{Corrade::InPlaceInit, 1, 2, 3, 4, 5};

    StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    StaticArrayView<3, const int> bc = ac.slice<3>(ac + 1);
    CORRADE_COMPARE(bc[0], 2);
    CORRADE_COMPARE(bc[1], 3);
    CORRADE_COMPARE(bc[2], 4);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* No idea, and who cares */
    #endif
    StaticArrayView<3, const int> cbc = Array5.slice<3>(Array5 + 1);
    CORRADE_COMPARE(cbc[0], 2);
    CORRADE_COMPARE(cbc[1], 3);
    CORRADE_COMPARE(cbc[2], 4);
}

void StaticArrayTest::sliceZeroNullPointerAmbiguity() {
    StaticArray<5, int> a{Corrade::InPlaceInit, 1, 2, 3, 4, 5};
    const StaticArray<5, int> ac{Corrade::InPlaceInit, 1, 2, 3, 4, 5};

    /* These should all unambigously pick the std::size_t overloads, not the
       T* overloads */

    ArrayView<int> b = a.sliceSize(0, 3);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(b[1], 2);
    CORRADE_COMPARE(b[2], 3);

    ArrayView<const int> bc = ac.sliceSize(0, 3);
    CORRADE_COMPARE(bc.size(), 3);
    CORRADE_COMPARE(bc[0], 1);
    CORRADE_COMPARE(bc[1], 2);
    CORRADE_COMPARE(bc[2], 3);

    ArrayView<int> c = a.prefix(0);
    CORRADE_COMPARE(c.size(), 0);
    CORRADE_COMPARE(c.data(), static_cast<void*>(a.data()));

    ArrayView<const int> cc = ac.prefix(0);
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

void StaticArrayTest::cast() {
    StaticArray<6, std::uint32_t> a;
    const StaticArray<6, std::uint32_t> ca;
    StaticArray<6, const std::uint32_t> ac;
    const StaticArray<6, const std::uint32_t> cac;

    auto b = arrayCast<std::uint64_t>(a);
    auto bc = arrayCast<const std::uint64_t>(ac);
    auto cb = arrayCast<const std::uint64_t>(ca);
    auto cbc = arrayCast<const std::uint64_t>(cac);

    auto d = arrayCast<std::uint16_t>(a);
    auto dc = arrayCast<const std::uint16_t>(ac);
    auto cd = arrayCast<const std::uint16_t>(ca);
    auto cdc = arrayCast<const std::uint16_t>(cac);

    CORRADE_VERIFY(std::is_same<decltype(b), StaticArrayView<3, std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(bc), StaticArrayView<3, const std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cb), StaticArrayView<3, const std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cbc), StaticArrayView<3, const std::uint64_t>>::value);

    CORRADE_VERIFY(std::is_same<decltype(d), StaticArrayView<12,  std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cd), StaticArrayView<12, const std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(dc), StaticArrayView<12, const std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cdc), StaticArrayView<12, const std::uint16_t>>::value);

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
    StaticArray<5, int> a;
    CORRADE_COMPARE(arraySize(a), 5);

    constexpr StaticArray<5, int> ca;
    constexpr std::size_t size = arraySize(ca);
    CORRADE_COMPARE(size, 5);
}

void StaticArrayTest::constructorExplicitInCopyInitialization() {
    /* See constructHelpers.h for details about this compiler-specific issue */
    struct ExplicitDefault {
        explicit ExplicitDefault() {}
    };

    /* This should check the StaticArray internals for
       non-trivially-constructible types. There's no known way to trigger this
       for trivially constructible types (those can't have members with
       explicit constructors), nor for types with a NoInit constructor (which
       then would need an explicit default constructor, not an implicit
       one). */
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!__has_trivial_constructor(ExplicitDefault));
    #else
    CORRADE_VERIFY(!std::is_trivially_constructible<ExplicitDefault>::value);
    #endif

    struct ContainingExplicitDefaultWithImplicitConstructor {
        ExplicitDefault a;
    };

    /* This alone works */
    ContainingExplicitDefaultWithImplicitConstructor a;
    static_cast<void>(a);

    /* So this should too */
    #ifdef CORRADE_BUILD_DEPRECATED
    CORRADE_IGNORE_DEPRECATED_PUSH
    StaticArray<3, ContainingExplicitDefaultWithImplicitConstructor> b{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP
    #endif
    StaticArray<3, ContainingExplicitDefaultWithImplicitConstructor> c{Corrade::ValueInit};
    StaticArray<3, ContainingExplicitDefaultWithImplicitConstructor> d{Corrade::DirectInit};
    #ifdef CORRADE_BUILD_DEPRECATED
    CORRADE_COMPARE(b.size(), 3);
    #endif
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(d.size(), 3);
}

void StaticArrayTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    StaticArray<3, ExtremelyTrivial> a{Corrade::DirectInit, 3, 'a'};
    CORRADE_COMPARE(a.front().a, 3);

    /* This copy-constructs new values */
    StaticArray<3, ExtremelyTrivial> b{a};
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
        /* Clang complains this function is unused. But removing it may have
           unintended consequences, so don't. */
        CORRADE_UNUSED MoveOnlyPointer& operator=(MoveOnlyPointer&&) = default;

        std::nullptr_t a;
    };

    struct MoveOnlyStruct {
        int a;
        char c;
        MoveOnlyPointer b;
    };

    /* This needs special handling on GCC 4.8, where T{Utility::move(b)}
       attempts to convert MoveOnlyStruct to int to initialize the first
       argument and fails miserably. */
    StaticArray<3, MoveOnlyStruct> a{Corrade::DirectInit, 3, 'a', nullptr};
    CORRADE_COMPARE(a.front().a, 3);

    /* This move-constructs new values */
    StaticArray<3, MoveOnlyStruct> b{Utility::move(a)};
    CORRADE_COMPARE(b.front().a, 3);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::StaticArrayTest)
