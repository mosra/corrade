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

#include <cctype> /* std::ctype */
#include <algorithm> /* std::transform() */
#include <locale> /* std::locale::classic() */

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Test/cpuVariantHelpers.h"
#ifdef CORRADE_ENABLE_SSE2
#include "Corrade/Utility/IntrinsicsSse2.h"
#endif

#ifdef CORRADE_ENABLE_NEON
#include <arm_neon.h>
#endif

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringBenchmark: TestSuite::Tester {
    explicit StringBenchmark();

    void captureImplementations();
    void restoreImplementations();

    void lowercase();
    #ifdef CORRADE_ENABLE_SSE2
    void lowercaseSse2TwoCompares();
    #endif
    #ifdef CORRADE_ENABLE_NEON
    void lowercaseNeon();
    #endif
    void lowercaseBranchless();
    void lowercaseBranchless32();
    void lowercaseNaive();
    void lowercaseStl();
    void lowercaseStlFacet();

    void uppercase();
    void uppercaseBranchless();
    void uppercaseBranchless32();
    void uppercaseNaive();
    void uppercaseStl();
    void uppercaseStlFacet();

    void lowercaseSmall();
    /* Comparing the "small" case only to the scalar variant that was fastest
       of the above, not all */
    void lowercaseSmallBranchless();

    void uppercaseSmall();
    /* Comparing the "small" case only to the scalar variant that was fastest
       of the above, not all */
    void uppercaseSmallBranchless();

    private:
        Containers::Optional<Containers::String> _text;
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(String::Implementation::lowercaseInPlace) _lowercaseInPlaceImplementation;
        decltype(String::Implementation::uppercaseInPlace) _uppercaseInPlaceImplementation;
        #endif
};

using namespace Containers::Literals;

const struct {
    Cpu::Features features;
} LowercaseUppercaseData[]{
    {Cpu::Scalar},
    #ifdef CORRADE_ENABLE_SSE2
    {Cpu::Sse2},
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    {Cpu::Avx2},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t size;
} LowercaseUppercaseSmallData[]{
    {Cpu::Scalar, 15},
    #ifdef CORRADE_ENABLE_SSE2
    /* This should fall back to the scalar case */
    {Cpu::Sse2, 15},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Sse2, 16},
    /* This should do two overlapping vector operations */
    {Cpu::Sse2, 17},
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    /* This should fall back to the SSE2 and then the scalar case */
    {Cpu::Avx2, 15},
    /* This should fall back to the SSE2 case */
    {Cpu::Avx2, 31},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Avx2, 32},
    /* This should do two overlapping vector operations */
    {Cpu::Avx2, 33},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    /* This should fall back to the scalar case */
    {Cpu::Simd128, 15},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Simd128, 16},
    /* This should do two overlapping vector operations */
    {Cpu::Simd128, 17},
    #endif
};

StringBenchmark::StringBenchmark() {
    addInstancedBenchmarks({&StringBenchmark::lowercase}, 10,
        cpuVariantCount(LowercaseUppercaseData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({
        #ifdef CORRADE_ENABLE_SSE2
        &StringBenchmark::lowercaseSse2TwoCompares,
        #endif
        #ifdef CORRADE_ENABLE_NEON
        &StringBenchmark::lowercaseNeon,
        #endif
        &StringBenchmark::lowercaseBranchless,
        &StringBenchmark::lowercaseBranchless32,
        &StringBenchmark::lowercaseNaive,
        &StringBenchmark::lowercaseStl,
        &StringBenchmark::lowercaseStlFacet}, 10);

    addInstancedBenchmarks({&StringBenchmark::uppercase}, 10,
        cpuVariantCount(LowercaseUppercaseData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::uppercaseBranchless,
                   &StringBenchmark::uppercaseBranchless32,
                   &StringBenchmark::uppercaseNaive,
                   &StringBenchmark::uppercaseStl,
                   &StringBenchmark::uppercaseStlFacet}, 10);

    addInstancedBenchmarks({&StringBenchmark::lowercaseSmall}, 10,
        cpuVariantCount(LowercaseUppercaseSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::lowercaseSmallBranchless}, 10);

    addInstancedBenchmarks({&StringBenchmark::uppercaseSmall}, 10,
        cpuVariantCount(LowercaseUppercaseSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::uppercaseSmallBranchless}, 10);

    _text = Path::readString(Path::join(CONTAINERS_STRING_TEST_DIR, "lorem-ipsum.txt"));
}

void StringBenchmark::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    _lowercaseInPlaceImplementation = String::Implementation::lowercaseInPlace;
    _uppercaseInPlaceImplementation = String::Implementation::uppercaseInPlace;
    #endif
}

void StringBenchmark::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    String::Implementation::lowercaseInPlace = _lowercaseInPlaceImplementation;
    String::Implementation::uppercaseInPlace = _uppercaseInPlaceImplementation;
    #endif
}

