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

#include "String.h"

#include <cctype>
#include <cstring>

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/Implementation/cpu.h"
#if defined(CORRADE_ENABLE_AVX2) || defined(CORRADE_ENABLE_BMI1)
#include "Corrade/Utility/IntrinsicsAvx.h" /* TZCNT is in AVX headers :( */
#elif defined(CORRADE_ENABLE_SSE41)
#include "Corrade/Utility/IntrinsicsSse4.h"
#elif defined(CORRADE_ENABLE_SSE2)
#include "Corrade/Utility/IntrinsicsSse2.h"
#endif
#ifdef CORRADE_ENABLE_SIMD128
#include <wasm_simd128.h>
#endif

namespace Corrade { namespace Utility { namespace String {

namespace Implementation {

void ltrimInPlace(std::string& string, const Containers::ArrayView<const char> characters) {
    string.erase(0, string.find_first_not_of(characters, 0, characters.size()));
}

void rtrimInPlace(std::string& string, const Containers::ArrayView<const char> characters) {
    string.erase(string.find_last_not_of(characters, std::string::npos, characters.size())+1);
}

void trimInPlace(std::string& string, const Containers::ArrayView<const char> characters) {
    rtrimInPlace(string, characters);
    ltrimInPlace(string, characters);
}

std::string ltrim(std::string string, const Containers::ArrayView<const char> characters) {
    ltrimInPlace(string, characters);
    return string;
}

std::string rtrim(std::string string, const Containers::ArrayView<const char> characters) {
    rtrimInPlace(string, characters);
    return string;
}

std::string trim(std::string string, const Containers::ArrayView<const char> characters) {
    trimInPlace(string, characters);
    return string;
}

std::string join(const std::vector<std::string>& strings, const Containers::ArrayView<const char> delimiter) {
    /* IDGAF that this has two extra allocations due to the Array being created
       and then the String converted to a std::string vector, the input
       std::string instances are MUCH worse */
    Containers::Array<Containers::StringView> stringViews{strings.size()};
    for(std::size_t i = 0; i != strings.size(); ++i)
        stringViews[i] = strings[i];
    return Containers::StringView{delimiter}.join(stringViews);
}

std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, const Containers::ArrayView<const char> delimiter) {
    /* IDGAF that this has two extra allocations due to the Array being created
       and then the String converted to a std::string vector, the input
       std::string instances are MUCH worse */
    Containers::Array<Containers::StringView> stringViews{strings.size()};
    for(std::size_t i = 0; i != strings.size(); ++i)
        stringViews[i] = strings[i];
    return Containers::StringView{delimiter}.joinWithoutEmptyParts(stringViews);
}

bool beginsWith(Containers::ArrayView<const char> string, const Containers::ArrayView<const char> prefix) {
    /* This is soon meant to be deprecated so all the ugly conversions don't
       bother me too much */
    return Containers::StringView{string}.hasPrefix(Containers::StringView{prefix});
}

bool endsWith(Containers::ArrayView<const char> string, const Containers::ArrayView<const char> suffix) {
    /* This is soon meant to be deprecated so all the ugly conversions don't
       bother me too much */
    return Containers::StringView{string}.hasSuffix(Containers::StringView{suffix});
}

std::string stripPrefix(std::string string, const Containers::ArrayView<const char> prefix) {
    CORRADE_ASSERT(beginsWith({string.data(), string.size()}, prefix),
        "Utility::String::stripPrefix(): string doesn't begin with given prefix", {});
    string.erase(0, prefix.size());
    return string;
}

std::string stripSuffix(std::string string, const Containers::ArrayView<const char> suffix) {
    CORRADE_ASSERT(endsWith({string.data(), string.size()}, suffix),
        "Utility::String::stripSuffix(): string doesn't end with given suffix", {});
    string.erase(string.size() - suffix.size());
    return string;
}

}

namespace {
    using namespace Containers::Literals;
    constexpr Containers::StringView Whitespace = " \t\f\v\r\n"_s;
}

std::string ltrim(std::string string) { return ltrim(std::move(string), Whitespace); }

std::string rtrim(std::string string) { return rtrim(std::move(string), Whitespace); }

std::string trim(std::string string) { return trim(std::move(string), Whitespace); }

void ltrimInPlace(std::string& string) { ltrimInPlace(string, Whitespace); }

void rtrimInPlace(std::string& string) { rtrimInPlace(string, Whitespace); }

void trimInPlace(std::string& string) { trimInPlace(string, Whitespace); }

#ifdef CORRADE_BUILD_DEPRECATED
Containers::Array<Containers::StringView> split(const Containers::StringView string, const char delimiter) {
    return string.split(delimiter);
}

Containers::Array<Containers::StringView> splitWithoutEmptyParts(const Containers::StringView string, const char delimiter) {
    return string.splitWithoutEmptyParts(delimiter);
}

Containers::Array<Containers::StringView> splitWithoutEmptyParts(const Containers::StringView string, const Containers::StringView delimiters) {
    return string.splitOnAnyWithoutEmptyParts(delimiters);
}

Containers::Array<Containers::StringView> splitWithoutEmptyParts(const Containers::StringView string) {
    return string.splitOnWhitespaceWithoutEmptyParts();
}
#endif

std::vector<std::string> split(const std::string& string, const char delimiter) {
    /* IDGAF that this has one extra allocation due to the Array being copied
       to a std::vector, the owning std::string instances are much worse */
    Containers::Array<Containers::StringView> parts = Containers::StringView{string}.split(delimiter);
    return std::vector<std::string>{parts.begin(), parts.end()};
}

std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const char delimiter) {
    /* IDGAF that this has one extra allocation due to the Array being copied
       to a std::vector, the owning std::string instances are much worse */
    Containers::Array<Containers::StringView> parts = Containers::StringView{string}.splitWithoutEmptyParts(delimiter);
    return std::vector<std::string>{parts.begin(), parts.end()};
}

std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const std::string& delimiters) {
    /* IDGAF that this has one extra allocation due to the Array being copied
       to a std::vector, the owning std::string instances are much worse */
    Containers::Array<Containers::StringView> parts = Containers::StringView{string}.splitOnAnyWithoutEmptyParts(delimiters);
    return std::vector<std::string>{parts.begin(), parts.end()};
}

std::vector<std::string> splitWithoutEmptyParts(const std::string& string) {
    /* IDGAF that this has one extra allocation due to the Array being copied
       to a std::vector, the owning std::string instances are much worse */
    Containers::Array<Containers::StringView> parts = Containers::StringView{string}.splitOnWhitespaceWithoutEmptyParts();
    return std::vector<std::string>{parts.begin(), parts.end()};
}

