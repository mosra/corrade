/*
    Growable APIs for Corrade::Containers::Array

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Array.html#Containers-Array-growable

    Depends on CorradeArray.h.

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    The library makes use of AddressSanitizer annotations if compiling with
    ASan enabled, you can `#define CORRADE_CONTAINERS_NO_SANITIZER_ANNOTATIONS`
    to disable them.

    v2020.06-1507-gfbd9 (2023-09-13)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* CorradeArray.h is a dependency, remove all of Containers/Array.h and hide
   everything it already includes */
#pragma ACME noexpand CorradeArray.h
#pragma ACME enable Corrade_Containers_Array_h
#pragma ACME enable Corrade_Tags_h
#pragma ACME enable Corrade_Containers_constructHelpers_h
#include "CorradeArray.h"

/* CORRADE_NO_STD_IS_TRIVIALLY_TRAITS (and thus CORRADE_TARGET_LIBSTDCXX)
   needed, and because it's so complex to check for it I can as well pull in
   the whole thing. CORRADE_TARGET_DINKUMWARE is needed for an ASan forward
   declaration workaround, additionally we need CORRADE_TARGET_GCC and
   CORRADE_TARGET_CLANG which are already defined in CorradeArray.h. */
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

/* CORRADE_ASSERT and CORRADE_DEBUG_ASSERT are already in CorradeArray.h */

/* From Containers.h we need just the array forward declarations, the array
   view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_Containers_Containers_h
#include "Corrade/Containers/GrowableArray.h"