void StringBenchmark::lowercase() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = String::Implementation::lowercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        String::lowercaseInPlace(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

#ifdef CORRADE_ENABLE_SSE2
/* An "obvious" variant of the actual SSE2 implementation in
   String::lowercaseInPlace(). It's the same count of instructions but runs
   considerably slower for some reason -- maybe because the two compares, or
   all the bit ops can't be pipelined, compared to bit ops + arithmetic + one
   compare in the other? I know too little to be sure what's going on so this
   just records the state. */
CORRADE_ENABLE_SSE2 CORRADE_NEVER_INLINE void lowercaseInPlaceSse2TwoCompares(Containers::MutableStringView string) {
    const std::size_t size = string.size();
    char* const data = string.data();
    char* const end = data + size;

    /* Omitting the less-than-a-vector fallback here */
    CORRADE_INTERNAL_DEBUG_ASSERT(size >= 16);

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

void StringBenchmark::lowercaseSse2TwoCompares() {
    if(!(Cpu::runtimeFeatures() >= Cpu::Sse2))
        CORRADE_SKIP(Cpu::Sse2 << "not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text;

    std::size_t i = 0;
    CORRADE_BENCHMARK(1)
        lowercaseInPlaceSse2TwoCompares(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}
#endif

#ifdef CORRADE_ENABLE_NEON
/* Trivial port of the SSE2 code to NEON, with the same "aligned load/store is
   the same as unaligned" simplification as the WASM code. Included just to
   have baseline comparison to the scalar code because the compiler seems to
   autovectorize better than what this function does. */
CORRADE_ENABLE_NEON void lowercaseInPlaceNeon(Containers::MutableStringView string) {
    const std::size_t size = string.size();
    char* const data = string.data();
    char* const end = data + size;

    /* Omitting the less-than-a-vector fallback here */
    CORRADE_INTERNAL_DEBUG_ASSERT(size >= 16);

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

void StringBenchmark::lowercaseNeon() {
    if(!(Cpu::runtimeFeatures() >= Cpu::Neon))
        CORRADE_SKIP(Cpu::Neon << "not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text;

    std::size_t i = 0;
    CORRADE_BENCHMARK(1)
        lowercaseInPlaceNeon(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}
#endif

CORRADE_NEVER_INLINE void lowercaseInPlaceBranchless(Containers::MutableStringView string) {
    for(char& c: string)
        c += (std::uint8_t(c - 'A') < 26) << 5;
}

void StringBenchmark::lowercaseBranchless() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        lowercaseInPlaceBranchless(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

/* Compared to lowercaseInPlaceBranchless() above it has `unsigned` instead of
   `std::uint8_t`, making it almost 8x slower because it seems to prevent
   autovectorization. */
CORRADE_NEVER_INLINE void lowercaseInPlaceBranchless32(Containers::MutableStringView string) {
    for(char& c: string)
        c += (unsigned(c - 'A') < 26) << 5;
}

void StringBenchmark::lowercaseBranchless32() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        lowercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

/* This is the original implementation that used to be in
   String::lowercaseInPlace() */
CORRADE_NEVER_INLINE void lowercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string)
        if(c >= 'A' && c <= 'Z') c |= 0x20;
}

void StringBenchmark::lowercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        lowercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::tolower));
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).tolower(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::uppercase() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::uppercaseInPlace = String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        String::uppercaseInPlace(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

CORRADE_NEVER_INLINE void uppercaseInPlaceBranchless(Containers::MutableStringView string) {
    for(char& c: string)
        c -= (std::uint8_t(c - 'a') < 26) << 5;
}

void StringBenchmark::uppercaseBranchless() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        uppercaseInPlaceBranchless(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

/* Compared to uppercaseInPlaceBranchless() above it has `unsigned` instead of
   `std::uint8_t`, making it almost 8x slower because it seems to prevent
   autovectorization. */
CORRADE_NEVER_INLINE void uppercaseInPlaceBranchless32(Containers::MutableStringView string) {
    for(char& c: string)
        c -= (unsigned(c - 'a') < 26) << 5;
}

void StringBenchmark::uppercaseBranchless32() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        uppercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

/* This is the original implementation that used to be in
   String::uppercaseInPlace() */
CORRADE_NEVER_INLINE void uppercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string)
        if(c >= 'a' && c <= 'z') c &= ~0x20;
}

void StringBenchmark::uppercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    std::size_t i = 0;
    CORRADE_BENCHMARK(10)
        uppercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::toupper));
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*10;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(10) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).toupper(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::lowercaseSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseSmallData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = String::Implementation::lowercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseSmallData);
    #endif
    setTestCaseDescription(Utility::format("{}, {} bytes", Utility::Test::cpuVariantName(data), data.size));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Stripping to a whole number of blocks for simpler code */
    CORRADE_VERIFY(_text);
    std::size_t repeatCount = _text->size()/data.size;
    Containers::String string = _text->prefix(data.size*repeatCount);

    std::size_t i = 0;
    CORRADE_BENCHMARK(repeatCount)
        String::lowercaseInPlace(string.sliceSize((i++)*data.size, data.size));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseSmallBranchless() {
    /* Stripping to a whole number of blocks for simpler code */
    CORRADE_VERIFY(_text);
    std::size_t repeatCount = _text->size()/15;
    Containers::String string = _text->prefix(15*repeatCount);

    std::size_t i = 0;
    CORRADE_BENCHMARK(repeatCount)
        lowercaseInPlaceBranchless(string.sliceSize((i++)*15, 15));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::uppercaseSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseSmallData[testCaseInstanceId()];
    String::Implementation::uppercaseInPlace = String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseSmallData);
    #endif
    setTestCaseDescription(Utility::format("{}, {} bytes", Utility::Test::cpuVariantName(data), data.size));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Stripping to a whole number of blocks for simpler code */
    CORRADE_VERIFY(_text);
    std::size_t repeatCount = _text->size()/data.size;
    Containers::String string = _text->prefix(data.size*repeatCount);

    std::size_t i = 0;
    CORRADE_BENCHMARK(repeatCount)
        String::uppercaseInPlace(string.sliceSize((i++)*data.size, data.size));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseSmallBranchless() {
    /* Stripping to a whole number of blocks for simpler code */
    CORRADE_VERIFY(_text);
    std::size_t repeatCount = _text->size()/15;
    Containers::String string = _text->prefix(15*repeatCount);

    std::size_t i = 0;
    CORRADE_BENCHMARK(repeatCount)
        uppercaseInPlaceBranchless(string.sliceSize((i++)*15, 15));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringBenchmark)