namespace {

Containers::StaticArray<3, std::string> partitionInternal(const std::string& string, Containers::ArrayView<const char> separator) {
    const std::size_t pos = string.find(separator, 0, separator.size());
    return {
        string.substr(0, pos),
        pos == std::string::npos ? std::string{} : string.substr(pos, separator.size()),
        pos == std::string::npos ? std::string{} : string.substr(pos + separator.size())
    };
}

Containers::StaticArray<3, std::string> rpartitionInternal(const std::string& string, Containers::ArrayView<const char> separator) {
    const std::size_t pos = string.rfind(separator, std::string::npos, separator.size());
    return {
        pos == std::string::npos ? std::string{} : string.substr(0, pos),
        pos == std::string::npos ? std::string{} : string.substr(pos, separator.size()),
        pos == std::string::npos ? string.substr(0) : string.substr(pos + separator.size())
    };
}

}

Containers::StaticArray<3, std::string> partition(const std::string& string, char separator) {
    return partitionInternal(string, {&separator, 1});
}

Containers::StaticArray<3, std::string> partition(const std::string& string, const std::string& separator) {
    return partitionInternal(string, {separator.data(), separator.size()});
}

Containers::StaticArray<3, std::string> rpartition(const std::string& string, char separator) {
    return rpartitionInternal(string, {&separator, 1});
}

Containers::StaticArray<3, std::string> rpartition(const std::string& string, const std::string& separator) {
    return rpartitionInternal(string, {separator.data(), separator.size()});
}

namespace Implementation {

namespace {

/* Basically a variant of the stringFindCharacterImplementation(), using the
   same high-level logic with branching only on every four vectors. See its
   documentation for more information. */
#if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE(SSE2,BMI1) typename std::decay<decltype(commonPrefix)>::type commonPrefixImplementation(CORRADE_CPU_DECLARE(Cpu::Sse2|Cpu::Bmi1)) {
  return [](const char* const a, const char* const b, const std::size_t sizeA, const std::size_t sizeB) CORRADE_ENABLE(SSE2,BMI1) {
    const std::size_t size = Utility::min(sizeA, sizeB);

    /* If we have less than 16 bytes, do it the stupid way */
    /** @todo that this worked best for stringFindCharacterImplementation()
        doesn't mean it's the best variant here as well */
    {
        const char *i = a, *j = b;
        switch(size) {
            case 15: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case 14: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case 13: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case 12: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case 11: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case 10: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  9: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  8: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  7: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  6: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  5: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  4: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  3: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  2: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  1: if(*i++ != *j++) return i - 1; CORRADE_FALLTHROUGH
            case  0: return a + size;
        }
    }

    /* Unconditionally compare the first vector a slower, unaligned way */
    {
        const __m128i chunkA = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a));
        const __m128i chunkB = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b));
        const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunkA, chunkB));
        if(mask != 0xffff)
            return a + _tzcnt_u32(~mask);
    }

    /* Go to the next aligned position of *one* of the inputs. If the pointer
       was already aligned, we'll go to the next aligned vector; if not, there
       will be an overlap and we'll check some bytes twice.

       The other input is then processed unaligned, or if we're lucky it's
       aligned the same way (such as when the strings are at the start of a
       default-aligned allocation, which on 64 bits is 16 bytes). Alternatively
       we could try to load both in an aligned way and then compare shifted
       values, but on recent architecture the extra overhead from the patching
       would probably be larger than just reading unaligned. */
    const char* i = reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(a + 16) & ~0xf);
    const char* j = b + (i - a);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > a && j > b && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Go four vectors at a time with the aligned pointer */
    const char* const endA = a + size;
    const char* const endB = b + size;
    for(; i + 4*16 <= endA; i += 4*16, j += 4*16) {
        const __m128i iA = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i iB = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i iC = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i iD = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);
        /* The second input is loaded unaligned always */
        const __m128i jA = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j) + 0);
        const __m128i jB = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j) + 1);
        const __m128i jC = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j) + 2);
        const __m128i jD = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j) + 3);

        const __m128i eqA = _mm_cmpeq_epi8(iA, jA);
        const __m128i eqB = _mm_cmpeq_epi8(iB, jB);
        const __m128i eqC = _mm_cmpeq_epi8(iC, jC);
        const __m128i eqD = _mm_cmpeq_epi8(iD, jD);

        const __m128i and1 = _mm_and_si128(eqA, eqB);
        const __m128i and2 = _mm_and_si128(eqC, eqD);
        const __m128i and3 = _mm_and_si128(and1, and2);
        if(_mm_movemask_epi8(and3) != 0xffff) {
            const int maskA = _mm_movemask_epi8(eqA);
            if(maskA != 0xffff)
                return i + 0*16 + _tzcnt_u32(~maskA);
            const int maskB = _mm_movemask_epi8(eqB);
            if(maskB != 0xffff)
                return i + 1*16 + _tzcnt_u32(~maskB);
            const int maskC = _mm_movemask_epi8(eqC);
            if(maskC != 0xffff)
                return i + 2*16 + _tzcnt_u32(~maskC);
            const int maskD = _mm_movemask_epi8(eqD);
            if(maskD != 0xffff)
                return i + 3*16 + _tzcnt_u32(~maskD);
            CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        }
    }

    /* Handle remaining less than four aligned vectors */
    for(; i + 16 <= endA; i += 16, j += 16) {
        const __m128i chunkA = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        /* The second input is loaded unaligned always */
        const __m128i chunkB = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j));
        const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunkA, chunkB));
        if(mask != 0xffff)
            return i + _tzcnt_u32(~mask);
    }

    /* Handle remaining less than a vector with an unaligned load, again
       overlapping back with the previous already-compared elements */
    if(i < endA) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > endA && endB - j == endA - i);
        i = endA - 16;
        j = endB - 16;
        const __m128i chunkA = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        const __m128i chunkB = _mm_loadu_si128(reinterpret_cast<const __m128i*>(j));
        const int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunkA, chunkB));
        if(mask != 0xffff)
            return i + _tzcnt_u32(~mask);
    }

    return endA;
  };
}
#endif

CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(commonPrefix)>::type commonPrefixImplementation(CORRADE_CPU_DECLARE(Cpu::Scalar)) {
  return [](const char* const a, const char* const b, const std::size_t sizeA, const std::size_t sizeB) {
    const std::size_t size = Utility::min(sizeA, sizeB);
    const char* const endA = a + size;
    for(const char *i = a, *j = b; i != endA; ++i, ++j)
        if(*i != *j) return i;
    return endA;
  };
}

}

