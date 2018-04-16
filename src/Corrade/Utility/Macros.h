#ifndef Corrade_Utility_Macros_h
#define Corrade_Utility_Macros_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Macro @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(), @ref CORRADE_IGNORE_DEPRECATED_PUSH, @ref CORRADE_IGNORE_DEPRECATED_POP, @ref CORRADE_UNUSED, @ref CORRADE_ALIGNAS(), @ref CORRADE_AUTOMATIC_INITIALIZER(), @ref CORRADE_AUTOMATIC_FINALIZER()
 */

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Internal macro implementation */
#define _CORRADE_HELPER_PASTE2(a, b) a ## b
#define _CORRADE_HELPER_PASTE(a, b) _CORRADE_HELPER_PASTE2(a, b)
#define _CORRADE_HELPER_STR(X) #X
#define _CORRADE_HELPER_DEFER(M, ...) M(__VA_ARGS__)
#endif

/** @hideinitializer
@brief Deprecation mark

Marked function, class or typedef will emit deprecation warning on supported
compilers (GCC, Clang, MSVC):

@snippet Utility.cpp CORRADE_DEPRECATED

Might not work for template aliases, namespaces and enum values on all
compilers, use @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE()
and @ref CORRADE_DEPRECATED_ENUM() instead. See @ref CORRADE_DEPRECATED_FILE()
for file-level deprecation.
*/
#if defined(__GNUC__) || defined(__clang__)
#define CORRADE_DEPRECATED(message) __attribute((deprecated(message)))
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED(message) __declspec(deprecated(message))
#else
#define CORRADE_DEPRECATED(message)
#endif

/** @hideinitializer
@brief Alias deprecation mark

Marked alias will emit deprecation warning on supported compilers (GCC, Clang,
MSVC 2017):

@snippet Utility.cpp CORRADE_DEPRECATED_ALIAS

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_NAMESPACE(),
    @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__GNUC__) || defined(__clang__)
#define CORRADE_DEPRECATED_ALIAS(message) __attribute((deprecated(message)))
#elif defined(_MSC_VER) && _MSC_VER >= 1910
#define CORRADE_DEPRECATED_ALIAS(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_ALIAS(message)
#endif

/** @hideinitializer
@brief Namespace deprecation mark

Marked enum or enum value will emit deprecation warning on supported compilers
(C++17 feature, MSVC 2015 and Clang):

@snippet Utility.cpp CORRADE_DEPRECATED_NAMESPACE

GCC claims support since version 4.9, but even in version 7.3 it only emits an
"attribute ignored" warning at the declaration location no diagnostic when such
namespace is used --- which is practically useless
([source](https://stackoverflow.com/q/46052410)).

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__clang__)
/* Clang < 6.0 warns that this is a C++14 extension, Clang 6.0 warns that this
   is a C++17 extension. Clang < 6.0 doesn't know -Wc++17-extensions, so can't
   disable both in the same macro. YAY!! */
