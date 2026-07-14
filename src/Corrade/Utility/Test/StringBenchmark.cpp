/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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
#include <cstring> /* std::memchr */
#include <locale> /* std::locale::classic() */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StringStl.h" /* for std::istringstream::str() */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Test/cpuVariantHelpers.h"
#include "Corrade/Utility/Test/StringTest.h"

/* On GCC 4.8 these have to be included after StringTest.h which includes the
   AVX intrinsics headers, otherwise __m256i and other types don't get defined
   for some reason */
#include <algorithm> /* std::transform(), std::replace() */
#include <random>

/* That a compiler advertises C++17 support doesn't necessarily mean it uses a
   STL implementation that implements all its features (such as using a new
   Clang with an old libstdc++). So check for presence of the header as
   well. */
#ifdef CORRADE_TARGET_CXX17
#ifdef __has_include
#if __has_include(<charconv>)
#include <charconv> /* std::from_chars() */
#define CAN_USE_STL_FROM_CHARS
/* The floating-point std::from_chars() variants are available only since
   libstdc++ 11.1 and libc++ 20. With the MSVC STL it's available since MSVC
   2017, which is also the first version that advertises C++17 support, so
   there it doesn't need any special handling.
    https://github.com/gcc-mirror/gcc/commit/932fbc868ad429167a3d4d5625aa9d6dc0b4506b
    https://github.com/llvm/llvm-project/commit/6c4267fb1779bc5550bb413f33250f9365acfbc6 */
#if (defined(CORRADE_TARGET_LIBSTDCXX) && _GLIBCXX_RELEASE >= 11) || (defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION >= 200000) || defined(CORRADE_TARGET_DINKUMWARE)
#define CAN_USE_STL_FROM_CHARS_FLOAT
#endif
#endif
#endif
#endif

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringBenchmark: TestSuite::Tester {
    explicit StringBenchmark();

    void captureImplementations();
    void restoreImplementations();

    template<char character> void commonPrefix();
    template<char character> void commonPrefixNaive();
    template<char character> void commonPrefixStl();

    void commonPrefixCommonSmall();
    void commonPrefixCommonSmallStl();
    void commonPrefixRareDifferentlyAligned();
    void commonPrefixRareMemcmp();

    void lowercase();
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

    template<char character> void replaceAllInPlaceCharacter();
    template<char character> void replaceAllInPlaceCharacterNaive();
    template<char character> void replaceAllInPlaceCharacterMemchrLoop();
    template<char character> void replaceAllInPlaceCharacterStl();

    void replaceAllInPlaceCharacterCommonSmall();
    void replaceAllInPlaceCharacterCommonSmallStl();

    /* Benchmarks just the unsigned variants, as signed is just a trivial
       (and thus constant-time) transformation on top */
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimal();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimalNaive();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimalStl();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimalStlNonNullTerminated();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimalStlStream();
    #ifdef CAN_USE_STL_FROM_CHARS
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseDecimalStlFromChars();
    #endif
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimal();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimalNaive();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimalStl();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimalStlNonNullTerminated();
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimalStlStream();
    #ifdef CAN_USE_STL_FROM_CHARS
    template<Containers::Array<Containers::String> StringBenchmark::*numbers> void parseHexadecimalStlFromChars();
    #endif
    template<class T> void parseFloat();
    template<class T, T(*parse)(const char*, char**)> void parseFloatStl();
    template<class T, T(*parse)(const char*, char**)> void parseFloatStlNonNullTerminated();
    template<class T> void parseFloatStlStream();
    #ifdef CAN_USE_STL_FROM_CHARS_FLOAT
    template<class T> void parseFloatStlFromChars();
    #endif

    private:
        Containers::Optional<Containers::String> _text;
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(String::Implementation::commonPrefix) _commonPrefixImplementation;
        decltype(String::Implementation::lowercaseInPlace) _lowercaseInPlaceImplementation;
        decltype(String::Implementation::uppercaseInPlace) _uppercaseInPlaceImplementation;
        decltype(String::Implementation::replaceAllInPlaceCharacter) _replaceAllInPlaceCharacterImplementation;
        #endif

        Containers::Array<Containers::String> _numbers,
            _numbersLeadingZeros,
            _numbersHex,
            _numbersHexLeadingZeros;
        std::uint64_t _numbersSum;
        double _numbersSumFloat;
};

using namespace Containers::Literals;

template<char> struct CharacterTraits;
template<> struct CharacterTraits<' '> {
    enum: std::size_t { Count = 500 };
    static const char* name() { return "common"; }
};
template<> struct CharacterTraits<'\n'> {
    enum: std::size_t { Count = 9 };
    static const char* name() { return "rare"; }
};

