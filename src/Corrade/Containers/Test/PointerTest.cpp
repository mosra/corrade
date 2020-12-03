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

#include "Corrade/Containers/Pointer.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct IntPtr {
    explicit IntPtr(int* a): a{a} {}
    IntPtr(const IntPtr&) = delete;
    IntPtr(IntPtr&& other): a{other.a} {
        other.a = nullptr;
    }
    ~IntPtr() { delete a; }
    IntPtr& operator=(const IntPtr&) = delete;
    /* Some compilers get upset when move assignment is not implemented even
       though it's not actually used */
    CORRADE_UNUSED IntPtr& operator=(IntPtr&& other) {
        std::swap(a, other.a);
        return *this;
    }

    int* a;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct PointerConverter<int, IntPtr> {
    static Pointer<int> from(IntPtr&& other) {
        Pointer<int> ret{other.a};
        other.a = nullptr;
        return ret;
    }

    static IntPtr to(Pointer<int>&& other) {
        return IntPtr{other.release()};
    }
};

template<> struct DeducedPointerConverter<IntPtr>: PointerConverter<int, IntPtr> {};

}

namespace Test { namespace {

struct PointerTest: TestSuite::Tester {
    explicit PointerTest();

    void resetCounters();

    void construct();
    void constructDefault();
    void constructNullptr();
    void constructCopy();
    void constructMove();
    void constructMake();
    void constructInPlace();
    void constructInPlaceMake();
    void constructInPlaceMakeAmbiguous();
    void constructDerived();
    void convert();

    void boolConversion();
    void compareToNullptr();

    void access();
    void accessInvalid();

    void reset();
    void emplace();
    void release();

    void cast();

    void emplaceConstructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();

    void debug();
};

PointerTest::PointerTest() {
    addTests({&PointerTest::construct,
              &PointerTest::constructDefault,
              &PointerTest::constructNullptr}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::constructCopy});

    addTests({&PointerTest::constructMove,
              &PointerTest::constructMake,
              &PointerTest::constructInPlace,
              &PointerTest::constructInPlaceMake}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::constructInPlaceMakeAmbiguous,
              &PointerTest::constructDerived,
              &PointerTest::convert,

              &PointerTest::boolConversion,
              &PointerTest::compareToNullptr});

    addTests({&PointerTest::access}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::accessInvalid});

    addTests({&PointerTest::reset,
              &PointerTest::emplace,
              &PointerTest::release}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::cast,

              &PointerTest::emplaceConstructorExplicitInCopyInitialization,
              &PointerTest::copyConstructPlainStruct,
              &PointerTest::moveConstructPlainStruct,

              &PointerTest::debug});
}

struct Immovable {
    static int constructed;
    static int destructed;

    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
    explicit Immovable(int a) noexcept: a{a} { ++constructed; }
    /* To test perfect forwarding in in-place construction */
    explicit Immovable(int a, int&&) noexcept: Immovable{a} {}
    ~Immovable() { ++destructed; }
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;

    int a;
};

int Immovable::constructed = 0;
int Immovable::destructed = 0;

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
#endif
struct Throwable {
    /* These have to be defined and not defaulted so the compiler doesn't put
       noexcept on them. But Clang warns that they're unused. */
    explicit Throwable(int) {}
    Throwable(const Throwable&) {}
    Throwable(Throwable&&) {}
    Throwable& operator=(const Throwable&) { return *this; }
    Throwable& operator=(Throwable&&) { return *this; }
};
#ifdef __clang__
#pragma GCC diagnostic pop
#endif

void PointerTest::resetCounters() {
    Immovable::constructed = Immovable::destructed = 0;
}

void PointerTest::construct() {
    {
        Pointer<Immovable> a{new Immovable{42}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 42);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);

    CORRADE_VERIFY((std::is_nothrow_constructible<Pointer<int>, int*>::value));
    CORRADE_VERIFY(!(std::is_assignable<Pointer<int>, int*>::value));
}

void PointerTest::constructDefault() {
    Pointer<Immovable> a;
    Pointer<Immovable> b = {};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);

    CORRADE_VERIFY((std::is_nothrow_constructible<Pointer<int>>::value));
}

void PointerTest::constructNullptr() {
    Pointer<Immovable> a{nullptr};
    Pointer<Immovable> b = nullptr;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);

    CORRADE_VERIFY((std::is_nothrow_constructible<Pointer<int>, std::nullptr_t>::value));
    CORRADE_VERIFY((std::is_nothrow_assignable<Pointer<int>, std::nullptr_t>::value));
}

void PointerTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<Pointer<int>>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Pointer<int>>::value);
}

