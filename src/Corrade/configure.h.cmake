#ifndef Corrade_configure_h
#define Corrade_configure_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#cmakedefine CORRADE_BIG_ENDIAN
#cmakedefine CORRADE_GCC47_COMPATIBILITY
#cmakedefine CORRADE_MSVC2017_COMPATIBILITY
#cmakedefine CORRADE_MSVC2015_COMPATIBILITY

#cmakedefine CORRADE_BUILD_DEPRECATED
#cmakedefine CORRADE_BUILD_STATIC

#cmakedefine CORRADE_TARGET_APPLE
#cmakedefine CORRADE_TARGET_IOS
#cmakedefine CORRADE_TARGET_IOS_SIMULATOR
#cmakedefine CORRADE_TARGET_UNIX
#cmakedefine CORRADE_TARGET_WINDOWS
#cmakedefine CORRADE_TARGET_WINDOWS_RT
#cmakedefine CORRADE_TARGET_EMSCRIPTEN
#cmakedefine CORRADE_TARGET_ANDROID

#cmakedefine CORRADE_TESTSUITE_TARGET_XCTEST
#cmakedefine CORRADE_UTILITY_USE_ANSI_COLORS

/* Cherry-picked from https://sourceforge.net/p/predef/wiki/Architectures/.
   Can't detect this stuff directly from CMake because of (for example) macOS
   and fat binaries. */

/* First two is GCC/Clang for 32/64 bit, second two is MSVC 32/64bit */
#if defined(__i386) || defined(__x86_64) || defined(_M_IX86) || defined(_M_X64)
#define CORRADE_TARGET_X86

/* First two is GCC/Clang for 32/64 bit, second two is MSVC 32/64bit. MSVC
   doesn't have AArch64 support in the compiler yet, though there are some
   signs of it in headers (http://stackoverflow.com/a/37251625/6108877). */
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
#define CORRADE_TARGET_ARM

/* Otherwise one should expect CORRADE_TARGET_EMSCRIPTEN. No other platforms
   are currently tested for, but that's okay -- a runtime test for this is in
   Utility/Test/SystemTest.cpp */
#endif

/* Sanity checks */
#if defined(CORRADE_TARGET_EMSCRIPTEN) && (defined(CORRADE_TARGET_X86) || defined(CORRADE_TARGET_ARM))
#error CORRADE_TARGET_X86 or CORRADE_TARGET_ARM defined on Emscripten
#endif

#endif
