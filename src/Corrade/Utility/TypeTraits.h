#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h
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
 * @brief Macros @ref CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE, @ref CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED, @ref CORRADE_HAS_TYPE(), alias @ref Corrade::Utility::IsIterable
 */

#include <type_traits>

#include "Corrade/configure.h"

namespace Corrade { namespace Utility {

#ifdef DOXYGEN_GENERATING_OUTPUT
/** @hideinitializer
@brief Whether `long double` has the same precision as `double`
@m_since{2020,06}

Defined on platforms where the @cpp long double @ce type has a 64-bit precision
instead of 80-bit, thus same as @cpp double @ce. It's the case for
@ref CORRADE_TARGET_MSVC "MSVC" ([source](https://docs.microsoft.com/en-us/previous-versions/9cx8xs15(v=vs.140))),
32-bit @ref CORRADE_TARGET_ANDROID "Android" (no reliable source found,
sorry), @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"
and @ref CORRADE_TARGET_APPLE "Mac" (but not @ref CORRADE_TARGET_IOS "iOS")
with @ref CORRADE_TARGET_ARM "ARM" processors. Emscripten is a bit special
because it's @cpp long double @ce is *sometimes* 80-bit, but its precision
differs from the 80-bit representation elsewhere, so it's always treated as
64-bit. Note that even though the type size and precision may be the same,
these are still two distinct types, similarly to how @cpp int @ce and
@cpp signed int @ce behave the same but are treated as different types.
*/
/* Actual definitions is in configure.h so Magnum doesn't need to pull in this
   whole thing in its TypeTraits just for this macro */
#define CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
#endif

namespace Implementation {
    /* Kept private because I don't think this needs to be exposed (for math
       stuff there's Magnum and its TypeTraits that expose this). In each case,
       printing precision has one digit are more than the epsilon. Used by
       Debug, format(), TestSuite and kept in sync with Magnum's TypeTraits. */
    template<class> struct FloatPrecision;
    /* The default. Source: http://en.cppreference.com/w/cpp/io/ios_base/precision,
       Wikipedia says 6-digit number can be converted back and forth without
       loss: https://en.wikipedia.org/wiki/Single-precision_floating-point_format */
    template<> struct FloatPrecision<float> {
        enum: int { Digits = 6 };
        constexpr static float epsilon() { return 1.0e-5f; }
    };
    /* Wikipedia says 15-digit number can be converted back and forth without
       loss: https://en.wikipedia.org/wiki/Double-precision_floating-point_format */
    template<> struct FloatPrecision<double> {
        enum: int { Digits = 15 };
        constexpr static double epsilon() { return 1.0e-14; }
    };
    /* Wikipedia says 18-digit number can be converted back and forth without
       loss: https://en.wikipedia.org/wiki/Extended_precision#Working_range */
    template<> struct FloatPrecision<long double> {
        #ifndef CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE
        enum: int { Digits = 18 };
        constexpr static long double epsilon() { return 1.0e-17l; }
        #else
        enum: int { Digits = 15 };
        constexpr static long double epsilon() { return 1.0e-14l; }
        #endif
    };
}

/** @hideinitializer
@brief Whether the @ref std::is_trivially_copyable family of type traits is supported by the standard library
@m_since{2020,06}

The @ref std::is_trivially_constructible,
@ref std::is_trivially_default_constructible,
@ref std::is_trivially_copy_constructible,
@ref std::is_trivially_move_constructible,
@ref std::is_trivially_assignable,
@ref std::is_trivially_copy_assignable,
@ref std::is_trivially_move_assignable family of traits is not implemented
[on libstdc++ below version 5](https://gcc.gnu.org/gcc-5/changes.html).
Instead, legacy @cpp std::has_trivial_default_constructor @ce,
@cpp std::has_trivial_copy_constructor @ce and
@cpp std::has_trivial_copy_assign @ce are available, with slightly different
semantics (see e.g. https://stackoverflow.com/q/12754886 for more information).
From libstdc++ 5 onwards these are marked as deprecated and
[libstdc++ 7 removes them](https://gcc.gnu.org/gcc-7/changes.html), so
alternatively there are @cpp __has_trivial_constructor() @ce,
@cpp __has_trivial_copy() @ce and @cpp __has_trivial_assign() @ce builtins that
don't produce any deprecated warnings and are available until GCC 9 / Clang 10
at least --- however note that for SFINAE you need to wrap them in
@ref std::integral_constant as otherwise GCC would throw errors similar to the
following:

@m_class{m-console-wrap}

@code{.shell-session}
error: use of built-in trait ‘__has_trivial_copy(T)’ in function signature; use library traits instead
@endcode

This macro is defined if the standard variants are available. Unfortunately,
when libstdc++ is used through Clang, there's no way to check for its version
until libstdc++ 7, which added the `_GLIBCXX_RELEASE` macro. That means, when
using Clang with libstdc++ 5 or 6, it will still report those traits as being
unavailable. Both libc++ and MSVC STL have these traits in all versions
supported by Corrade, so there the macro is defined always.
@see @ref CORRADE_TARGET_LIBSTDCXX, @ref CORRADE_TARGET_LIBCXX,
    @ref CORRADE_TARGET_DINKUMWARE
*/
#if defined(DOXYGEN_GENERATING_OUTPUT) || !defined(CORRADE_TARGET_LIBSTDCXX) || __GNUC__ >= 5 || _GLIBCXX_RELEASE >= 7
#define CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
#endif

/** @hideinitializer
@brief Macro for creating traits class that checks for type expression validity
@param className        Resulting class name
@param ...              Type expression to check. Variadic parameter to allow
    unrestricted usage of template expressions containing commas.

Defines a traits class checking whether an expression is valid. You can use
@cpp T @ce to reference the type which is being checked.

Usage examples: checking for presence of a @cpp key_type @ce member
@cpp typedef @ce:

@snippet Utility.cpp CORRADE_HAS_TYPE-type

Checking for presence of @cpp size() @ce member function:

@snippet Utility.cpp CORRADE_HAS_TYPE-function

@todo rename to CORRADE_IS_INVOCABLE (or even better name?) and change
    __VA_ARGS__ to decltype(__VA_ARGS__) because that's what is used in most
    cases? Or not, like in the nasty Eigen workaround below?
*/
/* Two overloaded get() functions return type of different size. Templated
   get() is used when T has given attribute, non-templated otherwise. Bool
   value then indicates whether the templated version was called or not. */
#define CORRADE_HAS_TYPE(className, ...)                                    \
template<class U> class className {                                         \
    template<class T> static char get(T&&, __VA_ARGS__* = nullptr);         \
    static short get(...);                                                  \
    public:                                                                 \
        enum: bool { value = sizeof(get(std::declval<U>())) == sizeof(char) }; \
}

/** @hideinitializer
@brief C++20 is_constant_evaluated
@m_since{2022,10}

Expands to a predicate determining whether given @cpp constexpr @ce function
is being evaluated at compile-time or not. Under C++14 rules, constexpr
functions may be defined as long as at least some of the code paths are able
to be executed at compile-time. As long as a fallback is present, features
such as SIMD or inline assembly may be used in functions marked constexpr.

This support is available on all C++20 capable compilers, but also certain
ones (Clang 9, GCC 9, MSVC 2022 17.1) that expose the feature as a non-portable
extension. In which case it may be used under C++14 relaxed constexpr rules.
*/
#if (defined(CORRADE_TARGET_CLANG) && !defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__ >= 9) || (defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__*100 + __clang_minor__ >= 1104) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 9) || (defined(CORRADE_TARGET_MSVC) && _MSC_VER >= 1931)
#define CORRADE_IS_CONSTANT_EVALUATED (__builtin_is_constant_evaluated())
#elif CORRADE_CXX_STANDARD >= 202002
#define CORRADE_IS_CONSTANT_EVALUATED (std::is_constant_evaluated())
#endif

namespace Implementation {
    /* As of Eigen 3.4.0, due to these two commits in particular,
        https://gitlab.com/libeigen/eigen/-/commit/c0ca8a9fa3e03ad7ecb270adfe760a1bff7c0829
        https://gitlab.com/libeigen/eigen/-/commit/2bf1a31d811fef2085bad97f98e2d0095136b636
       the begin() / end() on Eigen Array types returns void if it's not a
       vector, which breaks the IsIterable detection code that (for simplicity)
       relied just on presence of these functions, and assumes sanity. SANITY,
       WITH EIGEN, HAHA, never. There's probably a more robust way to detect
       iterability than to adding a workaround for an Eigen-specific wart --
       for example by trying to increment the iterator like they have in that
       commit, but trying just
        CORRADE_HAS_TYPE(HasMemberBegin, decltype(++std::declval<T>().begin()))
       alone didn't work and adding some other HasBasicSanity SFINAE trait
       would only add more to the compiler suffering that wouldn't be needed
       for SANE code.

       See isIterableNotBecauseEigenDevsAttemptedToDisableBeginByMakingItVoid()
       in TypeTraitsTest for a test case. */
    template<class T> struct FineUnlessEigenDevsAttemptedToDisableBeginByMakingItVoid {
        typedef T Type;
    };
    template<> struct FineUnlessEigenDevsAttemptedToDisableBeginByMakingItVoid<void> {};

