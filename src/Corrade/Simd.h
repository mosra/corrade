#ifndef Corrade_Simd_h
#define Corrade_Simd_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Namespace @ref Corrade::Simd
 * @m_since_latest
 */

#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade {

/**
@brief Compile- and run-time CPU instruction set detection and dispatch
@m_since_latest

This namespace provides *tags* for x86, ARM and WebAssembly instruction sets,
which can be used for either system introspection or for choosing a particular
implementation based on the available instruction set. These tags build on top
of the @ref CORRADE_TARGET_SSE2, @ref CORRADE_TARGET_SSE3 etc. preprocessor
macros and provide a runtime feature detection as well.

This library is built if `WITH_UTILITY` is enabled when building Corrade. To
use this library with CMake, request the `Utility` component of the `Corrade`
package and link to the `Corrade::Utility` target:

@code{.cmake}
find_package(Corrade REQUIRED Utility)

# ...
target_link_libraries(your-app PRIVATE Corrade::Utility)
@endcode

@section Simd-compile-time Compile-time CPU instruction set detection and dispatch

Similarly to constructor tags such as @ref Containers::NoInit, the @ref Simd
namespace contains several tags for particular instruction sets ---
@ref Simd::Sse2, @ref Simd::Sse3, @ref Simd::Ssse3 etc. on
@ref CORRADE_TARGET_X86 "x86", @ref Simd::Neon on @ref CORRADE_TARGET_ARM "ARM"
and @ref Simd::Simd128 on @ref CORRADE_TARGET_WASM "WebAssembly". With those,
you can provide alternative implementations of a certain algorithm:

@snippet Corrade.cpp Simd-compile-time-declare

Then you can either call a particular implementation directly, or there's
@ref Simd::Default, which is a convenience alias to the latest available
instruction set based on which `CORRADE_TARGET_*` macros are defined by
@ref Corrade/Corrade.h, reflecting compiler settings. The tags are defined in a
way that makes the compiler pick the most advanced implementation for a
certain instruction set:

@snippet Corrade.cpp Simd-compile-time-use

@section Simd-runtime Runtime CPU instruction set detection and dispatch

It's not always practical to rely just on compile-time detection --- when an
application binary is shipped, the majority of the code uses the lowest common
denominator (for example just SSE2 on x86), while the most demanding functions
are compiled to make use of more advanced instructions for better performance.
Then, based on features detected at runtime, a particular implementation is
called. The @ref Simd::Features class in combination with the tags can be used
for this purpose, its interface is similar to @ref Containers::EnumSet and the
usual bitwise operations are supported:

@snippet Corrade.cpp Simd-runtime-dispatch

A default-constructed @ref Simd::Features instance performs a runtime detection
and contains all detected CPU instruction sets, so performing a runtime
dispatch is then as simple as this:

@snippet Corrade.cpp Simd-runtime-use

To avoid doing the (potentially expensive) feature detection each time, you can
save the @ref Simd::Features instance and reuse it. Like with compile-time
dispatch, it's also possible to force a particular feature directly or modify
the detected set in some way:

@snippet Corrade.cpp Simd-runtime-cache

<b></b>

@m_class{m-note m-success}

@par SSE3, SSSE3, SSE4.1/SSE4.2, AVX F16C and AVX FMA on MSVC
    A special case worth mentioning are SSE3 and newer instructions on Windows.
    MSVC only provides a very coarse `/arch:SSE2`, `/arch:AVX` and `/arch:AVX2`
    for either @ref Sse2, @ref Avx or @ref Avx2, but nothing in between. That
    means it's impossible to rely just on compile-time detection to use the
    later SSE features on machines that don't support AVX yet (or the various
    AVX additions on machines without AVX2), you have to use runtime dispatch
    there.

@todo POPCNT!!! for the Math function, LZCNT ... treat them as a step between
    4.2 and AVX? or as a subset of AVX?
*/
namespace Simd {

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class> struct TypeTraits;
#else
/**
@brief Traits class for SIMD tag types

Useful for detecting tag properties at compile time without the need for
repeated code such as method overloading, cascaded ifs or template
specializations for all tag types. All tag types have this class implemented.
*/
template<class T> struct TypeTraits {
    enum: std::uint32_t {
        /**
         * Tag-specific index. Implementation-defined, is unique among all
         * tags on given platform.
         */
        Index
    };

    /**
     * @brief Tag name
     *
     * Returns a string representation of the tag, such as @cpp "AvxFma" @ce
     * for @ref AvxFma.
     */
    static const char* name();
};
#endif

/**
@brief Scalar tag type
@m_since_latest

See the @ref Simd namespace and the @ref Scalar tag for more information.
*/
struct ScalarT {};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Scalar code is when nothing else is available, thus no bits set */
template<> struct TypeTraits<ScalarT> {
    enum: std::uint32_t { Index = 0 };
    static const char* name() { return "Scalar"; }
};
#endif

#if defined(CORRADE_TARGET_X86) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SSE2 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Sse2 tag for more information.
*/
struct Sse2T: ScalarT {};

/**
@brief SSE3 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Sse3 tag for more information.
*/
struct Sse3T: Sse2T {};

/**
@brief SSSE3 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Ssse3 tag for more information.
*/
struct Ssse3T: Sse3T {};

/**
@brief SSE4.1 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Sse41T tag for more information.
*/
struct Sse41T: Ssse3T {};

/**
@brief SSE4.2 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Sse42T tag for more information.
*/
struct Sse42T: Sse41T {};

/**
@brief AVX tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Avx tag for more information.
*/
struct AvxT: Sse42T {};

/**
@brief AVX F16C tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref AvxF16c tag for more information.
*/
struct AvxF16cT: AvxT {};

/**
@brief AVX FMA tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref AvxFma tag for more information.
*/
struct AvxFmaT: AvxF16cT {};

/**
@brief AVX2 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Avx2 tag for more information.
*/
struct Avx2T: AvxFmaT {};

/**
@brief AVX-512 Foundation tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Simd namespace
and the @ref Avx512f tag for more information.
*/
struct Avx512fT: Avx2T {};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<Sse2T> {
    enum: std::uint32_t { Index = 1 << 0 };
    static const char* name() { return "Sse2"; }
};
template<> struct TypeTraits<Sse3T> {
    enum: std::uint32_t { Index = 1 << 1 };
    static const char* name() { return "Sse3"; }
};
template<> struct TypeTraits<Ssse3T> {
    enum: std::uint32_t { Index = 1 << 2 };
    static const char* name() { return "Ssse3"; }
};
template<> struct TypeTraits<Sse41T> {
    enum: std::uint32_t { Index = 1 << 3 };
    static const char* name() { return "Sse41"; }
};
template<> struct TypeTraits<Sse42T> {
    enum: std::uint32_t { Index = 1 << 4 };
    static const char* name() { return "Sse42"; }
};
template<> struct TypeTraits<AvxT> {
    enum: std::uint32_t { Index = 1 << 5 };
    static const char* name() { return "Avx"; }
};
template<> struct TypeTraits<AvxF16cT> {
    enum: std::uint32_t { Index = 1 << 6 };
    static const char* name() { return "AvxF16c"; }
};
template<> struct TypeTraits<AvxFmaT> {
    enum: std::uint32_t { Index = 1 << 7 };
    static const char* name() { return "AvxFma"; }
};
template<> struct TypeTraits<Avx2T> {
    enum: std::uint32_t { Index = 1 << 8 };
    static const char* name() { return "Avx2"; }
};
template<> struct TypeTraits<Avx512fT> {
    enum: std::uint32_t { Index = 1 << 9 };
    static const char* name() { return "Avx512f"; }
};
#endif
#endif

#if defined(CORRADE_TARGET_ARM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief NEON tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Simd namespace
and the @ref Neon tag for more information.
*/
struct NeonT: ScalarT {};

/**
@brief NEON half-float tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Simd namespace
and the @ref NeonFp16 tag for more information.
*/
struct NeonFp16T {};

/**
@brief NEON FMA tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Simd namespace
and the @ref NeonFma tag for more information.
*/
struct NeonFmaT {};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<NeonT> {
    enum: std::uint32_t { Index = 1 << 0 };
    static const char* name() { return "Neon"; }
};
template<> struct TypeTraits<NeonFp16T> {
    enum: std::uint32_t { Index = 1 << 1 };
    static const char* name() { return "NeonFp16"; }
};
template<> struct TypeTraits<NeonFmaT> {
    enum: std::uint32_t { Index = 1 << 2 };
    static const char* name() { return "NeonFma"; }
};
#endif
#endif

#if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SIMD128 tag type
@m_since_latest

Available only on @ref CORRADE_TARGET_WASM "WebAssembly". See the @ref Simd
namespace and the @ref Simd128 tag for more information.
*/
struct Simd128T: ScalarT {};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<Simd128T> {
    enum: std::uint32_t { Index = 1 << 0 };
    static const char* name() { return "Simd128"; }
};
#endif
#endif

/**
@brief Default tag type
@m_since_latest

See the @ref Default tag for more information.
*/
#ifdef CORRADE_TARGET_X86
    #ifdef CORRADE_TARGET_AVX512F
    typedef Avx512fT DefaultT;
    #elif defined(CORRADE_TARGET_AVX2)
    typedef Avx2T DefaultT;
    #elif defined(CORRADE_TARGET_AVX_FMA)
    typedef AvxFmaT DefaultT;
    #elif defined(CORRADE_TARGET_AVX_F16C)
    typedef AvxF16cT DefaultT;
    #elif defined(CORRADE_TARGET_AVX)
    typedef AvxT DefaultT;
    #elif defined(CORRADE_TARGET_SSE42)
    typedef Sse42T DefaultT;
    #elif defined(CORRADE_TARGET_SSE41)
    typedef Sse41T DefaultT;
    #elif defined(CORRADE_TARGET_SSSE3)
    typedef Ssse3T DefaultT;
    #elif defined(CORRADE_TARGET_SSE3)
    typedef Sse3T DefaultT;
    #elif defined(CORRADE_TARGET_SSE2)
    typedef Sse2T DefaultT;
    #else
    typedef ScalarT DefaultT;
    #endif
#elif defined(CORRADE_TARGET_ARM)
    #ifdef CORRADE_TARGET_NEON
    typedef NeonT DefaultT;
    #else
    typedef ScalarT DefaultT;
    #endif
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
    #ifdef CORRADE_TARGET_SIMD128
    typedef Simd128T DefaultT;
    #else
    typedef ScalarT DefaultT;
    #endif
#else
    typedef ScalarT DefaultT;
#endif

/**
@brief Scalar tag
@m_since_latest

Code that isn't explicitly optimized with any particular SIMD optimizations.
Fallback if no other SIMD instruction set is chosen or available. The
next most widely supported instruction sets are @ref Sse2 on x86, @ref Neon on
ARM and @ref Simd128 on WebAssembly.
*/
constexpr ScalarT Scalar{};

#if defined(CORRADE_TARGET_X86) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SSE2 tag
@m_since_latest

[Streaming SIMD Extensions 2](https://en.wikipedia.org/wiki/SSE2). Available
only on @ref CORRADE_TARGET_X86 "x86", supported by all 64-bit x86 processors
and is present on majority of contemporary 32-bit x86 processors as well.
Superset of @ref Scalar, implied by @ref Sse3.
@see @ref CORRADE_TARGET_SSE2
*/
constexpr Sse2T Sse2{};

/**
@brief SSE3 tag
@m_since_latest

[Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSE3). Available
only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse2, implied by
@ref Ssse3.
@see @ref CORRADE_TARGET_SSE3
*/
constexpr Sse3T Sse3{};

/**
@brief SSSE3 tag
@m_since_latest

[Supplemental Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSSE3).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse3, implied
by @ref Sse41.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref Sse41 to handle those.
@see @ref CORRADE_TARGET_SSSE3
*/
constexpr Ssse3T Ssse3{};

/**
@brief SSE4.1 tag
@m_since_latest

[Streaming SIMD Extensions 4.1](https://en.wikipedia.org/wiki/SSE4#SSE4.1).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Ssse3,
implied by @ref Sse42.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref Sse41 to handle those.
@see @ref CORRADE_TARGET_SSE41
*/
constexpr Sse41T Sse41{};

/**
@brief SSE4.2 tag
@m_since_latest

[Streaming SIMD Extensions 4.2](https://en.wikipedia.org/wiki/SSE4#SSE4.2).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse41,
implied by @ref Avx.
@see @ref CORRADE_TARGET_SSE42
*/
constexpr Sse42T Sse42{};

/**
@brief AVX tag
@m_since_latest

[Advanced Vector Extensions](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse42,
implied by @ref AvxFma.
@see @ref CORRADE_TARGET_AVX
*/
constexpr AvxT Avx{};

/**
@brief AVX F16C tag
@m_since_latest

[F16C]https://en.wikipedia.org/wiki/F16C) instructions. Available only on
@ref CORRADE_TARGET_X86 "x86". Superset of @ref Avx, implied by @ref AvxFma.
@see @ref CORRADE_TARGET_AVX_F16C
*/
constexpr AvxF16cT AvxF16c{};

/**
@brief AVX FMA tag
@m_since_latest

[FMA3 instruction set](https://en.wikipedia.org/wiki/FMA_instruction_set).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref AvxF16c,
implied by @ref Avx2.
@see @ref CORRADE_TARGET_AVX_FMA
*/
constexpr AvxFmaT AvxFma{};

/**
@brief AVX2 tag
@m_since_latest

[Advanced Vector Extensions 2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref AvxFma,
implied by @ref Avx512f.
@see @ref CORRADE_TARGET_AVX2
*/
constexpr Avx2T Avx2{};

/**
@brief AVX-512 Foundation tag
@m_since_latest

[AVX-512](https://en.wikipedia.org/wiki/AVX-512) Foundation. Available only on
@ref CORRADE_TARGET_X86 "x86". Superset of @ref Avx2.
@see @ref CORRADE_TARGET_AVX512F
*/
constexpr Avx512fT Avx512f{};
#endif

#if defined(CORRADE_TARGET_ARM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief NEON tag type
@m_since_latest

[ARM NEON](https://en.wikipedia.org/wiki/ARM_architecture#Advanced_SIMD_(Neon)).
Available only on @ref CORRADE_TARGET_ARM "ARM". Superset of @ref Scalar,
implied by @ref NeonFp16.
@see @ref CORRADE_TARGET_NEON
*/
constexpr NeonT Neon{};

/**
@brief NEON half-float tag type
@m_since_latest

ARM NEON with IEEE [half-precision floating-point support](https://en.wikipedia.org/wiki/Half-precision_floating-point_format).
Available only on @ref CORRADE_TARGET_ARM "ARM". Superset of @ref Neon, implied
by @ref NeonFma.
@see @ref CORRADE_TARGET_NEON_FP16
*/
constexpr NeonFp16T NeonFp16{};

/**
@brief NEON FMA tag type
@m_since_latest

ARM NEON with FMA instructions. Available only on
@ref CORRADE_TARGET_ARM "ARM". Superset of @ref NeonFp16.
@see @ref CORRADE_TARGET_NEON_FMA
*/
constexpr NeonFmaT NeonFma{};
#endif

#if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SIMD128 tag type
@m_since_latest

[128-bit WebAssembly SIMD](https://github.com/webassembly/simd). Available only
on @ref CORRADE_TARGET_WASM "WebAssembly". Superset of @ref Scalar.
@see @ref CORRADE_TARGET_SIMD128
*/
constexpr Simd128T Simd128{};
#endif

/**
@brief Default tag
@m_since_latest

Highest instruction set available on given architecture with current compiler
flags. On @ref CORRADE_TARGET_X86 it's one of these:

-   @ref Avx512f if @ref CORRADE_TARGET_AVX512F is defined
-   @ref Avx2 if @ref CORRADE_TARGET_AVX2 is defined
-   @ref AvxFma if @ref CORRADE_TARGET_AVX_FMA is defined
-   @ref AvxF16c if @ref CORRADE_TARGET_AVX_F16C is defined
-   @ref Avx if @ref CORRADE_TARGET_AVX is defined
-   @ref Sse42 if @ref CORRADE_TARGET_SSE42 is defined
-   @ref Sse41 if @ref CORRADE_TARGET_SSE41 is defined
-   @ref Ssse3 if @ref CORRADE_TARGET_SSSE3 is defined
-   @ref Sse3 if @ref CORRADE_TARGET_SSE3 is defined
-   @ref Sse2 if @ref CORRADE_TARGET_SSE2 is defined
-   @ref Scalar otherwise

On @ref CORRADE_TARGET_ARM it's one of these:

-   @ref NeonFma if @ref CORRADE_TARGET_NEON_FMA is defined
-   @ref NeonFp16 if @ref CORRADE_TARGET_NEON_FP16 is defined
-   @ref Neon if @ref CORRADE_TARGET_NEON is defined
-   @ref Scalar otherwise

On @ref CORRADE_TARGET_EMSCRIPTEN it's one of these:

-   @ref Simd128 if @ref CORRADE_TARGET_SIMD128 is defined
-   @ref Scalar otherwise
*/
constexpr DefaultT Default{};

/**
@brief Feature set
@m_since_latest

Provides storage and comparison as well as runtime detection of CPU instruction
set. Provides an interface similar to an @ref Containers::EnumSet, with values
being the @ref Simd::Sse2, @ref Simd::Sse3 etc. tags.

See the @ref Simd namespace for an overview and usage examples.
*/
class CORRADE_UTILITY_EXPORT Features {
    public:
        /**
         * @brief Construct with runtime-detected CPU instruction set
         *
         * On @ref CORRADE_TARGET_X86 "x86" and GCC, Clang or MSVC uses the
         * `__get_cpuid()` / `__cpuid()` builtin to check for the @ref Sse2,
         * @ref Sse3, @ref Ssse3, @ref Sse41, @ref Sse42, @ref Avx,
         * @ref AvxF16c, @ref AvxFma, @ref Avx2 and @ref Avx512f runtime
         * features. @ref Avx needs OS support as well, if it's not present, no
         * following flags are checked either. On compilers other than GCC,
         * Clang and MSVC the check falls back to checking the preprocessor
         * @ref CORRADE_TARGET_SSE2 etc. variables.
         *
         * On @ref CORRADE_TARGET_ARM "ARM", detecting @ref Neon is very
         * involved including blacklisting certain chips that misreport their
         * capabilities and thus it's reported only if the preprocessor
         * variable @ref CORRADE_TARGET_NEON is set, similarly for
         * @ref NeonFp16 and @ref NeonFma.
         *
         * On @ref CORRADE_TARGET_WASM "WebAssembly" an attempt to use
         * @ref Simd128 instructions without runtime support results in a
         * WebAssembly compilation error and thus runtime detection is largely
         * meaningless. @ref Simd128 is reported only if the preprocessor
         * variable @ref CORRADE_TARGET_SIMD128 is set.
         *
         * If no known CPU instruction set is detected, the instance is equal
         * to @ref Scalar.
         */
        explicit Features();

        /** @brief Construct from a tag */
        template<class T, class = decltype(TypeTraits<T>::Index)> constexpr /*implicit*/ Features(T) noexcept: _data{TypeTraits<T>::Index} {}

        /** @brief Equality comparison */
        constexpr bool operator==(Features other) const {
            return _data == other._data;
        }

        /** @brief Non-equality comparison */
        constexpr bool operator!=(Features other) const {
            return _data != other._data;
        }

        /**
         * @brief Whether @p other is a subset of this (@f$ a \supseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == other @ce.
         */
        constexpr bool operator>=(Features other) const {
            return (_data & other._data) == other._data;
        }

        /**
         * @brief Whether @p other is a superset of this (@f$ a \subseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == a @ce.
         */
        constexpr bool operator<=(Features other) const {
            return (_data & other._data) == _data;
        }

        /** @brief Union of two feature sets */
        constexpr Features operator|(Features other) const {
            return Features(_data | other._data);
        }

        /** @brief Union two feature sets and assign */
        Features& operator|=(Features other) {
            _data |= other._data;
            return *this;
        }

        /** @brief Intersection of two feature sets */
        constexpr Features operator&(Features other) const {
            return Features(_data & other._data);
        }

        /** @brief Intersect two feature sets and assign */
        Features& operator&=(Features other) {
            _data &= other._data;
            return *this;
        }

        /** @brief XOR of two feature sets */
        constexpr Features operator^(Features other) const {
            return Features(_data ^ other._data);
        }

        /** @brief XOR two feature sets and assign */
        Features& operator^=(Features other) {
            _data ^= other._data;
            return *this;
        }

        /** @brief Feature set complement */
        constexpr Features operator~() const {
            return Features(~_data);
        }

        /**
         * @brief Boolean conversion
         *
         * Returns @cpp true @ce if at least one feature apart from
         * @ref Simd::Scalar is present, @cpp false @ce otherwise.
         */
        constexpr explicit operator bool() const {
            return _data != 0;
        }

        /**
         * @brief Integer representation
         *
         * For testing purposes. @ref Simd::Scalar is always @cpp 0 @ce, values
         * corresponding to other feature tags are unspecified.
         */
        constexpr explicit operator std::uint32_t() const {
            return _data;
        }

    private:
        constexpr explicit Features(std::uint32_t data): _data{data} {}

        std::uint32_t _data;
};

/** @relates Features
@brief Equality comparison of a tag and a feature set

Same as @ref Features::operator==().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr bool operator==(T a, Features b) {
    return Features(a) == b;
}

/** @relates Features
@brief Non-equality comparison of a tag and a feature set

Same as @ref Features::operator!=().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr bool operator!=(T a, Features b) {
    return Features(a) != b;
}

/** @relates Features
@brief Whether @p a is a superset of @p b (@f$ a \supseteq b @f$)

Same as @ref Features::operator>=().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr bool operator>=(T a, Features b) {
    return Features(a) >= b;
}

/** @relates Features
@brief Whether @p a is a subset of @p b (@f$ a \subseteq b @f$)

Same as @ref Features::operator<=().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr bool operator<=(T a, Features b) {
    return Features(a) <= b;
}

/** @relates Features
@brief Union of two feature sets

Same as @ref Features::operator|().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr Features operator|(T a, Features b) {
    return b | a;
}

/** @relates Features
@brief Intersection of two feature sets

Same as @ref Features::operator&().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr Features operator&(T a, Features b) {
    return b & a;
}

/** @relates Features
@brief XOR of two feature sets

Same as @ref Features::operator^().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr Features operator^(T a, Features b) {
    return b ^ a;
}

/** @relates Features
@brief Feature set complement

Same as @ref Features::operator~().
*/
template<class T, class = decltype(TypeTraits<T>::Index)> constexpr Features operator~(T a) {
    return ~Features(a);
}

/**
@debugoperator{Features}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, Features value);

/** @relates Features
@overload
@m_since_latest
*/
template<class T, class = decltype(TypeTraits<T>::Index)> inline Utility::Debug& operator<<(Utility::Debug& debug, T value) {
    return operator<<(debug, Features{value});
}

}

}

#if defined(CORRADE_TARGET_X86) || defined(DOXYGEN_GENERATING_OUTPUT)

// #error mention that it *might* be good to actually compile with /arch: on msvc because not doing that will inhibit the optimizer
// #error also not enabling /arch:AVX will cause sse/avx switch overhead
#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_SSE2 __attribute__((__target__("sse2")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_SSE2
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_SSE3 __attribute__((__target__("sse3")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_SSE3
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_SSSE3 __attribute__((__target__("ssse3")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_SSSE3
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_SSE41 __attribute__((__target__("sse4.1")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_SSE41
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_SSE42 __attribute__((__target__("sse4.2")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_SSE42
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_AVX __attribute__((__target__("avx")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_AVX
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_AVX_F16C __attribute__((__target__("avx,f16c")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_AVX_F16C
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_AVX_FMA __attribute__((__target__("avx,fma")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_AVX_FMA
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_AVX2 __attribute__((__target__("avx2")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_AVX2
#endif

// #error GCC since 4.9, msvc since 2017 15.3 https://devblogs.microsoft.com/cppblog/microsoft-visual-studio-2017-supports-intel-avx-512/
#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_AVX512F __attribute__((__target__("avx512f")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_AVX512F
#endif

#elif defined(CORRADE_TARGET_NEON) || defined(DOXYGEN_GENERATING_OUTPUT)

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_NEON __attribute__((__target__("fpu=neon")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_NEON
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_NEON_FP16 __attribute__((__target__("fpu=neon-fp16")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_NEON_FP16
#endif

#ifdef CORRADE_TARGET_GCC
#define CORRADE_ENABLE_NEON_FMA __attribute__((__target__("fpu=neon-vfpv4")))
#elif defined(CORRADE_TARGET_MSVC) || defined(DOXYGEN_GENERATING_OUTPUT)
#define CORRADE_ENABLE_NEON_FMA
#endif

#elif defined(CORRADE_TARGET_WASM) || defined(DOXYGEN_GENERATING_OUTPUT)

#ifdef CORRADE_TARGET_SIMD128
#define CORRADE_ENABLE_SIMD128 __attribute__((__target__("simd128")))
#endif

#endif

#endif
