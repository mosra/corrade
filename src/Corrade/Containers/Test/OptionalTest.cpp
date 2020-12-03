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
#include <vector>

#include "Corrade/Containers/Optional.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct MaybeInt {
    MaybeInt(int a): a{a} {}

    int a;
};

struct MaybePtr {
    MaybePtr(int* a): a{a} {}
    MaybePtr(const MaybePtr&) = delete;
    MaybePtr(MaybePtr&& other): a{other.a} {
        other.a = nullptr;
    }
    ~MaybePtr() { delete a; }
    MaybePtr& operator=(const MaybePtr&) = delete;
    /* Some compilers get upset when move assignment is not implemented even
       though it's not actually used */
    CORRADE_UNUSED MaybePtr& operator=(MaybePtr&& other) {
        std::swap(a, other.a);
        return *this;
    }

    int* a;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct OptionalConverter<int, MaybeInt> {
    static Optional<int> from(const MaybeInt& other) {
        return other.a;
    }

    static MaybeInt to(const Optional<int>& other) {
        return *other;
    }
};

template<> struct DeducedOptionalConverter<MaybeInt>: OptionalConverter<int, MaybeInt> {};

template<> struct OptionalConverter<int*, MaybePtr> {
    static Optional<int*> from(MaybePtr&& other) {
        Optional<int*> ret{other.a};
        other.a = nullptr;
        return ret;
    }

    static MaybePtr to(Optional<int*>&& other) {
        MaybePtr ret{*other};
        other = NullOpt;
        return ret;
    }
};

template<> struct DeducedOptionalConverter<MaybePtr>: OptionalConverter<int*, MaybePtr> {};

}

namespace Test { namespace {

struct OptionalTest: TestSuite::Tester {
    explicit OptionalTest();

    void nullOptNoDefaultConstructor();
    void nullOptInlineDefinition();

    void constructDefault();
    void constructNullOpt();
    void constructCopy();
    void constructCopyMake();
    void constructMove();
    void constructMoveMake();
    void constructInPlace();
    void constructInPlaceMake();
    void constructInPlaceMakeAmbiguous();
    void convertCopy();
    void convertMove();

    void constructCopyFromNull();
    void constructCopyFromSet();

    void constructMoveFromNull();
    void constructMoveFromSet();

    void boolConversion();

    void compareToOptional();
    void compareToNull();
    void compareToValue();

    void copyNullToNull();
    void copyNullToSet();
    void copySetToNull();
    void copySetToSet();

    void moveNullToNull();
    void moveNullToSet();
    void moveSetToNull();
    void moveSetToSet();

    void moveNullOptToNull();
    void moveNullOptToSet();

    void emplaceNull();
    void emplaceSet();

    void resetCounters();

    void access();
    void accessRvalue();
    void accessInvalid();

    void debug();