#if __clang__major__ < 6
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#else
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++17-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#endif
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED_NAMESPACE(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_NAMESPACE(message)
#endif

/** @hideinitializer
@brief Enum deprecation mark

Marked enum or enum value will emit deprecation warning on supported compilers
(C++17 feature, MSVC 2015, Clang and GCC 6+):

@snippet Utility.cpp CORRADE_DEPRECATED_ENUM

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_FILE()
*/
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 6)
#define CORRADE_DEPRECATED_ENUM(message) __attribute((deprecated(message)))
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED_ENUM(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_ENUM(message)
#endif

/** @hideinitializer
@brief File deprecation mark

Putting this in a file will emit deprecation warning when given file is
included or compiled (GCC, Clang):

@code{.cpp}
CORRADE_DEPRECATED_FILE("use Bar.h instead") // yes, no semicolon at the end
@endcode

Note that the warning is suppressed in case given directory is included as
system (`-isystem` on GCC and Clang).
@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM()
*/
#if defined(__clang__)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this file is deprecated: " message)))
#elif defined(__GNUC__)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#else
#define CORRADE_DEPRECATED_FILE(message)
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
@ref CORRADE_DEPRECATED_ENUM() are suppressed. Doesn't suppress
@ref CORRADE_DEPRECATED_FILE() warnings.
*/
#ifdef __GNUC__
#define CORRADE_IGNORE_DEPRECATED_PUSH _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#elif defined(_MSC_VER)
#define CORRADE_IGNORE_DEPRECATED_PUSH __pragma(warning(push)) __pragma(warning(disable: 4996))
#else
#define CORRADE_IGNORE_DEPRECATED_PUSH
#endif

/** @hideinitializer
@brief End code section with deprecation warnings ignored

See @ref CORRADE_IGNORE_DEPRECATED_PUSH for more information.
*/
#ifdef __GNUC__
#define CORRADE_IGNORE_DEPRECATED_POP _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define CORRADE_IGNORE_DEPRECATED_POP __pragma(warning(pop))
#else
#define CORRADE_IGNORE_DEPRECATED_POP
#endif

/** @hideinitializer
@brief Unused variable mark

Putting this before unused variable will suppress compiler warning about it
being unused. If possible, use @cpp static_cast<void>(var) @ce or nameless
function parameters instead.

@snippet Utility.cpp CORRADE_UNUSED
*/
#if defined(__GNUC__)
#define CORRADE_UNUSED __attribute__((__unused__))
#elif defined(_MSC_VER)
#define CORRADE_UNUSED __pragma(warning(suppress:4100))
#else
#define CORRADE_UNUSED
#endif

/** @hideinitializer
@brief Type alignment specifier

Expands to C++11 @cpp alignas() @ce specifier on supported compilers, otherwise
falls back to compiler-specific attribute. Example usage:

@snippet Utility.cpp CORRADE_ALIGNAS
*/
#if defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ < 408
#define CORRADE_ALIGNAS(alignment) __attribute__((aligned(alignment)))
#else
#define CORRADE_ALIGNAS(alignment) alignas(alignment)
#endif

/** @hideinitializer
@brief Noreturn fuction attribute

Expands to C++11 @cpp [[noreturn]] @ce attribute on supported compilers,
otherwise falls back to compiler-specific attribute. Example usage:

@snippet Utility.cpp CORRADE_NORETURN
*/
#if defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ < 408
#define CORRADE_NORETURN __attribute__((noreturn))
#else
#define CORRADE_NORETURN [[noreturn]]
#endif

/** @hideinitializer
@brief C++ standard version

Expands to `__cplusplus` macro on all sane compilers; on MSVC uses `_MSVC_LANG`
if defined (since Visual Studio 2015 Update 3), otherwise reports C++11. The
returned version is:

-   @cpp 201703 @ce when C++17 is used
-   @cpp 201402 @ce when C++14 is used
-   @cpp 201103 @ce when C++11 is used
*/
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#define CORRADE_CXX_STANDARD _MSVC_LANG
#else
#define CORRADE_CXX_STANDARD 201103L
#endif
#else
#define CORRADE_CXX_STANDARD __cplusplus
#endif

/** @hideinitializer
@brief Automatic initializer
@param function Initializer function name of type @cpp int(*)() @ce.

Function passed as argument will be called even before entering @cpp main() @ce
function. This is usable when e.g. automatically registering plugins or data
resources without forcing the user to write additional code in @cpp main() @ce.
@attention This macro does nothing in static libraries.
*/
#define CORRADE_AUTOMATIC_INITIALIZER(function)                             \
    namespace {                                                             \
        struct Initializer_##function { static const int i; };              \
        const int Initializer_##function::i = function();                   \
    }

/** @hideinitializer
@brief Automatic initializer
@param function Finalizer function name of type @cpp int(*)() @ce.

Function passed as argument will be called even before entering @cpp main() @ce
function. This is usable in conjuction with @ref CORRADE_AUTOMATIC_INITIALIZER()
when there is need to properly discard initialized data.
@attention This macro does nothing in static libraries.
*/
#define CORRADE_AUTOMATIC_FINALIZER(function)                               \
    class Finalizer_##function {                                            \
        public:                                                             \
            Finalizer_##function() {}                                       \
            ~Finalizer_##function() { function(); }                         \
    } Finalizer_##function;

#endif
