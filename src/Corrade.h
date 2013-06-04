#ifndef Corrade_Corrade_h
#define Corrade_Corrade_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "corradeCompatibility.h"

/** @todoc remove trailing underscores when Doxygen can handle `undef` */

namespace Corrade {

#ifdef DOXYGEN_GENERATING_OUTPUT

/**
@brief GCC 4.7 compatibility

`CORRADE_GCC47_COMPATIBILITY` is defined if compatibility mode for GCC 4.7 is
enabled.
@see @ref building-corrade
*/
#define CORRADE_GCC47_COMPATIBILITY_

/**
@brief GCC 4.6 compatibility

`CORRADE_GCC46_COMPATIBILITY` is defined if compatibility mode for GCC 4.6 is
enabled.
@see @ref building-corrade
*/
#define CORRADE_GCC46_COMPATIBILITY_

/**
@brief Static library build

`CORRADE_BUILD_STATIC` is defined if built as static libraries. Default are
shared libraries.
@see @ref building-corrade
*/
#define CORRADE_BUILD_STATIC_

/**
@brief Google Chrome Native Client target

`CORRADE_TARGET_NACL` is defined if the library is built for
[Google Chrome Native Client](https://developers.google.com/native-client/).
@see @ref building-corrade
*/
#define CORRADE_TARGET_NACL_

/**
@brief Google Chrome Native Client target with `newlib` toolchain

`CORRADE_TARGET_NACL_NEWLIB` is defined if the library is built for Google
Chrome Native Client with `newlib` toolchain.
@see @ref building-corrade
*/
#define CORRADE_TARGET_NACL_NEWLIB_

/**
@brief Google Chrome Native Client target with `glibc` toolchain

`CORRADE_TARGET_NACL_GLIBC` is defined if the library is built for Google
Chrome Native Client with `glibc` toolchain.
@see @ref building-corrade
*/
#define CORRADE_TARGET_NACL_GLIBC_

/**
@brief Emscripten target

`CORRADE_TARGET_EMSCRIPTEN` is defined if the library is built for
[Emscripten](https://github.com/kripken/emscripten/wiki).
@see @ref building-corrade
*/
#define CORRADE_TARGET_EMSCRIPTEN_

#endif

}

#endif
