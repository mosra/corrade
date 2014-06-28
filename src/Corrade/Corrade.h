#ifndef Corrade_Corrade_h
#define Corrade_Corrade_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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
 * @brief Basic definitions
 */

#include "Corrade/compatibility.h"

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
@brief GCC 4.6 compatibility

Defined if compatibility mode for GCC 4.6 is enabled.
@see @ref building-corrade
*/
#define CORRADE_GCC46_COMPATIBILITY
#undef CORRADE_GCC46_COMPATIBILITY

/**
@brief GCC 4.5 compatibility

Defined if compatibility mode for GCC 4.5 is enabled.
@see @ref building-corrade
*/
#define CORRADE_GCC45_COMPATIBILITY
#undef CORRADE_GCC45_COMPATIBILITY

/**
@brief GCC 4.4 compatibility

Defined if compatibility mode for GCC 4.4 is enabled.
@see @ref building-corrade
*/
#define CORRADE_GCC44_COMPATIBILITY
#undef CORRADE_GCC44_COMPATIBILITY

/**
@brief MSVC 2013 compatibility

Defined if compatibility mode for MSVC 2013 is enabled.
@see @ref building-corrade
*/
#define CORRADE_MSVC2013_COMPATIBILITY
#undef CORRADE_MSVC2013_COMPATIBILITY

/**
@brief Build with deprecated API included

Defined if the library contains deprecated API (which will be removed in the
future). To preserve backward compatibility, %Corrade is by default built with
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

Defined if the library is built for some Unix flavor (Linux, BSD, OS X...).
@see @ref CORRADE_TARGET_APPLE, @ref corrade-cmake
*/
#define CORRADE_TARGET_UNIX
#undef CORRADE_TARGET_UNIX

/**
@brief Apple target

Defined if the library is built for OS X.
@see @ref CORRADE_TARGET_UNIX, @ref corrade-cmake
*/
#define CORRADE_TARGET_APPLE
#undef CORRADE_TARGET_APPLE

/**
@brief Windows target

Defined if the library is built for Windows.
@see @ref corrade-cmake
*/
#define CORRADE_TARGET_WINDOWS
#undef CORRADE_TARGET_WINDOWS

/**
@brief Google Chrome Native Client target

Defined if the library is built for
[Google Chrome Native Client](https://developers.google.com/native-client/).
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_NACL
#undef CORRADE_TARGET_NACL

/**
@brief Google Chrome Native Client target with `newlib` toolchain

Defined if the library is built for Google Chrome Native Client with `newlib`
toolchain.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_NACL_NEWLIB
#undef CORRADE_TARGET_NACL_NEWLIB

/**
@brief Google Chrome Native Client target with `glibc` toolchain

Defined if the library is built for Google Chrome Native Client with `glibc`
toolchain.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_NACL_GLIBC
#undef CORRADE_TARGET_NACL_GLIBC

/**
@brief Emscripten target

Defined if the library is built for [Emscripten](https://github.com/kripken/emscripten/wiki).
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_EMSCRIPTEN
#undef CORRADE_TARGET_EMSCRIPTEN

/**
@brief Android target

Defined if the library is built for Android.
@see @ref building-corrade, @ref corrade-cmake
*/
#define CORRADE_TARGET_ANDROID
#undef CORRADE_TARGET_ANDROID
#endif

}

#endif
