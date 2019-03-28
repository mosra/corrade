/*
    Corrade's forward declaration for std::string
        — a lightweight alternative to the full <string> where supported

    https://doc.magnum.graphics/corrade/StlForwardString_8h.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.01-115-ged348b26 (2019-03-27)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* We need just the STL implementation detection from configure.h, copying it
   verbatim here. Keep in sync. */
#pragma ACME enable Corrade_configure_h
#include <ciso646>
#ifdef _LIBCPP_VERSION
#define CORRADE_TARGET_LIBCXX
#elif defined(__GLIBCXX__)
#define CORRADE_TARGET_LIBSTDCXX
/* GCC's <ciso646> provides the __GLIBCXX__ macro only since 6.1, so on older
   versions we'll try to get it from bits/c++config.h. GCC < 5.0 doesn't have
   __has_include, so on these versions we'll give up completely. */
#elif defined(__has_include)
    #if __has_include(<bits/c++config.h>)
        #include <bits/c++config.h>
        #ifdef __GLIBCXX__
        #define CORRADE_TARGET_LIBSTDCXX
        #endif
    #endif
#else
/* Otherwise it's MSVC STL (which has no std::string fwdecl) or no idea. */
#endif

#pragma ACME stats preprocessed-libcxx clang++ -stdlib=libc++ -std=c++11 -P -E -x c++ - | wc -l

#include "Corrade/Utility/StlForwardString.h"
