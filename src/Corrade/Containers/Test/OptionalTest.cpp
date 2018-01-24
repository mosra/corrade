/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Corrade { namespace Containers { namespace Test {

struct OptionalTest: TestSuite::Tester {
    explicit OptionalTest();

    void nullOptNotDefaultConstructible();

    void constructDefault();
    void constructNullOpt();
    void constructCopy();
    void constructCopyMake();
    void constructMove();
    void constructMoveMake();
    void constructInPlace();

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

    void emplaceNull();
    void emplaceSet();

    void resetCounters();

    void access();
    void debug();

    void vectorOfMovableOptional();
};

OptionalTest::OptionalTest() {
    addTests({&OptionalTest::nullOptNotDefaultConstructible});

    addTests({&OptionalTest::constructDefault,
              &OptionalTest::constructNullOpt,
              &OptionalTest::constructCopy,
              &OptionalTest::constructCopyMake,
              &OptionalTest::constructMove,
              &OptionalTest::constructMoveMake,

              &OptionalTest::constructCopyFromNull,
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

              &OptionalTest::emplaceNull,
              &OptionalTest::emplaceSet}, &OptionalTest::resetCounters, &OptionalTest::resetCounters);

    addTests({&OptionalTest::access,
              &OptionalTest::debug,

              &OptionalTest::vectorOfMovableOptional});
}

namespace {

struct Copyable {
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;

    Copyable(int a) noexcept: a{a} { ++constructed; }
    ~Copyable() { ++destructed; }
    Copyable(const Copyable& other) noexcept: a{other.a} {
        ++constructed;
        ++copied;
    }
    Copyable(Copyable&& other) noexcept: a{other.a} {
        ++constructed;
        ++moved;
    }
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

    Movable(int a): a{a} { ++constructed; }
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
    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;

    Immovable(int a): a{a} { ++constructed; }
    ~Immovable() { ++destructed; }

    int a;
};

int Immovable::constructed = 0;
int Immovable::destructed = 0;

void swap(Movable& a, Movable& b) {
    /* Swap these without copying the parent class */
    std::swap(a.a, b.a);
}

}

void OptionalTest::nullOptNotDefaultConstructible() {
    CORRADE_VERIFY(!std::is_default_constructible<NullOptT>::value);
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
}

void OptionalTest::constructCopy() {
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
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Optional<Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Optional<Copyable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Copyable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Optional<Copyable>>::value);
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
    {
        Optional<Movable> a{Movable{32}};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::destructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_constructible<Optional<Movable>>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Movable>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Optional<Movable>>::value);
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
        Optional<Immovable> a{InPlaceInit, 32};
        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Immovable::constructed, 2);
    CORRADE_COMPARE(Immovable::destructed, 2);
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

void OptionalTest::emplaceNull() {
    {
        Optional<Immovable> a;
        a.emplace(32);

        CORRADE_VERIFY(a);
        CORRADE_COMPARE(a->a, 32);
    }

    CORRADE_COMPARE(Immovable::constructed, 1);
    CORRADE_COMPARE(Immovable::destructed, 1);
}

void OptionalTest::emplaceSet() {
    {
        Optional<Immovable> a{InPlaceInit, 32};
        a.emplace(76);

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
    Optional<Copyable> a{32};
    const Optional<Copyable> ca{32};

    CORRADE_VERIFY(a);
    CORRADE_VERIFY(ca);
    CORRADE_COMPARE(a->a, 32);
    CORRADE_COMPARE(ca->a, 32);
    CORRADE_COMPARE((*a).a, 32);
    CORRADE_COMPARE((*ca).a, 32);
}

void OptionalTest::debug() {
    std::stringstream out;
    Debug{&out} << Containers::optional(42) << Optional<int>{} << Containers::NullOpt;
    CORRADE_COMPARE(out.str(), "42 Containers::NullOpt Containers::NullOpt\n");
}

void OptionalTest::vectorOfMovableOptional() {
    std::vector<Optional<Movable>> vec;

    vec.emplace_back(23);
    vec.emplace_back();
    vec.emplace_back(Containers::NullOpt);
    vec.push_back(Movable{67});

    CORRADE_COMPARE(vec[0]->a, 23);
    CORRADE_VERIFY(!vec[1]);
    CORRADE_VERIFY(!vec[2]);
    CORRADE_COMPARE(vec[3]->a, 67);
}

}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::OptionalTest)
