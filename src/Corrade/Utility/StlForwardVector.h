#ifndef Corrade_Utility_StlForwardVector_h
#define Corrade_Utility_StlForwardVector_h
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
@brief Forward declaration for @ref std::vector
@m_since{2019,10}

On @ref CORRADE_TARGET_LIBCXX "libc++" (3.9.0 and up) and
@ref CORRADE_TARGET_LIBSTDCXX "libstdc++" (all versions) includes a lightweight
implementation-specific STL header containing just the forward declaration of
@ref std::vector. On @ref CORRADE_TARGET_DINKUMWARE "MSVC STL" and other
implementations where forward declaration is not possible or is unknown is
equivalent to @cpp #include <vector> @ce.

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This header is also available as a single-header, dependency-less
    [CorradeStlForwardVector.h](https://github.com/mosra/magnum-singles/tree/master/CorradeStlForwardVector.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref Corrade/Utility/StlForwardArray.h,
    @ref Corrade/Utility/StlForwardString.h,
    @ref Corrade/Utility/StlForwardTuple.h,
    @ref Corrade/Utility/StlMath.h
*/

#include "Corrade/configure.h"

#if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION >= 3900
/* https://github.com/llvm-mirror/libcxx/blob/8c58c2293739d3d090c721827e4217c113ced89f/include/iosfwd#L199-L200.
   Present since b3792285ed48c8ee0e877b763b983093d656913a, which was released
   in libc++ 3.9.0. */
#include <iosfwd>
#else
/* Including the full definition otherwise. MSVC has the full definition in
   <vector>, which prevents any forward declaration, libstdc++ has the full
   definition in bits/stl_vector.h but the header depends on a lot of other
   internals and including them all is not much different from full <vector>. */
#include <vector>
#endif

#endif
