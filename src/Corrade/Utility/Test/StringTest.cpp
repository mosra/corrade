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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/Algorithms.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Memory.h"
#include "Corrade/Utility/StlMath.h" /* NAN, HUGE_VAL */
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Test/cpuVariantHelpers.h"
#include "Corrade/Utility/Test/StringTest.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include <string>
#include <vector>

#include "Corrade/Utility/DebugStl.h"
#endif

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void debugParseState();
    void debugParseDecimalFlag();
    void debugParseDecimalFlags();
    void debugParseHexadecimalFlag();
    void debugParseHexadecimalFlags();
    void debugParseFloatFlag();
    void debugParseFloatFlags();

    void captureImplementations();
    void restoreImplementations();

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

    void parseResultConstruct();
    void parseResultConstructCopy();

    void parseDecimalUnsigned();
    void parseDecimalUnsignedFailed();
    void parseDecimalSigned();
    void parseDecimalSignedFailed();

    void parseHexadecimalUnsigned();
    void parseHexadecimalUnsignedFailed();
    void parseHexadecimalSigned();
    void parseHexadecimalSignedFailed();

    void parseFloat();
    void parseFloatFailed();

    template<class T> void parseDecimalHexadecimalUnsignedLimits();
    template<class T> void parseDecimalHexadecimalSignedLimits();
    void parseDecimalHexadecimalFloatNonNullTerminated();
    void parseDecimalHexadecimalInvalid();

    void parseNumberSequence();
    void parseNumberSequenceOverflow();
    void parseNumberSequenceError();

    #ifdef CORRADE_BUILD_DEPRECATED
    void deprecatedFromArray();
    void deprecatedTrim();
    void deprecatedTrimInPlace();
    void deprecatedSplit();
    void deprecatedSplitMultipleCharacters();
    void deprecatedPartition();
    void deprecatedJoin();

    void deprecatedBeginsWith();
    void deprecatedBeginsWithEmpty();
    void deprecatedViewBeginsWith();
    void deprecatedEndsWith();
    void deprecatedEndsWithEmpty();
    void deprecatedViewEndsWith();

    void deprecatedStripPrefix();
    void deprecatedStripPrefixInvalid();
    void deprecatedStripSuffix();
    void deprecatedStripSuffixInvalid();
    #endif

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
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    Containers::Optional<std::uint64_t> min;
    Containers::Optional<std::uint64_t> max;
    String::ParseState state;
    std::uint64_t value;
} ParseDecimalUnsignedData[]{
    {"zero",
        "0", {}, {},
        String::ParseState::Success, 0},
    {"several zeros",
        "00000", {}, {},
        String::ParseState::Success, 0},
    {"zero with an explicit sign",
        "+0", {}, {},
        String::ParseState::Success, 0},
    {"all digits",
        "6532710984", {}, {},
        String::ParseState::Success, 6532710984},
    {"leading zeros",
        "0000004625183", {}, {},
        String::ParseState::Success, 4625183},
    {"explicit sign",
        "+420222333111", {}, {},
        String::ParseState::Success, 420222333111},
    {"explicit sign, leading zeros",
        "+0000777", {}, {},
        String::ParseState::Success, 777},
    {"max representable value",
        "18446744073709551615", {}, {},
        String::ParseState::Success, ~std::uint64_t{}},
    {"max representable value, leading zeros",
        "000000018446744073709551615", {}, {},
        String::ParseState::Success, ~std::uint64_t{}},
    {"overflow in last addition",
        "18446744073709551616", {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"overflow in last addition, leading zeros",
        "000018446744073709551616", {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"overflow in last multiply",
        "18446744073709551620", {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"overflow in last multiply, leading zeros",
        "0018446744073709551620", {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"a very large value",
        "10000000000000000000000000000000000000000000", {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"less than min",
        "235", 250, 950,
        String::ParseState::Clamped, 250},
    {"greater than max",
        "1003", 250, 950,
        String::ParseState::Clamped, 950},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseDecimalFlags flags;
    std::size_t expected;
} ParseDecimalUnsignedFailedData[]{
    {"empty string",
        "", {}, 0},
    {"null string",
        nullptr, {}, 0},
    {"negative sign",
        "-33", {}, 0},
    {"positive sign alone",
        "+", {}, 1},
    {"sign disallowed",
        "+33", String::ParseDecimalFlag::DisallowSign, 0},
    /* These two likely just pass with std::strtoull() */
    {"trailing whitespace",
        "12\t", {}, 2},
    {"leading whitespace",
        "  12", {}, 0},
    {"whitespace in the middle",
        "1 2", {}, 1},
    {"non-numeric character at the front",
        "e1342", {}, 0},
    {"non-numeric character after a sign",
        "+e1342", {}, 1},
    {"non-numeric character after leading zeros",
        "000e1342", {}, 3},
    {"non-numeric character after a sign and leading zeros",
        "+000e1342", {}, 4},
    {"non-numeric character inside",
        "134f2", {}, 3},
    {"non-numeric character at the end",
        "1342f", {}, 4},
    {"non-numeric character at the end, leading zeros",
        "0001342f", {}, 7},
    {"non-numeric character at the end, sign",
        "+1342f", {}, 5},
    {"non-numeric character at the end, sign and leading zeros",
        "+0001342f", {}, 8},
    /* This may cause std::strtoull() to switch to hex parsing */
    {"hexadecimal prefix",
        "0x1337", {}, 1},
    {"garbage at the last char of a max representable value",
        "1844674407370955161a", {}, 19},
    {"garbage at the last char of a max representable value, leading zeros",
        "001844674407370955161a", {}, 21},
    {"garbage after max representable value",
        "18446744073709551615a", {}, 20},
    {"garbage after max representable value, leading zeros",
        "000018446744073709551615a", {}, 24},
    {"garbage after a clamped value",
        "18446744073709551700a", {}, 20},
    {"garbage after a clamped value, leading zeros",
        "000018446744073709551700a", {}, 24},
    {"garbage after a very large value",
        "10000000000000000000000000000000000000000000e", {}, 44},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    Containers::Optional<std::int64_t> min;
    Containers::Optional<std::int64_t> max;
    String::ParseState state;
    std::int64_t value;
} ParseDecimalSignedData[]{
    {"zero",
        "0", {}, {},
        String::ParseState::Success, 0},
    {"several zeros",
        "00000", {}, {},
        String::ParseState::Success, 0},
    {"positive zero",
        "+0", {}, {},
        String::ParseState::Success, 0},
    {"negative zero",
        "-0", {}, {},
        String::ParseState::Success, 0},
    {"positive",
        "+420222333111", {}, {},
        String::ParseState::Success, +420222333111ll},
    {"negative",
        "-666222333111", {}, {},
        String::ParseState::Success, -666222333111ll},
    {"positive, leading zeros",
        "+0000777", {}, {},
        String::ParseState::Success, 777},
    {"negative, leading zeros",
        "-000666", {}, {},
        String::ParseState::Success, -666},
    {"min representable value",
        "-9223372036854775808", {}, {},
        String::ParseState::Success, INT64_MIN},
    {"min representable value, leading zeros",
        "-00000009223372036854775808", {}, {},
        String::ParseState::Success, INT64_MIN},
    {"min representable value minus one",
        "-9223372036854775809", {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"min representable value minus one, leading zeros",
        "-00000009223372036854775809", {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"max representable value",
        "9223372036854775807", {}, {},
        String::ParseState::Success, INT64_MAX},
    {"max representable value, leading zeros",
        "00000009223372036854775807", {}, {},
        String::ParseState::Success, INT64_MAX},
    {"max representable value plus one",
        "9223372036854775808", {}, {},
        String::ParseState::Clamped, INT64_MAX},
    {"max representable value plus one, leading zeros",
        "00000009223372036854775808", {}, {},
        String::ParseState::Clamped, INT64_MAX},
    /* No "overflow in last addition" / "multiply" tests here, as those verify
       the raw unsigned 64-bit parsing which is tested above already */
    {"a very large value",
        "10000000000000000000000000000000000000000000", {}, {},
        String::ParseState::Clamped, INT64_MAX},
    {"a very large negative value",
        "-10000000000000000000000000000000000000000000", {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"less than positive min",
        "235", 250, 950,
        String::ParseState::Clamped, 250},
    {"less than negative min",
        "-275", -250, 950,
        String::ParseState::Clamped, -250},
    {"greater than positive max",
        "1003", 250, 950,
        String::ParseState::Clamped, 950},
    {"greater than negative max",
        "-115", -950, -250,
        String::ParseState::Clamped, -250},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseDecimalFlags flags;
    std::size_t expected;
} ParseDecimalSignedFailedData[]{
    {"empty string",
        "", {}, 0},
    {"null string",
        nullptr, {}, 0},
    {"positive sign alone",
        "+", {}, 1},
    {"negative sign alone",
        "-", {}, 1},
    {"positive sign disallowed",
        "+33", String::ParseDecimalFlag::DisallowSign, 0},
    {"negative sign disallowed",
        "-666", String::ParseDecimalFlag::DisallowSign, 0},
    /* These two likely just pass with std::strtoull() */
    {"trailing whitespace",
        "12\t", {}, 2},
    {"leading whitespace",
        "  12", {}, 0},
    {"whitespace in the middle",
        "1 2", {}, 1},
    {"non-numeric character at the front",
        "e1342", {}, 0},
    {"non-numeric character after a sign",
        "-e1342", {}, 1},
    {"non-numeric character after leading zeros",
        "000e1342", {}, 3},
    {"non-numeric character after a sign and leading zeros",
        "+000e1342", {}, 4},
    /* No "non-numeric character inside" and "at the end" except for just one
       as those verify the raw unsigned 64-bit parsing which is tested above
       already */
    {"non-numeric character at the end, sign and leading zeros",
        "-0001342f", {}, 8},
    /* This may cause std::strtoull() to switch to hex parsing */
    {"hexadecimal prefix",
        "0x1337", {}, 1},
    /* No "garbage after max representable value" etc. tests here, as those
       verify the raw unsigned 64-bit parsing which is tested above already */
    {"garbage after a very large value",
        "10000000000000000000000000000000000000000000e", {}, 44},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    Containers::Optional<std::uint64_t> min;
    Containers::Optional<std::uint64_t> max;
    String::ParseHexadecimalFlags flags;
    String::ParseState state;
    std::uint64_t value;
} ParseHexadecimalUnsignedData[]{
    {"zero",
        "0", {}, {}, {},
        String::ParseState::Success, 0},
    {"several zeros",
        "00000", {}, {}, {},
        String::ParseState::Success, 0},
    {"zero with an explicit sign",
        "+0", {}, {}, {},
        String::ParseState::Success, 0},
    {"all chars",
        "6f53a27be10d9c84", {}, {}, {},
        String::ParseState::Success, 0x6f53a27be10d9c84ull},
    {"all chars, uppercase",
        "6F53A27BE10D9C84", {}, {}, {},
        String::ParseState::Success, 0x6f53a27be10d9c84ull},
    {"mixed case",
        "CAFE3456babe", {}, {}, {},
        String::ParseState::Success, 0xcafe3456babeull},
    {"leading zeros",
        "000000462ab83", {}, {}, {},
        String::ParseState::Success, 0x462ab83},
    {"explicit sign",
        "+420222eee111", {}, {}, {},
        String::ParseState::Success, 0x420222eee111},
    {"explicit sign, leading zeros",
        "+00007a7", {}, {}, {},
        String::ParseState::Success, 0x7a7},
    {"base prefix",
        "0xdead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, 0xdead},
    {"base prefix, explicit sign and leading zeros",
        "+0x00dead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, 0xdead},
    {"base prefix, uppercase",
        "0XdEaD", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, 0xdead},
    {"base prefix, hash prefix allowed as well",
        "0xdead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xdead},
    {"hash prefix",
        "#ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xffcc33},
    {"hash prefix, explicit sign and leading zeros",
        "+#00ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xffcc33},
    {"hash prefix, base prefix allowed as well",
        "#ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xffcc33},
    {"max representable value",
        "ffffffffffffffff", {}, {}, {},
        String::ParseState::Success, ~std::uint64_t{}},
    {"max representable value, leading zeros",
        "0000000ffffffffffffffff", {}, {}, {},
        String::ParseState::Success, ~std::uint64_t{}},
    {"one more character that overflows",
        "ffffffffffffffff0", {}, {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    /* This should be handled with the same check as above, just verifying that
       it doesn't get parsed as 0 for some reason */
    {"max representable value plus one",
        "10000000000000000", {}, {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"a very large value",
        "10000000000000000000000000000000000000000000", {}, {}, {},
        String::ParseState::Clamped, ~std::uint64_t{}},
    {"less than min",
        "2a5", 0x2e0, 0x9e0, {},
        String::ParseState::Clamped, 0x2e0},
    {"greater than max",
        "1bb3", 0x2e0, 0x9f0, {},
        String::ParseState::Clamped, 0x9f0},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseHexadecimalFlags flags;
    std::size_t expected;
} ParseHexadecimalUnsignedFailedData[]{
    {"empty string",
        "", {}, 0},
    {"null string",
        nullptr, {}, 0},
    {"negative sign",
        "-3e3", {}, 0},
    {"positive sign alone",
        "+", {}, 1},
    {"sign disallowed",
        "+3e3", String::ParseHexadecimalFlag::DisallowSign, 0},
    {"base prefix disallowed",
        "0x3", {}, 1},
    {"base prefix after a sign disallowed",
        "+0x3", {}, 2},
    {"base prefix while only hash prefix allowed",
        "0x3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"base prefix alone",
        "0x", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"base prefix with extra zeros",
        "000x3", String::ParseHexadecimalFlag::AllowBasePrefix, 3},
    {"base prefix with extra Xs",
        "0xxx3", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"base prefix followed by a sign",
        "0x+3", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"hash prefix disallowed",
        "#3", {}, 0},
    {"hash prefix after a sign disallowed",
        "+#3", {}, 1},
    {"hash prefix while only base prefix allowed",
        "#3", String::ParseHexadecimalFlag::AllowBasePrefix, 0},
    {"hash prefix alone",
        "#", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"multiple hash prefixes",
        "###3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"hash prefix followed by a sign",
        "#+3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"base prefix followed by a hash prefix",
        "0x#3", String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix, 2},
    {"hash prefix followed by a base prefix",
        "#0x3", String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix, 2},
    /* These two likely just pass with std::strtoull() */
    {"trailing whitespace",
        "12\t", {}, 2},
    {"leading whitespace",
        "  12", {}, 0},
    {"whitespace in the middle",
        "1 2", {}, 1},
    {"non-hex character at the front",
        "g13a2", {}, 0},
    {"non-hex character after a sign",
        "+g13a2", {}, 1},
    {"non-hex character after leading zeros",
        "000g13a2", {}, 3},
    {"non-hex character after a sign and leading zeros",
        "+000g13a2", {}, 4},
    {"non-hex character after a base prefix",
        "0xg13a2", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"non-hex character after a base prefix, a sign and leading zeros",
        "+0x00g13a2", String::ParseHexadecimalFlag::AllowBasePrefix, 5},
    {"non-hex character after a hash prefix",
        "#g13a2", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"non-hex character after a hash prefix, a sign and leading zeros",
        "+#00g13a2", String::ParseHexadecimalFlag::AllowHashPrefix, 4},
    {"non-hex character inside",
        "13ag2", {}, 3},
    {"non-hex character at the end",
        "13a2g", {}, 4},
    {"non-hex character at the end, leading zeros",
        "00013a2g", {}, 7},
    {"non-hex character at the end, sign",
        "+13a2g", {}, 5},
    {"non-hex character at the end, sign and leading zeros",
        "+00013a2g", {}, 8},
    {"non-hex character at the end, sign, base prefix and leading zeros",
        "+0x00013a2g", String::ParseHexadecimalFlag::AllowBasePrefix, 10},
    {"non-hex character at the end, sign, hash prefix and leading zeros",
        "+#00013a2g", String::ParseHexadecimalFlag::AllowHashPrefix, 9},
    {"garbage after max representable value",
        "ffffffffffffffffg", {}, 16},
    {"garbage after max representable value, leading zeros",
        "0000ffffffffffffffffg", {}, 20},
    {"garbage after a clamped value",
        "10000000000000000g", {}, 17},
    {"garbage after a clamped value, leading zeros",
        "000010000000000000000g", {}, 21},
    {"garbage after a very large value",
        "10000000000000000000000000000000000000000000g", {}, 44},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    Containers::Optional<std::int64_t> min;
    Containers::Optional<std::int64_t> max;
    String::ParseHexadecimalFlags flags;
    String::ParseState state;
    std::int64_t value;
} ParseHexadecimalSignedData[]{
    {"zero",
        "0", {}, {}, {},
        String::ParseState::Success, 0},
    {"several zeros",
        "00000", {}, {}, {},
        String::ParseState::Success, 0},
    {"positive zero",
        "+0", {}, {}, {},
        String::ParseState::Success, 0},
    {"negative zero",
        "-0", {}, {}, {},
        String::ParseState::Success, 0},
    {"positive",
        "+420222eee111", {}, {}, {},
        String::ParseState::Success, 0x420222eee111ll},
    {"negative",
        "-aaa222eee111", {}, {}, {},
        String::ParseState::Success, -0xaaa222eee111ll},
    {"positive, leading zeros",
        "+00007a7", {}, {}, {},
        String::ParseState::Success, 0x7a7},
    {"negative, leading zeros",
        "-0000a7a", {}, {}, {},
        String::ParseState::Success, -0xa7a},
    {"base prefix",
        "0xdead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, 0xdead},
    {"base prefix, negative and leading zeros",
        "-0x00dead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, -0xdead},
    {"base prefix, uppercase",
        "0XdEaD", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix,
        String::ParseState::Success, 0xdead},
    {"base prefix, positive, hash prefix allowed as well",
        "+0xdead", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xdead},
    {"hash prefix",
        "#ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xffcc33},
    {"hash prefix, positive and leading zeros",
        "+#00ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, 0xffcc33},
    {"hash prefix, negative, base prefix allowed as well",
        "-#ffcc33", {}, {}, String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix,
        String::ParseState::Success, -0xffcc33},
    {"min representable value",
        "-8000000000000000", {}, {}, {},
        String::ParseState::Success, INT64_MIN},
    {"min representable value, leading zeros",
        "-0008000000000000000", {}, {}, {},
        String::ParseState::Success, INT64_MIN},
    {"min representable value minus one",
        "-8000000000000001", {}, {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"min representable value minus one, leading zeros",
        "-0008000000000000001", {}, {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"max representable value",
        "7fffffffffffffff", {}, {}, {},
        String::ParseState::Success, INT64_MAX},
    {"max representable value, leading zeros",
        "00007fffffffffffffff", {}, {}, {},
        String::ParseState::Success, INT64_MAX},
    {"max representable value plus one",
        "8000000000000000", {}, {}, {},
        String::ParseState::Clamped, INT64_MAX},
    {"max representable value plus one, leading zeros",
        "00008000000000000000", {}, {}, {},
        String::ParseState::Clamped, INT64_MAX},
    /* No "one more character that overflows" etc. tests here, as those verify
       the raw unsigned 64-bit parsing which is tested above already */
    {"a very large value",
        "10000000000000000000000000000000000000000000", {}, {}, {},
        String::ParseState::Clamped, INT64_MAX},
    {"a very large negative value",
        "-10000000000000000000000000000000000000000000", {}, {}, {},
        String::ParseState::Clamped, INT64_MIN},
    {"less than positive min",
        "2a5", 0x2e0, 0x9e0, {},
        String::ParseState::Clamped, 0x2e0},
    {"less than negative min",
        "-2d5", -0x2b0, 9e0, {},
        String::ParseState::Clamped, -0x2b0},
    {"greater than positive max",
        "1bb3", 0x2e0, 0x9f0, {},
        String::ParseState::Clamped, 0x9f0},
    {"greater than negative max",
        "-1e5", -0x9f0, -0x2e0, {},
        String::ParseState::Clamped, -0x2e0},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseHexadecimalFlags flags;
    std::size_t expected;
} ParseHexadecimalSignedFailedData[]{
    {"empty string",
        "", {}, 0},
    {"null string",
        nullptr, {}, 0},
    {"positive sign alone",
        "+", {}, 1},
    {"negative sign alone",
        "-", {}, 1},
    {"positive sign disallowed",
        "+3e3", String::ParseHexadecimalFlag::DisallowSign, 0},
    {"negative sign disallowed",
        "-aaa", String::ParseHexadecimalFlag::DisallowSign, 0},
    {"base prefix disallowed",
        "0x3", {}, 1},
    {"base prefix after a sign disallowed",
        "-0x3", {}, 2},
    {"base prefix while only hash prefix allowed",
        "0x3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"base prefix alone",
        "0x", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"base prefix with extra zeros",
        "000x3", String::ParseHexadecimalFlag::AllowBasePrefix, 3},
    {"base prefix with extra Xs",
        "0xxx3", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"base prefix followed by a sign",
        "0x+3", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"hash prefix disallowed",
        "#3", {}, 0},
    {"hash prefix after a sign disallowed",
        "-#3", {}, 1},
    {"hash prefix while only base prefix allowed",
        "#3", String::ParseHexadecimalFlag::AllowBasePrefix, 0},
    {"hash prefix alone",
        "#", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"multiple hash prefixes",
        "###3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"hash prefix followed by a sign",
        "#-3", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"base prefix followed by a hash prefix",
        "0x#3", String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix, 2},
    {"hash prefix followed by a base prefix",
        "#0x3", String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag::AllowHashPrefix, 2},
    /* These two likely just pass with std::strtoull() */
    {"trailing whitespace",
        "12\t", {}, 2},
    {"leading whitespace",
        "  12", {}, 0},
    {"whitespace in the middle",
        "1 2", {}, 1},
    {"non-hex character at the front",
        "g13a2", {}, 0},
    {"non-hex character after a sign",
        "-g13a2", {}, 1},
    {"non-hex character after leading zeros",
        "000g13a2", {}, 3},
    {"non-hex character after a sign and leading zeros",
        "-000g13a2", {}, 4},
    {"non-hex character after a base prefix",
        "0xg13a2", String::ParseHexadecimalFlag::AllowBasePrefix, 2},
    {"non-hex character after a base prefix, a sign and leading zeros",
        "+0x00g13a2", String::ParseHexadecimalFlag::AllowBasePrefix, 5},
    {"non-hex character after a hash prefix",
        "#g13a2", String::ParseHexadecimalFlag::AllowHashPrefix, 1},
    {"non-hex character after a hash prefix, a sign and leading zeros",
        "-#00g13a2", String::ParseHexadecimalFlag::AllowHashPrefix, 4},
    /* No "non-hex character inside" and "at the end" except for just two as
       those verify the raw unsigned 64-bit parsing which is tested above
       already */
    {"non-hex character at the end, sign, base prefix and leading zeros",
        "-0x00013a2g", String::ParseHexadecimalFlag::AllowBasePrefix, 10},
    {"non-hex character at the end, sign, hash prefix and leading zeros",
        "+#00013a2g", String::ParseHexadecimalFlag::AllowHashPrefix, 9},
    /* No "garbage after max representable value" etc. tests here, as those
       verify the raw unsigned 64-bit parsing which is tested above already */
    {"garbage after a very large value",
        "10000000000000000000000000000000000000000000g", {}, 44},
};

/* Yeah, sure, undefined behavior and all. Do I care? No. */
union FloatFromBits {
    explicit FloatFromBits(std::uint32_t bits): bits{bits} {}
    std::uint32_t bits;
    float value;
};
union DoubleFromBits {
    explicit DoubleFromBits(std::uint64_t bits): bits{bits} {}
    std::uint64_t bits;
    double value;
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseState state;
    float value;
    String::ParseState stateDouble;
    double valueDouble;
} ParseFloatData[]{
    {"zero", "0",
        String::ParseState::Success, 0.0f,
        String::ParseState::Success, 0.0},
    {"very many zeros",
        "000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000e100",
        String::ParseState::Success, 0.0f,
        String::ParseState::Success, 0.0},
    {"positive zero", "+0",
        String::ParseState::Success, 0.0f,
        String::ParseState::Success, 0.0},
    {"negative zero", "-0",
        String::ParseState::Success, -0.0f,
        String::ParseState::Success, -0.0},
    {"negative zero with an exponent", "-0e-100",
        String::ParseState::Success, -0.0f,
        String::ParseState::Success, -0.0},
    {"value", "1337.420",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    {"leading zeros", "000000000000001337.420",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    /** @todo make this fail? if yes, then disallow also +. and -. */
    {"leading zero omitted", ".420",
        String::ParseState::Success, 0.420f,
        String::ParseState::Success, 0.420},
    {"leading zero omitted, positive", "+.420",
        String::ParseState::Success, 0.420f,
        String::ParseState::Success, 0.420},
    {"leading zero omitted, negative", "-.420",
        String::ParseState::Success, -0.420f,
        String::ParseState::Success, -0.420},
    {"positive value", "+1337.420",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    {"negative value", "-1337.420",
        String::ParseState::Success, -1337.420f,
        String::ParseState::Success, -1337.420},
    {"exponent", "1.33742e3",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    {"uppercase exponent", "1.33742E3",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    {"exponent with positive sign", "1.33742e+3",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},
    {"exponent with negative sign", "1337420e-3",
        String::ParseState::Success, 1337.420f,
        String::ParseState::Success, 1337.420},

    /* Overflow to positive/negative infinity */
    {"largest 32-bit value", "340282346638528859811704183484516925440",
        /* https://en.wikipedia.org/wiki/Single-precision_floating-point_format */
        String::ParseState::Success, FloatFromBits{0x7f7fffffu}.value,
        String::ParseState::Success, 340282346638528859811704183484516925440.0},
                      /* value changed here ---v to 6 from 4 */
    {"largest 32-bit value plus some", "340282366638528859811704183484516925440",
        String::ParseState::Clamped, HUGE_VALF,
        String::ParseState::Success, 340282366638528859811704183484516925440.0},
    {"smallest 32-bit value", "-340282346638528859811704183484516925440",
        /* Like above, but flipping the highest sign bit */
        String::ParseState::Success, FloatFromBits{0xff7fffffu}.value,
        String::ParseState::Success, -340282346638528859811704183484516925440.0},
                         /* value changed here ---v to 6 from 4 */
    {"smallest 32-bit value minus some", "-340282366638528859811704183484516925440",
        String::ParseState::Clamped, -HUGE_VALF,
        String::ParseState::Success, -340282366638528859811704183484516925440.0},

    {"largest 64-bit value",
        "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368",
        String::ParseState::Clamped, HUGE_VALF,
        /* https://en.wikipedia.org/wiki/Double-precision_floating-point_format */
        String::ParseState::Success, DoubleFromBits{0x7fefffffffffffffull}.value},
    {"largest 64-bit value plus some",
                      /* v--- value changed here to 8 from 7 */
        "179769313486231580814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368",
        String::ParseState::Clamped, HUGE_VALF,
        String::ParseState::Clamped, HUGE_VAL},
    {"smallest 64-bit value",
        "-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368",
        String::ParseState::Clamped, -HUGE_VALF,
        /* Like above, but flipping the highest sign bit */
        String::ParseState::Success, DoubleFromBits{0xffefffffffffffffull}.value},
    {"smallest 64-bit value plus one",
                       /* v--- value changed here to 8 from 7 */
        "-179769313486231580814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368",
        String::ParseState::Clamped, -HUGE_VALF,
        String::ParseState::Clamped, -HUGE_VAL},

    /* The string `infinity` is supported by std::strtof() as well but I don't
       intend to claim that as being supported so don't even test for that */
    {"infinity", "inf",
        /* It should *not* claim that a clamp happened since that's what we
           want to enter */
        String::ParseState::Success, HUGE_VALF,
        String::ParseState::Success, HUGE_VAL},
    {"positive infinity, mixed case", "+iNF",
        String::ParseState::Success, HUGE_VALF,
        String::ParseState::Success, HUGE_VAL},
    {"negative infinity, mixed case", "-Inf",
        String::ParseState::Success, -HUGE_VALF,
        String::ParseState::Success, -HUGE_VAL},
    /* Positive / negative NaN is ignored for practical purposes, so comparing
       to just NaN always */
    {"NaN", "nan",
        String::ParseState::Success, NAN,
        String::ParseState::Success, double(NAN)},
    {"negative NaN, mixed case", "-nAn",
        String::ParseState::Success, NAN,
        String::ParseState::Success, double(NAN)},
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    const char* string;
    String::ParseFloatFlags flags;
    std::size_t expected;
} ParseFloatFailedData[]{
    {"empty string",
        "", {}, 0},
    {"null string",
        nullptr, {}, 0},
    {"positive sign alone",
        "+", {}, 1},
    {"negative sign alone",
        "-", {}, 1},
    {"positive sign disallowed",
        "+33", String::ParseFloatFlag::DisallowSign, 0},
    {"negative sign disallowed",
        "-666", String::ParseFloatFlag::DisallowSign, 0},
    /* These two likely just pass with std::strtof() */
    {"trailing whitespace",
        "12\t", {}, 2},
    {"leading whitespace",
        "  12", {}, 0},
    {"whitespace in the middle",
        "1 2", {}, 1},
    {"non-numeric character at the front",
        "f13.42", {}, 0},
    {"non-numeric character after a sign",
        "-f13.42", {}, 1},
    {"non-numeric character after a decimal point",
        "13.f42", {}, 3},
    {"non-numeric character at the end",
        "13.42f", {}, 5},
    {"duplicated minus sign",
        "--13.37", {}, 1},
    {"duplicated plus sign",
        "++13.37", {}, 1},
    {"plus and minus sign",
        "+-13.37", {}, 1},
    {"minus and plus sign",
        "-+13.37", {}, 1},

    /* I don't intend to support this weird hex representation once Corrade has
       own float parsers so disallowing it here already. (A hex representation
       of a float/double bit pattern is something else, supporting that makes
       sense, but that doesn't need a complex float parser.) Checking all
       possible variants that should fail. */
    {"hex representation",
        "0xfeed.beef", {}, 1},
    {"hex representation, negative",
        "-0xfeed.beef", {}, 2},
    {"hex representation, positive",
        "+0xfeed.beef", {}, 2},
    {"hex representation, uppercase",
        "0XFEED.BEEF", {}, 1},
    {"hex representation, negative uppercase",
        "-0XFEED.BEEF", {}, 2},
    {"hex representation, positive uppercase",
        "+0XFEED.BEEF", {}, 2},
    {"hex representation with an exponent and spaces around",
        /* It fails on the space already */
        "   -0x1.bc70a3d70a3d7p+6 ", {}, 0},
    {"hex representation with an exponent and spaces around, uppercase",
        /* It fails on the space already */
        "\t\b0X1.BC70A3D70A3D7P6  ", {}, 0},

    /* Cases that currently fail but maybe eventually shouldn't? */
    {"space in the middle",
        "420 69", {}, 3},
    {"space after a plus sign",
        "+ 420.1337", {}, 1},
    {"space after a minus sign",
        "- 420.1337", {}, 1},
    {"space before an exponent",
        /** @todo interestingly enough here it points to the exponent, not to
            the space after */
        "+4.201337e +2", {}, 9},
    {"comma as a decimal separator",
        "13,37", {}, 2},
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
    addTests({&StringTest::debugParseState,
              &StringTest::debugParseDecimalFlag,
              &StringTest::debugParseDecimalFlags,
              &StringTest::debugParseHexadecimalFlag,
              &StringTest::debugParseHexadecimalFlags,
              &StringTest::debugParseFloatFlag,
              &StringTest::debugParseFloatFlags});

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
              &StringTest::lowercaseUppercaseStringNotOwned});

    addTests({&StringTest::replaceFirst,
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

    addTests({&StringTest::parseResultConstruct,
              &StringTest::parseResultConstructCopy});

    addInstancedTests({&StringTest::parseDecimalUnsigned},
        Containers::arraySize(ParseDecimalUnsignedData));

    addInstancedTests({&StringTest::parseDecimalUnsignedFailed},
        Containers::arraySize(ParseDecimalUnsignedFailedData));

    addInstancedTests({&StringTest::parseDecimalSigned},
        Containers::arraySize(ParseDecimalSignedData));

    addInstancedTests({&StringTest::parseDecimalSignedFailed},
        Containers::arraySize(ParseDecimalSignedFailedData));

    addInstancedTests({&StringTest::parseHexadecimalUnsigned},
        Containers::arraySize(ParseHexadecimalUnsignedData));

    addInstancedTests({&StringTest::parseHexadecimalUnsignedFailed},
        Containers::arraySize(ParseHexadecimalUnsignedFailedData));

    addInstancedTests({&StringTest::parseHexadecimalSigned},
        Containers::arraySize(ParseHexadecimalSignedData));

    addInstancedTests({&StringTest::parseHexadecimalSignedFailed},
        Containers::arraySize(ParseHexadecimalSignedFailedData));

    addInstancedTests({&StringTest::parseFloat},
        Containers::arraySize(ParseFloatData));

    addInstancedTests({&StringTest::parseFloatFailed},
        Containers::arraySize(ParseFloatFailedData));

    addTests<StringTest>({
        &StringTest::parseDecimalHexadecimalUnsignedLimits<std::uint8_t>,
        &StringTest::parseDecimalHexadecimalUnsignedLimits<std::uint16_t>,
        &StringTest::parseDecimalHexadecimalUnsignedLimits<std::uint32_t>,
        &StringTest::parseDecimalHexadecimalUnsignedLimits<std::uint64_t>});

    addTests<StringTest>({
        &StringTest::parseDecimalHexadecimalSignedLimits<std::int8_t>,
        &StringTest::parseDecimalHexadecimalSignedLimits<std::int16_t>,
        &StringTest::parseDecimalHexadecimalSignedLimits<std::int32_t>,
        &StringTest::parseDecimalHexadecimalSignedLimits<std::int64_t>});

    addTests({&StringTest::parseDecimalHexadecimalFloatNonNullTerminated,
              &StringTest::parseDecimalHexadecimalInvalid});

    addInstancedTests({&StringTest::parseNumberSequence},
        Containers::arraySize(ParseNumberSequenceData));

    addInstancedTests({&StringTest::parseNumberSequenceOverflow},
        Containers::arraySize(ParseNumberSequenceOverflowData));

    addTests({&StringTest::parseNumberSequenceError});

    #ifdef CORRADE_BUILD_DEPRECATED
    addTests({&StringTest::deprecatedFromArray,
              &StringTest::deprecatedTrim,
              &StringTest::deprecatedTrimInPlace,
              &StringTest::deprecatedSplit,
              &StringTest::deprecatedSplitMultipleCharacters,
              &StringTest::deprecatedPartition,
              &StringTest::deprecatedJoin,

              &StringTest::deprecatedBeginsWith,
              &StringTest::deprecatedBeginsWithEmpty,
              &StringTest::deprecatedViewBeginsWith,
              &StringTest::deprecatedEndsWith,
              &StringTest::deprecatedEndsWithEmpty,
              &StringTest::deprecatedViewEndsWith,

              &StringTest::deprecatedStripPrefix,
              &StringTest::deprecatedStripPrefixInvalid,
              &StringTest::deprecatedStripSuffix,
              &StringTest::deprecatedStripSuffixInvalid});
    #endif
}

using namespace Containers::Literals;

void StringTest::debugParseState() {
    Containers::String out;
    Debug{&out} << String::ParseState::Clamped << String::ParseState(0xef);
    CORRADE_COMPARE(out, "Utility::String::ParseState::Clamped Utility::String::ParseState(0xef)\n");
}

void StringTest::debugParseDecimalFlag() {
    Containers::String out;
    Debug{&out} << String::ParseDecimalFlag::DisallowSign << String::ParseDecimalFlag(0xef);
    CORRADE_COMPARE(out, "Utility::String::ParseDecimalFlag::DisallowSign Utility::String::ParseDecimalFlag(0xef)\n");
}

void StringTest::debugParseDecimalFlags() {
    Containers::String out;
    Debug{&out} << (String::ParseDecimalFlag::DisallowSign|String::ParseDecimalFlag(0xe0)) << String::ParseDecimalFlags{};
    CORRADE_COMPARE(out, "Utility::String::ParseDecimalFlag::DisallowSign|Utility::String::ParseDecimalFlag(0xe0) Utility::String::ParseDecimalFlags{}\n");
}

void StringTest::debugParseHexadecimalFlag() {
    Containers::String out;
    Debug{&out} << String::ParseHexadecimalFlag::AllowHashPrefix << String::ParseHexadecimalFlag(0xef);
    CORRADE_COMPARE(out, "Utility::String::ParseHexadecimalFlag::AllowHashPrefix Utility::String::ParseHexadecimalFlag(0xef)\n");
}

void StringTest::debugParseHexadecimalFlags() {
    Containers::String out;
    Debug{&out} << (String::ParseHexadecimalFlag::DisallowSign|String::ParseHexadecimalFlag::AllowBasePrefix|String::ParseHexadecimalFlag(0xe0)) << String::ParseHexadecimalFlags{};
    CORRADE_COMPARE(out, "Utility::String::ParseHexadecimalFlag::DisallowSign|Utility::String::ParseHexadecimalFlag::AllowBasePrefix|Utility::String::ParseHexadecimalFlag(0xe0) Utility::String::ParseHexadecimalFlags{}\n");
}

void StringTest::debugParseFloatFlag() {
    Containers::String out;
    Debug{&out} << String::ParseFloatFlag::DisallowSign << String::ParseFloatFlag(0xef);
    CORRADE_COMPARE(out, "Utility::String::ParseFloatFlag::DisallowSign Utility::String::ParseFloatFlag(0xef)\n");
}

void StringTest::debugParseFloatFlags() {
    Containers::String out;
    Debug{&out} << (String::ParseFloatFlag::DisallowSign|String::ParseFloatFlag(0xe0)) << String::ParseFloatFlags{};
    CORRADE_COMPARE(out, "Utility::String::ParseFloatFlag::DisallowSign|Utility::String::ParseFloatFlag(0xe0) Utility::String::ParseFloatFlags{}\n");
}

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

    Containers::String out;
    Error redirectOutput{&out};
    String::replaceAll("this completely messed up", "", "got ");
    CORRADE_COMPARE(out, "Utility::String::replaceAll(): empty search string would cause an infinite loop\n");
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

void StringTest::parseResultConstruct() {
    String::ParseResult a = {String::ParseState::Clamped, 1337};
    String::ParseResult b = String::ParseState::Success;
    CORRADE_COMPARE(a.state(), String::ParseState::Clamped);
    CORRADE_COMPARE(b.state(), String::ParseState::Success);
    /* Implicit conversion */
    CORRADE_COMPARE(a, String::ParseState::Clamped);
    CORRADE_COMPARE(b, String::ParseState::Success);
    CORRADE_COMPARE(a.index(), 1337);
    CORRADE_COMPARE(b.index(), 0);
}

void StringTest::parseResultConstructCopy() {
    String::ParseResult a{String::ParseState::Clamped, 1337};
    CORRADE_COMPARE(a.state(), String::ParseState::Clamped);
    CORRADE_COMPARE(a.index(), 1337);

    String::ParseResult b = a;
    CORRADE_COMPARE(b.state(), String::ParseState::Clamped);
    CORRADE_COMPARE(b.index(), 1337);

    String::ParseResult c{String::ParseState::Success};
    c = b;
    CORRADE_COMPARE(c.state(), String::ParseState::Clamped);
    CORRADE_COMPARE(c.index(), 1337);

    CORRADE_VERIFY(std::is_copy_constructible<String::ParseResult>::value);
    CORRADE_VERIFY(std::is_copy_assignable<String::ParseResult>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<String::ParseResult>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<String::ParseResult>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<String::ParseResult>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<String::ParseResult>::value);
}

void StringTest::parseDecimalUnsigned() {
    auto&& data = ParseDecimalUnsignedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint64_t value;
    String::ParseResult result = data.min ?
        String::parseDecimal(data.string, value, *data.min, *data.max) :
        String::parseDecimal(data.string, value);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(data.state, std::size_t{}));
    CORRADE_COMPARE(value, data.value);
}

void StringTest::parseDecimalUnsignedFailed() {
    auto&& data = ParseDecimalUnsignedFailedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint64_t value;
    String::ParseResult result = String::parseDecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(String::ParseState::Failed, data.expected));

    /* The failure index should point either to string end or to a non-numeric
       character inside, numeric characters can never be a failure */
    CORRADE_VERIFY(result.index() <= Containers::StringView{data.string}.size());
    if(data.string) {
        CORRADE_ITERATION(data.string);
        CORRADE_FAIL_IF(
            result.index() != Containers::StringView{data.string}.size() &&
            (data.string[result.index()] >= '0' && data.string[result.index()] <= '9'),
            "Failure points to an unexpected character" << (Containers::StringView{data.string + result.index(), 1}) << "at index" << result.index());
    }
}

void StringTest::parseDecimalSigned() {
    auto&& data = ParseDecimalSignedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::int64_t value;
    String::ParseResult result = data.min ?
        String::parseDecimal(data.string, value, *data.min, *data.max) :
        String::parseDecimal(data.string, value);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(data.state, std::size_t{}));
    CORRADE_COMPARE(value, data.value);
}

void StringTest::parseDecimalSignedFailed() {
    auto&& data = ParseDecimalSignedFailedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::int64_t value;
    String::ParseResult result = String::parseDecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(String::ParseState::Failed, data.expected));

    /* The failure index should point either to string end or to a non-numeric
       character inside, numeric characters can never be a failure */
    CORRADE_VERIFY(result.index() <= Containers::StringView{data.string}.size());
    if(data.string) {
        CORRADE_ITERATION(data.string);
        CORRADE_FAIL_IF(
            result.index() != Containers::StringView{data.string}.size() &&
            (data.string[result.index()] >= '0' && data.string[result.index()] <= '9'),
            "Failure points to an unexpected character" << (Containers::StringView{data.string + result.index(), 1}) << "at index" << result.index());
    }
}

void StringTest::parseHexadecimalUnsigned() {
    auto&& data = ParseHexadecimalUnsignedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint64_t value;
    String::ParseResult result = data.min ?
        String::parseHexadecimal(data.string, value, *data.min, *data.max, data.flags) :
        String::parseHexadecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(data.state, std::size_t{}));
    CORRADE_COMPARE(value, data.value);
}

void StringTest::parseHexadecimalUnsignedFailed() {
    auto&& data = ParseHexadecimalUnsignedFailedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::uint64_t value;
    String::ParseResult result = String::parseHexadecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(String::ParseState::Failed, data.expected));

    /* The failure index should point either to string end or to a non-hex
       character inside, hex characters can never be a failure */
    CORRADE_VERIFY(result.index() <= Containers::StringView{data.string}.size());
    if(data.string) {
        CORRADE_ITERATION(data.string);
        CORRADE_FAIL_IF(
            result.index() != Containers::StringView{data.string}.size() &&
            ((data.string[result.index()] >= '0' && data.string[result.index()] <= '9') ||
            (data.string[result.index()] >= 'a' && data.string[result.index()] <= 'f') ||
            (data.string[result.index()] >= 'A' && data.string[result.index()] <= 'F')),
            "Failure points to an unexpected character" << (Containers::StringView{data.string + result.index(), 1}) << "at index" << result.index());
    }
}

void StringTest::parseHexadecimalSigned() {
    auto&& data = ParseHexadecimalSignedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::int64_t value;
    String::ParseResult result = data.min ?
        String::parseHexadecimal(data.string, value, *data.min, *data.max, data.flags) :
        String::parseHexadecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(data.state, std::size_t{}));
    CORRADE_COMPARE(value, data.value);
}

void StringTest::parseHexadecimalSignedFailed() {
    auto&& data = ParseHexadecimalSignedFailedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::int64_t value;
    String::ParseResult result = String::parseHexadecimal(data.string, value, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(String::ParseState::Failed, data.expected));

    /* The failure index should point either to string end or to a non-hex
       character inside, hex characters can never be a failure */
    CORRADE_VERIFY(result.index() <= Containers::StringView{data.string}.size());
    if(data.string) {
        CORRADE_ITERATION(data.string);
        CORRADE_FAIL_IF(
            result.index() != Containers::StringView{data.string}.size() &&
            ((data.string[result.index()] >= '0' && data.string[result.index()] <= '9') ||
            (data.string[result.index()] >= 'a' && data.string[result.index()] <= 'f') ||
            (data.string[result.index()] >= 'A' && data.string[result.index()] <= 'F')),
            "Failure points to an unexpected character" << (Containers::StringView{data.string + result.index(), 1}) << "at index" << result.index());
    }
}

void StringTest::parseFloat() {
    auto&& data = ParseFloatData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    float value;
    double valueDouble;
    String::ParseResult result = String::parseFloat(data.string, value);
    String::ParseResult resultDouble = String::parseFloat(data.string, valueDouble);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(data.state, std::size_t{}));
    CORRADE_COMPARE(Containers::pair(resultDouble.state(), resultDouble.index()), Containers::pair(data.stateDouble, std::size_t{}));
    CORRADE_COMPARE(value, data.value);
    CORRADE_COMPARE(valueDouble, data.valueDouble);
}

void StringTest::parseFloatFailed() {
    auto&& data = ParseFloatFailedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    float value;
    double valueDouble;
    String::ParseResult result = String::parseFloat(data.string, value, data.flags);
    String::ParseResult resultDouble = String::parseFloat(data.string, valueDouble, data.flags);
    CORRADE_COMPARE(Containers::pair(result.state(), result.index()), Containers::pair(String::ParseState::Failed, data.expected));
    CORRADE_COMPARE(Containers::pair(resultDouble.state(), resultDouble.index()), Containers::pair(String::ParseState::Failed, data.expected));

    /* The failure index should point either to string end or to a non-numeric,
       non-exponent or non-decimal-point character inside, those can never be a
       failure */
    CORRADE_VERIFY(result.index() <= Containers::StringView{data.string}.size());
    if(data.string) {
        CORRADE_ITERATION(data.string);
        CORRADE_FAIL_IF(
            result.index() != Containers::StringView{data.string}.size() &&
            ((data.string[result.index()] >= '0' && data.string[result.index()] <= '9') ||
             /** @todo strtof() points to the `e` / `E` if there's a space
                 after, so that's currently accepted, update once we have saner
                 parsing */
             data.string[result.index()] == '.'),
            "Failure points to an unexpected character" << (Containers::StringView{data.string + result.index(), 1}) << "at index" << result.index());
    }
}

template<class T> struct ParseLimitsTraits;
template<> struct ParseLimitsTraits<std::uint8_t> {
    static const char* name() { return "std::uint8_t"; }
};
template<> struct ParseLimitsTraits<std::int8_t> {
    static const char* name() { return "std::int8_t"; }
};
template<> struct ParseLimitsTraits<std::uint16_t> {
    static const char* name() { return "std::uint16_t"; }
};
template<> struct ParseLimitsTraits<std::int16_t> {
    static const char* name() { return "std::int16_t"; }
};
template<> struct ParseLimitsTraits<std::uint32_t> {
    static const char* name() { return "std::uint32_t"; }
};
template<> struct ParseLimitsTraits<std::int32_t> {
    static const char* name() { return "std::int32_t"; }
};
template<> struct ParseLimitsTraits<std::uint64_t> {
    static const char* name() { return "std::uint64_t"; }
};
template<> struct ParseLimitsTraits<std::int64_t> {
    static const char* name() { return "std::int64_t"; }
};

template<class T> void StringTest::parseDecimalHexadecimalUnsignedLimits() {
    setTestCaseTemplateName(ParseLimitsTraits<T>::name());

    /* The cast should produce a max representable value for given bit width */
    const T value = T(~std::uint64_t{});
    Containers::String string = Utility::format("{}", value);
    Containers::String hexString = Utility::format("{:x}", value);

    /* Parsing exactly the limit succeeds */
    T actual;
    CORRADE_COMPARE(String::parseDecimal(string, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, value);
    CORRADE_COMPARE(String::parseHexadecimal(hexString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, value);

    /* Verify that flags are propagated correctly in all overloads */
    CORRADE_COMPARE(String::parseDecimal("+" + string, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, value);
    CORRADE_COMPARE(String::parseHexadecimal("+" + hexString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, value);
    CORRADE_COMPARE(String::parseDecimal("+" + string, actual, String::ParseDecimalFlag::DisallowSign), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseHexadecimal("+" + hexString, actual, String::ParseHexadecimalFlag::DisallowSign), String::ParseState::Failed);

    /* A value larger than the limit clamps to the limit. For decimal strings,
       the last character is always a number less than 9, so incrementing it by
       one works. For hexadecimal strings, the value is something like ffffffff
       so it has to be about prepending 1 and replacing all fs with 0s. */
    string.back() += 1;
    hexString = "1" + hexString;
    String::replaceAllInPlace(hexString, 'f', '0');
    CORRADE_COMPARE(String::parseDecimal(string, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, value);
    CORRADE_COMPARE(String::parseHexadecimal(hexString, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, value);
}

template<class T> void StringTest::parseDecimalHexadecimalSignedLimits() {
    setTestCaseTemplateName(ParseLimitsTraits<T>::name());

    /* These should produce a min and max representable value for given bit
       width */
    const T min = T(1ull << (sizeof(T)*8 - 1));
    const T max = T((1ull << (sizeof(T)*8 - 1)) - 1);
    CORRADE_VERIFY(min < 0 && max > 0);
    Containers::String minString = Utility::format("{}", min);
    /* Printing negative hexadecimal numbers is impossible with the STL */
    /** @todo clean up once we have our own printing routine as well, FFS */
    Containers::String minHexString = Utility::format("-{:x}", std::uint64_t(max) + 1);
    Containers::String maxString = Utility::format("{}", max);
    Containers::String maxHexString = Utility::format("{:x}", max);

    /* Parsing exactly the limit succeeds */
    T actual;
    CORRADE_COMPARE(String::parseDecimal(minString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, min);
    CORRADE_COMPARE(String::parseHexadecimal(minHexString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, min);
    CORRADE_COMPARE(String::parseDecimal(maxString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, max);
    CORRADE_COMPARE(String::parseHexadecimal(maxHexString, actual), String::ParseState::Success);
    CORRADE_COMPARE(actual, max);

    /* Verify that flags are propagated correctly in all overloads */
    CORRADE_COMPARE(String::parseDecimal(minString, actual, String::ParseDecimalFlag::DisallowSign), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseHexadecimal(minHexString, actual, String::ParseHexadecimalFlag::DisallowSign), String::ParseState::Failed);

    /* A value larger than the limit clamps to the limit. For decimal strings,
       the last character is always a number less than 9, so incrementing it by
       one works. For hexadecimal strings, the min value is something like
       -100000, so incrementing the last char works as well. The max value is
       then something like 7ffffffff so it has to be about incrementing the
       first and replacing all fs with 0s. */
    minString.back() += 1;
    minHexString.back() += 1;
    maxString.back() += 1;
    maxHexString.front() += 1;
    String::replaceAllInPlace(maxHexString, 'f', '0');
    CORRADE_COMPARE(String::parseDecimal(minString, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, min);
    CORRADE_COMPARE(String::parseHexadecimal(minHexString, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, min);
    CORRADE_COMPARE(String::parseDecimal(maxString, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, max);
    CORRADE_COMPARE(String::parseHexadecimal(maxHexString, actual), String::ParseState::Clamped);
    CORRADE_COMPARE(actual, max);
}

void StringTest::parseDecimalHexadecimalFloatNonNullTerminated() {
    std::uint64_t valueUnsigned;
    std::int64_t valueSigned;
    std::uint64_t valueHexUnsigned;
    std::int64_t valueHexSigned;
    float valueFloat;
    double valueDouble;

    /* Parsing this should not leak over to the 3s at the end */
    Containers::StringView nonNullTerminated = "999333"_s.prefix(3);
    CORRADE_COMPARE(String::parseDecimal(nonNullTerminated, valueUnsigned), String::ParseState::Success);
    CORRADE_COMPARE(String::parseDecimal(nonNullTerminated, valueSigned), String::ParseState::Success);
    CORRADE_COMPARE(String::parseHexadecimal(nonNullTerminated, valueHexUnsigned), String::ParseState::Success);
    CORRADE_COMPARE(String::parseHexadecimal(nonNullTerminated, valueHexSigned), String::ParseState::Success);
    CORRADE_COMPARE(String::parseFloat(nonNullTerminated, valueFloat), String::ParseState::Success);
    CORRADE_COMPARE(String::parseFloat(nonNullTerminated, valueDouble), String::ParseState::Success);
    CORRADE_COMPARE(valueUnsigned, 999);
    CORRADE_COMPARE(valueSigned, 999);
    CORRADE_COMPARE(valueHexUnsigned, 0x999);
    CORRADE_COMPARE(valueHexSigned, 0x999);
    CORRADE_COMPARE(valueFloat, 999.0f);
    CORRADE_COMPARE(valueDouble, 999.0);

    /* Parsing this should not just abort at the null terminator. In other
       words, this would pass if the string length wouldn't be correctly
       propagated all the way. Have to split in two literals because FUCKING C
       understands that as octal 03, ugh. */
    Containers::StringView nullInTheMiddle = "999\0" "333"_s;
    CORRADE_COMPARE(String::parseDecimal(nullInTheMiddle, valueUnsigned), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseDecimal(nullInTheMiddle, valueSigned), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseHexadecimal(nullInTheMiddle, valueHexUnsigned), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseHexadecimal(nullInTheMiddle, valueHexSigned), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseFloat(nullInTheMiddle, valueFloat), String::ParseState::Failed);
    CORRADE_COMPARE(String::parseFloat(nullInTheMiddle, valueDouble), String::ParseState::Failed);
}

void StringTest::parseDecimalHexadecimalInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* A single-value range is fine */
    std::uint64_t valueUnsigned;
    std::int64_t valueSigned;
    String::parseDecimal("222", valueUnsigned, 35, 35);
    String::parseDecimal("333", valueSigned, 36, 36);
    String::parseHexadecimal("22", valueUnsigned, 35, 35);
    String::parseHexadecimal("33", valueSigned, 36, 36);

    Containers::String out;
    Error redirectError{&out};
    String::parseDecimal("35", valueUnsigned, 36, 35);
    String::parseDecimal("36", valueSigned, 37, 36);
    String::parseHexadecimal("35", valueUnsigned, 36, 35);
    String::parseHexadecimal("36", valueSigned, 37, 36);
    CORRADE_COMPARE_AS(out,
        "Utility::String::parseDecimal(): expected min to be not greater than max but got 36 and 35\n"
        "Utility::String::parseDecimal(): expected min to be not greater than max but got 37 and 36\n"
        "Utility::String::parseHexadecimal(): expected min to be not greater than max but got 36 and 35\n"
        "Utility::String::parseHexadecimal(): expected min to be not greater than max but got 37 and 36\n",
        TestSuite::Compare::String);
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
    Containers::String out;
    Error redirectError{&out};
    String::parseNumberSequence("3,5y7,x,25", 0, ~std::uint32_t{});
    CORRADE_COMPARE(out, "Utility::parseNumberSequence(): unrecognized character y in 3,5y7,x,25\n");
}

#ifdef CORRADE_BUILD_DEPRECATED
CORRADE_IGNORE_DEPRECATED_PUSH
void StringTest::deprecatedFromArray() {
    CORRADE_COMPARE(String::fromArray(nullptr), "");
    CORRADE_COMPARE(String::fromArray(nullptr, 37), "");

    CORRADE_COMPARE(String::fromArray("abc\0def"), "abc");
    CORRADE_COMPARE(String::fromArray("abc\0def", 7), std::string("abc\0def", 7));
}

void StringTest::deprecatedTrim() {
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

void StringTest::deprecatedTrimInPlace() {
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

void StringTest::deprecatedSplit() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes. The explicit cast to avoid an ambiguous
       overload is kinda nasty, but since this is deprecated, I don't care
       anymore. */

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

void StringTest::deprecatedSplitMultipleCharacters() {
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

void StringTest::deprecatedPartition() {
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

void StringTest::deprecatedJoin() {
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

void StringTest::deprecatedBeginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(String::beginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", std::string{"over"}));

    CORRADE_VERIFY(!String::beginsWith("overcomplicated", "oven"));
    CORRADE_VERIFY(!String::beginsWith("overcomplicated", std::string{"oven"}));

    CORRADE_VERIFY(String::beginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::beginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::beginsWith("", 'h'));
}

void StringTest::deprecatedBeginsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(!String::beginsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::beginsWith("", ""));
}

void StringTest::deprecatedViewBeginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(String::viewBeginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(!String::viewBeginsWith("overcomplicated", "oven"));

    CORRADE_VERIFY(String::viewBeginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::viewBeginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewBeginsWith("", 'h'));
}

void StringTest::deprecatedEndsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(String::endsWith("overcomplicated", "complicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", std::string{"complicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "somplicated"));
    CORRADE_VERIFY(!String::endsWith("overcomplicated", std::string{"somplicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "overcomplicated even more"));

    CORRADE_VERIFY(!String::endsWith("hello", 'h'));
    CORRADE_VERIFY(String::endsWith("hello", 'o'));
    CORRADE_VERIFY(!String::endsWith("", 'h'));
}

void StringTest::deprecatedEndsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(!String::endsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::endsWith("", ""));
}

void StringTest::deprecatedViewEndsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes */

    CORRADE_VERIFY(String::viewEndsWith({"overcomplicated", 15}, "complicated"));
    CORRADE_VERIFY(!String::viewEndsWith("overcomplicated", "complicated"));

    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "somplicated"));
    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "overcomplicated even more"));

    CORRADE_VERIFY(!String::viewEndsWith({"hello", 5}, 'h'));
    CORRADE_VERIFY(String::viewEndsWith({"hello", 5}, 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("", 'h'));
}

void StringTest::deprecatedStripPrefix() {
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", "over"), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", std::string{"over"}), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", 'o'), "vercomplicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", ""), "overcomplicated");
}

void StringTest::deprecatedStripPrefixInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectOutput{&out};
    String::stripPrefix("overcomplicated", "complicated");
    CORRADE_COMPARE(out, "Utility::String::stripPrefix(): string doesn't begin with given prefix\n");
}

void StringTest::deprecatedStripSuffix() {
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", "complicated"), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", std::string{"complicated"}), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", 'd'), "overcomplicate");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", ""), "overcomplicated");
}

void StringTest::deprecatedStripSuffixInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectOutput{&out};
    String::stripSuffix("overcomplicated", "over");
    CORRADE_COMPARE(out, "Utility::String::stripSuffix(): string doesn't end with given suffix\n");
}
CORRADE_IGNORE_DEPRECATED_POP
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
