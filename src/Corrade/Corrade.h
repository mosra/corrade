#ifndef Corrade_Corrade_h
#define Corrade_Corrade_h
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

/** @file
 * @brief Basic definitions and forward declarations for the @ref Corrade namespace
 */

#include "Corrade/configure.h"

namespace Corrade {

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief MSVC compatibility
@m_since_latest

Defined if compatibility mode for Microsoft Visual C++ 2019+ *without* the
`/permissive-` flag set is enabled.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_MSVC_COMPATIBILITY
#undef CORRADE_MSVC_COMPATIBILITY

/**
@brief MSVC 2017 compatibility

Defined if compatibility mode for MSVC 2017 is enabled.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_MSVC2017_COMPATIBILITY
#undef CORRADE_MSVC2017_COMPATIBILITY

/**
@brief MSVC 2015 compatibility

Defined if compatibility mode for MSVC 2015 is enabled.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_MSVC2015_COMPATIBILITY
#undef CORRADE_MSVC2015_COMPATIBILITY

/**
@brief Build with deprecated API included

Defined if the library contains deprecated API (which will be removed in the
future). To preserve backward compatibility, Corrade is by default built with
deprecated API included.
@see @ref building-corrade, @ref corrade-cmake
*/
/* This macro is enabled by default in Doxyfile, but we may still want to
   temporarily remove it from there to catch outdated links. This makes it work
   in both cases. */
#ifdef CORRADE_BUILD_DEPRECATED
#define CORRADE_BUILD_DEPRECATED
#else
#define CORRADE_BUILD_DEPRECATED
#undef CORRADE_BUILD_DEPRECATED
#endif

/**
@brief Static library build

Defined if built as static libraries. Default are shared libraries.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_BUILD_STATIC
#undef CORRADE_BUILD_STATIC

/**
@brief Static library build with globals unique across shared libraries
@m_since{2020,06}

Enabled by default in a static build.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
#undef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS

/**
@brief Multi-threaded build
@m_since{2019,10}

Defined if the library is built in a way that makes it possible to safely use
certain Corrade features simultaneously in multiple threads. In particular:

-   @ref Corrade::Utility::Debug "Utility::Debug" and derived classes use it
    to have thread-local scoped output redirection and coloring
-   @ref Corrade::PluginManager::Manager "PluginManager::Manager" uses it for
    thread-local plugin loading, unloading and management

Apart from these, @ref Corrade::Utility::Resource "Utility::Resource" uses
global data but isn't affected by this option, as majority of its operation is
only reading from the global storage. All other functionality is free of any
read/write access to global data.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_BUILD_MULTITHREADED
#undef CORRADE_BUILD_MULTITHREADED

/**
@brief Build with runtime CPU dispatch
@m_since_latest

Defined if the library is built with performance-critical code paths optimized
for multiple architectures (such as SSE or AVX on x86), with the best matching
variant selected at runtime based on detected CPU features. If not defined, the
library is built with just a single variant that's picked at compile time
depending on target architecture flags being passed to the compiler.

The actual feature detection and dispatch both in the runtime and compile-time
scenario is performed by the @relativeref{Corrade,Cpu} library. See
@ref Cpu-usage-automatic-cached-dispatch for details and information about
performance tradeoffs.
@see @see @ref CORRADE_CPU_USE_IFUNC, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_BUILD_CPU_RUNTIME_DISPATCH
#undef CORRADE_BUILD_CPU_RUNTIME_DISPATCH

/**
@brief Debug build

Gets defined if you (even transitively) link to any `Corrade::*` target in
CMake and the build configuration (of your project, not Corrade itself) is
`Debug`. No need to @cpp #include @ce anything for this macro to be defined,
it's supplied via a compiler flag by CMake.

Not guaranteed to be defined if using custom buildsystems or doing things
differently than what's documented in @ref corrade-cmake.
*/
#define CORRADE_IS_DEBUG_BUILD
#undef CORRADE_IS_DEBUG_BUILD

/**
@brief C++ standard version

Expands to `__cplusplus` macro on all sane compilers; on MSVC uses `_MSVC_LANG`
if defined (since Visual Studio 2015 Update 3), otherwise reports C++11. The
returned version is:

-   @cpp 201103 @ce when C++11 is used
-   @cpp 201402 @ce when C++14 is used
-   @cpp 201703 @ce when C++17 is used
-   @cpp 202002 @ce when C++20 is used
-   greater than @cpp 202002 @ce when C++2b is used

Note that compilers that don't have full support for given standard may not
return the exact value, in which case it's recommended to check that the
reported value is *greater* than the previous standard, for example
@cpp #if CORRADE_CXX_STANDARD > 201703 @ce to test whether compiling as C++20.

Unlike most other `CORRADE_*` variables, this macro is not exposed to CMake as
because the meaning is unclear in projects that combine more different C++
standards in a single project.
@see @ref CORRADE_TARGET_LIBCXX, @ref CORRADE_TARGET_LIBSTDCXX,
    @ref CORRADE_TARGET_DINKUMWARE
*/
#define CORRADE_CXX_STANDARD
#undef CORRADE_CXX_STANDARD

/**
@brief Unix target

Defined if the library is built for some Unix flavor (Linux, BSD, macOS, iOS,
Android...). Note that while the behavior of Emscripten is closely emulating
Unix systems, `CORRADE_TARGET_UNIX` is not defined there, only
@ref CORRADE_TARGET_EMSCRIPTEN.
@see @ref CORRADE_TARGET_APPLE, @ref CORRADE_TARGET_ANDROID,
    @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_CLANG, @ref corrade-cmake
*/
#define CORRADE_TARGET_UNIX
#undef CORRADE_TARGET_UNIX

/**
@brief Apple target

Defined if the library is built for Apple platforms (macOS, iOS).
@see @ref CORRADE_TARGET_UNIX, @ref CORRADE_TARGET_IOS,
    @ref CORRADE_TARGET_CLANG, @ref corrade-cmake
*/
#define CORRADE_TARGET_APPLE
#undef CORRADE_TARGET_APPLE

/**
@brief iOS target

Defined if the library is built for iOS (device or simulator).
@see @ref CORRADE_TARGET_IOS_SIMULATOR, @ref CORRADE_TARGET_UNIX,
    @ref CORRADE_TARGET_APPLE, @ref corrade-cmake
*/
#define CORRADE_TARGET_IOS
#undef CORRADE_TARGET_IOS

/**
@brief iOS Simulator target

Defined if the library is built for iOS Simulator.
@see @ref CORRADE_TARGET_IOS, @ref CORRADE_TARGET_UNIX,
    @ref CORRADE_TARGET_APPLE, @ref corrade-cmake
*/
#define CORRADE_TARGET_IOS_SIMULATOR
#undef CORRADE_TARGET_IOS_SIMULATOR

/**
@brief Windows target

Defined if the library is built for Windows (desktop, Store or Phone).
@see @ref CORRADE_TARGET_WINDOWS_RT, @ref CORRADE_TARGET_MSVC,
    @ref corrade-cmake
*/
#define CORRADE_TARGET_WINDOWS
#undef CORRADE_TARGET_WINDOWS

/**
@brief Windows RT target

Defined if the library is built for Windows Store or Phone.
@see @ref CORRADE_TARGET_WINDOWS, @ref corrade-cmake
*/
#define CORRADE_TARGET_WINDOWS_RT
#undef CORRADE_TARGET_WINDOWS_RT

/**
@brief Emscripten target

Defined if the library is built for [Emscripten](http://kripken.github.io/emscripten-site/).
Note that while the behavior of Emscripten is closely emulating Unix systems,
@ref CORRADE_TARGET_UNIX is not defined there, only `CORRADE_TARGET_EMSCRIPTEN`.
@see @ref CORRADE_TARGET_CLANG, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_EMSCRIPTEN
#undef CORRADE_TARGET_EMSCRIPTEN

/**
@brief Android target

Defined if the library is built for Android.
@see @ref CORRADE_TARGET_UNIX, @ref CORRADE_TARGET_CLANG,
    @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_ANDROID
#undef CORRADE_TARGET_ANDROID

/**
@brief x86 target

Defined if the library is built for x86 platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC, @ref CORRADE_TARGET_WASM,
@ref CORRADE_TARGET_32BIT, @ref CORRADE_TARGET_BIG_ENDIAN and derived
instruction set variables are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC
nor @ref CORRADE_TARGET_WASM is defined, the platform might be either a very
old pre-WebAssembly @ref CORRADE_TARGET_EMSCRIPTEN or any other that the
library doesn't know about yet.
@see @ref CORRADE_TARGET_SSE2, @ref CORRADE_TARGET_SSE3,
    @ref CORRADE_TARGET_SSSE3, @ref CORRADE_TARGET_SSE41,
    @ref CORRADE_TARGET_SSE42, @ref CORRADE_TARGET_AVX,
    @ref CORRADE_TARGET_AVX_F16C, @ref CORRADE_TARGET_AVX_FMA,
    @ref CORRADE_TARGET_AVX2, @ref CORRADE_TARGET_AVX512F,
    @relativeref{Corrade,Cpu}
*/
#define CORRADE_TARGET_X86
#undef CORRADE_TARGET_X86

/**
@brief ARM target

Defined if the library is built for ARM platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_POWERPC, @ref CORRADE_TARGET_WASM,
@ref CORRADE_TARGET_32BIT, @ref CORRADE_TARGET_BIG_ENDIAN and derived
instruction set variables are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC
nor @ref CORRADE_TARGET_WASM is defined, the platform might be either a very
old pre-WebAssembly @ref CORRADE_TARGET_EMSCRIPTEN or any other that the
library doesn't know about yet.
@see @ref CORRADE_TARGET_NEON, @ref CORRADE_TARGET_NEON_FMA,
    @ref CORRADE_TARGET_NEON_FP16, @relativeref{Corrade,Cpu}
*/
#define CORRADE_TARGET_ARM
#undef CORRADE_TARGET_ARM

/**
@brief PowerPC target
@m_since{2019,10}

Defined if the library is built for PowerPC platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_WASM,
@ref CORRADE_TARGET_32BIT, @ref CORRADE_TARGET_BIG_ENDIAN and derived
instruction set variables are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC
nor @ref CORRADE_TARGET_WASM is defined, the platform might be either a very
old pre-WebAssembly @ref CORRADE_TARGET_EMSCRIPTEN or any other that the
library doesn't know about yet.
@see @ref CORRADE_TARGET_BIG_ENDIAN
*/
#define CORRADE_TARGET_POWERPC
#undef CORRADE_TARGET_POWERPC

/**
@brief WebAssembly target
@m_since_latest

Defined if the library is built for WebAssembly (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC,
@ref CORRADE_TARGET_32BIT, @ref CORRADE_TARGET_BIG_ENDIAN and derived
instruction set variables are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC
nor @ref CORRADE_TARGET_WASM is defined, the platform might be either a very
old pre-WebAssembly @ref CORRADE_TARGET_EMSCRIPTEN or any other that the
library doesn't know about yet.
@see @ref CORRADE_TARGET_SIMD128, @relativeref{Corrade,Cpu}
*/
#define CORRADE_TARGET_WASM
#undef CORRADE_TARGET_WASM

/**
@brief 32-bit target
@m_since_latest

Defined if the library is built for a 32-bit target. Not defined on 64-bit
platforms. Note that unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC,
@ref CORRADE_TARGET_WASM, @ref CORRADE_TARGET_BIG_ENDIAN and derived
instruction set variables are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries.
*/
#define CORRADE_TARGET_32BIT
#undef CORRADE_TARGET_32BIT

/**
@brief Big-Endian target
@m_since{2020,06}

Defined when the platform defaults to Big-Endian (such as HP/PA RISC, Motorola
68k, Big-Endian MIPS, PowerPC and SPARC). Not defined on Little-Endian
platforms (such as x86 and ARM). Note that some platforms are Bi-Endian,
meaning the endianness can be switched at runtime (and thus can't be detected
at compile-time), this macro only reflects the usual architecture default.
Moreover, unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC,
@ref CORRADE_TARGET_WASM and derived instruction set variables are not exposed
in CMake because the meaning is unclear on platforms with multi-architecture
binaries.
@see @ref CORRADE_TARGET_POWERPC, @ref Corrade::Utility::Endianness,
    @ref corrade-cmake
*/
#define CORRADE_TARGET_BIG_ENDIAN
#undef CORRADE_TARGET_BIG_ENDIAN

/**
@brief GCC compiler
@m_since{2020,06}

Defined if the code is being compiled by GCC or GCC-compatible Clang (which is
@ref CORRADE_TARGET_APPLE_CLANG but not @ref CORRADE_TARGET_CLANG_CL, for
example). While this variable is exposed in CMake as well, it's not guaranteed
that the reported compiler is consistent between CMake and C++ --- for example,
a library can be built with GCC and then used via Clang.

Major GCC version number can be accessed using the `__GNUC__` macro. Clang
always reports itself as GCC 4.4, its version is reported in `__clang_major__`
instead.
@see @ref CORRADE_TARGET_CLANG, @ref CORRADE_TARGET_APPLE_CLANG,
    @ref CORRADE_TARGET_CLANG_CL, @ref CORRADE_TARGET_MSVC,
    @ref CORRADE_TARGET_MINGW, @ref CORRADE_TARGET_UNIX,
    @ref CORRADE_TARGET_EMSCRIPTEN, @ref corrade-cmake
*/
#define CORRADE_TARGET_GCC
#undef CORRADE_TARGET_GCC

/**
@brief Clang compiler
@m_since{2020,06}

Defined if the code is being compiled by Clang or any of its variants
(@ref CORRADE_TARGET_APPLE_CLANG, @ref CORRADE_TARGET_CLANG_CL). If this
variable is defined, usually @ref CORRADE_TARGET_GCC is also defined, except
for Clang-CL. While this variable is exposed in CMake as well, it's not
guaranteed that the reported compiler is consistent between CMake and C++ ---
for example, a library can be built with Clang and then used via GCC.

Major Clang version number can be accessed using the `__clang_major__` macro,
however note that Apple Clang (in Xcode)
[uses its own numbering](https://en.wikipedia.org/wiki/Xcode#Toolchain_versions)
--- for example Clang 10 is actually Apple Clang 12.
@see @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_APPLE_CLANG,
    @ref CORRADE_TARGET_CLANG_CL, @ref CORRADE_TARGET_MSVC,
    @ref CORRADE_TARGET_MINGW, @ref CORRADE_TARGET_UNIX,
    @ref CORRADE_TARGET_POWERPC, @ref CORRADE_TARGET_EMSCRIPTEN,
    @ref corrade-cmake
*/
#define CORRADE_TARGET_CLANG
#undef CORRADE_TARGET_CLANG

/**
@brief Apple's Clang compiler
@m_since{2020,06}

Defined if the code is being compiled by Apple's Clang. If this variable is
defined, @ref CORRADE_TARGET_GCC and @ref CORRADE_TARGET_CLANG are also
defined. This is primarily useful when checking for Clang version, as Apple
uses a different versioning scheme. While this variable is exposed in CMake as
well, it's not guaranteed that the reported compiler is consistent between
CMake and C++ --- for example, a library can be built with Clang and then used
via GCC.

Major Apple Clang version number can be accessed using the `__clang_major__`
macro, which is the same macro as vanilla Clang. There is no macro that exposes
the matching vanilla Clang version, the only option is to use
[an external mapping table](https://en.wikipedia.org/wiki/Xcode#Toolchain_versions)
--- for example Apple Clang 12 is actually Clang 10.
@see @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_CLANG,
    @ref CORRADE_TARGET_CLANG_CL, @ref CORRADE_TARGET_MSVC,
    @ref CORRADE_TARGET_MINGW, @ref CORRADE_TARGET_APPLE, @ref corrade-cmake
*/
#define CORRADE_TARGET_APPLE_CLANG
#undef CORRADE_TARGET_APPLE_CLANG

/**
@brief Clang-CL compiler
@m_since{2020,06}

Defined if the code is being compiled by Clang with a MSVC frontend. If this
variable is defined, @ref CORRADE_TARGET_CLANG and @ref CORRADE_TARGET_MSVC is
also defined (but @ref CORRADE_TARGET_GCC not). While this variable is exposed
in CMake as well, it's not guaranteed that the reported compiler is consistent
between CMake and C++ --- for example, a library can be built with Clang-CL and
then used via MSVC.

Clang-CL uses the same versioning scheme as vanilla Clang, accessible using the
`__clang_major__` macro. It also exposes the `_MSC_VER` macro, reporting the
MSVC version it's compatible with.
@see @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_CLANG,
    @ref CORRADE_TARGET_APPLE_CLANG, @ref CORRADE_TARGET_MSVC,
    @ref CORRADE_TARGET_MINGW, @ref CORRADE_TARGET_WINDOWS, @ref corrade-cmake
*/
#define CORRADE_TARGET_CLANG_CL
#undef CORRADE_TARGET_CLANG_CL

/**
@brief MSVC compiler
@m_since{2020,06}

Defined if the code is being compiled by MSVC or Clang with a MSVC frontend. If
this variable is defined, @ref CORRADE_TARGET_CLANG might also be defined.
While this variable is exposed in CMake as well, it's not guaranteed that the
reported compiler is consistent between CMake and C++ --- for example, a
library can be built with MSVC and then used via Clang-CL.

MSVC version can be accessed using the `_MSC_VER` macro. The macro uses an
[internal version numbering](https://en.wikipedia.org/wiki/Microsoft_Visual_C++#Internal_version_numbering),
so for example MSVC 2019 16.7 (14.27) is reported as 1927.
@see @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_CLANG,
    @ref CORRADE_TARGET_APPLE_CLANG, @ref CORRADE_TARGET_CLANG_CL,
    @ref CORRADE_TARGET_MINGW, @ref CORRADE_TARGET_WINDOWS, @ref corrade-cmake
*/
#define CORRADE_TARGET_MSVC
#undef CORRADE_TARGET_MSVC

/**
@brief MinGW compiler
@m_since{2020,06}

Defined if the code is being compiled by GCC / Clang running under MinGW. If
this variable is defined, @ref CORRADE_TARGET_GCC and possibly also
@ref CORRADE_TARGET_CLANG are defined. While this variable is exposed in CMake
as well, it's not guaranteed that the reported compiler is consistent between
CMake and C++ --- for example, a library can be built with MSVC and then used
via Clang-CL.
@see @ref CORRADE_TARGET_GCC, @ref CORRADE_TARGET_CLANG,
    @ref CORRADE_TARGET_APPLE_CLANG, @ref CORRADE_TARGET_CLANG_CL,
    @ref CORRADE_TARGET_MSVC, @ref CORRADE_TARGET_WINDOWS, @ref corrade-cmake
*/
#define CORRADE_TARGET_MINGW
#undef CORRADE_TARGET_MINGW

/**
@brief STL libc++ target
@m_since{2019,10}

Defined if the library is built against Clang [libc++](https://libcxx.llvm.org/)
STL implementation. This is most common on @ref CORRADE_TARGET_APPLE "Apple"
macOS and iOS and on newer @ref CORRADE_TARGET_ANDROID "Android" NDKs, it's
also sometimes used on Linux. Note that unlike other `CORRADE_TARGET_*`
variables, this variable, @ref CORRADE_TARGET_LIBSTDCXX and
@ref CORRADE_TARGET_DINKUMWARE are not exposed in CMake because the detection
is non-trivial.

Major libc++ version number can be accessed using the `_LIBCPP_VERSION` version
macro. While libc++ can only be used with Clang (and not GCC), its version
might or might not be the same as `__clang_major__`.
@see @ref CORRADE_TARGET_LIBSTDCXX, @ref CORRADE_TARGET_DINKUMWARE,
    @ref CORRADE_CXX_STANDARD, @ref CORRADE_TARGET_CLANG
*/
#define CORRADE_TARGET_LIBCXX
#undef CORRADE_TARGET_LIBCXX

/**
@brief STL libstdc++ target
@m_since{2019,10}

Defined if the library is built against GCC [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/)
STL implementation. This is most common on Linux and under MinGW, note that
Clang is able to use libstdc++ as well. Note that unlike other `CORRADE_TARGET_*`
variables, this variable, @ref CORRADE_TARGET_LIBCXX and
@ref CORRADE_TARGET_DINKUMWARE are not exposed in CMake because the detection
is non-trivial.

Major libstdc++ version number can be accessed using the `_GLIBCXX_RELEASE`
macro, however it's available only since libstdc++ 7. Older versions define
just `__GLIBCXX__`, which contains a release date (and the dates, of course,
[overlap for patch releases](https://stackoverflow.com/a/37119478) and can be
just anything for custom GCC forks). If libstdc++ is used together with GCC, it
can be assumed both have the same version; if libstdc++ is used together with
Clang, the versions can be arbitrary.
@see @ref CORRADE_TARGET_LIBCXX, @ref CORRADE_TARGET_DINKUMWARE,
    @ref CORRADE_CXX_STANDARD, @ref CORRADE_TARGET_GCC,
    @ref CORRADE_TARGET_CLANG
*/
#define CORRADE_TARGET_LIBSTDCXX
#undef CORRADE_TARGET_LIBSTDCXX

/**
@brief STL Dinkumware target
@m_since{2019,10}

Defined if the library is built against Dinkumware STL implementation (used by
MSVC). Note that Clang is able to use this implementation as well.  Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_LIBSTDCXX and  @ref CORRADE_TARGET_DINKUMWARE are not
exposed in CMake because the detection is non-trivial.

The MSVC standard library is closely tied to a compiler version, thus
`_MSC_VER` can be used to determine its version on both MSVC and Clang-CL.
@see @ref CORRADE_TARGET_LIBCXX, @ref CORRADE_TARGET_LIBSTDCXX,
    @ref CORRADE_CXX_STANDARD, @ref CORRADE_TARGET_MSVC,
    @ref CORRADE_TARGET_CLANG_CL
*/
#define CORRADE_TARGET_DINKUMWARE
#undef CORRADE_TARGET_DINKUMWARE

/**
@brief SSE2 target
@m_since{2020,06}

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Streaming SIMD Extensions 2](https://en.wikipedia.org/wiki/SSE2) are enabled
at compile time (`-msse2` or higher on GCC/Clang, `/arch:SSE2` or higher on
MSVC). All x86-64 targets support SSE2. Implied by @ref CORRADE_TARGET_SSE3.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Sse2},
    @ref CORRADE_ENABLE_SSE2
*/
#define CORRADE_TARGET_SSE2
#undef CORRADE_TARGET_SSE2

/**
@brief SSE3 target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSE3) are enabled
at compile time (on GCC/Clang it's `-msse3` and higher, MSVC doesn't have a
direct option and it's only implied by `/arch:AVX`). Superset of
@ref CORRADE_TARGET_SSE2, implied by @ref CORRADE_TARGET_SSSE3.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Sse3},
    @ref CORRADE_ENABLE_SSE3
*/
#define CORRADE_TARGET_SSE3
#undef CORRADE_TARGET_SSE3

/**
@brief SSSE3 target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Supplemental Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSSE3)
are enabled at compile time (on GCC/Clang it's `-mssse3` and higher, MSVC
doesn't have a direct option and it's only implied by `/arch:AVX`). Superset of
@ref CORRADE_TARGET_SSE3, implied by @ref CORRADE_TARGET_SSE41.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref CORRADE_TARGET_SSE41 to
detect those.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Ssse3},
    @ref CORRADE_ENABLE_SSSE3
*/
#define CORRADE_TARGET_SSSE3
#undef CORRADE_TARGET_SSSE3

/**
@brief SSE4.1 target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Streaming SIMD Extensions 4.1](https://en.wikipedia.org/wiki/SSE4#SSE4.1)
are enabled at compile time (on GCC/Clang it's `-msse4.1` and higher, MSVC
doesn't have a direct option and it's only implied by `/arch:AVX`). Superset of
@ref CORRADE_TARGET_SSSE3, implied by @ref CORRADE_TARGET_SSE42.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref CORRADE_TARGET_SSE41 to
detect those.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Sse41},
    @ref CORRADE_ENABLE_SSE41
*/
#define CORRADE_TARGET_SSE41
#undef CORRADE_TARGET_SSE41

/**
@brief SSE4.2 target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Streaming SIMD Extensions 4.2](https://en.wikipedia.org/wiki/SSE4#SSE4.2)
are enabled at compile time (on GCC/Clang it's `-msse4.2` and higher, MSVC
doesn't have a direct option and it's only implied by `/arch:AVX`). Superset of
@ref CORRADE_TARGET_SSE41, implied by @ref CORRADE_TARGET_AVX.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Sse42},
    @ref CORRADE_ENABLE_SSE42
*/
#define CORRADE_TARGET_SSE42
#undef CORRADE_TARGET_SSE42

/**
@brief Target with POPCNT instructions
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[POPCNT](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#ABM_(Advanced_Bit_Manipulation))
is enabled at compile time. On GCC/Clang it's `-mpopcnt` and is also implied by
`-msse4.2` and higher, MSVC doesn't have a direct option but it's assumed to
be implied by `/arch:AVX`. To avoid failures at runtime, prefer to detect its
presence with @ref Cpu::runtimeFeatures().
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Popcnt},
    @ref CORRADE_ENABLE_POPCNT
*/
#define CORRADE_TARGET_POPCNT
#undef CORRADE_TARGET_POPCNT

/**
@brief Target with LZCNT instructions
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[LZCNT](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#ABM_(Advanced_Bit_Manipulation))
is enabled at compile time (on GCC/Clang it's `-mlznct`, MSVC doesn't have a
direct option but it's assumed to be implied by `/arch:AVX2`). However note
that this instruction has encoding compatible with an earlier `BSR` instruction
which has a slightly different behavior. To avoid wrong results if it isn't
available, prefer to detect its presence with @ref Cpu::runtimeFeatures()
instead.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Lzcnt},
    @ref CORRADE_ENABLE_LZCNT
*/
#define CORRADE_TARGET_LZCNT
#undef CORRADE_TARGET_LZCNT

/**
@brief Target with BMI1 instructions
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[BMI1](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#BMI1_(Bit_Manipulation_Instruction_Set_1))
including the `TZCNT` instruction is enabled at compile time (on GCC/Clang it's
`-mbmi`, MSVC doesn't have a direct option but it's assumed to be implied by
`/arch:AVX2`). However note that the `TZCNT` instruction has encoding
compatible with an earlier `BSF` instruction which has a slightly different
behavior. To avoid wrong results if it isn't available, prefer to detect its
presence with @ref Cpu::runtimeFeatures() instead.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Bmi1},
    @ref CORRADE_ENABLE_BMI1
*/
#define CORRADE_TARGET_BMI1
#undef CORRADE_TARGET_BMI1

/**
@brief AVX target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Advanced Vector Extensions](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions)
are enabled at compile time (`-mavx` and higher on GCC/Clang, `/arch:AVX` on
MSVC). Superset of @ref CORRADE_TARGET_SSE42, implied by
@ref CORRADE_TARGET_AVX2.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Avx},
    @ref CORRADE_ENABLE_AVX
*/
#define CORRADE_TARGET_AVX
#undef CORRADE_TARGET_AVX

/**
@brief AVX target with F16C
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if the
[F16C instruction set](https://en.wikipedia.org/wiki/F16C) is enabled at
compile time. On GCC/Clang it's `-mf16c`, MSVC doesn't have a direct option but
it's assumed to be implied by `/arch:AVX2`. To avoid failures at runtime,
prefer to detect its presence with @ref Cpu::runtimeFeatures().
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::AvxF16c},
    @ref CORRADE_ENABLE_AVX_F16C
*/
#define CORRADE_TARGET_AVX_F16C
#undef CORRADE_TARGET_AVX_F16C

/**
@brief AVX target with FMA
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if the
[FMA3 instruction set](https://en.wikipedia.org/wiki/FMA_instruction_set) is
enabled at compile time. On GCC/Clang it's `-mfma`, MSVC doesn't have a direct
option but it's assumes to be implied by `/arch:AVX2`. To avoid failures at
runtime, prefer to detect its presence with @ref Cpu::runtimeFeatures().

The FMA4 instruction set, which used to be supported only in certain range of
AMD processors and isn't anymore, is not detected, and AMD switched to FMA3
since.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::AvxFma},
    @ref CORRADE_ENABLE_AVX_FMA
*/
#define CORRADE_TARGET_AVX_FMA
#undef CORRADE_TARGET_AVX_FMA

/**
@brief AVX2 target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if
[Advanced Vector Extensions 2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2)
are enabled at compile time (`-mavx2` and higher on GCC/Clang, `/arch:AVX2` on
MSVC). Superset of @ref CORRADE_TARGET_AVX, implied by
@ref CORRADE_TARGET_AVX512F.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Avx2},
    @ref CORRADE_ENABLE_AVX2
*/
#define CORRADE_TARGET_AVX2
#undef CORRADE_TARGET_AVX2

/**
@brief AVX-512 Foundation target
@m_since_latest

Defined on @ref CORRADE_TARGET_X86 "x86" if [AVX-512](https://en.wikipedia.org/wiki/AVX-512)
Foundation instructions are enabled at compile time (`-mavx512f` and higher on
GCC/Clang, `/arch:AVX512` on MSVC). Superset of @ref CORRADE_TARGET_AVX2.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Avx512f},
    @ref CORRADE_ENABLE_AVX512F
*/
#define CORRADE_TARGET_AVX512F
#undef CORRADE_TARGET_AVX512F

/**
@brief NEON target
@m_since_latest

Defined on @ref CORRADE_TARGET_ARM "ARM" if
[ARM NEON](https://en.wikipedia.org/wiki/ARM_architecture#Advanced_SIMD_(Neon))
instructions are enabled at compile time (`-mfpu=neon` on GCC/Clang, implicitly
supported on ARM64). Implied by @ref CORRADE_TARGET_NEON_FMA.

Apart from NEON, there's several other mutually incompatible ARM instruction
sets. Detection for these will be added when the platforms become more
widespread:

-   Helium, which is a low-power alternative to NEON
-   SVE, which is a HPC-focused alternative to NEON, in 2021 found mostly just
    on [AWS Graviton](https://aws.amazon.com/ec2/graviton/)
-   SVE2, which is a next-generation vector instruction set designed to be a
    successor to both NEON and SVE, scheduled to appear in production in around
    2022
-   AMX, which is Apple's proprietary and patented instruction set available
    only on their own hardware

@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Neon},
    @ref CORRADE_ENABLE_NEON
*/
#define CORRADE_TARGET_NEON
#undef CORRADE_TARGET_NEON

/**
@brief NEON target with FMA
@m_since_latest

Defined on @ref CORRADE_TARGET_ARM "ARM" if NEON FMA instructions are enabled
at compile time (`-mfpu=neon-vfpv4` on GCC/Clang on 32-bit ARM, implicitly
supported on ARM64). Not defined if FMA is only available for scalar code and
not for NEON. Superset of @ref CORRADE_TARGET_NEON, implied by
@ref CORRADE_TARGET_NEON_FP16.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::NeonFma},
    @ref CORRADE_ENABLE_NEON_FMA
*/
#define CORRADE_TARGET_NEON_FMA
#undef CORRADE_TARGET_NEON_FMA

/**
@brief NEON target with FP16 vector arithmetic
@m_since_latest

Defined on @ref CORRADE_TARGET_ARM "ARM" if ARMv8.2-a NEON FP16 vector
arithmetic support is enabled at compile time (`-march=armv8.2-a+fp16` on
GCC/Clang). Superset of @ref CORRADE_TARGET_NEON_FMA.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::NeonFp16},
    @ref CORRADE_ENABLE_NEON_FP16
*/
#define CORRADE_TARGET_NEON_FP16
#undef CORRADE_TARGET_NEON_FP16

/**
@brief SIMD128 target
@m_since_latest

Defined on @ref CORRADE_TARGET_WASM "WebAssembly" if
[128-bit SIMD](https://github.com/webassembly/simd) instructions are enabled at
compile time (`-msimd128` passed to Clang), and the compiler supports the
finalized version of the intrinsics, which is since Clang 13 and Emscripten
2.0.18. Emscripten SDK 2.0.13 to 2.0.17 ship with a Clang that reports as 13
but isn't actually the final version.
@see @relativeref{Corrade,Cpu}, @relativeref{Corrade,Cpu::Simd128},
    @ref CORRADE_ENABLE_SIMD128
*/
#define CORRADE_TARGET_SIMD128
#undef CORRADE_TARGET_SIMD128

/**
@brief GNU IFUNC is allowed to be used for runtime dispatch in the Cpu library
@m_since_latest

Defined if the @relativeref{Corrade,Cpu} library can perform runtime dispatch
using [GNU IFUNC](https://sourceware.org/glibc/wiki/GNU_IFUNC), exposing the
@ref CORRADE_CPU_DISPATCHED_IFUNC() macro. Supported only on Linux with glibc
and on Android with API 30+. See @ref Cpu-usage-automatic-cached-dispatch for
details and information about performance tradeoffs.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_CPU_USE_IFUNC
#undef CORRADE_CPU_USE_IFUNC

/**
@brief PluginManager doesn't have dynamic plugin support on this platform

Defined if the @ref Corrade::PluginManager "PluginManager" library doesn't
support dynamic plugin loading due to platform limitations. Defined on
@ref CORRADE_TARGET_EMSCRIPTEN "Emscripten", @ref CORRADE_TARGET_WINDOWS_RT "Windows RT",
@ref CORRADE_TARGET_IOS "iOS" and @ref CORRADE_TARGET_ANDROID "Android".
*/
#define CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#undef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT

/**
@brief Target XCTest with TestSuite

Defined if the @ref Corrade::TestSuite "TestSuite" library is targeting Xcode
XCTest. Available only on Apple platforms. Enabled using
`CORRADE_TESTSUITE_TARGET_XCTEST` CMake option when building Corrade.
@see @ref CORRADE_TARGET_APPLE, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TESTSUITE_TARGET_XCTEST
#undef CORRADE_TESTSUITE_TARGET_XCTEST

/**
@brief Use ANSI escape sequences for colored debug output on Windows

By default colored output using @ref Corrade::Utility::Debug "Utility::Debug"
on Windows is done using WINAPI that has a limited functionality, because ANSI
escape sequences are supported only on Windows 10 or when using non-standard
console emulators. Available only on Windows, all other platforms use ANSI
sequences implicitly. Enabled using `CORRADE_UTILITY_USE_ANSI_COLORS` CMake
option when building Corrade.

Note that on Windows 10 you need to additionally enable ANSI color support in
the console. This is done automatically when you link to the
@ref main "Corrade Main library".
@see @ref CORRADE_TARGET_WINDOWS, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_UTILITY_USE_ANSI_COLORS
#undef CORRADE_UTILITY_USE_ANSI_COLORS
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Cpu {
    class Features;
}
#endif

}

#endif
