/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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

#include <sstream>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/Utility/Algorithms.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Memory.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Test/cpuVariantHelpers.h"
#include "Corrade/Utility/Test/StringTest.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void captureImplementations();
    void restoreImplementations();

    void fromArray();
    void trim();
    void trimInPlace();
    void split();
    void splitMultipleCharacters();
    void partition();
    void join();

    void commonPrefix();
    void commonPrefixAligned();
    void commonPrefixUnaligned();
    void commonPrefixUnalignedLessThanTwoVectors();
    void commonPrefixUnalignedLessThanOneVector();

    void lowercaseUppercase();
    void lowercaseUppercaseAligned();
    void lowercaseUppercaseUnaligned();
    void lowercaseUppercaseLessThanTwoVectors();
    void lowercaseUppercaseLessThanOneVector();
    void lowercaseUppercaseString();
    void lowercaseUppercaseStringSmall();
    void lowercaseUppercaseStringNotOwned();
    void lowercaseUppercaseStl();

    void beginsWith();
    void beginsWithEmpty();
    #ifdef CORRADE_BUILD_DEPRECATED
    void viewBeginsWith();
    #endif
    void endsWith();
    void endsWithEmpty();
    #ifdef CORRADE_BUILD_DEPRECATED
    void viewEndsWith();
    #endif

    void stripPrefix();
    void stripPrefixInvalid();
    void stripSuffix();
    void stripSuffixInvalid();

    void replaceFirst();
    void replaceFirstNotFound();
    void replaceFirstEmptySearch();
    void replaceFirstEmptyReplace();
    void replaceAll();
    void replaceAllNotFound();
    void replaceAllEmptySearch();
    void replaceAllEmptyReplace();
    void replaceAllCycle();
    void replaceAllCharacter();
    void replaceAllCharacterSmall();
    void replaceAllCharacterNonOwned();

    void replaceAllInPlaceCharacter();
    void replaceAllInPlaceCharacterAligned();
    void replaceAllInPlaceCharacterUnaligned();
    void replaceAllInPlaceCharacterLessThanTwoVectors();
    void replaceAllInPlaceCharacterLessThanOneVector();

    void parseNumberSequence();
    void parseNumberSequenceOverflow();
    void parseNumberSequenceError();

    private:
        #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
        decltype(String::Implementation::commonPrefix) _commonPrefixImplementation;
        decltype(String::Implementation::lowercaseInPlace) _lowercaseInPlaceImplementation;
        decltype(String::Implementation::uppercaseInPlace) _uppercaseInPlaceImplementation;
        decltype(String::Implementation::replaceAllInPlaceCharacter) _replaceAllInPlaceCharacterImplementation;
        #endif
};

const struct {
    Cpu::Features features;
    std::size_t vectorSize;
} CommonPrefixData[]{
    {Cpu::Scalar, 16},
    #if defined(CORRADE_ENABLE_SSE2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Sse2|Cpu::Bmi1, 16},
    #endif
    #if defined(CORRADE_ENABLE_AVX2) && defined(CORRADE_ENABLE_BMI1)
    {Cpu::Avx2|Cpu::Bmi1, 32},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t vectorSize;
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*lowercaseFunction)(char*, std::size_t);
    /* uppercase function has no extra variants */
} LowercaseUppercaseData[]{
    {Cpu::Scalar, 16, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE2
    {Cpu::Sse2, 16, "overflow + compare (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse2, 16, "two compares",
        lowercaseInPlaceImplementationSse2TwoCompares},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    {Cpu::Avx2, 32, nullptr, nullptr},
    #endif
    #if defined(CORRADE_ENABLE_NEON) && defined(CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH)
    {Cpu::Neon, 16, "trivial port (unused)",
        lowercaseInPlaceImplementationNeon},
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128, 16, nullptr, nullptr},
    #endif
};

const struct {
    Cpu::Features features;
    std::size_t vectorSize;
    const char* extra;
    /* Cases that define a function pointer are not present in the library, see
       the pointed-to function documentation for more info */
    void(*function)(char*, std::size_t, char, char);
} ReplaceAllInPlaceCharacterData[]{
    {Cpu::Scalar, 16, nullptr, nullptr},
    #ifdef CORRADE_ENABLE_SSE41
    {Cpu::Sse41, 16, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Sse41, 16, "unconditional replace",
        replaceAllInPlaceCharacterImplementationSse41Unconditional},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_AVX2
    {Cpu::Avx2, 32, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Avx2, 32, "unconditional replace",
        replaceAllInPlaceCharacterImplementationAvx2Unconditional},
    #endif
    #endif
    #ifdef CORRADE_ENABLE_SIMD128
    {Cpu::Simd128, 16, "conditional replace (default)", nullptr},
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    {Cpu::Simd128, 16, "unconditional replace",
        replaceAllInPlaceCharacterImplementationSimd128Unconditional},
    #endif
    #endif
};

const struct {
    const char* name;
    Containers::StringView string;
    Containers::Array<std::uint32_t> expected;
} ParseNumberSequenceData[]{
    {"empty",
        "", {InPlaceInit, {}}},
    {"single number",
        "5", {InPlaceInit, {5}}},
    {"random delimiters",
        "1,3\n8 5;9", {InPlaceInit, {1, 3, 8, 5, 9}}},
    {"duplicate numbers and delimiters",
        "1,\t\v5;;7  ,9\n3\r \f5,9", {InPlaceInit, {1, 5, 7, 9, 3, 5, 9}}},
    {"delimiters at start and end",
        "\t\v;;17,34,;;;", {InPlaceInit, {17, 34}}},
    {"just delimiters",
        "\t\v;;\n, ,;;;", {InPlaceInit, {}}},
    {"range",
        "7-11", {InPlaceInit, {7, 8, 9, 10, 11}}},
    {"range start == end",
        "11-11", {InPlaceInit, {11}}},
    {"range start < end",
        "11-7", {InPlaceInit, {}}},
    {"ranges and single numbers combined",
        "3-5,2,44,789-791", {InPlaceInit, {3, 4, 5, 2, 44, 789, 790, 791}}},
    {"zeros",
        "0,0-5,0-0", {InPlaceInit, {0, 0, 1, 2, 3, 4, 5, 0}}}
};

const struct {
    const char* name;
    std::uint32_t min, max;
    Containers::StringView string;
    Containers::Array<std::uint32_t> expected;
} ParseNumberSequenceOverflowData[]{
    {"zero min and max", 0, 0,
        "1,5,7", {InPlaceInit, {}}},
    {"min > max", 7, 1,
        "1,5,7", {InPlaceInit, {}}},
    {"less than min or larger than max", 3, 50,
        "2,34,55,1,17", {InPlaceInit, {34, 17}}},
    {"parse overflow in the middle", 0, ~std::uint32_t{},
        "14,9999999999,27", {InPlaceInit, {14, 27}}},
    {"parse overflow at the end", 0, ~std::uint32_t{},
        "14,27,9999999999", {InPlaceInit, {14, 27}}},
    {"0xfffffffe", 0, ~std::uint32_t{},
        "4294967294", {InPlaceInit, {0xfffffffe}}},
    {"0xffffffff", 0, ~std::uint32_t{},
        "4294967295", {InPlaceInit, {}}},
    {"range start underflow", 3, 50,
        "17,1-5,25", {InPlaceInit, {17, 3, 4, 5, 25}}},
    {"range end underflow", 3, 50,
        "17,0-2,25", {InPlaceInit, {17, 25}}},
    {"range start overflow", 3, 50,
        "17,55-60,25", {InPlaceInit, {17, 25}}},
    {"range end overflow", 3, 50,
        "17,45-60,25", {InPlaceInit, {17, 45, 46, 47, 48, 49, 25}}},
    {"range missing start", 3, 50,
        "17,-7,25", {InPlaceInit, {17, 3, 4, 5, 6, 7, 25}}},
    {"range missing end", 3, 50,
        "17,48-,25", {InPlaceInit, {17, 48, 49, 25}}},
    {"range missing both", 40, 45,
        "43,-,41", {InPlaceInit, {43, 40, 41, 42, 43, 44, 41}}},
    {"range missing start, 0xffffffff", 0xfffffffe, ~std::uint32_t{},
        "17,-4294967295,25", {InPlaceInit, {4294967294}}},
    {"range missing end, 0xfffffffe", 0, ~std::uint32_t{},
        "17,4294967294-,25", {InPlaceInit, {17, 4294967294, 25}}},
};

