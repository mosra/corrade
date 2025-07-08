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
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

namespace {

struct FloatInt {
    FloatInt(float a, int b): a{a}, b{b} {}

    float a;
    int b;
};

struct BoolPtr {
    BoolPtr(bool a, int* b): a{a}, b{b} {}
    BoolPtr(const BoolPtr&) = delete;
    BoolPtr(BoolPtr&& other): a{other.a}, b{other.b} {
        other.b = nullptr;
    }
    ~BoolPtr() { if(a) delete b; }
    BoolPtr& operator=(const BoolPtr&) = delete;
    /* Clang complains this function is unused. But removing it may have
       unintended consequences, so don't. */
    CORRADE_UNUSED BoolPtr& operator=(BoolPtr&& other) {
        Corrade::Utility::swap(a, other.a);
        Corrade::Utility::swap(b, other.b);
        return *this;
    }

    bool a;
    int* b;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct PairConverter<float, int, FloatInt> {
    static Pair<float, int> from(const FloatInt& other) {
        return {other.a, other.b};
    }

    static FloatInt to(const Pair<float, int>& other) {
        return {other.first(), other.second()};
    }
};

template<> struct DeducedPairConverter<FloatInt>: PairConverter<float, int, FloatInt> {};

template<> struct PairConverter<bool, int*, BoolPtr> {
    static Pair<bool, int*> from(BoolPtr&& other) {
        Pair<bool, int*> ret{other.a, other.b};
        other.b = nullptr;
        return ret;
    }

    static BoolPtr to(Pair<bool, int*>&& other) {
        BoolPtr ret{other.first(), other.second()};
        other.second() = nullptr;
        return ret;
    }
};

template<> struct DeducedPairConverter<BoolPtr>: PairConverter<bool, int*, BoolPtr> {};

}

namespace Test { namespace {

struct PairTest: TestSuite::Tester {
    explicit PairTest();

