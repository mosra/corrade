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

#include "Corrade/Simd.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"

#ifdef CORRADE_TARGET_X86
#ifdef CORRADE_TARGET_MSVC
#include <intrin.h>
#include <immintrin.h>
#elif defined(CORRADE_TARGET_GCC)
#include <cpuid.h>
/* _xgetbv() defined since GCC 8.1, in <xsaveintrin.h>, but I can't include
   that one alone, have to include <immintrin.h> instead:
    https://github.com/gcc-mirror/gcc/commit/b9bdd60b8792e2d3173ecfacd5c25aac894a94e5
   On Clang it got first defined in 3.5:
    https://github.com/llvm/llvm-project/commit/854f7d34ec3d7c0559ffde536cfa88fad5419bf0
   but that's (look again) in Intrin.h (yes, uppercase), which got fixed to
   be intrin.h in 3.9, however at least on Xcode 9.4 intrin.h has
   `#include_next <intrin.h>` and there *just isn't* any other intrin.h so
   it fails; then, looking at the commit history, the function changed
   locations a few times, mostly as an attempt to fix compilation of V8 (?!?!)
   and ... I have no patience for this anymore, so for Clang I'm using the same
   inline assembly as for GCC 8.0 and earlier. */
#if defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 801
#include <immintrin.h>
#endif
#endif
#endif