StringTest::StringTest() {
    addTests({&StringTest::fromArray,
              &StringTest::trim,
              &StringTest::trimInPlace,
              &StringTest::split,
              &StringTest::splitMultipleCharacters,
              &StringTest::partition,
              &StringTest::join});

    addInstancedTests({&StringTest::commonPrefix,
                       &StringTest::commonPrefixAligned,
                       &StringTest::commonPrefixUnaligned,
                       &StringTest::commonPrefixUnalignedLessThanTwoVectors,
                       &StringTest::commonPrefixUnalignedLessThanOneVector},
        cpuVariantCount(CommonPrefixData),
        &StringTest::captureImplementations,
        &StringTest::restoreImplementations);

    addInstancedTests({&StringTest::lowercaseUppercase,
                       &StringTest::lowercaseUppercaseAligned,
                       &StringTest::lowercaseUppercaseUnaligned,
                       &StringTest::lowercaseUppercaseLessThanTwoVectors,
                       &StringTest::lowercaseUppercaseLessThanOneVector},
        cpuVariantCount(LowercaseUppercaseData),
        &StringTest::captureImplementations,
        &StringTest::restoreImplementations);

    addTests({&StringTest::lowercaseUppercaseString,
              &StringTest::lowercaseUppercaseStringSmall,
              &StringTest::lowercaseUppercaseStringNotOwned,
              &StringTest::lowercaseUppercaseStl,

              &StringTest::beginsWith,
              &StringTest::beginsWithEmpty,
              #ifdef CORRADE_BUILD_DEPRECATED
              &StringTest::viewBeginsWith,
              #endif
              &StringTest::endsWith,
              &StringTest::endsWithEmpty,
              #ifdef CORRADE_BUILD_DEPRECATED
              &StringTest::viewEndsWith,
              #endif

              &StringTest::stripPrefix,
              &StringTest::stripPrefixInvalid,
              &StringTest::stripSuffix,
              &StringTest::stripSuffixInvalid,

              &StringTest::replaceFirst,
              &StringTest::replaceFirstNotFound,
              &StringTest::replaceFirstEmptySearch,
              &StringTest::replaceFirstEmptyReplace,
              &StringTest::replaceAll,
              &StringTest::replaceAllNotFound,
              &StringTest::replaceAllEmptySearch,
              &StringTest::replaceAllEmptyReplace,
              &StringTest::replaceAllCycle,
              &StringTest::replaceAllCharacter,
              &StringTest::replaceAllCharacterSmall,
              &StringTest::replaceAllCharacterNonOwned});

    addInstancedTests({&StringTest::replaceAllInPlaceCharacter,
                       &StringTest::replaceAllInPlaceCharacterAligned,
                       &StringTest::replaceAllInPlaceCharacterUnaligned,
                       &StringTest::replaceAllInPlaceCharacterLessThanTwoVectors,
                       &StringTest::replaceAllInPlaceCharacterLessThanOneVector},
        cpuVariantCount(ReplaceAllInPlaceCharacterData),
        &StringTest::captureImplementations,
        &StringTest::restoreImplementations);

    addInstancedTests({&StringTest::parseNumberSequence},
        Containers::arraySize(ParseNumberSequenceData));

    addInstancedTests({&StringTest::parseNumberSequenceOverflow},
        Containers::arraySize(ParseNumberSequenceOverflowData));

    addTests({&StringTest::parseNumberSequenceError});
}

using namespace Containers::Literals;

void StringTest::captureImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    _commonPrefixImplementation = String::Implementation::commonPrefix;
    _lowercaseInPlaceImplementation = String::Implementation::lowercaseInPlace;
    _uppercaseInPlaceImplementation = String::Implementation::uppercaseInPlace;
    _replaceAllInPlaceCharacterImplementation = String::Implementation::replaceAllInPlaceCharacter;
    #endif
}

void StringTest::restoreImplementations() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    String::Implementation::commonPrefix = _commonPrefixImplementation;
    String::Implementation::lowercaseInPlace = _lowercaseInPlaceImplementation;
    String::Implementation::uppercaseInPlace = _uppercaseInPlaceImplementation;
    String::Implementation::replaceAllInPlaceCharacter = _replaceAllInPlaceCharacterImplementation;
    #endif
}

void StringTest::fromArray() {
    CORRADE_COMPARE(String::fromArray(nullptr), "");
    CORRADE_COMPARE(String::fromArray(nullptr, 37), "");

    CORRADE_COMPARE(String::fromArray("abc\0def"), "abc");
    CORRADE_COMPARE(String::fromArray("abc\0def", 7), std::string("abc\0def", 7));
}

void StringTest::trim() {
    /* Spaces at the end */
    CORRADE_COMPARE(String::ltrim("abc  "), "abc  ");
    CORRADE_COMPARE(String::rtrim("abc  "), "abc");

    /* Spaces at the beginning */
    CORRADE_COMPARE(String::ltrim("  abc"), "abc");
    CORRADE_COMPARE(String::rtrim("  abc"), "  abc");

    /* Spaces on both beginning and end */
    CORRADE_COMPARE(String::trim("  abc  "), "abc");

    /* No spaces */
    CORRADE_COMPARE(String::trim("abc"), "abc");

    /* All spaces */
    CORRADE_COMPARE(String::trim("\t\r\n\f\v "), "");

    /* Special characters */
    CORRADE_COMPARE(String::ltrim("oubya", "aeiyou"), "bya");
    CORRADE_COMPARE(String::rtrim("oubya", "aeiyou"), "oub");
    CORRADE_COMPARE(String::trim("oubya", "aeiyou"), "b");

    /* Special characters as a string */
    CORRADE_COMPARE(String::ltrim("oubya", std::string{"aeiyou"}), "bya");
    CORRADE_COMPARE(String::rtrim("oubya", std::string{"aeiyou"}), "oub");
    CORRADE_COMPARE(String::trim("oubya", std::string{"aeiyou"}), "b");
}

void StringTest::trimInPlace() {
    /* Spaces at the end */
    {
        std::string a = "abc  ";
        String::ltrimInPlace(a);
        CORRADE_COMPARE(a, "abc  ");
    } {
        std::string a = "abc  ";
        String::rtrimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* Spaces at the beginning */
    {
        std::string a = "  abc";
        String::ltrimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    } {
        std::string a = "  abc";
        String::rtrimInPlace(a);
        CORRADE_COMPARE(a, "  abc");
    }

    /* Spaces on both beginning and end */
    {
        std::string a = "  abc  ";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* No spaces */
    {
        std::string a = "abc";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* All spaces */
    {
        std::string a = "\t\r\n\f\v ";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "");
    }

    /* Special characters */
    {
        std::string a = "oubya";
        String::ltrimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "bya");
    } {
        std::string a = "oubya";
        String::rtrimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "oub");
    } {
        std::string a = "oubya";
        String::trimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "b");
    }

    /* Special characters as a string */
    {
        std::string a = "oubya";
        String::ltrimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "bya");
    } {
        std::string a = "oubya";
        String::rtrimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "oub");
    } {
        std::string a = "oubya";
        String::trimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "b");
    }
}

