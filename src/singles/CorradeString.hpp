/*
    Corrade::Containers::String
    Corrade::Containers::StringView
        — lightweight and optimized string (view) classes

    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1BasicString.html
    https://doc.magnum.graphics/corrade/classCorrade_1_1Containers_1_1BasicStringView.html

    Depends on CorradeEnumSet.h, the implementation depends on CorradePair.h
    and CorradeCpu.hpp.

    This is a single-header library generated from the Corrade project. With
    the goal being easy integration, it's deliberately free of all comments
    to keep the file size small. More info, detailed changelogs and docs here:

    -   Project homepage — https://magnum.graphics/corrade/
    -   Documentation — https://doc.magnum.graphics/corrade/
    -   GitHub project page — https://github.com/mosra/corrade
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    The library has a separate non-inline implementation part, enable it *just
    once* like this:

        #define CORRADE_STRING_IMPLEMENTATION
        #include <CorradeString.hpp>

    If you need the deinlined symbols to be exported from a shared library,
    `#define CORRADE_UTILITY_EXPORT` and `CORRADE_UTILITY_LOCAL` as
    appropriate. Runtime CPU dispatch for the implementation is enabled by
    default, you can disable it with `#define CORRADE_NO_CPU_RUNTIME_DISPATCH`
    before including the file in both the headers and the implementation. To
    enable the IFUNC functionality for CPU runtime dispatch,
    `#define CORRADE_CPU_USE_IFUNC`.

    The STL compatibility bits are included as well --- opt-in by specifying
    either `#define CORRADE_STRING_STL_COMPATIBILITY` or
    `#define CORRADE_STRING_STL_VIEW_COMPATIBILITY` before including the file.
    Including it multiple times with different macros defined works too.

    v2020.06-1846-gc4cdf (2025-01-07)
    -   Fixed embarrassing bugs in the NEON and WASM SIMD code paths for find()
    -   SFINAE is now done in template args as that's simpler for the compiler
    -   std::string STL compatibility is now inline, meaning the
        CORRADE_STRING_STL_COMPATIBILITY macro doesn't need to be defined also
        for CORRADE_STRING_IMPLEMENTATION anymore
    v2020.06-1687-g6b5f (2024-06-29)
    -   New, SIMD-optimized count() API
    -   Literals are now available in an inline Literals::StringLiterals
        namespace for finer control over which literals get actually used
    -   String copy construction and copy assignment now makes the copy a SSO
        only if the original instance was a SSO as well
    v2020.06-1502-g147e (2023-09-11)
    -   Initial release

    Generated from Corrade {{revision}}, {{stats:loc}} / {{stats:preprocessed}} LoC
*/

#include "base.h"
#if !defined(CORRADE_CONSTEXPR_ASSERT) && !defined(NDEBUG)
#include <cassert>
#endif
// {{includes}}

/* From configure.h we need just CORRADE_TARGET_MSVC, CORRADE_TARGET_32BIT and
   CORRADE_TARGET_BIG_ENDIAN for the header. All other target detection is
   needed only for the source and is handled by CorradeCpu.hpp that pulls in
   the whole configure.h. */
#pragma ACME enable Corrade_configure_h
#ifdef _MSC_VER
#define CORRADE_TARGET_MSVC
#endif
#if !defined(__x86_64) && !defined(_M_X64) && !defined(__aarch64__) && !defined(_M_ARM64) && !defined(__powerpc64__) && !defined(__wasm64__)
#define CORRADE_TARGET_32BIT
#endif
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CORRADE_TARGET_BIG_ENDIAN
#elif __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error what kind of endianness is this?
#endif
#elif defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define CORRADE_TARGET_BIG_ENDIAN
#endif

/* split(), join() and partition() APIs require Array, GrowableArray and
   StringIterable. Not including these at the moment. */
#pragma ACME enable CORRADE_SINGLES_NO_ADVANCED_STRING_APIS

/* This macro is used only for Corrade's own tests */
#pragma ACME disable CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH

/* Make the CPU runtime dispatch enabled by default, which is the inverse of
   what's done in Corrade itself */
#ifndef CORRADE_NO_CPU_RUNTIME_DISPATCH
#define CORRADE_BUILD_CPU_RUNTIME_DISPATCH
#endif

/* CorradeEnumSet.h is a dependency, remove all of Containers/EnumSet.h and
   hide everything it already includes */
#pragma ACME noexpand CorradeEnumSet.h
#pragma ACME enable Corrade_Containers_EnumSet_h
#include "CorradeEnumSet.h"

/* From Containers.h we need just the String, StringView and Pair forward
   declarations for uses before they're defined, the EnumSet is already in
   CorradeEnumSet.h. */
#pragma ACME enable Corrade_Containers_Containers_h

/* Disable asserts that are not used. CORRADE_CONSTEXPR_DEBUG_ASSERT is used in
   the headers, wrapping the #include <cassert> above. When enabling additional
   asserts, be sure to update it above as well -- without the _DEBUG variants,
   as they just delegate to the non-debug version of the macro. Other asserts
   are then used in the implementation below. */
