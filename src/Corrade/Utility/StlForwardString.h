#ifndef Corrade_Utility_StlForwardString_h
#define Corrade_Utility_StlForwardString_h
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
@brief Forward declaration for @ref std::string
@m_since{2019,10}

On @ref CORRADE_TARGET_LIBCXX "libc++" and
@ref CORRADE_TARGET_LIBSTDCXX "libstdc++" includes a lightweight
implementation-specific STL header containing just the forward declaration of
@ref std::string. On @ref CORRADE_TARGET_DINKUMWARE "MSVC STL" and other
implementations where forward declaration is not possible or is unknown is
equivalent to @cpp #include <string> @ce.

@attention On libc++, the forward declaration header contains just
    @ref std::string and @ref std::wstring. While that is enough for majority
    of cases, types like @ref std::u32string are not there and in that case
    you need to @cpp #include <string> @ce in its complete form. You can detect
    this case using the @ref CORRADE_TARGET_LIBCXX macro.

@m_class{m-block m-success}

@par Single-header version
    This header is also available as a single-header, dependency-less
    [CorradeStlForwardString.h](https://github.com/mosra/magnum-singles/tree/master/CorradeStlForwardString.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref Corrade/Utility/StlForwardArray.h,
    @ref Corrade/Utility/StlForwardTuple.h,
    @ref Corrade/Utility/StlForwardVector.h,
    @ref Corrade/Utility/StlMath.h
*/

#include "Corrade/configure.h"

#ifdef CORRADE_TARGET_LIBCXX
/* https://github.com/llvm-mirror/libcxx/blob/8c58c2293739d3d090c721827e4217c113ced89f/include/iosfwd#L190-L195 */
#include <iosfwd>
#elif defined(CORRADE_TARGET_LIBSTDCXX)
/* https://github.com/gcc-mirror/gcc/blob/2a4787da69071b5d5bc178795accca264073b7e4/libstdc%2B%2B-v3/include/bits/stringfwd.h#L68-L73 */
#include <bits/stringfwd.h>
#else
/* Including the full definition otherwise. MSVC has the full definition in
   <xstring>, which prevents any forward declaration */
#include <string>
#endif

#endif
