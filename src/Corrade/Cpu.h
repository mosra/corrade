#ifndef Corrade_Cpu_h
#define Corrade_Cpu_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
 * @brief Namespace @ref Corrade::Cpu
 * @m_since_latest
 */

#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade {

/**
@brief Compile-time and runtime CPU instruction set detection and dispatch
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

@section Cpu-usage Usage

The @ref Cpu namespace contains tags such as @ref Cpu::Avx2, @ref Cpu::Sse2,
@ref Cpu::Neon or @ref Cpu::Simd128. These tags behave similarly to enum values
and their combination result in @ref Cpu::Features, which is similar to the
@ref Containers::EnumSet class --- they support the same bitwise operations,
can be tested for subsets and supersets, and are printable with
@ref Utility::Debug.

The most advanced base CPU instruction set enabled at compile time is then
exposed through the @ref Cpu::DefaultBase variable, which is an alias to one of
those tags, and it matches the architecture-specific @ref CORRADE_TARGET_SSE2
etc. macros. Since it's a @cpp constexpr @ce variable, it's usable in a
compile-time context. The most straightforward use is shown in the following
C++17 snippet:

@snippet Corrade-cpp17.cpp Cpu-usage-compile-time

<b></b>

@m_class{m-note m-info}

@par
    If you're writing multiplatform code targeting multiple architectures, you
    still need to partially rely on the preprocessor when using the
    architecture-specific tags, as those are defined only on the architecture
    they apply to. The above would need to be wrapped in
    @cpp #ifdef CORRADE_TARGET_X86 @ce; if you would be checking for
    @ref Cpu::Neon instead, then you'd need to wrap it in a
    @ref CORRADE_TARGET_ARM check. On the other hand, the per-architecture tags
    are available on given architecture always --- so for example
    @ref Cpu::Avx512f is present even on a compiler that doesn't even recognize
    AVX-512 yet.

@subsection Cpu-usage-dispatch-compile-time Dispatching on available CPU instruction set at compile time

The main purpose of these tags, however, is to provide means for a compile-time
overload resolution. In other words, picking the best candidate among a set of
functions implemented with various instruction sets. As an example, let's say
you have three different implementations of a certain algorithm transforming
numeric data. One is using AVX2 instructions, another is a slower variant using
just SSE 4.2 and as a fallback there's one with just regular scalar code. To
distinguish them, the functions have the same name, but use a different *tag
type*:

@snippet Corrade.cpp Cpu-usage-declare

Then you can either call a particular implementation directly --- for example
to test it --- or you can pass @ref Cpu::DefaultBase, and it'll pick the best
overload candidate for the set of CPU instruction features enabled at compile
time:

@snippet Corrade.cpp Cpu-usage-compile-time-call

-   If the user code was compiled with AVX2 or higher enabled, the
    @ref Cpu::Avx2 overload will be picked.
-   Otherwise, if just AVX, SSE 4.2 or anything else that includes SSE 4.2 was
    enabled, the @ref Cpu::Sse42 overload will be picked.
-   Otherwise (for example when compiling for generic x86-64 that has just the
    SSE2 feature set), the @ref Cpu::Scalar overload will be picked. If you
    wouldn't provide this overload, the compilation would fail for such a
    target --- which is useful for example to enforce a certain CPU feature set
    to be enabled in order to use a certain API.

<b></b>

@m_class{m-block m-warning}

@par SSE3, SSSE3 and SSE4.1/SSE4.2 on MSVC
    A special case worth mentioning are SSE3 and newer instructions on Windows.
    MSVC only provides a very coarse `/arch:SSE2`, `/arch:AVX` and `/arch:AVX2`
    for either @ref Sse2, @ref Avx or @ref Avx2, but nothing in between. That
    means it's impossible to rely just on compile-time detection to use the
    later SSE features on machines that don't support AVX yet (or the various
    AVX additions on machines without AVX2), you have to use runtime dispatch
    there, as shown below.

@subsection Cpu-usage-dispatch-runtime Runtime detection and manual dispatch

So far that was all compile-time detection, which has use mainly when a binary
can be optimized directly for the machine it will run on. But such approach is
not practical when shipping to a heterogenous set of devices. Instead, the
usual workflow is that the majority of code uses the lowest common
denominator (such as SSE2 on x86), with the most demanding functions having
alternative implementations --- picked at runtime --- that make use of more
advanced instructions for better performance.

Runtime detection is exposed through @ref Cpu::runtimeFeatures(). It will
detect CPU features on platforms that support it, and fall back to
@ref Cpu::compiledFeatures() on platforms that don't. You can then match the
returned @ref Cpu::Features against particular tags to decide which variant to
use:

@snippet Corrade.cpp Cpu-usage-runtime-manual-dispatch
*/
namespace Cpu {

/**
@brief Traits class for CPU detection tag types

Useful for detecting tag properties at compile time without the need for
repeated code such as method overloading, cascaded ifs or template
specializations for all tag types. All tag types in the @ref Cpu namespace
have this class implemented.
*/
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class> struct TypeTraits;
#else
template<class T> struct TypeTraits {
    enum: unsigned int {
        /**
         * Tag-specific index. Implementation-defined, is unique among all tags
         * on given platform.
         */
        Index
    };

    /**
     * @brief Tag name
     *
     * Returns a string representation of the tag, such as @cpp "Avx2" @ce
     * for @ref Avx2.
     */
    static const char* name();
};
#endif

namespace Implementation {
    /* A common type used in all tag constructors to avoid ambiguous calls when
       using {} */
    struct InitT {};
    constexpr InitT Init{};
}

/**
@brief Scalar tag type

See the @ref Cpu namespace and the @ref Scalar tag for more information.
*/
struct ScalarT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit ScalarT(Implementation::InitT) {}
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Scalar code is when nothing else is available, thus no bits set */
template<> struct TypeTraits<ScalarT> {
    enum: unsigned int { Index = 0 };
    static const char* name() { return "Scalar"; }
};
#endif

#if defined(CORRADE_TARGET_X86) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SSE2 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Sse2 tag for more information.
*/
struct Sse2T: ScalarT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Sse2T(Implementation::InitT): ScalarT{Implementation::Init} {}
    #endif
};

