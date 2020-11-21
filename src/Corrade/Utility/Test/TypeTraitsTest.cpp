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

#include <iterator>
#include <map>
#include <string>
#include <type_traits>
#include <valarray>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/LinkedList.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/TypeTraits.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TypeTraitsTest: TestSuite::Tester {
    explicit TypeTraitsTest();

    void longDoubleSize();
    void isTriviallyTraitsSupported();

    void hasType();
    void hasTypeComma();

    void isIterableMember();
    void isIterableFreeStd();
    void isIterableFree();
    void isIterableNot();

    void isStringLike();
    void isStringLikeNot();
};

TypeTraitsTest::TypeTraitsTest() {
    addTests({&TypeTraitsTest::longDoubleSize,
              &TypeTraitsTest::isTriviallyTraitsSupported,

              &TypeTraitsTest::hasType,
              &TypeTraitsTest::hasTypeComma,

              &TypeTraitsTest::isIterableMember,
              &TypeTraitsTest::isIterableFreeStd,
              &TypeTraitsTest::isIterableFree,
              &TypeTraitsTest::isIterableNot,

              &TypeTraitsTest::isStringLike,
              &TypeTraitsTest::isStringLikeNot});
}

void TypeTraitsTest::longDoubleSize() {
    #ifdef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
    Debug{} << "long double has the same size as double";
    {
        #if defined(CORRADE_TARGET_EMSCRIPTEN) && __LDBL_DIG__ != __DBL_DIG__
        CORRADE_EXPECT_FAIL("Emscripten's long double is 80-bit, but doesn't actually have an 80-bit precision, so it's treated as 64-bit.");
        #endif
        CORRADE_COMPARE(sizeof(long double), sizeof(double));
    }
    #else
    Debug{} << "long double doesn't have the same size as double";
    CORRADE_COMPARE_AS(sizeof(long double), sizeof(double),
        TestSuite::Compare::Greater);
    #endif
}

void TypeTraitsTest::isTriviallyTraitsSupported() {
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    Debug{} << "std::is_trivially_* traits supported";
    CORRADE_VERIFY((std::is_trivially_constructible<int, int>::value));
    CORRADE_VERIFY(std::is_trivially_default_constructible<int>::value);
    CORRADE_VERIFY(std::is_trivially_copy_constructible<int>::value);
    CORRADE_VERIFY(std::is_trivially_move_constructible<int>::value);
    CORRADE_VERIFY((std::is_trivially_assignable<int&, int>::value));
    CORRADE_VERIFY(std::is_trivially_copy_assignable<int>::value);
    CORRADE_VERIFY(std::is_trivially_move_assignable<int>::value);
    #else
    Debug{} << "std::is_trivially_* traits not supported";
    /* std::has_trivial_copy_constructor etc. emits a deprecation warning on
       GCC 5+, so using the builtins instead. See the macro docs for details */
    CORRADE_VERIFY(__has_trivial_constructor(int));
    CORRADE_VERIFY(__has_trivial_copy(int));
    CORRADE_VERIFY(__has_trivial_assign(int));
    #endif
}

CORRADE_HAS_TYPE(HasKeyType, typename T::key_type);
CORRADE_HAS_TYPE(HasSize, decltype(std::declval<T>().size()));
CORRADE_HAS_TYPE(HasBegin, decltype(std::begin(std::declval<T>())));

void TypeTraitsTest::hasType() {
    /* Member type */
    CORRADE_VERIFY((HasKeyType<std::map<int, int>>::value));
    CORRADE_VERIFY(!HasKeyType<std::vector<int>>::value);

    /* Member function */
    CORRADE_VERIFY(HasSize<std::vector<int>>::value);
    CORRADE_VERIFY(!(HasSize<std::pair<int, int>>::value));

    /* Non-member function */
    CORRADE_VERIFY(HasBegin<std::string>::value);
    CORRADE_VERIFY(!HasBegin<int*>::value);
}

CORRADE_HAS_TYPE(IteratorIsPointer, typename std::enable_if<std::is_same<decltype(std::declval<const T>().begin()), const typename T::value_type*>::value>::type);

void TypeTraitsTest::hasTypeComma() {
    /* Longer expressions with a comma should work too */
    CORRADE_VERIFY(!IteratorIsPointer<std::vector<int>>::value);
    CORRADE_VERIFY(IteratorIsPointer<std::initializer_list<int>>::value);
}

struct Type {};
struct NonIterableType {};
#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif
int* begin(Type) { return nullptr; }
int* end(Type) { return nullptr; }
#ifdef __clang__
#pragma GCC diagnostic pop
#endif
struct LinkedListItem: Containers::LinkedListItem<LinkedListItem> {};

void TypeTraitsTest::isIterableMember() {
    /* STL types with begin()/end() members */
    CORRADE_VERIFY(IsIterable<std::vector<int>>{});
    CORRADE_VERIFY(IsIterable<std::string>{});

    /* Corrade types */
    CORRADE_VERIFY(IsIterable<Containers::Array<int>>{});
}

void TypeTraitsTest::isIterableFreeStd() {
    CORRADE_VERIFY(IsIterable<std::valarray<int>>{});
    CORRADE_VERIFY(IsIterable<std::initializer_list<int>>{});
}

void TypeTraitsTest::isIterableFree() {
    /* Types with out-of-class begin()/end() */
    CORRADE_VERIFY(IsIterable<Type>{});

    /* Corrade types */
    CORRADE_VERIFY(IsIterable<Containers::LinkedList<LinkedListItem>>{});
}

void TypeTraitsTest::isIterableNot() {
    CORRADE_VERIFY(!IsIterable<int>{});
    CORRADE_VERIFY(!IsIterable<NonIterableType>{});
}

void TypeTraitsTest::isStringLike() {
    CORRADE_VERIFY(IsStringLike<std::string>{});
    CORRADE_VERIFY(IsStringLike<const std::wstring&>{});
    CORRADE_VERIFY(IsStringLike<std::u32string&&>{});

    CORRADE_VERIFY(IsStringLike<Containers::String&>{});
    CORRADE_VERIFY(IsStringLike<const Containers::MutableStringView&>{});
    CORRADE_VERIFY(IsStringLike<Containers::StringView>{});
}

void TypeTraitsTest::isStringLikeNot() {
    CORRADE_VERIFY(!IsStringLike<std::vector<int>>{});
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TypeTraitsTest)
