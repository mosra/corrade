/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <sstream>

#include "Corrade/Containers/MoveReference.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/TypeTraits.h" /* CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct MoveReferenceTest: TestSuite::Tester {
    explicit MoveReferenceTest();

    void construct();
    void constructConst();
    void constructDefault();
    void constructCopy();
    void constructFromLvalue();
    void constructIncomplete();
    void constructDerived();

    void convertToReference();

    void access();

    void debug();
};

MoveReferenceTest::MoveReferenceTest() {
    addTests({&MoveReferenceTest::construct,
              &MoveReferenceTest::constructConst,
              &MoveReferenceTest::constructDefault,
              &MoveReferenceTest::constructCopy,
              &MoveReferenceTest::constructFromLvalue,
              &MoveReferenceTest::constructIncomplete,
              &MoveReferenceTest::constructDerived,

              &MoveReferenceTest::convertToReference,

              &MoveReferenceTest::access,

              &MoveReferenceTest::debug});
}

void MoveReferenceTest::construct() {
    int a = 3;

    MoveReference<int> b = std::move(a);
    CORRADE_COMPARE(&b.get(), &a);
    CORRADE_COMPARE(b, 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<MoveReference<int>, int&&>::value);
    CORRADE_VERIFY(std::is_trivially_destructible<MoveReference<int>>::value);
}

void MoveReferenceTest::constructConst() {
    int a = 3;

    MoveReference<const int> cb = std::move(a);
    CORRADE_COMPARE(&cb.get(), &a);
    CORRADE_COMPARE(cb, 3);
}

void MoveReferenceTest::constructDefault() {
    CORRADE_VERIFY(std::is_constructible<MoveReference<int>, int&&>::value);
    CORRADE_VERIFY(!std::is_default_constructible<MoveReference<int>>::value);
}

void MoveReferenceTest::constructCopy() {
    int a = 3;
    MoveReference<int> b = std::move(a);
    CORRADE_COMPARE(b, 3);

    MoveReference<int> c = std::move(b);
    CORRADE_COMPARE(c, 3);

    int aa = 33;
    MoveReference<int> d = std::move(aa);
    CORRADE_COMPARE(d, 33);

    d = c;
    CORRADE_COMPARE(d, 3);

    CORRADE_VERIFY(std::is_copy_constructible<MoveReference<int>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<MoveReference<int>>::value);
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    CORRADE_VERIFY(std::is_trivially_copy_constructible<MoveReference<int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<MoveReference<int>>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<MoveReference<int>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<MoveReference<int>>::value);
}

void MoveReferenceTest::constructFromLvalue() {
    CORRADE_VERIFY(!std::is_constructible<MoveReference<int>, int&>::value);
    CORRADE_VERIFY(std::is_constructible<MoveReference<int>, int>::value);
    CORRADE_VERIFY(std::is_constructible<MoveReference<int>, int&&>::value);
    CORRADE_VERIFY(!std::is_constructible<MoveReference<const int>, const int&>::value);
    CORRADE_VERIFY(std::is_constructible<MoveReference<const int>, const int>::value);
    CORRADE_VERIFY(!std::is_constructible<MoveReference<const int>, const int&>::value);
}

void MoveReferenceTest::constructIncomplete() {
    struct Foo;
    int a = 5;
    Foo& refA = reinterpret_cast<Foo&>(a);

    MoveReference<Foo> b{static_cast<Foo&&>(refA)};
    MoveReference<Foo> c = b;
    CORRADE_COMPARE(&b.get(), static_cast<void*>(&a));
    CORRADE_COMPARE(&c.get(), static_cast<void*>(&a));
}

void MoveReferenceTest::constructDerived() {
    struct Base {
        explicit Base(int a): a{a} {}
        int a;
    };
    struct Derived: Base {
        explicit Derived(int a): Base{a} {}
    };

    Derived a{42};
    MoveReference<Derived> b = std::move(a);
    MoveReference<Base> c = b;
    CORRADE_COMPARE(c->a, 42);

    CORRADE_VERIFY(std::is_nothrow_constructible<MoveReference<Base>, MoveReference<Derived>>::value);

    CORRADE_VERIFY(std::is_constructible<MoveReference<Base>, Derived&&>::value);
    CORRADE_VERIFY(!std::is_constructible<MoveReference<Derived>, Base&&>::value);
    CORRADE_VERIFY(std::is_constructible<MoveReference<Base>, MoveReference<Derived>>::value);
    CORRADE_VERIFY(!std::is_constructible<MoveReference<Derived>, MoveReference<Base>>::value);
}

void MoveReferenceTest::convertToReference() {
    int a = 32;
    MoveReference<int> b = std::move(a);

    int& c = b;
    const int& cc = b;
    CORRADE_COMPARE(c, 32);
    CORRADE_COMPARE(cc, 32);
}

void MoveReferenceTest::access() {
    struct Foo {
        int a;
    };

    Foo a{15};
    MoveReference<Foo> b = std::move(a);
    CORRADE_COMPARE(b->a, 15);
    CORRADE_COMPARE((*b).a, 15);
    CORRADE_COMPARE(b.get().a, 15);
}

void MoveReferenceTest::debug() {
    int a = 18;
    MoveReference<int> b = std::move(a);

    std::ostringstream out;
    Debug{&out} << b;
    CORRADE_COMPARE(out.str(), "18\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::MoveReferenceTest)
