/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#include "Corrade/Containers/Pointer.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

namespace {

struct IntPtr {
    explicit IntPtr(int* a): a{a} {}
    IntPtr(const IntPtr&) = delete;
    /* With the guaranteed copy/move elision in C++17 this one might be unused
       as well */
    #ifdef CORRADE_TARGET_CXX17
    CORRADE_UNUSED
    #endif
    IntPtr(IntPtr&& other): a{other.a} {
        other.a = nullptr;
    }
    ~IntPtr() { delete a; }
    IntPtr& operator=(const IntPtr&) = delete;
    /* Some compilers get upset when move assignment is not implemented even
       though it's not actually used */
    CORRADE_UNUSED IntPtr& operator=(IntPtr&& other) {
        Corrade::Utility::swap(a, other.a);
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

    void isComplete();

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
    void constructDerivedTriviallyDestructible();
    void constructDerivedVirtualDestructor();
    void constructConvertibleButNotDerived();
    void constructIncomplete();

    void constructZeroNullPointerAmbiguity();

    void convert();

    void boolConversion();
    void compareToNullptr();

    void access();
    void accessInvalid();

    void reset();
    void emplace();
    void emplaceDerivedTriviallyDestructible();
    void emplaceDerivedVirtualDestructor();
    void release();

    void cast();

    void constructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();

    void debug();
};

PointerTest::PointerTest() {
    addTests({&PointerTest::isComplete});

    addTests({&PointerTest::construct,
              &PointerTest::constructDefault,
              &PointerTest::constructNullptr}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::constructCopy});

    addTests({&PointerTest::constructMove,
              &PointerTest::constructMake,
              &PointerTest::constructInPlace,
              &PointerTest::constructInPlaceMake}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::constructInPlaceMakeAmbiguous,
              &PointerTest::constructDerivedTriviallyDestructible,
              &PointerTest::constructDerivedVirtualDestructor,
              &PointerTest::constructConvertibleButNotDerived,
              &PointerTest::constructIncomplete,

              &PointerTest::constructZeroNullPointerAmbiguity,

              &PointerTest::convert,

              &PointerTest::boolConversion,
              &PointerTest::compareToNullptr});

    addTests({&PointerTest::access}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::accessInvalid});

    addTests({&PointerTest::reset,
              &PointerTest::emplace}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::emplaceDerivedTriviallyDestructible,
              &PointerTest::emplaceDerivedVirtualDestructor});

    addTests({&PointerTest::release}, &PointerTest::resetCounters, &PointerTest::resetCounters);

    addTests({&PointerTest::cast,

              &PointerTest::constructorExplicitInCopyInitialization,
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

#ifdef CORRADE_TARGET_CLANG
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
#ifdef CORRADE_TARGET_CLANG
#pragma GCC diagnostic pop
#endif

struct Incomplete;
struct Complete {};

void PointerTest::isComplete() {
    CORRADE_VERIFY(!Implementation::IsComplete<Incomplete>::value);
    CORRADE_VERIFY(Implementation::IsComplete<Complete>::value);
    CORRADE_VERIFY(Implementation::IsComplete<int>::value);
}

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

    CORRADE_VERIFY(std::is_nothrow_constructible<Pointer<int>, int*>::value);
    CORRADE_VERIFY(!std::is_assignable<Pointer<int>, int*>::value);
}

void PointerTest::constructDefault() {
    Pointer<Immovable> a;
    Pointer<Immovable> b = {};
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pointer<int>>::value);
}

void PointerTest::constructNullptr() {
    Pointer<Immovable> a{nullptr};
    Pointer<Immovable> b = nullptr;
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!b);

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pointer<int>, std::nullptr_t>::value);
    CORRADE_VERIFY(std::is_nothrow_assignable<Pointer<int>, std::nullptr_t>::value);
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

        Pointer<Immovable> b = Utility::move(a);
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);

        Pointer<Immovable> c{new Immovable{56}};
        CORRADE_VERIFY(c);
        CORRADE_COMPARE(c->a, 56);

        c = Utility::move(b);
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
        Pointer<Immovable> a{Corrade::InPlaceInit, -13, int{}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, -13);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);

    /* This is never noexcept since we allocate (duh) */
    CORRADE_VERIFY(std::is_constructible<Pointer<Immovable>, Corrade::InPlaceInitT, int>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pointer<Immovable>, Corrade::InPlaceInitT, int, int&&>::value);
    CORRADE_VERIFY(std::is_constructible<Pointer<Throwable>, Corrade::InPlaceInitT, int>::value);
    CORRADE_VERIFY(!std::is_nothrow_constructible<Pointer<Throwable>, Corrade::InPlaceInitT, int>::value);
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
    auto g = Pointer<Ambiguous>{Corrade::InPlaceInit, &parent};
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

