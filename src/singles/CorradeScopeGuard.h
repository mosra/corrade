/*
    Corrade::Containers::ScopeGuard
        — a lightweight alternative to std::unique_ptr with a custom deleter

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1ScopeGuard.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.01-41-g39c08d7c (2019-02-18)
    -   Ability to create a handle-less ScopeGuard
    v2018.10-232-ge927d7f3 (2019-01-28)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
/* No external includes, so avoid extraneous newline here */
/* We need just CORRADE_MSVC2015_COMPATIBILITY from configure.h. This is
   equivalent to the version check in UseCorrade.cmake. */
#pragma ACME enable Corrade_configure_h
#if defined(_MSC_VER) && _MSC_VER <= 1900
#define CORRADE_MSVC2015_COMPATIBILITY
#endif

#include "Corrade/Containers/ScopeGuard.h"
