/*
    Corrade's forward declaration for std::array
        — a lightweight alternative to the full <array> where supported

    https://doc.magnum.graphics/corrade/StlForwardArray_8h.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.01-115-ged348b26 (2019-03-27)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed-libcxx}} LoC
*/

#include "base.h"

/* We need just the STL implementation detection from configure.h, copying it
   verbatim here. Keep in sync. */
#pragma ACME enable Corrade_configure_h
#include <ciso646>
#ifdef _LIBCPP_VERSION
#define CORRADE_TARGET_LIBCXX
#elif defined(_CPPLIB_VER)
#define CORRADE_TARGET_DINKUMWARE
#else
/* Otherwise it's libstdc++ (which doesn't have std::array fwdecl) or no idea. */
#endif

#pragma ACME stats preprocessed-libcxx clang++ -stdlib=libc++ -std=c++11 -P -E -x c++ - | wc -l

#include "Corrade/Utility/StlForwardArray.h"