void PointerTest::constructMove() {
    {
        Pointer<Immovable> a{new Immovable{32}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);

        Pointer<Immovable> b = std::move(a);
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);

        Pointer<Immovable> c{new Immovable{56}};
        CORRADE_VERIFY(c);
        CORRADE_COMPARE(c->a, 56);

        c = std::move(b);
        CORRADE_VERIFY(c);
        CORRADE_COMPARE(c->a, 32);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Pointer<Immovable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Pointer<Immovable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Pointer<Throwable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Pointer<Throwable>>::value);
}

void PointerTest::constructMake() {
    {
        auto a = pointer(new Immovable{1337});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 1337);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void PointerTest::constructInPlace() {
    {
        /* Using int{} to test perfect forwarding */
        Pointer<Immovable> a{InPlaceInit, -13, int{}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, -13);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);

    /* This is never noexcept since we allocate (duh) */
    CORRADE_VERIFY((std::is_constructible<Pointer<Immovable>, InPlaceInitT, int>::value));
    CORRADE_VERIFY(!(std::is_nothrow_constructible<Pointer<Immovable>, InPlaceInitT, int, int&&>::value));
    CORRADE_VERIFY((std::is_constructible<Pointer<Throwable>, InPlaceInitT, int>::value));
    CORRADE_VERIFY(!(std::is_nothrow_constructible<Pointer<Throwable>, InPlaceInitT, int>::value));
}

void PointerTest::constructInPlaceMake() {
    {
        /* Using int{} to test perfect forwarding */
        auto a = pointer<Immovable>(1337, int{});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 1337);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void PointerTest::constructInPlaceMakeAmbiguous() {
    struct Ambiguous {
        Ambiguous() = default;
        Ambiguous(Ambiguous* parent, int = {}): parent{parent} {}
        Ambiguous(const Ambiguous&) = delete;
        Ambiguous& operator=(const Ambiguous&) = delete;
        Ambiguous* parent{};
    };

    /* All the commented-out calls below should static_assert because they're
       ambiguous */
    Ambiguous parent;
    //auto a = pointer(&parent);
    //auto b = pointer(new Ambiguous);
    //auto c = pointer<Ambiguous>(&parent);
    //auto d = pointer<Ambiguous>(new Ambiguous);
    auto e = pointer<Ambiguous>();
    auto f = pointer<Ambiguous>(&parent, 32);
    auto g = Pointer<Ambiguous>{InPlaceInit, &parent};
    auto h = Pointer<Ambiguous>{new Ambiguous};
    //CORRADE_COMPARE(a->parent, &parent);
    //CORRADE_COMPARE(b->parent, nullptr);
    //CORRADE_COMPARE(c->parent, &parent);
    //CORRADE_COMPARE(d->parent, nullptr);
    CORRADE_COMPARE(e->parent, nullptr);
    CORRADE_COMPARE(f->parent, &parent);
    CORRADE_COMPARE(g->parent, &parent);
    CORRADE_COMPARE(h->parent, nullptr);
}

void PointerTest::constructDerived() {
    struct Base { int a; };
    struct Derived: Base {
        Derived(int a): Base{a} {}
    };

    Pointer<Derived> a{InPlaceInit, 42};
    Pointer<Base> b = std::move(a);
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 42);

    /* Test assign as well */
    b = Pointer<Derived>{InPlaceInit, 36};
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 36);

    CORRADE_VERIFY((std::is_nothrow_constructible<Pointer<Base>, Pointer<Derived>>::value));

    CORRADE_VERIFY((std::is_constructible<Pointer<Base>, Derived*>::value));
    CORRADE_VERIFY(!(std::is_constructible<Pointer<Derived>, Base*>::value));
    CORRADE_VERIFY((std::is_constructible<Pointer<Base>, Pointer<Derived>>::value));
    CORRADE_VERIFY(!(std::is_constructible<Pointer<Derived>, Pointer<Base>>::value));
}

void PointerTest::convert() {
    IntPtr a{new int{5}};
    int* ptr = a.a;
    CORRADE_VERIFY(ptr);
    CORRADE_COMPARE(*ptr, 5);

    Pointer<int> b = std::move(a); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(b.get(), ptr);
    CORRADE_COMPARE(*b, 5);
    CORRADE_VERIFY(!a.a);

    IntPtr c = std::move(b); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(c.a, ptr);
    CORRADE_COMPARE(*c.a, 5);
    CORRADE_VERIFY(!b);

    auto d = pointer(IntPtr{new int{72}});
    CORRADE_VERIFY((std::is_same<decltype(d), Pointer<int>>::value));
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(*d, 72);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_constructible<Pointer<int>, IntPtr>::value));
    CORRADE_VERIFY(!(std::is_constructible<Pointer<float>, IntPtr>::value));
    CORRADE_VERIFY((std::is_constructible<IntPtr, Pointer<int>>::value));
    CORRADE_VERIFY(!(std::is_constructible<IntPtr, Pointer<float>>::value));

    /* Non-move conversion is not allowed */
    CORRADE_VERIFY((std::is_convertible<IntPtr&&, Pointer<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const IntPtr&, Pointer<int>>::value));
    CORRADE_VERIFY((std::is_convertible<Pointer<int>&&, IntPtr>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Pointer<int>&, IntPtr>::value));
}