namespace Corrade { namespace Simd {

using namespace Containers::Literals;

/* As the types all inherit from each other, there should be no members to
   keep them zero-cost. */
static_assert(sizeof(Simd::Scalar) == 1, "");
#ifdef CORRADE_TARGET_X86
static_assert(sizeof(Simd::Sse2) == 1, "");
static_assert(sizeof(Simd::Sse3) == 1, "");
static_assert(sizeof(Simd::Ssse3) == 1, "");
static_assert(sizeof(Simd::Sse41) == 1, "");
static_assert(sizeof(Simd::Sse42) == 1, "");
static_assert(sizeof(Simd::Avx) == 1, "");
static_assert(sizeof(Simd::AvxF16c) == 1, "");
static_assert(sizeof(Simd::AvxFma) == 1, "");
static_assert(sizeof(Simd::Avx2) == 1, "");
static_assert(sizeof(Simd::Avx512f) == 1, "");
#elif defined(CORRADE_TARGET_ARM)
static_assert(sizeof(Simd::Neon) == 1, "");
static_assert(sizeof(Simd::NeonFp16) == 1, "");
static_assert(sizeof(Simd::NeonFma) == 1, "");
#elif defined(CORRADE_TARGET_WASM)
static_assert(sizeof(Simd::Simd128) == 1, "");
#endif

Features::Features(): _data{} {
    /* Use cpuid on x86 and GCC/Clang/MSVC */
    #if defined(CORRADE_TARGET_X86) && (defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_GCC))
    union {
        struct {
            std::uint32_t ax, bx, cx, dx;
        } e;
        #ifdef CORRADE_TARGET_MSVC
        /* MSVC's __cpuid() macro accepts an array instead of four pointers and
           wants an int, not uint, for some reason */
        std::int32_t data[4];
        #endif
    } cpuid{};

    /* https://en.wikipedia.org/wiki/CPUID#EAX=1:_Processor_Info_and_Feature_Bits */
    #ifdef CORRADE_TARGET_MSVC
    __cpuid(cpuid.data, 1);
    #elif defined(CORRADE_TARGET_GCC)
    CORRADE_INTERNAL_ASSERT_OUTPUT(__get_cpuid(1, &cpuid.e.ax, &cpuid.e.bx, &cpuid.e.cx, &cpuid.e.dx));
    #endif
    if(cpuid.e.dx & (1 << 26)) _data |= TypeTraits<Sse2T>::Index;
    if(cpuid.e.cx & (1 <<  0)) _data |= TypeTraits<Sse3T>::Index;
    if(cpuid.e.cx & (1 <<  9)) _data |= TypeTraits<Ssse3T>::Index;
    if(cpuid.e.cx & (1 << 19)) _data |= TypeTraits<Sse41T>::Index;
    if(cpuid.e.cx & (1 << 20)) _data |= TypeTraits<Sse42T>::Index;

    /* AVX needs OS support checked, as the OS needs to be capable of saving
       and restoring the expanded registers when switching contexts:
       https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Operating_system_support */
    if((cpuid.e.cx & (1 << 27)) && /* XSAVE/XRESTORE CPU support */
       (cpuid.e.cx & (1 << 28)))   /* AVX CPU support */
    {
        /* XGETBV indicates that the registers will be properly saved and
           restored by the OS: https://stackoverflow.com/a/22521619. The
           _xgetbv() function is available on MSVC and since GCC 8.1 and
           probably on Clang also but I don't care -- see the comment at the
           #include above. */
        #if defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 801)
        const bool available = (_xgetbv(0) & 0x6) == 0x6;
        #else
        /* https://github.com/vectorclass/version2/blob/ff7450acfad9d3a7c6825d92cfb782a42ccfa71f/instrset_detect.cpp#L30-L32 */
        std::uint32_t a, d;
        __asm("xgetbv" : "=a"(a),"=d"(d) : "c"(0) : );
        const bool available = ((a|(std::uint64_t(d) << 32)) & 0x6) == 0x6;
        #endif

        if(available) _data |= TypeTraits<AvxT>::Index;
    }

    /* If AVX is not supported, we don't check any following flags either */
    if(!(_data & TypeTraits<AvxT>::Index)) return;

    if(cpuid.e.cx & (1 << 29)) _data |= TypeTraits<AvxF16cT>::Index;
    if(cpuid.e.cx & (1 << 12)) _data |= TypeTraits<AvxFmaT>::Index;

    /* https://en.wikipedia.org/wiki/CPUID#EAX=7,_ECX=0:_Extended_Features */
    #ifdef CORRADE_TARGET_MSVC
    __cpuidex(cpuid.data, 7, 0);
    /* __get_cpuid_count() is only since GCC 7.1 and Clang 5.0:
        https://github.com/gcc-mirror/gcc/commit/2488ebe5ef1788616c2fbc61e05af09f0749ebbe
        https://github.com/llvm/llvm-project/commit/f6e8408a116405d0cc25b506e8c5b60ab4ab7bcd
       Furthermore, Clang 5.0 is 9.3 on Apple, 9.2 has only Clang 4:
        https://en.wikipedia.org/wiki/Xcode#Toolchain_versions */
    #elif (defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 701) || (defined(CORRADE_TARGET_CLANG) && ((!defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__ >= 5) || (defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__*100 + __clang_minor__ >= 903)))
    CORRADE_INTERNAL_ASSERT_OUTPUT(__get_cpuid_count(7, 0, &cpuid.e.ax, &cpuid.e.bx, &cpuid.e.cx, &cpuid.e.dx));
    /* Looking at the above commits, on older GCC, __get_cpuid(7) seems to do
       what __get_cpuid_count(7, 0) does on new GCC, however that's only since
       6.3:
        https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77756
       Older Clang doesn't do any of that, and thus to avoid a combinatorial
       explosion of test scenarios, we'll just directly use the __cpuid
       assembly on both, which seems to be there since forever.
       __get_cpuid[_count]() only does a bunch of bounds checking on top of
       that anyway and if we didn't blow up on the assert above I HOPE we
       should be fine to check for extended features level zero. For higher
       levels (to check very recent AVX512 features) probably not but I assume
       those would need very recent compilers anyway so when that happens we
       can just pretend those don't exist on GCC 7.0 and older. */
    #else
    __cpuid_count(7, 0, cpuid.e.ax, cpuid.e.bx, cpuid.e.cx, cpuid.e.dx);
    #endif
    if(cpuid.e.bx & (1 << 5)) _data |= TypeTraits<Avx2T>::Index;
    if(cpuid.e.bx & (1 << 16)) _data |= TypeTraits<Avx512fT>::Index;

    /* Fall back to compile-time-defined features otherwise. On x86 this is
       very prone to code rot, please try to keep it up-to-date PLEASE. */
    #elif defined(CORRADE_TARGET_X86)
    #ifdef CORRADE_TARGET_SSE2
    _data |= TypeTraits<Sse2T>::Index;
    #endif
    #ifdef CORRADE_TARGET_SSE3
    _data |= TypeTraits<Sse3T>::Index;
    #endif
    #ifdef CORRADE_TARGET_SSSE3
    _data |= TypeTraits<Ssse3T>::Index;
    #endif
    #ifdef CORRADE_TARGET_SSE41
    _data |= TypeTraits<Sse41T>::Index;
    #endif
    #ifdef CORRADE_TARGET_SSE42
    _data |= TypeTraits<Sse42T>::Index;
    #endif
    #ifdef CORRADE_TARGET_AVX
    _data |= TypeTraits<AvxT>::Index;
    #endif
    #ifdef CORRADE_TARGET_AVX2
    _data |= TypeTraits<Avx2T>::Index;
    #endif

    #elif defined(CORRADE_TARGET_ARM)
    #ifdef CORRADE_TARGET_NEON
    _data |= TypeTraits<NeonT>::Index;
    #endif
    #ifdef CORRADE_TARGET_NEON_FP16
    _data |= TypeTraits<NeonFp16T>::Index;
    #endif
    #ifdef CORRADE_TARGET_NEON_FMA
    _data |= TypeTraits<NeonFmaT>::Index;
    #endif

    #elif defined(CORRADE_TARGET_WASM)
    #ifdef CORRADE_TARGET_SIMD128
    _data |= TypeTraits<Simd128T>::Index;
    #endif
    #endif
}

Utility::Debug& operator<<(Utility::Debug& debug, const Features value) {
    const Containers::StringView prefix = "|Simd::"_s.except(debug.immediateFlags() & Utility::Debug::Flag::Packed ? 6 : 0);

    /* First one without the | */
    debug << prefix.suffix(1) << Utility::Debug::nospace;
    if(!value) return debug << "Scalar"_s;

    bool written = false;
    #define _c(tag)                                                         \
        if(value & tag) {                                                   \
            if(!written) written = true;                                    \
            else debug << Utility::Debug::nospace << prefix << Utility::Debug::nospace; \
            debug << TypeTraits<tag ## T>::name();                          \
        }
    #ifdef CORRADE_TARGET_X86
    _c(Sse2)
    _c(Sse3)
    _c(Ssse3)
    _c(Sse3)
    _c(Sse41)
    _c(Sse42)
    _c(Avx)
    _c(AvxF16c)
    _c(AvxFma)
    _c(Avx2)
    _c(Avx512f)
    #elif defined(CORRADE_TARGET_ARM)
    _c(Neon)
    _c(NeonFp16)
    _c(NeonFma)
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    _c(Simd128)
    #endif
    #undef _c

    return debug;
}

}}
