#ifndef Corrade_Utility_Math_h
#define Corrade_Utility_Math_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
 * @brief Function @ref Corrade::Utility::min(), @ref Corrade::Utility::max()
 * @m_since_latest
 */

#include "Corrade/configure.h"

namespace Corrade { namespace Utility {

/**
@brief Minimum
@m_since_latest

The main purpose of this function is to be used in places where having to
@cpp #include <algorithm> @ce to get @ref std::min() would be an unnecessary
overkill. It's primarily meant to be used on builtin scalar types, thus the
arguments are taken by value and not by reference.

<em>NaN</em>s passed in the @p value parameter are propagated.
@see @ref max()
*/
template<class T> constexpr T min(T value, T min) {
    /* Implementation matching the one in Magnum */
    return min < value ? min : value;
}

/**
@brief Maximum
@m_since_latest

The main purpose of this function is to be used in places where having to
@cpp #include <algorithm> @ce to get @ref std::max() would be an unnecessary
overkill. It's primarily meant to be used on builtin scalar types, thus the
arguments are taken by value and not by reference.

<em>NaN</em>s passed in the @p value parameter are propagated.
@see @ref min()
*/
template<class T> constexpr T max(T value, T max) {
    /* Implementation matching the one in Magnum */
    return value < max ? max : value;
}

}}

#endif
