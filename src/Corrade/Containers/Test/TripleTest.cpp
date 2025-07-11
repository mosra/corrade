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

#include <new>

#include "Corrade/Containers/ArrayView.h" /* used in debugPropagateFlags() */
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/Triple.h"
#include "Corrade/TestSuite/Tester.h"

namespace {

struct FloatIntFlag {
    FloatIntFlag(float a, int b, bool c): a{a}, b{b}, c{c} {}

    float a;
    int b;
    bool c;
};

struct BoolPtrDouble {
    BoolPtrDouble(bool a, int* b, double c): a{a}, b{b}, c{c} {}
    BoolPtrDouble(const BoolPtrDouble&) = delete;
    BoolPtrDouble(BoolPtrDouble&& other): a{other.a}, b{other.b}, c{other.c} {
        other.b = nullptr;
    }
    ~BoolPtrDouble() { if(a) delete b; }
    BoolPtrDouble& operator=(const BoolPtrDouble&) = delete;
    /* Clang complains this function is unused. But removing it may have
       unintended consequences, so don't. */
    CORRADE_UNUSED BoolPtrDouble& operator=(BoolPtrDouble&& other) {
        Corrade::Utility::swap(a, other.a);
        Corrade::Utility::swap(b, other.b);
        Corrade::Utility::swap(c, other.c);
        return *this;
    }

    bool a;
    int* b;
    double c;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct TripleConverter<float, int, bool, FloatIntFlag> {
    static Triple<float, int, bool> from(const FloatIntFlag& other) {
        return {other.a, other.b, other.c};
    }

    static FloatIntFlag to(const Triple<float, int, bool>& other) {
        return {other.first(), other.second(), other.third()};
    }
};

template<> struct DeducedTripleConverter<FloatIntFlag>: TripleConverter<float, int, bool, FloatIntFlag> {};

template<> struct TripleConverter<bool, int*, double, BoolPtrDouble> {
    static Triple<bool, int*, double> from(BoolPtrDouble&& other) {
        Triple<bool, int*, double> ret{other.a, other.b, other.c};
        other.b = nullptr;
        return ret;
    }

    static BoolPtrDouble to(Triple<bool, int*, double>&& other) {
        BoolPtrDouble ret{other.first(), other.second(), other.third()};
        other.second() = nullptr;
        return ret;
    }
};

template<> struct DeducedTripleConverter<BoolPtrDouble>: TripleConverter<bool, int*, double, BoolPtrDouble> {};

}

namespace Test { namespace {

struct TripleTest: TestSuite::Tester {
    explicit TripleTest();

    #ifdef CORRADE_BUILD_DEPRECATED
    void constructDefaultInit();
    #endif
    void constructValueInit();
    void constructNoInit();
    void constructNoInitNoDefaultConstructor();
    void constructCopyCopyCopy();
    void constructCopyCopyCopyMake();
    void constructCopyCopyMove();
    void constructCopyCopyMoveMake();
    void constructCopyMoveCopy();
    void constructCopyMoveCopyMake();
    void constructMoveCopyCopy();
    void constructMoveCopyCopyMake();
    void constructCopyMoveMove();
    void constructCopyMoveMoveMake();
    void constructMoveCopyMove();
    void constructMoveCopyMoveMake();
    void constructMoveMoveCopy();
    void constructMoveMoveCopyMake();
    void constructMoveMoveMove();
    void constructMoveMoveMoveMake();
    void constructDifferentTypeCopy();
    void constructDifferentTypeMove();

    void convertCopy();
    void convertMove();

    void copy();
    void move();

    void resetCounters();

    void compare();
    void access();
    void accessRvalue();
    void accessRvalueLifetimeExtension();

    void debug();
    void debugPropagateFlags();

    void constructorExplicitInCopyInitialization();
    void copyMoveConstructPlainStruct();
};

TripleTest::TripleTest() {
    addTests({
              #ifdef CORRADE_BUILD_DEPRECATED
              &TripleTest::constructDefaultInit,
              #endif
              &TripleTest::constructValueInit},
        &TripleTest::resetCounters, &TripleTest::resetCounters);

    addTests({&TripleTest::constructNoInit,
              &TripleTest::constructNoInitNoDefaultConstructor});

    addTests({&TripleTest::constructCopyCopyCopy,
              &TripleTest::constructCopyCopyCopyMake,
              &TripleTest::constructCopyCopyMove,
              &TripleTest::constructCopyCopyMoveMake,
              &TripleTest::constructCopyMoveCopy,
              &TripleTest::constructCopyMoveCopyMake,
              &TripleTest::constructMoveCopyCopy,
              &TripleTest::constructMoveCopyCopyMake,
              &TripleTest::constructCopyMoveMove,
              &TripleTest::constructCopyMoveMoveMake,
              &TripleTest::constructMoveCopyMove,
              &TripleTest::constructMoveCopyMoveMake,
              &TripleTest::constructMoveMoveCopy,
              &TripleTest::constructMoveMoveCopyMake,
              &TripleTest::constructMoveMoveMove,
              &TripleTest::constructMoveMoveMoveMake,
              &TripleTest::constructDifferentTypeCopy,
              &TripleTest::constructDifferentTypeMove},
        &TripleTest::resetCounters, &TripleTest::resetCounters);

    addTests({&TripleTest::convertCopy,
              &TripleTest::convertMove});

    addTests({&TripleTest::copy,
              &TripleTest::move},
        &TripleTest::resetCounters, &TripleTest::resetCounters);

    addTests({&TripleTest::compare,
              &TripleTest::access,
              &TripleTest::accessRvalue,
              &TripleTest::accessRvalueLifetimeExtension,

              &TripleTest::debug,
              &TripleTest::debugPropagateFlags,

              &TripleTest::constructorExplicitInCopyInitialization,
              &TripleTest::copyMoveConstructPlainStruct});
}

struct Throwable {
    Throwable() {}
    Throwable(const Throwable&) {}
    Throwable(Throwable&&) {}
    Throwable(Corrade::NoInitT) {}
    Throwable& operator=(const Throwable&) { return *this; }
    /* Clang complains this function is unused. But removing it may have
       unintended consequences, so don't. */
    CORRADE_UNUSED Throwable& operator=(Throwable&&) { return *this; }
};

struct Copyable {
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;