void StringTest::split() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated.
       The explicit cast to avoid an ambiguous overload is kinda nasty, but
       since this is eventually getting deprecated, I don't care anymore. */

    /* Empty */
    CORRADE_COMPARE_AS(String::split(std::string{}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiter */
    CORRADE_COMPARE_AS(String::split(std::string{"/"}, '/'),
        (std::vector<std::string>{"", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"/"}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::split(std::string{"abcdef"}, '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"abcdef"}, '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::split(std::string{"ab/c/def"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab/c/def"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::split(std::string{"ab//c/def//"}, '/'),
        (std::vector<std::string>{"ab", "", "c", "def", "", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab//c/def//"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
}

void StringTest::splitMultipleCharacters() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated.
       The explicit cast to avoid an ambiguous overload is kinda nasty, but
       since this is eventually getting deprecated, I don't care anymore. */

    const char delimiters[] = ".:;";

    /* Empty */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{}, delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{".::;"}, delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"abcdef"}, delimiters),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab:c;def"}, delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab:c;;def."}, delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Whitespace */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab c  \t \ndef\r"}),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
}

void StringTest::partition() {
    /* Happy case */
    CORRADE_COMPARE_AS(String::partition("ab=c", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab=c", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c"}),
        TestSuite::Compare::Container);

    /* Two occurrences */
    CORRADE_COMPARE_AS(String::partition("ab=c=d", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c=d"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab=c=d", '='),
        (Containers::StaticArray<3, std::string>{"ab=c", "=", "d"}),
        TestSuite::Compare::Container);

    /* Not found */
    CORRADE_COMPARE_AS(String::partition("abc", '='),
        (Containers::StaticArray<3, std::string>{"abc", "", ""}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("abc", '='),
        (Containers::StaticArray<3, std::string>{"", "", "abc"}),
        TestSuite::Compare::Container);

    /* Empty input */
    CORRADE_COMPARE_AS(String::partition("", '='),
        (Containers::StaticArray<3, std::string>{"", "", ""}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("", '='),
        (Containers::StaticArray<3, std::string>{"", "", ""}),
        TestSuite::Compare::Container);

    /* More characters */
    CORRADE_COMPARE_AS(String::partition("ab, c, d", ", "),
        (Containers::StaticArray<3, std::string>{"ab", ", ", "c, d"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab, c, d", ", "),
        (Containers::StaticArray<3, std::string>{"ab, c", ", ", "d"}),
        TestSuite::Compare::Container);
}

void StringTest::join() {
    /* Empty */
    CORRADE_COMPARE(String::join({}, '/'), "");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({}, '/'), "");

    /* One empty value */
    CORRADE_COMPARE(String::join({""}, '/'), "");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({""}, '/'), "");

    /* Two empty values */
    CORRADE_COMPARE(String::join({"", ""}, '/'),
        "/");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"", ""}, '/'),
        "");

    /* One value */
    CORRADE_COMPARE(String::join({"abcdef"}, '/'),
        "abcdef");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"abcdef"}, '/'),
        "abcdef");

    /* Common case, also multi-character and std::string joiner */
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, '/'),
        "ab/c/def");
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, ", "),
        "ab, c, def");
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, std::string{", "}),
        "ab, c, def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, '/'),
        "ab/c/def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, ", "),
        "ab, c, def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, std::string{", "}),
        "ab, c, def");

    /* Empty parts */
    CORRADE_COMPARE(String::join({"ab", "", "c", "def", "", ""}, '/'),
        "ab//c/def//");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "", "c", "def", "", ""}, '/'),
        "ab/c/def");
}

void StringTest::commonPrefix() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    Containers::StringView a = "path/to/somewhere!"_s.exceptSuffix(1);
    CORRADE_COMPARE(a.flags(), Containers::StringViewFlag::Global);

    /* Usual case. The returned view should be a slice of the first argument,
       preserving its flags as well. */
    {
        Containers::StringView b = "path/to/someother/location";
        CORRADE_COMPARE(b.flags(), Containers::StringViewFlag::NullTerminated);

        Containers::StringView prefix1 = String::commonPrefix(a, b);
        Containers::StringView prefix2 = String::commonPrefix(b, a);
        CORRADE_COMPARE(prefix1, "path/to/some");
        CORRADE_COMPARE(prefix2, "path/to/some");
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(b.data()));
        CORRADE_COMPARE(prefix1.flags(), Containers::StringViewFlag::Global);
        /* Slicing a null-terminated array loses that flag */
        CORRADE_COMPARE(prefix2.flags(), Containers::StringViewFlags{});

    /* The whole string matches, thus null-termination is preserved as well */
    } {
        Containers::StringView b = "path/to/somewhere";
        CORRADE_COMPARE(b.flags(), Containers::StringViewFlag::NullTerminated);

        Containers::StringView prefix1 = String::commonPrefix(a, b);
        Containers::StringView prefix2 = String::commonPrefix(b, a);
        CORRADE_COMPARE(prefix1, "path/to/somewhere");
        CORRADE_COMPARE(prefix2, "path/to/somewhere");
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(b.data()));
        CORRADE_COMPARE(prefix1.flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(prefix2.flags(), Containers::StringViewFlag::NullTerminated);

    /* Difference at the first letter already, should return an empty prefix
       but still preserve the data pointer and flags */
    } {
        Containers::StringView prefix = String::commonPrefix(a, "/path");
        CORRADE_COMPARE(prefix, "");
        CORRADE_COMPARE(prefix.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(prefix.flags(), Containers::StringViewFlag::Global);

    /* Empty strings, should still preserve the data pointer and flags as
       well */
    } {
        Containers::StringView empty = "!"_s.exceptSuffix(1);
        Containers::StringView b = "";
        CORRADE_COMPARE(empty.flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(b.flags(), Containers::StringViewFlag::NullTerminated);

        Containers::StringView prefix1 = String::commonPrefix(a, b);
        Containers::StringView prefix2 = String::commonPrefix(b, a);
        CORRADE_COMPARE(prefix1, "");
        CORRADE_COMPARE(prefix2, "");
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(a.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(b.data()));
        CORRADE_COMPARE(prefix1.flags(), Containers::StringViewFlag::Global);
        CORRADE_COMPARE(prefix2.flags(), Containers::StringViewFlag::NullTerminated);

    /* Null terminator in the middle shouldn't break things */
    } {
        CORRADE_COMPARE(String::commonPrefix("abc\0de"_s, "abc\0df"_s), "abc\0d"_s);
    }
}

void StringTest::commonPrefixAligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    /* Like StringViewTest::findCharacterAligned(), but instead of finding a
       concrete character there are two strings with one getting changed at
       given position (back to front) and the prefix length is then verified */

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, with 12 vectors
       in total, corresponding to the code paths:
        - the first vector before the aligned four-at-a-time block, handled by
          the unaligned preamble
        - then two four-at-a-time blocks
        - then three more blocks after, handled by the aligned postamble
        - nothing left to be handled by the unaligned postamble

        +----+    +----+----+----+----+    +----+----+----+
        |ponm|    | lk : ji :h  g: fe |x2  |d   | bc |   a|
        +----+    +----+----+----+----+    +----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 4*2 + 3));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 4*2 + 3));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = arrayView(a);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::Aligned);

    /* Make sure the string isn't a multiple of vector size, copy it to the
       view to preserve the alignment */
    Containers::String source = "Hello hello, here's some string data hopefully long enough, YES?!"_s*6;
    Utility::copy(source.prefix(string.size()), string);
    CORRADE_COMPARE_AS(source.size(), 16,
        TestSuite::Compare::NotDivisible);

    /* If one string is a prefix of the other, it should return the shorter */
    {
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());

    /* If the strings are the same, it should return them whole */
    } {
        Containers::StringView prefix1 = String::commonPrefix(source.prefix(string.size()), string);
        Containers::StringView prefix2 = String::commonPrefix(string, source.prefix(string.size()));
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());
    }

    auto verify = [&](std::size_t position, char character) {
        CORRADE_ITERATION(Containers::StringView{&character, 1});
        string[position] = character;
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), position);
        CORRADE_COMPARE(prefix2.size(), position);
    };

    /* Last less-than-four vectors are treated separately. For each it should
       pick the earliest difference; test also the very first and very last of
       the range. */
    verify(data.vectorSize*12 - 1, 'a');
    verify(data.vectorSize*11 - 4, 'b');
    verify(data.vectorSize*10 + 4, 'c');
    verify(data.vectorSize* 9 + 0, 'd');

    /* First four of the four vectors at a time are the same, second four
       are different. Test each of the four separately, for each it should pick
       the first difference. */
    verify(data.vectorSize*9 - 2, 'e');
    verify(data.vectorSize*8 + 2, 'f');
    verify(data.vectorSize*8 - 1, 'g');
    verify(data.vectorSize*7 + 0, 'h');
    verify(data.vectorSize*7 - 7, 'i');
    verify(data.vectorSize*6 + 7, 'j');
    verify(data.vectorSize*6 - 3, 'k');
    verify(data.vectorSize*5 + 3, 'l');

    /* First vector is treated separately again. For each it should pick the
       earliest difference; test also the very first and very last of the
       range. */
    verify(data.vectorSize - 1, 'm');
    verify(data.vectorSize - 7, 'n');
    verify(7, 'o');
    verify(0, 'p');
}