    void emplaceConstructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();
    void vectorOfMovableOptional();
};

OptionalTest::OptionalTest() {
    addTests({&OptionalTest::nullOptNoDefaultConstructor,
              &OptionalTest::nullOptInlineDefinition});

    addTests({&OptionalTest::constructDefault,
              &OptionalTest::constructNullOpt,
              &OptionalTest::constructCopy,
              &OptionalTest::constructCopyMake,
              &OptionalTest::constructMove,
              &OptionalTest::constructMoveMake,
              &OptionalTest::constructInPlace,
              &OptionalTest::constructInPlaceMake,
              &OptionalTest::constructInPlaceMakeAmbiguous}, &OptionalTest::resetCounters, &OptionalTest::resetCounters);

    addTests({&OptionalTest::convertCopy,
              &OptionalTest::convertMove});

    addTests({&OptionalTest::constructCopyFromNull,
              &OptionalTest::constructCopyFromSet,

              &OptionalTest::constructMoveFromNull,
              &OptionalTest::constructMoveFromSet,

              &OptionalTest::boolConversion,

              &OptionalTest::compareToOptional,
              &OptionalTest::compareToNull,
              &OptionalTest::compareToValue,

              &OptionalTest::copyNullToNull,
              &OptionalTest::copyNullToSet,
              &OptionalTest::copySetToNull,
              &OptionalTest::copySetToSet,

              &OptionalTest::moveNullToNull,
              &OptionalTest::moveNullToSet,
              &OptionalTest::moveSetToNull,
              &OptionalTest::moveSetToSet,

              &OptionalTest::moveNullOptToNull,
              &OptionalTest::moveNullOptToSet,

              &OptionalTest::emplaceNull,
              &OptionalTest::emplaceSet}, &OptionalTest::resetCounters, &OptionalTest::resetCounters);

    addTests({&OptionalTest::access,
              &OptionalTest::accessRvalue,
              &OptionalTest::accessInvalid,

              &OptionalTest::debug,

              &OptionalTest::emplaceConstructorExplicitInCopyInitialization,
              &OptionalTest::copyConstructPlainStruct,
              &OptionalTest::moveConstructPlainStruct,
              &OptionalTest::vectorOfMovableOptional});
}

void OptionalTest::nullOptNoDefaultConstructor() {
    CORRADE_VERIFY(!std::is_default_constructible<NullOptT>::value);
}

void OptionalTest::nullOptInlineDefinition() {
    CORRADE_VERIFY((std::is_same<decltype(NullOpt), const NullOptT>::value));
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

    explicit Copyable(int a) noexcept: a{a} { ++constructed; }
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

    explicit Movable(int a) noexcept: a{a} { ++constructed; }
    /* To test perfect forwarding in in-place construction */
    explicit Movable(int a, int&&) noexcept: Movable{a} {}
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

void swap(Movable& a, Movable& b) {
    /* Swap these without copying the parent class */
    std::swap(a.a, b.a);
}

void OptionalTest::constructDefault() {
    {
        Optional<Copyable> a;
        CORRADE_VERIFY(!a);
    }

    CORRADE_COMPARE(Copyable::constructed, 0);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_VERIFY(std::is_nothrow_constructible<Optional<Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Optional<Throwable>>::value);
}

void OptionalTest::constructNullOpt() {
    {
        Optional<Copyable> a{NullOpt};
        Optional<Copyable> b = NullOpt;
        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Copyable::constructed, 0);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_VERIFY((std::is_nothrow_constructible<Optional<Copyable>, NullOptT>::value));
    CORRADE_VERIFY((std::is_nothrow_constructible<Optional<Throwable>, NullOptT>::value));
}

void OptionalTest::constructCopy() {
    /* copy construction tested below in constructCopyFrom*()/ copy*To*() */

    {
        Copyable v{32};
        Optional<Copyable> a{v};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);

    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Optional<Copyable>>::value);
    CORRADE_VERIFY((std::is_nothrow_constructible<Optional<Copyable>, const Copyable&>::value));
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Optional<Copyable>>::value);
    CORRADE_VERIFY((std::is_nothrow_assignable<Optional<Copyable>, const Copyable&>::value));

