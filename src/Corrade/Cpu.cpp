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

#ifndef CORRADE_NO_DEBUG
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"
#endif

/** @todo these are indented to work around acme.py extracting them to the top,
    fix properly */
/* getauxval() for ARM on Linux and Android with API level 18+ */
#if defined(CORRADE_TARGET_ARM) && defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)
    #include <sys/auxv.h>
/* sysctlbyname() for ARM on macOS / iOS */
#elif defined(CORRADE_TARGET_ARM) && defined(CORRADE_TARGET_APPLE)
    #include <sys/sysctl.h>
#endif

namespace Corrade { namespace Cpu {

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

#if defined(CORRADE_TARGET_ARM) && ((defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)) || defined(CORRADE_TARGET_APPLE))
Features runtimeFeatures() {
    /* Use getauxval() on ARM on Linux and Android */
    #if defined(CORRADE_TARGET_ARM) && defined(__linux__) && !(defined(CORRADE_TARGET_ANDROID) && __ANDROID_API__ < 18)
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

    /* No other (deinlined) implementation at the moment. The function should
       not be even defined here in that case -- it's inlined in the header
       instead, including the x86 implementation. */
    #else
    #error
    #endif
}
#endif

#ifndef CORRADE_NO_DEBUG
Utility::Debug& operator<<(Utility::Debug& debug, Features value) {
    using namespace Containers::Literals;

    const bool packed = debug.immediateFlags() >= Utility::Debug::Flag::Packed;

    const Containers::StringView prefix = "|Cpu::"_s.exceptSuffix(packed ? 5 : 0);

    /* First one without the | */
    debug << prefix.exceptPrefix(1) << Utility::Debug::nospace;
    if(!value) return debug << "Scalar"_s;

    bool written = false;
    #define _c(tag)                                                         \
        if(value & tag) {                                                   \
            if(!written) written = true;                                    \
            else debug << Utility::Debug::nospace << prefix << Utility::Debug::nospace; \
            debug << TypeTraits<tag ## T>::name();                          \
            value &= ~tag;                                                  \
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
    _c(Bmi1)
    _c(AvxF16c)
    _c(AvxFma)
    #elif defined(CORRADE_TARGET_ARM)
    _c(Neon)
    _c(NeonFma)
    _c(NeonFp16)
    #elif defined(CORRADE_TARGET_WASM)
    _c(Simd128)
    #endif
    #undef _c

    if(value) {
        if(written) debug << Utility::Debug::nospace << prefix << Utility::Debug::nospace;
        debug << (packed ? "" : "Features(") << Utility::Debug::nospace << reinterpret_cast<void*>(static_cast<unsigned int>(value)) << Utility::Debug::nospace << (packed ? "" : ")");
    }

    return debug;
}
#endif

}}
