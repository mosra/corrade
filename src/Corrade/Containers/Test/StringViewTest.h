#ifndef Corrade_Containers_Test_StringViewTest_h
#define Corrade_Containers_Test_StringViewTest_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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
/* Contains additional variants of StringView algorithms that are included
   just for historical / testing / benchmark comparison purposes; referenced
   explicitly via function pointers from the respective test case and benchmark
   instances. Used by StringViewTest.cpp and StringViewBenchmark.cpp. */

#include <cstdint>

#include "Corrade/Cpu.h"
#if defined(CORRADE_ENABLE_SSE41) && defined(CORRADE_ENABLE_BMI1)
#include "Corrade/Utility/IntrinsicsAvx.h" /* TZCNT is in AVX headers :( */
#endif
#if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_POPCNT)
#include "Corrade/Utility/IntrinsicsSse4.h"
#endif

namespace Corrade { namespace Containers { namespace Test { namespace {

/* Variant of the SSE2 + BMI1 implementation that uses testzero instead of
   movemask for the check, according to https://stackoverflow.com/a/26651301 */
#if defined(CORRADE_ENABLE_SSE41) && defined(CORRADE_ENABLE_BMI1)
CORRADE_ENABLE(SSE41,BMI1) CORRADE_NEVER_INLINE const char* stringFindCharacterImplementationSse41TestZero(const char* const data, const std::size_t size, const char character) {
    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) return j - 1; CORRADE_FALLTHROUGH
            case  0: return nullptr;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Unconditionally do a lookup in the first vector a slower, unaligned
       way */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        if(const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)))
            return data + _tzcnt_u32(mask);
    }

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll check some bytes twice. */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Go four vectors at a time with the aligned pointer */
    const char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);

        const __m128i eqA = _mm_cmpeq_epi8(vn1, a);
        const __m128i eqB = _mm_cmpeq_epi8(vn1, b);
        const __m128i eqC = _mm_cmpeq_epi8(vn1, c);
        const __m128i eqD = _mm_cmpeq_epi8(vn1, d);

        const __m128i or1 = _mm_or_si128(eqA, eqB);
        const __m128i or2 = _mm_or_si128(eqC, eqD);
        const __m128i or3 = _mm_or_si128(or1, or2);
        if(!_mm_testz_si128(or3, or3)) {
            if(const int mask = _mm_movemask_epi8(eqA))
                return i + 0*16 + _tzcnt_u32(mask);
            if(const int mask = _mm_movemask_epi8(eqB))
                return i + 1*16 + _tzcnt_u32(mask);
            if(const int mask = _mm_movemask_epi8(eqC))
                return i + 2*16 + _tzcnt_u32(mask);
            if(const int mask = _mm_movemask_epi8(eqD))
                return i + 3*16 + _tzcnt_u32(mask);
            CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        }
    }

    /* Handle remaining less than four aligned vectors */
    for(; i + 16 <= end; i += 16) {
        const __m128i chunk = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        if(const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)))
            return i + _tzcnt_u32(mask);
    }

    /* Handle remaining less than a vector with an unaligned search, again
       overlapping back with the previous already-searched elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        if(const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)))
            return i + _tzcnt_u32(mask);
    }

    return nullptr;
}
#endif

/* Variants of the current implementation that calculate the popcnt just on
   16 / 32 bytes each time, instead of 64 */
#if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_POPCNT)
CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2Popcnt16(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go one vector at a time */
    const char* const end = data + size;
    for(; i + 16 <= end; i += 16) {
        const __m128i chunk = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        count += _mm_popcnt_u32(_mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)));
    }

    /* Handle remaining less than a vector with an unaligned load, again with
       the overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ~((1 << (i + 16 - end)) - 1));
    }

    return count;
}

CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2Popcnt32(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go two vectors at a time */
    const char* const end = data + size;
    for(; i + 2*16 <= end; i += 2*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        count += _mm_popcnt_u32((_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1)) << 0) |
                                (_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1)) << 16));
    }

    /* Handle remaining less than two vectors */
    for(; i + 16 <= end; i += 16) {
        const __m128i chunk = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        count += _mm_popcnt_u32(_mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)));
    }

    /* Handle remaining less than a vector with an unaligned load, again with
       the overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ~((1 << (i + 16 - end)) - 1));
    }

    return count;
}
#endif

/* Variant of the current implementation with the postamble being implemented
   with popcount being called on every vector, or on all four vectors
   accumulated together.

   The 64-bit variants of POPCNT instructions aren't exposed on 32-bit systems
   for some reason, skipping there. */