    #ifdef CORRADE_BUILD_DEPRECATED
    void constructDefaultInit();
    #endif
    void constructValueInit();
    void constructNoInit();
    void constructNoInitNoDefaultConstructor();
    void constructCopyCopy();
    void constructCopyCopyMake();
    void constructCopyMove();
    void constructCopyMoveMake();
    void constructMoveCopy();
    void constructMoveCopyMake();
    void constructMoveMove();
    void constructMoveMoveMake();
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

PairTest::PairTest() {
    addTests({
              #ifdef CORRADE_BUILD_DEPRECATED
              &PairTest::constructDefaultInit,
              #endif
              &PairTest::constructValueInit},
        &PairTest::resetCounters, &PairTest::resetCounters);

    addTests({&PairTest::constructNoInit,
              &PairTest::constructNoInitNoDefaultConstructor});

    addTests({&PairTest::constructCopyCopy,
              &PairTest::constructCopyCopyMake,
              &PairTest::constructCopyMove,
              &PairTest::constructCopyMoveMake,
              &PairTest::constructMoveCopy,
              &PairTest::constructMoveCopyMake,
              &PairTest::constructMoveMove,
              &PairTest::constructMoveMoveMake,
              &PairTest::constructDifferentTypeCopy,
              &PairTest::constructDifferentTypeMove},
        &PairTest::resetCounters, &PairTest::resetCounters);

    addTests({&PairTest::convertCopy,
              &PairTest::convertMove});

    addTests({&PairTest::copy,
              &PairTest::move},
        &PairTest::resetCounters, &PairTest::resetCounters);

    addTests({&PairTest::compare,
              &PairTest::access,
              &PairTest::accessRvalue,
              &PairTest::accessRvalueLifetimeExtension,

              &PairTest::debug,
              &PairTest::debugPropagateFlags,

              &PairTest::constructorExplicitInCopyInitialization,
              &PairTest::copyMoveConstructPlainStruct});
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
void PairTest::constructDefaultInit() {
    {
        CORRADE_IGNORE_DEPRECATED_PUSH
        Pair<float, int> aTrivial{Corrade::DefaultInit};
        CORRADE_IGNORE_DEPRECATED_POP
        /* Trivial types are uninitialized, nothing to verify here. Funnily
           enough, as the constructor is constexpr but the default
           initialization of trivial types itself isn't, the compiler doesn't
           even complain the variable is unused. */

        CORRADE_IGNORE_DEPRECATED_PUSH
        Pair<Copyable, Copyable> a{Corrade::DefaultInit};
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_COMPARE(a.first().a, 0);
        CORRADE_COMPARE(a.second().a, 0);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
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
    Pair<Foo, Foo> b{Corrade::DefaultInit};
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_COMPARE(b.first().a, 3);
    CORRADE_COMPARE(b.second().a, 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Copyable>, Corrade::DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, int>, Corrade::DefaultInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<int, Throwable>, Corrade::DefaultInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::DefaultInitT, Pair<Copyable, Copyable>>::value);
}
#endif

void PairTest::constructValueInit() {
    {
        Pair<float, int> aTrivial1;
        Pair<float, int> aTrivial2{Corrade::ValueInit};
        CORRADE_COMPARE(aTrivial1.first(), 0.0f);
        CORRADE_COMPARE(aTrivial2.first(), 0.0f);
        CORRADE_COMPARE(aTrivial1.second(), 0);
        CORRADE_COMPARE(aTrivial2.second(), 0);

        Pair<Copyable, Copyable> a1;
        Pair<Copyable, Copyable> a2{Corrade::ValueInit};
        CORRADE_COMPARE(a1.first().a, 0);
        CORRADE_COMPARE(a2.first().a, 0);
        CORRADE_COMPARE(a1.second().a, 0);
        CORRADE_COMPARE(a2.second().a, 0);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 0);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr Pair<float, int> b1;
    constexpr Pair<float, int> b2{Corrade::ValueInit};
    CORRADE_COMPARE(b1.first(), 0.0f);
    CORRADE_COMPARE(b2.first(), 0.0f);
    CORRADE_COMPARE(b1.second(), 0);
    CORRADE_COMPARE(b2.second(), 0);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Copyable>, Corrade::ValueInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<int, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, int>, Corrade::ValueInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<int, Throwable>, Corrade::ValueInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::ValueInitT, Pair<Copyable, Copyable>>::value);
}

void PairTest::constructNoInit() {
    /* Deliberately not having a default constructor to verify the NoInit one
       is called */
    struct Foo {
        /*implicit*/ Foo(int a): a{a} {}
        explicit Foo(Corrade::NoInitT) {}
        int a;
    };

    /* Testing all four combinations */
    Pair<float, int> a{35.0f, 3};
    Pair<float, Foo> b{39.0f, 7};
    Pair<Foo, float> c{17, 37.0f};
    Pair<Foo, Foo> d{15, 36};
    new(&a) Pair<float, int>{Corrade::NoInit};
    new(&b) Pair<float, Foo>{Corrade::NoInit};
    new(&c) Pair<Foo, float>{Corrade::NoInit};
    new(&d) Pair<Foo, Foo>{Corrade::NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a.first(), 35.0f);
        CORRADE_COMPARE(a.second(), 3);

        CORRADE_COMPARE(b.first(), 39.0f);
        CORRADE_COMPARE(b.second().a, 7);

        CORRADE_COMPARE(c.first().a, 17);
        CORRADE_COMPARE(c.second(), 37.0f);

        CORRADE_COMPARE(d.first().a, 15);
        CORRADE_COMPARE(d.second().a, 36);
    }

    /* All combinations of trivial and nothrow-constructible types should be
       marked as such */
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<int, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<int, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, int>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Copyable>, Corrade::NoInitT>::value);

    /* Throwable constructor should be marked as such in all combinations with
       either nothrow or trivial types */
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, Copyable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, int>, Corrade::NoInitT>::value);

    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Copyable, Throwable>, Corrade::NoInitT>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<int, Throwable>, Corrade::NoInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Pair<int, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Pair<int, Copyable>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Pair<Copyable, int>>::value);
    CORRADE_VERIFY(!std::is_convertible<Corrade::NoInitT, Pair<Copyable, Copyable>>::value);
}

/* A variant of these is used in ArrayTest, StaticArrayTest and TripleTest */
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

void PairTest::constructNoInitNoDefaultConstructor() {
    /* In libstdc++ before version 8 std::is_trivially_constructible<T> doesn't
       work with (template) types where the default constructor isn't usable,
       failing compilation instead of producing std::false_type; in version 4.8
       this trait isn't available at all. std::is_trivial is used instead,
       verify that it compiles correctly everywhere. */

    Pair<int, Wrapped<NoDefaultConstructor>> a{Corrade::NoInit};
    Pair<Wrapped<NoDefaultConstructor>, int> b{Corrade::NoInit};
    Pair<Wrapped<NoDefaultConstructor>, Wrapped<NoDefaultConstructor>> c{Corrade::NoInit};

    /* No way to test anything here */
    CORRADE_VERIFY(true);
}

void PairTest::constructCopyCopy() {
    {
        Copyable first{5};
        Copyable second{7};
        Pair<Copyable, Copyable> a = {first, second};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    constexpr Pair<float, int> ca = {first, second};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);

    CORRADE_VERIFY(std::is_copy_constructible<Pair<int, int>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<Pair<int, int>>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Pair<int, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Pair<int, int>>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Copyable>, const Copyable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, Copyable>, const Throwable&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Copyable, Throwable>, const Copyable&, const Throwable&>::value);
}

void PairTest::constructCopyCopyMake() {
    {
        Copyable first{5};
        Copyable second{7};
        auto a = pair(first, second);
        CORRADE_VERIFY(std::is_same<decltype(a), Pair<Copyable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 0);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 0);

    constexpr float first = 35.0f;
    constexpr int second = 7;
    constexpr auto ca = pair(first, second);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Pair<float, int>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second(), 7);
}

