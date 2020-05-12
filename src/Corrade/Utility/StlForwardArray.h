#ifndef Corrade_Utility_StlForwardArray_h
#define Corrade_Utility_StlForwardArray_h
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
@brief Forward declaration for @ref std::array
@m_since{2019,10}

On @ref CORRADE_TARGET_LIBCXX "libc++" and
@ref CORRADE_TARGET_DINKUMWARE "MSVC STL" includes a lightweight
implementation-specific STL header containing just the forward declaration of
@ref std::array. On other implementations where forward declaration is not
possible or is unknown is equivalent to @cpp #include <array> @ce.

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This header is also available as a single-header, dependency-less
    [CorradeStlForwardArray.h](https://github.com/mosra/magnum-singles/tree/master/CorradeStlForwardArray.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref Corrade/Utility/StlForwardString.h,
    @ref Corrade/Utility/StlForwardTuple.h,
    @ref Corrade/Utility/StlForwardVector.h,
    @ref Corrade/Utility/StlMath.h
*/

#include "Corrade/configure.h"

#ifdef CORRADE_TARGET_LIBCXX
/* https://github.com/llvm-mirror/libcxx/blob/73d2eccc78ac83d5947243c4d26a53f668b4f432/include/__tuple#L223 */
#include <__tuple>
#elif defined(CORRADE_TARGET_DINKUMWARE)
/* MSVC has it defined next to std::pair */
#include <utility>
#else
/* Including the full definition otherwise. Couldn't find the declaration for
   libstdc++ (std::tuple is defined in <type_traits> but std::array is not),
   that however doesn't mean the forward declaration doesn't exist. */
#include <array>
#endif

#endif
