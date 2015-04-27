/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <valarray>
#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/LinkedList.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/TypeTraits.h"

namespace Corrade { namespace Utility { namespace Test {

struct TypeTraitsTest: TestSuite::Tester {
    explicit TypeTraitsTest();

    void hasType();
    void isIterable();
};

TypeTraitsTest::TypeTraitsTest() {
    addTests<TypeTraitsTest>({&TypeTraitsTest::hasType,
              &TypeTraitsTest::isIterable});
}

CORRADE_HAS_TYPE(HasKeyType, typename T::key_type);
#ifndef CORRADE_GCC44_COMPATIBILITY
CORRADE_HAS_TYPE(HasSize, decltype(std::declval<T>().size()));
#else
CORRADE_HAS_TYPE(HasSize, decltype((*static_cast<const T*>(nullptr)).size()));
#endif
#ifndef CORRADE_GCC45_COMPATIBILITY
CORRADE_HAS_TYPE(HasBegin, decltype(std::begin(std::declval<T>())));
#elif !defined(CORRADE_GCC44_COMPATIBILITY)
CORRADE_HAS_TYPE(HasSin, decltype(std::sin(std::declval<T>())));
#else
CORRADE_HAS_TYPE(HasSin, decltype(std::sin(*static_cast<const T*>(nullptr))));
#endif

void TypeTraitsTest::hasType() {
    /* Member type */
    CORRADE_VERIFY((HasKeyType<std::map<int, int>>{}));
    CORRADE_VERIFY(!HasKeyType<std::vector<int>>{});

    /* Member function */
    CORRADE_VERIFY(HasSize<std::vector<int>>{});
    CORRADE_VERIFY(!(HasSize<std::tuple<int, int>>{}));

    /* Non-member function */
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_VERIFY(HasBegin<std::string>{});
    CORRADE_VERIFY(!HasBegin<int*>{});
    #else
    CORRADE_VERIFY(HasSin<float>{});
    CORRADE_VERIFY(!HasSin<std::string>{});
    #endif
}

namespace {
    struct Type {};
    int* begin(Type);
    int* end(Type);
    struct LinkedListItem: Containers::LinkedListItem<LinkedListItem> {};
}

void TypeTraitsTest::isIterable() {
    /* Non-iterable types */
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_VERIFY(!IsIterable<int>{});
    #else
    CORRADE_VERIFY(!IsIterable<int>::value);
    #endif

    /* STL types with begin()/end() members */
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_VERIFY(IsIterable<std::vector<int>>{});
    CORRADE_VERIFY(IsIterable<std::string>{});
    #else
    CORRADE_VERIFY(IsIterable<std::vector<int>>::value);
    CORRADE_VERIFY(IsIterable<std::string>::value);
    #endif

    /* STL types with std::begin()/std::end() only */
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_VERIFY(IsIterable<std::valarray<int>>{});
    #else
    {
        CORRADE_EXPECT_FAIL("std::valarray isn't iterable in GCC 4.5 due to missing std::begin() etc overloads.");
        CORRADE_VERIFY(IsIterable<std::valarray<int>>::value);
    }
    #endif

    /* Types with out-of-class begin()/end() */
    {
        #ifdef CORRADE_GCC47_COMPATIBILITY
        CORRADE_EXPECT_FAIL("GCC 4.7 has broken SFINAE in this case (results in compile error when not present)");
        #endif
        #ifndef CORRADE_GCC45_COMPATIBILITY
        CORRADE_VERIFY(IsIterable<Type>{});
        #else
        CORRADE_VERIFY(IsIterable<Type>::value);
        #endif
    }

    /* Corrade types */
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_VERIFY(IsIterable<Containers::Array<int>>{});
    #else
    CORRADE_VERIFY(IsIterable<Containers::Array<int>>::value);
    #endif

    {
        #ifdef CORRADE_GCC47_COMPATIBILITY
        CORRADE_EXPECT_FAIL("GCC 4.7 has broken SFINAE in this case (results in compile error when not present)");
        #endif
        #ifndef CORRADE_GCC45_COMPATIBILITY
        CORRADE_VERIFY(IsIterable<Containers::LinkedList<LinkedListItem>>{});
        #else
        CORRADE_VERIFY(IsIterable<Containers::LinkedList<LinkedListItem>>::value);
        #endif
    }
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TypeTraitsTest)