const struct {
    Cpu::Features features;
} CommonPrefixData[]{
    {Cpu::Scalar},
    #if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Sse2|Cpu::Bmi1},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Avx2|Cpu::Bmi1},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t size;
} CommonPrefixSmallData[]{
    {Cpu::Scalar, 15},
    #if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
    /* This should fall back to the scalar case */
    {Cpu::Sse2|Cpu::Bmi1, 15},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Sse2|Cpu::Bmi1, 16},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Sse2|Cpu::Bmi1, 17},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    /* This should fall back to the SSE2 and then the scalar case */
    {Cpu::Avx2|Cpu::Bmi1, 15},
    /* This should fall back to the SSE2 case */
    {Cpu::Avx2|Cpu::Bmi1, 31},
    /* This should do one vector operation, skipping the four-vector block and
       the postamble */
    {Cpu::Avx2|Cpu::Bmi1, 32},
    /* This should do two overlapping vector operations, skipping the
       four-vector block and the single-vector aligned postamble */
    {Cpu::Avx2|Cpu::Bmi1, 33},
    #endif
};

const struct {
    Cpu::Features features;
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*function)(char*, std::size_t);
} LowercaseData[]{
    {Cpu::Scalar, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE2
    {Cpu::Sse2, "overflow + compare (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse2, "two compares",
        lowercaseInPlaceImplementationSse2TwoCompares},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    {Cpu::Avx2, nullptr, nullptr},
    #endif
    #if defined(CORRADE_ENABLE_NEON) && defined(CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH)
    {Cpu::Neon, "trivial port (unused)",
        lowercaseInPlaceImplementationNeon},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128, nullptr, nullptr},
    #endif
};