/**
@brief SSE3 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Sse3 tag for more information.
*/
struct Sse3T: Sse2T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Sse3T(Implementation::InitT): Sse2T{Implementation::Init} {}
    #endif
};

/**
@brief SSSE3 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Ssse3 tag for more information.
*/
struct Ssse3T: Sse3T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Ssse3T(Implementation::InitT): Sse3T{Implementation::Init} {}
    #endif
};

/**
@brief SSE4.1 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Sse41T tag for more information.
*/
struct Sse41T: Ssse3T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Sse41T(Implementation::InitT): Ssse3T{Implementation::Init} {}
    #endif
};

/**
@brief SSE4.2 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Sse42T tag for more information.
*/
struct Sse42T: Sse41T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Sse42T(Implementation::InitT): Sse41T{Implementation::Init} {}
    #endif
};

/**
@brief AVX tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Avx tag for more information.
*/
struct AvxT: Sse42T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit AvxT(Implementation::InitT): Sse42T{Implementation::Init} {}
    #endif
};

/**
@brief AVX2 tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Avx2 tag for more information.
*/
struct Avx2T: AvxT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Avx2T(Implementation::InitT): AvxT{Implementation::Init} {}
    #endif
};

/**
@brief AVX-512 Foundation tag type

Available only on @ref CORRADE_TARGET_X86 "x86". See the @ref Cpu namespace
and the @ref Avx512f tag for more information.
*/
struct Avx512fT: Avx2T {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Avx512fT(Implementation::InitT): Avx2T{Implementation::Init} {}
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<Sse2T> {
    enum: unsigned int { Index = 1 << 0 };
    static const char* name() { return "Sse2"; }
};
template<> struct TypeTraits<Sse3T> {
    enum: unsigned int { Index = 1 << 1 };
    static const char* name() { return "Sse3"; }
};
template<> struct TypeTraits<Ssse3T> {
    enum: unsigned int { Index = 1 << 2 };
    static const char* name() { return "Ssse3"; }
};
template<> struct TypeTraits<Sse41T> {
    enum: unsigned int { Index = 1 << 3 };
    static const char* name() { return "Sse41"; }
};
template<> struct TypeTraits<Sse42T> {
    enum: unsigned int { Index = 1 << 4 };
    static const char* name() { return "Sse42"; }
};
template<> struct TypeTraits<AvxT> {
    enum: unsigned int { Index = 1 << 5 };
    static const char* name() { return "Avx"; }
};
template<> struct TypeTraits<Avx2T> {
    enum: unsigned int { Index = 1 << 6 };
    static const char* name() { return "Avx2"; }
};
template<> struct TypeTraits<Avx512fT> {
    enum: unsigned int { Index = 1 << 7 };
    static const char* name() { return "Avx512f"; }
};
#endif
#endif

#if defined(CORRADE_TARGET_ARM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief NEON tag type

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Cpu namespace
and the @ref Neon tag for more information.
*/
struct NeonT: ScalarT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit NeonT(Implementation::InitT): ScalarT{Implementation::Init} {}
    #endif
};

