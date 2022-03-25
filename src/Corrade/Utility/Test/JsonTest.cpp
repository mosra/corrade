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

#include <sstream>

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/Utility/FormatStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/Utility/Json.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

using namespace Containers::Literals;

struct JsonTest: TestSuite::Tester {
    public:
        explicit JsonTest();

        void singleObject();
        void singleArray();
        void singleNull();
        void singleBoolean();
        void singleNumber();
        void singleString();

        void simpleObject();
        void simpleArray();
        void nested();

        void error();

        void parseNull();
        void parseNulls();
        void parseBool();
        void parseBools();
        void parseDouble();
        void parseDoubles();
        void parseFloat();
        void parseFloats();
        void parseUnsignedInt();
        void parseUnsignedInts();
        void parseInt();
        void parseInts();
        void parseUnsignedLong();
        void parseUnsignedLongs();
        void parseLong();
        #ifndef CORRADE_TARGET_32BIT
        void parseLongs();
        #endif
        void parseSize();
        void parseSizes();
        void parseString();
        void parseStringKeys();
        void parseStrings();

        void parseOption();
        void parseSubtree();
        void reparseNumberDifferentType();

        void parseError();
        void parseOptionError();
        void parseDirectError();
        void parseTokenNotOwned();

        void file();
        void fileReadError();
        void fileOptionReadError();
        void fileError();
        void fileParseOptionError();
        void fileParseError();

        void asTypeWrongType();
        void asTypeNotParsed();
        void asTypeWrongParsedType();

        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        void tokenConstructCopy();
        #endif
        void constructCopy();
        void constructMove();

        void debugTokenType();
        void debugTokenParsedType();
};

const struct {
    const char* name;
    const char* data;
    const char* message;
} ErrorData[] {
    {"empty",
        " \n\r \n \t\t  ",
        "file too short, expected a value at <in>:3:6"},
    {"object end alone",
        "\n} ",
        "expected a value but got } at <in>:2:1"},
    {"array end alone",
        "]",
        "expected a value but got ] at <in>:1:1"},
    {"object end after array",
        "\n   [ \n\n } ",
        "unexpected } at <in>:4:2 for an array starting at <in>:2:4"},
    {"array end after object",
        "\n   { \n\n ] ",
        "unexpected ] at <in>:4:2 for an object starting at <in>:2:4"},
    {"number as a key",
        "{\n    5:",
        "expected \" or } but got 5 at <in>:2:5"},
    {"object as a key",
        "{\n    {",
        "expected \" or } but got { at <in>:2:5"},
    {"object end after key",
        "{\n  \"hello\"\n}",
        "expected : but got } at <in>:3:1"},
    {"object end after colon",
        "{\n  \"hello\":\n}",
        "expected a value but got } at <in>:3:1"},
    {"misplaced colon",
        "{\n  \"key\" \"value\":",
        "expected : but got \" at <in>:2:9"},
    {"nothing after key",
        "{\n  \"hello\"\n",
        "file too short, expected : at <in>:3:1"},
    {"nothing after colon",
        "{\n  \"hello\":\n",
        "file too short, expected a value at <in>:3:1"},
    {"nothing after object value",
        "\n   {  \"hello\": 3\n",
        "file too short, expected closing } for object starting at <in>:2:4"},
    {"nothing after array value",
        "\n   [  3\n",
        "file too short, expected closing ] for array starting at <in>:2:4"},
    {"stray comma before object end",
        "{\"hello\": 3,\n   }",
        "expected \" but got } at <in>:2:4"},
    {"stray comma before array end",
        "[3,\n   ]",
        "expected a value but got ] at <in>:2:4"},
    {"colon after object value",
        "{\n  \"hello\": \"hi\":",
        "expected , or } but got : at <in>:2:16"},
    {"colon in an array",
        "[\n  \"hello\":",
        "expected , or ] but got : at <in>:2:10"},
    {"\\v in a string",
        "  \n\"vertical\n \\vtab\n\" ",
        "unexpected string escape sequence \\v at <in>:3:2"},
    {"unterminated string",
        "  \n\"hello!! \n\\\" ",
        "file too short, unterminated string literal starting at <in>:2:1"},
    {"comment",
        "\n\n    /* JSON, wake up! */",
        "unexpected / at <in>:3:5"},
    /* supported by strtod(), but not by JSON, so it should fail. OTOH, -.5e5
       will fail only later during parse */
    {"fractional number without a leading zero",
        "\n\n\t  .5e5",
        "unexpected . at <in>:3:4"},
    /* supported by strto*(), but not by JSON, so it should fail */
    {"explicitly positive number",
        "\n\n\t  +1.5",
        "unexpected + at <in>:3:4"},
    /* supported by strtod(), but not by JSON, so it should fail. OTOH, -INF
       will fail only later during parse */
    {"INF",
        "\n\n\t  INF",
        "unexpected I at <in>:3:4"},
    /* supported by strtod(), but not by JSON, so it should fail. OTOH, -INF
       will fail only later during parse */
    {"NAN",
        "\n\n\t  NAN",
        "unexpected N at <in>:3:4"},
    {"BOM", /** @todo handle this gracefully? */
        "\xef\xbb\xbf",
        "unexpected \xef at <in>:1:1"},
    {"comma after a root literal",
        "false,",
        "expected document end but got , at <in>:1:6"},
    {"comma after a root number",
        "56,",
        "expected document end but got , at <in>:1:3"},
    {"comma after a root string",
        "\"hey\",",
        "expected document end but got , at <in>:1:6"},
    {"comma after a root object",
        "{},",
        "expected document end but got , at <in>:1:3"},
    {"comma after a root array",
        "[],",
        "expected document end but got , at <in>:1:3"},
};

const struct {
    const char* name;
    const char* json;
    bool expected;
} ParseBoolData[]{
    {"true", "true", true},
    {"false", "false", false}
};

const struct {
    const char* name;
    const char* json;
    double expected;
} ParseDoubleOrFloatData[]{
    {"", "35.7", 35.7},
    {"negative", "-35.7", -35.7},
    {"negative zero", "-0", -0.0}, /** @todo check this more precisely */
    {"exponent", "-3550.0e-2", -35.5},
    {"exponent uppercase", "-35.5E2", -3550},
    {"exponent explicit plus", "-35.5E+2", -3550},
    {"127 characters", "1234.56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456", 1234.567890123456789}
};

const struct {
    const char* name;
    const char* json;
    std::uint32_t expected;
} ParseUnsignedIntData[]{
    {"", "357", 357},
    {"zero", "0", 0},
    {"max value", "4294967295", 4294967295},
    /* Can't test 127 characters as that wouldn't fit */
};

const struct {
    const char* name;
    const char* json;
    std::int32_t expected;
} ParseIntData[]{
    {"", "357", 357},
    {"negative", "-464", -464},
    {"min value", "-2147483648", -2147483648},
    {"max value", "2147483647", 2147483647}
    /* Can't test 127 characters as that wouldn't fit */
};

const struct {
    const char* name;
    const char* json;
    std::uint64_t expected;
} ParseUnsignedLongData[]{
    {"", "357", 357},
    {"zero", "0", 0},
    {"max 52bit value", "4503599627370495", 4503599627370495ull}
    /* Can't test 127 characters as that wouldn't fit */
};

const struct {
    const char* name;
    const char* json;
    std::int64_t expected;
} ParseLongData[]{
    {"", "357", 357},
    {"negative", "-464", -464},
    {"min 53bit value", "-4503599627370496", -4503599627370496ll},
    {"max 53bit value", "4503599627370495", 4503599627370495ll}
    /* Can't test 127 characters as that wouldn't fit */
};

const struct {
    const char* name;
    const Containers::StringView json;
    const Containers::StringView expected;
} ParseStringData[]{
    {"",
        "\"hello!\"",
        "hello!"},
    {"empty",
        "\"\"",
        ""},
    {"escapes",
        "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"",
        "\"\\/\b\f\n\r\t"},
    /* Unicode escapes deliberately not supported right now */
    /** @todo handle also surrogate pairs, add a helper to the Unicode lib
        first https://en.wikipedia.org/wiki/JSON#Character_encoding */
    {"SSO string with escapes",
        "\"\\\\\"",
        "\\"},
    {"non-SSO string with escapes",
        "\"this is a very long escaped\\nstring, \\\"yes\\\"!\"",
        "this is a very long escaped\nstring, \"yes\"!"},
    {"global literal",
        "\"hello!\""_s,
        "hello!"_s},
};