void PairTest::constructCopyMove() {
    {
        Copyable first{5};
        Pair<Copyable, Movable> a = {first, Movable{7}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr Pair<float, Foo> ca = {first, Foo{7}};
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Copyable, Movable>, const Copyable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, Movable>, const Throwable&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Copyable, Throwable>, const Copyable&, Throwable&&>::value);
}

void PairTest::constructCopyMoveMake() {
    {
        Copyable first{5};
        auto a = pair(first, Movable{7});
        CORRADE_VERIFY(std::is_same<decltype(a), Pair<Copyable, Movable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float first = 35.0f;
    struct Foo { int a; };
    constexpr auto ca = pair(first, Foo{7});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Pair<float, Foo>>::value);
    CORRADE_COMPARE(ca.first(), 35.0f);
    CORRADE_COMPARE(ca.second().a, 7);
}

void PairTest::constructMoveCopy() {
    {
        Copyable second{7};
        Pair<Movable, Copyable> a = {Movable{5}, second};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr Pair<Foo, float> ca = {Foo{7}, second};
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Movable, Copyable>, Movable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, Copyable>, Throwable&&, const Copyable&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Movable, Throwable>, Movable&&, const Throwable&>::value);
}

void PairTest::constructMoveCopyMake() {
    {
        Copyable second{7};
        auto a = pair(Movable{5}, second);
        CORRADE_VERIFY(std::is_same<decltype(a), Pair<Movable, Copyable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Copyable::constructed, 2);
        CORRADE_COMPARE(Copyable::destructed, 0);
        CORRADE_COMPARE(Copyable::copied, 1);
        CORRADE_COMPARE(Copyable::moved, 0);

        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    constexpr float second = 35.0f;
    struct Foo { int a; };
    constexpr auto ca = pair(Foo{7}, second);
    CORRADE_VERIFY(std::is_same<decltype(ca), const Pair<Foo, float>>::value);
    CORRADE_COMPARE(ca.first().a, 7);
    CORRADE_COMPARE(ca.second(), 35.0f);
}

void PairTest::constructMoveMove() {
    {
        Pair<Movable, Movable> a = {Movable{5}, Movable{7}};
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    struct Foo { int a; };
    constexpr Pair<Foo, Foo> ca = {Foo{5}, Foo{7}};
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pair<Movable, Movable>, Movable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Throwable, Movable>, Throwable&&, Movable&&>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pair<Movable, Throwable>, Movable&&, Throwable&&>::value);
}

void PairTest::constructMoveMoveMake() {
    {
        auto a = pair(Movable{5}, Movable{7});
        CORRADE_VERIFY(std::is_same<decltype(a), Pair<Movable, Movable>>::value);
        CORRADE_COMPARE(a.first().a, 5);
        CORRADE_COMPARE(a.second().a, 7);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 2);
        CORRADE_COMPARE(Movable::moved, 2);
    }

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 2);

    struct Foo { int a; };
    constexpr auto ca = pair(Foo{5}, Foo{7});
    CORRADE_VERIFY(std::is_same<decltype(ca), const Pair<Foo, Foo>>::value);
    CORRADE_COMPARE(ca.first().a, 5);
    CORRADE_COMPARE(ca.second().a, 7);
}

void PairTest::constructDifferentTypeCopy() {
    Pair<short, float> a{-35, 0.5f};
    Pair<long, double> b{a};
    CORRADE_COMPARE(b.first(), -35);
    CORRADE_COMPARE(b.second(), 0.5);

    constexpr Pair<short, float> ca{-35, 0.5f};
    constexpr Pair<long, double> cb{ca};
    CORRADE_COMPARE(cb.first(), -35);
    CORRADE_COMPARE(cb.second(), 0.5);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(std::is_convertible<const Pair<long, double>&, Pair<long, double>>::value);
    CORRADE_VERIFY(!std::is_convertible<const Pair<short, float>&, Pair<long, double>>::value);

    /* Only possible to construct with types that can be converted */
    CORRADE_VERIFY(std::is_constructible<Pair<long, double>, const Pair<short, float>&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<long, double>, const Pair<short*, float>&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<long, double>, const Pair<short, float*>&>::value);
}

void PairTest::constructDifferentTypeMove() {
    struct MovableDerived: Movable {
        using Movable::Movable;
    };

    {
        Pair<short, MovableDerived> a1{-35, MovableDerived{15}};
        Pair<MovableDerived, float> a2{MovableDerived{-35}, 0.5f};
        Pair<long, Movable> b1{Utility::move(a1)};
        Pair<Movable, double> b2{Utility::move(a2)};
        CORRADE_COMPARE(b1.first(), -35);
        CORRADE_COMPARE(b2.first().a, -35);
        CORRADE_COMPARE(b1.second().a, 15);
        CORRADE_COMPARE(b2.second(), 0.5);
    }

    /* Constructed two temporaries, move-constructed them to a1, a2, and then
       move-constructed a1, a2 to b1, b2. Interestingly enough the explicit T()
       for warning suppression doesn't contribute to this. */
    CORRADE_COMPARE(Movable::constructed, 6);
    CORRADE_COMPARE(Movable::destructed, 6);
    CORRADE_COMPARE(Movable::moved, 4);

    struct Foo { int a; };
    struct FooDerived: Foo {
        constexpr explicit FooDerived(int a): Foo{a} {}
    };
    constexpr Pair<long, Foo> cb1{Pair<short, FooDerived>{-35, FooDerived{15}}};
    constexpr Pair<Foo, double> cb2{Pair<FooDerived, float>{FooDerived{-35}, 0.5f}};
    CORRADE_COMPARE(cb1.first(), -35);
    CORRADE_COMPARE(cb2.first().a, -35);
    CORRADE_COMPARE(cb1.second().a, 15);
    CORRADE_COMPARE(cb2.second(), 0.5);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(std::is_convertible<Pair<long, double>&&, Pair<long, double>>::value);
    CORRADE_VERIFY(!std::is_convertible<Pair<short, float>&&, Pair<long, double>>::value);

    /* Only possible to construct with types that can be converted */
    CORRADE_VERIFY(std::is_constructible<Pair<long, double>, Pair<short, float>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<long, double>, Pair<short*, float>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<long, double>, Pair<short, float*>&&>::value);
}

void PairTest::convertCopy() {
    FloatInt a{35.0f, 7};

    Pair<float, int> b = a;
    CORRADE_COMPARE(b.first(), 35.0f);
    CORRADE_COMPARE(b.second(), 7);

    FloatInt c = b;
    CORRADE_COMPARE(c.a, 35.0f);
    CORRADE_COMPARE(c.b, 7);

    auto d = pair(FloatInt{35.0f, 7});
    CORRADE_VERIFY(std::is_same<decltype(d), Pair<float, int>>::value);
    CORRADE_COMPARE(d.first(), 35.0f);
    CORRADE_COMPARE(d.second(), 7);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY(std::is_constructible<Pair<float, int>, FloatInt>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<int, float>, FloatInt>::value);
    CORRADE_VERIFY(std::is_constructible<FloatInt, Pair<float, int>>::value);
    CORRADE_VERIFY(!std::is_constructible<FloatInt, Pair<int, float>>::value);
}

void PairTest::convertMove() {
    BoolPtr a{true, new int{7}};
    CORRADE_COMPARE(*a.b, 7);

    Pair<bool, int*> b = Utility::move(a);
    CORRADE_COMPARE(b.first(), true);
    CORRADE_COMPARE(*b.second(), 7);

    BoolPtr c = Utility::move(b);
    CORRADE_COMPARE(c.a, true);
    CORRADE_COMPARE(*c.b, 7);

    int dv = 35;
    auto d = pair(BoolPtr{false, &dv});
    CORRADE_VERIFY(std::is_same<decltype(d), Pair<bool, int*>>::value);
    CORRADE_COMPARE(d.first(), false);
    CORRADE_COMPARE(*d.second(), 35);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY(std::is_constructible<Pair<bool, int*>, BoolPtr&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<bool, float*>, BoolPtr&&>::value);
    CORRADE_VERIFY(std::is_constructible<BoolPtr, Pair<bool, int*>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtr, Pair<bool, float*>&&>::value);

    /* Copy construction is not allowed */
    CORRADE_VERIFY(!std::is_constructible<Pair<bool, int*>, BoolPtr&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pair<bool, int*>, const BoolPtr&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtr, Pair<bool, int*>&>::value);
    CORRADE_VERIFY(!std::is_constructible<BoolPtr, const Pair<bool, int*>&>::value);
}

void PairTest::copy() {
    {
        Pair<Copyable, int> a{Copyable{5}, 3};

        Pair<Copyable, int> b = a;
        CORRADE_COMPARE(b.first().a, 5);
        CORRADE_COMPARE(b.second(), 3);

        Pair<Copyable, int> c;
        c = a;
        CORRADE_COMPARE(c.first().a, 5);
        CORRADE_COMPARE(c.second(), 3);

        CORRADE_COMPARE(Copyable::constructed, 4);
        CORRADE_COMPARE(Copyable::destructed, 1);
        CORRADE_COMPARE(Copyable::copied, 2);
        CORRADE_COMPARE(Copyable::moved, 1);
    }

    CORRADE_COMPARE(Copyable::constructed, 4);
    CORRADE_COMPARE(Copyable::destructed, 4);
    CORRADE_COMPARE(Copyable::copied, 2);
    CORRADE_COMPARE(Copyable::moved, 1);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Pair<Copyable, int>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Pair<Copyable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Pair<Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Pair<Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Pair<int, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Pair<int, Throwable>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copyable<Pair<float, int>>::value);
    /* Comparison with std::pair is in PairStlTest::triviallyCopyable() */
    #endif
}

