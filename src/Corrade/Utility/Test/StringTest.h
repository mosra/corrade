#ifndef Corrade_Utility_Test_StringTest_h
#define Corrade_Utility_Test_StringTest_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
/* Contains additional variants of Utility::String algorithms that are included
   just for historical / testing / benchmark comparison purposes; referenced
   explicitly via function pointers from the respective test case and benchmark
   instances. Used by StringTest.cpp and StringBenchmark.cpp. */

#include <cstddef>

#include "Corrade/Cpu.h"
#ifdef CORRADE_ENABLE_AVX2
#include "Corrade/Utility/IntrinsicsAvx.h"
#elif defined(CORRADE_ENABLE_SSE41)
#include "Corrade/Utility/IntrinsicsSse4.h"
#elif defined(CORRADE_ENABLE_SSE2)
#include "Corrade/Utility/IntrinsicsSse2.h"
#endif
#include "Corrade/Utility/String.h" /* replaceAllInPlaceCharacterImplementation() */

#ifdef CORRADE_ENABLE_NEON
#include <arm_neon.h>
#endif
#ifdef CORRADE_ENABLE_SIMD128
#include <wasm_simd128.h>
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

#ifdef CORRADE_ENABLE_SSE2
/* An "obvious" variant of the actual SSE2 implementation in
   String::lowercaseInPlace(). It's the same count of instructions but runs
   considerably slower for some reason -- maybe because the two compares, or
   all the bit ops can't be pipelined, compared to bit ops + arithmetic + one
   compare in the other? I know too little to be sure what's going on so this
   just records the state. */
CORRADE_ENABLE_SSE2 CORRADE_NEVER_INLINE void lowercaseInPlaceImplementationSse2TwoCompares(char* data, std::size_t size) {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    {
        char* j = data;
        switch(size) {
            case 15: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 14: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 13: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 12: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 11: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 10: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  9: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  8: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  7: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  6: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  5: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  4: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  3: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  2: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  1: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    /* Core algorithm */
    const __m128i a = _mm_set1_epi8('A');
    const __m128i z = _mm_set1_epi8('Z');
    const __m128i lowercaseBit = _mm_set1_epi8(0x20);
    const auto lowercaseOneVector = [&](const __m128i chars) CORRADE_ENABLE_SSE2 {
        /* Mark all bytes that aren't A-Z */
        const __m128i notUppercase = _mm_or_si128(_mm_cmpgt_epi8(a, chars),
                                                  _mm_cmpgt_epi8(chars, z));
        /* Inverse the mask, thus only bytes that are A-Z, and for them OR the
           lowercase bit with the input */
        return _mm_or_si128(_mm_andnot_si128(notUppercase, lowercaseBit), chars);
    };

    /* Unconditionally convert the first vector in a slower, unaligned way. Any
       extra branching to avoid the unaligned load & store if already aligned
       would be most probably more expensive than the actual operation. */
    {
        const __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(data), lowercaseOneVector(chars));
    }

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, lowercasing
       already-lowercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Convert all aligned vectors using aligned load/store */
    for(; i + 16 <= end; i += 16) {
        const __m128i chars = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        _mm_store_si128(reinterpret_cast<__m128i*>(i), lowercaseOneVector(chars));
    }

    /* Handle remaining less than a vector with an unaligned load & store,
       again overlapping back with the previous already-converted elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(i), lowercaseOneVector(chars));
    }
}
#endif

#ifdef CORRADE_ENABLE_NEON
/* Trivial port of the SSE2 code to NEON, with the same "aligned load/store is
   the same as unaligned" simplification as the WASM code. Included just to
   have baseline comparison to the scalar code because the compiler seems to
   autovectorize better than what this function does. */
CORRADE_ENABLE_NEON CORRADE_NEVER_INLINE void lowercaseInPlaceImplementationNeon(char* data, std::size_t size) {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    {
        char* j = data;
        switch(size) {
            case 15: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 14: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 13: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 12: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 11: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 10: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  9: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  8: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  7: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  6: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  5: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  4: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  3: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  2: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  1: *j += (std::uint8_t(*j - 'A') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    /* Core algorithm */
    const uint8x16_t aAndAbove = vdupq_n_u8(char(256u - std::uint8_t('A')));
    const uint8x16_t lowest25 = vdupq_n_u8(25);
    const uint8x16_t lowercaseBit = vdupq_n_u8(0x20);
    const uint8x16_t zero = vdupq_n_u8(0);
    const auto lowercaseOneVectorInPlace = [&](std::uint8_t* const data) CORRADE_ENABLE_NEON {
        const uint8x16_t chars = vld1q_u8(data);
        /* Moves 'A' and everything above to 0 and up (it overflows and wraps
           around) */
        const uint8x16_t uppercaseInLowest25 = vaddq_u8(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'A' to 'Z'
           (now 0 to 25) zero and everything else non-zero */
        const uint8x16_t lowest25IsZero = vqsubq_u8(uppercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const uint8x16_t maskUppercase = vceqq_u8(lowest25IsZero, zero);
        /* For the masked chars a lowercase bit is set, and the bit is then
           added to the original chars, making the uppercase chars lowercase */
        vst1q_u8(data, vaddq_u8(chars, vandq_u8(maskUppercase, lowercaseBit)));
    };

    /* Unconditionally convert the first unaligned vector */
    lowercaseOneVectorInPlace(reinterpret_cast<std::uint8_t*>(data));

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, lowercasing
       already-lowercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Convert all aligned vectors */
    for(; i + 16 <= end; i += 16)
        lowercaseOneVectorInPlace(reinterpret_cast<std::uint8_t*>(i));

    /* Handle remaining less than a vector, again overlapping back with the
       previous already-converted elements, in an unaligned way */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        lowercaseOneVectorInPlace(reinterpret_cast<std::uint8_t*>(i));
    }
}
#endif

/* Original variant that did the replacement unconditionally. Was faster than
   branching on every vector, but branching on every four vectors like find()
   does has comparable speed for common characters, and is significantly faster
   for rare characters. */
#ifdef CORRADE_ENABLE_SSE41
CORRADE_ENABLE_SSE41 CORRADE_NEVER_INLINE void replaceAllInPlaceCharacterImplementationSse41Unconditional(char* const data, const std::size_t size, const char search, const char replace) {
    /* If we have less than 16 bytes, do it the stupid way */
    {
        char* j = data;
        switch(size) {
            case 15: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 14: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 13: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 12: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 11: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 10: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  9: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  8: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  7: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  6: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  5: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  4: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  3: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  2: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  1: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    const __m128i vsearch = _mm_set1_epi8(search);
    const __m128i vreplace = _mm_set1_epi8(replace);

    /* Calculate the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll process some bytes twice. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally process the first vector a slower, unaligned way */
    {
        const __m128i in = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(data), out);
    }

    /* Process all aligned vectors. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. */
    char* const end = data + size;
    for(; i + 16 <= end; i += 16) {
        const __m128i in = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_store_si128(reinterpret_cast<__m128i*>(i), out);
    }

    /* Handle remaining less than a vector in an unaligned way. Overlapping
       bytes are again no-op. */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const __m128i in = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(i), out);
    }
}
#endif

#ifdef CORRADE_ENABLE_AVX2
CORRADE_ENABLE_AVX2 CORRADE_NEVER_INLINE void replaceAllInPlaceCharacterImplementationAvx2Unconditional(char* const data, const std::size_t size, const char search, const char replace) {
    /* If we have less than 32 bytes, fall back to the SSE variant */
    if(size < 32)
        return String::Implementation::replaceAllInPlaceCharacterImplementation(Cpu::Sse41)(data, size, search, replace);

    const __m256i vsearch = _mm256_set1_epi8(search);
    const __m256i vreplace = _mm256_set1_epi8(replace);

    /* Calculate the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll process some bytes twice. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 32) & ~0x1f);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 32 == 0);

    /* Unconditionally process the first vector a slower, unaligned way */
    {
        /* _mm256_lddqu_si256 is just an alias to _mm256_loadu_si256, no reason
           to use it: https://stackoverflow.com/a/47426790 */
        const __m256i in = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(data), out);
    }

    /* Process all aligned vectors. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. */
    char* const end = data + size;
    for(; i + 32 <= end; i += 32) {
        const __m256i in = _mm256_load_si256(reinterpret_cast<const __m256i*>(i));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_store_si256(reinterpret_cast<__m256i*>(i), out);
    }

    /* Handle remaining less than a vector in an unaligned way. Overlapping
       bytes are again no-op. */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 32 > end);
        i = end - 32;
        /* _mm256_lddqu_si256 is just an alias to _mm256_loadu_si256, no reason
           to use it: https://stackoverflow.com/a/47426790 */
        const __m256i in = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(i));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(i), out);
    }
}
#endif