void PointerTest::constructDerivedTriviallyDestructible() {
    /* Verifies that conversion from a derived type is allowed for trivially
       destructible type, and disallowed for non-trivially-destructible */

    struct Base {
        int a;
        explicit Base(int a): a{a} {}
    };
    struct Derived: Base {
        explicit Derived(int a, int b): Base{a}, b{b} {}
        int b;
        /* Uncomment this to trigger the assertion */
        //Containers::Pointer<int> a;
    };

    Pointer<Derived> a{Corrade::InPlaceInit, 42, 17};
    Pointer<Base> b = Utility::move(a);
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 42);
    CORRADE_COMPARE(static_cast<Derived&>(*b).b, 17);

    /* Test assign as well */
    b = Pointer<Derived>{Corrade::InPlaceInit, 36, 63};
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 36);
    CORRADE_COMPARE(static_cast<Derived&>(*b).b, 63);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pointer<Base>, Pointer<Derived>>::value);

    CORRADE_VERIFY(std::is_constructible<Pointer<Base>, Derived*>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<Derived>, Base*>::value);
    CORRADE_VERIFY(std::is_constructible<Pointer<Base>, Pointer<Derived>>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<Derived>, Pointer<Base>>::value);
}

void PointerTest::constructDerivedVirtualDestructor() {
    /* Verifies that conversion from a derived type is allowed for base types
       with virtual destructor, and disallowed for base types without */

    struct Base {
        int a;
        explicit Base(int a): a{a} {}
        /* Comment this out to trigger the assertion */
        virtual ~Base() = default;
    };
    struct Derived: Base {
        explicit Derived(int a, int& derivedDestructed): Base{a}, derivedDestructed(derivedDestructed) {}
        /* The virtual here isn't enough, the base class has to have it too */
        virtual ~Derived() {
            ++derivedDestructed;
        }
        int& derivedDestructed;
    };

    int aDerivedDestructed = 0;
    {
        Pointer<Derived> a{Corrade::InPlaceInit, 42, aDerivedDestructed};
        Pointer<Base> b = Utility::move(a);
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 42);
    }
    CORRADE_COMPARE(aDerivedDestructed, 1);

    /* Test assign as well */
    int bDerivedDestructed = 0;
    {
        Pointer<Base> b;
        b = Pointer<Derived>{Corrade::InPlaceInit, 36, bDerivedDestructed};
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 36);
    }
    CORRADE_COMPARE(bDerivedDestructed, 1);

    CORRADE_VERIFY(std::is_nothrow_constructible<Pointer<Base>, Pointer<Derived>>::value);

    CORRADE_VERIFY(std::is_constructible<Pointer<Base>, Derived*>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<Derived>, Base*>::value);
    CORRADE_VERIFY(std::is_constructible<Pointer<Base>, Pointer<Derived>>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<Derived>, Pointer<Base>>::value);
}

void PointerTest::constructConvertibleButNotDerived() {
    /* The base-from-derived constructor cannot use std::is_base_of<T, U>
       because it requires T to be defined, which is sometimes a problem with
       generic code such as the following.

        template<class T> void foo(Containers::Pointer<T>&& derived) {
            Containers::Pointer<Base> base{Utility::move(derived)};
            ...
        }

       Instead, std::is_convertible<U*, T*> is used, which doesn't have this
       requirement, and the only difference compared to std::is_base_of<T, U>
       seems to be that it doesn't work for private inheritance (which is fine,
       as the Pointer wouldn't work with that either). This test verifies that
       code where U is convertible to T but isn't derived from T correctly
       fails to compile (i.e., causing a "no matching constructor" error, not
       some subsequent failure inside the constructor). */

    struct Base {
        int a;
        explicit Base(int a): a{a} {}
        virtual ~Base() = default;
    };

    struct Derived: Base {};

    struct Unrelated {
        operator Base&() { return base; }
        Base base{3};
    };

    CORRADE_VERIFY(std::is_constructible<Containers::Pointer<Base>, Containers::Pointer<Derived>&&>::value);
    /* With std::is_convertible<U&, T&> this would pass (although the commented
       out code below would still fail to compile), which is incorrect */
    CORRADE_VERIFY(!std::is_constructible<Containers::Pointer<Base>, Containers::Pointer<Unrelated>&&>::value);

    Containers::Pointer<Unrelated> a{Corrade::InPlaceInit};
    Containers::Pointer<Base> b;
    CORRADE_COMPARE(a->base.a, 3);

    /* This shouldn't compile */
    // b = Utility::move(a);
    CORRADE_VERIFY(!b);
}