void StringTest::commonPrefixUnaligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    /* Like StringViewTest::findCharacterUnaligned(), but instead of finding a
       concrete character there are two strings with one getting changed at
       given position and the prefix length is then verified */

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing:
        - the first unaligned vector having all bytes but one overlapping with
          the four-at-a-time block
        - there being just one four-at-a-time block (the if() branch that skips
          the block was sufficiently tested in firstCharacterAligned())
        - there being just one full vector after, and the last unaligned vector
          again overlapping with all but one byte with it

            +----+                +----+
            |e   |                |   a|
            +----+                +----+
             +----+----+----+----+----+
        | .. |d   :    :    :   c|b   | .. |
             +----+----+----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 4 + 2));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 4 + 2));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.slice(data.vectorSize - 1, a.size() - (data.vectorSize - 1));
    CORRADE_COMPARE(string.size(), data.vectorSize*5 + 2);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Make sure the string isn't a multiple of vector size, copy it to the
       view to preserve the alignment */
    Containers::String source = "Hey hey, another string, totally not as long this time"_s*5;
    Utility::copy(source.prefix(string.size()), string);
    CORRADE_COMPARE_AS(source.size(), 16,
        TestSuite::Compare::NotDivisible);

    /* If one string is a prefix of the other, it should return the shorter */
    {
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());

    /* If the strings are the same, it should return them whole */
    } {
        Containers::StringView prefix1 = String::commonPrefix(source.prefix(string.size()), string);
        Containers::StringView prefix2 = String::commonPrefix(string, source.prefix(string.size()));
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());
    }

    auto verify = [&](std::size_t position, char character) {
        CORRADE_ITERATION(Containers::StringView{&character, 1});
        string[position] = character;
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), position);
        CORRADE_COMPARE(prefix2.size(), position);
    };

    /* Last byte should be handled by the final unaligned check */
    verify(string.size() - 1, 'a');

    /* The byte right after the aligned block is handled by the "less than
       four vectors" block */
    verify(data.vectorSize*4 + 1, 'b');
    CORRADE_COMPARE_AS(string.data() + data.vectorSize*4 + 1, data.vectorSize,
        TestSuite::Compare::Aligned);

    /* The four-vectors-at-a-time should handle the aligned middle portion.
       Test just the very first and very last of the aligned range. */
    verify(data.vectorSize*4 + 0, 'c');
    verify(data.vectorSize*0 + 1, 'd');
    CORRADE_COMPARE_AS(string.data() + 1, data.vectorSize,
        TestSuite::Compare::Aligned);

    /* First byte should be handled by the initial unaligned check */
    verify(0, 'e');
}

void StringTest::commonPrefixUnalignedLessThanTwoVectors() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    /* Like StringViewTest::findCharacterUnalignedLessThanTwoVectors(), but
       instead of finding a concrete character there are two strings with one
       getting changed at given position and the prefix length is then
       verified */

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing
       so there's just two unaligned blocks overlapping with two bytes. Cannot
       overlap with just one byte as that'd mean one of them has to be aligned.

           +-----+
          X|4  32|
           +-----+
        | .. | .. | .. |
              +-----+
              |321 0|Y
              +-----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*3);
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*3);
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.sliceSize(1, data.vectorSize*2 - 2);
    /* The data pointer shouldn't be aligned, and the first (and only) aligned
       position inside shouldn't fit a whole vector */
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);
    CORRADE_COMPARE_AS(a.data() + 2*data.vectorSize, static_cast<void*>(string.end()),
        TestSuite::Compare::Greater);

    /* Make sure the string isn't a multiple of vector size, copy it to the
       view to preserve the alignment */
    Containers::String source = "WE are GETTING shorter ONCE again"_s*3;
    Utility::copy(source.prefix(string.size()), string);
    CORRADE_COMPARE_AS(source.size(), 16,
        TestSuite::Compare::NotDivisible);

    /* If one string is a prefix of the other, it should return the shorter */
    {
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());

    /* Common prefix where the suffix is also the same shouldn't result in
       output that's longer than the input */
    } {
        Containers::StringView prefix1 = String::commonPrefix(source.exceptSuffix(1), string.exceptSuffix(1));
        Containers::StringView prefix2 = String::commonPrefix(string.exceptSuffix(1), source.exceptSuffix(1));
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size() - 1);
        CORRADE_COMPARE(prefix2.size(), string.size() - 1);

    /* If the strings are the same, it should return them whole */
    } {
        Containers::StringView prefix1 = String::commonPrefix(source.prefix(string.size()), string);
        Containers::StringView prefix2 = String::commonPrefix(string, source.prefix(string.size()));
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());
    }

    auto verify = [&](std::size_t position, char character) {
        CORRADE_ITERATION(Containers::StringView{&character, 1});
        string[position] = character;
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), position);
        CORRADE_COMPARE(prefix2.size(), position);
    };

    /* Characters changed right before or right after the string shouldn't
       affect anything */
    *(string.begin() - 1) = 'X';
    *string.end() = 'Y';
    CORRADE_COMPARE(String::commonPrefix(source, string), string);
    CORRADE_COMPARE(String::commonPrefix(string, source), string);

    /* Last byte should be handled by the final unaligned check */
    verify(string.size() - 1, '0');

    /* A byte right after the end of the first vector should be handled by the
       final unaligned check */
    verify(data.vectorSize, '1');

    /* Bytes right before the end of the first vector should be handled by the
       initial unaligned check */
    verify(data.vectorSize - 1, '2');
    verify(data.vectorSize - 2, '3');

    /* First byte should be handled by the initial unaligned check */
    verify(0, '4');
}