    explicit Copyable(int a = 0) noexcept: a{a} { ++constructed; }
    Copyable(const Copyable& other) noexcept: a{other.a} {
        ++constructed;
        ++copied;
    }
    Copyable(Copyable&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }
    Copyable(Corrade::NoInitT) noexcept {}
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

struct Movable {
    static int constructed;
    static int destructed;
    static int moved;

    explicit Movable(int a = 0) noexcept: a{a} { ++constructed; }
    /* To test perfect forwarding in in-place construction. Clang complains
       this function is unused. But removing it may have unintended
       consequences, so don't. */
    CORRADE_UNUSED explicit Movable(int a, int&&) noexcept: Movable{a} {}
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

#ifdef CORRADE_BUILD_DEPRECATED
void TripleTest::constructDefaultInit() {
    {
        CORRADE_IGNORE_DEPRECATED_PUSH
        Triple<float, int, bool> aTrivial{Corrade::DefaultInit};
        CORRADE_IGNORE_DEPRECATED_POP
        /* Trivial types are uninitialized, nothing to verify here. Funnily
           enough, as the constructor is constexpr but the default
           initialization of trivial types itself isn't, the compiler doesn't
           even complain the variable is unused. */

        CORRADE_IGNORE_DEPRECATED_PUSH
        Triple<Copyable, Copyable, Copyable> a{Corrade::DefaultInit};
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_COMPARE(a.first().a, 0);
        CORRADE_COMPARE(a.second().a, 0);
        CORRADE_COMPARE(a.third().a, 0);

        CORRADE_COMPARE(Copyable::constructed, 3);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 3);
    CORRADE_COMPARE(Copyable::destructed, 3);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    /* Can't test constexpr on trivial types because DefaultInit leaves them
       uninitialized */
    struct Foo { int a = 3; };
    CORRADE_IGNORE_DEPRECATED_PUSH
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    /* Can't, because MSVC 2015 forces me to touch the members, which then
       wouldn't be a default initialization. */
    constexpr
    #endif
    Triple<Foo, Foo, Foo> b{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_COMPARE(b.first().a, 3);
    CORRADE_COMPARE(b.second().a, 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Copyable>, Corrade::DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, int, int>, Corrade::DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Throwable, int>, Corrade::DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, int, Throwable>, Corrade::DefaultInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DefaultInitT, Triple<Copyable, Copyable, Copyable>>::value);
}
#endif

void TripleTest::constructValueInit() {
    {
        Triple<float, int, bool> aTrivial1;
        Triple<float, int, bool> aTrivial2{Corrade::ValueInit};
        CORRADE_COMPARE(aTrivial1.first(), 0.0f);
        CORRADE_COMPARE(aTrivial2.first(), 0.0f);
        CORRADE_COMPARE(aTrivial1.second(), 0);
        CORRADE_COMPARE(aTrivial2.second(), 0);
        CORRADE_COMPARE(aTrivial1.third(), false);
        CORRADE_COMPARE(aTrivial2.third(), false);

        Triple<Copyable, Copyable, Copyable> a1;
        Triple<Copyable, Copyable, Copyable> a2{Corrade::ValueInit};
        CORRADE_COMPARE(a1.first().a, 0);
        CORRADE_COMPARE(a2.first().a, 0);
        CORRADE_COMPARE(a1.second().a, 0);
        CORRADE_COMPARE(a2.second().a, 0);
        CORRADE_COMPARE(a1.third().a, 0);
        CORRADE_COMPARE(a2.third().a, 0);

        CORRADE_COMPARE(Copyable::constructed, 6);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 6);
    CORRADE_COMPARE(Copyable::destructed, 6);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr Triple<float, int, bool> b1;
    constexpr Triple<float, int, bool> b2{Corrade::ValueInit};
    CORRADE_COMPARE(b1.first(), 0.0f);
    CORRADE_COMPARE(b2.first(), 0.0f);
    CORRADE_COMPARE(b1.second(), 0);
    CORRADE_COMPARE(b2.second(), 0);
    CORRADE_COMPARE(b1.third(), false);
    CORRADE_COMPARE(b2.third(), false);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Copyable>, Corrade::ValueInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, int, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, int, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, int, int>, Corrade::ValueInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Throwable, int>, Corrade::ValueInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, int, Throwable>, Corrade::ValueInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::ValueInitT, Triple<Copyable, Copyable, Copyable>>::value);
}

void TripleTest::constructNoInit() {
    /* Deliberately not having a default constructor to verify the NoInit one
       is called */
    struct Foo {
        /*implicit*/ Foo(int a): a{a} {}
        explicit Foo(Corrade::NoInitT) {}
        int a;
    };

    /* Testing all eight combinations */
    Triple<float, int, double> a{35.0f, 3, 3.14};
    Triple<int, float, Foo> b{9, 22.0f, 11};
    Triple<float, Foo, int> c{19.0f, 29, 56};
    Triple<Foo, float, int> d{16, 23.0f, -15};
    Triple<float, Foo, Foo> e{13.0f, 76, 41};
    Triple<Foo, float, Foo> f{12, 29.0f, 34};
    Triple<Foo, Foo, float> g{17, 33, 37.0f};
    Triple<Foo, Foo, Foo> h{15, 36, 72};
    new(&a) Triple<float, int, double>{Corrade::NoInit};
    new(&b) Triple<int, float, Foo>{Corrade::NoInit};
    new(&c) Triple<float, Foo, int>{Corrade::NoInit};
    new(&d) Triple<Foo, float, int>{Corrade::NoInit};
    new(&e) Triple<float, Foo, Foo>{Corrade::NoInit};
    new(&f) Triple<Foo, float, Foo>{Corrade::NoInit};
    new(&g) Triple<Foo, Foo, float>{Corrade::NoInit};
    new(&h) Triple<Foo, Foo, Foo>{Corrade::NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a.first(), 35.0f);
        CORRADE_COMPARE(a.second(), 3);
        CORRADE_COMPARE(a.third(), 3.14);

        CORRADE_COMPARE(b.first(), 9);
        CORRADE_COMPARE(b.second(), 22.0f);
        CORRADE_COMPARE(b.third().a, 11);

        CORRADE_COMPARE(c.first(), 19.0f);
        CORRADE_COMPARE(c.second().a, 29);
        CORRADE_COMPARE(c.third(), 56);

        CORRADE_COMPARE(d.first().a, 16);
        CORRADE_COMPARE(d.second(), 23.0f);
        CORRADE_COMPARE(d.third(), -15);

        CORRADE_COMPARE(e.first(), 13.0f);
        CORRADE_COMPARE(e.second().a, 76);
        CORRADE_COMPARE(e.third().a, 41);

        CORRADE_COMPARE(f.first().a, 12);
        CORRADE_COMPARE(f.second(), 29.0f);
        CORRADE_COMPARE(f.third().a, 34);

        CORRADE_COMPARE(g.first().a, 17);
        CORRADE_COMPARE(g.second().a, 33);
        CORRADE_COMPARE(g.third(), 37.0f);

        CORRADE_COMPARE(h.first().a, 15);
        CORRADE_COMPARE(h.second().a, 36);
        CORRADE_COMPARE(h.third().a, 72);
    }

