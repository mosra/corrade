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

/* We need CORRADE_MSVC{,2015}_COMPATIBILITY from configure.h, which is handled
   by CorradeArrayView.h already, and CORRADE_TARGET_GCC +
   CORRADE_TARGET_CLANG */
#pragma ACME enable Corrade_configure_h
#ifdef __GNUC__
#define CORRADE_TARGET_GCC
#endif
#ifdef __clang__
#define CORRADE_TARGET_CLANG
#endif

/* From Containers.h we need just the array forward declarations, the array
   view ones are again already in CorradeArrayView.h. */
#pragma ACME enable Corrade_Containers_Containers_h

/* CorradeArrayView.h has CORRADE_ASSERT, CORRADE_CONSTEXPR_ASSERT and
   CORRADE_CONSTEXPR_DEBUG_ASSERT, we additionally need CORRADE_DEBUG_ASSERT
   here */
#pragma ACME enable CORRADE_ASSERT
#pragma ACME enable CORRADE_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_CONSTEXPR_DEBUG_ASSERT
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_DEBUG_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_EXPRESSION
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_EXPRESSION
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_DEBUG_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_DEBUG_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE
#include "Corrade/Utility/DebugAssert.h"
#ifndef CorradeArray_h
#define CorradeArray_h
namespace Corrade { namespace Containers {

template<class T, class = void(*)(T*, std::size_t)> class Array;
/* Needs to be defined here because it's referenced before its definition */
template<std::size_t, class> class StaticArray;
/* These need to be defined here because the other occurence is guarded with
   #ifndef CORRADE_MSVC2015_COMPATIBILITY */
template<class T> using Array2 = StaticArray<2, T>;
template<class T> using Array3 = StaticArray<3, T>;
template<class T> using Array4 = StaticArray<4, T>;

}}
#endif
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
