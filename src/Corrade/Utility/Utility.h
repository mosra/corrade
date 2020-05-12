#ifndef Corrade_Utility_Utility_h
#define Corrade_Utility_Utility_h
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
 * @brief Forward declarations for the @ref Corrade::Utility namespace
 */

#include <cstdint>
#include <cstddef>

#include "Corrade/configure.h"
#include "Corrade/Containers/Containers.h"

namespace Corrade { namespace Utility {

class Arguments;

template<std::size_t> class HashDigest;
/* AbstractHash is not used directly */

class Configuration;
class ConfigurationGroup;
enum class ConfigurationValueFlag: std::uint8_t;
typedef Containers::EnumSet<ConfigurationValueFlag> ConfigurationValueFlags;
template<class> struct ConfigurationValue;
#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
class FileWatcher;
#endif

class Debug;
class Warning;
class Error;
class Fatal;

/* Endianness used only statically */
class MurmurHash2;

class Resource;
class Sha1;
class Translator;

#if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
/* Tweakable doesn't need forward declaration */
template<class> struct TweakableParser;
enum class TweakableState: std::uint8_t;
#endif

}}

#endif
