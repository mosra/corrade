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

#include "Corrade/Simd.h"
#include "Corrade/Containers/ArrayView.h"

using namespace Corrade;

#define DOXYGEN_IGNORE(...) __VA_ARGS__

#ifdef CORRADE_TARGET_X86
/* [Simd-compile-time-declare] */
void transformData(Simd::ScalarT, Containers::ArrayView<float> data);
void transformData(Simd::Sse41T, Containers::ArrayView<float> data);
void transformData(Simd::Avx2T, Containers::ArrayView<float> data);
/* [Simd-compile-time-declare] */

void transformData(Simd::Features features, Containers::ArrayView<float> data);
/* [Simd-runtime-dispatch] */
void transformData(Simd::Features features, Containers::ArrayView<float> data) {
    if(features & Simd::Avx2) transformData(Simd::Avx2, data);
    else if(features & Simd::Sse41) transformData(Simd::Sse41, data);
    else transformData(Simd::Scalar, data);
}
/* [Simd-runtime-dispatch] */
#endif

void foo();
void foo() {

#ifdef CORRADE_TARGET_X86
{
Containers::ArrayView<float> data, another;
/* [Simd-compile-time-use] */
/* Calls the Scalar implementation because there's nothing better for SSE3 */
transformData(Simd::Sse3, data);

/* Calls the AVX2 implementation if CORRADE_TARGET_AVX2 is defined, or the
   SSE4.1 one if either CORRADE_TARGET_SSE41 or CORRADE_TARGET_SSE42 is
   defined, falls back to scalar otherwise */
transformData(Simd::Default, another);
/* [Simd-compile-time-use] */
}

{
Containers::ArrayView<float> data;
/* [Simd-runtime-use] */
transformData(Simd::Features{}, data);
/* [Simd-runtime-use] */
}

{
Containers::ArrayView<float> data;
/* [Simd-runtime-cache] */
/* Don't want the AVX2 implementation as it's weirdly slow, OTOH this processor
   supports SSE4a so we can use the SSE4.1 implementation even though it isn't
   autodetected */
Simd::Features features;
features &= ~Simd::Avx2;
features |= Simd::Sse41;

transformData(features, data);
/* [Simd-runtime-cache] */
}
#endif

}