#ifdef CORRADE_TARGET_X86
CORRADE_UTILITY_CPU_DISPATCHER(commonPrefixImplementation, Cpu::Bmi1)
#else
CORRADE_UTILITY_CPU_DISPATCHER(commonPrefixImplementation)
#endif
CORRADE_UTILITY_CPU_DISPATCHED(commonPrefixImplementation, const char* CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(commonPrefix)(const char* a, const char* b, std::size_t sizeA, std::size_t sizeB))({
    return commonPrefixImplementation(CORRADE_CPU_SELECT(Cpu::Default))(a, b, sizeA, sizeB);
})

}

namespace Implementation {

namespace {

CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(lowercaseInPlace)>::type lowercaseInPlaceImplementation(Cpu::ScalarT) {
  return [](char* data, const std::size_t size) {
    /* A proper Unicode-aware *and* locale-aware solution would involve far
       more than iterating over bytes -- multi-byte characters, composed
       characters (ä formed from ¨ and a), SS -> ß in German but not elsewhere
       etc... */

    /* Branchless idea from https://stackoverflow.com/a/3884737, what it does
       is adding (1 << 5) for 'A' and all 26 letters after, and (0 << 5) for
       anything after (and before as well, which is what the unsigned cast
       does). The (1 << 5) bit (0x20) is what differs between lowercase and
       uppercase characters. See Test/StringBenchmark.cpp for other alternative
       implementations leading up to this point. In particular, the
       std::uint8_t() is crucial, unsigned() is 6x to 8x slower. */
    const char* const end = data + size;
    for(char* c = data; c != end; ++c)
        *c += (std::uint8_t(*c - 'A') < 26) << 5;
  };
}

CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(lowercaseInPlace)>::type uppercaseInPlaceImplementation(Cpu::ScalarT) {
  return [](char* data, const std::size_t size) {
    /* Same as above, except that (1 << 5) is subtracted for 'a' and all 26
       letters after. */
    const char* const end = data + size;
    for(char* c = data; c != end; ++c)
        *c -= (std::uint8_t(*c - 'a') < 26) << 5;
  };
}

#ifdef CORRADE_ENABLE_SSE2
/* The core vector algorithm was reverse-engineered from what GCC (and
   apparently also Clang) does for the scalar case with SSE2 optimizations
   enabled. It's the same count of instructions as the "obvious" case of doing
   two comparisons per character, ORing that, and then applying a bitmask, but
   considerably faster. The obvious case is implemented and benchmarked in
   StringBenchmark::lowercaseSse2TwoCompares() for comparison. */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SSE2 typename std::decay<decltype(lowercaseInPlace)>::type lowercaseInPlaceImplementation(Cpu::Sse2T) {
  return [](char* const data, const std::size_t size) CORRADE_ENABLE_SSE2 {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    /** @todo investigate perf implications of putting this into a helper
        function that's reused across all SSE / NEON / WASM SIMD variants --
        the way it is now there's just one jump into the switch, with the
        helper it'd be another if() around */
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
    const __m128i aAndAbove = _mm_set1_epi8(char(256u - std::uint8_t('A')));
    const __m128i lowest25 = _mm_set1_epi8(25);
    const __m128i lowercaseBit = _mm_set1_epi8(0x20);
    const auto lowercaseOneVector = [&](const __m128i chars) CORRADE_ENABLE_SSE2 {
        /* Moves 'A' and everything above to 0 and up (it overflows and wraps
           around) */
        const __m128i uppercaseInLowest25 = _mm_add_epi8(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'A' to 'Z'
           (now 0 to 25) zero and everything else non-zero */
        const __m128i lowest25IsZero = _mm_subs_epu8(uppercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const __m128i maskUppercase = _mm_cmpeq_epi8(lowest25IsZero, _mm_setzero_si128());
        /* For the masked chars a lowercase bit is set, and the bit is then
           added to the original chars, making the uppercase chars lowercase */
        return _mm_add_epi8(chars, _mm_and_si128(maskUppercase, lowercaseBit));
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
  };
}

/* Compared to the lowercase implementation it (obviously) uses the scalar
   uppercasing code in the less-than-16 case. In the vector case zeroes out the
   a-z range instead of A-Z, and subtracts the lowercase bit instead of
   adding. */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SSE2 typename std::decay<decltype(lowercaseInPlace)>::type uppercaseInPlaceImplementation(Cpu::Sse2T) {
  return [](char* const data, const std::size_t size) CORRADE_ENABLE_SSE2 {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    /** @todo investigate perf implications of putting this into a helper
        function that's reused across all SSE / NEON / WASM SIMD variants --
        the way it is now there's just one jump into the switch, with the
        helper it'd be another if() around */
    {
        char* j = data;
        switch(size) {
            case 15: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 14: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 13: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 12: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 11: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 10: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  9: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  8: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  7: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  6: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  5: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  4: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  3: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  2: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  1: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    /* Core algorithm */
    const __m128i aAndAbove = _mm_set1_epi8(char(256u - std::uint8_t('a')));
    const __m128i lowest25 = _mm_set1_epi8(25);
    const __m128i lowercaseBit = _mm_set1_epi8(0x20);
    const auto uppercaseOneVector = [&](const __m128i chars) CORRADE_ENABLE_SSE2 {
        /* Moves 'a' and everything above to 0 and up (it overflows and wraps
           around) */
        const __m128i lowercaseInLowest25 = _mm_add_epi8(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'a' to 'z'
           (now 0 to 25) zero and everything else non-zero */
        const __m128i lowest25IsZero = _mm_subs_epu8(lowercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const __m128i maskUppercase = _mm_cmpeq_epi8(lowest25IsZero, _mm_setzero_si128());
        /* For the masked chars a lowercase bit is set, and the bit is then
           subtracted from the original chars, making the lowercase chars
           uppercase */
        return _mm_sub_epi8(chars, _mm_and_si128(maskUppercase, lowercaseBit));
    };

    /* Unconditionally convert the first vector in a slower, unaligned way. Any
       extra branching to avoid the unaligned load & store if already aligned
       would be most probably more expensive than the actual operation. */
    {
        const __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(data), uppercaseOneVector(chars));
    }

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, uppercasing
       already-uppercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Convert all aligned vectors using aligned load/store */
    for(; i + 16 <= end; i += 16) {
        const __m128i chars = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        _mm_store_si128(reinterpret_cast<__m128i*>(i), uppercaseOneVector(chars));
    }

    /* Handle remaining less than a vector with an unaligned load & store,
       again overlapping back with the previous already-converted elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(i), uppercaseOneVector(chars));
    }
  };
}
#endif

#ifdef CORRADE_ENABLE_AVX2
/* Trivial extension of the SSE2 code to AVX2. The only significant difference
   is a workaround for MinGW, see the comment below. */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_AVX2 typename std::decay<decltype(lowercaseInPlace)>::type lowercaseInPlaceImplementation(Cpu::Avx2T) {
  return [](char* const data, const std::size_t size) CORRADE_ENABLE_AVX2 {
    char* const end = data + size;

    /* If we have less than 32 bytes, fall back to the SSE variant */
    /** @todo deinline it here? any speed gains from rewriting using 128-bit
        AVX? or does the compiler do that automatically? */
    if(size < 32)
        return lowercaseInPlaceImplementation(Cpu::Sse2)(data, size);

    /* Core algorithm */
    const __m256i aAndAbove = _mm256_set1_epi8(char(256u - std::uint8_t('A')));
    const __m256i lowest25 = _mm256_set1_epi8(25);
    const __m256i lowercaseBit = _mm256_set1_epi8(0x20);
    /* Compared to the SSE2 case, this performs the operation in-place on a
       __m256i reference instead of taking and returning it by value. This is
       in order to work around a MinGW / Windows GCC bug, where it doesn't
       align __m256i instances passed to or returned from functions to 32 bytes
       but still uses aligned load/store for them. Reported back in 2011, still
       not fixed even in late 2023:
        https://gcc.gnu.org/bugzilla/show_bug.cgi?id=49001
        https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412 */
    const auto lowercaseOneVectorInPlace = [&](__m256i& chars) CORRADE_ENABLE_AVX2 {
        /* Moves 'A' and everything above to 0 and up (it overflows and wraps
           around) */
        const __m256i uppercaseInLowest25 = _mm256_add_epi8(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'A' to 'Z'
           (now 0 to 25) zero and everything else non-zero */
        const __m256i lowest25IsZero = _mm256_subs_epu8(uppercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const __m256i maskUppercase = _mm256_cmpeq_epi8(lowest25IsZero, _mm256_setzero_si256());
        /* For the masked chars a lowercase bit is set, and the bit is then
           added to the original chars, making the uppercase chars lowercase */
        chars = _mm256_add_epi8(chars, _mm256_and_si256(maskUppercase, lowercaseBit));
    };

    /* Unconditionally convert the first vector in a slower, unaligned way. Any
       extra branching to avoid the unaligned load & store if already aligned
       would be most probably more expensive than the actual operation. */
    {
        __m256i chars = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
        lowercaseOneVectorInPlace(chars);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(data), chars);
    }

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, lowercasing
       already-lowercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 32) & ~0x1f);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 32 == 0);

    /* Convert all aligned vectors using aligned load/store */
    for(; i + 32 <= end; i += 32) {
        __m256i chars = _mm256_load_si256(reinterpret_cast<const __m256i*>(i));
        lowercaseOneVectorInPlace(chars);
        _mm256_store_si256(reinterpret_cast<__m256i*>(i), chars);
    }

    /* Handle remaining less than a vector with an unaligned load & store,
       again overlapping back with the previous already-converted elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 32 > end);
        i = end - 32;
        __m256i chars = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(i));
        lowercaseOneVectorInPlace(chars);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(i), chars);
    }
  };
}

/* Again just trivial extension to AVX2, and the MinGW workaround */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_AVX2 typename std::decay<decltype(lowercaseInPlace)>::type uppercaseInPlaceImplementation(Cpu::Avx2T) {
  return [](char* const data, const std::size_t size) CORRADE_ENABLE_AVX2 {
    char* const end = data + size;

    /* If we have less than 32 bytes, fall back to the SSE variant */
    /** @todo deinline it here? any speed gains from rewriting using 128-bit
        AVX? or does the compiler do that automatically? */
    if(size < 32)
        return uppercaseInPlaceImplementation(Cpu::Sse2)(data, size);

    /* Core algorithm */
    const __m256i aAndAbove = _mm256_set1_epi8(char(256u - std::uint8_t('a')));
    const __m256i lowest25 = _mm256_set1_epi8(25);
    const __m256i lowercaseBit = _mm256_set1_epi8(0x20);
    /* See the comment next to lowercaseOneVectorInPlace() above for why this
       is done in-place */
    const auto uppercaseOneVectorInPlace = [&](__m256i& chars) CORRADE_ENABLE_AVX2 {
        /* Moves 'a' and everything above to 0 and up (it overflows and wraps
           around) */
        const __m256i lowercaseInLowest25 = _mm256_add_epi8(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'a' to 'z'
           (now 0 to 25) zero and everything else non-zero */
        const __m256i lowest25IsZero = _mm256_subs_epu8(lowercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const __m256i maskUppercase = _mm256_cmpeq_epi8(lowest25IsZero, _mm256_setzero_si256());
        /* For the masked chars a lowercase bit is set, and the bit is then
           subtracted from the original chars, making the lowercase chars
           uppercase */
        chars = _mm256_sub_epi8(chars, _mm256_and_si256(maskUppercase, lowercaseBit));
    };

    /* Unconditionally convert the first vector in a slower, unaligned way. Any
       extra branching to avoid the unaligned load & store if already aligned
       would be most probably more expensive than the actual operation. */
    {
        __m256i chars = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
        uppercaseOneVectorInPlace(chars);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(data), chars);
    }

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, uppercasing
       already-uppercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 32) & ~0x1f);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 32 == 0);

    /* Convert all aligned vectors using aligned load/store */
    for(; i + 32 <= end; i += 32) {
        __m256i chars = _mm256_load_si256(reinterpret_cast<const __m256i*>(i));
        uppercaseOneVectorInPlace(chars);
        _mm256_store_si256(reinterpret_cast<__m256i*>(i), chars);
    }

    /* Handle remaining less than a vector with an unaligned load & store,
       again overlapping back with the previous already-converted elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 32 > end);
        i = end - 32;
        __m256i chars = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(i));
        uppercaseOneVectorInPlace(chars);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(i), chars);
    }
  };
}
#endif

#ifdef CORRADE_ENABLE_SIMD128
/* Trivial port of the SSE2 code to WASM SIMD. As WASM SIMD doesn't
   differentiate between aligned and unaligned load, the load/store code is the
   same for both aligned and unaligned case, making everything slightly
   shorter. The high-level operation stays the same as with SSE2 tho, even if
   just for memory access patterns I think it still makes sense to do as much
   as possible aligned. */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SIMD128 typename std::decay<decltype(lowercaseInPlace)>::type lowercaseInPlaceImplementation(Cpu::Simd128T) {
  return [](char* data, const std::size_t size) CORRADE_ENABLE_SIMD128 {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    /** @todo this doesn't seem to perform any differently from the
        non-unrolled scalar case, turn into a plain loop? */
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
    const v128_t aAndAbove = wasm_i8x16_const_splat(char(256u - std::uint8_t('A')));
    const v128_t lowest25 = wasm_i8x16_const_splat(25);
    const v128_t lowercaseBit = wasm_i8x16_const_splat(0x20);
    const v128_t zero = wasm_i8x16_const_splat(0);
    const auto lowercaseOneVectorInPlace = [&](v128_t* const data) CORRADE_ENABLE_SIMD128 {
        const v128_t chars = wasm_v128_load(data);
        /* Moves 'A' and everything above to 0 and up (it overflows and wraps
           around) */
        const v128_t uppercaseInLowest25 = wasm_i8x16_add(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'A' to 'Z'
           (now 0 to 25) zero and everything else non-zero */
        const v128_t lowest25IsZero = wasm_u8x16_sub_sat(uppercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const v128_t maskUppercase = wasm_i8x16_eq(lowest25IsZero, zero);
        /* For the masked chars a lowercase bit is set, and the bit is then
           added to the original chars, making the uppercase chars lowercase */
        wasm_v128_store(data, wasm_i8x16_add(chars, wasm_v128_and(maskUppercase, lowercaseBit)));
    };

    /* Unconditionally convert the first unaligned vector */
    lowercaseOneVectorInPlace(reinterpret_cast<v128_t*>(data));

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, lowercasing
       already-lowercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Convert all aligned vectors */
    for(; i + 16 <= end; i += 16)
        lowercaseOneVectorInPlace(reinterpret_cast<v128_t*>(i));

    /* Handle remaining less than a vector, again overlapping back with the
       previous already-converted elements, in an unaligned way */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        lowercaseOneVectorInPlace(reinterpret_cast<v128_t*>(i));
    }
  };
}

/* Again just a trivial port of the SSE2 code to WASM SIMD, with the same
   "aligned load/store is the same as unaligned" simplification as the
   lowercase variant above */
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SIMD128 typename std::decay<decltype(lowercaseInPlace)>::type uppercaseInPlaceImplementation(Cpu::Simd128T) {
  return [](char* data, const std::size_t size) CORRADE_ENABLE_SIMD128 {
    char* const end = data + size;

    /* If we have less than 16 bytes, do it the stupid way, equivalent to the
       scalar variant and just unrolled. */
    /** @todo this doesn't seem to perform any differently from the
        non-unrolled scalar case, turn into a plain loop? */
    {
        char* j = data;
        switch(size) {
            case 15: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 14: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 13: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 12: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 11: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case 10: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  9: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  8: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  7: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  6: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  5: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  4: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  3: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  2: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  1: *j -= (std::uint8_t(*j - 'a') < 26) << 5; ++j; CORRADE_FALLTHROUGH
            case  0: return;
        }
    }

    /* Core algorithm */
    const v128_t aAndAbove = wasm_i8x16_const_splat(char(256u - std::uint8_t('a')));
    const v128_t lowest25 = wasm_i8x16_const_splat(25);
    const v128_t lowercaseBit = wasm_i8x16_const_splat(0x20);
    const v128_t zero = wasm_i8x16_const_splat(0);
    const auto uppercaseOneVectorInPlace = [&](v128_t* const data) CORRADE_ENABLE_SIMD128 {
        const v128_t chars = wasm_v128_load(data);
        /* Moves 'a' and everything above to 0 and up (it overflows and wraps
           around) */
        const v128_t lowercaseInLowest25 = wasm_i8x16_add(chars, aAndAbove);
        /* Subtracts 25 with saturation, which makes the original 'a' to 'z'
           (now 0 to 25) zero and everything else non-zero */
        const v128_t lowest25IsZero = wasm_u8x16_sub_sat(lowercaseInLowest25, lowest25);
        /* Mask indicating where uppercase letters where, i.e. which values are
           now zero */
        const v128_t maskUppercase = wasm_i8x16_eq(lowest25IsZero, zero);
        /* For the masked chars a lowercase bit is set, and the bit is then
           subtracted from the original chars, making the lowercase chars
           uppercase */
        wasm_v128_store(data, wasm_i8x16_sub(chars, wasm_v128_and(maskUppercase, lowercaseBit)));
    };

    /* Unconditionally convert the first unaligned vector. WASM doesn't
       differentiate between aligned and unaligned load, it's always unaligned,
       however even if just for memory access patterns I think it still makes
       sense to do as much as possible aligned, so this matches what the SSE2
       code does. */
    uppercaseOneVectorInPlace(reinterpret_cast<v128_t*>(data));

    /* Go to the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll convert some bytes twice. Which is fine, uppercasing
       already-uppercased data is a no-op. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 16) & ~0xf);
    CORRADE_INTERNAL_DEBUG_ASSERT(i >= data && reinterpret_cast<std::uintptr_t>(i) % 16 == 0);

    /* Convert all aligned vectors */
    for(; i + 16 <= end; i += 16)
        uppercaseOneVectorInPlace(reinterpret_cast<v128_t*>(i));

    /* Handle remaining less than a vector with an unaligned load & store,
       again overlapping back with the previous already-converted elements */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        uppercaseOneVectorInPlace(reinterpret_cast<v128_t*>(i));
    }
  };
}
#endif

}

CORRADE_UTILITY_CPU_DISPATCHER_BASE(lowercaseInPlaceImplementation)
CORRADE_UTILITY_CPU_DISPATCHER_BASE(uppercaseInPlaceImplementation)
CORRADE_UTILITY_CPU_DISPATCHED(lowercaseInPlaceImplementation, void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(lowercaseInPlace)(char* data, std::size_t size))({
    return lowercaseInPlaceImplementation(Cpu::DefaultBase)(data, size);
})
CORRADE_UTILITY_CPU_DISPATCHED(uppercaseInPlaceImplementation, void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(uppercaseInPlace)(char* data, std::size_t size))({
    return uppercaseInPlaceImplementation(Cpu::DefaultBase)(data, size);
})

}

Containers::String lowercase(const Containers::StringView string) {
    /* Theoretically doing the copy in the same loop as case change could be
       faster for *really long* strings due to cache reuse, but until that
       proves to be a bottleneck I'll go with the simpler solution.

       Not implementing through lowercase(Containers::String) as the call stack
       is deep enough already and we don't need the extra checks there. */
    Containers::String out{string};
    lowercaseInPlace(out);
    return out;
}

Containers::String lowercase(Containers::String string) {
    /* In the rare scenario where we'd get a non-owned string (such as
       String::nullTerminatedView() passed right into the function), make it
       owned first. Usually it'll get copied however, which already makes it
       owned. */
    if(!string.isSmall() && string.deleter()) string = Containers::String{string};

    lowercaseInPlace(string);
    return string;
}

std::string lowercase(std::string string) {
    lowercaseInPlace(string);
    return string;
}

Containers::String uppercase(const Containers::StringView string) {
    /* Theoretically doing the copy in the same loop as case change could be
       faster for *really long* strings due to cache reuse, but until that
       proves to be a bottleneck I'll go with the simpler solution.

       Not implementing through uppercase(Containers::String) as the call stack
       is deep enough already and we don't need the extra checks there. */
    Containers::String out{string};
    uppercaseInPlace(out);
    return out;
}

Containers::String uppercase(Containers::String string) {
    /* In the rare scenario where we'd get a non-owned string (such as
       String::nullTerminatedView() passed right into the function), make it
       owned first. Usually it'll get copied however, which already makes it
       owned. */
    if(!string.isSmall() && string.deleter()) string = Containers::String{string};

    uppercaseInPlace(string);
    return string;
}

std::string uppercase(std::string string) {
    uppercaseInPlace(string);
    return string;
}

Containers::String replaceFirst(const Containers::StringView string, const Containers::StringView search, const Containers::StringView replace) {
    /* Handle also the case when the search string is empty -- find() returns
       (empty) begin in that case and we just prepend the replace string */
    const Containers::StringView found = string.find(search);
    if(!search || found) {
        Containers::String output{NoInit, string.size() + replace.size() - found.size()};
        const std::size_t begin = found.begin() - string.begin();
        /* Not using Utility::copy() to avoid dependency on Algorithms.h */
        /** @todo might want to have some assign() API on the String itself? */
        std::memcpy(output.data(), string.data(), begin);
        std::memcpy(output.data() + begin, replace.data(), replace.size());
        const std::size_t end = begin + search.size();
        std::memcpy(output.data() + begin + replace.size(), string.data() + end, string.size() - end);
        return output;
    }

    /** @todo would it make sense to have an overload that takes a String as an
        input to avoid a copy here? it'd however cause an extra unused copy for
        the (more common) case when the substring is actually found */
    return string;
}

Containers::String replaceAll(Containers::StringView string, const Containers::StringView search, const Containers::StringView replace) {
    CORRADE_ASSERT(!search.isEmpty(),
        "Utility::String::replaceAll(): empty search string would cause an infinite loop", {});
    Containers::Array<char> output;
    while(const Containers::StringView found = string.find(search)) {
        arrayAppend(output, string.prefix(found.begin()));
        arrayAppend(output, replace);
        string = string.slice(found.end(), string.end());
    }
    arrayAppend(output, string);
    arrayAppend(output, '\0');
    const std::size_t size = output.size();
    /* This assumes that the growable array uses std::malloc() (which has to be
       std::free()'d later) in order to be able to std::realloc(). The deleter
       doesn't use the size argument so it should be fine to transfer it over
       to a String with the size excluding the null terminator. */
    void(*const deleter)(char*, std::size_t) = output.deleter();
    CORRADE_INTERNAL_ASSERT(deleter);
    return Containers::String{output.release(), size - 1, deleter};
}

Containers::String replaceAll(Containers::String string, const char search, const char replace) {
    /* If not even a single character is found, pass the argument through
       unchanged */
    const Containers::MutableStringView found = string.find(search);
    if(!found) return Utility::move(string);

    /* Convert the found pointer to an index to be able to replace even after a
       potential reallocation below */
    const std::size_t firstFoundPosition = found.begin() - string.begin();

    /* Otherwise, in the rare scenario where we'd get a non-owned string (such
       as String::nullTerminatedView() passed right into the function), make it
       owned first. Usually it'll get copied however, which already makes it
       owned. */
    if(!string.isSmall() && string.deleter()) string = Containers::String{string};

    /* Replace the already-found occurence and delegate the rest further */
    string[firstFoundPosition] = replace;
    replaceAllInPlace(string.exceptPrefix(firstFoundPosition + 1), search, replace);
    return string;
}

namespace Implementation {

namespace {

/* Has to be first because the Avx2 variant may delegate to it if
   CORRADE_ENABLE_SSE41 isn't defined due to compiler warts */
CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(replaceAllInPlaceCharacter)>::type replaceAllInPlaceCharacterImplementation(Cpu::ScalarT) {
    return [](char* const data, const std::size_t size, const char search, const char replace) {
        for(char* i = data, *end = data + size; i != end; ++i)
            if(*i == search) *i = replace;
    };
}

/* SIMD implementation of character replacement. All tricks inherited from
   stringFindCharacterImplementation(), in particular the unaligned preamble
   and postamble, as well as reducing the branching overhead by going through
   four vectors at a time. See its documentation for more background info. */
#ifdef CORRADE_ENABLE_SSE41
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SSE41 typename std::decay<decltype(replaceAllInPlaceCharacter)>::type replaceAllInPlaceCharacterImplementation(Cpu::Sse41T) {
  return [](char* const data, const std::size_t size, const char search, const char replace) CORRADE_ENABLE_SSE41 {
    /* If we have less than 16 bytes, do it the stupid way */
    /** @todo that this worked best for stringFindCharacterImplementation()
        doesn't mean it's the best variant here as well */
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

    /* Unconditionally process the first vector a slower, unaligned way. Do the
       replacement unconditionally because it's faster than checking first. */
    {
        const __m128i in = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(data), out);
    }

    /* Go four aligned vectors at a time. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. Similarly to the find()
       implementation, this reduces the branching overhead compared to
       branching on every vector, making it comparable to an unconditional
       replace with a character that occurs often, but significantly faster for
       characters that are rare. For comparison see StringTest.h and the
       replaceAllInPlaceCharacterImplementationSse41Unconditional() variant. */
    char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const __m128i inA = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 0);
        const __m128i inB = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 1);
        const __m128i inC = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 2);
        const __m128i inD = _mm_load_si128(reinterpret_cast<const __m128i*>(i) + 3);

        const __m128i eqA = _mm_cmpeq_epi8(inA, vsearch);
        const __m128i eqB = _mm_cmpeq_epi8(inB, vsearch);
        const __m128i eqC = _mm_cmpeq_epi8(inC, vsearch);
        const __m128i eqD = _mm_cmpeq_epi8(inD, vsearch);

        const __m128i or1 = _mm_or_si128(eqA, eqB);
        const __m128i or2 = _mm_or_si128(eqC, eqD);
        const __m128i or3 = _mm_or_si128(or1, or2);
        /* If any of the four vectors contained the character, replace all of
           them -- branching again on each would hurt the "common character"
           case */
        if(_mm_movemask_epi8(or3)) {
            const __m128i outA = _mm_blendv_epi8(inA, vreplace, eqA);
            const __m128i outB = _mm_blendv_epi8(inB, vreplace, eqB);
            const __m128i outC = _mm_blendv_epi8(inC, vreplace, eqC);
            const __m128i outD = _mm_blendv_epi8(inD, vreplace, eqD);

            _mm_store_si128(reinterpret_cast<__m128i*>(i) + 0, outA);
            _mm_store_si128(reinterpret_cast<__m128i*>(i) + 1, outB);
            _mm_store_si128(reinterpret_cast<__m128i*>(i) + 2, outC);
            _mm_store_si128(reinterpret_cast<__m128i*>(i) + 3, outD);
        }
    }

    /* Handle remaining less than four aligned vectors. Again do the
       replacement unconditionally. */
    for(; i + 16 <= end; i += 16) {
        const __m128i in = _mm_load_si128(reinterpret_cast<const __m128i*>(i));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_store_si128(reinterpret_cast<__m128i*>(i), out);
    }

    /* Handle remaining less than a vector in an unaligned way, again
       unconditionally and again overlapping bytes are no-op. */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 16 > end);
        i = end - 16;
        const __m128i in = _mm_loadu_si128(reinterpret_cast<const __m128i*>(i));
        const __m128i out = _mm_blendv_epi8(in, vreplace, _mm_cmpeq_epi8(in, vsearch));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(i), out);
    }
  };
}
#endif

#ifdef CORRADE_ENABLE_AVX2
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_AVX2 typename std::decay<decltype(replaceAllInPlaceCharacter)>::type replaceAllInPlaceCharacterImplementation(Cpu::Avx2T) {
  return [](char* const data, const std::size_t size, const char search, const char replace) CORRADE_ENABLE_AVX2 {
    /* If we have less than 32 bytes, fall back to the SSE variant */
    /** @todo deinline it here? any speed gains from rewriting using 128-bit
        AVX? or does the compiler do that automatically? */
    if(size < 32)
        return replaceAllInPlaceCharacterImplementation(Cpu::Sse41)(data, size, search, replace);

    const __m256i vsearch = _mm256_set1_epi8(search);
    const __m256i vreplace = _mm256_set1_epi8(replace);

    /* Calculate the next aligned position. If the pointer was already aligned,
       we'll go to the next aligned vector; if not, there will be an overlap
       and we'll process some bytes twice. */
    char* i = reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(data + 32) & ~0x1f);
    CORRADE_INTERNAL_DEBUG_ASSERT(i > data && reinterpret_cast<std::uintptr_t>(i) % 32 == 0);

    /* Unconditionally process the first vector a slower, unaligned way. Do the
       replacement unconditionally because it's faster than checking first. */
    {
        /* _mm256_lddqu_si256 is just an alias to _mm256_loadu_si256, no reason
           to use it: https://stackoverflow.com/a/47426790 */
        const __m256i in = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(data), out);
    }

