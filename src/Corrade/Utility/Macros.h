#ifndef Corrade_Utility_Macros_h
#define Corrade_Utility_Macros_h
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
 * @brief Macro @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(), @ref CORRADE_DEPRECATED_NAMESPACE(), @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(), @ref CORRADE_DEPRECATED_MACRO(), @ref CORRADE_IGNORE_DEPRECATED_PUSH, @ref CORRADE_IGNORE_DEPRECATED_POP, @ref CORRADE_UNUSED, @ref CORRADE_ALIGNAS(), @ref CORRADE_NORETURN, @ref CORRADE_FALLTHROUGH, @ref CORRADE_THREAD_LOCAL, @ref CORRADE_CONSTEXPR14, @ref CORRADE_ALWAYS_INLINE, @ref CORRADE_NEVER_INLINE, @ref CORRADE_FUNCTION, @ref CORRADE_LINE_STRING, @ref CORRADE_AUTOMATIC_INITIALIZER(), @ref CORRADE_AUTOMATIC_FINALIZER()
 */

#include "Corrade/configure.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Internal macro implementation */
#define _CORRADE_HELPER_PASTE2(a, b) a ## b
#define _CORRADE_HELPER_PASTE(a, b) _CORRADE_HELPER_PASTE2(a, b)
#define _CORRADE_HELPER_STR(x) #x
#define _CORRADE_HELPER_DEFER(m, ...) m(__VA_ARGS__)
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
(C++17 feature, MSVC and Clang, GCC 10+):

@snippet Utility.cpp CORRADE_DEPRECATED_NAMESPACE

Note that this doesn't work on namespace aliases (i.e., marking
@cpp namespace Bar = Foo; @ce with this macro will result in a compile error.