void PointerTest::boolConversion() {
    Pointer<int> a;
    Pointer<int> b{new int{5}};

    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!!b);

    CORRADE_VERIFY(!(std::is_convertible<Pointer<int>, int>::value));
    CORRADE_VERIFY(!(std::is_convertible<Pointer<int>, bool>::value));
}

void PointerTest::compareToNullptr() {
    Pointer<int> a;
    Pointer<int> b{new int{5}};

    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b != nullptr);
}

void PointerTest::access() {
    {
        Pointer<Immovable> a{InPlaceInit, 5};
        const Pointer<Immovable> ca{InPlaceInit, 8};

        CORRADE_COMPARE(a->a, 5);
        CORRADE_COMPARE(ca->a, 8);
        CORRADE_COMPARE((*a).a, 5);
        CORRADE_COMPARE((*ca).a, 8);
        CORRADE_COMPARE(a.get()->a, 5);
        CORRADE_COMPARE(ca.get()->a, 8);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
}

void PointerTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct Innocent {
        void foo() const {}
    };

    Pointer<Innocent> a;
    const Pointer<Innocent> ca;

    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!ca);

    std::ostringstream out;
    {
        Error redirectError{&out};
        a->foo();
        ca->foo();
        (*a).foo();
        (*ca).foo();
    }
    CORRADE_COMPARE(out.str(),
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n");
}

void PointerTest::reset() {
    {
        Pointer<Immovable> a{InPlaceInit, 5};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 5);

        a.reset(new Immovable{16});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 16);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
}

void PointerTest::emplace() {
    {
        Pointer<Immovable> a{InPlaceInit, 5};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 5);

        /* Using int{} to test perfect forwarding */
        a.emplace(16, int{});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 16);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
}

void PointerTest::release() {
    Immovable* ptr;
    {
        Pointer<Immovable> a{InPlaceInit, 5};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 5);

        ptr = a.release();
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(ptr);
    }

    delete ptr;

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void PointerTest::cast() {
    struct Base {};
    struct Derived: Base {
        Derived(int a): a{a} {}
        int a;
    };

    Pointer<Base> a{new Derived{42}};
    Pointer<Derived> b = pointerCast<Derived>(std::move(a));
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 42);
}

void PointerTest::emplaceConstructorExplicitInCopyInitialization() {
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
    Pointer<ContainingExplicitDefaultWithImplicitConstructor> b{InPlaceInit};
    Pointer<ContainingExplicitDefaultWithImplicitConstructor> c;
    c.emplace();
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(c);
}

void PointerTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably -- a{InPlaceInit, 3, 'a'} would in-place
       initialize them, which is fine and doesn't need workarounds */
    const ExtremelyTrivial value{3, 'a'};
    Pointer<ExtremelyTrivial> a{InPlaceInit, value};
    CORRADE_COMPARE(a->a, 3);

    /* This copy-constructs new values -- emplace(4, 'b') would in-place
       initialize them, which is fine and doesn't need workarounds */
    const ExtremelyTrivial another{4, 'b'};
    a.emplace(another);
    CORRADE_COMPARE(a->a, 4);
}

void PointerTest::moveConstructPlainStruct() {
    struct MoveOnlyStruct {
        int a;
        char c;
        Pointer<int> b;
    };

    /* This needs special handling on GCC 4.8, where T{std::move(b)} attempts
       to convert MoveOnlyStruct to int to initialize the first argument and
       fails miserably -- a{InPlaceInit, 3, 'a', nullptr} would in-place
       initialize them, which is fine and doesn't need workarounds */
    Pointer<MoveOnlyStruct> a{InPlaceInit, MoveOnlyStruct{3, 'a', nullptr}};
    CORRADE_COMPARE(a->a, 3);

    /* This copy-constructs new values -- emplace(4, 'b', nullptr) would
       in-place initialize them, which is fine and doesn't need workarounds */
    a.emplace(MoveOnlyStruct{4, 'b', nullptr});
    CORRADE_COMPARE(a->a, 4);
}

void PointerTest::debug() {
    std::stringstream out;
    std::intptr_t a = 0xdeadbeef;
    Pointer<int> aptr{reinterpret_cast<int*>(a)};
    Debug{&out} << aptr << Pointer<int>{} << nullptr;
    aptr.release();
    CORRADE_COMPARE(out.str(), "0xdeadbeef 0x0 nullptr\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PointerTest)