void PairTest::move() {
    {
        Pair<Movable, int> a{Movable{5}, 3};

        Pair<Movable, int> b = Utility::move(a);
        CORRADE_COMPARE(b.first().a, 5);
        CORRADE_COMPARE(b.second(), 3);

        Pair<Movable, int> c;
        c = Utility::move(a);
        CORRADE_COMPARE(c.first().a, 5);
        CORRADE_COMPARE(c.second(), 3);

        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::destructed, 1);
        CORRADE_COMPARE(Movable::moved, 3);
    }

    CORRADE_COMPARE(Movable::constructed, 4);
    CORRADE_COMPARE(Movable::destructed, 4);
    CORRADE_COMPARE(Movable::moved, 3);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Pair<Movable, int>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Pair<Movable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Pair<Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Pair<Throwable, int>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Pair<int, Throwable>>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Pair<int, Throwable>>::value);

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_move_constructible<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_move_assignable<Pair<float, int>>::value);
    /* Comparison with std::pair is in PairStlTest::triviallyMovable() */
    #endif
}

void PairTest::resetCounters() {
    Copyable::constructed = Copyable::destructed = Copyable::copied = Copyable::moved =
        Movable::constructed = Movable::destructed = Movable::moved = 0;
}

void PairTest::compare() {
    Pair<float, int> a{35.0f, 4};
    Pair<float, int> b{35.0f, 4};
    Pair<float, int> c{35.1f, 4};
    Pair<float, int> d{35.0f, 5};

    CORRADE_VERIFY(a == a);
    CORRADE_VERIFY(a == b);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(a != c);
    CORRADE_VERIFY(c != a);
    CORRADE_VERIFY(a != d);
    CORRADE_VERIFY(d != a);
}

