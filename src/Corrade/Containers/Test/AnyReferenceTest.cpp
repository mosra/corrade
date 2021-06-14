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

#include "Corrade/Containers/AnyReference.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/TypeTraits.h" /* CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED */

namespace Corrade { namespace Containers { namespace Test { namespace {

struct AnyReferenceTest: TestSuite::Tester {
    explicit AnyReferenceTest();

    void constructLvalue();
    void constructRvalue();
    void constructConst();
    void constructDefault();
    void constructCopy();
    void constructIncomplete();
    void constructDerived();

    void convertToReference();
    void convertToConst();

    void access();

    void debug();
};

AnyReferenceTest::AnyReferenceTest() {
    addTests({&AnyReferenceTest::constructLvalue,
              &AnyReferenceTest::constructRvalue,
              &AnyReferenceTest::constructConst,
              &AnyReferenceTest::constructDefault,
              &AnyReferenceTest::constructCopy,
              &AnyReferenceTest::constructIncomplete,
              &AnyReferenceTest::constructDerived,

              &AnyReferenceTest::convertToReference,
              &AnyReferenceTest::convertToConst,

              &AnyReferenceTest::access,

              &AnyReferenceTest::debug});
}

void AnyReferenceTest::constructLvalue() {
    int a = 3;

    AnyReference<int> b = a;
    CORRADE_COMPARE(&b.get(), &a);
    CORRADE_VERIFY(!b.isRvalue());
    CORRADE_COMPARE(b, 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<AnyReference<int>, int&>::value);
    CORRADE_VERIFY(std::is_trivially_destructible<AnyReference<int>>::value);
}

void AnyReferenceTest::constructRvalue() {
    int a = 3;

    AnyReference<int> b = Utility::move(a);
    CORRADE_COMPARE(&b.get(), &a);
    CORRADE_VERIFY(b.isRvalue());
    CORRADE_COMPARE(b, 3);

    CORRADE_VERIFY(std::is_nothrow_constructible<AnyReference<int>, int&&>::value);
    CORRADE_VERIFY(std::is_trivially_destructible<AnyReference<int>>::value);
}

void AnyReferenceTest::constructConst() {
    int a = 3;

    AnyReference<const int> cbl = a;
    AnyReference<const int> cbr = Utility::move(a);
    CORRADE_COMPARE(&cbl.get(), &a);
    CORRADE_COMPARE(&cbr.get(), &a);
    CORRADE_COMPARE(cbl, 3);
    CORRADE_VERIFY(!cbl.isRvalue());
    CORRADE_COMPARE(cbr, 3);
    CORRADE_VERIFY(cbr.isRvalue());
}

void AnyReferenceTest::constructDefault() {
    CORRADE_VERIFY(std::is_constructible<AnyReference<int>, int&>::value);
    CORRADE_VERIFY(!std::is_default_constructible<AnyReference<int>>::value);
}

void AnyReferenceTest::constructCopy() {
    int a = 3;
    AnyReference<int> bl = a;
    AnyReference<int> br = Utility::move(a);
    CORRADE_COMPARE(bl, 3);
    CORRADE_VERIFY(!bl.isRvalue());
    CORRADE_COMPARE(br, 3);
    CORRADE_VERIFY(br.isRvalue());

    AnyReference<int> cl = bl;
    AnyReference<int> cr = br;
    CORRADE_COMPARE(cl, 3);
    CORRADE_VERIFY(!cl.isRvalue());
    CORRADE_COMPARE(cr, 3);
    CORRADE_VERIFY(cr.isRvalue());

    int aa = 33;
    AnyReference<int> dl = aa;
    AnyReference<int> dr = aa;
    CORRADE_COMPARE(dl, 33);
    CORRADE_COMPARE(dr, 33);
    CORRADE_VERIFY(!dl.isRvalue());
    CORRADE_VERIFY(!dr.isRvalue());

    dl = cl;
    dr = cr;
    CORRADE_COMPARE(dl, 3);
    CORRADE_COMPARE(dr, 3);
    CORRADE_VERIFY(!dl.isRvalue());
    CORRADE_VERIFY(dr.isRvalue());

    CORRADE_VERIFY(std::is_copy_constructible<AnyReference<int>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<AnyReference<int>>::value);
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    CORRADE_VERIFY(std::is_trivially_copy_constructible<AnyReference<int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<AnyReference<int>>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<AnyReference<int>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<AnyReference<int>>::value);
}

void AnyReferenceTest::constructIncomplete() {
    struct Foo;
    int a = 5;
    int& refA = a;
    Foo& refFoo = reinterpret_cast<Foo&>(refA);

    AnyReference<Foo> bl{refFoo};
    AnyReference<Foo> br{Utility::move(refFoo)};
    AnyReference<Foo> cl = bl;
    AnyReference<Foo> cr = br;
    CORRADE_COMPARE(&bl.get(), static_cast<void*>(&a));
    CORRADE_VERIFY(!bl.isRvalue());
    CORRADE_COMPARE(&br.get(), static_cast<void*>(&a));
    CORRADE_VERIFY(br.isRvalue());
    CORRADE_COMPARE(&cl.get(), static_cast<void*>(&a));
    CORRADE_VERIFY(!cl.isRvalue());
    CORRADE_COMPARE(&cr.get(), static_cast<void*>(&a));
    CORRADE_VERIFY(cr.isRvalue());
}

void AnyReferenceTest::constructDerived() {
    struct Base {
        Base(int a): a{a} {}
        int a;
    };
    struct Derived: Base {
        Derived(int a): Base{a} {}
    };

    Derived a{42};
    AnyReference<Derived> bl = a;
    AnyReference<Derived> br = Utility::move(a);
    AnyReference<Base> cl = bl;
    AnyReference<Base> cr = br;
    CORRADE_COMPARE(cl->a, 42);
    CORRADE_VERIFY(!cl.isRvalue());
    CORRADE_COMPARE(cr->a, 42);
    CORRADE_VERIFY(cr.isRvalue());

    CORRADE_VERIFY(std::is_nothrow_constructible<AnyReference<Base>, AnyReference<Derived>>::value);

    CORRADE_VERIFY(std::is_constructible<AnyReference<Base>, Derived&>::value);
    CORRADE_VERIFY(std::is_constructible<AnyReference<Base>, Derived&&>::value);
    CORRADE_VERIFY(!std::is_constructible<AnyReference<Derived>, Base&>::value);
    CORRADE_VERIFY(!std::is_constructible<AnyReference<Derived>, Base&&>::value);
    CORRADE_VERIFY(std::is_constructible<AnyReference<Base>, AnyReference<Derived>>::value);
    CORRADE_VERIFY(!std::is_constructible<AnyReference<Derived>, AnyReference<Base>>::value);
}

void AnyReferenceTest::convertToReference() {
    int a = 32;
    AnyReference<int> bl = a;
    AnyReference<int> br = Utility::move(a);
    CORRADE_VERIFY(!bl.isRvalue());
    CORRADE_VERIFY(br.isRvalue());

    int& cl = bl;
    int& cr = br;
    const int& ccl = bl;
    const int& ccr = br;
    CORRADE_COMPARE(cl, 32);
    CORRADE_COMPARE(cr, 32);
    CORRADE_COMPARE(ccl, 32);
    CORRADE_COMPARE(ccr, 32);

    /* Implicit conversion to a r-value reference is not allowed to prevent
       accidents */
    CORRADE_VERIFY(std::is_convertible<AnyReference<int>&&, int&>::value);
    CORRADE_VERIFY(!std::is_convertible<AnyReference<int>&&, int&&>::value);
}

void AnyReferenceTest::convertToConst() {
    int a = 18;
    AnyReference<int> bl = a;
    AnyReference<int> br = Utility::move(a);

    AnyReference<const int> cl = bl;
    AnyReference<const int> cr = br;
    CORRADE_COMPARE(cl, 18);
    CORRADE_VERIFY(!cl.isRvalue());
    CORRADE_COMPARE(cr, 18);
    CORRADE_VERIFY(cr.isRvalue());
}

void AnyReferenceTest::access() {
    struct Foo {
        int a;
    };

    Foo a{15};
    AnyReference<Foo> b = a;
    CORRADE_COMPARE(b->a, 15);
    CORRADE_COMPARE((*b).a, 15);
    CORRADE_COMPARE(b.get().a, 15);
}

void AnyReferenceTest::debug() {
    int a = 18;
    AnyReference<int> b = a;

    std::ostringstream out;
    Debug{&out} << b;
    CORRADE_COMPARE(out.str(), "18\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::AnyReferenceTest)