    /* Go four aligned vectors at a time. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. Similarly to the SSE2
       implementation, this reduces the branching overhead compared to
       branching on every vector, making it comparable to an unconditional
       replace with a character that occurs often, but significantly faster for
       characters that are rare. For comparison see StringTest.h and the
       replaceAllInPlaceCharacterImplementationAvx2Unconditional() variant. */
    char* const end = data + size;
    for(; i + 4*32 <= end; i += 4*32) {
        const __m256i inA = _mm256_load_si256(reinterpret_cast<const __m256i*>(i) + 0);
        const __m256i inB = _mm256_load_si256(reinterpret_cast<const __m256i*>(i) + 1);
        const __m256i inC = _mm256_load_si256(reinterpret_cast<const __m256i*>(i) + 2);
        const __m256i inD = _mm256_load_si256(reinterpret_cast<const __m256i*>(i) + 3);

        const __m256i eqA = _mm256_cmpeq_epi8(inA, vsearch);
        const __m256i eqB = _mm256_cmpeq_epi8(inB, vsearch);
        const __m256i eqC = _mm256_cmpeq_epi8(inC, vsearch);
        const __m256i eqD = _mm256_cmpeq_epi8(inD, vsearch);

        const __m256i or1 = _mm256_or_si256(eqA, eqB);
        const __m256i or2 = _mm256_or_si256(eqC, eqD);
        const __m256i or3 = _mm256_or_si256(or1, or2);
        /* If any of the four vectors contained the character, replace all of
           them -- branching again on each would hurt the "common character"
           case */
        if(_mm256_movemask_epi8(or3)) {
            const __m256i outA = _mm256_blendv_epi8(inA, vreplace, eqA);
            const __m256i outB = _mm256_blendv_epi8(inB, vreplace, eqB);
            const __m256i outC = _mm256_blendv_epi8(inC, vreplace, eqC);
            const __m256i outD = _mm256_blendv_epi8(inD, vreplace, eqD);

            _mm256_store_si256(reinterpret_cast<__m256i*>(i) + 0, outA);
            _mm256_store_si256(reinterpret_cast<__m256i*>(i) + 1, outB);
            _mm256_store_si256(reinterpret_cast<__m256i*>(i) + 2, outC);
            _mm256_store_si256(reinterpret_cast<__m256i*>(i) + 3, outD);
        }
    }