/**
@brief NEON FMA tag type

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Cpu namespace
and the @ref NeonFma tag for more information.
*/
struct NeonFmaT: NeonT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit NeonFmaT(Implementation::InitT): NeonT{Implementation::Init} {}
    #endif
};

/**
@brief NEON FP16 tag type

Available only on @ref CORRADE_TARGET_ARM "ARM". See the @ref Cpu namespace
and the @ref NeonFp16 tag for more information.
*/
struct NeonFp16T: NeonFmaT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit NeonFp16T(Implementation::InitT): NeonFmaT{Implementation::Init} {}
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<NeonT> {
    enum: unsigned int { Index = 1 << 0 };
    static const char* name() { return "Neon"; }
};
template<> struct TypeTraits<NeonFmaT> {
    enum: unsigned int { Index = 1 << 1 };
    static const char* name() { return "NeonFma"; }
};
template<> struct TypeTraits<NeonFp16T> {
    enum: unsigned int { Index = 1 << 2 };
    static const char* name() { return "NeonFp16"; }
};
#endif
#endif

#if defined(CORRADE_TARGET_WASM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SIMD128 tag type

Available only on @ref CORRADE_TARGET_WASM "WebAssembly". See the @ref Cpu
namespace and the @ref Simd128 tag for more information.
*/
struct Simd128T: ScalarT {
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* Explicit constructor to avoid ambiguous calls when using {} */
    constexpr explicit Simd128T(Implementation::InitT): ScalarT{Implementation::Init} {}
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct TypeTraits<Simd128T> {
    enum: unsigned int { Index = 1 << 0 };
    static const char* name() { return "Simd128"; }
};
#endif
#endif

/**
@brief Scalar tag

Code that isn't explicitly optimized with any advanced CPU instruction set.
Fallback if no other CPU instruction set is chosen or available. The next most
widely supported instruction sets are @ref Sse2 on x86, @ref Neon on ARM and
@ref Simd128 on WebAssembly.
*/
constexpr ScalarT Scalar{Implementation::Init};

#if defined(CORRADE_TARGET_X86) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SSE2 tag

[Streaming SIMD Extensions 2](https://en.wikipedia.org/wiki/SSE2). Available
only on @ref CORRADE_TARGET_X86 "x86", supported by all 64-bit x86 processors
and is present on majority of contemporary 32-bit x86 processors as well.
Superset of @ref Scalar, implied by @ref Sse3.
@see @ref CORRADE_TARGET_SSE2
*/
constexpr Sse2T Sse2{Implementation::Init};

/**
@brief SSE3 tag

[Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSE3). Available
only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse2, implied by
@ref Ssse3.
@see @ref CORRADE_TARGET_SSE3
*/
constexpr Sse3T Sse3{Implementation::Init};

/**
@brief SSSE3 tag

[Supplemental Streaming SIMD Extensions 3](https://en.wikipedia.org/wiki/SSSE3).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse3, implied
by @ref Sse41.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref Sse41 to handle those.
@see @ref CORRADE_TARGET_SSSE3
*/
constexpr Ssse3T Ssse3{Implementation::Init};

/**
@brief SSE4.1 tag

[Streaming SIMD Extensions 4.1](https://en.wikipedia.org/wiki/SSE4#SSE4.1).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Ssse3,
implied by @ref Sse42.

Note that certain older AMD processors have [SSE4a](https://en.wikipedia.org/wiki/SSE4#SSE4a)
but neither SSSE3 nor SSE4.1. Both can be however treated as a subset of SSE4.1
to a large extent, and it's recommended to use @ref Sse41 to handle those.
@see @ref CORRADE_TARGET_SSE41
*/
constexpr Sse41T Sse41{Implementation::Init};

/**
@brief SSE4.2 tag

[Streaming SIMD Extensions 4.2](https://en.wikipedia.org/wiki/SSE4#SSE4.2).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse41,
implied by @ref Avx.
@see @ref CORRADE_TARGET_SSE42
*/
constexpr Sse42T Sse42{Implementation::Init};

/**
@brief AVX tag

[Advanced Vector Extensions](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Sse42,
implied by @ref Avx2.
@see @ref CORRADE_TARGET_AVX
*/
constexpr AvxT Avx{Implementation::Init};

/**
@brief AVX2 tag

[Advanced Vector Extensions 2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2).
Available only on @ref CORRADE_TARGET_X86 "x86". Superset of @ref Avx,
implied by @ref Avx512f.
@see @ref CORRADE_TARGET_AVX2
*/
constexpr Avx2T Avx2{Implementation::Init};

/**
@brief AVX-512 Foundation tag

[AVX-512](https://en.wikipedia.org/wiki/AVX-512) Foundation. Available only on
@ref CORRADE_TARGET_X86 "x86". Superset of @ref Avx2.
@see @ref CORRADE_TARGET_AVX512F
*/
constexpr Avx512fT Avx512f{Implementation::Init};
#endif

#if defined(CORRADE_TARGET_ARM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief NEON tag type

[ARM NEON](https://en.wikipedia.org/wiki/ARM_architecture#Advanced_SIMD_(Neon)).
Available only on @ref CORRADE_TARGET_ARM "ARM". Superset of @ref Scalar,
implied by @ref NeonFp16.
@see @ref CORRADE_TARGET_NEON
*/
constexpr NeonT Neon{Implementation::Init};

/**
@brief NEON FMA tag type

[ARM NEON](https://en.wikipedia.org/wiki/ARM_architecture#Advanced_SIMD_(Neon))
with FMA instructions. Available only on @ref CORRADE_TARGET_ARM "ARM".
Superset of @ref Neon, implied by @ref NeonFp16.
@see @ref CORRADE_TARGET_NEON_FMA
*/
constexpr NeonFmaT NeonFma{Implementation::Init};

/**
@brief NEON FP16 tag type

[ARM NEON](https://en.wikipedia.org/wiki/ARM_architecture#Advanced_SIMD_(Neon))
with ARMv8.2-a FP16 vector arithmetic. Available only on
@ref CORRADE_TARGET_ARM "ARM". Superset of @ref NeonFma.
@see @ref CORRADE_TARGET_NEON_FP16
*/
constexpr NeonFp16T NeonFp16{Implementation::Init};
#endif

#if defined(CORRADE_TARGET_WASM) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief SIMD128 tag type

[128-bit WebAssembly SIMD](https://github.com/webassembly/simd). Available only
on @ref CORRADE_TARGET_WASM "WebAssembly". Superset of @ref Scalar.
@see @ref CORRADE_TARGET_SIMD128
*/
constexpr Simd128T Simd128{Implementation::Init};
#endif

/**
@brief Default base tag type

See the @ref DefaultBase tag for more information.
*/
typedef
    #ifdef CORRADE_TARGET_X86
    #ifdef CORRADE_TARGET_AVX512F
    Avx512fT
    #elif defined(CORRADE_TARGET_AVX2)
    Avx2T
    #elif defined(CORRADE_TARGET_AVX)
    AvxT
    #elif defined(CORRADE_TARGET_SSE42)
    Sse42T
    #elif defined(CORRADE_TARGET_SSE41)
    Sse41T
    #elif defined(CORRADE_TARGET_SSSE3)
    Ssse3T
    #elif defined(CORRADE_TARGET_SSE3)
    Sse3T
    #elif defined(CORRADE_TARGET_SSE2)
    Sse2T
    #else
    ScalarT
    #endif

    #elif defined(CORRADE_TARGET_ARM)
    #ifdef CORRADE_TARGET_NEON_FP16
    NeonFp16T
    #elif defined(CORRADE_TARGET_NEON_FMA)
    NeonFmaT
    #elif defined(CORRADE_TARGET_NEON)
    NeonT
    #else
    ScalarT
    #endif

    #elif defined(CORRADE_TARGET_WASM)
    #ifdef CORRADE_TARGET_SIMD128
    Simd128T
    #else
    ScalarT
    #endif
    #endif
    DefaultBaseT;

/**
@brief Default base tag

Highest base instruction set available on given architecture with current
compiler flags. Ordered by priority, on @ref CORRADE_TARGET_X86 it's one of these:

-   @ref Avx512f if @ref CORRADE_TARGET_AVX512F is defined
-   @ref Avx2 if @ref CORRADE_TARGET_AVX2 is defined
-   @ref Avx if @ref CORRADE_TARGET_AVX is defined
-   @ref Sse42 if @ref CORRADE_TARGET_SSE42 is defined
-   @ref Sse41 if @ref CORRADE_TARGET_SSE41 is defined
-   @ref Ssse3 if @ref CORRADE_TARGET_SSSE3 is defined
-   @ref Sse3 if @ref CORRADE_TARGET_SSE3 is defined
-   @ref Sse2 if @ref CORRADE_TARGET_SSE2 is defined
-   @ref Scalar otherwise

On @ref CORRADE_TARGET_ARM it's one of these:

-   @ref NeonFp16 if @ref CORRADE_TARGET_NEON_FP16 is defined
-   @ref NeonFma if @ref CORRADE_TARGET_NEON_FMA is defined
-   @ref Neon if @ref CORRADE_TARGET_NEON is defined
-   @ref Scalar otherwise

On @ref CORRADE_TARGET_WASM it's one of these:

-   @ref Simd128 if @ref CORRADE_TARGET_SIMD128 is defined
-   @ref Scalar otherwise

See also @ref compiledFeatures(), which returns a *combination* of these tags
instead of just the highest available, and @ref runtimeFeatures() which is
capable of detecting the available CPU feature set at runtime.
*/
constexpr DefaultBaseT DefaultBase{Implementation::Init};

/**
@brief Feature set

Provides storage and comparison as well as runtime detection of CPU instruction
set. Provides an interface similar to an @ref Containers::EnumSet, with values
being the @ref Sse2, @ref Sse3 etc. tags.

See the @ref Cpu namespace for an overview and usage examples.
@see @ref compiledFeatures(), @ref runtimeFeatures()
*/
class Features {
    public:
        /**
         * @brief Default constructor
         *
         * Equivalent to @ref Scalar.
         */
        constexpr explicit Features() noexcept: _data{} {}

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
            return Features{_data | other._data};
        }

        /** @brief Union two feature sets and assign */
        Features& operator|=(Features other) {
            _data |= other._data;
            return *this;
        }

        /** @brief Intersection of two feature sets */
        constexpr Features operator&(Features other) const {
            return Features{_data & other._data};
        }

        /** @brief Intersect two feature sets and assign */
        Features& operator&=(Features other) {
            _data &= other._data;
            return *this;
        }

        /** @brief XOR of two feature sets */
        constexpr Features operator^(Features other) const {
            return Features{_data ^ other._data};
        }

        /** @brief XOR two feature sets and assign */
        Features& operator^=(Features other) {
            _data ^= other._data;
            return *this;
        }

        /** @brief Feature set complement */
        constexpr Features operator~() const {
            return Features{~_data};
        }

        /**
         * @brief Boolean conversion
         *
         * Returns @cpp true @ce if at least one feature apart from @ref Scalar
         * is present, @cpp false @ce otherwise.
         */
        constexpr explicit operator bool() const { return _data; }

        /**
         * @brief Integer representation
         *
         * For testing purposes. @ref Cpu::Scalar is always @cpp 0 @ce, values
         * corresponding to other feature tags are unspecified.
         */
        constexpr explicit operator unsigned int() const { return _data; }

    private:
        friend constexpr Features compiledFeatures();
        #if defined(CORRADE_TARGET_X86) && (defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_GCC))
        /* MSVC demands the export macro to be here as well */
        friend CORRADE_UTILITY_EXPORT Features runtimeFeatures();
        #endif

        constexpr explicit Features(unsigned int data) noexcept: _data{data} {}

        unsigned int _data;
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

/** @debugoperator{Features} */
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, Features value);

/** @relates Features
@overload
*/
template<class T, class = decltype(TypeTraits<T>::Index)> inline Utility::Debug& operator<<(Utility::Debug& debug, T value) {
    return operator<<(debug, Features{value});
}

/**
@brief CPU instruction sets enabled at compile time

On @ref CORRADE_TARGET_X86 "x86" returns a combination of @ref Sse2, @ref Sse3,
@ref Ssse3, @ref Sse41, @ref Sse42, @ref Avx, @ref Avx2 and @ref Avx512f based
on what all @ref CORRADE_TARGET_SSE2 etc. preprocessor variables are defined.

On @ref CORRADE_TARGET_ARM "ARM", returns a combination of @ref Neon,
@ref NeonFma and @ref NeonFp16 based on what all @ref CORRADE_TARGET_NEON etc.
preprocessor variables are defined.

On @ref CORRADE_TARGET_WASM "WebAssembly", returns @ref Simd128 based on
whether the @ref CORRADE_TARGET_SIMD128 preprocessor variable is defined.

On other platforms or if no known CPU instruction set is enabled, the returned
value is equal to @ref Scalar, which in turn is equivalent to empty (or
default-constructed) @ref Features.
@see @ref DefaultBase
*/
constexpr Features compiledFeatures() {
    return Features{
        #ifdef CORRADE_TARGET_X86
        #ifdef CORRADE_TARGET_SSE2
        TypeTraits<Sse2T>::Index|
        #endif
        #ifdef CORRADE_TARGET_SSE3
        TypeTraits<Sse3T>::Index|
        #endif
        #ifdef CORRADE_TARGET_SSSE3
        TypeTraits<Ssse3T>::Index|
        #endif
        #ifdef CORRADE_TARGET_SSE41
        TypeTraits<Sse41T>::Index|
        #endif
        #ifdef CORRADE_TARGET_SSE42
        TypeTraits<Sse42T>::Index|
        #endif
        #ifdef CORRADE_TARGET_AVX
        TypeTraits<AvxT>::Index|
        #endif
        #ifdef CORRADE_TARGET_AVX2
        TypeTraits<Avx2T>::Index|
        #endif

        #elif defined(CORRADE_TARGET_ARM)
        #ifdef CORRADE_TARGET_NEON
        TypeTraits<NeonT>::Index|
        #endif
        #ifdef CORRADE_TARGET_NEON_FMA
        TypeTraits<NeonFmaT>::Index|
        #endif
        #ifdef CORRADE_TARGET_NEON_FP16
        TypeTraits<NeonFp16T>::Index|
        #endif

        #elif defined(CORRADE_TARGET_WASM)
        #ifdef CORRADE_TARGET_SIMD128
        TypeTraits<Simd128T>::Index|
        #endif
        #endif
        0};
}

/**
@brief Detect available CPU instruction sets at runtime

On @ref CORRADE_TARGET_X86 "x86" and GCC, Clang or MSVC uses the
[CPUID](https://en.wikipedia.org/wiki/CPUID) builtin to check for the
@ref Sse2, @ref Sse3, @ref Ssse3, @ref Sse41, @ref Sse42, @ref Avx, @ref Avx2
and @ref Avx512f runtime features. @ref Avx needs OS support as well, if it's
not present, no following flags are checked either. On compilers other than
GCC, Clang and MSVC the function is @cpp constexpr @ce and delegates into
@ref compiledFeatures().

On @ref CORRADE_TARGET_ARM "ARM", no runtime detection is implemented at the
moment. The function is @cpp constexpr @ce and delegates into
@ref compiledFeatures().

On @ref CORRADE_TARGET_WASM "WebAssembly" an attempt to use SIMD instructions
without runtime support results in a WebAssembly compilation error and thus
runtime detection is largely meaningless. While this may change once the
[feature detection proposal](https://github.com/WebAssembly/feature-detection/blob/main/proposals/feature-detection/Overview.md)
is implemented, at the moment the function is @cpp constexpr @ce and delegates
into @ref compiledFeatures().

On other platforms or if no known CPU instruction set is detected, the returned
value is equal to @ref Scalar, which in turn is equivalent to empty (or
default-constructed) @ref Features.

@see @ref DefaultBase
*/
#if (defined(CORRADE_TARGET_X86) && (defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_GCC))) || defined(DOXYGEN_GENERATING_OUTPUT)
CORRADE_UTILITY_EXPORT Features runtimeFeatures();
#else
constexpr Features runtimeFeatures() { return compiledFeatures(); }
#endif

}

}

#endif