    /* All combinations of trivial and nothrow-constructible types should be
       marked as such */
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<int, int, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<int, int, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<int, Copyable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, int, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<int, Copyable, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, int, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Copyable>, Corrade::NoInitT>::value);

    /* Throwable constructor should be marked as such in all combinations with
       either nothrow or trivial types */
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, int, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, int, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, Copyable>, Corrade::NoInitT>::value);

    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Throwable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Throwable, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, Copyable>, Corrade::NoInitT>::value);

    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, int, Throwable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<int, Copyable, Throwable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, int, Throwable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Copyable, Throwable>, Corrade::NoInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<int, int, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<int, int, Copyable>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<int, Copyable, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<Copyable, int, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<int, Copyable, Copyable>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<Copyable, int, Copyable>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<Copyable, Copyable, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Triple<Copyable, Copyable, Copyable>>::value);
}

/* A variant of these is used in ArrayTest, StaticArrayTest and PairTest */
struct NoDefaultConstructor {
    /* Clang complains this one is unused. Well, yes, it's here to make the
       struct non-default-constructible. */
    CORRADE_UNUSED /*implicit*/ NoDefaultConstructor(int a): a{a} {}
    /*implicit*/ NoDefaultConstructor(Corrade::NoInitT) {}
    int a;
};
template<class T> struct Wrapped {
    /* This works only if T is default-constructible */
    /*implicit*/ Wrapped(): a{} {}
    /*implicit*/ Wrapped(Corrade::NoInitT): a{Corrade::NoInit} {}
    T a;
};

void TripleTest::constructNoInitNoDefaultConstructor() {
    /* In libstdc++ before version 8 std::is_trivially_constructible<T> doesn't
       work with (template) types where the default constructor isn't usable,
       failing compilation instead of producing std::false_type; in version 4.8
       this trait isn't available at all. std::is_trivial is used instead,
       verify that it compiles correctly everywhere. */

    Triple<int, int, Wrapped<NoDefaultConstructor>> a{Corrade::NoInit};
    Triple<int, Wrapped<NoDefaultConstructor>, int> b{Corrade::NoInit};
    Triple<Wrapped<NoDefaultConstructor>, int, int> c{Corrade::NoInit};
    Triple<int, Wrapped<NoDefaultConstructor>, Wrapped<NoDefaultConstructor>> d{Corrade::NoInit};
    Triple<Wrapped<NoDefaultConstructor>, int, Wrapped<NoDefaultConstructor>> e{Corrade::NoInit};
    Triple<Wrapped<NoDefaultConstructor>, Wrapped<NoDefaultConstructor>, int> f{Corrade::NoInit};
    Triple<Wrapped<NoDefaultConstructor>, Wrapped<NoDefaultConstructor>, Wrapped<NoDefaultConstructor>> g{Corrade::NoInit};

    /* No way to test anything here */
    CORRADE_VERIFY(true);
}