void PointerTest::constructIncomplete() {
    /* Declaring with incomplete type should work without errors, making an
       instance only if the actual destruction is done with the type defined.
       Remove the * to test the assertion failure in the destructor. */
    Containers::Pointer<Incomplete>* a = nullptr;
    /* Uncomment these to test compile or assertion failures in other APIs. The
       emplace() in particular doesn't need a static_assert, as there will be
       a compiler failure either when trying to allocate incomplete T, or when
       trying to assign U to incomplete T. */
    //a->emplace();
    //a->emplace<Complete>();
    //a->reset();
    CORRADE_VERIFY(!a);
}

/* Without a corresponding SFINAE check in the std::nullptr_t constructor, this
   is ambiguous, but *only* if the size_t overload has a second 64-bit
   argument. If both would be the same, it wouldn't be ambigous, if the size_t
   overload second argument was 32-bit and the other 16-bit it wouldn't be
   either. */
int integerPointerOverload(std::size_t, long long) {
    return 76;
}
int integerPointerOverload(const Pointer<int>&, int) {
    return 39;
}

void PointerTest::constructZeroNullPointerAmbiguity() {
    /* Obvious cases */
    CORRADE_COMPARE(integerPointerOverload(25, 2), 76);
    CORRADE_COMPARE(integerPointerOverload(nullptr, 2), 39);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerPointerOverload(0, 3), 76);
}

void PointerTest::convert() {
    IntPtr a{new int{5}};
    int* ptr = a.a;
    CORRADE_VERIFY(ptr);
    CORRADE_COMPARE(*ptr, 5);

    Pointer<int> b = Utility::move(a); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(b.get(), ptr);
    CORRADE_COMPARE(*b, 5);
    CORRADE_VERIFY(!a.a);

    IntPtr c = Utility::move(b); /* implicit conversion *is* allowed */
    CORRADE_COMPARE(c.a, ptr);
    CORRADE_COMPARE(*c.a, 5);
    CORRADE_VERIFY(!b);

    auto d = pointer(IntPtr{new int{72}});
    CORRADE_VERIFY(std::is_same<decltype(d), Pointer<int>>::value);
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(*d, 72);

    /* Conversion from a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Pointer<int>, IntPtr>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<float>, IntPtr>::value);
    CORRADE_VERIFY(std::is_constructible<IntPtr, Pointer<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<IntPtr, Pointer<float>>::value);

    /* Non-move conversion is not allowed. Not using is_convertible to catch
       also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Pointer<int>, IntPtr&&>::value);
    CORRADE_VERIFY(!std::is_constructible<Pointer<int>, const IntPtr&>::value);
    CORRADE_VERIFY(std::is_constructible<IntPtr, Pointer<int>&&>::value);
    CORRADE_VERIFY(!std::is_constructible<IntPtr, const Pointer<int>&>::value);
}

void PointerTest::boolConversion() {
    Pointer<int> a;
    Pointer<int> b{new int{5}};

    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!!b);

    /* Implicit conversion is not allowed */
    CORRADE_VERIFY(!std::is_convertible<Pointer<int>, int>::value);
    CORRADE_VERIFY(!std::is_convertible<Pointer<int>, bool>::value);
}

void PointerTest::compareToNullptr() {
    Pointer<int> a;
    Pointer<int> b{new int{5}};

    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b != nullptr);
}

