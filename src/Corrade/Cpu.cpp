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

#include "Cpu.h"

#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/DebugAssert.h"

#ifdef CORRADE_TARGET_X86
#include <cstdint>
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

/* ARM on Linux and Android with API level 18+ */
#elif defined(CORRADE_TARGET_ARM) && defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)
#include <asm/hwcap.h>
#include <sys/auxv.h>

/* ARM on macOS / iOS */
#elif defined(CORRADE_TARGET_ARM) && defined(CORRADE_TARGET_APPLE)
#include <sys/sysctl.h>
#endif

namespace Corrade { namespace Cpu {

using namespace Containers::Literals;

/* As the types all inherit from each other, there should be no members to
   keep them zero-cost. */
static_assert(sizeof(Cpu::Scalar) == 1, "");
#ifdef CORRADE_TARGET_X86
static_assert(sizeof(Cpu::Sse2) == 1, "");
static_assert(sizeof(Cpu::Sse3) == 1, "");
static_assert(sizeof(Cpu::Ssse3) == 1, "");
static_assert(sizeof(Cpu::Sse41) == 1, "");
static_assert(sizeof(Cpu::Sse42) == 1, "");
static_assert(sizeof(Cpu::Avx) == 1, "");
static_assert(sizeof(Cpu::AvxF16c) == 1, "");
static_assert(sizeof(Cpu::AvxFma) == 1, "");
static_assert(sizeof(Cpu::Avx2) == 1, "");
static_assert(sizeof(Cpu::Avx512f) == 1, "");
#elif defined(CORRADE_TARGET_ARM)
static_assert(sizeof(Cpu::Neon) == 1, "");
static_assert(sizeof(Cpu::NeonFma) == 1, "");
static_assert(sizeof(Cpu::NeonFp16) == 1, "");
#elif defined(CORRADE_TARGET_WASM)
static_assert(sizeof(Cpu::Simd128) == 1, "");
#endif

#if defined(CORRADE_TARGET_ARM) && defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)
namespace Implementation {

/* As getauxval() can't be called from within an ifunc resolver because there
   it's too early for an external call, the value of AT_HWCAP is instead passed
   to it from outside, on glibc 2.13+ and on Android API 30+:
    https://github.com/bminor/glibc/commit/7520ff8c744a704ca39741c165a2360d63a4f47a
    https://android.googlesource.com/platform/bionic/+/e949195f6489653ee3771535951ed06973246c3e/libc/include/sys/ifunc.h
   Which means we need a variant of runtimeFeatures() that is able to operate
   with a value fed from outside, which is then used inside such resolvers.
   For simplicity this variant is available always and the public
   Cpu::runtimeFeatures() just delegates to it. */
Features runtimeFeatures(const unsigned long caps) {
    unsigned int out = 0;
    #ifdef CORRADE_TARGET_32BIT
    if(caps & HWCAP_NEON) out |= TypeTraits<NeonT>::Index;
    /* Since FMA is enabled by passing -mfpu=neon-vfpv4, I assume this is the
       flag that corresponds to it. */
    if(caps & HWCAP_VFPv4) out |= TypeTraits<NeonFmaT>::Index;
    #else
    /* On ARM64 NEON and NEON FMA is implicit. For extra security make use of
       the CORRADE_TARGET_ defines (which should be always there). */
    out |=
        #ifdef CORRADE_TARGET_NEON
        TypeTraits<NeonT>::Index|
        #endif
        #ifdef CORRADE_TARGET_NEON_FMA
        TypeTraits<NeonFmaT>::Index|
        #endif
        0;
    /* The HWCAP flags are extremely cryptic. The only vague confirmation is in
       a *commit message* to the kernel hwcaps file, FFS. The HWCAP_FPHP seems
       to correspond to scalar FP16, so the other should be the vector one?
       https://github.com/torvalds/linux/blame/master/arch/arm64/include/uapi/asm/hwcap.h
       This one also isn't present on 32-bit, so I assume it's ARM64-only? */
    if(caps & HWCAP_ASIMDHP) out |= TypeTraits<NeonFp16T>::Index;
    #endif
    return Features{out};
}

}
#endif

/* Helper for getting macOS / iOS ARM properties. Yep, it's stringly typed. */
#if defined(CORRADE_TARGET_ARM) && defined(CORRADE_TARGET_APPLE)
namespace {

int appleSysctlByName(const char* name) {
  int value;
  std::size_t size = sizeof(value);
  /* First pointer/size pair is for querying the value, second is for setting
     the value. Returns 0 on success. */
  return sysctlbyname(name, &value, &size, nullptr, 0) ? 0 : value;
}

}
#endif

#if (defined(CORRADE_TARGET_X86) && (defined(CORRADE_TARGET_MSVC) || defined(CORRADE_TARGET_GCC))) || (defined(CORRADE_TARGET_ARM) && ((defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)) || defined(CORRADE_TARGET_APPLE)))
#if defined(CORRADE_TARGET_X86) && defined(CORRADE_TARGET_GCC)
/* GCC 10 complains otherwise, however 4.8 doesn't. Doing it via a function
   attribute instead of passing -mxsave as detecting target architecture in
   CMake is annoying at best. */
__attribute__((__target__("xsave")))
#endif
Features runtimeFeatures() {
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
    CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT(__get_cpuid(1, &cpuid.e.ax, &cpuid.e.bx, &cpuid.e.cx, &cpuid.e.dx));
    #endif

    unsigned int out = 0;
    if(cpuid.e.dx & (1 << 26)) out |= TypeTraits<Sse2T>::Index;
    if(cpuid.e.cx & (1 <<  0)) out |= TypeTraits<Sse3T>::Index;
    if(cpuid.e.cx & (1 <<  9)) out |= TypeTraits<Ssse3T>::Index;
    if(cpuid.e.cx & (1 << 19)) out |= TypeTraits<Sse41T>::Index;
    if(cpuid.e.cx & (1 << 20)) out |= TypeTraits<Sse42T>::Index;

    /* https://en.wikipedia.org/wiki/CPUID#EAX=80000001h:_Extended_Processor_Info_and_Feature_Bits,
       bit 5 says "ABM (lzcnt and popcnt)", but
       https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#ABM_(Advanced_Bit_Manipulation)
       says that while LZCNT is advertised in the ABM CPUID bit, POPCNT is a
       separate CPUID flag. Get POPCNT first, ABM later. */
    if(cpuid.e.cx & (1 << 23)) out |= TypeTraits<PopcntT>::Index;

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

        if(available) out |= TypeTraits<AvxT>::Index;
    }

