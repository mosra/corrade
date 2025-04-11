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

    Structured bindings for StridedDimensions on C++17 are opt-in due to
    reliance on a potentially heavy STL header ---
    `#define CORRADE_STRUCTURED_BINDINGS` before including the file. Including
    it multiple times with different macros defined works too.

    v2020.06-1890-g77f9f (2025-04-11)
    -   Ability to construct StridedArrayView<T> from a StridedArrayView<T[]>
        of one dimension less
    -   Cleanup and unification of SFINAE code, it's now done in template args
        in all cases as that's simpler for the compiler
    -   Workaround for a MSVC compiler crash when slicing to a member function
        that returns a C array reference
    -   Added a missing StridedArrayView arraySize() overload
    v2020.06-1872-gbf086 (2025-03-03)
    -   Fixed slice() to work for non-overloaded member functions
    v2020.06-1846-gc4cdf (2025-01-07)
    -   Worked around an issue where certain new Clang versions would do an OOB
        access with negative strides on 32-bit builds (such as on Emscripten)
    -   Fixed StridedDimensions structured bindings constexpr signature
    -   Structured bindings of const types now work even w/o <utility>
    v2020.06-1687-g6b5f (2024-06-29)
    -   Structured bindings for StridedDimensions on C++17
    v2020.06-1454-gfc3b7 (2023-08-27)
    -   New expanded() and collapsed() APIs
    -   Ability to slice to struct members and member functions
    -   New exceptPrefix() API, the except() API is renamed to exceptSuffix().
        The suffix() API, which took an offset, is removed and will be
        eventually reintroduced again but taking suffix size, consistently with
        prefix() that takes prefix size.
    -   New sliceSize() API, taking a begin + size instead of begin + end
    -   New stridedArrayView() convenience helpers for creating 1D strided
        array views from ArrayView instances and a pointer + size
    -   The Size and Stride member typedefs are moved to the Containers
        namespace; Size1D, Size2D, Size3D, Size4D, Stride1D, Stride2D,
        Stride3D and Stride4D convenience typedefs were added
    -   Renamed empty() to isEmpty() for consistency with other bool-returning
        APIs
    -   MSVC 2022 compatibility
    -   Removed dependency on <utility>, resulting in about ~600 preprocessed
        lines less
    v2020.06-0-g61d1b58c (2020-06-27)
    -   Added mutable StridedDimensions::begin()/end()
    -   New cross-dimension arrayCast() overloads
    -   Added isContiguous() and asContiguous() overloads
    -   Similarly to ArrayView, there's now a StridedArrayView<void> and
        StridedArrayView<const void> specialization usable for type-erased
        storage in constexpr contexts
    v2019.10-0-g162d6a7d (2019-10-24)
    -   Don't assert when creating arrays with non-zero stride but zero size
    -   Added a StridedArrayView4D convenience typedef
    v2019.01-301-gefe8d740 (2019-08-05)
    -   MSVC 2019 compatibility
    -   New constructor taking just a size, with stride calculated implicitly
    -   Added except() for taking everything except last N elements
    -   Added every() for taking every Nth element
    v2019.01-173-ge663b49c (2019-04-30)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"

/* CorradeArrayView.h is a dependency, remove all of Containers/ArrayView.h and
   hide everything it already includes */
#pragma ACME noexpand CorradeArrayView.h
#pragma ACME enable Corrade_Containers_ArrayView_h
#pragma ACME enable Corrade_Utility_Move_h
#include "CorradeArrayView.h"

/* We need just CORRADE_MSVC{,2015}_COMPATIBILITY from configure.h, which is
   handled by CorradeArrayView.h already, and CORRADE_TARGET_GCC +
   CORRADE_TARGET_CLANG */
#pragma ACME enable Corrade_configure_h
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif
#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif

/* We need just CORRADE_CONSTEXPR14 from Macros.h */
#pragma ACME disable CORRADE_CONSTEXPR14

/* From Containers.h we need just the strided array view forward declarations,
   the array view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_Containers_Containers_h

/* CorradeArrayView.h has CORRADE_ASSERT, CORRADE_CONSTEXPR_ASSERT and
   CORRADE_CONSTEXPR_DEBUG_ASSERT, we additionally need CORRADE_DEBUG_ASSERT
   here */
#include "assert.h"
#pragma ACME forget CORRADE_DEBUG_ASSERT
#include "Corrade/Utility/DebugAssert.h"
#ifndef CorradeStridedArrayView_h
#define CorradeStridedArrayView_h
namespace Corrade { namespace Containers {

/* These need to be defined here because the other occurence is guarded with
   #ifndef CORRADE_MSVC2015_COMPATIBILITY. And thus also StridedDimensions. */
template<unsigned, class> class StridedDimensions;
template<unsigned dimensions> using Size = StridedDimensions<dimensions, std::size_t>;
template<unsigned dimensions> using Stride = StridedDimensions<dimensions, std::ptrdiff_t>;
/* Needs to be defined here because it's referenced before its definition */
template<unsigned, class> class StridedIterator;
/* These need to be defined here because the other occurence is guarded with
   #ifndef CORRADE_MSVC2015_COMPATIBILITY. And thus also StridedArrayView. */
template<unsigned, class> class StridedArrayView;
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;
template<class T> using StridedArrayView4D = StridedArrayView<4, T>;
/* Needs to be defined here because sliceBit() uses it */
template<unsigned, class> class BasicStridedBitArrayView;

}}
#endif
#include "Corrade/Containers/StridedArrayView.h"
#ifdef CORRADE_STRUCTURED_BINDINGS
// {{includes}}
/* The StlForwardTupleSizeElement header and the STL detection logic it relies
   on is already pulled in by CorradeArrayView.h */
#pragma ACME enable Corrade_Utility_StlForwardTupleSizeElement_h
#pragma ACME disable Corrade_Containers_StructuredBindings_h
#pragma ACME enable Corrade_Containers_StructuredBindings_Pair_h
#pragma ACME enable Corrade_Containers_StructuredBindings_Triple_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StaticArray_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StaticArrayView_h
#include "Corrade/Containers/StructuredBindings.h"
#endif