void StringTest::commonPrefixUnalignedLessThanOneVector() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = CommonPrefixData[testCaseInstanceId()];
    String::Implementation::commonPrefix = String::Implementation::commonPrefixImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(CommonPrefixData);
    #endif
    setTestCaseDescription(Utility::Test::cpuVariantName(data));

    /* Like StringViewTest::findCharacterUnalignedLessThanTwoVectors(), but
       instead of finding a concrete character there are two strings with one
       getting changed at given position and the prefix length is then
       verified */

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Deliberately pick an unaligned
       pointer even though it shouldn't matter here. */
    Containers::Array<char> a{Corrade::ValueInit, data.vectorSize};
    Containers::MutableStringView string = a.exceptPrefix(1);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Make sure the string isn't a multiple of vector size, copy it to the
       view to preserve the alignment */
    Containers::String source = "This one is shortest ever!"_s*2;
    Utility::copy(source.prefix(string.size()), string);
    CORRADE_COMPARE_AS(source.size(), 16,
        TestSuite::Compare::NotDivisible);

    /* If one string is a prefix of the other, it should return the shorter */
    {
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());

    /* If the strings are the same, it should return them whole */
    } {
        Containers::StringView prefix1 = String::commonPrefix(source.prefix(string.size()), string);
        Containers::StringView prefix2 = String::commonPrefix(string, source.prefix(string.size()));
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), string.size());
        CORRADE_COMPARE(prefix2.size(), string.size());
    }

    auto verify = [&](std::size_t position, char character) {
        CORRADE_ITERATION(Containers::StringView{&character, 1});
        string[position] = character;
        Containers::StringView prefix1 = String::commonPrefix(source, string);
        Containers::StringView prefix2 = String::commonPrefix(string, source);
        CORRADE_COMPARE(prefix1.data(), static_cast<const void*>(source.data()));
        CORRADE_COMPARE(prefix2.data(), static_cast<const void*>(string.data()));
        CORRADE_COMPARE(prefix1.size(), position);
        CORRADE_COMPARE(prefix2.size(), position);
    };

    /* It should pick the first found of the two */
    verify(data.vectorSize/2 + 1, 'a');
    verify(7, 'b');
}

constexpr char AllBytes[]{
    '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
    '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
    '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
       ' ',    '!',    '"',    '#',    '$',    '%',    '&',   '\'',
       '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
       '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
       '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
       '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
       'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
       'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
       'X',    'Y',    'Z',    '[',   '\\',    ']',    '^',    '_',
       '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
       'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
       'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
       'x',    'y',    'z',    '{',    '|',    '}',    '~', '\x7f',
    '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
    '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x8d', '\x8e', '\x8f',
    '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
    '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e', '\x9f',
    '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7',
    '\xa8', '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf',
    '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf',
    '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7',
    '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf',
    '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7',
    '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde', '\xdf',
    '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7',
    '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef',
    '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7',
    '\xf8', '\xf9', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff',
};

constexpr char AllBytesUppercase[]{
    '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
    '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
    '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
       ' ',    '!',    '"',    '#',    '$',    '%',    '&',   '\'',
       '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
       '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
       '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
       '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
       'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
       'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
       'X',    'Y',    'Z',    '[',   '\\',    ']',    '^',    '_',
       '`',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
       'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
       'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
       'X',    'Y',    'Z',    '{',    '|',    '}',    '~', '\x7f',
    '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
    '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x8d', '\x8e', '\x8f',
    '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
    '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e', '\x9f',
    '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7',
    '\xa8', '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf',
    '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf',
    '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7',
    '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf',
    '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7',
    '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde', '\xdf',
    '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7',
    '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef',
    '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7',
    '\xf8', '\xf9', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff',
};

constexpr char AllBytesLowercase[]{
    '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
    '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
    '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
       ' ',    '!',    '"',    '#',    '$',    '%',    '&',   '\'',
       '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
       '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
       '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
       '@',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
       'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
       'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
       'x',    'y',    'z',    '[',   '\\',    ']',    '^',    '_',
       '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
       'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
       'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
       'x',    'y',    'z',    '{',    '|',    '}',    '~', '\x7f',
    '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87',
    '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x8d', '\x8e', '\x8f',
    '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97',
    '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e', '\x9f',
    '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7',
    '\xa8', '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf',
    '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7',
    '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf',
    '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7',
    '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf',
    '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7',
    '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde', '\xdf',
    '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7',
    '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef',
    '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7',
    '\xf8', '\xf9', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff',
};

void StringTest::lowercaseUppercase() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.lowercaseFunction ? data.lowercaseFunction :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    String::Implementation::uppercaseInPlace =
        String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Because the conversion is done using a bit operation on a range, check
       that the conversion is done on all characters and there's no off-by-one
       error at the bounds or other characters changed randomly */
    {
        CORRADE_COMPARE(Containers::arraySize(AllBytes), 256);
        CORRADE_COMPARE(Containers::arraySize(AllBytesUppercase), 256);
        CORRADE_COMPARE(Containers::arraySize(AllBytesLowercase), 256);
        /* It should be all ordered byte values */
        for(std::size_t i = 0; i != 256; ++i)
            CORRADE_COMPARE(std::uint8_t(AllBytes[i]), i);

        /* The conversion should only change alpha characters, nothing else.
           To ensure the vectorized variants don't treat certain bytes
           differently, shift it gradually by 0 to vectorSize - 1 bytes. */
        for(std::size_t i = 0; i != data.vectorSize; ++i) {
            CORRADE_ITERATION(i);
            CORRADE_COMPARE(String::uppercase(
                " "_s*i + Containers::StringView{AllBytes, 256}),
                " "_s*i + (Containers::StringView{AllBytesUppercase, 256}));
            CORRADE_COMPARE(String::lowercase(
                " "_s*i + Containers::StringView{AllBytes, 256}),
                " "_s*i + (Containers::StringView{AllBytesLowercase, 256}));
            CORRADE_COMPARE(String::uppercase(
                " "_s*i + Containers::StringView{AllBytesLowercase, 256}),
                " "_s*i + (Containers::StringView{AllBytesUppercase, 256}));
            CORRADE_COMPARE(String::lowercase(
                " "_s*i + Containers::StringView{AllBytesUppercase, 256}),
                " "_s*i + (Containers::StringView{AllBytesLowercase, 256}));
        }

    /* The rest is just a basic verification of the scalar fallback. See the
       other test cases below for verifying actual corner cases of the vector
       implementations. */

    /* No-op */
    } {
        CORRADE_COMPARE(String::lowercase("hello"_s), "hello");
        CORRADE_COMPARE(String::uppercase("YEAH"_s), "YEAH");

    /* Lowercase / uppercase */
    } {
        CORRADE_COMPARE(String::lowercase("YEAh!"_s), "yeah!");
        CORRADE_COMPARE(String::uppercase("Hello!"_s), "HELLO!");

    /* Special chars */
    } {
        Containers::StringView a = ".,?- \"!/(98765%";
        CORRADE_COMPARE(String::lowercase(a), a);
        CORRADE_COMPARE(String::uppercase(a), a);

    /* UTF-8 deliberately not changed in any way */
    } {
        CORRADE_COMPARE(String::lowercase("HÝŽDĚ"_s), "hÝŽdĚ");
        CORRADE_COMPARE(String::uppercase("hýždě"_s), "HýžDě");

    /* In-place. These are called from the copying functions so just verify
       they're exported and callable, everything else is already tested
       above */
    } {
        Containers::String yeah = "YEAh!"_s;
        String::lowercaseInPlace(yeah);
        CORRADE_COMPARE(yeah, "yeah!");

        Containers::String hello = "Hello!"_s;
        String::uppercaseInPlace(hello);
        CORRADE_COMPARE(hello, "HELLO!");
    }
}

