#ifndef Corrade_Utility_Move_h
#define Corrade_Utility_Move_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include <type_traits>

#include "Corrade/configure.h"

/** @file
 * @brief Function @ref Corrade::Utility::forward(), @ref Corrade::Utility::move(), @ref Corrade::Utility::swap()
 * @m_since_latest
 */

namespace Corrade { namespace Utility {

/* forward() and move() are basically copied from libstdc++'s bits/move.h */

/**
@brief Forward an l-value
@m_since_latest

Returns @cpp static_cast<T&&>(t) @ce. Equivalent to @ref std::forward(), which
is used to implement perfect forwarding, but without the
@cpp #include <utility> @ce dependency and guaranteed to be @cpp constexpr @ce
even in C++11.
@see @ref move(), @ref swap()
*/
template<class T> constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

/**
@brief Forward an r-value
@m_since_latest

Returns @cpp static_cast<T&&>(t) @ce. Equivalent to @ref std::forward(), which
is used to implement perfect forwarding, but without the
@cpp #include <utility> @ce dependency and guaranteed to be @cpp constexpr @ce
even in C++11.
@see @ref move(), @ref swap()
*/
template<class T> constexpr T&& forward(typename std::remove_reference<T>::type&& t) noexcept {
    /* bits/move.h in libstdc++ has this and it makes sense to have it here
       also, although I can't really explain what accidents it prevents */
    static_assert(!std::is_lvalue_reference<T>::value, "T can't be a lvalue reference");
    return static_cast<T&&>(t);
}

/**
@brief Convert a value to an r-value
@m_since_latest

Returns @cpp static_cast<typename std::remove_reference<T>::type&&>(t) @ce.
Equivalent to @m_class{m-doc-external} [std::move()](https://en.cppreference.com/w/cpp/utility/move),
but without the @cpp #include <utility> @ce dependency and guaranteed to be
@cpp constexpr @ce even in C++11.
@see @ref forward(), @ref swap()
*/
template<class T> constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

/**
@brief Swap two values
@m_since_latest

Swaps two values. Equivalent to @ref std::swap(), but without the
@cpp #include <utility> @ce dependency, and without the internals delegating to
@m_class{m-doc-external} [std::move()](https://en.cppreference.com/w/cpp/utility/move),
hurting debug performance. In order to keep supporting custom specializations,
the usage pattern should be similar to the standard utility, i.e. with
@cpp using Utility::swap @ce:

@snippet Utility.cpp swap
@see @ref forward(), @ref move()
*/
/* The common_type is to prevent ambiguity with (also unrestricted) std::swap.
   See MoveTest::swapStlTypesAdlAmbiguity() and swapUtilityTypesAdlAmbiguity()
   for details. Besides resolving ambiguity, in practice it means that
   std::swap() will get preferred over this overload in all cases where ADL
   finds it due to a STL type being used. Which means potentially slightly
   worse debug perf than if this overload was used for those too, due to
   libstdc++, libc++ and MSVC STL all delegating to move() instead of inlining
   it. */
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T> void swap(T& a, T& b) noexcept(...);
#else
template<class T> void swap(T& a, typename std::common_type<T>::type& b) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
    /* "Deinlining" move() for nicer debug perf */
    T tmp = static_cast<T&&>(a);
    a = static_cast<T&&>(b);
    b = static_cast<T&&>(tmp);
}
#endif

/**
@brief Swap two arrays
@m_since_latest

Does the same as @ref swap(T&, T&), but for every array element.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<std::size_t size, class T> void swap(T(&a)[size], T(&b)[size]) noexcept(...);
#else
template<std::size_t size, class T> void swap(T(&a)[size], typename std::common_type<T>::type(&b)[size]) noexcept(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value) {
    for(std::size_t i = 0; i != size; ++i) {
        /* "Deinlining" move() for nicer debug perf */
        T tmp = static_cast<T&&>(a[i]);
        a[i] = static_cast<T&&>(b[i]);
        b[i] = static_cast<T&&>(tmp);
    }
}
#endif

}}

#endif
