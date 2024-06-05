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
#ifdef CORRADE_ENABLE_SSE2
#include "Corrade/Utility/IntrinsicsSse2.h"
#endif
#ifdef CORRADE_ENABLE_AVX2
#include "Corrade/Utility/IntrinsicsAvx.h"
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

CORRADE_UTILITY_CPU_MAYBE_UNUSED typename std::decay<decltype(replaceAllInPlaceCharacter)>::type replaceAllInPlaceCharacterImplementation(Cpu::ScalarT) {
    return [](char* const data, const std::size_t size, const char search, const char replace) {
        for(char* i = data, *end = data + size; i != end; ++i)
            if(*i == search) *i = replace;
    };
}

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