void TripleTest::constructCopyCopyCopy() {
    {
        Copyable first{5};
        Copyable second{7};
        Copyable third{9};
        Triple<Copyable, Copyable, Copyable> a = {first, second, third};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 6);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 3);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 6);
    CORRADE_COMPARE(Copyable::destructed, 6);
    CORRADE_COMPARE(Copyable::copied, 3);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    constexpr bool third = true;
    constexpr Triple<float, int, bool> ca = {first, second, third};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);
    CORRADE_COMPARE(ca.third(), true);

    CORRADE_VERIFY(std::is_copy_constructible<Triple<int, int, int>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<Triple<int, int, int>>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Triple<int, int, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Triple<int, int, int>>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Copyable>, const Copyable&, const Copyable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, Copyable>, const Throwable&, const Copyable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, Copyable>, const Copyable&, const Throwable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Copyable, Throwable>, const Copyable&, const Copyable&, const Throwable&>::value);
}

void TripleTest::constructCopyCopyCopyMake() {
    {
        Copyable first{5};
        Copyable second{7};
        Copyable third{9};
        auto a = triple(first, second, third);
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Copyable, Copyable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 6);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 3);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 6);
    CORRADE_COMPARE(Copyable::destructed, 6);
    CORRADE_COMPARE(Copyable::copied, 3);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    constexpr bool third = true;
    constexpr auto ca = triple(first, second, third);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<float, int, bool>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);
    CORRADE_COMPARE(ca.third(), true);
}

void TripleTest::constructCopyCopyMove() {
    {
        Copyable first{5};
        Copyable second{7};
        Triple<Copyable, Copyable, Movable> a = {first, second, Movable{9}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    struct Foo { int a; };
    constexpr Triple<float, int, Foo> ca = {first, second, Foo{9}};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);
    CORRADE_COMPARE(ca.third().a, 9);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Copyable, Movable>, const Copyable&, const Copyable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, Movable>, const Throwable&, const Copyable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, Movable>, const Copyable&, const Throwable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Copyable, Throwable>, const Copyable&, const Copyable&, Throwable&&>::value);
}

void TripleTest::constructCopyCopyMoveMake() {
    {
        Copyable first{5};
        Copyable second{7};
        auto a = triple(first, second, Movable{9});
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Copyable, Copyable, Movable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    struct Foo { int a; };
    constexpr auto ca = triple(first, second, Foo{9});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<float, int, Foo>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);
    CORRADE_COMPARE(ca.third().a, 9);
}

void TripleTest::constructCopyMoveCopy() {
    {
        Copyable first{5};
        Copyable third{9};
        Triple<Copyable, Movable, Copyable> a = {first, Movable{7}, third};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr bool third = true;
    constexpr Triple<float, Foo, bool> ca = {first, Foo{7}, third};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third(), true);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Movable, Copyable>, const Copyable&, Movable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Movable, Copyable>, const Throwable&, Movable&&, const Copyable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, Copyable>, const Copyable&, Throwable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Movable, Throwable>, const Copyable&, Movable&&, const Throwable&>::value);
}

void TripleTest::constructCopyMoveCopyMake() {
    {
        Copyable first{5};
        Copyable third{9};
        auto a = triple(first, Movable{7}, third);
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Copyable, Movable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr bool third = true;
    constexpr auto ca = triple(first, Foo{7}, third);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<float, Foo, bool>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third(), true);
}

void TripleTest::constructMoveCopyCopy() {
    {
        Copyable second{7};
        Copyable third{9};
        Triple<Movable, Copyable, Copyable> a = {Movable{5}, second, third};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr bool third = true;
    constexpr Triple<Foo, float, bool> ca = {Foo{7}, second, third};
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);
    CORRADE_COMPARE(ca.third(), true);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Movable, Copyable, Copyable>, Movable&&, const Copyable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, Copyable>, Throwable&&, const Copyable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Throwable, Copyable>, Movable&&, const Throwable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Copyable, Throwable>, Movable&&, const Copyable&, const Throwable&>::value);
}

void TripleTest::constructMoveCopyCopyMake() {
    {
        Copyable second{7};
        Copyable third{9};
        auto a = triple(Movable{5}, second, third);
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Movable, Copyable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr bool third = true;
    constexpr auto ca = triple(Foo{7}, second, third);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<Foo, float, bool>>::value);
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);
    CORRADE_COMPARE(ca.third(), true);
}

void TripleTest::constructCopyMoveMove() {
    {
        Copyable first{5};
        Triple<Copyable, Movable, Movable> a = {first, Movable{7}, Movable{9}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr Triple<float, Foo, Foo> ca = {first, Foo{7}, Foo{9}};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third().a, 9);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Copyable, Movable, Movable>, const Copyable&, Movable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Movable, Movable>, const Throwable&, Movable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Throwable, Movable>, const Copyable&, Throwable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Copyable, Movable, Throwable>, const Copyable&, Movable&&, Throwable&&>::value);
}

void TripleTest::constructCopyMoveMoveMake() {
    {
        Copyable first{5};
        auto a = triple(first, Movable{7}, Movable{9});
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Copyable, Movable, Movable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr auto ca = triple(first, Foo{7}, Foo{9});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<float, Foo, Foo>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third().a, 9);
}

