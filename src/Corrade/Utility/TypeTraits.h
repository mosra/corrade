#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h
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

/** @file
 * @brief Macros @ref CORRADE_HAS_TYPE(), alias @ref Corrade::Utility::IsIterable
 */

#include <iterator> /* for std::begin() in libc++ */
#include <utility>

#include "Corrade/configure.h"

namespace Corrade { namespace Utility {

/** @hideinitializer
@brief Macro for creating traits class that checks for type expression validity
@param typeExpression   Type expression to check
@param className        Resulting class name

Defines a traits class checking whether @p typeExpression is valid. You can use
@cpp T @ce to reference the type which is being checked. The defined class is
then implicitly convertible to `bool` holding the result.

Usage examples: checking for presence of @cpp const_iterator @ce member type:

@snippet Utility.cpp CORRADE_HAS_TYPE-type

Checking for presence of @cpp size() @ce member function:

@snippet Utility.cpp CORRADE_HAS_TYPE-function
*/
/* Two overloaded get() functions return type of different size. Templated
   get() is used when T has given attribute, non-templated otherwise. Bool
   value then indicates whether the templated version was called or not. */
#define CORRADE_HAS_TYPE(className, typeExpression)                         \
template<class U> class className {                                         \
    template<class T> static char get(T&&, typeExpression* = nullptr);      \
    static short get(...);                                                  \
    public:                                                                 \
        enum: bool { value = sizeof(get(std::declval<U>())) == sizeof(char) }; \
}

namespace Implementation {
    CORRADE_HAS_TYPE(HasMemberBegin, decltype(std::declval<T>().begin()));
    CORRADE_HAS_TYPE(HasMemberEnd, decltype(std::declval<T>().end()));
    CORRADE_HAS_TYPE(HasBegin, decltype(begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasEnd, decltype(end(std::declval<T>())));
    CORRADE_HAS_TYPE(HasStdBegin, decltype(std::begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasStdEnd, decltype(std::end(std::declval<T>())));
}

/**
@brief Traits class for checking whether given type is iterable

Equivalent to @ref std::true_type if the class is has either @cpp begin() @ce /
@cpp end() @ce members, is usable with free @cpp begin() @ce / @cpp end() @ce
functions or has @ref std::begin() / @ref std::end() overloads. Otherwise
equivalent to @ref std::false_type.
@todoc use the ellipsis macro once m.css has it
*/
/* When using {}, MSVC 2015 complains that even the explicitly defaulted
   constructor doesn't exist */
template<class T> using IsIterable = std::integral_constant<bool,
    #ifndef DOXYGEN_GENERATING_OUTPUT
    (Implementation::HasMemberBegin<T>::value ||
     Implementation::HasBegin<T>::value ||
     Implementation::HasStdBegin<T>::value) &&
    (Implementation::HasMemberEnd<T>::value ||
     Implementation::HasEnd<T>::value ||
     Implementation::HasStdEnd<T>::value)
    #else
    implementation-specific
    #endif
    >;

}}

#endif
