/*
    Corrade::Containers::StridedArrayView
        — multidimensional strided array view

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1StridedArrayView.html

    Depends on CorradeArrayView.h.

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.01-173-ge663b49c (2019-04-30)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* CorradeArrayView.h is a dependency, remove all of Containers/ArrayView.h and
   hide everything it already includes */
#pragma ACME noexpand CorradeArrayView.h
#pragma ACME enable Corrade_Containers_ArrayView_h
#include "CorradeArrayView.h"

/* We need just CORRADE_MSVC2015_COMPATIBILITY from configure.h, this is
   equivalent to the version check in UseCorrade.cmake.
   CORRADE_MSVC2019_COMPATIBILITY is handled by CorradeArrayView.h already.
   From Containers.h we need just the array forward declarations, the array
   view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_configure_h
#pragma ACME enable Corrade_Containers_Containers_h
#if defined(_MSC_VER) && _MSC_VER <= 1900
#define CORRADE_MSVC2015_COMPATIBILITY
#endif

/* Disable all asserts, CorradeStridedArrayView.h has both CORRADE_ASSERT and
   CORRADE_CONSTEXPR_ASSERT */
#pragma ACME enable CORRADE_ASSERT
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE

#ifndef CorradeStridedArrayView_h
#define CorradeStridedArrayView_h

namespace Corrade { namespace Containers {

template<unsigned, class> class StridedDimensions;
template<unsigned, class> class StridedArrayView;
template<unsigned, class> class StridedIterator;
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;

}}

#endif
#include "Corrade/Containers/StridedArrayView.h"