void PointerTest::access() {
    {
        Pointer<Immovable> a{Corrade::InPlaceInit, 5};
        const Pointer<Immovable> ca{Corrade::InPlaceInit, 8};

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
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    struct Innocent {
        void foo() const {}
    };

    Pointer<Innocent> a;
    const Pointer<Innocent> ca;

    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(!ca);

    Containers::String out;
    {
        Error redirectError{&out};
        a->foo();
        ca->foo();
        (*a).foo();
        (*ca).foo();
    }
    CORRADE_COMPARE(out,
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n"
        "Containers::Pointer: the pointer is null\n");
}

void PointerTest::reset() {
    {
        Pointer<Immovable> a{Corrade::InPlaceInit, 5};
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
        Pointer<Immovable> a{Corrade::InPlaceInit, 5};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 5);

        /* Using int{} to test perfect forwarding */
        Immovable& out = a.emplace(16, int{});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(&out, &*a);
        CORRADE_COMPARE(out.a, 16);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
}

void PointerTest::emplaceDerivedTriviallyDestructible() {
    /* Verifies that emplace() with a derived type is allowed for trivially
       destructible type, and disallowed for non-trivially-destructible */

    struct Base {
        int a;
        explicit Base(int a): a{a} {}
    };
    struct Derived: Base {
        explicit Derived(int a, int b, int&&): Base{a}, b{b} {}
        int b;
        /* Uncomment this to trigger the assertion */
        //Containers::Pointer<int> a;
    };

    /* Using int{} to test perfect forwarding */
    Pointer<Base> a;
    Derived& out = a.emplace<Derived>(42, 17, int{});
    CORRADE_VERIFY(a);
    CORRADE_COMPARE(&out, &*a);
    CORRADE_COMPARE(out.a, 42);
    CORRADE_COMPARE(static_cast<Derived&>(out).b, 17);
}

void PointerTest::emplaceDerivedVirtualDestructor() {
    /* Verifies that emplace() with a derived type is allowed for base types
       with virtual destructor, and disallowed for base types without */

    struct Base {
        int a;
        explicit Base(int a): a{a} {}
        /* Comment this out to trigger the assertion */
        virtual ~Base() = default;
    };
    struct Derived: Base {
        explicit Derived(int a, int& derivedDestructed, int&&): Base{a}, derivedDestructed(derivedDestructed) {}
        /* The virtual here isn't enough, the base class has to have it too */
        virtual ~Derived() {
            ++derivedDestructed;
        }
        int& derivedDestructed;
    };

    int derivedDestructed = 0;
    {
        /* Using int{} to test perfect forwarding */
        Pointer<Base> a;
        Derived& out = a.emplace<Derived>(42, derivedDestructed, int{});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(&out, &*a);
        CORRADE_COMPARE(out.a, 42);
    }
    CORRADE_COMPARE(derivedDestructed, 1);
}

void PointerTest::release() {
    Immovable* ptr;
    {
        Pointer<Immovable> a{Corrade::InPlaceInit, 5};
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
    Pointer<Derived> b = pointerCast<Derived>(Utility::move(a));
    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(b->a, 42);
}

void PointerTest::constructorExplicitInCopyInitialization() {
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
    Pointer<ContainingExplicitDefaultWithImplicitConstructor> b{Corrade::InPlaceInit};
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
    Pointer<ExtremelyTrivial> a{Corrade::InPlaceInit, value};
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

    /* This needs special handling on GCC 4.8, where T{Utility::move(b)}
       attempts to convert MoveOnlyStruct to int to initialize the first
       argument and fails miserably -- a{InPlaceInit, 3, 'a', nullptr} would
       in-place initialize them, which is fine and doesn't need workarounds */
    Pointer<MoveOnlyStruct> a{Corrade::InPlaceInit, MoveOnlyStruct{3, 'a', nullptr}};
    CORRADE_COMPARE(a->a, 3);

    /* This copy-constructs new values -- emplace(4, 'b', nullptr) would
       in-place initialize them, which is fine and doesn't need workarounds */
    a.emplace(MoveOnlyStruct{4, 'b', nullptr});
    CORRADE_COMPARE(a->a, 4);
}

/* GCC 11 adds -Wfree-nonheap-object, which then complains that "void operator
   delete(void*) called on a pointer to an unallocated object 3735928559" which
   is extremely silly, given it would happen only if we threw an exception
   before the release() call, which we don't. The suppression has to be outside
   of the function as otherwise it doesn't work at all. I don't like your
   "helpful" warnings, GCC, shut up. */
#if defined(CORRADE_TARGET_GCC) && __GNUC__ >= 11 && __OPTIMIZE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
void PointerTest::debug() {
    Containers::String out;
    std::intptr_t a = 0xdeadbeef;
    Pointer<int> aptr{reinterpret_cast<int*>(a)};
    Debug{&out} << aptr << Pointer<int>{} << nullptr;
    aptr.release();
    CORRADE_COMPARE(out, "0xdeadbeef 0x0 nullptr\n");
}
#if defined(CORRADE_TARGET_GCC) && __GNUC__ >= 11 && __OPTIMIZE__
#pragma GCC diagnostic pop
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PointerTest)
