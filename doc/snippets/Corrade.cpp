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

/* [Cpu-usage-extra-declare] */
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse2), DOXYGEN_ELLIPSIS(int));
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(int));
/* [Cpu-usage-extra-declare] */
int lookup(CORRADE_CPU_DECLARE(Cpu::Scalar), DOXYGEN_ELLIPSIS(int));
/* Might needed if Default doesn't include SSE2 on 32-bit */

namespace Foo {
/* [Cpu-usage-extra-ambiguity] */
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt), DOXYGEN_ELLIPSIS(int));
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(int));
/* [Cpu-usage-extra-ambiguity] */

int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt), int);
/* [Cpu-usage-extra-ambiguity-resolve] */
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(int)) {
    // Or the other variant, or a custom third implementation ...
    return lookup(CORRADE_CPU_SELECT(Cpu::Sse41|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(0));
}
/* [Cpu-usage-extra-ambiguity-resolve] */
}

/* [Cpu-usage-target-attributes] */
int lookup(CORRADE_CPU_DECLARE(Cpu::Scalar), DOXYGEN_ELLIPSIS(int)) {
    DOXYGEN_ELLIPSIS(return 0;)
}
#ifdef CORRADE_ENABLE_SSE2
CORRADE_ENABLE_SSE2 int lookup(CORRADE_CPU_DECLARE(Cpu::Sse2), DOXYGEN_ELLIPSIS(int)) {
    DOXYGEN_ELLIPSIS(return 0;)
}
#endif
#if defined(CORRADE_ENABLE_SSE41) && \
    defined(CORRADE_ENABLE_POPCNT) && \
    defined(CORRADE_ENABLE_LZCNT)
CORRADE_ENABLE_SSE41 CORRADE_ENABLE_POPCNT CORRADE_ENABLE_LZCNT
int lookup(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(int)) {
    DOXYGEN_ELLIPSIS(return 0;)
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
/* [Cpu-usage-extra-compile-time-call] */
int found = lookup(CORRADE_CPU_SELECT(Cpu::Default), DOXYGEN_ELLIPSIS(0));
/* [Cpu-usage-extra-compile-time-call] */
static_cast<void>(found);
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