const struct {
    Cpu::Features features;
} UppercaseData[]{
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
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*function)(char*, std::size_t);
} LowercaseSmallData[]{
    {Cpu::Scalar, 15, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE2
    /* This should fall back to the scalar case */
    {Cpu::Sse2, 15, "overflow + compare (default)", nullptr},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Sse2, 16, "overflow + compare (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse2, 16, "two compares",
        lowercaseInPlaceImplementationSse2TwoCompares},
    #endif
    /* This should do two overlapping vector operations */
    {Cpu::Sse2, 17, "overflow + compare (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse2, 17, "two compares",
        lowercaseInPlaceImplementationSse2TwoCompares},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    /* This should fall back to the SSE2 and then the scalar case */
    {Cpu::Avx2, 15, nullptr, nullptr},
    /* This should fall back to the SSE2 case */
    {Cpu::Avx2, 31, nullptr, nullptr},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Avx2, 32, nullptr, nullptr},
    /* This should do two overlapping vector operations */
    {Cpu::Avx2, 33, nullptr, nullptr},
    #endif
    #if defined(CORRADE_ENABLE_NEON) && defined(CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH)
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Neon, 16, "trivial port (unused)",
        lowercaseInPlaceImplementationNeon},
    /* This should do two overlapping vector operations */
    {Cpu::Neon, 17, "trivial port (unused)",
        lowercaseInPlaceImplementationNeon},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    /* This should fall back to the scalar case */
    {Cpu::Simd128, 15, nullptr, nullptr},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Simd128, 16, nullptr, nullptr},
    /* This should do two overlapping vector operations */
    {Cpu::Simd128, 17, nullptr, nullptr},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t size;
} UppercaseSmallData[]{
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

const struct {
    Cpu::Features features;
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*function)(char*, std::size_t, char, char);
} ReplaceAllInPlaceCharacterData[]{
    {Cpu::Scalar, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE41
    {Cpu::Sse41, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse41, "unconditional replace",
        replaceAllInPlaceCharacterImplementationSse41Unconditional},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    {Cpu::Avx2, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Avx2, "unconditional replace",
        replaceAllInPlaceCharacterImplementationAvx2Unconditional},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Simd128, "unconditional replace",
        replaceAllInPlaceCharacterImplementationSimd128Unconditional},
    #endif
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t size;
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*function)(char*, std::size_t, char, char);
} ReplaceAllInPlaceCharacterSmallData[]{
    {Cpu::Scalar, 15, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE41
    /* This should fall back to the scalar case */
    {Cpu::Sse41, 15, nullptr, nullptr},
    /* This should do one unaligned vector operation, skipping the rest */
    {Cpu::Sse41, 16, nullptr, nullptr},
    /* This should do two overlapping unaligned vector operations */
    {Cpu::Sse41, 17, nullptr, nullptr},
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    /* This should fall back to the SSE2 and then the scalar case */
    {Cpu::Avx2, 15, nullptr, nullptr},
    /* This should fall back to the SSE2 case */
    {Cpu::Avx2, 31, nullptr, nullptr},
    /* This should do one vector operation, skipping the postamble */
    {Cpu::Avx2, 32, nullptr, nullptr},
    /* This should do two overlapping vector operations */
    {Cpu::Avx2, 33, nullptr, nullptr},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    /* This should fall back to the scalar case */
    {Cpu::Simd128, 15, nullptr, nullptr},
    /* This should do one unaligned vector operation, skipping the rest */
    {Cpu::Simd128, 16, nullptr, nullptr},
    /* This should do two overlapping unaligned vector operations */
    {Cpu::Simd128, 17, nullptr, nullptr},
    #endif
};

StringBenchmark::StringBenchmark() {
    addInstancedBenchmarks({&StringBenchmark::commonPrefix<' '>}, 100,
        cpuVariantCount(CommonPrefixData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks<StringBenchmark>({
        &StringBenchmark::commonPrefixNaive<' '>,
        &StringBenchmark::commonPrefixStl<' '>}, 20);

    addInstancedBenchmarks({&StringBenchmark::commonPrefixCommonSmall}, 100,
        cpuVariantCount(CommonPrefixSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::commonPrefixCommonSmallStl}, 20);

    addInstancedBenchmarks({&StringBenchmark::commonPrefix<'\n'>,
                            &StringBenchmark::commonPrefixRareDifferentlyAligned}, 100,
        cpuVariantCount(CommonPrefixData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks<StringBenchmark>({
        &StringBenchmark::commonPrefixRareMemcmp,

        &StringBenchmark::commonPrefixNaive<'\n'>,
        &StringBenchmark::commonPrefixStl<'\n'>}, 100);

    addInstancedBenchmarks({&StringBenchmark::lowercase}, 100,
        cpuVariantCount(LowercaseData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::lowercaseBranchless,
                   &StringBenchmark::lowercaseBranchless32,
                   &StringBenchmark::lowercaseNaive,
                   &StringBenchmark::lowercaseStl,
                   &StringBenchmark::lowercaseStlFacet}, 20);

    addInstancedBenchmarks({&StringBenchmark::uppercase}, 100,
        cpuVariantCount(UppercaseData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::uppercaseBranchless,
                   &StringBenchmark::uppercaseBranchless32,
                   &StringBenchmark::uppercaseNaive,
                   &StringBenchmark::uppercaseStl,
                   &StringBenchmark::uppercaseStlFacet}, 20);

    addInstancedBenchmarks({&StringBenchmark::lowercaseSmall}, 100,
        cpuVariantCount(LowercaseSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::lowercaseSmallBranchless}, 20);

    addInstancedBenchmarks({&StringBenchmark::uppercaseSmall}, 100,
        cpuVariantCount(UppercaseSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::uppercaseSmallBranchless}, 20);

    addInstancedBenchmarks({&StringBenchmark::replaceAllInPlaceCharacter<' '>}, 100,
        cpuVariantCount(ReplaceAllInPlaceCharacterData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks<StringBenchmark>({
        &StringBenchmark::replaceAllInPlaceCharacterNaive<' '>,
        &StringBenchmark::replaceAllInPlaceCharacterMemchrLoop<' '>,
        &StringBenchmark::replaceAllInPlaceCharacterStl<' '>}, 20);

    addInstancedBenchmarks({&StringBenchmark::replaceAllInPlaceCharacterCommonSmall}, 100,
        cpuVariantCount(ReplaceAllInPlaceCharacterSmallData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks({&StringBenchmark::replaceAllInPlaceCharacterCommonSmallStl}, 20);

    addInstancedBenchmarks({&StringBenchmark::replaceAllInPlaceCharacter<'\n'>}, 100,
        cpuVariantCount(ReplaceAllInPlaceCharacterData),
        &StringBenchmark::captureImplementations,
        &StringBenchmark::restoreImplementations);

    addBenchmarks<StringBenchmark>({
        &StringBenchmark::replaceAllInPlaceCharacterNaive<'\n'>,
        &StringBenchmark::replaceAllInPlaceCharacterMemchrLoop<'\n'>,
        &StringBenchmark::replaceAllInPlaceCharacterStl<'\n'>}, 20);

    addBenchmarks<StringBenchmark>({
        &StringBenchmark::parseDecimal<&StringBenchmark::_numbers>,
        &StringBenchmark::parseDecimalNaive<&StringBenchmark::_numbers>,
        &StringBenchmark::parseDecimalStl<&StringBenchmark::_numbers>,
        &StringBenchmark::parseDecimalStlNonNullTerminated<&StringBenchmark::_numbers>,
        &StringBenchmark::parseDecimalStlStream<&StringBenchmark::_numbers>,
        #ifdef CAN_USE_STL_FROM_CHARS
        &StringBenchmark::parseDecimalStlFromChars<&StringBenchmark::_numbers>,
        #endif

        &StringBenchmark::parseDecimal<&StringBenchmark::_numbersLeadingZeros>,
        &StringBenchmark::parseDecimalNaive<&StringBenchmark::_numbersLeadingZeros>,
        &StringBenchmark::parseDecimalStl<&StringBenchmark::_numbersLeadingZeros>,
        &StringBenchmark::parseDecimalStlNonNullTerminated<&StringBenchmark::_numbersLeadingZeros>,
        &StringBenchmark::parseDecimalStlStream<&StringBenchmark::_numbersLeadingZeros>,
        #ifdef CAN_USE_STL_FROM_CHARS
        &StringBenchmark::parseDecimalStlFromChars<&StringBenchmark::_numbersLeadingZeros>,
        #endif

        &StringBenchmark::parseHexadecimal<&StringBenchmark::_numbersHex>,
        &StringBenchmark::parseHexadecimalNaive<&StringBenchmark::_numbersHex>,
        &StringBenchmark::parseHexadecimalStl<&StringBenchmark::_numbersHex>,
        &StringBenchmark::parseHexadecimalStlNonNullTerminated<&StringBenchmark::_numbersHex>,
        &StringBenchmark::parseHexadecimalStlStream<&StringBenchmark::_numbersHex>,
        #ifdef CAN_USE_STL_FROM_CHARS
        &StringBenchmark::parseHexadecimalStlFromChars<&StringBenchmark::_numbersHex>,
        #endif

        &StringBenchmark::parseHexadecimal<&StringBenchmark::_numbersHexLeadingZeros>,
        &StringBenchmark::parseHexadecimalNaive<&StringBenchmark::_numbersHexLeadingZeros>,
        &StringBenchmark::parseHexadecimalStl<&StringBenchmark::_numbersHexLeadingZeros>,
        &StringBenchmark::parseHexadecimalStlNonNullTerminated<&StringBenchmark::_numbersHexLeadingZeros>,
        &StringBenchmark::parseHexadecimalStlStream<&StringBenchmark::_numbersHexLeadingZeros>,
        #ifdef CAN_USE_STL_FROM_CHARS
        &StringBenchmark::parseHexadecimalStlFromChars<&StringBenchmark::_numbersHexLeadingZeros>,
        #endif

        &StringBenchmark::parseFloat<float>,
        &StringBenchmark::parseFloatStl<float, std::strtof>,
        &StringBenchmark::parseFloatStlNonNullTerminated<float, std::strtof>,
        &StringBenchmark::parseFloatStlStream<float>,
        #ifdef CAN_USE_STL_FROM_CHARS_FLOAT
        &StringBenchmark::parseFloatStlFromChars<float>,
        #endif

        &StringBenchmark::parseFloat<double>,
        &StringBenchmark::parseFloatStl<double, std::strtod>,
        &StringBenchmark::parseFloatStlNonNullTerminated<double, std::strtod>,
        &StringBenchmark::parseFloatStlStream<double>,
        #ifdef CAN_USE_STL_FROM_CHARS_FLOAT
        &StringBenchmark::parseFloatStlFromChars<double>,
        #endif
    }, 50);

    _text = Path::readString(Path::join(CONTAINERS_STRING_TEST_DIR, "lorem-ipsum.txt"));

    /* A list with a bunch of 64-bit numbers */
    {
        std::random_device rd;
        std::mt19937 gen{rd()};
        /* Anything between a single zero and the full 64-bit value to verify
           the general case including the constant overhead. Limiting the
           distibution to e.g. {~std::uint64_t{}/100, ~std::uint64_t{}} would
           put more emphasis on the actual number parsing and less on the
           constant overhead. The leading zeros case is for checking if there's
           any fast path for consuming these in the STL implementations like I
           have in mine. */
        std::uniform_int_distribution<std::uint64_t> distrib{0, ~std::uint64_t{}};
        const std::size_t count = 1000;
        _numbers = Containers::Array<Containers::String>{ValueInit, count};
        _numbersLeadingZeros = Containers::Array<Containers::String>{ValueInit, count};
        _numbersHex = Containers::Array<Containers::String>{ValueInit, count};
        _numbersHexLeadingZeros = Containers::Array<Containers::String>{ValueInit, count};
        _numbersSum = 0;
        _numbersSumFloat = 0.0;
        for(std::size_t i = 0; i != count; ++i) {
            std::uint64_t value = distrib(gen);
            _numbersSum += value;
            _numbersSumFloat += value;
            _numbers[i] = Utility::format("{}", value);
            _numbersLeadingZeros[i] = Utility::format("{:.40}", value);
            _numbersHex[i] = Utility::format("{:x}", value);
            _numbersHexLeadingZeros[i] = Utility::format("{:.32x}", value);
        }
        CORRADE_INTERNAL_ASSERT(_numbersSum);
        CORRADE_INTERNAL_ASSERT(!std::isinf(float(_numbersSumFloat)));
    }
}

void StringBenchmark::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    _commonPrefixImplementation = String::Implementation::commonPrefix;
    _lowercaseInPlaceImplementation = String::Implementation::lowercaseInPlace;
    _uppercaseInPlaceImplementation = String::Implementation::uppercaseInPlace;
    _replaceAllInPlaceCharacterImplementation = String::Implementation::replaceAllInPlaceCharacter;
    #endif
}

void StringBenchmark::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    String::Implementation::commonPrefix = _commonPrefixImplementation;
    String::Implementation::lowercaseInPlace = _lowercaseInPlaceImplementation;
    String::Implementation::uppercaseInPlace = _uppercaseInPlaceImplementation;
    String::Implementation::replaceAllInPlaceCharacter = _replaceAllInPlaceCharacterImplementation;
    #endif
}

constexpr std::size_t CharacterRepeats = 100;

template<char character> void StringBenchmark::commonPrefix() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::format("{}, {}",
        CharacterTraits<character>::name(),
        Utility::Test::cpuVariantName(data)));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);

    /* This works similarly to StringViewBenchmark::findCharacterCommon(),
       except that while there it was finding the next space, here the common
       prefix is until the next space that got changed to an underscore. */

    Containers::String string = *_text;
    String::replaceAllInPlace(string, character, '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::StringView a = string;
        Containers::StringView b = *_text;
        for(;;) {
            Containers::StringView prefix = String::commonPrefix(a, b);
            if(prefix.end() == a.end())
                break;
            ++count;
            a = a.exceptPrefix(prefix.size() + 1);
            b = b.exceptPrefix(prefix.size() + 1);
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringBenchmark::commonPrefixNaive() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    /* This works similarly to StringViewBenchmark::findCharacterCommonNaive(),
       except that while there it was finding the next space, here the common
       prefix is until the next space that got changed to an underscore. */

    Containers::String string = *_text;
    String::replaceAllInPlace(string, character, '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::StringView a = string;
        Containers::StringView b = *_text;
        for(;;) {
            std::size_t j = 0;
            for(; j != a.size(); ++j) {
                if(a[j] != b[j])
                    break;
            }
            if(j == a.size())
                break;

            ++count;
            a = a.exceptPrefix(j + 1);
            b = b.exceptPrefix(j + 1);
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

template<char character> void StringBenchmark::commonPrefixStl() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    /* Yes, making a std::string, to have it perform VERY NICE with the STL
       iterators -- it'd be cheating to pass a pair of pointers there */
    std::string string = *_text;
    String::replaceAllInPlace(string, character, '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        auto beginA = string.begin();
        auto beginB = _text->begin();
        auto endA = string.end();
        for(;;) {
            auto mismatch = std::mismatch(beginA, endA, beginB);
            if(mismatch.first == endA)
                break;
            ++count;
            beginA = mismatch.first + 1;
            beginB = mismatch.second + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<character>::Count*CharacterRepeats);
}

void StringBenchmark::commonPrefixCommonSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixSmallData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixSmallData);
    #endif
    setTestCaseDescription(Utility::format("{}, {} bytes", Utility::Test::cpuVariantName(data), data.size));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* This works similarly to StringViewBenchmark::findCharacterCommonSmall(),
       except that while there it was finding the next space, here the common
       prefix is until the next space that got changed to an underscore. */

    Containers::String string = *_text;
    String::replaceAllInPlace(string, ' ', '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::StringView a = string;
        Containers::StringView b = *_text;
        for(;;) {
            Containers::StringView prefix = String::commonPrefix(a.prefix(Utility::min(data.size, a.size())), b);
            if(prefix.end() == a.end())
                break;
            ++count;
            a = a.exceptPrefix(prefix.size() + 1);
            b = b.exceptPrefix(prefix.size() + 1);
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
}

void StringBenchmark::commonPrefixCommonSmallStl() {
    #if defined(CORRADE_TARGET_DINKUMWARE) && defined(CORRADE_IS_DEBUG_BUILD)
    CORRADE_SKIP("Takes too long on MSVC's STL in debug mode.");
    #endif

    CORRADE_VERIFY(_text);

    /* Yes, making a std::string, to have it perform VERY NICE with the STL
       iterators -- it'd be cheating to pass a pair of pointers there */
    std::string string = *_text;
    String::replaceAllInPlace(string, ' ', '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        auto beginA = string.begin();
        auto beginB = _text->begin();
        auto endA = string.end();
        for(;;) {
            auto mismatch = std::mismatch(beginA, Utility::min(beginA + 15, endA), beginB);
            if(mismatch.first == endA)
                break;
            ++count;
            beginA = mismatch.first + 1;
            beginB = mismatch.second + 1;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<' '>::Count*CharacterRepeats);
}

void StringBenchmark::commonPrefixRareDifferentlyAligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);

    /* Compared to commonPrefix(), we explicitly shift the second string by 7
       characters, so if the first one gets the alignment adjusted, the second
       is always off */
    Containers::String string = "1234567"_s + *_text;
    String::replaceAllInPlace(string, '\n', '_');
    CORRADE_COMPARE_AS(
        reinterpret_cast<std::uintptr_t>(_text->data()) % 16,
        reinterpret_cast<std::uintptr_t>(string.data() + 7) % 16,
        TestSuite::Compare::NotEqual);

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::StringView b = *_text;
        Containers::StringView a = string.exceptPrefix(7);
        for(;;) {
            Containers::StringView prefix = String::commonPrefix(a, b);
            if(prefix.end() == a.end())
                break;
            ++count;
            a = a.exceptPrefix(prefix.size() + 1);
            b = b.exceptPrefix(prefix.size() + 1);
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<'\n'>::Count*CharacterRepeats);
}

void StringBenchmark::commonPrefixRareMemcmp() {
    CORRADE_VERIFY(_text);

    /* Mainly to have a comparison for the tight loop performance in our
       implementation. As memcmp doesn't give back the position of the
       difference but just *how* they're different, call it explicitly on all
       subslices that have the last byte different to make the operation as
       close as possible to what commonPrefix() does */
    std::size_t offsets[CharacterTraits<'\n'>::Count + 1];
    offsets[0] = 0;
    for(std::size_t i = 1; i != Containers::arraySize(offsets); ++i)
        offsets[i] = _text->exceptPrefix(offsets[i - 1]).find('\n').begin() - _text->begin() + 1;
    CORRADE_VERIFY(!_text->exceptPrefix(offsets[Containers::arraySize(offsets) - 1]).find('\n'));

    Containers::String string = *_text;
    String::replaceAllInPlace(string, '\n', '_');

    std::size_t count = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::StringView a = string;
        Containers::StringView b = *_text;
        for(std::size_t i = 0; i != Containers::arraySize(offsets); ++i) {
            count += std::memcmp(a.data() + offsets[i], b.data() + offsets[i], _text->size() - offsets[i]) ? 1 : 0;
        }
    }

    CORRADE_COMPARE(count, CharacterTraits<'\n'>::Count*CharacterRepeats);
}

void StringBenchmark::lowercase() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.function ? data.function :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        String::lowercaseInPlace(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

CORRADE_NEVER_INLINE void lowercaseInPlaceBranchless(Containers::MutableStringView string) {
    for(char& c: string)
        c += (std::uint8_t(c - 'A') < 26) << 5;
}

void StringBenchmark::lowercaseBranchless() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
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
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        lowercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

/* This is the original implementation that used to be in
   String::lowercaseInPlace() */
CORRADE_NEVER_INLINE void lowercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string) if(c >= 'A' && c <= 'Z')
        c |= 0x20;
}

void StringBenchmark::lowercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        lowercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::tolower));
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::lowercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).tolower(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('L'));
    CORRADE_VERIFY(string.contains('l'));
}

void StringBenchmark::uppercase() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = UppercaseData[testCaseInstanceId()];
    String::Implementation::uppercaseInPlace = String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(UppercaseData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
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
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
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
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        uppercaseInPlaceBranchless32(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

/* This is the original implementation that used to be in
   String::uppercaseInPlace() */
CORRADE_NEVER_INLINE void uppercaseInPlaceNaive(Containers::MutableStringView string) {
    for(char& c: string) if(c >= 'a' && c <= 'z')
        c &= ~0x20;
}

void StringBenchmark::uppercaseNaive() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        uppercaseInPlaceNaive(string.sliceSize((i++)*_text->size(), _text->size()));

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStl() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    /* According to https://twitter.com/MalwareMinigun/status/1087767603647377408,
       std::tolower() / std::toupper() causes a mutex lock and a virtual
       dispatch per character (!!). C++ experts recommend using a lambda here,
       even, but that's even more stupider: https://twitter.com/cjdb_ns/status/1087754367367827456 */
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::transform(slice.begin(), slice.end(), slice.begin(), static_cast<int (*)(int)>(std::toupper));
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::uppercaseStlFacet() {
    CORRADE_VERIFY(_text);
    Containers::String string = *_text*CharacterRepeats;

    /* https://twitter.com/MalwareMinigun/status/1087768362912862208 OMG FFS */
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::MutableStringView slice = string.sliceSize((i++)*_text->size(), _text->size());
        std::use_facet<std::ctype<char>>(std::locale::classic()).toupper(slice.begin(), slice.end());
    }

    CORRADE_VERIFY(!string.contains('a'));
    CORRADE_VERIFY(string.contains('A'));
}

void StringBenchmark::lowercaseSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseSmallData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.function ? data.function :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseSmallData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {} bytes, {}" : "{}, {} bytes",
        Utility::Test::cpuVariantName(data), data.size, data.extra));

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
    auto&& data = UppercaseSmallData[testCaseInstanceId()];
    String::Implementation::uppercaseInPlace = String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(UppercaseSmallData);
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

template<char character> void StringBenchmark::replaceAllInPlaceCharacter() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}, {}" : "{}, {}",
        CharacterTraits<character>::name(),
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);

    Containers::String string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats)
        String::replaceAllInPlace(string.sliceSize((i++)*_text->size(), _text->size()), character, '_');

    CORRADE_VERIFY(!string.contains(character));
    CORRADE_VERIFY(string.contains('_'));
}