void TripleTest::constructMoveCopyMove() {
    {
        Copyable second{7};
        Triple<Movable, Copyable, Movable> a = {Movable{5}, second, Movable{9}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr Triple<Foo, float, Foo> ca = {Foo{7}, second, Foo{9}};
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);
    CORRADE_COMPARE(ca.third().a, 9);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Movable, Copyable, Movable>, Movable&&, const Copyable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Copyable, Movable>, Throwable&&, const Copyable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Throwable, Movable>, Movable&&, const Throwable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Copyable, Throwable>, Movable&&, const Copyable&, Throwable&&>::value);
}

void TripleTest::constructMoveCopyMoveMake() {
    {
        Copyable second{7};
        Copyable third{9};
        auto a = triple(Movable{5}, second, third);
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Movable, Copyable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr auto ca = triple(Foo{7}, second, Foo{9});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<Foo, float, Foo>>::value);
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);
    CORRADE_COMPARE(ca.third().a, 9);
}

void TripleTest::constructMoveMoveCopy() {
    {
        Copyable third{9};
        Triple<Movable, Movable, Copyable> a = {Movable{5}, Movable{7}, third};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    struct Foo { int a; };
    constexpr Triple<Foo, Foo, Foo> ca = {Foo{5}, Foo{7}, Foo{9}};
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Movable, Movable, Copyable>, Movable&&, Movable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Movable, Copyable>, Throwable&&, Movable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Throwable, Copyable>, Movable&&, Throwable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Movable, Throwable>, Movable&&, Movable&&, Throwable&&>::value);
}

void TripleTest::constructMoveMoveCopyMake() {
    {
        Copyable third{9};
        auto a = triple(Movable{5}, Movable{7}, third);
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Movable, Movable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    struct Foo { int a; };
    constexpr bool third = true;
    constexpr auto ca = triple(Foo{5}, Foo{7}, third);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<Foo, Foo, bool>>::value);
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third(), true);
}

void TripleTest::constructMoveMoveMove() {
    {
        Triple<Movable, Movable, Movable> a = {Movable{5}, Movable{7}, Movable{9}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::destructed, 3);
        CORRADE_COMPARE(Movable::moved, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 6);
    CORRADE_COMPARE(Movable::destructed, 6);
    CORRADE_COMPARE(Movable::moved, 3);

    struct Foo { int a; };
    constexpr Triple<Foo, Foo, Foo> ca = {Foo{5}, Foo{7}, Foo{9}};
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third().a, 9);

    CORRADE_VERIFY(std::is_nothrow_constructible<Triple<Movable, Movable, Movable>, Movable&&, Movable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Throwable, Movable, Movable>, Throwable&&, Movable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Throwable, Movable>, Movable&&, Throwable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Triple<Movable, Movable, Throwable>, Movable&&, Movable&&, Throwable&&>::value);
}

void TripleTest::constructMoveMoveMoveMake() {
    {
        auto a = triple(Movable{5}, Movable{7}, Movable{9});
        CORRADE_VERIFY(std::is_same<decltype(a), Triple<Movable, Movable, Movable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);
        CORRADE_COMPARE(a.third().a, 9);

        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::destructed, 3);
        CORRADE_COMPARE(Movable::moved, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 6);
    CORRADE_COMPARE(Movable::destructed, 6);
    CORRADE_COMPARE(Movable::moved, 3);

    struct Foo { int a; };
    constexpr auto ca = triple(Foo{5}, Foo{7}, Foo{9});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Triple<Foo, Foo, Foo>>::value);
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);
    CORRADE_COMPARE(ca.third().a, 9);
}

void TripleTest::constructDifferentTypeCopy() {
    Triple<short, char, float> a{-35, 15, 0.5f};
    Triple<long, int, double> b{a};
    CORRADE_COMPARE(b.first(), -35);
    CORRADE_COMPARE(b.second(), 15);
    CORRADE_COMPARE(b.third(), 0.5);

    constexpr Triple<short, char, float> ca{-35, 15, 0.5f};
    constexpr Triple<long, int, double> cb{ca};
    CORRADE_COMPARE(cb.first(), -35);
    CORRADE_COMPARE(cb.second(), 15);
    CORRADE_COMPARE(cb.third(), 0.5);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(std::is_convertible<const Triple<long, int, double>&, Triple<long, int, double>>::value);
    CORRADE_VERIFY(!std::is_convertible<const Triple<short, char, float>&, Triple<long, int, double>>::value);

    /* Only possible to construct with types that can be converted */
    CORRADE_VERIFY(std::is_constructible<Triple<long, int, double>, const Triple<short, char, float>&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, const Triple<short*, char, float>&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, const Triple<short, char*, float>&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, const Triple<short, char, float*>&>::value);
}