    /* Handle remaining less than four aligned vectors. Again do the
       replacement unconditionally. */
    for(; i + 32 <= end; i += 32) {
        const __m256i in = _mm256_load_si256(reinterpret_cast<const __m256i*>(i));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_store_si256(reinterpret_cast<__m256i*>(i), out);
    }

    /* Handle remaining less than a vector in an unaligned way, again
       unconditionally and again overlapping bytes are no-op. */
    if(i < end) {
        CORRADE_INTERNAL_DEBUG_ASSERT(i + 32 > end);
        i = end - 32;
        /* _mm256_lddqu_si256 is just an alias to _mm256_loadu_si256, no reason
           to use it: https://stackoverflow.com/a/47426790 */
        const __m256i in = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(i));
        const __m256i out = _mm256_blendv_epi8(in, vreplace, _mm256_cmpeq_epi8(in, vsearch));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(i), out);
    }
  };
}
#endif

/* Just a direct translation of the SSE4.1 code */
#ifdef CORRADE_ENABLE_SIMD128
CORRADE_UTILITY_CPU_MAYBE_UNUSED CORRADE_ENABLE_SIMD128 typename std::decay<decltype(replaceAllInPlaceCharacter)>::type replaceAllInPlaceCharacterImplementation(Cpu::Simd128T) {
  return [](char* const data, const std::size_t size, const char search, const char replace) CORRADE_ENABLE_SIMD128 {
    /* If we have less than 16 bytes, do it the stupid way */
    /** @todo that this worked best for stringFindCharacterImplementation()
        doesn't mean it's the best variant here as well; also check the
        pre-/post-increment differences between x86 and WASM like in the find
        variant */
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

    /* Go four aligned vectors at a time. Bytes overlapping with the previous
       unaligned load will be processed twice, but as everything is already
       replaced there, it'll be a no-op for those. Similarly to the SSE2 / AVX2
       implementation, this reduces the branching overhead compared to
       branching on every vector, making it comparable to an unconditional
       replace with a character that occurs often, but significantly faster for
       characters that are rare, on x86 at least. Elsewhere it *can* be slower
       due to the slow movemask emulation. See stringFindCharacterImplementation()
       for details on wasm_i8x16_bitmask() alternatives, compare to the
       replaceAllInPlaceCharacterImplementationSimd128Unconditional() variant
       in StringTest.h. */
    char* const end = data + size;
    for(; i + 4*16 <= end; i += 4*16) {
        const v128_t inA = wasm_v128_load(reinterpret_cast<const v128_t*>(i) + 0);
        const v128_t inB = wasm_v128_load(reinterpret_cast<const v128_t*>(i) + 1);
        const v128_t inC = wasm_v128_load(reinterpret_cast<const v128_t*>(i) + 2);
        const v128_t inD = wasm_v128_load(reinterpret_cast<const v128_t*>(i) + 3);

        const v128_t eqA = wasm_i8x16_eq(inA, vsearch);
        const v128_t eqB = wasm_i8x16_eq(inB, vsearch);
        const v128_t eqC = wasm_i8x16_eq(inC, vsearch);
        const v128_t eqD = wasm_i8x16_eq(inD, vsearch);

        const v128_t or1 = wasm_v128_or(eqA, eqB);
        const v128_t or2 = wasm_v128_or(eqC, eqD);
        const v128_t or3 = wasm_v128_or(or1, or2);
        /* If any of the four vectors contained the character, replace all of
           them -- branching again on each would hurt the "common character"
           case */
        if(wasm_i8x16_bitmask(or3)) {
            const v128_t outA = wasm_v128_bitselect(vreplace, inA, eqA);
            const v128_t outB = wasm_v128_bitselect(vreplace, inB, eqB);
            const v128_t outC = wasm_v128_bitselect(vreplace, inC, eqC);
            const v128_t outD = wasm_v128_bitselect(vreplace, inD, eqD);

            wasm_v128_store(reinterpret_cast<v128_t*>(i) + 0, outA);
            wasm_v128_store(reinterpret_cast<v128_t*>(i) + 1, outB);
            wasm_v128_store(reinterpret_cast<v128_t*>(i) + 2, outC);
            wasm_v128_store(reinterpret_cast<v128_t*>(i) + 3, outD);
        }
    }

    /* Handle remaining less than four aligned vectors. Again do the
       replacement unconditionally. */
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
  };
}
#endif

}

