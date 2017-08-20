#ifndef Corrade_Corrade_h
#define Corrade_Corrade_h
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

/** @file
 * @brief Basic definitions
 */

#include "Corrade/configure.h"

namespace Corrade {

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief GCC 4.7 compatibility

Defined if compatibility mode for GCC 4.7 is enabled.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_GCC47_COMPATIBILITY
#undef CORRADE_GCC47_COMPATIBILITY

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
@brief Unix target

Defined if the library is built for some Unix flavor (Linux, BSD, macOS, iOS,
Android...).
@see @ref CORRADE_TARGET_APPLE, @ref CORRADE_TARGET_ANDROID, @ref corrade-cmake
*/
#define CORRADE_TARGET_UNIX
#undef CORRADE_TARGET_UNIX

/**
@brief Apple target

Defined if the library is built for Apple platforms (macOS, iOS).
@see @ref CORRADE_TARGET_UNIX, @ref CORRADE_TARGET_IOS, @ref corrade-cmake
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
@see @ref CORRADE_TARGET_WINDOWS_RT, @ref corrade-cmake
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
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_EMSCRIPTEN
#undef CORRADE_TARGET_EMSCRIPTEN

/**
@brief Android target

Defined if the library is built for Android.
@see @ref CORRADE_TARGET_UNIX, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_ANDROID
#undef CORRADE_TARGET_ANDROID

/**
@brief x86 target

Defined if the library is built for x86 platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable and
@ref CORRADE_TARGET_ARM are not exposed in CMake because the meaning is unclear
on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86 nor @ref CORRADE_TARGET_ARM is defined, the platform
might be either @ref CORRADE_TARGET_EMSCRIPTEN or any other that the library
doesn't know about yet.
*/
#define CORRADE_TARGET_X86
#undef CORRADE_TARGET_X86

/**
@brief ARM target

Defined if the library is built for ARM platforms (32 or 64-bit). Note that
unlike other `CORRADE_TARGET_*` variables, this variable and
@ref CORRADE_TARGET_X86 are not exposed in CMake because the meaning is unclear
on platforms with multi-architecture binaries. If neither
@ref CORRADE_TARGET_X86 nor @ref CORRADE_TARGET_ARM is defined, the platform
might be either @ref CORRADE_TARGET_EMSCRIPTEN or any other that the library
doesn't know about yet.
*/
#define CORRADE_TARGET_ARM
#undef CORRADE_TARGET_ARM

/**
@brief Use ANSI escape sequences for colored Debug output on Windows

By default colored output using @ref Corrade::Utility::Debug "Utility::Debug"
on Windows is done using WINAPI that has a limited functionality, because ANSI
escape sequences are supported only on Windows 10 or when using non-standard
console emulators. Available only on Windows, all other platforms use ANSI
sequences implicitly. Enabled using `UTILITY_USE_ANSI_COLORS` CMake option when
building Corrade.
@see @ref CORRADE_TARGET_WINDOWS, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_UTILITY_USE_ANSI_COLORS
#undef CORRADE_UTILITY_USE_ANSI_COLORS

/**
@brief Target XCTest with TestSuite

Defined if the @ref Corrade::TestSuite "TestSuite" library is targeting Xcode
XCTest. Available only on Apple platforms. Enabled using `TESTSUITE_TARGET_XCTEST`
CMake option when building Corrade.
@see @ref CORRADE_TARGET_APPLE, @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TESTSUITE_TARGET_XCTEST
#undef CORRADE_TESTSUITE_TARGET_XCTEST
#endif

}

#endif