template<char character> void StringBenchmark::replaceAllInPlaceCharacterNaive() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    Containers::String string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        for(char& j: string.sliceSize((i++)*_text->size(), _text->size()))
            if(j == character)
                j = '_';
    }

    CORRADE_VERIFY(!string.contains(character));
    CORRADE_VERIFY(string.contains('_'));
}

template<char character> void StringBenchmark::replaceAllInPlaceCharacterMemchrLoop() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    Containers::String string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        char* a = string.data() + (i++)*_text->size();
        char* end = a + _text->size();
        while(char* found = static_cast<char*>(std::memchr(a, character, end - a))) {
            *found = '_';
            a = found + 1;
        }
    }

    CORRADE_VERIFY(!string.contains(character));
    CORRADE_VERIFY(string.contains('_'));
}

template<char character> void StringBenchmark::replaceAllInPlaceCharacterStl() {
    setTestCaseDescription(CharacterTraits<character>::name());

    CORRADE_VERIFY(_text);

    /* Yes, making a std::string, to have it perform VERY NICE with the STL
       iterators -- it'd be cheating to pass a pair of pointers there */
    std::string string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        auto begin = string.begin() + (i++)*_text->size();
        std::replace(begin, begin + _text->size(), character, '_');
    }

    CORRADE_VERIFY(!Containers::StringView{string}.contains(character));
    CORRADE_VERIFY(Containers::StringView{string}.contains('_'));
}