const struct {
    const char* name;
    Json::Option option;
    std::size_t tokenParsed, tokenParsed2, tokenNotParsedCount;
    JsonToken::ParsedType parsedType;
    const char* tokenData;
} ParseOptionData[]{
    {"nulls", Json::Option::ParseLiterals,
        2, 8, 17 - 4,
        JsonToken::ParsedType::Other, "null"},
    {"bools", Json::Option::ParseLiterals,
        4, 10, 17 - 4,
        JsonToken::ParsedType::Other, "true"},
    {"doubles", Json::Option::ParseDoubles,
        12, 16, 17 - 2,
        JsonToken::ParsedType::Double, "35"},
    {"floats", Json::Option::ParseFloats,
        12, 16, 17 - 2,
        JsonToken::ParsedType::Float, "35"},
    {"string keys", Json::Option::ParseStringKeys,
        17, 13, 17 - 9,
        JsonToken::ParsedType::Other, "\"string\""},
    {"strings", Json::Option::ParseStrings,
        18, 14, 17 - 11,
        JsonToken::ParsedType::Other, "\"hello\""},
};

const struct {
    const char* name;
    bool(Json::*function)(const JsonToken&);
    std::size_t parseRoot;
    std::size_t tokenParsed, tokenParsedDeep, tokenNotParsed, tokenNotParsedCount;
    JsonToken::ParsedType parsedType;
    const char* tokenData;
} ParseSubtreeData[]{
    {"nulls", &Json::parseLiterals, 1,
        3, 9, 22, 21 - 4,
        JsonToken::ParsedType::Other, "null"},
    {"bools", &Json::parseLiterals, 1,
        5, 11, 23, 21 - 4,
        JsonToken::ParsedType::Other, "true"},
    {"doubles", &Json::parseDoubles, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::Double, "35"},
    {"floats", &Json::parseFloats, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::Float, "35"},
    {"unsigned ints", &Json::parseUnsignedInts, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::UnsignedInt, "35"},
    {"ints", &Json::parseInts, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::Int, "35"},
    {"unsigned longs", &Json::parseUnsignedLongs, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::UnsignedLong, "35"},
    #ifndef CORRADE_TARGET_32BIT
    {"longs", &Json::parseLongs, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::Long, "35"},
    #endif
    {"sizes", &Json::parseSizes, 1,
        18, 13, 24, 21 - 2,
        JsonToken::ParsedType::Size, "35"},
    {"string keys", &Json::parseStringKeys, 6,
        6, 14, 19, 21 - 5,
        JsonToken::ParsedType::Other, "\"nested\""},
    {"strings", &Json::parseStrings, 1,
        21, 16, 25, 21 - 11,
        JsonToken::ParsedType::Other, "\"hello\""}
};

