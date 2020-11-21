#ifndef Corrade_configure_h
#define Corrade_configure_h
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

#cmakedefine CORRADE_MSVC2019_COMPATIBILITY
#cmakedefine CORRADE_MSVC2017_COMPATIBILITY
#cmakedefine CORRADE_MSVC2015_COMPATIBILITY

#cmakedefine CORRADE_BUILD_DEPRECATED
#cmakedefine CORRADE_BUILD_STATIC
#cmakedefine CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
#cmakedefine CORRADE_BUILD_MULTITHREADED

#cmakedefine CORRADE_TARGET_APPLE
#cmakedefine CORRADE_TARGET_IOS
#cmakedefine CORRADE_TARGET_IOS_SIMULATOR
#cmakedefine CORRADE_TARGET_UNIX
#cmakedefine CORRADE_TARGET_WINDOWS
#cmakedefine CORRADE_TARGET_WINDOWS_RT
#cmakedefine CORRADE_TARGET_EMSCRIPTEN
#cmakedefine CORRADE_TARGET_ANDROID

#cmakedefine CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
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

/* First is GCC/Clang, second is MSVC */
#elif defined(__powerpc64__) || defined(_M_PPC)
#define CORRADE_TARGET_POWERPC

/* Otherwise one should expect CORRADE_TARGET_EMSCRIPTEN. No other platforms
   are currently tested for, but that's okay -- a runtime test for this is in
   Utility/Test/SystemTest.cpp */
#endif

/* Sanity checks */
#if defined(CORRADE_TARGET_EMSCRIPTEN) && (defined(CORRADE_TARGET_X86) || defined(CORRADE_TARGET_ARM))
#error CORRADE_TARGET_X86 / _ARM / _POWERPC defined on Emscripten
#endif

/* C++ standard */
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#define CORRADE_CXX_STANDARD _MSVC_LANG
#else
#define CORRADE_CXX_STANDARD 201103L
#endif
#else
#define CORRADE_CXX_STANDARD __cplusplus
#endif

/* Standard library edition. Keep in sync with CorradeStl* singles. References:
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65473
   https://github.com/gcc-mirror/gcc/commit/19665740d336d4ee7d0cf92b5b0643fa1d7da14a
   https://en.cppreference.com/w/cpp/header/ciso646 */
#include <ciso646>
#ifdef _LIBCPP_VERSION
#define CORRADE_TARGET_LIBCXX
#elif defined(_CPPLIB_VER)
#define CORRADE_TARGET_DINKUMWARE
#elif defined(__GLIBCXX__)
#define CORRADE_TARGET_LIBSTDCXX
/* GCC's <ciso646> provides the __GLIBCXX__ macro only since 6.1, so on older
   versions we'll try to get it from bits/c++config.h */
#elif defined(__has_include)
    #if __has_include(<bits/c++config.h>)
        #include <bits/c++config.h>
        #ifdef __GLIBCXX__
        #define CORRADE_TARGET_LIBSTDCXX
        #endif
    #endif
/* GCC < 5.0 doesn't have __has_include, so on these versions we'll just assume
   it's libstdc++ as I don't think those versions are used with anything else
   nowadays anyway. Clang reports itself as GCC 4.4, so exclude that one. */
#elif defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
#define CORRADE_TARGET_LIBSTDCXX
#else
/* Otherwise no idea. */
#endif

#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif

#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif

#if defined(__clang__) && defined(_MSC_VER)
#define CORRADE_TARGET_CLANG_CL
#endif

#if defined(__clang__) && defined(__apple_build_version__)
/* The extra space is here to avoid issues with old & broken FindCorrade.cmake
   matching this as CORRADE_TARGET_APPLE. Should be removed in late 2025 when
   everyone has their find modules finally updated. */
#define  CORRADE_TARGET_APPLE_CLANG
#endif

#ifdef _MSC_VER
#define CORRADE_TARGET_MSVC
#endif

#ifdef __MINGW32__
#define CORRADE_TARGET_MINGW
#endif

/* First checking the GCC/Clang builtin, if available. As a fallback do an
   architecture-based check, which is mirrored from SDL_endian.h. Doing this
   *properly* would mean we can't decide this at compile time as some
   architectures allow switching endianness at runtime (and worse, have
   per-page endianness). So let's pretend we never saw this article:
    https://en.wikipedia.org/wiki/Endianness#Bi-endianness
   For extra safety this gets runtime-tested in TargetTest, so when porting
   to a new platform, make sure you run that test. */
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CORRADE_TARGET_BIG_ENDIAN
#elif __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error what kind of endianness is this?
#endif
#elif defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define CORRADE_TARGET_BIG_ENDIAN
#endif
/* Can't really be marked as deprecated as it's only ever used in #ifdef
   statements. OTOH I don't want to remove this right away as it would cause
   temporary breakages until projects such as magnum-plugins update. */
#if defined(CORRADE_BUILD_DEPRECATED) && defined(CORRADE_TARGET_BIG_ENDIAN)
#define CORRADE_BIG_ENDIAN
#endif

/* SIMD extensions. Not much here yet. */
#ifdef CORRADE_TARGET_GCC
#ifdef __SSE2__
#define CORRADE_TARGET_SSE2
#endif

#elif defined(CORRADE_TARGET_MSVC)
/* _M_IX86_FP is defined only on 32bit, 64bit has SSE2 always (so we need to
   detect 64bit instead: https://stackoverflow.com/a/18570487) */
#if (defined(_M_IX86_FP) && _M_IX86_FP == 2) || defined(_M_AMD64) || defined(_M_X64)
#define CORRADE_TARGET_SSE2
#endif
#endif

/* Documented in Utility/TypeTraits.h */
#if defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_ANDROID) && !__LP64__) || defined(CORRADE_TARGET_EMSCRIPTEN)
#define CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
#endif

#endif
