/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/LinkedList.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/TypeTraits.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TypeTraitsTest: TestSuite::Tester {
    explicit TypeTraitsTest();

    void hasType();

    void isIterableMember();
    void isIterableFreeStd();
    void isIterableFree();
    void isIterableNot();

    void isStringLike();
    void isStringLikeNot();
};

TypeTraitsTest::TypeTraitsTest() {
    addTests({&TypeTraitsTest::hasType,

              &TypeTraitsTest::isIterableMember,
              &TypeTraitsTest::isIterableFreeStd,
              &TypeTraitsTest::isIterableFree,
              &TypeTraitsTest::isIterableNot,

              &TypeTraitsTest::isStringLike,
              &TypeTraitsTest::isStringLikeNot});
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
    CORRADE_VERIFY(IsStringLike<std::wstring>{});
    CORRADE_VERIFY(IsStringLike<std::u32string>{});
}

void TypeTraitsTest::isStringLikeNot() {
    CORRADE_VERIFY(!IsStringLike<std::vector<int>>{});
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TypeTraitsTest)
