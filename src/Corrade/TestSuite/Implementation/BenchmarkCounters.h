#ifndef Corrade_TestSuite_Implementation_BenchmarkCounters_h
#define Corrade_TestSuite_Implementation_BenchmarkCounters_h
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

#include <cstdint>

/* For wall clock */
#include <chrono>

/* For CPU clock */
#ifndef CORRADE_TARGET_WINDOWS
#include <ctime>
#elif !defined(CORRADE_TARGET_WINDOWS_RT)
#include <windows.h>
#endif

/* For RDTSC */
#ifdef CORRADE_TARGET_X86
#ifdef __GNUC__
#include <x86intrin.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#endif
#endif

#include "Corrade/configure.h"

namespace Corrade { namespace TestSuite { namespace Implementation {

/* Wall time in nanoseconds */
inline std::uint64_t wallTime() {
    /* OH GOD WHY SO COMPLICATED */
    return  std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

/* CPU time in nanoseconds */
inline std::uint64_t cpuTime() {
    #ifndef CORRADE_TARGET_WINDOWS
    return std::clock()*1000000000/CLOCKS_PER_SEC;
    #elif !defined(CORRADE_TARGET_WINDOWS_RT)
    /* FILETIME returns multiples of 100 nanoseconds */
    FILETIME a, b, c, d;
    if(!GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d))
        return 0; /* LCOV_EXCL_LINE */
    return ((std::uint64_t(d.dwHighDateTime) << 32)|std::uint64_t(d.dwLowDateTime))*100;
    #else
    return 0;
    #endif
}

/* RDTSC */
inline std::uint64_t rdtsc() {
    #ifdef CORRADE_TARGET_X86
    return __rdtsc();
    #else
    return 0;
    #endif
}

}}}

#endif