#ifdef CORRADE_ENABLE_SIMD128
CORRADE_ENABLE_SIMD128 CORRADE_NEVER_INLINE void replaceAllInPlaceCharacterImplementationSimd128Unconditional(char* const data, const std::size_t size, const char search, const char replace) {
    /* If we have less than 16 bytes, do it the stupid way */
    {
        char* j = data;
        switch(size) {
            case 15: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 14: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 13: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 12: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 11: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case 10: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  9: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  8: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  7: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  6: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  5: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  4: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  3: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  2: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  1: if(*j++ == search) *(j - 1) = replace; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    const v128_t vsearch = wasm_i8x16_splat(search);
    const v128_t vreplace = wasm_i8x16_splat(replace);

    /* Calculate the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll process some bytes twice. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally process the first vector a slower, unaligned way. WASM
       doesn't differentiate between aligned and unaligned load/store, it's
       always unaligned, but the hardware might behave better if we try to
       avoid unaligned operations. */
    {
        const v128_t in = wasm_v128_load(reinterpret_cast<const v128_t*>(data));
        const v128_t out = wasm_v128_bitselect(vreplace, in, wasm_i8x16_eq(in, vsearch));
        wasm_v128_store(reinterpret_cast<v128_t*>(data), out);
    }

    /* Process all aligned vectors. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. */
    char* const end = data + size;
    for(; i + 16 <= end; i += 16) {
        const v128_t in = wasm_v128_load(reinterpret_cast<const v128_t*>(i));
        const v128_t out = wasm_v128_bitselect(vreplace, in, wasm_i8x16_eq(in, vsearch));
        wasm_v128_store(reinterpret_cast<v128_t*>(i), out);
    }

    /* Handle remaining less than a vector in an unaligned way. Overlapping
       bytes are again no-op. Again WASM doesn't have any dedicated unaligned
       load/store instruction. */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const v128_t in = wasm_v128_load(reinterpret_cast<const v128_t*>(i));
        const v128_t out = wasm_v128_bitselect(vreplace, in, wasm_i8x16_eq(in, vsearch));
        wasm_v128_store(reinterpret_cast<v128_t*>(i), out);
    }
}
#endif

}}}}
#endif

#endif