void StringTest::lowercaseUppercaseAligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.lowercaseFunction ? data.lowercaseFunction :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    String::Implementation::uppercaseInPlace =
        String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, with 4 vectors
       in total, corresponding to the code paths:
        - the first vector before the aligned block, handled by the unaligned
          preamble
        - then three aligned blocks
        - nothing left to be handled by the unaligned postamble

        +----+  +----+----+----+
        |    |  |    |    |    |
        +----+  +----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 3));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 3));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = arrayView(a);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::Aligned);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    /** @todo remove the casts once std::string overloads are dropped */
    const std::size_t count = data.vectorSize/4;

    /* All uppercase */
    Utility::copy(("HELLOWORLDTODAYIS"_s*count).exceptSuffix(count), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"helloworldtodayis"_s*count}.exceptSuffix(count));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"HELLOWORLDTODAYIS"_s*count}.exceptSuffix(count));

    /* All lowercase */
    Utility::copy(("awesomefancypants"_s*count).exceptSuffix(count), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"awesomefancypants"_s*count}.exceptSuffix(count));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"AWESOMEFANCYPANTS"_s*count}.exceptSuffix(count));

    /* Mixed case, every even uppercase */
    Utility::copy(("ThIsIsArAnSoMyEaH"_s*count).exceptSuffix(count), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.exceptSuffix(count));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.exceptSuffix(count));

    /* Mixed case, every odd uppercase */
    Utility::copy(("tHiSiSaRaNsOmYeAh"_s*count).exceptSuffix(count), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.exceptSuffix(count));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.exceptSuffix(count));
}

void StringTest::lowercaseUppercaseUnaligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.lowercaseFunction ? data.lowercaseFunction :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    String::Implementation::uppercaseInPlace =
        String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing:
        - the first unaligned vector having all bytes but one overlapping with
          the aligned block
        - there being three aligned blocks (the if() branch that skips
          the block was sufficiently tested in lowercaseUppercaseAligned())
        - the last unaligned vector overlapping with all but one byte with it

        +----+      +----+
        |    |      |    |
        +----+      +----+
         +----+----+----+
         |    |    |    |
         +----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 3 + 1));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 3 + 1));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.slice(data.vectorSize - 1, a.size() - (data.vectorSize - 1));
    CORRADE_COMPARE(string.size(), data.vectorSize*3 + 2);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    /** @todo remove the casts once std::string overloads are dropped */
    const std::size_t count = data.vectorSize/4;

    /* All uppercase */
    Utility::copy(("HELLOWORLDTODAYIS"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"helloworldtodayis"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"HELLOWORLDTODAYIS"_s*count}.prefix(string.size()));

    /* All lowercase */
    Utility::copy(("awesomefancypants"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"awesomefancypants"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"AWESOMEFANCYPANTS"_s*count}.prefix(string.size()));

    /* Mixed case, every even uppercase */
    Utility::copy(("ThIsIsArAnSoMyEaH"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));

    /* Mixed case, every odd uppercase */
    Utility::copy(("tHiSiSaRaNsOmYeAh"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));
}

void StringTest::lowercaseUppercaseLessThanTwoVectors() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.lowercaseFunction ? data.lowercaseFunction :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    String::Implementation::uppercaseInPlace =
        String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing
       so there's just two unaligned blocks overlapping with two bytes. Cannot
       overlap with just one byte as that'd mean one of them has to be aligned.

           +-----+
           |     |
           +-----+
        | .. | .. | .. |
              +-----+
              |     |
              +-----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*3);
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*3);
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.sliceSize(1, data.vectorSize*2 - 2);
    /* The data pointer shouldn't be aligned, and the first (and only) aligned
       position inside shouldn't fit a whole vector */
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);
    CORRADE_COMPARE_AS(a.data() + 2*data.vectorSize, static_cast<void*>(string.end()),
        TestSuite::Compare::Greater);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    /** @todo remove the casts once std::string overloads are dropped */
    const std::size_t count = data.vectorSize/8;

    /* All uppercase */
    Utility::copy(("HELLOWORLDTODAYIS"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"helloworldtodayis"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"HELLOWORLDTODAYIS"_s*count}.prefix(string.size()));

    /* All lowercase */
    Utility::copy(("awesomefancypants"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"awesomefancypants"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"AWESOMEFANCYPANTS"_s*count}.prefix(string.size()));

    /* Mixed case, every even uppercase */
    Utility::copy(("ThIsIsArAnSoMyEaH"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));

    /* Mixed case, every odd uppercase */
    Utility::copy(("tHiSiSaRaNsOmYeAh"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));

    /* Data outside of the string shouldn't be affected by the process */
    /** @todo have Utility::fill(), finally */
    Utility::copy("X"_s*a.size(), a);
    String::lowercaseInPlace(string);
    CORRADE_COMPARE(string, "x"_s*string.size());
    CORRADE_COMPARE(*(string.begin() - 1), 'X');
    CORRADE_COMPARE(*string.end(), 'X');

    Utility::copy("z"_s*a.size(), a);
    String::uppercaseInPlace(string);
    CORRADE_COMPARE(string, "Z"_s*string.size());
    CORRADE_COMPARE(*(string.begin() - 1), 'z');
    CORRADE_COMPARE(*string.end(), 'z');
}

void StringTest::lowercaseUppercaseLessThanOneVector() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = LowercaseUppercaseData[testCaseInstanceId()];
    String::Implementation::lowercaseInPlace = data.lowercaseFunction ? data.lowercaseFunction :
        String::Implementation::lowercaseInPlaceImplementation(data.features);
    String::Implementation::uppercaseInPlace =
        String::Implementation::uppercaseInPlaceImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(LowercaseUppercaseData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!Utility::Test::isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Deliberately pick an unaligned
       pointer even though it shouldn't matter here. */
    Containers::Array<char> a{Corrade::ValueInit, data.vectorSize};
    Containers::MutableStringView string = a.exceptPrefix(1);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Test variants are copied to the view to preserve the above mem layout */
    /** @todo remove the casts once std::string overloads are dropped */
    const std::size_t count = data.vectorSize/16;

    /* All uppercase */
    Utility::copy(("HELLOWORLDTODAYIS"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"helloworldtodayis"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"HELLOWORLDTODAYIS"_s*count}.prefix(string.size()));

    /* All lowercase */
    Utility::copy(("awesomefancypants"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"awesomefancypants"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"AWESOMEFANCYPANTS"_s*count}.prefix(string.size()));

    /* Mixed case, every even uppercase */
    Utility::copy(("ThIsIsArAnSoMyEaH"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));

    /* Mixed case, every odd uppercase */
    Utility::copy(("tHiSiSaRaNsOmYeAh"_s*count).prefix(string.size()), string);
    CORRADE_COMPARE(String::lowercase(Containers::StringView{string}), Containers::StringView{"thisisaransomyeah"_s*count}.prefix(string.size()));
    CORRADE_COMPARE(String::uppercase(Containers::StringView{string}), Containers::StringView{"THISISARANSOMYEAH"_s*count}.prefix(string.size()));
}

