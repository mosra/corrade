#ifndef Corrade_Utility_VisibilityMacros_h
#define Corrade_Utility_VisibilityMacros_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Macro @ref CORRADE_VISIBILITY_EXPORT, @ref CORRADE_VISIBILITY_IMPORT, @ref CORRADE_VISIBILITY_STATIC, @ref CORRADE_VISIBILITY_LOCAL
 */

#include "Corrade/configure.h"

/** @hideinitializer
 * @brief Export symbol into shared library
 */
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_EXPORT __declspec(dllexport)
#else
#define CORRADE_VISIBILITY_EXPORT __attribute__ ((visibility ("default")))
#endif

/** @hideinitializer
 * @brief Import symbol from shared library
 */
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_IMPORT __declspec(dllimport)
#else
#define CORRADE_VISIBILITY_IMPORT __attribute__ ((visibility ("default")))
#endif

/** @hideinitializer
 * @brief Public symbol in static library
 */
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_STATIC
#else
#define CORRADE_VISIBILITY_STATIC __attribute__ ((visibility ("default")))
#endif

/** @hideinitializer
@brief Local symbol

The symbol name will not be exported into shared or static library.
*/
#ifdef CORRADE_TARGET_WINDOWS
#define CORRADE_VISIBILITY_LOCAL
#else
#define CORRADE_VISIBILITY_LOCAL __attribute__ ((visibility ("hidden")))
#endif

#endif
