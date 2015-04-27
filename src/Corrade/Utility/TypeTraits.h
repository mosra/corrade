#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h
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

/** @file
 * @brief Macros @ref CORRADE_HAS_TYPE(), alias @ref Corrade::Utility::IsIterable
 */

#include <iterator> /* for std::begin() in libc++ */
#include <utility>

#include "Corrade/compatibility.h"
#include "Corrade/configure.h"

namespace Corrade { namespace Utility {

/** @hideinitializer
@brief Macro for creating traits class that checks for type expression validity
@param typeExpression   Type expression to check
@param className        Resulting class name

Defines a traits class checking whether @p typeExpression is valid. You can use
`T` to reference the type which is being checked. The defined class is then
implicitly convertible to `bool` holding the result.

Usage examples: checking for presence of `const_iterator` member type:
@code
CORRADE_HAS_TYPE(HasKeyType, typename T::key_type);

static_assert(HasKeyType<std::map<int, int>>{}, "");
static_assert(!HasKeyType<std::vector<int>>{}, "");
@endcode

Checking for presence of `size()` member function:
@code
CORRADE_HAS_TYPE(HasSize, decltype(std::declval<T>().size()));

static_assert(HasSize<std::vector<int>>{}, "");
static_assert(!HasSize<std::tuple<int, int>>{}, "");
@endcode
*/
/* Two overloaded get() functions return type of different size. Templated
   get() is used when T has given attribute, non-templated otherwise. Bool
   value then indicates whether the templated version was called or not. */
#ifndef CORRADE_GCC44_COMPATIBILITY
#define CORRADE_HAS_TYPE(className, typeExpression)                         \
template<class U> class className {                                         \
    template<class T> static char get(T&&, typeExpression* = nullptr);      \
    static short get(...);                                                  \
    public:                                                                 \
        enum: bool { Value = sizeof(get(std::declval<U>())) == sizeof(char) }; \
        constexpr operator bool() const { return Value; } \
}
#else
#define CORRADE_HAS_TYPE(className, typeExpression)                         \
template<class U> class className {                                         \
    template<class T> static char get(T&&, typeExpression* = nullptr);      \
    static short get(...);                                                  \
    static U&& reference();                                                 \
    public:                                                                 \
        enum: bool { Value = sizeof(get(reference())) == sizeof(char) };    \
        constexpr operator bool() const { return Value; } \
}
#endif

namespace Implementation {
    #ifndef CORRADE_GCC44_COMPATIBILITY
    CORRADE_HAS_TYPE(HasMemberBegin, decltype(std::declval<T>().begin()));
    CORRADE_HAS_TYPE(HasMemberEnd, decltype(std::declval<T>().end()));
    #else
    CORRADE_HAS_TYPE(HasMemberBegin, decltype((*static_cast<const T*>(nullptr)).begin()));
    CORRADE_HAS_TYPE(HasMemberEnd, decltype((*static_cast<const T*>(nullptr)).end()));
    #endif
    /** @todo Re-enable these for GCC 4.7 when I find some workaround */
    #ifndef CORRADE_GCC47_COMPATIBILITY
    CORRADE_HAS_TYPE(HasBegin, decltype(begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasEnd, decltype(end(std::declval<T>())));
    #endif
    #ifndef CORRADE_GCC45_COMPATIBILITY
    CORRADE_HAS_TYPE(HasStdBegin, decltype(std::begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasStdEnd, decltype(std::end(std::declval<T>())));
    #endif
}

/**
@brief Traits class for checking whether given class is iterable

Equivalent to `std::true_type` if the class is has either `begin()` and `end()`
members, is usable with free `begin()`/`end()` functions or has
`std::begin()`/`std::end()` overloads. Otherwise equivalent to
`std::false_type`.
*/
#if !defined(CORRADE_GCC46_COMPATIBILITY) && !defined(CORRADE_MSVC2013_COMPATIBILITY)
template<class T> using IsIterable = std::integral_constant<bool,
    (Implementation::HasMemberBegin<T>{} ||
    #ifndef CORRADE_GCC47_COMPATIBILITY
    Implementation::HasBegin<T>{} ||
    #endif
    Implementation::HasStdBegin<T>{}) && (Implementation::HasMemberEnd<T>{} ||
    #ifndef CORRADE_GCC47_COMPATIBILITY
    Implementation::HasEnd<T>{} ||
    #endif
    Implementation::HasStdEnd<T>{})>;
#elif defined(CORRADE_MSVC2013_COMPATIBILITY)
template<class T> struct IsIterable: public std::integral_constant<bool,
    (Implementation::HasMemberBegin<T>::Value || Implementation::HasBegin<T>::Value || Implementation::HasStdBegin<T>::Value) &&
    (Implementation::HasMemberEnd<T>::Value || Implementation::HasEnd<T>::Value || Implementation::HasStdEnd<T>::Value)> {};
#elif !defined(CORRADE_GCC45_COMPATIBILITY)
template<class T> struct IsIterable: public std::integral_constant<bool, (Implementation::HasMemberBegin<T>{} || Implementation::HasStdBegin<T>{}) && (Implementation::HasMemberEnd<T>{} || Implementation::HasStdEnd<T>{})> {};
#else
template<class T> struct IsIterable: public std::integral_constant<bool, Implementation::HasMemberBegin<T>::Value && Implementation::HasMemberEnd<T>::Value> {};
#endif

}}

#endif