#if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_POPCNT) && !defined(CORRADE_TARGET_32BIT)
CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2PostamblePopcnt16(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go four vectors at a time */
    const char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);
        count += _mm_popcnt_u64(
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1))) <<  0) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1))) << 16) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(c, vn1))) << 32) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(d, vn1))) << 48));
    }

    /* Handle remaining less than four vectors */
    for(; i + 16 <= end; i += 16) {
        const __m128i chunk = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        count += _mm_popcnt_u32(_mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1)));
    }

    /* Handle remaining less than a vector with an unaligned load, again with
       the overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ~((1 << (i + 16 - end)) - 1));
    }

    return count;
}

CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2PostamblePopcnt64(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go four vectors at a time */
    const char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);
        count += _mm_popcnt_u64(
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1))) <<  0) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1))) << 16) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(c, vn1))) << 32) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(d, vn1))) << 48));
    }

    /* Accumulate remaining less than four vectors for a last popcnt */
    std::uint64_t mask = 0;
    if(i + 2*16 <= end) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1))) << 0;
        mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1))) << 16;
        i += 2*16;
    }
    if(i + 16 <= end) {
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(c, vn1))) << 32;
        i += 16;
    }

    /* Load remaining less than a vector with an unaligned load, again with the
       overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        mask |= std::uint64_t(found & ~((1 << (i + 16 - end)) - 1)) << 48;
    }

    return count + _mm_popcnt_u64(mask);
}

CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2PostamblePopcnt64Switch(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go four vectors at a time */
    const char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);
        count += _mm_popcnt_u64(
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1))) <<  0) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1))) << 16) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(c, vn1))) << 32) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(d, vn1))) << 48));
    }

    /* Accumulate remaining less than four vectors for a last popcnt */
    std::uint64_t mask = 0;
    switch((end - i)/16) {
        case 3:
            mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(
                _mm_load_si128(reinterpret_cast<const __m128i*>(i)), vn1))) << 48;
            i += 16;
            CORRADE_FALLTHROUGH
        case 2:
            mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(
                _mm_load_si128(reinterpret_cast<const __m128i*>(i)), vn1))) << 32;
            i += 16;
            CORRADE_FALLTHROUGH
        case 1:
            mask |= std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(
                _mm_load_si128(reinterpret_cast<const __m128i*>(i)), vn1))) << 16;
            i += 16;
    }

    /* Load remaining less than a vector with an unaligned load, again with the
       overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        mask |= std::uint64_t(found & ~((1 << (i + 16 - end)) - 1));
    }

    return count + _mm_popcnt_u64(mask);
}

CORRADE_ENABLE(SSE2,POPCNT) CORRADE_NEVER_INLINE std::size_t stringCountCharacterImplementationSse2PostamblePopcnt64Loop(const char* const data, const std::size_t size, const char character) {
    std::size_t count = 0;

    /* If we have less than 16 bytes, do it the stupid way */
    {
        const char* j = data;
        switch(size) {
            case 15: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 14: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 13: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 12: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 11: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case 10: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  9: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  8: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  7: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  6: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  5: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  4: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  3: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  2: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  1: if(*j++ == character) ++count; CORRADE_FALLTHROUGH
            case  0: return count;
        }
    }

    const __m128i vn1 = _mm_set1_epi8(character);

    /* Calculate the next aligned position */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Unconditionally load the first vector a slower, unaligned way, and mask
       out the part that overlaps with the next aligned position to not count
       it twice */
    {
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        count += _mm_popcnt_u32(found & ((1 << (i - data)) - 1));
    }

    /* Go four vectors at a time */
    const char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i b = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i c = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);
        count += _mm_popcnt_u64(
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(a, vn1))) <<  0) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(b, vn1))) << 16) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(c, vn1))) << 32) |
            (std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(d, vn1))) << 48));
    }

    /* Accumulate remaining less than four vectors for a last popcnt */
    std::uint64_t mask = 0;
    for(; i + 16 <= end; i += 16) {
        mask = (mask << 16)|std::uint64_t(_mm_movemask_epi8(_mm_cmpeq_epi8(
            _mm_load_si128(reinterpret_cast<const __m128i*>(i)), vn1)));
    }

    /* Load remaining less than a vector with an unaligned load, again with the
       overlapping part masked out to not count it twice */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(end - 16));
        const std::uint32_t found = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vn1));
        mask |= std::uint64_t(found & ~((1 << (i + 16 - end)) - 1)) << 48;
    }

    return count + _mm_popcnt_u64(mask);
}
#endif

}}}}
#endif

#endif
