#ifndef Corrade_Utility_Move_h
#define Corrade_Utility_Move_h
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

#include <type_traits>

#include "Corrade/configure.h"

/** @file
 * @brief Function @ref Corrade::Utility::forward(), @ref Corrade::Utility::move()
 * @m_since_latest
 */

namespace Corrade { namespace Utility {

/* Basically a copy of bits/move.h from libstdc++ */

/**
@brief Forward an l-value
@m_since_latest

Returns @cpp static_cast<T&&>(t) @ce. Equivalent to @ref std::forward(), which
is used to implement perfect forwarding, but without the
@cpp #include <utility> @ce dependency and guaranteed to be @cpp constexpr @ce
even in C++11.
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
*/
template<class T> constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

}}

#endif
