#ifndef Corrade_Corrade_h
#define Corrade_Corrade_h
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

/** @file
 * @brief Basic definitions
 */

#include "Corrade/configure.h"

namespace Corrade {

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief MSVC 2019 compatibility

Defined if compatibility mode for MSVC 2019 is enabled.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_MSVC2019_COMPATIBILITY
#undef CORRADE_MSVC2019_COMPATIBILITY

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
#define CORRADE_BUILD_DEPRECATED
/* (enabled by default) */

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
certain Corrade features simultaenously in multiple threads. In particular:

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
-   greater than @cpp 201703 @ce when C++2a is used

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
@ref CORRADE_TARGET_ARM, @ref CORRADE_TARGET_POWERPC and
@ref CORRADE_TARGET_BIG_ENDIAN are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM nor
@ref CORRADE_TARGET_POWERPC is defined, the platform might be either
@ref CORRADE_TARGET_EMSCRIPTEN or any other that the library doesn't know about
yet.
*/
#define CORRADE_TARGET_X86
#undef CORRADE_TARGET_X86

/**
@brief ARM target

Defined if the library is built for ARM platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_POWERPC and
@ref CORRADE_TARGET_BIG_ENDIAN are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM nor
@ref CORRADE_TARGET_POWERPC is defined, the platform might be either
@ref CORRADE_TARGET_EMSCRIPTEN or any other that the library doesn't know about
yet.
*/
#define CORRADE_TARGET_ARM
#undef CORRADE_TARGET_ARM

/**
@brief PowerPC target
@m_since{2019,10}

Defined if the library is built for PowerPC platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM and
@ref CORRADE_TARGET_BIG_ENDIAN are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM nor
@ref CORRADE_TARGET_POWERPC is defined, the platform might be either
@ref CORRADE_TARGET_EMSCRIPTEN or any other that the library doesn't know about
yet.
@see @ref CORRADE_TARGET_BIG_ENDIAN
*/
#define CORRADE_TARGET_POWERPC
#undef CORRADE_TARGET_POWERPC

/**
@brief Big-Endian target
@m_since{2020,06}

Defined when the platform defaults to Big-Endian (such as HP/PA RISC, Motorola
68k, Big-Endian MIPS, PowerPC and SPARC). Not defined on Little-Endian
platforms (such as x86 and ARM). Note that some platforms are Bi-Endian,
meaning the endianness can be switched at runtime (and thus can't be detected
at compile-time), this macro only reflects the usual architecture default.
Moreover, unlike other `CORRADE_TARGET_*` variables, this variable,
@ref CORRADE_TARGET_X86, @ref CORRADE_TARGET_ARM and
@ref CORRADE_TARGET_POWERPC are not exposed in CMake because the meaning is
unclear on platforms with multi-architecture binaries.
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
@see @ref CORRADE_TARGET_LIBCXX, @ref CORRADE_TARGET_LIBSTDCXX,
    @ref CORRADE_CXX_STANDARD, @ref CORRADE_TARGET_MSVC
*/
#define CORRADE_TARGET_DINKUMWARE
#undef CORRADE_TARGET_DINKUMWARE

/**
@brief SSE2 target
@m_since{2020,06}

Defined on x86 if [SSE2](https://en.wikipedia.org/wiki/SSE2) instructions are
enabled at compile time (`-msse2` or higher on GCC/Clang, `/arch:SSE2` or
higher on MSVC). Nowadays it can be assumed that all x86 targets support SSE2,
so (unlike AVX and newer) this feature doesn't need any runtime detection.
*/
#define CORRADE_TARGET_SSE2
#undef CORRADE_TARGET_SSE2

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
XCTest. Available only on Apple platforms. Enabled using `TESTSUITE_TARGET_XCTEST`
CMake option when building Corrade.
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
sequences implicitly. Enabled using `UTILITY_USE_ANSI_COLORS` CMake option when
building Corrade.

Note that on Windows 10 you need to additionally enable ANSI color support in
the console. This is done automatically when you link to the
@ref main "Corrade Main library".
@see @ref CORRADE_TARGET_WINDOWS, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_UTILITY_USE_ANSI_COLORS
#undef CORRADE_UTILITY_USE_ANSI_COLORS
#endif

}

#endif
