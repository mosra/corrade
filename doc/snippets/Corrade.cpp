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
#define DOXYGEN_IGNORE(...) __VA_ARGS__

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
CORRADE_ENABLE(SSE41,POPCNT,LZCNT) int lookup(
    CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt), DOXYGEN_ELLIPSIS(int))
{
    DOXYGEN_ELLIPSIS(return 0;)
}
#endif
/* [Cpu-usage-target-attributes] */

namespace Bar {
using TransformT = void(*)(Containers::ArrayView<float>);
TransformT transformImplementation(Cpu::ScalarT);
TransformT transformImplementation(Cpu::Sse42T);
TransformT transformImplementation(Cpu::Avx2T);
TransformT transformImplementation(Cpu::Features);
/* [Cpu-usage-automatic-runtime-dispatch-declare] */
using TransformT = void(*)(Containers::ArrayView<float>);

TransformT transformImplementation(Cpu::ScalarT) {
    return [](Containers::ArrayView<float> data) { DOXYGEN_ELLIPSIS(static_cast<void>(data);) };
}
TransformT transformImplementation(Cpu::Sse42T) {
    return [](Containers::ArrayView<float> data) { DOXYGEN_ELLIPSIS(static_cast<void>(data);) };
}
TransformT transformImplementation(Cpu::Avx2T) {
    return [](Containers::ArrayView<float> data) { DOXYGEN_ELLIPSIS(static_cast<void>(data);) };
}

CORRADE_CPU_DISPATCHER_BASE(transformImplementation)
/* [Cpu-usage-automatic-runtime-dispatch-declare] */

namespace Baz {
TransformT transformImplementation(Cpu::Avx2T);
/* [Cpu-usage-automatic-runtime-dispatch-target-attributes] */
#ifdef CORRADE_ENABLE_AVX2
CORRADE_ENABLE_AVX2 TransformT transformImplementation(Cpu::Avx2T) {
    return [](Containers::ArrayView<float> data) CORRADE_ENABLE_AVX2 { DOXYGEN_ELLIPSIS(static_cast<void>(data);) };
}
#endif
/* [Cpu-usage-automatic-runtime-dispatch-target-attributes] */
}

using LookupT = int(*)(int);
LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Scalar));
LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Sse2));
LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt));
LookupT lookupImplementation(Cpu::Features);
/* [Cpu-usage-automatic-runtime-dispatch-extra-declare] */
using LookupT = int(*)(DOXYGEN_ELLIPSIS(int));

LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
    DOXYGEN_ELLIPSIS(return {};)
}
LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Sse2)) {
    DOXYGEN_ELLIPSIS(return {};)
}
LookupT lookupImplementation(CORRADE_CPU_DECLARE(Cpu::Sse41|Cpu::Popcnt|Cpu::Lzcnt)) {
    DOXYGEN_ELLIPSIS(return {};)
}

CORRADE_CPU_DISPATCHER(lookupImplementation, Cpu::Popcnt, Cpu::Lzcnt)
/* [Cpu-usage-automatic-runtime-dispatch-extra-declare] */

#ifdef CORRADE_CPU_USE_IFUNC
/* [Cpu-usage-automatic-cached-dispatch-ifunc] */
CORRADE_CPU_DISPATCHED_IFUNC(lookupImplementation, int lookup(DOXYGEN_ELLIPSIS(int)))
/* [Cpu-usage-automatic-cached-dispatch-ifunc] */
#else
/* [Cpu-usage-automatic-cached-dispatch-pointer] */
CORRADE_CPU_DISPATCHED_POINTER(lookupImplementation, int(*lookup)(DOXYGEN_ELLIPSIS(int)))
/* [Cpu-usage-automatic-cached-dispatch-pointer] */
#endif

namespace BarInsideABar {
int lookup(DOXYGEN_ELLIPSIS(int));
/* [Cpu-usage-automatic-cached-dispatch-compile-time] */
int lookup(DOXYGEN_ELLIPSIS(int)) {
    return lookupImplementation(CORRADE_CPU_SELECT(Cpu::Default))(DOXYGEN_ELLIPSIS(0));
}
/* [Cpu-usage-automatic-cached-dispatch-compile-time] */
}

}
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
using namespace Bar;
Containers::ArrayView<float> data;
/* [Cpu-usage-automatic-runtime-dispatch-call] */
/* Dispatch once and cache the function pointer */
TransformT transform = transformImplementation(Cpu::runtimeFeatures());

/* Call many times */
transform(data);
/* [Cpu-usage-automatic-runtime-dispatch-call] */
}

{
using namespace Bar;
#ifndef CORRADE_CPU_USE_IFUNC
#define LOOKUP_USES_FUNCTION_POINTER
#endif
/* [Cpu-usage-automatic-cached-dispatch-call] */
#ifdef LOOKUP_USES_FUNCTION_POINTER
int (*lookup)(DOXYGEN_ELLIPSIS(int))DOXYGEN_IGNORE( = nullptr);
#else
int lookup(DOXYGEN_ELLIPSIS(int));
#endif

int found = lookup(DOXYGEN_ELLIPSIS(0));
/* [Cpu-usage-automatic-cached-dispatch-call] */
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