void TripleTest::constructDifferentTypeMove() {
    struct MovableDerived: Movable {
        using Movable::Movable;
    };

    {
        Triple<short, float, MovableDerived> a1{-35, 0.5f, MovableDerived{15}};
        Triple<short, MovableDerived, float> a2{-35, MovableDerived{15}, 0.5f};
        Triple<MovableDerived, char, float> a3{MovableDerived{-35}, 15, 0.5f};
        Triple<long, double, Movable> b1{Utility::move(a1)};
        Triple<long, Movable, double> b2{Utility::move(a2)};
        Triple<Movable, char, double> b3{Utility::move(a3)};
        CORRADE_COMPARE(b1.first(), -35);
        CORRADE_COMPARE(b2.first(), -35);
        CORRADE_COMPARE(b3.first().a, -35);
        CORRADE_COMPARE(b1.second(), 0.5);
        CORRADE_COMPARE(b2.second().a, 15);
        CORRADE_COMPARE(b3.second(), 15);
        CORRADE_COMPARE(b1.third().a, 15);
        CORRADE_COMPARE(b2.third(), 0.5);
        CORRADE_COMPARE(b2.third(), 0.5);
    }

    /* Constructed three temporaries, move-constructed them to a1, a2, a3, and
       then move-constructed a1, a2, a3 to b1, b2, b3. Interestingly enough the
       explicit T() for warning suppression doesn't contribute to this. */
    CORRADE_COMPARE(Movable::constructed, 9);
    CORRADE_COMPARE(Movable::destructed, 9);
    CORRADE_COMPARE(Movable::moved, 6);

    struct Foo { int a; };
    struct FooDerived: Foo {
        constexpr explicit FooDerived(int a): Foo{a} {}
    };
    constexpr Triple<long, double, Foo> cb1{Triple<short, float, FooDerived>{-35, 0.5f, FooDerived{15}}};
    constexpr Triple<long, Foo, double> cb2{Triple<short, FooDerived, float>{-35, FooDerived{15}, 0.5f}};
    constexpr Triple<Foo, char, double> cb3{Triple<FooDerived, char, float>{FooDerived{-35}, 15, 0.5f}};
    CORRADE_COMPARE(cb1.first(), -35);
    CORRADE_COMPARE(cb2.first(), -35);
    CORRADE_COMPARE(cb3.first().a, -35);
    CORRADE_COMPARE(cb1.second(), 0.5);
    CORRADE_COMPARE(cb2.second().a, 15);
    CORRADE_COMPARE(cb3.second(), 15);
    CORRADE_COMPARE(cb1.third().a, 15);
    CORRADE_COMPARE(cb2.third(), 0.5);
    CORRADE_COMPARE(cb3.third(), 0.5);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(std::is_convertible<Triple<long, int, double>&&, Triple<long, int, double>>::value);
    CORRADE_VERIFY(!std::is_convertible<Triple<short, char, float>&&, Triple<long, int, double>>::value);

    /* Only possible to construct with types that can be converted */
    CORRADE_VERIFY(std::is_constructible<Triple<long, int, double>, Triple<short, char, float>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, Triple<short*, char, float>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, Triple<short, char*, float>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<long, int, double>, Triple<short, char, float*>&&>::value);
}

void TripleTest::convertCopy() {
    FloatIntFlag a{35.0f, 7, true};

    Triple<float, int, bool> b = a;
    CORRADE_COMPARE(b.first(), 35.0f);
    CORRADE_COMPARE(b.second(), 7);
    CORRADE_COMPARE(b.third(), true);

    FloatIntFlag c = b;
    CORRADE_COMPARE(c.a, 35.0f);
    CORRADE_COMPARE(c.b, 7);
    CORRADE_COMPARE(c.c, true);

    auto d = triple(FloatIntFlag{35.0f, 7, true});
    CORRADE_VERIFY(std::is_same<decltype(d), Triple<float, int, bool>>::value);
    CORRADE_COMPARE(d.first(), 35.0f);
    CORRADE_COMPARE(d.second(), 7);
    CORRADE_COMPARE(d.third(), true);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY(std::is_constructible<Triple<float, int, bool>, FloatIntFlag>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<int, float, bool>, FloatIntFlag>::value);
    CORRADE_VERIFY(std::is_constructible<FloatIntFlag, Triple<float, int, bool>>::value);
    CORRADE_VERIFY(!std::is_constructible<FloatIntFlag, Triple<int, float, bool>>::value);
}

void TripleTest::convertMove() {
    BoolPtrDouble a{true, new int{7}, 1.5};
    CORRADE_COMPARE(*a.b, 7);

    Triple<bool, int*, double> b = Utility::move(a);
    CORRADE_COMPARE(b.first(), true);
    CORRADE_COMPARE(*b.second(), 7);
    CORRADE_COMPARE(b.third(), 1.5);

    BoolPtrDouble c = Utility::move(b);
    CORRADE_COMPARE(c.a, true);
    CORRADE_COMPARE(*c.b, 7);
    CORRADE_COMPARE(c.c, 1.5);

    int dv = 35;
    auto d = triple(BoolPtrDouble{false, &dv, 0.5});
    CORRADE_VERIFY(std::is_same<decltype(d), Triple<bool, int*, double>>::value);
    CORRADE_COMPARE(d.first(), false);
    CORRADE_COMPARE(*d.second(), 35);
    CORRADE_COMPARE(d.third(), 0.5);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY(std::is_constructible<Triple<bool, int*, double>, BoolPtrDouble&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<bool, float*, double>, BoolPtrDouble&&>::value);
    CORRADE_VERIFY(std::is_constructible<BoolPtrDouble, Triple<bool, int*, double>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtrDouble, Triple<bool, float*, double>&&>::value);

    /* Copy construction is not allowed */
    CORRADE_VERIFY(!std::is_constructible<Triple<bool, int*, double>, BoolPtrDouble&>::value);
    CORRADE_VERIFY(!std::is_constructible<Triple<bool, int*, double>, const BoolPtrDouble&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtrDouble, Triple<bool, int*, double>&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtrDouble, const Triple<bool, int*, double>&>::value);
}