#include "assert.h"
#pragma ACME forget CORRADE_CONSTEXPR_ASSERT
#pragma ACME forget CORRADE_CONSTEXPR_DEBUG_ASSERT

#ifndef CorradeString_hpp
#define CorradeString_hpp
/* Reduced contents of the visibility header */
#pragma ACME enable Corrade_Utility_visibility_h
#ifndef CORRADE_UTILITY_EXPORT
#define CORRADE_UTILITY_EXPORT
#endif
#ifndef CORRADE_UTILITY_LOCAL
#define CORRADE_UTILITY_LOCAL
#endif
#if defined(CORRADE_BUILD_CPU_RUNTIME_DISPATCH) && !defined(CORRADE_CPU_USE_IFUNC)
    #define CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(name)                \
        CORRADE_UTILITY_EXPORT decltype(name) name ## Implementation(Cpu::Features);
    #define CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(name) (*name)
/* IFUNC or compile-time CPU dispatch, runtime dispatcher is not exposed */
#else
    #define CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(name)
    #define CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(name) (name)
#endif

namespace Corrade { namespace Containers {

class String;
template<class> class BasicStringView;
typedef BasicStringView<const char> StringView;
template<class, class> class Pair;

}}
#endif
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/String.h"
#ifdef CORRADE_STRING_STL_COMPATIBILITY
// {{includes}}
/* Include <string> directly, don't bother with forward-declarations here */
#include <string>
#pragma ACME enable Corrade_Utility_StlForwardString_h
#pragma ACME disable CORRADE_STRING_STL_INLINE
#include "Corrade/Containers/StringStl.h"
#endif
// TODO CORRADE_STRING_STL_HASH_COMPATIBILITY once AbstractHash is <string>-free
#ifdef CORRADE_STRING_STL_VIEW_COMPATIBILITY
// {{includes}}
#include "Corrade/Containers/StringStlView.h"
#endif
#ifdef CORRADE_STRING_IMPLEMENTATION
// {{includes}}
/* CORRADE_CONSTEXPR{,_DEBUG}_ASSERT already present above. The implementation
   then uses CORRADE_ASSERT, CORRADE_DEBUG_ASSERT,
   CORRADE_INTERNAL_DEBUG_ASSERT and
   CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE. */
#pragma ACME forget <cassert>
#pragma ACME forget "Corrade/Utility/Assert.h"
#pragma ACME forget "Corrade/Utility/DebugAssert.h"
#pragma ACME forget CORRADE_ASSERT
#pragma ACME forget CORRADE_DEBUG_ASSERT
#pragma ACME forget CORRADE_INTERNAL_ASSERT
#pragma ACME forget CORRADE_INTERNAL_DEBUG_ASSERT
#pragma ACME forget CORRADE_INTERNAL_ASSERT_UNREACHABLE
#pragma ACME forget CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_CONSTEXPR_DEBUG_ASSERT
/* Include <cassert> again in case the assert macros used above were all custom
   defined and so <cassert> didn't get included */
#if (!defined(CORRADE_ASSERT) || !defined(CORRADE_INTERNAL_ASSERT) || !defined(CORRADE_INTERNAL_ASSERT_UNREACHABLE)) && !defined(NDEBUG)
#include <cassert>
#endif

#include "Corrade/Utility/DebugAssert.h"

/* Only CORRADE_FALLTHROUGH, CORRADE_NOOP, CORRADE_PASSTHROUGH and
   CORRADE_UNUSED is used from Macros.h */
#pragma ACME disable CORRADE_FALLTHROUGH
#pragma ACME disable CORRADE_NOOP
#pragma ACME disable CORRADE_PASSTHROUGH
#pragma ACME disable CORRADE_UNUSED

/* CorradePair.h and CorradeCpu.hpp is a dependency for the implementation,
   remove all of Cpu.h and hide everything it already includes */
#pragma ACME noexpand CorradePair.h
#pragma ACME noexpand CorradeCpu.hpp
#pragma ACME enable Corrade_Containers_Pair_h
#pragma ACME enable Corrade_Cpu_h
#include "CorradePair.h"
#include "CorradeCpu.hpp"

/* Extracted from StringView.cpp as otherwise the includes don't stay inside
   correct ifdefs; replacing the GCC 4.8 compatibility intrinsic headers with
   direct includes because the BMI / POPCNT builtins don't work there anyway */
#pragma ACME enable Corrade_Utility_IntrinsicsAvx_h
#pragma ACME enable Corrade_Utility_IntrinsicsSse4_h
#if ((defined(CORRADE_ENABLE_SSE2) || defined(CORRADE_ENABLE_AVX)) && defined(CORRADE_ENABLE_BMI1)) || (defined(CORRADE_ENABLE_AVX) && defined(CORRADE_ENABLE_POPCNT))
#include <immintrin.h>
#elif defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_POPCNT)
#include <smmintrin.h>
#include <nmmintrin.h>
#endif
#ifdef CORRADE_ENABLE_NEON
#include <arm_neon.h>
#endif
#ifdef CORRADE_ENABLE_SIMD128
#include <wasm_simd128.h>
#endif
#include "Corrade/Containers/StringView.cpp"
#include "Corrade/Containers/String.cpp"
#endif