const struct {
    const char* name;
    bool(Json::*function)(const JsonToken&);
    Containers::StringView json; /* testing \0 bytes in strings */
    const char* message;
} ParseErrorData[]{
    {"invalid null literal", &Json::parseLiterals,
        "no!",
        "parseLiterals(): invalid null literal no!"},
    {"invalid true literal", &Json::parseLiterals,
        "toomuch",
        "parseLiterals(): invalid bool literal toomuch"},
    {"invalid false literal", &Json::parseLiterals,
        "foe",
        "parseLiterals(): invalid bool literal foe"},
    {"double literal too long", &Json::parseDoubles,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseDoubles(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    {"float literal too long", &Json::parseFloats,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseFloats(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    {"unsigned int literal too long", &Json::parseUnsignedInts,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseUnsignedInts(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    {"int literal too long", &Json::parseInts,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseInts(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    {"unsigned long literal too long", &Json::parseUnsignedLongs,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseUnsignedLongs(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    #ifndef CORRADE_TARGET_32BIT
    {"long literal too long", &Json::parseLongs,
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
        "parseLongs(): too long numeric literal 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"},
    #endif
    {"invalid double literal", &Json::parseDoubles,
        "78.5x",
        "parseDoubles(): invalid floating-point literal 78.5x"},
    {"invalid float literal", &Json::parseFloats,
        "78.5x",
        "parseFloats(): invalid floating-point literal 78.5x"},
    {"invalid unsigned integer literal", &Json::parseUnsignedInts,
        "78x",
        "parseUnsignedInts(): invalid unsigned integer literal 78x"},
    {"invalid integer literal", &Json::parseInts,
        "-78x",
        "parseInts(): invalid integer literal -78x"},
    {"invalid unsigned long literal", &Json::parseUnsignedLongs,
        "78x",
        "parseUnsignedLongs(): invalid unsigned integer literal 78x"},
    #ifndef CORRADE_TARGET_32BIT
    {"invalid long literal", &Json::parseLongs,
        "-78x",
        "parseLongs(): invalid integer literal -78x"},
    #endif
    {"unsigned integer literal with an exponent", &Json::parseUnsignedInts,
        "78e5",
        "parseUnsignedInts(): invalid unsigned integer literal 78e5"},
    {"integer literal with an exponent", &Json::parseInts,
        "78e5",
        "parseInts(): invalid integer literal 78e5"},
    {"unsigned long literal with an exponent", &Json::parseUnsignedLongs,
        "78e5",
        "parseUnsignedLongs(): invalid unsigned integer literal 78e5"},
    #ifndef CORRADE_TARGET_32BIT
    {"long literal with an exponent", &Json::parseLongs,
        "78e5",
        "parseLongs(): invalid integer literal 78e5"},
    #endif
    {"unsigned integer literal with a period", &Json::parseUnsignedInts,
        "78.0",
        "parseUnsignedInts(): invalid unsigned integer literal 78.0"},
    {"integer literal with a period", &Json::parseInts,
        "78.0",
        "parseInts(): invalid integer literal 78.0"},
    {"unsigned long literal with a period", &Json::parseUnsignedLongs,
        "78.0",
        "parseUnsignedLongs(): invalid unsigned integer literal 78.0"},
    #ifndef CORRADE_TARGET_32BIT
    {"long literal with a period", &Json::parseLongs,
        "78.0",
        "parseLongs(): invalid integer literal 78.0"},
    #endif
    {"unsigned integer literal with a minus", &Json::parseUnsignedInts,
        "-78",
        /** @todo what the fuck stroul(), returning 18446744073709551538?! */
        "parseUnsignedInts(): too large integer literal -78"},
    {"unsigned long literal with a minus", &Json::parseUnsignedLongs,
        "-78",
        /** @todo what the fuck stroull(), returning 18446744073709551538?! */
        "parseUnsignedLongs(): too large integer literal -78"},
    /* std::strtoull() returns 1 in this case, very useful */
    /** @todo fix once we have our own parsing routines */
    {"large unsigned long literal with a minus", &Json::parseUnsignedLongs,
        "-18446744073709551615",
        nullptr},
    {"unsigned integer literal too large", &Json::parseUnsignedInts,
        "4294967296",
        "parseUnsignedInts(): too large integer literal 4294967296"},
    {"integer literal too small", &Json::parseInts,
        "-2147483649",
        "parseInts(): too small or large integer literal -2147483649"},
    {"integer literal too large", &Json::parseInts,
        "2147483648",
        "parseInts(): too small or large integer literal 2147483648"},
    {"unsigned long literal too large", &Json::parseUnsignedLongs,
        "4503599627370496",
        "parseUnsignedLongs(): too large integer literal 4503599627370496"},
    #ifndef CORRADE_TARGET_32BIT
    {"long literal too small", &Json::parseLongs,
        "-4503599627370497",
        "parseLongs(): too small or large integer literal -4503599627370497"},
    {"long literal too large", &Json::parseLongs,
        "4503599627370496",
        "parseLongs(): too small or large integer literal 4503599627370496"},
    #endif
    /* NAN or INF without a leading - fails during parse already */
    {"negative double INF literal", &Json::parseDoubles,
        "-INF",
        /* *Has to* be handled on 32bit to avoid clashing with the NaN bit
           pattern reusal, not done on 64bit for perf reasons -- will be fixed
           once we have our own parsing routines */
        #ifndef CORRADE_TARGET_32BIT
        nullptr
        #else
        "parseDoubles(): invalid floating-point literal -INF"
        #endif
    },
    {"negative float INF literal", &Json::parseFloats,
        "-INF",
        nullptr},
    {"negative double NaN literal", &Json::parseDoubles,
        "-NAN",
        /* *Has to* be handled on 32bit to avoid clashing with the NaN bit
           pattern reusal, not done on 64bit for perf reasons -- will be fixed
           once we have our own parsing routines */
        #ifndef CORRADE_TARGET_32BIT
        nullptr
        #else
        "parseDoubles(): invalid floating-point literal -NAN"
        #endif
    },
    {"negative float NaN literal", &Json::parseFloats,
        "-NAN",
        nullptr},
    /* Those should fail but unfortunately they don't */
    /** @todo fix once we have our own parsing routines, checking
        post-conversion would be an unnecessary maintenance overhead */
    {"double literal with leading zero", &Json::parseDoubles,
        "01.5",
        nullptr},
    {"float literal with leading zero", &Json::parseFloats,
        "-01.5",
        nullptr},
    /* Leading + and leading period fails the initial tokenization already */
    {"negative double literal with leading period", &Json::parseDoubles,
        "-.5",
        nullptr},
    {"negative float literal with leading period", &Json::parseFloats,
        "-.5",
        nullptr},
    {"double literal with trailing period", &Json::parseDoubles,
        "-1.",
        nullptr},
    {"float literal with trailing period", &Json::parseFloats,
        "1.",
        nullptr},
    {"unsigned int literal with leading zero", &Json::parseUnsignedInts,
        "045",
        nullptr},
    {"int literal with leading zero", &Json::parseInts,
        "-045",
        nullptr},
    {"unsigned long literal with leading zero", &Json::parseUnsignedLongs,
        "045",
        nullptr},
    #ifndef CORRADE_TARGET_32BIT
    {"long literal with leading zero", &Json::parseLongs,
        "-045",
        nullptr},
    #endif
    {"hexadecimal double literal", &Json::parseDoubles,
        "0x355P6",
        nullptr},
    {"hexadecimal float literal", &Json::parseDoubles,
        "0X35p-6",
        nullptr},
    {"hexadecimal unsigned int literal", &Json::parseUnsignedInts,
        "0xabc",
        "parseUnsignedInts(): invalid unsigned integer literal 0xabc"},
    {"hexadecimal int literal", &Json::parseInts,
        "-0XABC",
        "parseInts(): invalid integer literal -0XABC"},
    {"hexadecimal unsigned long literal", &Json::parseUnsignedLongs,
        "0XABC",
        "parseUnsignedLongs(): invalid unsigned integer literal 0XABC"},
    #ifndef CORRADE_TARGET_32BIT
    {"hexadecimal long literal", &Json::parseLongs,
        "-0xabc",
        "parseLongs(): invalid integer literal -0xabc"},
    #endif
    {"invalid unicode escape", &Json::parseStrings,
        "\"\\undefined\"",
        "parseStrings(): sorry, unicode escape sequences are not implemented yet"},
    /** @todo test for \u alone, or just 3 chars, invalid surrogates */
    /* These are deliberately not handled at the moment */
    {"zero byte", &Json::parseStrings,
        "\"\0\""_s,
        nullptr},
    {"unescaped newline", &Json::parseStrings,
        "\"\n\""_s, /** @todo probably also others? */
        nullptr},
    {"wrong start of a UTF-8 sequence", &Json::parseStrings,
        "\"\xb0\"", /* taken from the UnicodeTest */
        nullptr},
    {"garbage inside a UTF-8 sequence", &Json::parseStrings,
        "\"\xea\x40\xb8\"", /* taken from the UnicodeTest */
        nullptr},
    {"incomplete UTF-8 sequence", &Json::parseStrings,
        "\"\xce\"", /* taken from the UnicodeTest */
        nullptr}
};

const struct {
    const char* name;
    Json::Option option;
    const char* json;
    const char* message;
} ParseOptionErrorData[]{
    {"literals", Json::Option::ParseLiterals,
        "none",
        "parseLiterals(): invalid null literal none at <in>:1:1"},
    {"doubles", Json::Option::ParseDoubles,
        "-haha",
        "parseDoubles(): invalid floating-point literal -haha at <in>:1:1"},
    {"floats", Json::Option::ParseFloats,
        "-haha",
        "parseFloats(): invalid floating-point literal -haha at <in>:1:1"},
    {"string keys", Json::Option::ParseStringKeys,
        "{\"\\undefined\": null}",
        "parseStringKeys(): sorry, unicode escape sequences are not implemented yet at <in>:1:2"},
    {"strings", Json::Option::ParseStrings,
        "\"\\undefined\"",
        "parseStrings(): sorry, unicode escape sequences are not implemented yet at <in>:1:1"},
};

const struct {
    const char* name;
    bool(*function)(const JsonToken&);
    const char* json;
    const char* message;
} ParseDirectErrorData[]{
    {"null", [](const JsonToken& token) { return !!token.parseNull(); },
        "none",
        "parseNull(): invalid null literal none"},
    {"bool", [](const JsonToken& token) { return !!token.parseBool(); },
        "fail",
        "parseBool(): invalid bool literal fail"},
    {"double", [](const JsonToken& token) { return !!token.parseDouble(); },
        "75x",
        "parseDouble(): invalid floating-point literal 75x"},
    {"float", [](const JsonToken& token) { return !!token.parseFloat(); },
        "75x",
        "parseFloat(): invalid floating-point literal 75x"},
    {"unsigned int", [](const JsonToken& token) { return !!token.parseUnsignedInt(); },
        "75x",
        "parseUnsignedInt(): invalid unsigned integer literal 75x"},
    {"int", [](const JsonToken& token) { return !!token.parseInt(); },
        "75x",
        "parseInt(): invalid integer literal 75x"},
    {"unsigned long", [](const JsonToken& token) { return !!token.parseUnsignedLong(); },
        "75x",
        "parseUnsignedLong(): invalid unsigned integer literal 75x"},
    {"long", [](const JsonToken& token) { return !!token.parseLong(); },
        "75x",
        "parseLong(): invalid integer literal 75x"},
    {"size", [](const JsonToken& token) { return !!token.parseSize(); },
        "75x",
        "parseSize(): invalid unsigned integer literal 75x"},
    {"string", [](const JsonToken& token) { return !!token.parseString(); },
        "\"\\undefined\"",
        "parseString(): sorry, unicode escape sequences are not implemented yet"}
};

JsonTest::JsonTest() {
    addTests({&JsonTest::singleObject,
              &JsonTest::singleArray,
              &JsonTest::singleNull,
              &JsonTest::singleBoolean,
              &JsonTest::singleNumber,
              &JsonTest::singleString,

              &JsonTest::simpleObject,
              &JsonTest::simpleArray,
              &JsonTest::nested});

    addInstancedTests({&JsonTest::error},
        Containers::arraySize(ErrorData));

    addTests({&JsonTest::parseNull,
              &JsonTest::parseNulls});

    addInstancedTests({&JsonTest::parseBool,
                       &JsonTest::parseBools},
        Containers::arraySize(ParseBoolData));

    addInstancedTests({&JsonTest::parseDouble,
                       &JsonTest::parseDoubles},
        Containers::arraySize(ParseDoubleOrFloatData));

    addInstancedTests({&JsonTest::parseFloat,
                       &JsonTest::parseFloats},
        Containers::arraySize(ParseDoubleOrFloatData));

    addInstancedTests({&JsonTest::parseUnsignedInt,
                       &JsonTest::parseUnsignedInts},
        Containers::arraySize(ParseUnsignedIntData));

    addInstancedTests({&JsonTest::parseInt,
                       &JsonTest::parseInts},
        Containers::arraySize(ParseIntData));

    addInstancedTests({&JsonTest::parseUnsignedLong,
                       &JsonTest::parseUnsignedLongs},
        Containers::arraySize(ParseUnsignedLongData));

    addInstancedTests({&JsonTest::parseLong,
                       #ifndef CORRADE_TARGET_32BIT
                       &JsonTest::parseLongs
                       #endif
                       },
        Containers::arraySize(ParseLongData));

    addInstancedTests({&JsonTest::parseSize,
                       &JsonTest::parseSizes},
        #ifndef CORRADE_TARGET_32BIT
        Containers::arraySize(ParseUnsignedLongData)
        #else
        Containers::arraySize(ParseUnsignedIntData)
        #endif
    );

    addInstancedTests({&JsonTest::parseString,
                       &JsonTest::parseStringKeys,
                       &JsonTest::parseStrings},
        Containers::arraySize(ParseStringData));

    addInstancedTests({&JsonTest::parseOption},
        Containers::arraySize(ParseOptionData));

    addInstancedTests({&JsonTest::parseSubtree},
        Containers::arraySize(ParseSubtreeData));

    addTests({&JsonTest::reparseNumberDifferentType});

    addInstancedTests({&JsonTest::parseError},
        Containers::arraySize(ParseErrorData));

    addInstancedTests({&JsonTest::parseOptionError},
        Containers::arraySize(ParseOptionErrorData));

    addInstancedTests({&JsonTest::parseDirectError},
        Containers::arraySize(ParseDirectErrorData));

    addTests({&JsonTest::parseTokenNotOwned,

              &JsonTest::file,
              &JsonTest::fileReadError,
              &JsonTest::fileOptionReadError,
              &JsonTest::fileError,
              &JsonTest::fileParseOptionError,
              &JsonTest::fileParseError,

              &JsonTest::asTypeWrongType,
              &JsonTest::asTypeNotParsed,
              &JsonTest::asTypeWrongParsedType,

              #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
              &JsonTest::tokenConstructCopy,
              #endif
              &JsonTest::constructCopy,
              &JsonTest::constructMove,

              &JsonTest::debugTokenType,
              &JsonTest::debugTokenParsedType});
}

void JsonTest::error() {
    auto&& data = ErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromString(data.data));
    CORRADE_COMPARE(out.str(), Utility::formatString("Utility::Json: {}\n", data.message));
}

void JsonTest::singleObject() {
    Containers::Optional<Json> json = Json::fromString(" {  \n \r  } ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& object = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &object);
    CORRADE_COMPARE(object.data(), "{  \n \r  }");
    CORRADE_COMPARE(object.type(), JsonToken::Type::Object);
    CORRADE_VERIFY(object.isParsed());
    CORRADE_COMPARE(object.childCount(), 0);
    CORRADE_COMPARE(object.children().size(), 0);
    CORRADE_VERIFY(!object.firstChild());
    CORRADE_COMPARE(object.next(), json->tokens().end());
    CORRADE_VERIFY(!object.parent());
}

void JsonTest::singleArray() {
    Containers::Optional<Json> json = Json::fromString(" [  \n \r  ] ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& array = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &array);
    CORRADE_COMPARE(array.data(), "[  \n \r  ]");
    CORRADE_COMPARE(array.type(), JsonToken::Type::Array);
    CORRADE_VERIFY(array.isParsed());
    CORRADE_COMPARE(array.childCount(), 0);
    CORRADE_COMPARE(array.children().size(), 0);
    CORRADE_VERIFY(!array.firstChild());
    CORRADE_COMPARE(array.next(), json->tokens().end());
    CORRADE_VERIFY(!array.parent());
}

void JsonTest::singleNull() {
    /* Detects only the first letter and assumes sanity by default */
    Containers::Optional<Json> json = Json::fromString(" nULLtotallyinvalidyes\n ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& null = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &null);
    CORRADE_COMPARE(null.data(), "nULLtotallyinvalidyes");
    CORRADE_COMPARE(null.type(), JsonToken::Type::Null);
    CORRADE_VERIFY(!null.isParsed());
    CORRADE_COMPARE(null.childCount(), 0);
    CORRADE_COMPARE(null.children().size(), 0);
    CORRADE_VERIFY(!null.firstChild());
    CORRADE_COMPARE(null.next(), json->tokens().end());
    CORRADE_VERIFY(!null.parent());
}

void JsonTest::singleBoolean() {
    /* Detects only the first letter and assumes sanity by default */
    Containers::Optional<Json> json = Json::fromString(" fALsetotallyinvalidyes\n ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& boolean = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &boolean);
    CORRADE_COMPARE(boolean.data(), "fALsetotallyinvalidyes");
    CORRADE_COMPARE(boolean.type(), JsonToken::Type::Bool);
    CORRADE_VERIFY(!boolean.isParsed());
    CORRADE_COMPARE(boolean.childCount(), 0);
    CORRADE_COMPARE(boolean.children().size(), 0);
    CORRADE_VERIFY(!boolean.firstChild());
    CORRADE_COMPARE(boolean.next(), json->tokens().end());
    CORRADE_VERIFY(!boolean.parent());
}

void JsonTest::singleNumber() {
    /* Detects only the first letter and assumes sanity by default */
    Containers::Optional<Json> json = Json::fromString(" -hahahahah\n ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& number = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &number);
    CORRADE_COMPARE(number.data(), "-hahahahah");
    CORRADE_COMPARE(number.type(), JsonToken::Type::Number);
    CORRADE_VERIFY(!number.isParsed());
    CORRADE_COMPARE(number.childCount(), 0);
    CORRADE_COMPARE(number.children().size(), 0);
    CORRADE_VERIFY(!number.firstChild());
    CORRADE_COMPARE(number.next(), json->tokens().end());
    CORRADE_VERIFY(!number.parent());
}

void JsonTest::singleString() {
    /* Assumes sanity of unicode escapes by default */
    Containers::Optional<Json> json = Json::fromString(" \"\\uNICODE yay\\\"\" \n ");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 1);

    const JsonToken& string = json->tokens()[0];
    CORRADE_COMPARE(&json->root(), &string);
    CORRADE_COMPARE(string.data(), "\"\\uNICODE yay\\\"\"");
    CORRADE_COMPARE(string.type(), JsonToken::Type::String);
    CORRADE_VERIFY(!string.isParsed());
    CORRADE_COMPARE(string.childCount(), 0);
    CORRADE_COMPARE(string.children().size(), 0);
    CORRADE_VERIFY(!string.firstChild());
    CORRADE_COMPARE(string.next(), json->tokens().end());
    CORRADE_VERIFY(!string.parent());
}

void JsonTest::simpleObject() {
    Containers::Optional<Json> json = Json::fromString(R"(
        {"key1": "hello",
         "key2":null,
         "key3"   :-375.26e5,
         "key4":   0,
         "key5": false,
         "key6" : "abc",
         "key7": [],
         "key8": true}
    )");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 17);

    /* Verify data */
    const JsonToken& object = json->tokens()[0];
    /* GCC 4.8 can't handle raw string literals in macros */
    const char* objectData = R"({"key1": "hello",
         "key2":null,
         "key3"   :-375.26e5,
         "key4":   0,
         "key5": false,
         "key6" : "abc",
         "key7": [],
         "key8": true})";
    CORRADE_COMPARE(object.data(), objectData);
    CORRADE_COMPARE(object.type(), JsonToken::Type::Object);

    const JsonToken& string1 = json->tokens()[2];
    CORRADE_COMPARE(string1.data(), "\"hello\"");
    CORRADE_COMPARE(string1.type(), JsonToken::Type::String);

    const JsonToken& null = json->tokens()[4];
    CORRADE_COMPARE(null.data(), "null");
    CORRADE_COMPARE(null.type(), JsonToken::Type::Null);

    const JsonToken& number1 = json->tokens()[6];
    CORRADE_COMPARE(number1.data(), "-375.26e5");
    CORRADE_COMPARE(number1.type(), JsonToken::Type::Number);

    const JsonToken& number2 = json->tokens()[8];
    CORRADE_COMPARE(number2.data(), "0");
    CORRADE_COMPARE(number2.type(), JsonToken::Type::Number);

    const JsonToken& bool1 = json->tokens()[10];
    CORRADE_COMPARE(bool1.data(), "false");
    CORRADE_COMPARE(bool1.type(), JsonToken::Type::Bool);

    const JsonToken& string2 = json->tokens()[12];
    CORRADE_COMPARE(string2.data(), "\"abc\"");
    CORRADE_COMPARE(string2.type(), JsonToken::Type::String);

    const JsonToken& array = json->tokens()[14];
    CORRADE_COMPARE(array.data(), "[]");
    CORRADE_COMPARE(array.type(), JsonToken::Type::Array);

    const JsonToken& bool2 = json->tokens()[16];
    CORRADE_COMPARE(bool2.data(), "true");
    CORRADE_COMPARE(bool2.type(), JsonToken::Type::Bool);

    /* No tokens should be parsed, except for objects and arrays */
    for(const JsonToken& i: json->tokens())
        CORRADE_COMPARE(i.isParsed(),
           i.type() == JsonToken::Type::Object ||
           i.type() == JsonToken::Type::Array);

    /* Verify keys */
    for(std::size_t i = 0; i != 8; ++i) {
        CORRADE_ITERATION(i);
        const JsonToken& key = json->tokens()[1 + 2*i];
        /* Unlike objects and arrays, the key token data don't contain the
           nested value even the value is a child */
        CORRADE_COMPARE(key.data(), formatString("\"key{}\"", i + 1));
        CORRADE_COMPARE(key.type(), JsonToken::Type::String);
        CORRADE_VERIFY(!key.isParsed());
    }

    /* Verify traversal */
    CORRADE_COMPARE(object.childCount(), 16);
    CORRADE_VERIFY(object.firstChild());
    CORRADE_COMPARE(object.firstChild()->firstChild(), &string1);
    CORRADE_COMPARE(object.children().size(), 16);
    CORRADE_COMPARE(&object.children().front().children().front(), &string1);
    CORRADE_COMPARE(&object.children().back(), &bool2);
    CORRADE_COMPARE(object.next(), json->tokens().end());
    CORRADE_VERIFY(!object.parent());

    /* The object values should ... */
    const JsonToken* prevKey = nullptr;
    const JsonToken* prevValue = nullptr;
    for(const JsonToken* key = object.firstChild(); key != json->tokens().end(); key = key->next()) {
        CORRADE_ITERATION(key->data());
        /* Have exactly one child */
        CORRADE_COMPARE(key->childCount(), 1);
        CORRADE_COMPARE(key->children().size(), 1);
        /* All the same parent */
        CORRADE_COMPARE(key->parent(), &object);
        /* Next should always point to the key */
        if(prevKey) CORRADE_COMPARE(prevKey->next(), key);
        prevKey = key;

        /* The value having no nested children */
        const JsonToken* value = key->firstChild();
        CORRADE_VERIFY(value);
        CORRADE_COMPARE(value->childCount(), 0);
        CORRADE_COMPARE(value->children().size(), 0);
        /* Key being the parent */
        CORRADE_COMPARE(value->parent(), key);
        /* Next should always point to the next key */
        if(prevValue) CORRADE_COMPARE(prevValue->next(), key);
        prevValue = value;
    }
    CORRADE_COMPARE(prevValue, &json->tokens().back());
}

void JsonTest::simpleArray() {
    Containers::Optional<Json> json = Json::fromString(R"(
        ["hello", null,   -375.26e5,0,   false, "abc",{}, true ]
    )");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 9);

    /* Verify data */
    const JsonToken& array = json->tokens()[0];
    /* GCC 4.8 can't handle raw string literals in macros */
    const char* arrayData = R"(["hello", null,   -375.26e5,0,   false, "abc",{}, true ])";
    CORRADE_COMPARE(array.data(), arrayData);
    CORRADE_COMPARE(array.type(), JsonToken::Type::Array);

    const JsonToken& string1 = json->tokens()[1];
    CORRADE_COMPARE(string1.data(), "\"hello\"");
    CORRADE_COMPARE(string1.type(), JsonToken::Type::String);

    const JsonToken& null = json->tokens()[2];
    CORRADE_COMPARE(null.data(), "null");
    CORRADE_COMPARE(null.type(), JsonToken::Type::Null);

    const JsonToken& number1 = json->tokens()[3];
    CORRADE_COMPARE(number1.data(), "-375.26e5");
    CORRADE_COMPARE(number1.type(), JsonToken::Type::Number);

    const JsonToken& number2 = json->tokens()[4];
    CORRADE_COMPARE(number2.data(), "0");
    CORRADE_COMPARE(number2.type(), JsonToken::Type::Number);

    const JsonToken& bool1 = json->tokens()[5];
    CORRADE_COMPARE(bool1.data(), "false");
    CORRADE_COMPARE(bool1.type(), JsonToken::Type::Bool);

    const JsonToken& string2 = json->tokens()[6];
    CORRADE_COMPARE(string2.data(), "\"abc\"");
    CORRADE_COMPARE(string2.type(), JsonToken::Type::String);

    const JsonToken& object = json->tokens()[7];
    CORRADE_COMPARE(object.data(), "{}");
    CORRADE_COMPARE(object.type(), JsonToken::Type::Object);

    const JsonToken& bool2 = json->tokens()[8];
    CORRADE_COMPARE(bool2.data(), "true");
    CORRADE_COMPARE(bool2.type(), JsonToken::Type::Bool);

    /* No tokens should be parsed, except for objects and arrays */
    for(const JsonToken& i: json->tokens())
        CORRADE_COMPARE(i.isParsed(),
           i.type() == JsonToken::Type::Object ||
           i.type() == JsonToken::Type::Array);

    /* Verify traversal */
    CORRADE_COMPARE(array.childCount(), 8);
    CORRADE_COMPARE(array.firstChild(), &string1);
    CORRADE_COMPARE(array.children().size(), 8);
    CORRADE_COMPARE(&array.children().front(), &string1);
    CORRADE_COMPARE(&array.children().back(), &bool2);
    CORRADE_COMPARE(array.next(), json->tokens().end());
    CORRADE_VERIFY(!array.parent());

    /* The array children should ... */
    const JsonToken* prev = nullptr;
    for(const JsonToken& i: array.children()) {
        CORRADE_ITERATION(i.data());
        /* Have no children */
        CORRADE_COMPARE(i.childCount(), 0);
        CORRADE_COMPARE(i.children().size(), 0);
        CORRADE_VERIFY(!i.firstChild());
        /* All the same parent */
        CORRADE_COMPARE(i.parent(), &array);
        /* Next should always point to ... the next */
        if(prev) CORRADE_COMPARE(prev->next(), &i);
        prev = &i;
    }
    CORRADE_COMPARE(prev, &json->tokens().back());
}

void JsonTest::nested() {
    Containers::Optional<Json> json = Json::fromString(R"(
        [{"hello": 5,
          "yes": true,
          "matrix": [[0, 1],
                     [2, 3]],
          "braces": {"again": {}}},
          -15.75,
          "bye!",
          []]
    )");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 21);

    /* Verify data */
    const JsonToken& array = json->tokens()[0];
    /* GCC 4.8 can't handle raw string literals in macros */
    const char* arrayData = R"([{"hello": 5,
          "yes": true,
          "matrix": [[0, 1],
                     [2, 3]],
          "braces": {"again": {}}},
          -15.75,
          "bye!",
          []])";
    CORRADE_COMPARE(array.data(), arrayData);
    CORRADE_COMPARE(array.type(), JsonToken::Type::Array);

    const JsonToken& object = json->tokens()[1];
    /* GCC 4.8 can't handle raw string literals in macros */
    const char* objectData = R"({"hello": 5,
          "yes": true,
          "matrix": [[0, 1],
                     [2, 3]],
          "braces": {"again": {}}})";
    CORRADE_COMPARE(object.data(), objectData);
    CORRADE_COMPARE(object.type(), JsonToken::Type::Object);

    const JsonToken& hello = json->tokens()[2];
    CORRADE_COMPARE(hello.data(), "\"hello\"");
    CORRADE_COMPARE(hello.type(), JsonToken::Type::String);

    const JsonToken& five = json->tokens()[3];
    CORRADE_COMPARE(five.data(), "5");
    CORRADE_COMPARE(five.type(), JsonToken::Type::Number);

    const JsonToken& yes = json->tokens()[4];
    CORRADE_COMPARE(yes.data(), "\"yes\"");
    CORRADE_COMPARE(yes.type(), JsonToken::Type::String);

    const JsonToken& true_ = json->tokens()[5];
    CORRADE_COMPARE(true_.data(), "true");
    CORRADE_COMPARE(true_.type(), JsonToken::Type::Bool);

    const JsonToken& matrix = json->tokens()[6];
    CORRADE_COMPARE(matrix.data(), "\"matrix\"");
    CORRADE_COMPARE(matrix.type(), JsonToken::Type::String);

    const JsonToken& matrixArray1 = json->tokens()[7];
    /* GCC 4.8 can't handle raw string literals in macros */
    const char* matrixArray1Data = R"([[0, 1],
                     [2, 3]])";
    CORRADE_COMPARE(matrixArray1.data(), matrixArray1Data);
    CORRADE_COMPARE(matrixArray1.type(), JsonToken::Type::Array);

    const JsonToken& matrixArray2 = json->tokens()[8];
    CORRADE_COMPARE(matrixArray2.data(), "[0, 1]");
    CORRADE_COMPARE(matrixArray2.type(), JsonToken::Type::Array);

    const JsonToken& zero = json->tokens()[9];
    CORRADE_COMPARE(zero.data(), "0");
    CORRADE_COMPARE(zero.type(), JsonToken::Type::Number);

    const JsonToken& one = json->tokens()[10];
    CORRADE_COMPARE(one.data(), "1");
    CORRADE_COMPARE(one.type(), JsonToken::Type::Number);

    const JsonToken& matrixArray3 = json->tokens()[11];
    CORRADE_COMPARE(matrixArray3.data(), "[2, 3]");
    CORRADE_COMPARE(matrixArray3.type(), JsonToken::Type::Array);

    const JsonToken& two = json->tokens()[12];
    CORRADE_COMPARE(two.data(), "2");
    CORRADE_COMPARE(two.type(), JsonToken::Type::Number);

    const JsonToken& three = json->tokens()[13];
    CORRADE_COMPARE(three.data(), "3");
    CORRADE_COMPARE(three.type(), JsonToken::Type::Number);

    const JsonToken& braces = json->tokens()[14];
    CORRADE_COMPARE(braces.data(), "\"braces\"");
    CORRADE_COMPARE(braces.type(), JsonToken::Type::String);

    const JsonToken& bracesObject = json->tokens()[15];
    CORRADE_COMPARE(bracesObject.data(), "{\"again\": {}}");
    CORRADE_COMPARE(bracesObject.type(), JsonToken::Type::Object);

    const JsonToken& again = json->tokens()[16];
    CORRADE_COMPARE(again.data(), "\"again\"");
    CORRADE_COMPARE(again.type(), JsonToken::Type::String);

    const JsonToken& emptyObject = json->tokens()[17];
    CORRADE_COMPARE(emptyObject.data(), "{}");
    CORRADE_COMPARE(emptyObject.type(), JsonToken::Type::Object);

    const JsonToken& number = json->tokens()[18];
    CORRADE_COMPARE(number.data(), "-15.75");
    CORRADE_COMPARE(number.type(), JsonToken::Type::Number);

    const JsonToken& bye = json->tokens()[19];
    CORRADE_COMPARE(bye.data(), "\"bye!\"");
    CORRADE_COMPARE(bye.type(), JsonToken::Type::String);

    const JsonToken& emptyArray = json->tokens()[20];
    CORRADE_COMPARE(emptyArray.data(), "[]");
    CORRADE_COMPARE(emptyArray.type(), JsonToken::Type::Array);

    /* No tokens should be parsed, except for objects and arrays */
    for(const JsonToken& i: json->tokens())
        CORRADE_COMPARE(i.isParsed(),
           i.type() == JsonToken::Type::Object ||
           i.type() == JsonToken::Type::Array);

    /* Verify child counts */
    CORRADE_COMPARE(array.childCount(), 20);
    CORRADE_COMPARE(object.childCount(), 16);
    CORRADE_COMPARE(matrix.childCount(), 7);
    CORRADE_COMPARE(matrixArray1.childCount(), 6);
    CORRADE_COMPARE(matrixArray2.childCount(), 2);
    CORRADE_COMPARE(matrixArray3.childCount(), 2);
    CORRADE_COMPARE(braces.childCount(), 3);
    CORRADE_COMPARE(bracesObject.childCount(), 2);
    for(const JsonToken* key: {&hello, &yes, &again}) {
        CORRADE_ITERATION(key->data());
        CORRADE_COMPARE(key->childCount(), 1);
    }
    for(const JsonToken* value: {&three, &true_, &zero, &one, &two, &three, &number, &emptyObject, &bye, &emptyArray}) {
        CORRADE_ITERATION(value->data());
        CORRADE_COMPARE(value->childCount(), 0);
    }

    /* Verify first childs */
    CORRADE_COMPARE(array.firstChild(), &object);
    CORRADE_COMPARE(object.firstChild(), &hello);
    CORRADE_COMPARE(hello.firstChild(), &five);
    CORRADE_COMPARE(yes.firstChild(), &true_);
    CORRADE_COMPARE(matrix.firstChild(), &matrixArray1);
    CORRADE_COMPARE(matrixArray1.firstChild(), &matrixArray2);
    CORRADE_COMPARE(matrixArray2.firstChild(), &zero);
    CORRADE_COMPARE(matrixArray3.firstChild(), &two);
    CORRADE_COMPARE(braces.firstChild(), &bracesObject);
    CORRADE_COMPARE(bracesObject.firstChild(), &again);
    CORRADE_COMPARE(again.firstChild(), &emptyObject);

    /* Verify next tokens */
    CORRADE_COMPARE(array.next(), json->tokens().end());
    CORRADE_COMPARE(object.next(), &number);
    CORRADE_COMPARE(hello.next(), &yes);
    CORRADE_COMPARE(yes.next(), &matrix);
    CORRADE_COMPARE(matrix.next(), &braces);
    CORRADE_COMPARE(matrixArray1.next(), &braces);
    CORRADE_COMPARE(matrixArray2.next(), &matrixArray3);
    CORRADE_COMPARE(matrixArray3.next(), &braces);
    CORRADE_COMPARE(braces.next(), &number);
    CORRADE_COMPARE(bracesObject.next(), &number);
    CORRADE_COMPARE(again.next(), &number);
    CORRADE_COMPARE(emptyObject.next(), &number);
    CORRADE_COMPARE(number.next(), &bye);
    CORRADE_COMPARE(bye.next(), &emptyArray);
    CORRADE_COMPARE(emptyArray.next(), json->tokens().end());
}

void JsonTest::parseNull() {
    Containers::Optional<Json> json = Json::fromString("null");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseNull(), nullptr);
}

void JsonTest::parseNulls() {
    Containers::Optional<Json> json = Json::fromString("null");
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Null);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), "null");

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseLiterals(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Null);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Other);
        CORRADE_COMPARE(json->root().data(), "null");

        /* both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asNull(), nullptr);
        CORRADE_COMPARE(json->root().parseNull(), nullptr);
    }
}

void JsonTest::parseBool() {
    auto&& data = ParseBoolData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseBool(), data.expected);
}

void JsonTest::parseBools() {
    auto&& data = ParseBoolData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Bool);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseLiterals(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Bool);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Other);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asBool(), data.expected);
        CORRADE_COMPARE(json->root().parseBool(), data.expected);
    }
}

void JsonTest::parseDouble() {
    auto&& data = ParseDoubleOrFloatData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseDouble(), data.expected);
}

void JsonTest::parseDoubles() {
    auto&& data = ParseDoubleOrFloatData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseDoubles(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Double);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asDouble(), data.expected);
        CORRADE_COMPARE(json->root().parseDouble(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseFloat(), float(data.expected));
    }
}

void JsonTest::parseFloat() {
    auto&& data = ParseDoubleOrFloatData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseFloat(), float(data.expected));
}

void JsonTest::parseFloats() {
    auto&& data = ParseDoubleOrFloatData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseFloats(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Float);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asFloat(), float(data.expected));
        CORRADE_COMPARE(json->root().parseFloat(), float(data.expected));

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), data.expected);
    }
}

void JsonTest::parseUnsignedInt() {
    auto&& data = ParseUnsignedIntData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseUnsignedInt(), data.expected);
}

void JsonTest::parseUnsignedInts() {
    auto&& data = ParseUnsignedIntData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseUnsignedInts(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::UnsignedInt);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asUnsignedInt(), data.expected);
        CORRADE_COMPARE(json->root().parseUnsignedInt(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), double(data.expected));
    }
}

void JsonTest::parseInt() {
    auto&& data = ParseIntData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseInt(), data.expected);
}

void JsonTest::parseInts() {
    auto&& data = ParseIntData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseInts(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Int);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asInt(), data.expected);
        CORRADE_COMPARE(json->root().parseInt(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), double(data.expected));
    }
}

void JsonTest::parseUnsignedLong() {
    auto&& data = ParseUnsignedLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseUnsignedLong(), data.expected);
}

void JsonTest::parseUnsignedLongs() {
    auto&& data = ParseUnsignedLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseUnsignedLongs(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::UnsignedLong);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asUnsignedLong(), data.expected);
        CORRADE_COMPARE(json->root().parseUnsignedLong(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), double(data.expected));
    }
}

void JsonTest::parseLong() {
    auto&& data = ParseLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseLong(), data.expected);
}

#ifndef CORRADE_TARGET_32BIT
void JsonTest::parseLongs() {
    auto&& data = ParseLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseLongs(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Long);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asLong(), data.expected);
        CORRADE_COMPARE(json->root().parseLong(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), double(data.expected));
    }
}
#endif

void JsonTest::parseSize() {
    #ifndef CORRADE_TARGET_32BIT
    auto&& data = ParseUnsignedLongData[testCaseInstanceId()];
    #else
    auto&& data = ParseUnsignedIntData[testCaseInstanceId()];
    #endif
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseSize(), data.expected);
}

void JsonTest::parseSizes() {
    #ifndef CORRADE_TARGET_32BIT
    auto&& data = ParseUnsignedLongData[testCaseInstanceId()];
    #else
    auto&& data = ParseUnsignedIntData[testCaseInstanceId()];
    #endif
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseSizes(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::Number);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Size);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asSize(), data.expected);
        CORRADE_COMPARE(json->root().parseSize(), data.expected);

        /* Parsing as a different type should parse from scratch */
        CORRADE_COMPARE(json->root().parseDouble(), double(data.expected));
    }
}

