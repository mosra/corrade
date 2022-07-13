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
#include "Corrade/Utility/Debug.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

#ifdef CORRADE_TARGET_X86
int main() {
/* [Cpu-usage-compile-time] */
Utility::Debug{} << "Base compiled instruction set:" << Cpu::DefaultBase;

if constexpr(Cpu::DefaultBase >= Cpu::Avx2) {
    // AVX2 code
} else {
    // scalar code
}
/* [Cpu-usage-compile-time] */

/* [Cpu-usage-extra-compile-time] */
Utility::Debug{} << "Base and extra instruction sets:" << Cpu::Default;

if constexpr(Cpu::Default >= (Cpu::Avx2|Cpu::AvxFma)) {
    // AVX2+FMA code
} else {
    // scalar code
}
/* [Cpu-usage-extra-compile-time] */
}
#endif
