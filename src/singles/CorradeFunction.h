/*
    Corrade::Containers::Function
        — a lightweight alternative to std::function

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Function_3_01R_07Args_8_8_8_08_4.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2020.06-1631-g9001f (2024-05-17)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#include <cstddef>
#if !defined(CORRADE_CONSTEXPR_ASSERT) && !defined(NDEBUG)
#include <cassert>
#endif

/* Copied from configure.h */
#pragma ACME enable Corrade_configure_h
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif
#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif
#ifdef __MINGW32__
#define CORRADE_TARGET_MINGW
#endif
#if !defined(__x86_64) && !defined(_M_X64) && !defined(__aarch64__) && !defined(_M_ARM64) && !defined(__powerpc64__) && !defined(__wasm64__)
#define CORRADE_TARGET_32BIT
#endif

/* CORRADE_NO_STD_IS_TRIVIALLY_TRAITS (and thus CORRADE_TARGET_LIBSTDCXX)
   needed, and because it's so complex to check for it I can as well pull in
   the whole thing */
#pragma ACME enable Corrade_configure_h
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#define CORRADE_CXX_STANDARD _MSVC_LANG
#else
#define CORRADE_CXX_STANDARD 201103L
#endif
#else
#define CORRADE_CXX_STANDARD __cplusplus
#endif
#if CORRADE_CXX_STANDARD >= 202002
#include <version>
#else
#include <ciso646>
#endif
#ifdef _LIBCPP_VERSION
#define CORRADE_TARGET_LIBCXX
#elif defined(_CPPLIB_VER)
#define CORRADE_TARGET_DINKUMWARE
#elif defined(__GLIBCXX__)
#define CORRADE_TARGET_LIBSTDCXX
/* GCC's <ciso646> provides the __GLIBCXX__ macro only since 6.1, so on older
   versions we'll try to get it from bits/c++config.h */
#elif defined(__has_include)
    #if __has_include(<bits/c++config.h>)
        #include <bits/c++config.h>
        #ifdef __GLIBCXX__
        #define CORRADE_TARGET_LIBSTDCXX
        #endif
    #endif
/* GCC < 5.0 doesn't have __has_include, so on these versions we'll just assume
   it's libstdc++ as I don't think those versions are used with anything else
   nowadays anyway. Clang reports itself as GCC 4.4, so exclude that one. */
#elif defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 5
#define CORRADE_TARGET_LIBSTDCXX
#else
/* Otherwise no idea. */
#endif

#if defined(CORRADE_TARGET_LIBSTDCXX) && __GNUC__ < 5 && _GLIBCXX_RELEASE < 7
#define CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
#endif

/* The CORRADE_MSVC2015_COMPATIBILITY and CORRADE_MSVC2017_COMPATIBILITY checks
   are equivalent to the version check in UseCorrade.cmake. */
#if defined(_MSC_VER) && _MSC_VER < 1910
#define CORRADE_MSVC2015_COMPATIBILITY
#endif
#if defined(_MSC_VER) && _MSC_VER < 1920
#define CORRADE_MSVC2017_COMPATIBILITY
#endif

/* https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros */
#ifdef _WIN32
#define CORRADE_TARGET_WINDOWS
#endif

/* Disable asserts that are not used. CORRADE_CONSTEXPR_DEBUG_ASSERT is used,
   wrapping the #include <cassert> above. When enabling additional asserts, be
   sure to update it above as well -- without the _DEBUG variants, as they just
   delegate to the non-debug version of the macro. */
#include "assert.h"
#pragma ACME forget CORRADE_CONSTEXPR_ASSERT
#pragma ACME forget CORRADE_CONSTEXPR_DEBUG_ASSERT

#include "Corrade/Containers/Function.h"