void StringBenchmark::replaceAllInPlaceCharacterCommonSmall() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterSmallData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterSmallData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {} bytes, {}" : "{}, {} bytes",
        Utility::Test::cpuVariantName(data), data.size, data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    CORRADE_VERIFY(_text);

    Containers::String string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        Containers::MutableStringView a = string.sliceSize((i++)*_text->size(), _text->size());
        while(a) {
            Containers::MutableStringView prefix = a.prefix(Utility::min(data.size, a.size()));
            String::replaceAllInPlace(prefix, ' ', '_');
            a = a.suffix(prefix.end());
        }
    }

    CORRADE_VERIFY(!string.contains(' '));
    CORRADE_VERIFY(string.contains('_'));
}

void StringBenchmark::replaceAllInPlaceCharacterCommonSmallStl() {
    #if defined(CORRADE_TARGET_DINKUMWARE) && defined(CORRADE_IS_DEBUG_BUILD)
    CORRADE_SKIP("Takes too long on MSVC's STL in debug mode.");
    #endif

    CORRADE_VERIFY(_text);

    /* Yes, making a std::string, to have it perform VERY NICE with the STL
       iterators -- it'd be cheating to pass a pair of pointers there */
    std::string string = *_text*CharacterRepeats;
    std::size_t i = 0;
    CORRADE_BENCHMARK(CharacterRepeats) {
        auto offset = string.begin() + (i++)*_text->size();
        auto sliceEnd = offset + _text->size();
        while(offset != sliceEnd) {
            auto end = Utility::min(offset + 15, sliceEnd);
            std::replace(offset, end, ' ', '_');
            offset = end;
        }
    }

    CORRADE_VERIFY(!Containers::StringView{string}.contains(' '));
    CORRADE_VERIFY(Containers::StringView{string}.contains('_'));
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimal() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        std::uint64_t value{};
        String::parseDecimal((this->*numbers)[i++], value);
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}