void PairTest::access() {
    Pair<float, int> a{35.0f, 4};
    CORRADE_COMPARE(a.first(), 35.0f);
    CORRADE_COMPARE(a.second(), 4);

    a.first() = 72.0f;
    a.second() = 5;
    CORRADE_COMPARE(a.first(), 72.0f);
    CORRADE_COMPARE(a.second(), 5);

    constexpr Pair<float, int> ca{35.0f, 4};
    constexpr float first = ca.first();
    constexpr int second = ca.second();
    CORRADE_COMPARE(first, 35.0f);
    CORRADE_COMPARE(second, 4);
}

void PairTest::accessRvalue() {
    Movable b1 = Pair<Movable, int>{Movable{5}, 3}.first();
    Movable b2 = Pair<int, Movable>{5, Movable{3}}.second();
    CORRADE_COMPARE(b1.a, 5);
    CORRADE_COMPARE(b2.a, 3);
}

void PairTest::accessRvalueLifetimeExtension() {
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
           output of first() and second() to be destroyed only at the end of
           scope, and not already at the ;. A more common case of this would be
           with temporary expressions in a range-for loop, see
           OptionalTest::accessRvalueLifetimeExtension() for an example. */
        auto&& first = Pair<DiesLoudly, int>{DiesLoudly{}, 0}.first();
        auto&& second = Pair<int, DiesLoudly>{0, DiesLoudly{}}.second();
        Debug{} << "shouldn't be dead yet";

        /* So the compiler doesn't complain about the variables being unused
           (even though it's a load-bearing reference) */
        CORRADE_VERIFY(&first);
        CORRADE_VERIFY(&second);
    }
    CORRADE_COMPARE(out,
        "shouldn't be dead yet\n"
        "dying!\n"
        "dying!\n");
}