void StringTest::lowercaseUppercaseString() {
    /* It should just operate in-place, not allocate a copy */

    {
        Containers::String in{Containers::AllocatedInit, "YEAh!"};
        const char* data = in.data();
        Containers::String out = String::lowercase(Utility::move(in));
        CORRADE_COMPARE(out, "yeah!");
        CORRADE_VERIFY(out.data() == data);
    } {
        Containers::String in{Containers::AllocatedInit, "Hello!"};
        const char* data = in.data();
        Containers::String out = String::uppercase(Utility::move(in));
        CORRADE_COMPARE(out, "HELLO!");
        CORRADE_VERIFY(out.data() == data);
    }
}

void StringTest::lowercaseUppercaseStringSmall() {
    /* For SSO there's no allocation to preserve, so just check that it works */

    {
        Containers::String string{"YEAh!"};
        CORRADE_VERIFY(string.isSmall());
        CORRADE_COMPARE(String::lowercase(string), "yeah!");
    } {
        Containers::String string{"Hello!"};        CORRADE_VERIFY(string.isSmall());
        CORRADE_COMPARE(String::uppercase(string), "HELLO!");
    }
}

void StringTest::lowercaseUppercaseStringNotOwned() {
    /* Will make a copy as it can't touch a potentially immutable data */

    {
        const char* data = "YEAh!";
        Containers::String in = Containers::String::nullTerminatedView(data);
        CORRADE_VERIFY(!in.isSmall());
        CORRADE_VERIFY(in.deleter());

        Containers::String out = String::lowercase(Utility::move(in));
        CORRADE_COMPARE(out, "yeah!");
        CORRADE_VERIFY(out.data() != data);
    } {
        const char* data = "Hello!";
        Containers::String in = Containers::String::nullTerminatedView(data);
        CORRADE_VERIFY(!in.isSmall());
        CORRADE_VERIFY(in.deleter());

        Containers::String out = String::uppercase(Utility::move(in));
        CORRADE_COMPARE(out, "HELLO!");
        CORRADE_VERIFY(out.data() != data);
    }
}

void StringTest::lowercaseUppercaseStl() {
    /* These just call into the in-place implementations tested above, so
       verify just basic functionality */
    CORRADE_COMPARE(String::lowercase(std::string{"YEAh!"}), "yeah!");
    CORRADE_COMPARE(String::uppercase(std::string{"Hello!"}), "HELLO!");
}

void StringTest::beginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(String::beginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", std::string{"over"}));

    CORRADE_VERIFY(!String::beginsWith("overcomplicated", "oven"));
    CORRADE_VERIFY(!String::beginsWith("overcomplicated", std::string{"oven"}));

    CORRADE_VERIFY(String::beginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::beginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::beginsWith("", 'h'));
}

void StringTest::beginsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(!String::beginsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::beginsWith("", ""));
}

#ifdef CORRADE_BUILD_DEPRECATED
void StringTest::viewBeginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_IGNORE_DEPRECATED_PUSH
    CORRADE_VERIFY(String::viewBeginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(!String::viewBeginsWith("overcomplicated", "oven"));

    CORRADE_VERIFY(String::viewBeginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::viewBeginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewBeginsWith("", 'h'));
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif

void StringTest::endsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(String::endsWith("overcomplicated", "complicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", std::string{"complicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "somplicated"));
    CORRADE_VERIFY(!String::endsWith("overcomplicated", std::string{"somplicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "overcomplicated even more"));

    CORRADE_VERIFY(!String::endsWith("hello", 'h'));
    CORRADE_VERIFY(String::endsWith("hello", 'o'));
    CORRADE_VERIFY(!String::endsWith("", 'h'));
}

void StringTest::endsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(!String::endsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::endsWith("", ""));
}

#ifdef CORRADE_BUILD_DEPRECATED
void StringTest::viewEndsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_IGNORE_DEPRECATED_PUSH
    CORRADE_VERIFY(String::viewEndsWith({"overcomplicated", 15}, "complicated"));
    CORRADE_VERIFY(!String::viewEndsWith("overcomplicated", "complicated"));

    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "somplicated"));
    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "overcomplicated even more"));

    CORRADE_VERIFY(!String::viewEndsWith({"hello", 5}, 'h'));
    CORRADE_VERIFY(String::viewEndsWith({"hello", 5}, 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("", 'h'));
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif

void StringTest::stripPrefix() {
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", "over"), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", std::string{"over"}), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", 'o'), "vercomplicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", ""), "overcomplicated");
}

void StringTest::stripPrefixInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectOutput{&out};
    String::stripPrefix("overcomplicated", "complicated");
    CORRADE_COMPARE(out.str(), "Utility::String::stripPrefix(): string doesn't begin with given prefix\n");
}

void StringTest::stripSuffix() {
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", "complicated"), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", std::string{"complicated"}), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", 'd'), "overcomplicate");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", ""), "overcomplicated");
}

void StringTest::stripSuffixInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectOutput{&out};
    String::stripSuffix("overcomplicated", "over");
    CORRADE_COMPARE(out.str(), "Utility::String::stripSuffix(): string doesn't end with given suffix\n");
}

void StringTest::replaceFirst() {
    CORRADE_COMPARE(String::replaceFirst(
        "this part will get replaced and this will get not",
        "will get", "got"),
        "this part got replaced and this will get not");
}

void StringTest::replaceFirstNotFound() {
    CORRADE_COMPARE(String::replaceFirst("this part will not get replaced",
        "will get", "got"), "this part will not get replaced");
}

void StringTest::replaceFirstEmptySearch() {
    CORRADE_COMPARE(String::replaceFirst("this completely messed up",
        "", "got "), "got this completely messed up");
}

void StringTest::replaceFirstEmptyReplace() {
    CORRADE_COMPARE(String::replaceFirst("this completely messed up",
        "completely ", ""), "this messed up");
}

void StringTest::replaceAll() {
    CORRADE_COMPARE(String::replaceAll(
        "this part will get replaced and this will get replaced also",
        "will get", "got"),
        "this part got replaced and this got replaced also");
}

void StringTest::replaceAllNotFound() {
    CORRADE_COMPARE(String::replaceAll("this part will not get replaced",
        "will get", "got"), "this part will not get replaced");
}

void StringTest::replaceAllEmptySearch() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectOutput{&out};
    String::replaceAll("this completely messed up", "", "got ");
    CORRADE_COMPARE(out.str(), "Utility::String::replaceAll(): empty search string would cause an infinite loop\n");
}

void StringTest::replaceAllEmptyReplace() {
    CORRADE_COMPARE(String::replaceAll("lalalalala!",
        "la", ""), "!");
}

void StringTest::replaceAllCycle() {
    CORRADE_COMPARE(String::replaceAll("lalala",
        "la", "lala"), "lalalalalala");
}

void StringTest::replaceAllCharacter() {
    /* No occurences */
    CORRADE_COMPARE(String::replaceAll("we?? are? loud??", '!', '?'),
        "we?? are? loud??");

    /* Multiple occurences */
    CORRADE_COMPARE(String::replaceAll("we?? are? loud??", '?', '!'),
        "we!! are! loud!!");

    /* Just that character alone */
    CORRADE_COMPARE(String::replaceAll(Containers::String{Containers::AllocatedInit, "?"}, '?', '!'),
        "!");

    /* Empty string */
    CORRADE_COMPARE(String::replaceAll({}, '?', '!'),
        {});
}

void StringTest::replaceAllCharacterSmall() {
    /* Shouldn't attempt to call deleter() on the string */
    Containers::String in = "hello";
    CORRADE_VERIFY(in.isSmall());
    CORRADE_COMPARE(String::replaceAll(Utility::move(in), 'e', 'a'), "hallo");
}

