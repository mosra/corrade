/*
    Corrade::Containers::Triple
        — a lightweight alternative to a three-component std::tuple

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Triple.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    Structured bindings on C++17 are opt-in due to reliance on a potentially
    heavy STL header --- `#define CORRADE_STRUCTURED_BINDINGS` before including
    the file. The STL compatibility bits are included as well --- opt-in with
    `#define CORRADE_TRIPLE_STL_COMPATIBILITY` before including the file.
    Including it multiple times with different macros defined works too.

    v2020.06-1846-gc4cdf (2025-01-07)
    -   Non-const C++17 structured bindings are now constexpr as well
    -   Structured bindings of const types now work even w/o <utility>
    v2020.06-1687-g6b5f (2024-06-29)
    -   Added explicit conversion constructors
    -   Structured bindings on C++17
    v2020.06-1502-g147e (2023-09-11)
    -   Fixes to the Utility::swap() helper to avoid ambiguity with std::swap()
    v2020.06-1454-gfc3b7 (2023-08-27)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* We don't need much from configure.h here. The CORRADE_MSVC2015_COMPATIBILITY
   check is equivalent to the version check in UseCorrade.cmake. */
#pragma ACME enable Corrade_configure_h
#if defined(_MSC_VER) && _MSC_VER < 1910
#define CORRADE_MSVC2015_COMPATIBILITY
#endif
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif
#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif
/* We need just CORRADE_CONSTEXPR14 from Macros.h, but that one relies on
   CORRADE_CXX_STANDARD which thus has to be defined early enough */
#ifdef _MSC_VER
#ifdef _MSVC_LANG
#define CORRADE_CXX_STANDARD _MSVC_LANG
#else
#define CORRADE_CXX_STANDARD 201103L
#endif
#else
#define CORRADE_CXX_STANDARD __cplusplus
#endif
#pragma ACME disable CORRADE_CONSTEXPR14

#include "Corrade/Containers/Triple.h"
#ifdef CORRADE_TRIPLE_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/TripleStl.h"
#endif
#ifdef CORRADE_STRUCTURED_BINDINGS
// {{includes}}
/* CORRADE_TARGET_LIBSTDCXX, CORRADE_TARGET_LIBCXX and
   CORRADE_TARGET_DINKUMWARE is needed for the StlForwardTupleSize.h header
   needed by StructuredBindings */
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
/* Containers.h is pulled in by StructuredBindings.h. We don't need anything
   from there. */
#pragma ACME enable Corrade_Containers_Containers_h
#pragma ACME disable Corrade_Containers_StructuredBindings_h
#pragma ACME enable Corrade_Containers_StructuredBindings_Pair_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StaticArray_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StaticArrayView_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StridedDimensions_h
#include "Corrade/Containers/StructuredBindings.h"
#endif