    CORRADE_VERIFY(std::is_copy_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_constructible<Optional<Throwable>>::value);
    CORRADE_VERIFY(!(std::is_nothrow_constructible<Optional<Throwable>, const Throwable&>::value));
    CORRADE_VERIFY(std::is_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_copy_assignable<Optional<Throwable>>::value);
    CORRADE_VERIFY(!(std::is_nothrow_assignable<Optional<Throwable>, const Throwable&>::value));
}

void OptionalTest::constructCopyMake() {
    {
        Copyable v{32};
        auto a = optional(v);
        CORRADE_VERIFY(a);
        CORRADE_VERIFY((std::is_same<decltype(a), Optional<Copyable>>::value));
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    /* Argument is forwarded (passed as T&), so there's just one copy: on return */
    CORRADE_COMPARE(Copyable::copied, 1);
}

void OptionalTest::constructMove() {
    /* move construction tested below in constructMoveFrom*()/ move*To*() */

    {
        Optional<Movable> a{Movable{32}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    CORRADE_VERIFY(!std::is_copy_constructible<Movable>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Movable>::value);
    {
        CORRADE_EXPECT_FAIL("Optional currently doesn't propagate deleted copy constructor/assignment correctly.");
        CORRADE_VERIFY(!std::is_copy_constructible<Optional<Movable>>::value);
        CORRADE_VERIFY(!std::is_copy_assignable<Optional<Movable>>::value);
    }

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Optional<Movable>>::value);
    CORRADE_VERIFY((std::is_nothrow_constructible<Optional<Movable>, Movable&&>::value));
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Optional<Movable>>::value);
    CORRADE_VERIFY((std::is_nothrow_assignable<Optional<Movable>, Movable&&>::value));

    CORRADE_VERIFY(std::is_move_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_constructible<Optional<Throwable>>::value);
    CORRADE_VERIFY(!(std::is_nothrow_constructible<Optional<Throwable>, Throwable&&>::value));
    CORRADE_VERIFY(std::is_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Throwable>::value);
    CORRADE_VERIFY(!std::is_nothrow_move_assignable<Optional<Throwable>>::value);
    CORRADE_VERIFY(!(std::is_nothrow_assignable<Optional<Throwable>, Throwable&&>::value));
}

void OptionalTest::constructMoveMake() {
    {
        auto a = optional(Movable{32});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    /* Argument is forwarded (passed as T&&), so there's just one move: on return */
    CORRADE_COMPARE(Movable::moved, 1);
}

void OptionalTest::constructInPlace() {
    {
        /* Using int{} to test perfect forwarding */
        Optional<Immovable> a{InPlaceInit, 32, int{}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);

    CORRADE_VERIFY(!std::is_copy_constructible<Immovable>::value);
    CORRADE_VERIFY(!std::is_move_constructible<Immovable>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Immovable>::value);
    CORRADE_VERIFY(!std::is_move_assignable<Immovable>::value);
    {
        CORRADE_EXPECT_FAIL("Optional currently doesn't propagate deleted copy/move constructor/assignment correctly.");
        CORRADE_VERIFY(!std::is_copy_constructible<Optional<Immovable>>::value);
        CORRADE_VERIFY(!std::is_move_constructible<Optional<Immovable>>::value);
        CORRADE_VERIFY(!std::is_copy_assignable<Optional<Immovable>>::value);
        CORRADE_VERIFY(!std::is_move_assignable<Optional<Immovable>>::value);
    }

    CORRADE_VERIFY((std::is_nothrow_constructible<Optional<Immovable>, InPlaceInitT, int, int&&>::value));
    CORRADE_VERIFY((std::is_constructible<Optional<Throwable>, InPlaceInitT, int>::value));
    CORRADE_VERIFY(!(std::is_nothrow_constructible<Optional<Throwable>, InPlaceInitT, int>::value));
}

void OptionalTest::constructInPlaceMake() {
    {
        /* Using int{} to test perfect forwarding */
        auto a = optional<Movable>(15, int{});
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 15);
    }

    /* Interesting. So there is a full RVO? */
    CORRADE_COMPARE(Movable::constructed, 1);
    CORRADE_COMPARE(Movable::destructed, 1);
    CORRADE_COMPARE(Movable::moved, 0);
}

void OptionalTest::constructInPlaceMakeAmbiguous() {
    struct Ambiguous {
        Ambiguous() = default;
        Ambiguous(Ambiguous& parent, int = {}): parent{&parent} {}
        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        /* https://developercommunity.visualstudio.com/content/problem/358751/c2580-for-different-versions-of-constructors.html,
           affects 2015 as well (https://stackoverflow.com/a/36658141) */
        Ambiguous(const Ambiguous&) = default;
        #else
        Ambiguous(const Ambiguous& other): parent{other.parent} {}
        #endif
        Ambiguous& operator=(const Ambiguous&) = default;
        Ambiguous* parent{};
    };

    /* Similar to what's in the Pointer test, though there it is a
       static_assert. Here we can't disambiguate. */
    Ambiguous parent;
    auto a = optional(parent);
    auto b = optional(Ambiguous{});
    auto c = optional<Ambiguous>(parent);
    auto d = optional<Ambiguous>(Ambiguous{});
    auto e = optional<Ambiguous>();
    auto f = optional<Ambiguous>(parent, 32);
    auto g = Optional<Ambiguous>{InPlaceInit, parent};
    auto h = Optional<Ambiguous>{parent};
    CORRADE_COMPARE(a->parent, nullptr); /* wrong, but we can't disambiguate */
    CORRADE_COMPARE(b->parent, nullptr);
    CORRADE_COMPARE(c->parent, &parent);
    CORRADE_COMPARE(d->parent, nullptr);
    CORRADE_COMPARE(e->parent, nullptr);
    CORRADE_COMPARE(f->parent, &parent);
    CORRADE_COMPARE(g->parent, &parent);
    CORRADE_COMPARE(h->parent, nullptr);
}

void OptionalTest::convertCopy() {
    MaybeInt a(5);
    CORRADE_COMPARE(a.a, 5);

    Optional<int> b(a);
    CORRADE_VERIFY(b);
    CORRADE_COMPARE(*b, 5);

    MaybeInt c(b);
    CORRADE_COMPARE(c.a, 5);

    auto d = optional(MaybeInt{35});
    CORRADE_VERIFY((std::is_same<decltype(d), Optional<int>>::value));
    CORRADE_VERIFY(d);
    CORRADE_COMPARE(*d, 35);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_constructible<Optional<int>, MaybeInt>::value));
    CORRADE_VERIFY(!(std::is_constructible<Optional<float>, MaybeInt>::value));
    CORRADE_VERIFY((std::is_constructible<MaybeInt, Optional<int>>::value));
    CORRADE_VERIFY(!(std::is_constructible<MaybeInt, Optional<float>>::value));

    /* Implicit conversion is not allowed */
    CORRADE_VERIFY(!(std::is_convertible<const MaybeInt&, Optional<int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<const Optional<int>&, MaybeInt>::value));
}

void OptionalTest::convertMove() {
    MaybePtr a(new int{35});
    CORRADE_COMPARE(*a.a, 35);

    Optional<int*> b(std::move(a));
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(*b);
    CORRADE_COMPARE(**b, 35);
    CORRADE_VERIFY(!a.a);

    MaybePtr c(std::move(b));
    CORRADE_VERIFY(c.a);
    CORRADE_COMPARE(*c.a, 35);
    CORRADE_VERIFY(!b);

    int dv = 17; /* to avoid a leak */
    auto d = optional(MaybePtr{&dv});
    CORRADE_VERIFY((std::is_same<decltype(d), Optional<int*>>::value));
    CORRADE_VERIFY(d);
    CORRADE_VERIFY(*d);
    CORRADE_COMPARE(**d, 17);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_constructible<Optional<int*>, MaybePtr&&>::value));
    CORRADE_VERIFY(!(std::is_constructible<Optional<float*>, MaybePtr&&>::value));
    CORRADE_VERIFY((std::is_constructible<MaybePtr, Optional<int*>&&>::value));
    CORRADE_VERIFY(!(std::is_constructible<MaybePtr, Optional<float*>&&>::value));

    /* Copy construction is not allowed */
    CORRADE_VERIFY(!(std::is_constructible<Optional<int*>, MaybePtr&>::value));
    CORRADE_VERIFY(!(std::is_constructible<Optional<int*>, const MaybePtr&>::value));
    CORRADE_VERIFY(!(std::is_constructible<MaybePtr, Optional<int*>&>::value));
    CORRADE_VERIFY(!(std::is_constructible<MaybePtr, const Optional<int*>&>::value));

    /* Implicit conversion is not allowed */
    CORRADE_VERIFY(!(std::is_convertible<MaybePtr&&, Optional<int*>>::value));
    CORRADE_VERIFY(!(std::is_convertible<Optional<int*>&&, MaybePtr>::value));
}

