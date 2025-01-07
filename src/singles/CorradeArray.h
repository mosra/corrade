/*
    Corrade::Containers::Array
    Corrade::Containers::StaticArray
        — lightweight alternatives to std::vector / std::array

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1Array.html
    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1StaticArray.html

    Depends on CorradeArrayView.h.

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    Structured bindings for StaticArray on C++17 are opt-in due to reliance on
    a potentially heavy STL header --- `#define CORRADE_STRUCTURED_BINDINGS`
    before including the file. Including it multiple times with different
    macros defined works too.

    v2020.06-1846-gc4cdf (2025-01-07)
    -   StaticArray default constructor and constructors from a C array are now
        implicit
    -   SFINAE is now done in template args as that's simpler for the compiler
    -   Structured bindings of const types now work even w/o <utility>
    v2020.06-1687-g6b5f (2024-06-29)
    -   Ability to InPlaceInit-construct an Array from an ArrayView, in
        addition to std::initializer_list
    -   StaticArray is now trivially copyable and constexpr if the underlying
        type is
    -   Structured bindings for StaticArray on C++17
    v2020.06-1506-g43e1c (2023-09-13)
    -   Preventing a conflict with the Array declaration in Corrade's
        Containers.h due to default template arguments being used in both
    v2020.06-1454-gfc3b7 (2023-08-27)
    -   New exceptPrefix() API, the except() API is renamed to exceptSuffix().
        The suffix() API, which took an offset, is removed and will be
        eventually reintroduced again but taking suffix size, consistently with
        prefix() that takes prefix size.
    -   New sliceSize() API, taking a begin + size instead of begin + end
    -   Element access with operator[](), front() and back() is now
        bounds-checked with assertions
    -   Convenience Array2, Array3, Array4 aliases for StaticArray
    -   The DefaultInit, ValueInit, NoInit, DirectInit and InPlaceInit tags
        were moved from Containers to the root namespace
    -   Renamed empty() to isEmpty() for consistency with other bool-returning
        APIs
    -   MSVC 2022 compatibility
    -   Further workarounds for various compiler-specific issues and standard
        defects when using {}-initialization for aggregate types
    -   Removed dependency on <utility>, resulting in about ~450 preprocessed
        lines less
    v2020.06-0-g61d1b58c (2020-06-27)
    -   Default initialization got changed to ValueInit, which means builtin
        types are zero-initialized instead of kept uninitialized
    -   Working around various compiler-specific issues and standard defects
        when using {}-initialization for aggregate types
    v2019.10-0-g162d6a7d (2019-10-24)
    -   StaticArray is now copy/movable if the underlying type is
    v2019.01-301-gefe8d740 (2019-08-05)
    -   MSVC 2019 compatibility
    -   Added except() for taking everything except last N elements
    -   Added StaticArray::slice() and suffix() with compile-time begin / end
    v2019.01-173-ge663b49c (2019-04-30)
    -   Different implementation for Array-to-view conversion
    v2019.01-107-g80d9f347 (2019-03-23)
    -   Added missing <initializer_list> include
    v2019.01-47-g524c127e (2019-02-18)
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

/* We need CORRADE_MSVC{,2015}_COMPATIBILITY and CORRADE_CXX_STANDARD from
   configure.h, which is handled by CorradeArrayView.h already; plus
   CORRADE_TARGET_GCC, CORRADE_TARGET_CLANG, CORRADE_TARGET_MSVC and
   CORRADE_MSVC2017_COMPATIBILITY */
#pragma ACME enable Corrade_configure_h
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif
#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif
#ifdef _MSC_VER
#define CORRADE_TARGET_MSVC
#endif
#if defined(_MSC_VER) && _MSC_VER < 1920
#define CORRADE_MSVC2017_COMPATIBILITY
#endif
/* For the trivially copyable variant of StaticArray, relies on
   CORRADE_TARGET_LIBSTDCXX detection that's in CorradeArrayView.h already */
#if defined(CORRADE_TARGET_LIBSTDCXX) && __GNUC__ < 5 && _GLIBCXX_RELEASE < 7
#define CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
#endif

/* We need just CORRADE_CONSTEXPR14 from Macros.h */
#pragma ACME disable CORRADE_CONSTEXPR14

/* CorradeArrayView.h has CORRADE_ASSERT, CORRADE_CONSTEXPR_ASSERT and
   CORRADE_CONSTEXPR_DEBUG_ASSERT, we additionally need CORRADE_DEBUG_ASSERT
   here */
#include "assert.h"
#pragma ACME forget CORRADE_DEBUG_ASSERT
#include "Corrade/Utility/DebugAssert.h"
#ifndef CorradeArray_h
#define CorradeArray_h
namespace Corrade { namespace Containers {

/* In case Corrade/Containers/Containers.h is included too, these two would
   conflict */
#ifndef Corrade_Containers_Containers_h
template<class T, class = void(*)(T*, std::size_t)> class Array;
#endif
/* Needs to be defined here because it's referenced before its definition */
template<std::size_t, class> class StaticArray;
/* These need to be defined here because the other occurence is guarded with
   #ifndef CORRADE_MSVC2015_COMPATIBILITY */
template<class T> using Array2 = StaticArray<2, T>;
template<class T> using Array3 = StaticArray<3, T>;
template<class T> using Array4 = StaticArray<4, T>;

}}
#endif
/* From Containers.h we need just the array forward declarations, the array
   view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_Containers_Containers_h
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
#ifdef CORRADE_STRUCTURED_BINDINGS
// {{includes}}
/* The StlForwardTupleSizeElement header and the STL detection logic it relies
   on is already pulled in by CorradeArrayView.h */
#pragma ACME enable Corrade_Utility_StlForwardTupleSizeElement_h
#pragma ACME disable Corrade_Containers_StructuredBindings_h
#pragma ACME enable Corrade_Containers_StructuredBindings_Pair_h
#pragma ACME enable Corrade_Containers_StructuredBindings_Triple_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StaticArrayView_h
#pragma ACME enable Corrade_Containers_StructuredBindings_StridedDimensions_h
#include "Corrade/Containers/StructuredBindings.h"
#endif
