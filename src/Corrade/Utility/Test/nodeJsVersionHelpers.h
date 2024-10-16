#ifndef Corrade_Utility_Test_cpuVariantHelpers_h
#define Corrade_Utility_Test_cpuVariantHelpers_h
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

#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <emscripten/em_asm.h>

namespace Corrade { namespace Utility { namespace Test {

inline int nodeJsVersion() {
    #pragma GCC diagnostic push
    /* The damn thing moved the warning to another group in some version. Not
       sure if it happened in Clang 10 already, but -Wc++20-extensions is new
       in Clang 10, so just ignore both. HOWEVER, Emscripten often uses a
       prerelease Clang, so if it reports version 10, it's likely version 9. So
       check for versions _above_ 10 instead. */
    #pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
    #if __clang_major__ > 10
    #pragma GCC diagnostic ignored "-Wc++20-extensions"
    #endif
    return EM_ASM_INT({
        /* https://stackoverflow.com/a/6656430, coerce to an int (remember
           asm.js?). And don't blow up when running in a browser. Similar check
           is in Utility.js.in. */
        if(typeof process === 'undefined')
            return 0;
        return process.versions.node.split('.')[0]|0;
    });
    #pragma GCC diagnostic pop
}

/* Returns true only if running under node.js and the major version is less
   than what's given. Returns false if not running under node.js, the version
   is the same or greater. */
inline bool nodeJsVersionLess(int than) {
    const int version = nodeJsVersion();
    return version && version < than;
}

}}}
#else
#error this file is available only on the Emscripten build
#endif

#endif
