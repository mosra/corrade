#ifndef Corrade_Utility_StlMath_h
#define Corrade_Utility_StlMath_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
@m_since{2019,10}

With @ref CORRADE_TARGET_LIBSTDCXX "libstdc++" from GCC 6 and newer,
@cpp #include <cmath> @ce is above 10k lines when building with `-std=c++17`.
This is due to C++17 additions to the math library, such as
@ref std::riemann_zeta(). Because these APIs are seldom used in
graphics-related tasks, it doesn't make sense to have compile times inflated by
them. If you include @ref Corrade/Utility/StlMath.h instead, it will ensure the
new functions are not present by undefining the internal
`_GLIBCXX_USE_STD_SPEC_FUNCS` macro, making @ref cmath "&lt;cmath&gt;" roughly
the same size as on C++11. If you *need* the additions, @cpp #include <cmath> @ce
before this header.

The C++17 additions are also present on @ref CORRADE_TARGET_DINKUMWARE "MSVC STL"
since version 2017 15.7, however there the additions are very lightweight, so
no workaround is needed. For Clang, at the time of writing,
@ref CORRADE_TARGET_LIBCXX "libc++" 10 doesn't have these additions yet. On
these two this header effectively does just a simple @cpp #include <cmath> @ce.

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This header is also available as a single-header, dependency-less
    [CorradeStlMath.h](https://github.com/mosra/magnum-singles/tree/master/CorradeStlMath.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref Corrade/Utility/StlForwardArray.h,
    @ref Corrade/Utility/StlForwardString.h,
    @ref Corrade/Utility/StlForwardTuple.h,
    @ref Corrade/Utility/StlForwardVector.h
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