    /* If AVX is not supported, we don't check any following flags either */
    if(out & TypeTraits<AvxT>::Index) {
        if(cpuid.e.cx & (1 << 29)) out |= TypeTraits<AvxF16cT>::Index;
        if(cpuid.e.cx & (1 << 12)) out |= TypeTraits<AvxFmaT>::Index;

        /* https://en.wikipedia.org/wiki/CPUID#EAX=7,_ECX=0:_Extended_Features */
        #ifdef CORRADE_TARGET_MSVC
        __cpuidex(cpuid.data, 7, 0);
        /* __get_cpuid_count() is only since GCC 7.1 and Clang 5.0:
            https://github.com/gcc-mirror/gcc/commit/2488ebe5ef1788616c2fbc61e05af09f0749ebbe
            https://github.com/llvm/llvm-project/commit/f6e8408a116405d0cc25b506e8c5b60ab4ab7bcd
           Furthermore, Clang 5.0 is 9.3 on Apple, 9.2 has only Clang 4:
            https://en.wikipedia.org/wiki/Xcode#Toolchain_versions */
        #elif (defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 701) || (defined(CORRADE_TARGET_CLANG) && ((!defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__ >= 5) || (defined(CORRADE_TARGET_APPLE_CLANG) && __clang_major__*100 + __clang_minor__ >= 903)))
        CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT(__get_cpuid_count(7, 0, &cpuid.e.ax, &cpuid.e.bx, &cpuid.e.cx, &cpuid.e.dx));
        /* Looking at the above commits, on older GCC, __get_cpuid(7) seems to
           do what __get_cpuid_count(7, 0) does on new GCC, however that's only
           since 6.3:
            https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77756
           Older Clang doesn't do any of that, and thus to avoid a
           combinatorial explosion of test scenarios, we'll just directly use
           the __cpuid assembly on both, which seems to be there since forever.
           __get_cpuid[_count]() only does a bunch of bounds checking on top of
           that anyway and if we didn't blow up on the assert above I HOPE we
           should be fine to check for extended features level zero. For higher
           levels (to check very recent AVX512 features) probably not but I
           assume those would need very recent compilers anyway so when that
           happens we can just pretend those don't exist on GCC 7.0 and
           older. */
        #else
        __cpuid_count(7, 0, cpuid.e.ax, cpuid.e.bx, cpuid.e.cx, cpuid.e.dx);
        #endif
        if(cpuid.e.bx & (1 << 5)) out |= TypeTraits<Avx2T>::Index;
        if(cpuid.e.bx & (1 << 16)) out |= TypeTraits<Avx512fT>::Index;
    }

