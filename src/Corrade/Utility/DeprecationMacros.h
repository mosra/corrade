#ifndef Corrade_Utility_DeprecationMacros_h
#define Corrade_Utility_DeprecationMacros_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
 * @brief Macro @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(), @ref CORRADE_DEPRECATED_MACRO(), @ref CORRADE_IGNORE_DEPRECATED_PUSH, @ref CORRADE_IGNORE_DEPRECATED_POP
 * @m_since_latest
 */

#include "Corrade/configure.h"

/* Used by CORRADE_DEPRECATED_FILE() and CORRADE_DEPRECATED_MACRO(), defined in
   Macros.h as well. Don't want cyclic header dependency so defining twice. */
#if !defined(DOXYGEN_GENERATING_OUTPUT) && !defined(_CORRADE_HELPER_STR) && !defined(CORRADE_DEPRECATED_FILE) && !defined(CORRADE_DEPRECATED_MACRO)
#define _CORRADE_HELPER_STR(x) #x
#endif

/** @hideinitializer
@brief Deprecation mark

Marked function, class or typedef will emit deprecation warning on supported
compilers (GCC, Clang, MSVC):

@snippet Utility.cpp CORRADE_DEPRECATED

Might not work for template aliases, namespaces and enum values on all
compilers, use @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE()
and @ref CORRADE_DEPRECATED_ENUM() instead. See @ref CORRADE_DEPRECATED_FILE()
for file-level deprecation and @ref CORRADE_DEPRECATED_MACRO() for deprecating
macros.

Note that on MSVC and GCC this doesn't warn when using nested names of
deprecated classes or typedefs, only when the type is instantiated --- i.e.,
@cpp DeprecatedStruct a; @ce will warn, but @cpp DeprecatedStruct::Value @ce
will not.
*/
#if !defined(CORRADE_DEPRECATED) || defined(DOXYGEN_GENERATING_OUTPUT)
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG)
#define CORRADE_DEPRECATED(message) __attribute((deprecated(message)))
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_DEPRECATED(message) __declspec(deprecated(message))
#else
#define CORRADE_DEPRECATED(message)
#endif
#endif

/** @hideinitializer
@brief Alias deprecation mark

Marked alias will emit deprecation warning on supported compilers (GCC, Clang,
MSVC 2017+):

@snippet Utility.cpp CORRADE_DEPRECATED_ALIAS

Note that on MSVC and GCC this doesn't warn when using nested names of
deprecated aliases, only when the type is instantiated --- i.e.,
@cpp DeprecatedAlias a; @ce will warn, but @cpp DeprecatedAlias::Value @ce
will not.

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_NAMESPACE(),
    @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(),
    @ref CORRADE_DEPRECATED_MACRO()
*/
#if !defined(CORRADE_DEPRECATED_ALIAS) || defined(DOXYGEN_GENERATING_OUTPUT)
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG)
#define CORRADE_DEPRECATED_ALIAS(message) __attribute((deprecated(message)))
#elif defined(CORRADE_TARGET_MSVC) && _MSC_VER >= 1910
#define CORRADE_DEPRECATED_ALIAS(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_ALIAS(message)
#endif
#endif

/** @hideinitializer
@brief Namespace deprecation mark

Marked enum or enum value will emit deprecation warning on supported compilers
(C++17 feature, MSVC and Clang, GCC 10+):

@snippet Utility.cpp CORRADE_DEPRECATED_NAMESPACE

Note that this doesn't work on namespace aliases (i.e., marking
@cpp namespace Bar = Foo; @ce with this macro will result in a compile error.
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(),
    @ref CORRADE_DEPRECATED_MACRO()
*/
#if !defined(CORRADE_DEPRECATED_NAMESPACE) || defined(DOXYGEN_GENERATING_OUTPUT)
#if defined(CORRADE_TARGET_CLANG)
/* Clang < 6.0 warns that this is a C++14 extension, Clang 6.0+ warns that
   namespace attributes are a C++17 extension and deprecated attribute is a
   C++14 extension. Clang < 6.0 doesn't know -Wc++17-extensions, so can't
   disable both in the same macro. Also, Apple has its own versioning and Clang
   6 maps to AppleClang 10. */
