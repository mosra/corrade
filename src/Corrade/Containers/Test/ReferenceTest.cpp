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

#include "Corrade/Containers/Reference.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace {

struct IntRef {
    constexpr IntRef(const int& a): a{&a} {}

    const int* a;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct ReferenceConverter<const int, IntRef> {
    constexpr static Reference<const int> from(IntRef other) {
        return *other.a;
    }

    constexpr static IntRef to(Reference<const int> other) {
        return IntRef{*other};
    }
};

}

namespace Test { namespace {

struct ReferenceTest: TestSuite::Tester {
    explicit ReferenceTest();

    void construct();
    void constructConst();
    void constructDefault();
    void constructCopy();
    void constructFromRvalue();
    void constructIncomplete();
    void constructDerived();
    void convert();

    void convertToReference();
    void convertToConst();

    void access();
    void unambiguousOverloadWithTypeConvertibleFromInt();

    void debug();
};

ReferenceTest::ReferenceTest() {
    addTests({&ReferenceTest::construct,
              &ReferenceTest::constructConst,
              &ReferenceTest::constructDefault,
              &ReferenceTest::constructCopy,
              &ReferenceTest::constructFromRvalue,
              &ReferenceTest::constructIncomplete,
              &ReferenceTest::constructDerived,
              &ReferenceTest::convert,

              &ReferenceTest::convertToReference,
              &ReferenceTest::convertToConst,
              &ReferenceTest::access,
              &ReferenceTest::unambiguousOverloadWithTypeConvertibleFromInt,

              &ReferenceTest::debug});
}

/* Needs to be here in order to use it in constexpr */
constexpr int Int = 3;

void ReferenceTest::construct() {
    int a = 3;

    Reference<int> b = a;
    CORRADE_COMPARE(&b.get(), &a);
    CORRADE_COMPARE(b, 3);

    constexpr Reference<const int> cb = Int;
    CORRADE_COMPARE(&cb.get(), &Int);
    CORRADE_COMPARE(cb, 3);

    CORRADE_VERIFY((std::is_nothrow_constructible<Reference<int>, int&>::value));
    #if (!defined(__GNUC__) && !defined(__clang__)) || __GNUC__ >= 5 || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 5)
    /* https://gcc.gnu.org/onlinedocs/gcc-4.9.2/libstdc++/manual/manual/status.html#status.iso.2011
       vs https://gcc.gnu.org/onlinedocs/gcc-5.5.0/libstdc++/manual/manual/status.html#status.iso.2011.
       Also, until GCC 7 it's not possible to detect what libstdc++ version
       is used when on Clang, because __GLIBCXX__ is a RELEASE DATE that
       has absolutely no relation to the version and is completely useless:
       https://gcc.gnu.org/onlinedocs/libstdc++/manual/abi.html#abi.versioning.__GLIBCXX__
       So in case of Clang I'm trying to use _GLIBCXX_RELEASE, but that
       cuts off libstdc++ 5 or libstdc++ 6 because these don't have this
       macro yet. */
    CORRADE_VERIFY(std::is_trivially_destructible<Reference<int>>::value);
    #endif
}

void ReferenceTest::constructConst() {
    int a = 3;

    Reference<const int> cb = a;
    CORRADE_COMPARE(&cb.get(), &a);
    CORRADE_COMPARE(cb, 3);
}

void ReferenceTest::constructDefault() {
    CORRADE_VERIFY((std::is_constructible<Reference<int>, int&>::value));
    CORRADE_VERIFY(!std::is_default_constructible<Reference<int>>::value);
}

void ReferenceTest::constructCopy() {
    int a = 3;
    Reference<int> b = a;
    CORRADE_COMPARE(b, 3);

    Reference<int> c = b;
    CORRADE_COMPARE(c, 3);

    int aa = 33;
    Reference<int> d = aa;
    CORRADE_COMPARE(d, 33);

    d = c;
    CORRADE_COMPARE(d, 3);

    constexpr Reference<const int> cb = Int;
    CORRADE_COMPARE(cb, 3);

    constexpr Reference<const int> cc = cb;
    CORRADE_COMPARE(cc, 3);

    CORRADE_VERIFY(std::is_copy_constructible<Reference<int>>::value);
    CORRADE_VERIFY(std::is_copy_assignable<Reference<int>>::value);
    #if (!defined(__GNUC__) && !defined(__clang__)) || __GNUC__ >= 5 || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 5)
    CORRADE_VERIFY(std::is_trivially_copy_constructible<Reference<int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Reference<int>>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<Reference<int>>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<Reference<int>>::value);
}

void ReferenceTest::constructFromRvalue() {
    /* Neither of the ones below should compile */
    //Reference<int> a{1337};
    //Reference<const int> b{42};

    CORRADE_VERIFY((std::is_constructible<Reference<int>, int&>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<int>, int>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<int>, int&&>::value));
    CORRADE_VERIFY((std::is_constructible<Reference<const int>, const int&>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<const int>, const int>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<const int>, const int&&>::value));
}

void ReferenceTest::constructIncomplete() {
    struct Foo;
    int a = 5;
    int& refA = a;
    Foo& refFoo = reinterpret_cast<Foo&>(refA);

    Reference<Foo> b{refFoo};
    Reference<Foo> c = b;
    CORRADE_COMPARE(&b.get(), static_cast<void*>(&a));
    CORRADE_COMPARE(&c.get(), static_cast<void*>(&a));
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(int a): a{a} {}
    int a;
};
struct Derived: Base {
    constexpr Derived(int a): Base{a} {}
};
constexpr Derived DerivedInstance{42};

void ReferenceTest::constructDerived() {
    Derived a{42};
    Reference<Derived> b = a;
    Reference<Base> c = b;
    CORRADE_COMPARE(c->a, 42);

    constexpr Reference<const Derived> cb = DerivedInstance;
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Reference<const Base> cc = cb;
    CORRADE_COMPARE(cc->a, 42);

    CORRADE_VERIFY((std::is_nothrow_constructible<Reference<Base>, Reference<Derived>>::value));

    CORRADE_VERIFY((std::is_constructible<Reference<Base>, Derived&>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<Derived>, Base&>::value));
    CORRADE_VERIFY((std::is_constructible<Reference<Base>, Reference<Derived>>::value));
    CORRADE_VERIFY(!(std::is_constructible<Reference<Derived>, Reference<Base>>::value));
}

void ReferenceTest::convert() {
    const int a = 1348;
    IntRef b = a;
    CORRADE_COMPARE(*b.a, 1348);

    Reference<const int> c = b; /* implicit conversion *is* allowed */
    CORRADE_COMPARE(*c, 1348);

    IntRef d = c; /* implicit conversion *is* allowed */
    CORRADE_COMPARE(*d.a, 1348);

    constexpr IntRef cb = Int;
    CORRADE_COMPARE(*cb.a, 3);

    constexpr Reference<const int> cc = cb;
    CORRADE_COMPARE(*cc, 3);

    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Pointer dereference not constexpr on MSVC 2015(?) */
    #endif
    IntRef cd = cc;
    CORRADE_COMPARE(*cd.a, 3);

    /* Conversion from a different type is not allowed */
    CORRADE_VERIFY((std::is_convertible<Reference<const int>, IntRef>::value));
    CORRADE_VERIFY(!(std::is_convertible<Reference<const float>, IntRef>::value));
    CORRADE_VERIFY((std::is_convertible<IntRef, Reference<const int>>::value));
    CORRADE_VERIFY(!(std::is_convertible<IntRef, Reference<const float>>::value));
}

void ReferenceTest::convertToReference() {
    int a = 32;
    Reference<int> b = a;

    int& c = b;
    const int& cc = b;
    CORRADE_COMPARE(c, 32);
    CORRADE_COMPARE(cc, 32);

    constexpr Reference<const int> cb = Int;
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Pointer dereference not constexpr on MSVC 2015(?) */
    #endif
    const int& ccc = cb;
    CORRADE_COMPARE(ccc, 3);
}

void ReferenceTest::convertToConst() {
    int a = 18;
    Reference<int> b = a;

    Reference<const int> c = b;
    CORRADE_COMPARE(c, 18);
}

constexpr struct Foo {
    int a;
} FooInstance{15};

void ReferenceTest::access() {
    Foo a{15};
    Reference<Foo> b = a;
    CORRADE_COMPARE(b->a, 15);
    CORRADE_COMPARE((*b).a, 15);
    CORRADE_COMPARE(b.get().a, 15);

    constexpr Reference<const Foo> cb = FooInstance;
    CORRADE_COMPARE(cb->a, 15);
    CORRADE_COMPARE((*cb).a, 15);
    CORRADE_COMPARE(cb.get().a, 15);
}

//struct ConvertibleFromInt {
//   ConvertibleFromInt(int) {}
//;
//
//void foo(Reference<int>);
//void foo(ConvertibleFromInt) {}

void ReferenceTest::unambiguousOverloadWithTypeConvertibleFromInt() {
    CORRADE_SKIP("Not implemented yet.");

    /* The function below does not compile due to ambiguous overload error, see
       https://cplusplus.github.io/LWG/issue2993 and libc++ implementation in
       https://patchwork.ozlabs.org/patch/929968/ (too complex for my taste) */
    //foo(0);
}

void ReferenceTest::debug() {
    int a = 18;
    Reference<int> b = a;

    std::ostringstream out;
    Debug{&out} << b;
    CORRADE_COMPARE(out.str(), "18\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ReferenceTest)
