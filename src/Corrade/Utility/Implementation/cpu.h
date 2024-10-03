#ifndef Corrade_Utility_Implementation_cpu_h
#define Corrade_Utility_Implementation_cpu_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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
#include "Corrade/Utility/Macros.h"

/* Function-pointer-based CPU dispatch definitions for the implementation */
#if defined(CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH) || (defined(CORRADE_BUILD_CPU_RUNTIME_DISPATCH) && !defined(CORRADE_CPU_USE_IFUNC))
    #define CORRADE_UTILITY_CPU_DISPATCHER(...) CORRADE_CPU_DISPATCHER(__VA_ARGS__)
    #define CORRADE_UTILITY_CPU_DISPATCHER_BASE(...) CORRADE_CPU_DISPATCHER_BASE(__VA_ARGS__)
    #define CORRADE_UTILITY_CPU_DISPATCHED(dispatcher, ...)                 \
        CORRADE_CPU_DISPATCHED_POINTER(dispatcher, __VA_ARGS__) CORRADE_NOOP
    #define CORRADE_UTILITY_CPU_MAYBE_UNUSED
/* IFUNC or compile-time CPU dispatch, runtime dispatcher is not exposed */
#else
    /* Runtime dispatcher implementation is either hidden or not present at
       all */
    #if defined(CORRADE_BUILD_CPU_RUNTIME_DISPATCH) && defined(CORRADE_CPU_USE_IFUNC)
        #define CORRADE_UTILITY_CPU_DISPATCHER(...)                         \
            namespace { CORRADE_CPU_DISPATCHER(__VA_ARGS__) }
        #define CORRADE_UTILITY_CPU_DISPATCHER_BASE(...)                    \
            namespace { CORRADE_CPU_DISPATCHER_BASE(__VA_ARGS__) }
        #define CORRADE_UTILITY_CPU_DISPATCHED(dispatcher, ...)             \
            CORRADE_CPU_DISPATCHED_IFUNC(dispatcher, __VA_ARGS__) CORRADE_NOOP
        #define CORRADE_UTILITY_CPU_MAYBE_UNUSED
    #elif !defined(CORRADE_BUILD_CPU_RUNTIME_DISPATCH)
        #define CORRADE_UTILITY_CPU_DISPATCHER(...)
        #define CORRADE_UTILITY_CPU_DISPATCHER_BASE(...)
        #define CORRADE_UTILITY_CPU_DISPATCHED(dispatcher, ...)             \
            __VA_ARGS__ CORRADE_PASSTHROUGH
        #define CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_UNUSED
    #else
    #error mosra messed up!
    #endif
#endif

#endif