    /* And now the LZCNT bit, finally
       https://en.wikipedia.org/wiki/CPUID#EAX=80000001h:_Extended_Processor_Info_and_Feature_Bits */
    #ifdef CORRADE_TARGET_MSVC
    __cpuid(cpuid.data, 0x80000001);
    #elif defined(CORRADE_TARGET_GCC)
    CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT(__get_cpuid(0x80000001, &cpuid.e.ax, &cpuid.e.bx, &cpuid.e.cx, &cpuid.e.dx));
    #endif
    if(cpuid.e.cx & (1 << 5)) out |= TypeTraits<LzcntT>::Index;

    return Features{out};

    /* Use getauxval() on ARM on Linux and Android */
    #elif defined(CORRADE_TARGET_ARM) && defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)
    /* People say getauxval() is "extremely slow":
        https://lemire.me/blog/2020/07/17/the-cost-of-runtime-dispatch/#comment-538459
       Like, can anything be worse than reading and parsing the text from
       /proc/cpuinfo? */
    return Implementation::runtimeFeatures(getauxval(AT_HWCAP));

    /* Use sysctlbyname() on ARM on macOS / iOS */
    #elif defined(CORRADE_TARGET_ARM) && defined(CORRADE_TARGET_APPLE)
    unsigned int out = 0;
    /* https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_instruction_set_characteristics,
       especially "funny" is how most of the values are getting rid of the NEON
       naming, probably because they want to push their proprietary AMX.
       Sigh. */
    #ifdef CORRADE_TARGET_32BIT
    /* Apple says I should use hw.optional.AdvSIMD instead tho */
    if(appleSysctlByName("hw.optional.neon")) out |= TypeTraits<NeonT>::Index;
    /* On 32bit I have no idea how to query FMA / vfpv4 support, so that'll
       only be implied if FP16 is available as well. Since I don't think there are many 32bit iOS devices left, that's not worth bothering with. */
    #else
    /* To avoid string operations, on 64bit I just assume NEON and FMA being
       present, like in the Linux case. Again, for extra security make use of
       the CORRADE_TARGET_ defines (which should be always there on ARM64) */
    out |=
        #ifdef CORRADE_TARGET_NEON
        TypeTraits<NeonT>::Index|
        #endif
        #ifdef CORRADE_TARGET_NEON_FMA
        TypeTraits<NeonFmaT>::Index|
        #endif
        0;
    #endif
    /* Apple says I should use hw.optional.arm.FEAT_FP16 instead tho */
    if(appleSysctlByName("hw.optional.neon_fp16")) {
        /* As noted above, if FP16 is available on 32bit, bite the bullet and
           assume FMA is there as well */
        #ifdef CORRADE_TARGET_32BIT
        out |= TypeTraits<NeonFmaT>::Index;
        #endif
        out |= TypeTraits<NeonFp16T>::Index;
    }

    return Features{out};

    /* No other implementation at the moment. The function should not be even
       defined here in that case -- it's inlined in the header instead. */
    #else
    #error
    #endif
}
#endif

Utility::Debug& operator<<(Utility::Debug& debug, const Features value) {
    const Containers::StringView prefix = "|Cpu::"_s.exceptSuffix(debug.immediateFlags() & Utility::Debug::Flag::Packed ? 5 : 0);

    /* First one without the | */
    debug << prefix.exceptPrefix(1) << Utility::Debug::nospace;
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
    _c(Sse41)
    _c(Sse42)
    _c(Avx)
    _c(Avx2)
    _c(Avx512f)
    /* Print the extras at the end so the base instruction set is always first
       even in case of Cpu::Default, where it's just one */
    _c(Popcnt)
    _c(Lzcnt)
    _c(AvxF16c)
    _c(AvxFma)
    #elif defined(CORRADE_TARGET_ARM)
    _c(Neon)
    _c(NeonFma)
    _c(NeonFp16)
    #elif defined(CORRADE_TARGET_EMSCRIPTEN)
    _c(Simd128)
    #endif
    #undef _c

    return debug;
}

}}