CORRADE_NEVER_INLINE std::uint64_t parseDecimalNaive(Containers::StringView string) {
    std::uint64_t out = 0;
    for(char i: string)
        out = out*10 + i - '0';
    return out;
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimalNaive() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        sum += Test::parseDecimalNaive((this->*numbers)[i++]);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimalStl() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        sum += std::strtoull((this->*numbers)[i++].data(), nullptr, 10);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimalStlNonNullTerminated() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        /* Does a copy compared to the above, in case of many leading zeros an
           allocated copy */
        sum += std::strtoull(Containers::String{(this->*numbers)[i++]}.data(), nullptr, 10);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimalStlStream() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    std::istringstream in;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        /* Lol, calling str() doesn't clear the eofbit on this damn thing?! */
        in.clear();
        /* This uses the String -> std::string conversion instead of passing
           .data() to avoid calling strlen() for each number */
        in.str((this->*numbers)[i++]);
        std::uint64_t value{};
        in >> value;
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}

#ifdef CAN_USE_STL_FROM_CHARS
template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseDecimalStlFromChars() {
    if(numbers == &StringBenchmark::_numbersLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        const Containers::String& string = (this->*numbers)[i++];
        std::uint64_t value{};
        std::from_chars(string.begin(), string.end(), value, 10);
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}
#endif

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimal() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        std::uint64_t value{};
        String::parseHexadecimal((this->*numbers)[i++], value);
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}