void StringTest::replaceAllCharacterNonOwned() {
    const char* data = "we?? are? loud??";
    Containers::String in = Containers::String::nullTerminatedView(data);
    CORRADE_VERIFY(!in.isSmall());
    CORRADE_VERIFY(in.deleter());

    /* Will make a copy as it can't touch a potentially immutable data */
    Containers::String out = String::replaceAll(Utility::move(in), '?', '!');
    CORRADE_COMPARE(out, "we!! are! loud!!");
    CORRADE_VERIFY(out.data() != data);
}

void StringTest::replaceAllInPlaceCharacter() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    char string[] = "we? are? loud??";
    String::replaceAllInPlace(string, '?', '!');
    CORRADE_COMPARE(string, "we! are! loud!!"_s);
}

void StringTest::replaceAllInPlaceCharacterAligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, with 4 vectors
       in total, corresponding to the code paths:
        - the first vector before the aligned block, handled by the unaligned
          preamble
        - then two four-at-a-time blocks
        - then three more blocks after, handled by the aligned postamble
        - nothing left to be handled by the unaligned postamble

        +----+    +----+----+----+----+    +----+----+----+
        |    |    |    |    |    |    |x2  |    |    |    |
        +----+    +----+----+----+----+    +----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 4*2 + 3));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 4*2 + 3));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = arrayView(a);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::Aligned);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. The extra characters are then cut
       off. */
    const std::size_t count = data.vectorSize*3/4;

    Utility::copy(("H e ll o w or ld!"_s*count).exceptSuffix(count), string);
    String::replaceAllInPlace(string, ' ', '-');
    CORRADE_COMPARE(string, ("H-e-ll-o-w-or-ld!"_s*count).exceptSuffix(count));
}

void StringTest::replaceAllInPlaceCharacterUnaligned() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing:
        - the first unaligned vector having all bytes but one overlapping with
          the aligned block
        - there being three aligned blocks (the if() branch that skips
          the block was sufficiently tested in
          replaceAllInPlaceCharacterAligned())
        - the last unaligned vector overlapping with all but one byte with it

        +----+      +----+
        |    |      |    |
        +----+      +----+
         +----+----+----+
         |    |    |    |
         +----+----+----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*(1 + 3 + 1));
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*(1 + 3 + 1));
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.slice(data.vectorSize - 1, a.size() - (data.vectorSize - 1));
    CORRADE_COMPARE(string.size(), data.vectorSize*3 + 2);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    const std::size_t count = data.vectorSize/4;

    Utility::copy(("H e ll o w or ld!"_s*count).prefix(string.size()), string);
    String::replaceAllInPlace(string, ' ', '-');
    CORRADE_COMPARE(string, ("H-e-ll-o-w-or-ld!"_s*count).prefix(string.size()));
}

void StringTest::replaceAllInPlaceCharacterLessThanTwoVectors() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Also, aligned, but then slicing
       so there's just two unaligned blocks overlapping with two bytes. Cannot
       overlap with just one byte as that'd mean one of them has to be aligned.

           +-----+
           |     |
           +-----+
        | .. | .. | .. |
              +-----+
              |     |
              +-----+
    */
    Containers::Array<char> a;
    if(data.vectorSize == 16)
        a = Utility::allocateAligned<char, 16>(Corrade::ValueInit, data.vectorSize*3);
    else if(data.vectorSize == 32)
        a = Utility::allocateAligned<char, 32>(Corrade::ValueInit, data.vectorSize*3);
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
    Containers::MutableStringView string = a.sliceSize(1, data.vectorSize*2 - 2);
    /* The data pointer shouldn't be aligned, and the first (and only) aligned
       position inside shouldn't fit a whole vector */
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);
    CORRADE_COMPARE_AS(a.data() + 2*data.vectorSize, static_cast<void*>(string.end()),
        TestSuite::Compare::Greater);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    const std::size_t count = data.vectorSize/8;

    Utility::copy(("H e ll o w or ld!"_s*count).prefix(string.size()), string);
    String::replaceAllInPlace(string, ' ', '-');
    CORRADE_COMPARE(string, ("H-e-ll-o-w-or-ld!"_s*count).prefix(string.size()));

    /* Inverse of the above */
    Utility::copy((" H e  l l o  !   "_s*count).prefix(string.size()), string);
    String::replaceAllInPlace(string, ' ', '-');
    CORRADE_COMPARE(string, ("-H-e--l-l-o--!---"_s*count).prefix(string.size()));

    /* Characters right before or right after the string shouldn't be
       affected by the process */
    /** @todo have Utility::fill(), finally */
    Utility::copy("X"_s*a.size(), a);
    String::replaceAllInPlace(string, 'X', 'Y');
    CORRADE_COMPARE(string, "Y"_s*string.size());
    CORRADE_COMPARE(*(string.begin() - 1), 'X');
    CORRADE_COMPARE(*string.end(), 'X');
}

void StringTest::replaceAllInPlaceCharacterLessThanOneVector() {
    #ifdef CORRADE_UTILITY_FORCE_CPU_POINTER_DISPATCH
    auto&& data = ReplaceAllInPlaceCharacterData[testCaseInstanceId()];
    String::Implementation::replaceAllInPlaceCharacter = data.function ? data.function :
        String::Implementation::replaceAllInPlaceCharacterImplementation(data.features);
    #else
    auto&& data = cpuVariantCompiled(ReplaceAllInPlaceCharacterData);
    #endif
    setTestCaseDescription(Utility::format(
        data.extra ? "{}, {}" : "{}",
        Utility::Test::cpuVariantName(data), data.extra));

    if(!isCpuVariantSupported(data))
        CORRADE_SKIP("CPU features not supported");

    /* Allocating an array to not have it null-terminated or SSO'd in order to
       trigger ASan if the algorithm goes OOB. Deliberately pick an unaligned
       pointer even though it shouldn't matter here. */
    Containers::Array<char> a{Corrade::ValueInit, data.vectorSize};
    Containers::MutableStringView string = a.exceptPrefix(1);
    CORRADE_COMPARE_AS(string.data(), data.vectorSize,
        TestSuite::Compare::NotAligned);

    /* Test data copied to the view to preserve the above mem layout. The
       string is 17 characters to not be exactly the same for each vector to
       catch mismatched loads and stores. */
    const std::size_t count = data.vectorSize/16;

    Utility::copy(("H e ll o w or ld!"_s*count).prefix(string.size()), string);
    String::replaceAllInPlace(string, ' ', '-');
    CORRADE_COMPARE(string, ("H-e-ll-o-w-or-ld!"_s*count).prefix(string.size()));
}

void StringTest::parseNumberSequence() {
    auto&& data = ParseNumberSequenceData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Containers::Array<std::uint32_t>> out = String::parseNumberSequence(data.string, 0, ~std::uint32_t{});
    CORRADE_VERIFY(out);
    CORRADE_COMPARE_AS(*out, data.expected, TestSuite::Compare::Container);
}

void StringTest::parseNumberSequenceOverflow() {
    auto&& data = ParseNumberSequenceOverflowData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Containers::Array<std::uint32_t>> out = String::parseNumberSequence(data.string, data.min, data.max);
    CORRADE_VERIFY(out);
    CORRADE_COMPARE_AS(*out, data.expected, TestSuite::Compare::Container);
}

void StringTest::parseNumberSequenceError() {
    std::ostringstream out;
    Error redirectError{&out};
    String::parseNumberSequence("3,5y7,x,25", 0, ~std::uint32_t{});
    CORRADE_COMPARE(out.str(), "Utility::parseNumberSequence(): unrecognized character y in 3,5y7,x,25\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