    CORRADE_HAS_TYPE(HasMemberBegin, typename FineUnlessEigenDevsAttemptedToDisableBeginByMakingItVoid<decltype(std::declval<T>().begin())>::Type);
    CORRADE_HAS_TYPE(HasMemberEnd, decltype(std::declval<T>().end()));
    CORRADE_HAS_TYPE(HasBegin, decltype(begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasEnd, decltype(end(std::declval<T>())));
    /* std::string has c_str() and substr(), std::string_view has substr() and
       std::filesystem::path has c_str(), so we need both to cover all */
    CORRADE_HAS_TYPE(HasMemberCStr, decltype(std::declval<T>().c_str()));
    CORRADE_HAS_TYPE(HasMemberSubstr, decltype(std::declval<T>().substr()));
}

/**
@brief Traits class for checking whether given type is iterable

Equivalent to @ref std::true_type if the class is has either @cpp begin() @ce /
@cpp end() @ce members, is usable with free @cpp begin() @ce / @cpp end() @ce
functions or has @ref std::begin() / @ref std::end() overloads. Otherwise
equivalent to @ref std::false_type.

Used together with @ref IsStringLike by @ref Debug to decide whether given type
should be printed as a container of its contents or as a whole.
@todoc use the ellipsis macro once m.css has it
*/
/* When using {}, MSVC 2015 complains that even the explicitly defaulted
   constructor doesn't exist */
template<class T> using IsIterable = std::integral_constant<bool,
    #ifndef DOXYGEN_GENERATING_OUTPUT
    (Implementation::HasMemberBegin<T>::value || Implementation::HasBegin<T>::value) &&
    (Implementation::HasMemberEnd<T>::value || Implementation::HasEnd<T>::value)
    #else
    implementation-specific
    #endif
    >;

/**
@brief Traits class for checking whether given type is string-like
@m_since{2019,10}

Equivalent to @ref std::true_type if the class is has a @cpp c_str() @ce or a
@cpp substr() @ce member or is a
@ref Containers::BasicStringView "Containers::[Mutable]StringView" /
@ref Containers::String. Otherwise equivalent to @ref std::false_type. Useful
for dispatching on the @ref std::string or the C++17 @ref std::string_view
and @ref std::filesystem::path types without having to include or
@ref StlForwardString.h "forward-declare" them.

Used together with @ref IsIterable by @ref Debug to decide whether given type
should be printed as a container of its contents or as a whole.
@todoc use the ellipsis macro once m.css has it
*/
template<class T> using IsStringLike = std::integral_constant<bool,
    #ifndef DOXYGEN_GENERATING_OUTPUT
    Implementation::HasMemberCStr<T>::value || Implementation::HasMemberSubstr<T>::value || std::is_same<typename std::decay<T>::type, Containers::StringView>::value || std::is_same<typename std::decay<T>::type, Containers::MutableStringView>::value || std::is_same<typename std::decay<T>::type, Containers::String>::value
    #else
    implementation-specific
    #endif
    >;

}}

#endif