GCC claims support since version 4.9, but even in version 8.2 it only emits an
"attribute ignored" warning at the declaration location and no diagnostic when
such namespace is used --- which is practically useless
([source](https://stackoverflow.com/q/46052410)).

@see @ref CORRADE_DEPRECATED(), @ref CORRADE_DEPRECATED_ALIAS(),
    @ref CORRADE_DEPRECATED_ENUM(), @ref CORRADE_DEPRECATED_FILE(),
    @ref CORRADE_DEPRECATED_MACRO()
*/
#if defined(__clang__)
/* Clang < 6.0 warns that this is a C++14 extension, Clang 6.0+ warns that
   namespace attributes are a C++17 extension and deprecated attribute is a
   C++14 extension. Clang < 6.0 doesn't know -Wc++17-extensions, so can't
   disable both in the same macro. Also, Apple has its own versioning of Clang
   (even in __clang_major__, args) and at the moment (April 2018) latest Xcode
   9.3 maps to Clang 5.1, so I assume Xcode 10 will be Clang 6. Yay? */
#if (!defined(CORRADE_TARGET_APPLE) && __clang_major__ < 6) || (defined(CORRADE_TARGET_APPLE) && __clang_major__ < 10)
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#else
#define CORRADE_DEPRECATED_NAMESPACE(message) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wc++14-extensions\"") _Pragma("GCC diagnostic ignored \"-Wc++17-extensions\"") [[deprecated(message)]] _Pragma("GCC diagnostic pop")
#endif
#elif defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 10)
#define CORRADE_DEPRECATED_NAMESPACE(message) [[deprecated(message)]]
#else
#define CORRADE_DEPRECATED_NAMESPACE(message)
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
included or compiled (GCC 4.8, Clang, MSVC):

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
#if defined(__clang__)
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this file is deprecated: " message)))
#elif defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ >= 408
#define CORRADE_DEPRECATED_FILE(message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED_FILE(_message) __pragma(message ("warning: " __FILE__ " is deprecated: " _message))
#else
#define CORRADE_DEPRECATED_FILE(message)
#endif

/** @hideinitializer
@brief Macro deprecation mark

Putting this in a macro definition will emit deprecation warning when given
macro is used (GCC 4.8, Clang, MSVC):

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
#if defined(__clang__)
#define CORRADE_DEPRECATED_MACRO(macro,message) _Pragma(_CORRADE_HELPER_STR(GCC warning ("this macro is deprecated: " message)))
#elif defined(__GNUC__) && __GNUC__*100 + __GNUC_MINOR__ >= 408
#define CORRADE_DEPRECATED_MACRO(macro,message) _Pragma(_CORRADE_HELPER_STR(GCC warning message))
#elif defined(_MSC_VER)
#define CORRADE_DEPRECATED_MACRO(macro,_message) __pragma(message (__FILE__ ": warning: " _CORRADE_HELPER_STR(macro) " is deprecated: " _message))
#else
#define CORRADE_DEPRECATED_MACRO(macro,message)
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
#ifdef __clang__
#define CORRADE_IGNORE_DEPRECATED_PUSH _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") _Pragma("GCC diagnostic ignored \"-W#pragma-messages\"")
#elif defined(__GNUC__)
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
/* clang-cl doesn't understand MSVC warning numbers right now, so the
   MSVC-specific variant below doesn't work */
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG_CL)
#define CORRADE_UNUSED __attribute__((__unused__))
#elif defined(CORRADE_TARGET_MSVC)
#define CORRADE_UNUSED __pragma(warning(suppress:4100))
#else
#define CORRADE_UNUSED
#endif

/** @hideinitializer
@brief Switch case fall-through

Suppresses a warning about a @cpp case @ce fallthrough in a @cpp switch @ce on
GCC >= 7 and Clang. GCC versions before 7 don't warn about the fallthrough, so
there's no need to suppress anything; for the same reason the macro does
nothing on MSVC. Expected to be put at a place where a @cpp break; @ce would
usually be:

@snippet Utility.cpp CORRADE_FALLTHROUGH
*/
#if defined(CORRADE_TARGET_GCC) && __GNUC__ >= 7
#define CORRADE_FALLTHROUGH __attribute__((fallthrough));
#elif defined(CORRADE_TARGET_CLANG)
#define CORRADE_FALLTHROUGH [[clang::fallthrough]];
#else
#define CORRADE_FALLTHROUGH
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
@brief Thread-local annotation
@m_since{2019,10}

Expands to C++11 @cpp thread_local @ce keyword on all compilers except old
Apple Clang, where it's defined as @cpp __thread @ce. Note that the
pre-standard @cpp __thread @ce has some semantic differences, in particular
regarding RAII.
*/
#ifdef __has_feature
#if !__has_feature(cxx_thread_local) /* Apple Clang 7.3 says false here */
#define CORRADE_THREAD_LOCAL __thread
#endif
#endif
#ifndef CORRADE_THREAD_LOCAL /* Assume it's supported otherwise */
#define CORRADE_THREAD_LOCAL thread_local
#endif

/** @hideinitializer
@brief C++14 constexpr
@m_since{2020,06}

Expands to @cpp constexpr @ce on C++14 and newer, empty on C++11. Useful for
selectively marking functions that make use of C++14 relaxed constexpr rules.
@see @ref CORRADE_CXX_STANDARD
*/
#if CORRADE_CXX_STANDARD >= 201402
#define CORRADE_CONSTEXPR14 constexpr
#else
#define CORRADE_CONSTEXPR14
#endif

/** @hideinitializer
@brief Always inline a function
@m_since{2019,10}

Stronger than the standard @cpp inline @ce keyword where supported, but even
then the compiler might decide to not inline the function (for example if it's
recursive). Expands to @cpp __attribute__((always_inline)) inline @ce on GCC
and Clang (both keywords need to be specified,
[docs](https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Function-Attributes.html)),
to @cpp __forceinline @ce on MSVC ([docs](https://docs.microsoft.com/en-us/cpp/cpp/inline-functions-cpp))
and to just @cpp inline @ce elsewhere. On GCC and Clang this makes the function
inline also in Debug mode (`-g`), while on MSVC compiling in Debug (`/Ob0`)
always suppresses all inlining. Example usage:

@snippet Utility.cpp CORRADE_ALWAYS_INLINE

@see @ref CORRADE_NEVER_INLINE
*/
#ifdef __GNUC__
#define CORRADE_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define CORRADE_ALWAYS_INLINE __forceinline
#else
#define CORRADE_ALWAYS_INLINE inline
#endif

/** @hideinitializer
@brief Never inline a function
@m_since{2019,10}

Prevents the compiler from inlining a function during an optimization pass.
Expands to @cpp __attribute__((noinline)) @ce on GCC and Clang
([docs](https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Function-Attributes.html)),
to @cpp __declspec(noinline) @ce on MSVC
([docs](https://docs.microsoft.com/en-us/cpp/cpp/noinline)) and is empty
elsewhere. Example usage:

@snippet Utility.cpp CORRADE_NEVER_INLINE

@see @ref CORRADE_ALWAYS_INLINE
*/
#ifdef __GNUC__
#define CORRADE_NEVER_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define CORRADE_NEVER_INLINE __declspec(noinline)
#else
#define CORRADE_NEVER_INLINE
#endif

/** @hideinitializer
@brief Function name
@m_since{2019,10}

Gives out an undecorated function name. Equivalent to the standard C++11
@cpp __func__ @ce on all sane platforms and to @cpp __FUNCTION__ @ce on
@ref CORRADE_TARGET_ANDROID "Android", because it just *has to be different*.

Note that the function name is *not* a string literal, meaning it can't be
concatenated with other string literals like @cpp __FILE__ @ce or
@ref CORRADE_LINE_STRING. Details [in this Stack Overflow answer](https://stackoverflow.com/a/18301370).
*/
#ifndef CORRADE_TARGET_ANDROID
#define CORRADE_FUNCTION __func__
#else
/* C++11 standard __func__ on Android behaves like GCC's __PRETTY_FUNCTION__,
   while GCC's __FUNCTION__ does the right thing.. I wonder -- do they have
   *any* tests for libc at all?! */
#define CORRADE_FUNCTION __FUNCTION__
#endif

/** @hideinitializer
@brief Line number as a string
@m_since{2019,10}

Turns the standard @cpp __LINE__ @ce macro into a string. Useful for example
to have correct line numbers when embedding GLSL shaders directly in the code:

@snippet Utility.cpp CORRADE_LINE_STRING

Depending on where the source string is located in the file, the potential
error or warning messages from the shader compiler will show the errors for
example like

@code{.shell-session}
1:97(4): error: no function with name 'THIS_IS_AN_ERROR'
@endcode

@m_class{m-noindent}

instead of showing the line number relatively as @cb{.shell-session} 1:5(4) @ce.
Note that GLSL in particular, unlike C, interprets the @cpp #line @ce statement
as applying to the immediately following line, which is why the extra
@cpp "\n" @ce is needed.

@see @ref CORRADE_FUNCTION
*/
#define CORRADE_LINE_STRING _CORRADE_HELPER_DEFER(_CORRADE_HELPER_STR, __LINE__)

/** @hideinitializer
@brief No-op
@m_since{2019,10}

Eats all arguments passed to it. Useful on compilers that don't support
defining function macros on command line --- for example,
@cpp -DA_MACRO=CORRADE_NOOP @ce is the same as doing @cpp -D'A_MACRO(arg)=' @ce.
*/
#define CORRADE_NOOP(...)

/** @hideinitializer
@brief Automatic initializer
@param function Initializer function name of type @cpp int(*)() @ce.

Function passed as argument will be called even before entering @cpp main() @ce
function. This is usable when e.g. automatically registering plugins or data
resources without forcing the user to write additional code in @cpp main() @ce.

@attention This macro does nothing in static libraries --- the global data
    defined by it (which cause the initialization) are thrown away by the
    linker as unused.

It's possible to override this macro for testing purposes or when global
constructors are not desired. For a portable way to defining the macro out on
compiler command line, see @ref CORRADE_NOOP().
*/
#ifndef CORRADE_AUTOMATIC_INITIALIZER
#define CORRADE_AUTOMATIC_INITIALIZER(function)                             \
    namespace {                                                             \
        struct Initializer_##function { static const int i; };              \
        const int Initializer_##function::i = function();                   \
    }
#endif

/** @hideinitializer
@brief Automatic finalizer
@param function Finalizer function name of type @cpp int(*)() @ce.

Function passed as argument will be called after exiting the @cpp main() @ce
function. This is usable in conjuction with @ref CORRADE_AUTOMATIC_INITIALIZER()
when there is need to properly discard initialized data.

@attention This macro does nothing in static libraries --- the global data
    defined by it (which cause the finalization) are thrown away by the
    linker as unused.

It's possible to override this macro for testing purposes or when global
destructors are not desired. For a portable way to defining the macro out on
compiler command line, see @ref CORRADE_NOOP().
*/
#ifndef CORRADE_AUTOMATIC_FINALIZER
#define CORRADE_AUTOMATIC_FINALIZER(function)                               \
    class Finalizer_##function {                                            \
        public:                                                             \
            Finalizer_##function() {}                                       \
            ~Finalizer_##function() { function(); }                         \
    } Finalizer_##function;
#endif

#endif