void JsonTest::parseString() {
    auto&& data = ParseStringData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->root().parseString(), Containers::String{data.expected});
}

void JsonTest::parseStringKeys() {
    auto&& data = ParseStringData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Fake-propagate original global flags here */
    Containers::String jsonData = format("{{{}: null}}", data.json);
    Containers::Optional<Json> json = Json::fromString({jsonData.data(), jsonData.size(), data.json.flags()});
    CORRADE_VERIFY(json);
    const JsonToken& token = json->tokens()[1];
    CORRADE_VERIFY(!token.isParsed());
    CORRADE_COMPARE(token.type(), JsonToken::Type::String);
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(token.data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseStringKeys(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(token.isParsed());
        CORRADE_COMPARE(token.type(), JsonToken::Type::String);
        CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Other);
        CORRADE_COMPARE(token.data(), data.json);

        /* Both functions should return a cached value, preserving the global
           flag */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(token.asString(), data.expected);
        CORRADE_COMPARE(token.asString().flags() & ~Containers::StringViewFlag::NullTerminated,
            data.json.flags() & ~Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(token.parseString(), Containers::String{data.expected});
    }
}

void JsonTest::parseStrings() {
    auto&& data = ParseStringData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);
    CORRADE_VERIFY(!json->root().isParsed());
    CORRADE_COMPARE(json->root().type(), JsonToken::Type::String);
    CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::None);
    CORRADE_COMPARE(json->root().data(), data.json);

    /* Calling the parse function several times should have the same observed
       behavior, internally it should just skip parsing */
    for(std::size_t iteration: {0, 1}) {
        CORRADE_ITERATION(iteration);
        CORRADE_VERIFY(json->parseStrings(json->root()));

        /* The token data should not get corrupted by this */
        CORRADE_VERIFY(json->root().isParsed());
        CORRADE_COMPARE(json->root().type(), JsonToken::Type::String);
        CORRADE_COMPARE(json->root().parsedType(), JsonToken::ParsedType::Other);
        CORRADE_COMPARE(json->root().data(), data.json);

        /* Both functions should return a cached value, preserving the global
           flag */
        /** @todo how to actually verify? by corrupting the JSON underneath? */
        CORRADE_COMPARE(json->root().asString(), data.expected);
        CORRADE_COMPARE(json->root().asString().flags() & ~Containers::StringViewFlag::NullTerminated,
            data.json.flags() & ~Containers::StringViewFlag::NullTerminated);
        CORRADE_COMPARE(json->root().parseString(), Containers::String{data.expected});
    }
}