void TripleTest::copy() {
    {
        Triple<Copyable, int, float> a{Copyable{5}, 3, 1.5f};

        Triple<Copyable, int, float> b = a;
        CORRADE_COMPARE(b.first().a, 5);
        CORRADE_COMPARE(b.second(), 3);
        CORRADE_COMPARE(b.third(), 1.5f);

        Triple<Copyable, int, float> c;
        c = a;
        CORRADE_COMPARE(c.first().a, 5);
        CORRADE_COMPARE(c.second(), 3);
        CORRADE_COMPARE(c.third(), 1.5f);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 1);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 1);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Triple<Copyable, int, float>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Triple<Copyable, int, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Triple<Throwable, int, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Triple<Throwable, int, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Triple<int, Throwable, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Triple<int, Throwable, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Triple<int, float, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Triple<int, float, Throwable>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Triple<float, int, bool>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Triple<float, int, bool>>::value);
    CORRADE_VERIFY(std::is_trivially_copyable<Triple<float, int, bool>>::value);
    /* Comparison with std::tuple is in TripleStlTest::triviallyCopyable() */
    #endif
}

void TripleTest::move() {
    {
        Triple<float, Movable, int> a{1.5f, Movable{5}, 3};

        Triple<float, Movable, int> b = Utility::move(a);
        CORRADE_COMPARE(b.first(), 1.5f);
        CORRADE_COMPARE(b.second().a, 5);
        CORRADE_COMPARE(b.third(), 3);

        Triple<float, Movable, int> c;
        c = Utility::move(a);
        CORRADE_COMPARE(c.first(), 1.5f);
        CORRADE_COMPARE(c.second().a, 5);
        CORRADE_COMPARE(c.third(), 3);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 3);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Triple<float, Movable, int>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Triple<float, Movable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Triple<Throwable, int, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Triple<Throwable, int, float>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Triple<float, Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Triple<float, Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Triple<float, int, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Triple<float, int, Throwable>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_move_constructible<Triple<float, int, bool>>::value);
    CORRADE_VERIFY(std::is_trivially_move_assignable<Triple<float, int, bool>>::value);
    /* Comparison with std::tuple is in TripleStlTest::triviallyMovable() */
    #endif
}

void TripleTest::resetCounters() {
    Copyable::constructed = Copyable::destructed = Copyable::copied = Copyable::moved =
        Movable::constructed = Movable::destructed = Movable::moved = 0;
}

void TripleTest::compare() {
    Triple<float, int, bool> a{35.0f, 4, true};
    Triple<float, int, bool> b{35.0f, 4, true};
    Triple<float, int, bool> c{35.1f, 4, true};
    Triple<float, int, bool> d{35.0f, 5, true};
    Triple<float, int, bool> e{35.0f, 4, false};

    CORRADE_VERIFY(a == a);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);
    CORRADE_VERIFY(a != d);
    CORRADE_VERIFY(d != a);
    CORRADE_VERIFY(a != e);
    CORRADE_VERIFY(e != a);
}

void TripleTest::access() {
    Triple<float, int, bool> a{35.0f, 4, true};
    CORRADE_COMPARE(a.first(), 35.0f);
    CORRADE_COMPARE(a.second(), 4);
    CORRADE_COMPARE(a.third(), true);

    a.first() = 72.0f;
    a.second() = 5;
    a.third() = false;
    CORRADE_COMPARE(a.first(), 72.0f);
    CORRADE_COMPARE(a.second(), 5);
    CORRADE_COMPARE(a.third(), false);

    constexpr Triple<float, int, bool> ca{35.0f, 4, true};
    constexpr float first = ca.first();
    constexpr int second = ca.second();
    constexpr bool third = ca.third();
    CORRADE_COMPARE(first, 35.0f);
    CORRADE_COMPARE(second, 4);
    CORRADE_COMPARE(third, true);
}

void TripleTest::accessRvalue() {
    Movable b1 = Triple<Movable, int, int>{Movable{5}, 3, 7}.first();
    Movable b2 = Triple<int, Movable, int>{5, Movable{3}, 7}.second();
    Movable b3 = Triple<int, int, Movable>{5, 3, Movable{7}}.third();
    CORRADE_COMPARE(b1.a, 5);
    CORRADE_COMPARE(b2.a, 3);
    CORRADE_COMPARE(b3.a, 7);
}

void TripleTest::accessRvalueLifetimeExtension() {
    struct DiesLoudly {
        DiesLoudly() = default;
        DiesLoudly(const DiesLoudly&) = delete;
        DiesLoudly(DiesLoudly&& other): orphaned{true} {
            other.orphaned = false;
        }

        ~DiesLoudly() {
            if(orphaned) Debug{} << "dying!";
        }

        bool orphaned = true;
    };

    Containers::String out;
    Debug redirectOutput{&out};
    {
        /* Here a reference lifetime extension should kick in, causing the
           output of first(), second() and third() to be destroyed only at the
           end of scope, and not already at the ;. A more common case of this
           would be with temporary expressions in a range-for loop, see
           OptionalTest::accessRvalueLifetimeExtension() for an example. */
        auto&& first = Triple<DiesLoudly, int, int>{DiesLoudly{}, 0, 0}.first();
        auto&& second = Triple<int, DiesLoudly, int>{0, DiesLoudly{}, 0}.second();
        auto&& third = Triple<int, int, DiesLoudly>{0, 0, DiesLoudly{}}.third();
        Debug{} << "shouldn't be dead yet";

        /* So the compiler doesn't complain about the variables being unused
           (even though it's a load-bearing reference) */
        CORRADE_VERIFY(&first);
        CORRADE_VERIFY(&second);
        CORRADE_VERIFY(&third);
    }
    CORRADE_COMPARE(out,
        "shouldn't be dead yet\n"
        "dying!\n"
        "dying!\n"
        "dying!\n");
}

