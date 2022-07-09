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

#include "Corrade/Cpu.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/Debug.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

#ifdef CORRADE_TARGET_X86
/* [Cpu-usage-declare] */
void transform(Cpu::ScalarT, Containers::ArrayView<float> data);
void transform(Cpu::Sse42T, Containers::ArrayView<float> data);
void transform(Cpu::Avx2T, Containers::ArrayView<float> data);
/* [Cpu-usage-declare] */

/* [Cpu-usage-target-attributes] */
void transform(Cpu::ScalarT, Containers::ArrayView<float> data) {
    DOXYGEN_ELLIPSIS(static_cast<void>(data);)
}
#ifdef CORRADE_ENABLE_SSE42
CORRADE_ENABLE_SSE42 void transform(Cpu::Sse42T, Containers::ArrayView<float> data) {
    DOXYGEN_ELLIPSIS(static_cast<void>(data);)
}
#endif
#ifdef CORRADE_ENABLE_AVX2
CORRADE_ENABLE_AVX2 void transform(Cpu::Avx2T, Containers::ArrayView<float> data) {
    DOXYGEN_ELLIPSIS(static_cast<void>(data);)
}
#endif
/* [Cpu-usage-target-attributes] */
#endif

inline void foo(Cpu::ScalarT) {}

int main() {
#ifdef CORRADE_TARGET_X86
{
Containers::ArrayView<float> data;
/* [Cpu-usage-compile-time-call] */
transform(Cpu::DefaultBase, data);
/* [Cpu-usage-compile-time-call] */
}

{
Containers::ArrayView<float> data;
/* [Cpu-usage-runtime-manual-dispatch] */
Cpu::Features features = Cpu::runtimeFeatures();
Utility::Debug{} << "Instruction set available at runtime:" << features;

if(features & Cpu::Avx2)
    transform(Cpu::Avx2, data);
else if(features & Cpu::Sse41)
    transform(Cpu::Sse41, data);
else
    transform(Cpu::Scalar, data);
/* [Cpu-usage-runtime-manual-dispatch] */
}

{
/* [Cpu-tag-from-type] */
foo(Cpu::Avx2);
foo(Cpu::tag<Cpu::Avx2T>());
/* [Cpu-tag-from-type] */
}
{
/* [Cpu-features-from-type] */
Cpu::Features a = Cpu::Avx2;
Cpu::Features b = Cpu::features<Cpu::Avx2T>();
/* [Cpu-features-from-type] */
static_cast<void>(a);
static_cast<void>(b);
}
#endif

}