#if (!defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__ < 6) || (defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__ < 10)
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#else
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"") _Pragma("GCC diagnostic ignored \"-Wc++17-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#endif
#elif defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 10)
#define CORRADE_DEPRECATED_NAMESPACE(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_NAMESPACE(message)
#endif
#endif

/** @hideinitializer
@brief Enum deprecation mark

Marked enum or enum value will emit deprecation warning on supported compilers
(C++17 feature, MSVC 2017+, Clang and GCC 6+):

@snippet Utility.cpp CORRADE_DEPRECATED_ENUM

Note that on MSVC and GCC this doesn't warn when using values of deprecated
enums, only when the enum is instantiated --- i.e., @cpp DeprecatedEnum e; @ce
will warn, but @cpp DeprecatedEnum::Value @ce will not. Moreover, on MSVC this
doesn't warn when a enum value marked as deprecated is used. MSVC 2015 supports
the annotation, but ignores it for both enums and enum values.

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_FILE(),
    @ref CORRADE_DEPRECATED_MACRO()
*/
#if !defined(CORRADE_DEPRECATED_ENUM) || defined(DOXYGEN_GENERATING_OUTPUT)
/* Qt's Meta Object Compiler doesn't recognize enum value attributes in older
   Qt. Not sure what version was it fixed in as the bugreports don't tell
   (https://bugreports.qt.io/browse/QTBUG-78820) so disabling it for MOC
   altogether */
#if (defined(CORRADE_TARGET_CLANG) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 6)) && !defined(Q_MOC_RUN)
#define CORRADE_DEPRECATED_ENUM(message) __attribute((deprecated(message)))
/* Enabled even for MSVC 2015 because it silently ignores it (lol) */
#elif defined(CORRADE_TARGET_MSVC) && !defined(Q_MOC_RUN)
#define CORRADE_DEPRECATED_ENUM(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_ENUM(message)
#endif
#endif

/** @hideinitializer
@brief File deprecation mark

Putting this in a file will emit deprecation warning when given file is
included or compiled (GCC, Clang, MSVC):

@code{.cpp}
CORRADE_DEPRECATED_FILE("use Bar.h instead") // yes, no semicolon at the end
@endcode

On Clang the message is prepended with *this file is deprecated*, which is not
possible on GCC. Note that the warning is suppressed in case given
directory is included as system (`-isystem` on GCC and Clang).

On MSVC the message is prepended with *warning: &lt;file&gt; is deprecated*.
The message just appears in the log output without any association to a
particular file, so the file is included in the message. Due to MSVC
limitations, the message doesn't contribute to the warning log or warning count
in any way.
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM(),
    @ref CORRADE_DEPRECATED_MACRO()
*/
#if !defined(CORRADE_DEPRECATED_FILE) || defined(DOXYGEN_GENERATING_OUTPUT)
#ifdef CORRADE_TARGET_CLANG
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this file is deprecated: " message)))
#elif defined(CORRADE_TARGET_GCC)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_DEPRECATED_FILE(_message) __pragma(message ("warning: " __FILE__ " is deprecated: " _message))
#else
#define CORRADE_DEPRECATED_FILE(message)
#endif
#endif

/** @hideinitializer
@brief Macro deprecation mark

Putting this in a macro definition will emit deprecation warning when given
macro is used (GCC, Clang, MSVC):

@code{.cpp}
#define MAKE_FOO(args) \
    CORRADE_DEPRECATED_MACRO(MAKE_FOO(),"use MAKE_BAR() instead") MAKE_BAR(args)
@endcode

On Clang and MSVC the message is prepended with *this macro is deprecated*,
which is not possible on GCC.

On MSVC the message is prepended with *&lt;file&gt; warning: &lt;macro&gt; is deprecated*,
where the macro name is taken from the first argument. The message just appears
in the log output without any association to a particular file, so the file is
included in the message. Due to MSVC limitations, the message doesn't
contribute to the warning log or warning count in any way.
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM(),
    @ref CORRADE_DEPRECATED_FILE()
*/
#ifndef CORRADE_DEPRECATED_MACRO
#ifdef CORRADE_TARGET_CLANG
#define CORRADE_DEPRECATED_MACRO(macro,message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this macro is deprecated: " message)))
#elif defined(CORRADE_TARGET_GCC)
#define CORRADE_DEPRECATED_MACRO(macro,message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_DEPRECATED_MACRO(macro,_message) __pragma(message (__FILE__ ": warning: " _CORRADE_HELPER_STR(macro) " is deprecated: " _message))
#else
#define CORRADE_DEPRECATED_MACRO(macro,message)
#endif
#endif

/** @hideinitializer
@brief Begin code section with deprecation warnings ignored

Suppresses compiler warnings when using a deprecated API (GCC, Clang, MSVC).
Useful when testing or writing APIs that depend on deprecated functionality.
In order to avoid warning suppressions to leak, for every
@ref CORRADE_IGNORE_DEPRECATED_PUSH there has to be a corresponding
@ref CORRADE_IGNORE_DEPRECATED_POP. Example usage:

@snippet Utility.cpp CORRADE_IGNORE_DEPRECATED

In particular, warnings from @ref CORRADE_DEPRECATED(),
@ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE() and
@ref CORRADE_DEPRECATED_ENUM() are suppressed. The
@ref CORRADE_DEPRECATED_FILE() and @ref CORRADE_DEPRECATED_MACRO() warnings are
suppressed only on Clang.
*/
#ifndef CORRADE_IGNORE_DEPRECATED_PUSH
#ifdef CORRADE_TARGET_CLANG
#define CORRADE_IGNORE_DEPRECATED_PUSH _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") _Pragma("GCC diagnostic ignored \"-W#pragma-messages\"")
#elif defined(CORRADE_TARGET_GCC)
#define CORRADE_IGNORE_DEPRECATED_PUSH _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_IGNORE_DEPRECATED_PUSH __pragma(warning(push)) __pragma(warning(disable: 4996))
#else
#define CORRADE_IGNORE_DEPRECATED_PUSH
#endif
#endif

/** @hideinitializer
@brief End code section with deprecation warnings ignored

See @ref CORRADE_IGNORE_DEPRECATED_PUSH for more information.
*/
#ifndef CORRADE_IGNORE_DEPRECATED_POP
#ifdef CORRADE_TARGET_GCC
#define CORRADE_IGNORE_DEPRECATED_POP _Pragma("GCC diagnostic pop")
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_IGNORE_DEPRECATED_POP __pragma(warning(pop))
#else
#define CORRADE_IGNORE_DEPRECATED_POP
#endif
#endif

#endif
