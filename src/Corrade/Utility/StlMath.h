#ifndef Corrade_Utility_StlMath_h
#define Corrade_Utility_StlMath_h
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
@brief Include &lt;cmath&gt;, but without the heavy C++17 additions

With @ref CORRADE_TARGET_LIBSTDCXX "libstdc++" from GCC 6 and newer,
@cpp #include <cmath> @ce is above 10k lines when building with `-std=c++17`.
This is due to C++17 additions to the math library, such as
@ref std::riemann_zeta(). Because these APIs are seldom used in
graphics-related tasks, it doesn't make sense to have compile times inflated by
them. If you include @ref Corrade/Utility/StlMath.h instead, it will ensure the
new functions are not present, making the @ref cmath "&lt;cmath&gt;" roughly the same
size as on C++11. If you *need* the additions, @cpp #include <cmath> @ce
* *before* this header.

Currently, the C++17 additions are present neither on
@ref CORRADE_TARGET_LIBCXX "libc++" 8 nor
@ref CORRADE_TARGET_DINKUMWARE "MSVC STL" 2017, so there this header
effectively does just a simple @cpp #include <cmath> @ce.
*/

#include "Corrade/configure.h"

/* On GCC 6.1+ this brings in bits/c++config.h, which defines the below macro
   to 1 on C++17 and up. On other implementations does nothing. */
#include <ciso646>

#ifdef _GLIBCXX_USE_STD_SPEC_FUNCS
#undef _GLIBCXX_USE_STD_SPEC_FUNCS
#define _GLIBCXX_USE_STD_SPEC_FUNCS 0
#endif
#include <cmath>

#endif