void OptionalTest::constructCopyFromNull() {
    {
        Optional<Copyable> a;
        Optional<Copyable> b{a};

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Copyable::constructed, 0);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::constructCopyFromSet() {
    {
        Optional<Copyable> a{InPlaceInit, 32};
        Optional<Copyable> b{a};

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::constructMoveFromNull() {
    {
        Optional<Copyable> a;
        Optional<Copyable> b{std::move(a)};

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Copyable::constructed, 0);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::constructMoveFromSet() {
    {
        Optional<Copyable> a{InPlaceInit, 32};
        Optional<Copyable> b{std::move(a)};

        /* A is still set, because the type destructor needs to be called */
        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 1);
}

void OptionalTest::boolConversion() {
    Optional<int> a;
    Optional<int> b{5};

    CORRADE_VERIFY(!a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!!b);

    CORRADE_VERIFY(!(std::is_convertible<Optional<int>, int>::value));
    CORRADE_VERIFY(!(std::is_convertible<Optional<int>, bool>::value));
}

void OptionalTest::compareToOptional() {
    Optional<int> a;
    Optional<int> b{5};
    Optional<int> c{6};

    CORRADE_VERIFY(a == a);
    CORRADE_VERIFY(b == b);
    CORRADE_VERIFY(a != b);
    CORRADE_VERIFY(b != a);
    CORRADE_VERIFY(b != c);
    CORRADE_VERIFY(c != b);
}

void OptionalTest::compareToNull() {
    Optional<int> a;
    Optional<int> b{5};

    CORRADE_VERIFY(a == NullOpt);
    CORRADE_VERIFY(b != NullOpt);
    CORRADE_VERIFY(NullOpt == a);
    CORRADE_VERIFY(NullOpt != b);
}

void OptionalTest::compareToValue() {
    Optional<int> a;
    Optional<int> b{5};
    Optional<int> c{6};

    CORRADE_VERIFY(a != 6);
    CORRADE_VERIFY(b != 6);
    CORRADE_VERIFY(c == 6);
    CORRADE_VERIFY(6 != a);
    CORRADE_VERIFY(6 != b);
    CORRADE_VERIFY(6 == c);
}

void OptionalTest::copyNullToNull() {
    {
        Optional<Copyable> a;
        Optional<Copyable> b;
        b = a;

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Copyable::constructed, 0);
    CORRADE_COMPARE(Copyable::destructed, 0);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::copyNullToSet() {
    {
        Optional<Copyable> a;
        Optional<Copyable> b{InPlaceInit, 32};
        b = a;

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Copyable::constructed, 1);
    CORRADE_COMPARE(Copyable::destructed, 1);
    CORRADE_COMPARE(Copyable::copied, 0);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::copySetToNull() {
    {
        Optional<Copyable> a{InPlaceInit, 32};
        Optional<Copyable> b;
        b = a;

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);
    }

    CORRADE_COMPARE(Copyable::constructed, 2);
    CORRADE_COMPARE(Copyable::destructed, 2);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::copySetToSet() {
    {
        Optional<Copyable> a{InPlaceInit, 32};
        Optional<Copyable> b{InPlaceInit, 78};
        b = a;

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);
    }

    CORRADE_COMPARE(Copyable::constructed, 3);
    CORRADE_COMPARE(Copyable::destructed, 3);
    CORRADE_COMPARE(Copyable::copied, 1);
    CORRADE_COMPARE(Copyable::moved, 0);
}

void OptionalTest::moveNullToNull() {
    {
        Optional<Movable> a;
        Optional<Movable> b;
        b = std::move(a);

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Movable::constructed, 0);
    CORRADE_COMPARE(Movable::destructed, 0);
    CORRADE_COMPARE(Movable::moved, 0);
}

void OptionalTest::moveNullToSet() {
    {
        Optional<Movable> a;
        Optional<Movable> b{InPlaceInit, 32};
        b = std::move(a);

        CORRADE_VERIFY(!a);
        CORRADE_VERIFY(!b);
    }

    CORRADE_COMPARE(Movable::constructed, 1);
    CORRADE_COMPARE(Movable::destructed, 1);
    CORRADE_COMPARE(Movable::moved, 0);
}

void OptionalTest::moveSetToNull() {
    {
        Optional<Movable> a{InPlaceInit, 32};
        Optional<Movable> b;
        b = std::move(a);

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);
}

void OptionalTest::moveSetToSet() {
    {
        Optional<Copyable> a{InPlaceInit, 32};
        Optional<Copyable> b{InPlaceInit, 78};
        b = std::move(a);

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);
    }

    /* One extra construction due to the temporary inside std::swap() */
    CORRADE_COMPARE(Copyable::constructed, 3);
    CORRADE_COMPARE(Copyable::destructed, 3);
    CORRADE_COMPARE(Copyable::copied, 0);
    /* A to temp, B to A, temp to B */
    CORRADE_COMPARE(Copyable::moved, 3);

    {
        Optional<Movable> a{InPlaceInit, 32};
        Optional<Movable> b{InPlaceInit, 78};
        b = std::move(a);

        CORRADE_VERIFY(a);
        CORRADE_VERIFY(b);
        CORRADE_COMPARE(b->a, 32);
    }

    /* The local swap() specialization should take effect, no temporary Movable
       object created and thus just two objects constructed and nothing moved */
    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 0);
}

void OptionalTest::moveNullOptToNull() {
    {
        Optional<Immovable> a;
        a = NullOpt;

        CORRADE_VERIFY(!a);
    }

    CORRADE_COMPARE(Immovable::constructed, 0);
    CORRADE_COMPARE(Immovable::destructed, 0);
}

void OptionalTest::moveNullOptToSet() {
    {
        Optional<Immovable> a{InPlaceInit, 32};
        a = NullOpt;

        CORRADE_VERIFY(!a);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void OptionalTest::emplaceNull() {
    {
        Optional<Immovable> a;
        /* Using int{} to test perfect forwarding */
        a.emplace(32, int{});

        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void OptionalTest::emplaceSet() {
    {
        Optional<Immovable> a{InPlaceInit, 32};
        /* Using int{} to test perfect forwarding */
        a.emplace(76, int{});

        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 76);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
}

void OptionalTest::resetCounters() {
    Copyable::constructed = Copyable::destructed = Copyable::copied = Copyable::moved =
        Movable::constructed = Movable::destructed = Movable::moved =
            Immovable::constructed = Immovable::destructed = 0;
}

void OptionalTest::access() {
    Optional<Copyable> a{Copyable{32}};
    const Optional<Copyable> ca{Copyable{18}};

    CORRADE_VERIFY(a);
    CORRADE_VERIFY(ca);
    CORRADE_COMPARE(a->a, 32);
    CORRADE_COMPARE(ca->a, 18);
    CORRADE_COMPARE((*a).a, 32);
    CORRADE_COMPARE((*ca).a, 18);
}

void OptionalTest::accessRvalue() {
    Movable b = *Optional<Movable>{InPlaceInit, 42};
    CORRADE_COMPARE(b.a, 42);

    #if !defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4
    /* operator*() const && causes ambiguous overload on GCC 4.8 (and I assume
       4.9 as well), so disabling it there. It's not a widely needed feature
       (const&&, *why*) so I think this is okay. */
    const Optional<Movable> ca{Movable{1337}};
    const Movable&& cb = *std::move(ca);
    CORRADE_COMPARE(cb.a, 1337);
    #endif
}

void OptionalTest::accessInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    struct Innocent {
        void foo() const {}
    };

    Optional<Innocent> a;
    const Optional<Innocent> ca;

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
        "Containers::Optional: the optional is empty\n"
        "Containers::Optional: the optional is empty\n"
        "Containers::Optional: the optional is empty\n"
        "Containers::Optional: the optional is empty\n");
}

void OptionalTest::debug() {
    std::stringstream out;
    Debug{&out} << optional(42) << Optional<int>{} << NullOpt;
    CORRADE_COMPARE(out.str(), "42 Containers::NullOpt Containers::NullOpt\n");
}

void OptionalTest::emplaceConstructorExplicitInCopyInitialization() {
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
    Optional<ContainingExplicitDefaultWithImplicitConstructor> b{InPlaceInit};
    Optional<ContainingExplicitDefaultWithImplicitConstructor> c;
    c.emplace();
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(c);
}

void OptionalTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    const ExtremelyTrivial value{3, 'a'};
    Optional<ExtremelyTrivial> a{value};
    CORRADE_COMPARE(a->a, 3);

    /* This copy-constructs the wrapped value */
    Optional<ExtremelyTrivial> b = a;
    CORRADE_COMPARE(b->a, 3);

    /* This deletes and then copy-constructs the wrapped value */
    Optional<ExtremelyTrivial> c;
    c = b;
    CORRADE_COMPARE(c->a, 3);
}

void OptionalTest::moveConstructPlainStruct() {
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
       argument and fails miserably. */
    Optional<MoveOnlyStruct> a{MoveOnlyStruct{3, 'a', nullptr}};
    CORRADE_COMPARE(a->a, 3);

    /* This copy-constructs the wrapped value */
    Optional<MoveOnlyStruct> b = std::move(a);
    CORRADE_COMPARE(b->a, 3);

    /* This deletes and then copy-constructs the wrapped value */
    Optional<MoveOnlyStruct> c;
    c = std::move(b);
    CORRADE_COMPARE(c->a, 3);
}

void OptionalTest::vectorOfMovableOptional() {
    std::vector<Optional<Movable>> vec;

    vec.emplace_back(Movable{23});
    vec.emplace_back();
    vec.emplace_back(NullOpt);
    vec.push_back(Movable{67});

    CORRADE_COMPARE(vec[0]->a, 23);
    CORRADE_VERIFY(!vec[1]);
    CORRADE_VERIFY(!vec[2]);
    CORRADE_COMPARE(vec[3]->a, 67);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::OptionalTest)