void PairTest::debug() {
    Containers::String out;
    Debug{&out} << pair(42.5f, 3);
    CORRADE_COMPARE(out, "{42.5, 3}\n");
}

void PairTest::debugPropagateFlags() {
    Containers::String out;
    /* The modifier shouldn't become persistent for values after. The nospace
       modifier shouldn't get propagated. */
    Debug{&out} << ">" << Debug::nospace << Debug::packed << pair(Containers::arrayView({3, 4, 5}), Containers::arrayView({"A", "B"})) << Containers::arrayView({"a", "b", "c"});
    CORRADE_COMPARE(out, ">{345, AB} {a, b, c}\n");
}

void PairTest::constructorExplicitInCopyInitialization() {
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
    Pair<ContainingExplicitDefaultWithImplicitConstructor, ContainingExplicitDefaultWithImplicitConstructor> b;
    CORRADE_VERIFY(&b.first() != &b.second());
}

void PairTest::copyMoveConstructPlainStruct() {
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
    Pair<ExtremelyTrivial, ExtremelyTrivial> aCC{value, value};
    Pair<ExtremelyTrivial, MoveOnlyStruct> aCM{value, MoveOnlyStruct{3, 'a', nullptr}};
    Pair<MoveOnlyStruct, ExtremelyTrivial> aMC{MoveOnlyStruct{3, 'a', nullptr}, value};
    Pair<MoveOnlyStruct, MoveOnlyStruct> aMM{MoveOnlyStruct{3, 'a', nullptr}, MoveOnlyStruct{3, 'a', nullptr}};
    CORRADE_COMPARE(aCC.second().a, 3);
    CORRADE_COMPARE(aCM.second().a, 3);
    CORRADE_COMPARE(aMC.second().a, 3);
    CORRADE_COMPARE(aMM.second().a, 3);

    /* This copy/move-constructs the wrapped value. No special handling needed
       in the implementation as the constructor is implicit. */
    Pair<ExtremelyTrivial, ExtremelyTrivial> bCC = aCC;
    Pair<MoveOnlyStruct, MoveOnlyStruct> bMM = Utility::move(aMM);
    CORRADE_COMPARE(bCC.second().a, 3);
    CORRADE_COMPARE(bMM.second().a, 3);

    /* This copy/move-assigns constructs the wrapped value. No special handling
       needed in the implementation as the assignment is implicit. */
    Pair<ExtremelyTrivial, ExtremelyTrivial> cCC;
    Pair<MoveOnlyStruct, MoveOnlyStruct> cMM{MoveOnlyStruct{6, 'b', nullptr}, MoveOnlyStruct{6, 'b', nullptr}};
    cCC = bCC;
    cMM = Utility::move(bMM);
    CORRADE_COMPARE(cCC.second().a, 3);
    CORRADE_COMPARE(cMM.second().a, 3);

    /* Same as the initial case but with the copy conversion constructor
       instead. The move can't be tested as attempting to move the struct hits
       another GCC 4.8 bug where it attempts to use a copy constructor for
       some reason. So let's just hope the move conversion constructor won't
       regress. */
    Pair<DerivedExtremelyTrivial, DerivedExtremelyTrivial> dCopy{DerivedExtremelyTrivial{3, 'a'}, DerivedExtremelyTrivial{4, 'b'}};
    Pair<ExtremelyTrivial, ExtremelyTrivial> eCopy{dCopy};
    CORRADE_COMPARE(eCopy.second().a, 4);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PairTest)