void JsonTest::parseOption() {
    auto&& data = ParseOptionData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(R"({
        "null": null,
        "bool": true,
        "nested": {
            "null": null,
            "bool": true,
            "number": 35,
            "string": "hello"
        },
        "number": 35,
        "string": "hello"
    })", data.option);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 19);

    const JsonToken& tokenParsed = json->tokens()[data.tokenParsed];
    CORRADE_COMPARE(tokenParsed.data(), data.tokenData);
    CORRADE_COMPARE(tokenParsed.parsedType(), data.parsedType);

    const JsonToken& tokenParsed2 = json->tokens()[data.tokenParsed2];
    CORRADE_COMPARE(tokenParsed2.data(), data.tokenData);
    CORRADE_COMPARE(tokenParsed2.parsedType(), data.parsedType);

    /* Verify tokens of other type are not parsed by accident */
    std::size_t notParsedCount = 0;
    for(const JsonToken& token: json->tokens())
        if(!token.isParsed()) ++notParsedCount;
    CORRADE_COMPARE(notParsedCount, data.tokenNotParsedCount);
}

void JsonTest::parseSubtree() {
    auto&& data = ParseSubtreeData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(R"([
        {
            "null": null,
            "bool": true,
            "nested": {
                "null": null,
                "bool": true,
                "number": 35,
                "nested": [
                    "hello"
                ]
            },
            "number": 35,
            "nested": [
                "hello"
            ]
        },
        null,
        true,
        35,
        "hello"
    ])");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 26);
    CORRADE_VERIFY(((*json).*data.function)(json->tokens()[data.parseRoot]));

    const JsonToken& tokenParsed = json->tokens()[data.tokenParsed];
    CORRADE_COMPARE(tokenParsed.data(), data.tokenData);
    CORRADE_COMPARE(tokenParsed.parsedType(), data.parsedType);

    const JsonToken& tokenParsedDeep = json->tokens()[data.tokenParsedDeep];
    CORRADE_COMPARE(tokenParsedDeep.data(), data.tokenData);
    CORRADE_COMPARE(tokenParsedDeep.parsedType(), data.parsedType);

    const JsonToken& tokenNotParsedOut = json->tokens()[data.tokenNotParsed];
    CORRADE_COMPARE(tokenNotParsedOut.data(), data.tokenData);
    CORRADE_VERIFY(!tokenNotParsedOut.isParsed());

    /* Verify tokens of other type are not parsed by accident */
    std::size_t notParsedCount = 0;
    for(const JsonToken& token: json->tokens())
        if(!token.isParsed()) ++notParsedCount;
    CORRADE_COMPARE(notParsedCount, data.tokenNotParsedCount);
}