CORRADE_NEVER_INLINE std::uint64_t parseHexadecimalNaive(Containers::StringView string) {
    std::uint64_t out = 0;
    for(char i: string) {
        out <<= 4;
        if(i >= '0' && i <= '9')
            out += i - '0';
        else if(i >= 'a' && i <= 'f')
            out += i - 'a' + 10;
        else CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE();
    }
    return out;
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimalNaive() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        sum += Test::parseHexadecimalNaive((this->*numbers)[i++]);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimalStl() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        sum += std::strtoull((this->*numbers)[i++].data(), nullptr, 16);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimalStlNonNullTerminated() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size())
        /* Does a copy compared to the above, in case of many leading zeros an
           allocated copy */
        sum += std::strtoull(Containers::String{(this->*numbers)[i++]}.data(), nullptr, 16);

    CORRADE_COMPARE(sum, _numbersSum);
}

template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimalStlStream() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    std::istringstream in;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        /* Lol, calling str() doesn't clear the eofbit on this damn thing?! */
        in.clear();
        /* This uses the String -> std::string conversion instead of passing
           .data() to avoid calling strlen() for each number */
        in.str((this->*numbers)[i++]);
        std::uint64_t value{};
        in >> std::hex >> value;
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}

#ifdef CAN_USE_STL_FROM_CHARS
template<Containers::Array<Containers::String> StringBenchmark::*numbers> void StringBenchmark::parseHexadecimalStlFromChars() {
    if(numbers == &StringBenchmark::_numbersHexLeadingZeros)
        setTestCaseDescription("leading zeros");

    std::size_t i = 0;
    std::uint64_t sum = 0;
    CORRADE_BENCHMARK((this->*numbers).size()) {
        const Containers::String& string = (this->*numbers)[i++];
        std::uint64_t value{};
        std::from_chars(string.begin(), string.end(), value, 16);
        sum += value;
    }

    CORRADE_COMPARE(sum, _numbersSum);
}
#endif