CORRADE_UTILITY_CPU_DISPATCHER_BASE(replaceAllInPlaceCharacterImplementation)
CORRADE_UTILITY_CPU_DISPATCHED(replaceAllInPlaceCharacterImplementation, void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(replaceAllInPlaceCharacter)(char* data, std::size_t size, char search, char replace))({
    return replaceAllInPlaceCharacterImplementation(Cpu::DefaultBase)(data, size, search, replace);
})

}

Containers::Optional<Containers::Array<std::uint32_t>> parseNumberSequence(const Containers::StringView string, const std::uint32_t min, const std::uint32_t max) {
    Containers::Array<std::uint32_t> out;

    bool hasNumber = false; /* whether we have a number already */
    std::uint32_t number = 0; /* value of that number */
    bool overflow = false; /* if we've overflown the 32 bits during parsing */
    std::uint32_t rangeStart = ~0u; /* if not ~0u, we're in a range */

    /* Going through one iteration more in order to handle the end-of-string
       condition in the same place as delimiters */
    for(std::size_t i = 0; i <= string.size(); ++i) {
        const char c = i == string.size() ? 0 : string[i];

        /* End of string or a delimiter */
        if(i == string.size() || c == ',' || c == ';' || c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r' || c == '\n') {
            /* If anything has overflown, ignore this piece completely and
               reset the bit again */
            if(overflow) {
                overflow = false;

            /* If we are in a range, fill it. Clamp range end to the soecified
               bounds as well, worst case the loop will not iterate at all. */
            } else if(rangeStart != ~std::uint32_t{}) {
                const std::uint32_t rangeEnd = hasNumber && number < max ? number + 1 : max;

                /* If the range is non-empty, add an uninitialized sequence to
                   the array and then fill it. Should be vastly more efficient
                   for large ranges than arrayAppend() in a loop. */
                if(rangeEnd > rangeStart)
                    for(std::uint32_t& j: arrayAppend(out, NoInit, rangeEnd - rangeStart))
                        j = rangeStart++;
                rangeStart = ~std::uint32_t{};

            /* Otherwise, if we have just one number, save it to the output if
               it's in bounds.*/
            } else if(hasNumber && number >= min && number < max) {
                arrayAppend(out, number);

            /* If we have nothing, there was multiple delimiters after each
               other. */
            }

            hasNumber = false;
            number = 0;

        /* Number */
        } else if(c >= '0' && c <= '9') {
            hasNumber = true;

            /* If there's an overflow, remember that to discard the whole piece
               later. Apparently I can't actually test for overflow with
               `number < next` (huh? why did I think it would work?) so going
               with a bigger type for the multiplication. Answers at
               https://stackoverflow.com/a/1815371 are mostly just crap, using
               a *division* to test if a multiplication overflowed?! */
            const std::uint64_t next = std::uint64_t{number}*10 + (c - '0');
            if(next > ~std::uint32_t{}) overflow = true;

            number = next;

        /* Range specification */
        } else if(c == '-') {
            /* If we have a number, remember it as a range start if it's in
               bounds. Otherwise use the min value. */
            rangeStart = hasNumber && number >= min ? number : min;

            hasNumber = false;
            number = 0;

        /* Something weird, bail */
        } else {
            /** @todo sanitize when Debug::chr / Debug::str is done */
            Error{} << "Utility::parseNumberSequence(): unrecognized character" << Containers::StringView{&c, 1} << "in" << string;
            return {};
        }
    }

    /* GCC 4.8 decases when seeing just `return out` here */
    return Containers::optional(Utility::move(out));
}

}}}