void JsonTest::reparseNumberDifferentType() {
    /* It should be possible to reparse a token with different numeric types
       several times over */

    Containers::Optional<Json> json = Json::fromString("35");
    CORRADE_VERIFY(json);

    const JsonToken& token = json->root();
    CORRADE_VERIFY(json->parseDoubles(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Double);
    CORRADE_COMPARE(token.asDouble(), 35.0);

    CORRADE_VERIFY(json->parseFloats(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Float);
    CORRADE_COMPARE(token.asFloat(), 35.0f);

    CORRADE_VERIFY(json->parseUnsignedInts(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::UnsignedInt);
    CORRADE_COMPARE(token.asUnsignedInt(), 35);

    CORRADE_VERIFY(json->parseInts(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Int);
    CORRADE_COMPARE(token.asInt(), 35);

    CORRADE_VERIFY(json->parseUnsignedLongs(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::UnsignedLong);
    CORRADE_COMPARE(token.asUnsignedLong(), 35);

    #ifndef CORRADE_TARGET_32BIT
    CORRADE_VERIFY(json->parseLongs(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Long);
    CORRADE_COMPARE(token.asLong(), 35);
    #endif

    CORRADE_VERIFY(json->parseSizes(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Size);
    CORRADE_COMPARE(token.asSize(), 35);

    /* ... and back again */
    CORRADE_VERIFY(json->parseDoubles(token));
    CORRADE_COMPARE(token.parsedType(), JsonToken::ParsedType::Double);
    CORRADE_COMPARE(token.asDouble(), 35.0);
}

void JsonTest::parseError() {
    auto&& data = ParseErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(format("\n\n     {}", data.json));
    CORRADE_VERIFY(json);

    const JsonToken& token = json->root();
    const JsonToken::Type type = token.type();

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_EXPECT_FAIL_IF(!data.message, "Not implemented yet.");
    CORRADE_VERIFY(!((*json).*data.function)(json->root()));
    if(!data.message) return;
    CORRADE_COMPARE(out.str(), formatString("Utility::Json::{} at <in>:3:6\n", data.message));

    /* Verify that the JSON token doesn't get corrupted by the error */
    CORRADE_VERIFY(!token.isParsed());
    CORRADE_COMPARE(token.type(), type);
    CORRADE_COMPARE(token.data(), data.json);
    CORRADE_COMPARE(token.childCount(), 0);
}

void JsonTest::parseOptionError() {
    /* The particular corner cases got all tested in parseError(), here just
       verifying that the error gets correctly propagated also when using
       Json::Option */

    auto&& data = ParseOptionErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromString(data.json, data.option));
    CORRADE_COMPARE(out.str(), formatString("Utility::Json::{}\n", data.message));
}

void JsonTest::parseDirectError() {
    /* The particular corner cases got all tested in parseError(), here just
       verifying that the error gets correctly propagated also when using
       JsonToken::parseWhatever() */

    auto&& data = ParseDirectErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Json> json = Json::fromString(data.json);
    CORRADE_VERIFY(json);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!data.function(json->root()));
    CORRADE_COMPARE(out.str(), formatString("Utility::JsonToken::{}\n", data.message));
}

void JsonTest::parseTokenNotOwned() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Containers::Optional<Json> json = Json::fromString("{}");
    CORRADE_VERIFY(json);

    JsonToken token = json->root();

    std::ostringstream out;
    Error redirectError{&out};
    json->parseLiterals(token);
    json->parseDoubles(token);
    json->parseFloats(token);
    json->parseUnsignedInts(token);
    json->parseInts(token);
    json->parseUnsignedLongs(token);
    #ifndef CORRADE_TARGET_32BIT
    json->parseLongs(token);
    #endif
    json->parseSizes(token);
    json->parseStringKeys(token);
    json->parseStrings(token);
    const char* expected =
        "Utility::Json::parseLiterals(): token not owned by the instance\n"
        "Utility::Json::parseDoubles(): token not owned by the instance\n"
        "Utility::Json::parseFloats(): token not owned by the instance\n"
        "Utility::Json::parseUnsignedInts(): token not owned by the instance\n"
        "Utility::Json::parseInts(): token not owned by the instance\n"
        "Utility::Json::parseUnsignedLongs(): token not owned by the instance\n"
        #ifndef CORRADE_TARGET_32BIT
        "Utility::Json::parseLongs(): token not owned by the instance\n"
        "Utility::Json::parseUnsignedLongs(): token not owned by the instance\n"
        #else
        "Utility::Json::parseUnsignedInts(): token not owned by the instance\n"
        #endif
        "Utility::Json::parseStringKeys(): token not owned by the instance\n"
        "Utility::Json::parseStrings(): token not owned by the instance\n";
    CORRADE_COMPARE(out.str(), expected);
}

void JsonTest::file() {
    /* The file has a parse error, but tokenization should succeed */
    Containers::Optional<Json> json = Json::fromFile(Path::join(JSON_TEST_DIR, "parse-error.json"));
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 2);

    const JsonToken& array = json->tokens()[0];
    CORRADE_COMPARE(array.data(), "[\n    -haha\n]");
    CORRADE_COMPARE(array.type(), JsonToken::Type::Array);

    const JsonToken& number = json->tokens()[1];
    CORRADE_COMPARE(number.data(), "-haha");
    CORRADE_COMPARE(number.type(), JsonToken::Type::Number);
}

void JsonTest::fileReadError() {
    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromFile("nonexistent"));
    /* There's an error from Path::read() before */
    CORRADE_COMPARE_AS(out.str(),
        "\nUtility::Json::fromFile(): can't read nonexistent\n",
        TestSuite::Compare::StringHasSuffix);
}

void JsonTest::fileOptionReadError() {
    /* The options parameter is a separate file loading code path, test it as
       well */

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromFile("nonexistent", Json::Option::ParseStrings));
    /* There's an error from Path::read() before */
    CORRADE_COMPARE_AS(out.str(),
        "\nUtility::Json::fromFile(): can't read nonexistent\n",
        TestSuite::Compare::StringHasSuffix);
}

void JsonTest::fileError() {
    Containers::String filename = Path::join(JSON_TEST_DIR, "error.json");

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromFile(filename));
    CORRADE_COMPARE(out.str(), formatString("Utility::Json: expected a value but got ] at {}:3:1\n", filename));
}

