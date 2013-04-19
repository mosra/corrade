#ifndef Corrade_Utility_Visibility_h
#define Corrade_Utility_Visibility_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
 * @brief Macros @ref CORRADE_VISIBILITY_EXPORT, @ref CORRADE_VISIBILITY_IMPORT, @ref CORRADE_VISIBILITY_LOCAL
 */

/**
@brief Export symbol

The symbol name will be exported into shared library.
*/
#ifdef _WIN32
#define CORRADE_VISIBILITY_EXPORT __declspec(dllexport)
#else
#define CORRADE_VISIBILITY_EXPORT __attribute__ ((visibility ("default")))
#endif

/**
@brief Import symbol

The symbol name will be imported from shared library.
*/
#ifdef _WIN32
#define CORRADE_VISIBILITY_IMPORT __declspec(dllimport)
#else
#define CORRADE_VISIBILITY_IMPORT __attribute__ ((visibility ("default")))
#endif

/**
@brief Local symbol

The symbol name will not be exported into shared library.
*/
#ifdef _WIN32
#define CORRADE_VISIBILITY_LOCAL
#else
#define CORRADE_VISIBILITY_LOCAL __attribute__ ((visibility ("hidden")))
#endif

#endif
