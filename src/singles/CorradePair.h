/*
    Corrade::Containers::Pair
        — a lightweight alternative to std::pair

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Pair.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    The STL compatibility bits are included as well --- opt-in by specifying
    `#define CORRADE_PAIR_STL_COMPATIBILITY` before including the file.
    Including it multiple times with different macros defined works too.

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

#include "Corrade/Containers/Pair.h"
#ifdef CORRADE_PAIR_STL_COMPATIBILITY
// {{includes}}
/* The <utility> header is conditionally included for MSVC 2015 from Move.h,
   which makes the script think it's not needed to be included again. */
#pragma ACME forget <utility>
#include "Corrade/Containers/PairStl.h"
#endif
