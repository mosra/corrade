/*
    Corrade::Containers::ArrayView
    Corrade::Containers::StaticArrayView
        — lightweight alternatives to std::span / gsl::span

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1ArrayView.html
    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1StaticArrayView.html

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.01-107-g80d9f347 (2019-03-23)
    -   Including <cassert> only when needed
    v2019.01-41-g39c08d7c (2019-02-18)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
// {{includes}}
#include <cstddef>
#if (!defined(CORRADE_ASSERT) || !defined(CORRADE_CONSTEXPR_ASSERT)) && !defined(NDEBUG)
#include <cassert>
#endif

/* We need just CORRADE_MSVC2017_COMPATIBILITY from configure.h. This is
   equivalent to the version check in UseCorrade.cmake. */
#pragma ACME enable Corrade_configure_h
#if defined(_MSC_VER) && _MSC_VER <= 1920
#define CORRADE_MSVC2017_COMPATIBILITY
#endif

/* We need just the array forward declarations from Containers.h */
#pragma ACME enable Corrade_Containers_Containers_h

/* Disable asserts that are not used. CORRADE_ASSERT and
   CORRADE_CONSTEXPR_ASSERT is used, wrapping the #include <cassert> above.
   When enabling additional asserts, be sure to update it above as well. */
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE

#ifndef CorradeArrayView_h
#define CorradeArrayView_h

namespace Corrade { namespace Containers {

template<class> class ArrayView;
template<std::size_t, class> class StaticArrayView;

}}

#endif
#include "Corrade/Containers/ArrayView.h"
#ifdef CORRADE_ARRAYVIEW_STL_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/ArrayViewStl.h"
#endif
#ifdef CORRADE_ARRAYVIEW_STL_SPAN_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/ArrayViewStlSpan.h"
#endif