void JsonTest::fileParseOptionError() {
    Containers::String filename = Path::join(JSON_TEST_DIR, "parse-error.json");

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!Json::fromFile(filename, Json::Option::ParseDoubles));
    CORRADE_COMPARE(out.str(), formatString("Utility::Json::parseDoubles(): invalid floating-point literal -haha at {}:2:5\n", filename));
}

void JsonTest::fileParseError() {
    /* The filename should get remembered even for subsequent parse() calls,
       but of course not for JsonToken::parse() */

    Containers::String filename = Path::join(JSON_TEST_DIR, "parse-error.json");
    Containers::Optional<Json> json = Json::fromFile(filename);
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 2);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!json->parseDoubles(json->root()));
    CORRADE_COMPARE(out.str(), formatString("Utility::Json::parseDoubles(): invalid floating-point literal -haha at {}:2:5\n", filename));
}

void JsonTest::asTypeWrongType() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Containers::Optional<Json> json = Json::fromString("{}");
    CORRADE_VERIFY(json);

    const JsonToken& root = json->root();

    std::ostringstream out;
    Error redirectError{&out};
    root.asNull();
    root.asBool();
    root.asDouble();
    root.asFloat();
    root.asUnsignedInt();
    root.asInt();
    root.asUnsignedLong();
    #ifndef CORRADE_TARGET_32BIT
    root.asLong();
    #endif
    root.asSize();
    root.asString();
    const char* expected =
        "Utility::JsonToken::asNull(): token is a parsed Utility::JsonToken::Type::Object\n"
        "Utility::JsonToken::asBool(): token is a parsed Utility::JsonToken::Type::Object\n"
        "Utility::JsonToken::asDouble(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        "Utility::JsonToken::asFloat(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        "Utility::JsonToken::asUnsignedInt(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        "Utility::JsonToken::asInt(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        "Utility::JsonToken::asUnsignedLong(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        #ifndef CORRADE_TARGET_32BIT
        "Utility::JsonToken::asLong(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        #endif
        "Utility::JsonToken::asSize(): token is a Utility::JsonToken::Type::Object parsed as Utility::JsonToken::ParsedType::Other\n"
        "Utility::JsonToken::asString(): token is a parsed Utility::JsonToken::Type::Object\n";
    CORRADE_COMPARE(out.str(), expected);
}

void JsonTest::asTypeNotParsed() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Containers::Optional<Json> json = Json::fromString(R"([
        nOOO, fALSE, -yey, "\uhh"
    ])");
    CORRADE_VERIFY(json);

    std::ostringstream out;
    Error redirectError{&out};
    json->tokens()[1].asNull();
    json->tokens()[2].asBool();
    json->tokens()[3].asDouble();
    json->tokens()[3].asFloat();
    json->tokens()[3].asUnsignedInt();
    json->tokens()[3].asInt();
    json->tokens()[3].asUnsignedLong();
    #ifndef CORRADE_TARGET_32BIT
    json->tokens()[3].asLong();
    #endif
    json->tokens()[3].asSize();
    json->tokens()[4].asString();
    const char* expected =
        "Utility::JsonToken::asNull(): token is an unparsed Utility::JsonToken::Type::Null\n"
        "Utility::JsonToken::asBool(): token is an unparsed Utility::JsonToken::Type::Bool\n"
        "Utility::JsonToken::asDouble(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        "Utility::JsonToken::asFloat(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        "Utility::JsonToken::asUnsignedInt(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        "Utility::JsonToken::asInt(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        "Utility::JsonToken::asUnsignedLong(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        #ifndef CORRADE_TARGET_32BIT
        "Utility::JsonToken::asLong(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        #endif
        "Utility::JsonToken::asSize(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::None\n"
        "Utility::JsonToken::asString(): token is an unparsed Utility::JsonToken::Type::String\n";
    CORRADE_COMPARE(out.str(), expected);
}

void JsonTest::asTypeWrongParsedType() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Containers::Optional<Json> json = Json::fromString(R"([
        35.7, -35.7, 25, -17
    ])");
    CORRADE_VERIFY(json);
    CORRADE_COMPARE(json->tokens().size(), 5);

    json->parseDoubles(json->tokens()[1]);
    json->parseFloats(json->tokens()[2]);
    json->parseUnsignedInts(json->tokens()[3]);
    json->parseInts(json->tokens()[4]);

    /* Deliberately trying to get doubles as floats or ints as longs. Currently
       that fails but might be deemed too restrictive in future and relaxed. */
    std::ostringstream out;
    Error redirectError{&out};
    json->tokens()[2].asDouble();
    json->tokens()[1].asFloat();
    json->tokens()[4].asUnsignedInt();
    json->tokens()[3].asInt();
    json->tokens()[3].asUnsignedLong();
    #ifndef CORRADE_TARGET_32BIT
    json->tokens()[4].asLong();
    #endif
    json->tokens()[4].asSize();
    const char* expected =
        "Utility::JsonToken::asDouble(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::Float\n"
        "Utility::JsonToken::asFloat(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::Double\n"
        "Utility::JsonToken::asUnsignedInt(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::Int\n"
        "Utility::JsonToken::asInt(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::UnsignedInt\n"
        "Utility::JsonToken::asUnsignedLong(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::UnsignedInt\n"
        #ifndef CORRADE_TARGET_32BIT
        "Utility::JsonToken::asLong(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::Int\n"
        #endif
        "Utility::JsonToken::asSize(): token is a Utility::JsonToken::Type::Number parsed as Utility::JsonToken::ParsedType::Int\n";
    CORRADE_COMPARE(out.str(), expected);
}

#ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
void JsonTest::tokenConstructCopy() {
    CORRADE_VERIFY(std::is_trivially_copyable<JsonToken>{});
}
#endif

void JsonTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<Json>{});
    CORRADE_VERIFY(!std::is_copy_assignable<Json>{});
}

void JsonTest::constructMove() {
    Containers::Optional<Json> a = Json::fromString("\"\\\\\"", Json::Option::ParseStrings);
    CORRADE_VERIFY(a);

    Json b = *std::move(a);
    CORRADE_COMPARE(b.root().type(), JsonToken::Type::String);
    CORRADE_COMPARE(b.root().data(), "\"\\\\\"");
    CORRADE_VERIFY(b.root().isParsed());
    CORRADE_COMPARE(b.root().asString(), "\\");

    Containers::Optional<Json> c = Json::fromString("{}");
    CORRADE_VERIFY(c);

    c = std::move(b);
    CORRADE_COMPARE(c->root().type(), JsonToken::Type::String);
    CORRADE_COMPARE(c->root().data(), "\"\\\\\"");
    CORRADE_VERIFY(c->root().isParsed());
    CORRADE_COMPARE(c->root().asString(), "\\");

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Json>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Json>::value);
}

void JsonTest::debugTokenType() {
    std::ostringstream out;
    Debug{&out} << JsonToken::Type::Number << JsonToken::Type(0xdeadbabedeadbabe);
    {
        #ifdef CORRADE_TARGET_32BIT
        CORRADE_EXPECT_FAIL("Debug has shitty hex printing currently, using just the low 32 bits on 32-bit platforms.");
        #endif
        CORRADE_COMPARE(out.str(), "Utility::JsonToken::Type::Number Utility::JsonToken::Type(0xdeadbabedeadbabe)\n");
    }
    #ifdef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Utility::JsonToken::Type::Number Utility::JsonToken::Type(0xdeadbabe)\n");
    #endif
}

void JsonTest::debugTokenParsedType() {
    std::ostringstream out;
    Debug{&out} << JsonToken::ParsedType::UnsignedInt << JsonToken::ParsedType(0xdeadbabedeadbabeull);
    {
        #ifdef CORRADE_TARGET_32BIT
        CORRADE_EXPECT_FAIL("Debug has shitty hex printing currently, using just the low 32 bits on 32-bit platforms.");
        #endif
        CORRADE_COMPARE(out.str(), "Utility::JsonToken::ParsedType::UnsignedInt Utility::JsonToken::ParsedType(0xdeadbabedeadbabe)\n");
    }
    #ifdef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out.str(), "Utility::JsonToken::ParsedType::UnsignedInt Utility::JsonToken::ParsedType(0xdeadbabe)\n");
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::JsonTest)