template<class T> void StringBenchmark::parseFloat() {
    setTestCaseTemplateName(std::is_same<T, float>::value ? "float" : "double");

    std::size_t i = 0;
    T sum = 0;
    CORRADE_BENCHMARK(_numbers.size()) {
        T value{};
        String::parseFloat(_numbers[i++], value);
        sum += value;
    }

    CORRADE_COMPARE(sum, T(_numbersSumFloat));
}

template<class T, T(*parse)(const char*, char**)> void StringBenchmark::parseFloatStl() {
    setTestCaseTemplateName(std::is_same<T, float>::value ? "float" : "double");

    std::size_t i = 0;
    T sum = 0;
    CORRADE_BENCHMARK(_numbers.size())
        sum += parse(_numbers[i++].data(), nullptr);

    CORRADE_COMPARE(sum, T(_numbersSumFloat));
}

template<class T, T(*parse)(const char*, char**)> void StringBenchmark::parseFloatStlNonNullTerminated() {
    setTestCaseTemplateName(std::is_same<T, float>::value ? "float" : "double");

    std::size_t i = 0;
    T sum = 0;
    CORRADE_BENCHMARK(_numbers.size())
        /* Does a copy compared to the above, but likely fitting into SSO */
        sum += parse(_numbers[i++].data(), nullptr);

    CORRADE_COMPARE(sum, T(_numbersSumFloat));
}

template<class T> void StringBenchmark::parseFloatStlStream() {
    setTestCaseTemplateName(std::is_same<T, float>::value ? "float" : "double");

    std::size_t i = 0;
    T sum = 0;
    std::istringstream in;
    CORRADE_BENCHMARK(_numbers.size()) {
        /* Lol, calling str() doesn't clear the eofbit on this damn thing?! */
        in.clear();
        /* This uses the String -> std::string conversion instead of passing
           .data() to avoid calling strlen() for each number */
        in.str(_numbers[i++]);
        T value{};
        in >> value;
        sum += value;
    }

    CORRADE_COMPARE(sum, T(_numbersSumFloat));
}

#ifdef CAN_USE_STL_FROM_CHARS_FLOAT
template<class T> void StringBenchmark::parseFloatStlFromChars() {
    setTestCaseTemplateName(std::is_same<T, float>::value ? "float" : "double");

    std::size_t i = 0;
    T sum = 0;
    CORRADE_BENCHMARK(_numbers.size()) {
        const Containers::String& string = _numbers[i++];
        T value{};
        std::from_chars(string.begin(), string.end(), value);
        sum += value;
    }

    CORRADE_COMPARE(sum, T(_numbersSumFloat));
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringBenchmark)