void TripleTest::debug() {
    Containers::String out;
    Debug{&out} << triple(42.5f, 3, true);
    CORRADE_COMPARE(out, "{42.5, 3, true}\n");
}

void TripleTest::debugPropagateFlags() {
    Containers::String out;
    /* The modifier shouldn't become persistent for values after. The nospace
       modifier shouldn't get propagated. */
    Debug{&out} << ">" << Debug::nospace << Debug::packed << triple(Containers::arrayView({3, 4, 5}), false, Containers::arrayView({"A", "B"})) << Containers::arrayView({"a", "b", "c"});
    CORRADE_COMPARE(out, ">{345, false, AB} {a, b, c}\n");
}

void TripleTest::constructorExplicitInCopyInitialization() {
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
    Triple<ContainingExplicitDefaultWithImplicitConstructor, ContainingExplicitDefaultWithImplicitConstructor, ContainingExplicitDefaultWithImplicitConstructor> b;
    CORRADE_VERIFY(&b.first() != &b.second());
}

void TripleTest::copyMoveConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };
    struct DerivedExtremelyTrivial: ExtremelyTrivial {
        explicit DerivedExtremelyTrivial(int a, char b): ExtremelyTrivial{a, b} {}
    };

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

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably, and similarly with MoveOnlyStruct. */
    const ExtremelyTrivial value{3, 'a'};
    Triple<ExtremelyTrivial, ExtremelyTrivial, ExtremelyTrivial> aCCC{value, value, value};
    Triple<ExtremelyTrivial, ExtremelyTrivial, MoveOnlyStruct> aCCM{value, value, MoveOnlyStruct{3, 'a', nullptr}};
    Triple<ExtremelyTrivial, MoveOnlyStruct, ExtremelyTrivial> aCMC{value, MoveOnlyStruct{3, 'a', nullptr}, value};
    Triple<MoveOnlyStruct, ExtremelyTrivial, ExtremelyTrivial> aMCC{MoveOnlyStruct{3, 'a', nullptr}, value, value};
    Triple<ExtremelyTrivial, MoveOnlyStruct, MoveOnlyStruct> aCMM{value, MoveOnlyStruct{3, 'a', nullptr}, MoveOnlyStruct{3, 'a', nullptr}};
    Triple<MoveOnlyStruct, ExtremelyTrivial, MoveOnlyStruct> aMCM{MoveOnlyStruct{3, 'a', nullptr}, value, MoveOnlyStruct{3, 'a', nullptr}};
    Triple<MoveOnlyStruct, MoveOnlyStruct, ExtremelyTrivial> aMMC{MoveOnlyStruct{3, 'a', nullptr}, MoveOnlyStruct{3, 'a', nullptr}, value};
    Triple<MoveOnlyStruct, MoveOnlyStruct, MoveOnlyStruct> aMMM{MoveOnlyStruct{3, 'a', nullptr}, MoveOnlyStruct{3, 'a', nullptr}, MoveOnlyStruct{3, 'a', nullptr}};
    CORRADE_COMPARE(aCCC.second().a, 3);
    CORRADE_COMPARE(aCCM.second().a, 3);
    CORRADE_COMPARE(aCMC.second().a, 3);
    CORRADE_COMPARE(aMCC.second().a, 3);
    CORRADE_COMPARE(aCMM.second().a, 3);
    CORRADE_COMPARE(aMCM.second().a, 3);
    CORRADE_COMPARE(aMMC.second().a, 3);
    CORRADE_COMPARE(aMMM.second().a, 3);

    /* This copy/move-constructs the wrapped value. No special handling needed
       in the implementation as the constructor is implicit. */
    Triple<ExtremelyTrivial, ExtremelyTrivial, ExtremelyTrivial> bCCC = aCCC;
    Triple<MoveOnlyStruct, MoveOnlyStruct, MoveOnlyStruct> bMMM = Utility::move(aMMM);
    CORRADE_COMPARE(bCCC.second().a, 3);
    CORRADE_COMPARE(bMMM.second().a, 3);

    /* This copy/move-assigns constructs the wrapped value. No special handling
       needed in the implementation as the assignment is implicit. */
    Triple<ExtremelyTrivial, ExtremelyTrivial, ExtremelyTrivial> cCCC;
    Triple<MoveOnlyStruct, MoveOnlyStruct, MoveOnlyStruct> cMMM{MoveOnlyStruct{6, 'b', nullptr}, MoveOnlyStruct{6, 'b', nullptr}, MoveOnlyStruct{6, 'b', nullptr}};
    cCCC = bCCC;
    cMMM = Utility::move(bMMM);
    CORRADE_COMPARE(cCCC.second().a, 3);
    CORRADE_COMPARE(cMMM.second().a, 3);

    /* Same as the initial case but with the copy conversion constructor
       instead. The move can't be tested as attempting to move the struct hits
       another GCC 4.8 bug where it attempts to use a copy constructor for
       some reason. So let's just hope the move conversion constructor won't
       regress. */
    Triple<DerivedExtremelyTrivial, DerivedExtremelyTrivial, DerivedExtremelyTrivial> dCopy{DerivedExtremelyTrivial{3, 'a'}, DerivedExtremelyTrivial{4, 'b'}, DerivedExtremelyTrivial{5, 'c'}};
    Triple<ExtremelyTrivial, ExtremelyTrivial, ExtremelyTrivial> eCopy{dCopy};
    CORRADE_COMPARE(eCopy.second().a, 4);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::TripleTest)
