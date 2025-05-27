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

#include <cmath> /* NAN, INFINITY */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/Json.h"
#include "Corrade/Utility/JsonWriter.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

using namespace Containers::Literals;

struct JsonWriterTest: TestSuite::Tester {
    public:
        explicit JsonWriterTest();

        void emptyState();

        void singleObject();
        void singleArray();
        void singleNull();
        void singleBoolean();
        template<class T> void singleNumber();
        void singleString();
        void singleRawJson();

        void singleEmptyBoolArray();
        template<class T> void singleEmptyNumberArray();
        void singleEmptyStringArray();
        void singleBoolArray();
        template<class T> void singleNumberArray();
        void singleStringArray();

        void simpleObject();
        void simpleArray();
        void compactArray();
        void nested();

        void objectScope();
        void arrayScope();
        void compactArrayScope();

        void escapedString();
        template<class T> void negativeZero();
        void minMaxInteger();
        void unclosedObjectOrArrayOnDestruction();
        void rawJsonInObjectKey();
        void rawJsonInObjectValue();
        void rawJsonInArray();
        void rawJsonTokens();
        void rawJsonParsedTokens();
        void rawJsonTokenStringKey();

        void toStringFlags();
        void toFile();
        void toFileFailed();

        void tooBigIndent();
        void currentArraySizeNoValue();
        void currentArraySizeObject();
        void objectEndButNoObject();
        void arrayEndButNoArray();
        void arrayEndButObjectEndExpected();
        void objectEndButArrayEndExpected();
        void valueButObjectKeyExpected();
        void objectKeyButValueExpected();
        void objectKeyButDocumentEndExpected();
        void valueButDocumentEndExpected();
        void disallowedInCompactArray();
        void toStringOrFileNoValue();
        void toStringOrFileIncompleteObject();
        void toStringOrFileIncompleteObjectValue();
        void toStringOrFileIncompleteArray();

        void invalidFloat();
        void invalidDouble();
        void invalidUnsignedLong();
        void invalidLong();

        void constructCopy();
        void constructMove();
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation;
    const char* finalNewline;
} SingleValueData[]{
    {"", {}, 0, 0, ""},
    {"wrap, typographical space, indent", JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 4, 0, "\n"},
    {"wrap, typographical space, indent, initial indent", JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 4, 56, ""}
};

const struct {
    TestSuite::TestCaseDescriptionSourceLocation name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation, wrapAfter;
    Containers::StringView expectedEmpty,
        expectedBool,
        expectedNumber,
        expectedString;
} SingleArrayValueData[]{
    {"", {}, 0, 0, 0,
        R"([])",
        R"([true,false,true,false])",
        R"([1,2,3,4])",
        /* Not using a raw string literal here just to verify it's indeed
           escaped properly. Other cases have it for brevity. */
        "[\"\\n\",\"a\",\"b\",\"\\t\"]"},
    {"no wrapping, non-zero indent, wrap after 1", {}, 8, 56, 1,
        /* Wrap after and indent should get ignored */
        R"([])",
        R"([true,false,true,false])",
        R"([1,2,3,4])",
        R"(["\n","a","b","\t"])"},
    {"no wrapping, typographical space, non-zero indent, wrap after 1", JsonWriter::Option::TypographicalSpace, 8, 56, 1,
        /* Wrap after and indent should get ignored */
        R"([])",
        R"([true, false, true, false])",
        R"([1, 2, 3, 4])",
        R"(["\n", "a", "b", "\t"])"},
    {"four-space indent, wrap after 0", JsonWriter::Option::Wrap, 4, 0, 0,
        /* All on the same line so no wrapping */
        R"([]
)",
        R"([true,false,true,false]
)",
        R"([1,2,3,4]
)",
        R"(["\n","a","b","\t"]
)"},
    {"four-space indent, wrap after 2", JsonWriter::Option::Wrap, 4, 0, 2,
        R"([]
)",
        R"([
    true,false,
    true,false
]
)",
        R"([
    1,2,
    3,4
]
)",
        R"([
    "\n","a",
    "b","\t"
]
)"},
    {"nine-space initial indent, two-space indent and a typographical space, wrap after 2", JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2, 9, 2,
        R"([])", /* no final newline */
        R"([
           true, false,
           true, false
         ])", /* no final newline */
        R"([
           1, 2,
           3, 4
         ])", /* no final newline */
        R"([
           "\n", "a",
           "b", "\t"
         ])"} /* no final newline */
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation;
    const char* expected;
} SimpleObjectData[]{
    {"", {}, 0, 0,
        R"({"key":true,"anotherObject":{},"number":-35.765,"nestedArray":[],"that":null})"},
    {"no wrapping, non-zero indent", {}, 8, 56,
        /* Indent should get ignored */
        R"({"key":true,"anotherObject":{},"number":-35.765,"nestedArray":[],"that":null})"},
    {"no wrapping, typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7, 134,
        /* Indent should get ignored */
        R"({"key": true, "anotherObject": {}, "number": -35.765, "nestedArray": [], "that": null})"},
    {"four-space indent",
        JsonWriter::Option::Wrap, 4, 0,
        R"({
    "key":true,
    "anotherObject":{},
    "number":-35.765,
    "nestedArray":[],
    "that":null
}
)"},
    {"nine-space initial indent, two space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2, 9,
        R"({
           "key": true,
           "anotherObject": {},
           "number": -35.765,
           "nestedArray": [],
           "that": null
         })"}, /* no final newline */
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation;
    const char* expected;
} SimpleArrayData[]{
    {"", {}, 0, 0,
        R"([true,"hello",{},-35.765,[],null])"},
    {"no wrapping, non-zero indent", {}, 8, 56,
        /* Indent should get ignored */
        R"([true,"hello",{},-35.765,[],null])"},
    {"no wrapping, typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7, 134,
        /* Indent should get ignored */
        R"([true, "hello", {}, -35.765, [], null])"},
    {"four-space indent",
        JsonWriter::Option::Wrap, 4, 0,
        R"([
    true,
    "hello",
    {},
    -35.765,
    [],
    null
]
)"},
    {"nine-space initial indent, two-space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2, 9,
        /* No change in expected output compared to above */
        R"([
           true,
           "hello",
           {},
           -35.765,
           [],
           null
         ])"},  /* no final newline */
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation, wrapAfter;
    const char* expected;
} CompactArrayData[]{
    /* Tests similar cases as SingleArrayValueData */
    {"", {}, 0, 0, 0,
        R"([13,5.5,"yes",null,true])"},
    {"no wrapping, non-zero indent, wrap after 1", {}, 8, 56, 1,
        /* Wrap after and indent should get ignored */
        R"([13,5.5,"yes",null,true])"},
    {"no wrapping, typographical space, non-zero indent, wrap after 1", JsonWriter::Option::TypographicalSpace, 8, 56, 1,
        /* Wrap after and indent should get ignored */
        R"([13, 5.5, "yes", null, true])"},
    {"four-space indent, wrap after 0", JsonWriter::Option::Wrap, 4, 0, 0,
        /* All on the same line so no wrapping */
        R"([13,5.5,"yes",null,true]
)"},
    {"four-space indent, wrap after 3", JsonWriter::Option::Wrap, 4, 0, 3,
        R"([
    13,5.5,"yes",
    null,true
]
)"},
    {"nine-space initial indent, two-space indent and a typographical space, wrap after 3", JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2, 9, 3,
        R"([
           13, 5.5, "yes",
           null, true
         ])"},/* no final newline */
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation, initialIndentation;
    const char* expected;
} NestedData[]{
    {"", {}, 0, 0,
        R"([{"hello":5,"yes":true,"matrix":[[0,1],[2,3]],"matrixAsArray":[0,1,2,3],"braces":{"again":{}}},-15.75,"bye!",[]])"},
    {"non-zero indent", {}, 8, 56,
        /* Indent should get ignored */
        R"([{"hello":5,"yes":true,"matrix":[[0,1],[2,3]],"matrixAsArray":[0,1,2,3],"braces":{"again":{}}},-15.75,"bye!",[]])"},
    {"typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7, 134,
        /* Indent should get ignored */
        R"([{"hello": 5, "yes": true, "matrix": [[0, 1], [2, 3]], "matrixAsArray": [0, 1, 2, 3], "braces": {"again": {}}}, -15.75, "bye!", []])"},
    {"four-space indent",
        JsonWriter::Option::Wrap, 4, 0,
        R"([
    {
        "hello":5,
        "yes":true,
        "matrix":[
            [
                0,
                1
            ],
            [
                2,
                3
            ]
        ],
        "matrixAsArray":[
            0,1,
            2,3
        ],
        "braces":{
            "again":{}
        }
    },
    -15.75,
    "bye!",
    []
]
)"},
    {"nine-space initial indent, two-space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2, 9,
        R"([
           {
             "hello": 5,
             "yes": true,
             "matrix": [
               [
                 0,
                 1
               ],
               [
                 2,
                 3
               ]
             ],
             "matrixAsArray": [
               0, 1,
               2, 3
             ],
             "braces": {
               "again": {}
             }
           },
           -15.75,
           "bye!",
           []
         ])"},  /* no final newline */
};

const struct {
    const char* name;
    float floatValue;
    double doubleValue;
    const char* message;
} InvalidFloatDoubleData[]{
    /* MinGW is stupid and has NAN as a double, while it's said to be a float
       by the standard and all other compilers. So have to cast in both
       cases. */
    {"NaN", float(NAN), double(NAN),
        /* MSVC (w/o clang-cl) before 2019 shows -nan(ind), clang-cl shows
           -nan(ind) in 2022 17.10 but not in 2022 17.11 */
        #if defined(CORRADE_TARGET_MSVC) && (_MSC_VER < 1920 /* MSVC <2019 */ || (defined(CORRADE_TARGET_CLANG_CL) && _MSC_VER < 1941 /* <17.11 */))
        "-nan(ind)"
        #else
        "nan"
        #endif
    },
    /* Same for INFINITY on MinGW */
    {"Infinity", float(INFINITY), double(INFINITY),
        "inf"},
};

const struct {
    const char* name;
    std::int64_t value;
    const char* message;
} InvalidLongData[]{
    {"too small", -4503599627370497ll,
        "-4503599627370497"},
    {"too large", 4503599627370496ll,
        "4503599627370496"},
};

typedef typename std::conditional<std::is_same<std::uint64_t, unsigned long>::value, unsigned long long, unsigned long>::type TheOtherUnsignedLongType;
typedef typename std::conditional<std::is_same<std::int64_t, long>::value, long long, long>::type TheOtherLongType;

JsonWriterTest::JsonWriterTest() {
    addInstancedTests({&JsonWriterTest::emptyState,

                       &JsonWriterTest::singleObject,
                       &JsonWriterTest::singleArray,
                       &JsonWriterTest::singleNull,
                       &JsonWriterTest::singleBoolean,
                       &JsonWriterTest::singleNumber<float>,
                       &JsonWriterTest::singleNumber<double>,
                       &JsonWriterTest::singleNumber<std::uint32_t>,
                       &JsonWriterTest::singleNumber<std::int32_t>,
                       &JsonWriterTest::singleNumber<std::uint64_t>,
                       &JsonWriterTest::singleNumber<std::int64_t>,
                       &JsonWriterTest::singleNumber<TheOtherUnsignedLongType>,
                       &JsonWriterTest::singleNumber<TheOtherLongType>,
                       /* Explicitly verifying this doesn't cause any ambiguity */
                       &JsonWriterTest::singleNumber<std::size_t>,
                       &JsonWriterTest::singleString,
                       &JsonWriterTest::singleRawJson},
        Containers::arraySize(SingleValueData));

    addInstancedTests({
        &JsonWriterTest::singleEmptyBoolArray,
        &JsonWriterTest::singleEmptyNumberArray<float>,
        &JsonWriterTest::singleEmptyNumberArray<double>,
        &JsonWriterTest::singleEmptyNumberArray<std::uint32_t>,
        &JsonWriterTest::singleEmptyNumberArray<std::int32_t>,
        &JsonWriterTest::singleEmptyNumberArray<std::uint64_t>,
        &JsonWriterTest::singleEmptyNumberArray<std::int64_t>,
        &JsonWriterTest::singleEmptyNumberArray<TheOtherUnsignedLongType>,
        &JsonWriterTest::singleEmptyNumberArray<TheOtherLongType>,
        /* Explicitly verifying this doesn't cause any ambiguity */
        &JsonWriterTest::singleEmptyNumberArray<std::size_t>,
        &JsonWriterTest::singleEmptyStringArray,
        &JsonWriterTest::singleBoolArray,
        &JsonWriterTest::singleNumberArray<float>,
        &JsonWriterTest::singleNumberArray<double>,
        &JsonWriterTest::singleNumberArray<std::uint32_t>,
        &JsonWriterTest::singleNumberArray<std::int32_t>,
        &JsonWriterTest::singleNumberArray<std::uint64_t>,
        &JsonWriterTest::singleNumberArray<std::int64_t>,
        &JsonWriterTest::singleNumberArray<TheOtherUnsignedLongType>,
        &JsonWriterTest::singleNumberArray<TheOtherLongType>,
        /* Explicitly verifying this doesn't cause any ambiguity */
        &JsonWriterTest::singleNumberArray<std::size_t>,
        &JsonWriterTest::singleStringArray
    }, Containers::arraySize(SingleArrayValueData));

    addInstancedTests({&JsonWriterTest::simpleObject},
        Containers::arraySize(SimpleObjectData));

    addInstancedTests({&JsonWriterTest::simpleArray},
        Containers::arraySize(SimpleArrayData));

    addInstancedTests({&JsonWriterTest::compactArray},
        Containers::arraySize(CompactArrayData));

    addInstancedTests({&JsonWriterTest::nested},
        Containers::arraySize(NestedData));

    addTests({&JsonWriterTest::objectScope,
              &JsonWriterTest::arrayScope,
              &JsonWriterTest::compactArrayScope,

              &JsonWriterTest::escapedString,
              &JsonWriterTest::negativeZero<float>,
              &JsonWriterTest::negativeZero<double>,
              &JsonWriterTest::minMaxInteger,
              &JsonWriterTest::unclosedObjectOrArrayOnDestruction,
              &JsonWriterTest::rawJsonInObjectKey,
              &JsonWriterTest::rawJsonInObjectValue,
              &JsonWriterTest::rawJsonInArray,
              &JsonWriterTest::rawJsonTokens,
              &JsonWriterTest::rawJsonParsedTokens,
              &JsonWriterTest::rawJsonTokenStringKey,

              &JsonWriterTest::toStringFlags,
              &JsonWriterTest::toFile,
              &JsonWriterTest::toFileFailed,

              &JsonWriterTest::tooBigIndent,
              &JsonWriterTest::currentArraySizeNoValue,
              &JsonWriterTest::currentArraySizeObject,
              &JsonWriterTest::objectEndButNoObject,
              &JsonWriterTest::arrayEndButNoArray,
              &JsonWriterTest::arrayEndButObjectEndExpected,
              &JsonWriterTest::objectEndButArrayEndExpected,
              &JsonWriterTest::valueButObjectKeyExpected,
              &JsonWriterTest::objectKeyButValueExpected,
              &JsonWriterTest::objectKeyButDocumentEndExpected,
              &JsonWriterTest::valueButDocumentEndExpected,
              &JsonWriterTest::disallowedInCompactArray,
              &JsonWriterTest::toStringOrFileNoValue,
              &JsonWriterTest::toStringOrFileIncompleteObject,
              &JsonWriterTest::toStringOrFileIncompleteObjectValue,
              &JsonWriterTest::toStringOrFileIncompleteArray});

    addInstancedTests({&JsonWriterTest::invalidFloat,
                       &JsonWriterTest::invalidDouble},
        Containers::arraySize(InvalidFloatDoubleData));

    addTests({&JsonWriterTest::invalidUnsignedLong});

    addInstancedTests({&JsonWriterTest::invalidLong},
        Containers::arraySize(InvalidLongData));

    addTests({&JsonWriterTest::constructCopy,
              &JsonWriterTest::constructMove});
}

void JsonWriterTest::emptyState() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    CORRADE_COMPARE(json.size(), 0);
    CORRADE_VERIFY(json.isEmpty());
}

void JsonWriterTest::singleObject() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.beginObject();

    /* At this point, the size should be a single character */
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), 1);

    json.endObject();

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "{}"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleArray() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.beginArray();

    /* At this point, the size should be a single character, and 0 items in the
       array */
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), 1);
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json.endArray();

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "[]"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleNull() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.write(nullptr);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "null"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleBoolean() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.write(true);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "true"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

template<class> struct NameTraits;
#define _c(type) \
    template<> struct NameTraits<type> { \
        static const char* name() { return #type; } \
    };
_c(float)
_c(double)
_c(std::uint32_t)
_c(std::int32_t)
_c(std::uint64_t)
_c(std::int64_t)
_c(TheOtherUnsignedLongType)
_c(TheOtherLongType)
#undef _c

template<class T> void JsonWriterTest::singleNumber() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(NameTraits<T>::name());

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.write(T{35});

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "35"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleString() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.write("hello");

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "\"hello\""_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleRawJson() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json
        .writeJson("{\"key\": none, /* HEY JSON HOW ARE YA */ }");

    /* Except for the final newline, the result should be same regardless of
       the indentation setting. The final newline should be added and counted
       into size() even before toString() is called. */
    Containers::String expected = "{\"key\": none, /* HEY JSON HOW ARE YA */ }"_s + data.finalNewline;
    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), expected.size());
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::singleEmptyBoolArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.writeArray(Containers::StridedBitArrayView1D{}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedEmpty.size());
    CORRADE_COMPARE(json.toString(), data.expectedEmpty);
}

template<class T> void JsonWriterTest::singleEmptyNumberArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(NameTraits<T>::name());

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.writeArray(Containers::StridedArrayView1D<const T>{}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedEmpty.size());
    CORRADE_COMPARE(json.toString(), data.expectedEmpty);
}

void JsonWriterTest::singleEmptyStringArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.writeArray(Containers::StringIterable{}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedEmpty.size());
    CORRADE_COMPARE(json.toString(), data.expectedEmpty);
}

void JsonWriterTest::singleBoolArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.writeArray({true, false, true, false}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedBool.size());
    CORRADE_COMPARE(json.toString(), data.expectedBool);
}

template<class T> void JsonWriterTest::singleNumberArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(NameTraits<T>::name());

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.writeArray({T(1), T(2), T(3), T(4)}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedNumber.size());
    CORRADE_COMPARE(json.toString(), data.expectedNumber);
}

void JsonWriterTest::singleStringArray() {
    auto&& data = SingleArrayValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    /* Using one _s literal to disambiguate from std::initializer_list<bool>.
       I wonder how much extra pain this will cause. */
    json.writeArray({"\n", "a"_s, "b", "\t"}, data.wrapAfter);

    CORRADE_VERIFY(!json.isEmpty());
    CORRADE_COMPARE(json.size(), data.expectedString.size());
    CORRADE_COMPARE(json.toString(), data.expectedString);
}

void JsonWriterTest::simpleObject() {
    auto&& data = SimpleObjectData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};

    Containers::StringView out = json
        .beginObject()
            .writeKey("key").write(true)
            .writeKey("anotherObject").beginObject().endObject()
            .writeKey("number").write(-35.765f)
            .writeKey("nestedArray").beginArray().endArray()
            .writeKey("that").write(nullptr)
        .endObject()
        .toString();
    CORRADE_COMPARE(out, data.expected);
}

void JsonWriterTest::simpleArray() {
    auto&& data = SimpleArrayData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json.write(true);
    CORRADE_COMPARE(json.currentArraySize(), 1);

    json.write("hello");
    CORRADE_COMPARE(json.currentArraySize(), 2);

    json.beginObject().endObject();
    CORRADE_COMPARE(json.currentArraySize(), 3);

    json.write(-35.765f);
    CORRADE_COMPARE(json.currentArraySize(), 4);

    json.beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json.endArray();
    CORRADE_COMPARE(json.currentArraySize(), 5);

    json.write(nullptr);
    CORRADE_COMPARE(json.currentArraySize(), 6);

    Containers::StringView out = json.endArray().toString();
    CORRADE_COMPARE(out, data.expected);
}

void JsonWriterTest::compactArray() {
    auto&& data = CompactArrayData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};
    json.beginCompactArray(data.wrapAfter);
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json.write(13);
    CORRADE_COMPARE(json.currentArraySize(), 1);

    json.write(5.5);
    CORRADE_COMPARE(json.currentArraySize(), 2);

    json.write("yes");
    CORRADE_COMPARE(json.currentArraySize(), 3);

    json.write(nullptr);
    CORRADE_COMPARE(json.currentArraySize(), 4);

    json.write(true);
    CORRADE_COMPARE(json.currentArraySize(), 5);

    Containers::StringView out = json.endArray().toString();
    CORRADE_COMPARE(out, data.expected);
}

void JsonWriterTest::nested() {
    auto&& data = NestedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation, data.initialIndentation};

    json.beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json    .beginObject()
                .writeKey("hello").write(5)
                .writeKey("yes").write(true)
                .writeKey("matrix")
                    .beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json                .beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json                    .write(0).write(1);
    CORRADE_COMPARE(json.currentArraySize(), 2);

    json                .endArray();
    CORRADE_COMPARE(json.currentArraySize(), 1);

    json                .beginArray();
    CORRADE_COMPARE(json.currentArraySize(), 0);

    json                    .write(2).write(3);
    CORRADE_COMPARE(json.currentArraySize(), 2);

    json                .endArray();
    CORRADE_COMPARE(json.currentArraySize(), 2);

    json            .endArray()
                .writeKey("matrixAsArray").writeArray({0, 1, 2, 3}, 2)
                .writeKey("braces")
                    .beginObject()
                        .writeKey("again").beginObject().endObject()
                    .endObject()
            .endObject();
    CORRADE_COMPARE(json.currentArraySize(), 1);

    json    .write(-15.75)
            .write("bye!")
            .beginArray().endArray();
    CORRADE_COMPARE(json.currentArraySize(), 4);

    Containers::StringView out = json.endArray().toString();
    CORRADE_COMPARE(out, data.expected);
}

void JsonWriterTest::objectScope() {
    JsonWriter json;

    {
        Containers::ScopeGuard object = json.beginObjectScope();

        json.writeKey("hello").write("there")
            .writeKey("works").write(true);
    }

    /* GCC 4.8 can't handle raw string literals in macros */
    const char* expected = R"({"hello":"there","works":true})";
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::arrayScope() {
    JsonWriter json;

    {
        Containers::ScopeGuard array = json.beginArrayScope();

        json.write("hello!")
            .write("works?")
            .write(true);
    }

    /* GCC 4.8 can't handle raw string literals in macros */
    const char* expected = R"(["hello!","works?",true])";
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::compactArrayScope() {
    /* Using an indented formatter to test that this doesn't do the same as
       beginArrayScope() */
    JsonWriter json{JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 2};

    {
        Containers::ScopeGuard array = json.beginCompactArrayScope(2);

        json.write(13)
            .write(5.5f)
            .write("yes")
            .write(true);
    }

    /* GCC 4.8 can't handle raw string literals in macros */
    const char* expected = R"([
  13, 5.5,
  "yes", true
]
)";
    CORRADE_COMPARE(json.toString(), expected);
}

void JsonWriterTest::escapedString() {
    JsonWriter json;

    /* UTF-8 doesn't get escaped; / also not */
    CORRADE_COMPARE(json.write("\"a\\h/o\bj\r \fs\nv\tě\"te!").toString(),
        "\"\\\"a\\\\h/o\\bj\\r \\fs\\nv\\tě\\\"te!\"");
}

template<class T> void JsonWriterTest::negativeZero() {
    setTestCaseTemplateName(NameTraits<T>::name());

    JsonWriter json;
    CORRADE_COMPARE(json.write(T(-0.0)).toString(), "-0");
}

void JsonWriterTest::minMaxInteger() {
    JsonWriter json;
    CORRADE_COMPARE(json
        .beginArray()
        .write(-4503599627370496ll)
        .write(4503599627370495ll)
        .write(4503599627370495ull)
        .endArray().toString(),
        "[-4503599627370496,4503599627370495,4503599627370495]");
}

void JsonWriterTest::unclosedObjectOrArrayOnDestruction() {
    {
        JsonWriter json;
        json.beginArray()
            .beginObject();
    }

    /* This is fine as long as we don't call toString() or toFile() */
    CORRADE_VERIFY(true);
}

void JsonWriterTest::rawJsonInObjectKey() {
    /* Accidentally using writeJson() for writing a key is tested in
       objectKeyButValueExpected() */

    JsonWriter json;
    json.beginObject()
        .writeJsonKey("/* A comment*/ \"key\"")
        .write(-13)
        .writeJsonKey("another")
        .write(false)
        .endObject();
    CORRADE_COMPARE(json.toString(), "{/* A comment*/ \"key\":-13,another:false}");
}

void JsonWriterTest::rawJsonInObjectValue() {
    JsonWriter json;
    json.beginObject()
        .writeKey("key")
        .writeJson("/* A comment */ false")
        /* Test using it more than once to verify it doesn't do something
           unexpected */
        .writeKey("another")
        .writeJson("-13")
        .endObject();
    CORRADE_COMPARE(json.toString(), "{\"key\":/* A comment */ false,\"another\":-13}");
}

void JsonWriterTest::rawJsonInArray() {
    JsonWriter json;
    json.beginArray()
        .writeJson("/* A comment */ 6776")
        /* Test using it more than once to verify it prints a comma before */
        .writeJson("0x3567")
        .endArray();
    CORRADE_COMPARE(json.toString(), "[/* A comment */ 6776,0x3567]");
}

void JsonWriterTest::rawJsonTokens() {
    /* The output should be exactly the same */
    const char* json = R"([null,[],true,{},6.52,{"key":"value","\"escaped\"":"\"also\""}])";
    Containers::Optional<Json> input = Json::fromString(json);
    CORRADE_VERIFY(input);

    JsonWriter output;
    output.writeJson(input->root());
    CORRADE_COMPARE(output.toString(), json);
}

void JsonWriterTest::rawJsonParsedTokens() {
    /* Like rawJsonTokens(), but expanded to cover all possible parsed types */
    Containers::Array<JsonTokenOffsetSize> offsetsSizes{InPlaceInit, {
        {},     /* 0 */
        {},     /* 1 */
        {},     /* 2 */
        {},     /* 3 */
        {},     /* 4 */
        {},     /* 5 */
        {},     /* 6 */
        {},     /* 7 */
        {},     /* 8 */
        {},     /* 9 */
        {},     /* 10 */
        {},     /* 11 */
        {0, 5}, /* 12 */
        {5, 7}, /* 13 */
        {},     /* 14 */
        {},     /* 15 */
    }};

    /*          1      6         */
    Json input{"\"key\"\"value\"", {InPlaceInit, {
        JsonTokenData{JsonToken::Type::Array, 15},   /* 0 */
        JsonTokenData{nullptr},                     /* 1 */
        JsonTokenData{JsonToken::Type::Array, 0},   /* 2 */
        JsonTokenData{true},                        /* 3 */
        JsonTokenData{JsonToken::Type::Object, 0},  /* 4 */
        JsonTokenData{6.52f},                       /* 5 */
        JsonTokenData{6.52, offsetsSizes[6]},       /* 6 */
        JsonTokenData{652u},                        /* 7 */
        JsonTokenData{-652},                        /* 8 */
        JsonTokenData{652ull, offsetsSizes[9]},     /* 9 */
        JsonTokenData{-652ll, offsetsSizes[10]},    /* 10 */
        JsonTokenData{JsonToken::Type::Object, 4},  /* 11 */
        JsonTokenData{JsonToken::Type::String, ~std::uint64_t{}, true}, /* 12 */
        JsonTokenData{JsonToken::Type::String, ~std::uint64_t{}}, /* 13 */
        JsonTokenData{JsonToken::Type::String, 0, true}, /* 14 */
        JsonTokenData{JsonToken::Type::String, 1}, /* 15 */
    }}, Utility::move(offsetsSizes), {InPlaceInit, {
        "\"escaped\"",
        "\"also\""
    }}};

    JsonWriter output;
    output.writeJson(input.root());
    /* Can't use raw string literals because idiotic MSVC 2015, 2017, 2019
       *and* 2022 then thinks the `"\"escaped` is an "invalid escape sequence"
       and subsequently a user-defined literal named `escaped`?! What the
       hell. */
    CORRADE_COMPARE(output.toString(), "[null,[],true,{},6.52,6.52,652,-652,652,-652,{\"key\":\"value\",\"\\\"escaped\\\"\":\"\\\"also\\\"\"}]");
}

void JsonWriterTest::rawJsonTokenStringKey() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::Optional<Json> unparsed = Json::fromString(R"({"key":"value"})");
    CORRADE_VERIFY(unparsed);

    Json parsed{"\"key\"\"value\"", {InPlaceInit, {
        JsonTokenData{JsonToken::Type::Object, 2},
        JsonTokenData{JsonToken::Type::String, ~std::uint64_t{}, true},
        JsonTokenData{JsonToken::Type::String, ~std::uint64_t{}},
    }}, {InPlaceInit, {{}, {0, 5}, {5, 7}}}, {}};

    JsonWriter output;
    output.beginArray();

    /* These are fine */
    output.writeJson(unparsed->tokens()[2]);
    output.writeJson(parsed.tokens()[2]);

    Containers::String out;
    Error redirectError{&out};
    output.writeJson(unparsed->tokens()[1]);
    output.writeJson(parsed.tokens()[1]);
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::writeJson(): expected a value token but got an object key\n"
        "Utility::JsonWriter::writeJson(): expected a value token but got an object key\n");
}

void JsonWriterTest::toStringFlags() {
    JsonWriter json;
    Containers::StringView out = json
        .write("heya")
        .toString();
    CORRADE_COMPARE(out.flags(), Containers::StringViewFlag::NullTerminated);
    CORRADE_COMPARE(out[out.size()], '\0');
}

void JsonWriterTest::toFile() {
    JsonWriter json;
    json.write("heya");

    Containers::String filename = Path::join(JSONWRITER_TEST_DIR, "file.json");
    CORRADE_VERIFY(Path::make(JSONWRITER_TEST_DIR));
    CORRADE_VERIFY(json.toFile(filename));
    CORRADE_COMPARE_AS(filename, "\"heya\"", TestSuite::Compare::FileToString);
}

void JsonWriterTest::toFileFailed() {
    /* Attempt to write to a directory, which is easier than trying to find a
       platform-specific unwritable location */

    JsonWriter json;
    json.write("heya");

    CORRADE_VERIFY(Path::make(JSONWRITER_TEST_DIR));

    Containers::String out;
    Error redirectError{&out};
    CORRADE_VERIFY(!json.toFile(JSONWRITER_TEST_DIR));
    /* There's an error from Path::write() before */
    CORRADE_COMPARE_AS(out,
        "\nUtility::JsonWriter::toFile(): can't write to " JSONWRITER_TEST_DIR "\n",
        TestSuite::Compare::StringHasSuffix);
}

void JsonWriterTest::tooBigIndent() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    JsonWriter{{}, 9};
    CORRADE_COMPARE(out, "Utility::JsonWriter: indentation can be at most 8 characters, got 9\n");
}

void JsonWriterTest::currentArraySizeNoValue() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.currentArraySize();
    CORRADE_COMPARE(out, "Utility::JsonWriter::currentArraySize(): not in an array\n");
}

void JsonWriterTest::currentArraySizeObject() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginArray()
            .beginObject()
            .writeKey("hello");

    Containers::String out;
    Error redirectError{&out};
    json.currentArraySize();
    CORRADE_COMPARE(out, "Utility::JsonWriter::currentArraySize(): not in an array\n");
}

void JsonWriterTest::objectEndButNoObject() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.endObject();
    CORRADE_COMPARE(out, "Utility::JsonWriter::endObject(): expected a value\n");
}

void JsonWriterTest::arrayEndButNoArray() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.endArray();
    CORRADE_COMPARE(out, "Utility::JsonWriter::endArray(): expected a value\n");
}

void JsonWriterTest::arrayEndButObjectEndExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginObject();

    Containers::String out;
    Error redirectError{&out};
    json.endArray();
    CORRADE_COMPARE(out, "Utility::JsonWriter::endArray(): expected an object key or object end\n");
}

void JsonWriterTest::objectEndButArrayEndExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginArray();

    Containers::String out;
    Error redirectError{&out};
    json.endObject();
    CORRADE_COMPARE(out, "Utility::JsonWriter::endObject(): expected an array value or array end\n");
}

void JsonWriterTest::valueButObjectKeyExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Json tokens{{}, {InPlaceInit, {
        JsonTokenData{false},
    }}, {InPlaceInit, {{}}}, {}};

    JsonWriter json;
    json.beginObject();

    Containers::String out;
    Error redirectError{&out};
    json.write("hello")
        .writeArray({5})
        .writeJson("false")
        .writeJson(tokens.root());
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::write(): expected an object key or object end\n"
        "Utility::JsonWriter::writeArray(): expected an object key or object end\n"
        "Utility::JsonWriter::writeJson(): expected an object key or object end\n"
        "Utility::JsonWriter::writeJson(): expected an object key or object end\n");
}

void JsonWriterTest::objectKeyButValueExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginObject()
        .writeKey("hi");

    Containers::String out;
    Error redirectError{&out};
    json.writeKey("hello")
        .writeJsonKey("\"hello?\"");
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::writeKey(): expected an object value\n"
        "Utility::JsonWriter::writeJsonKey(): expected an object value\n");
}

void JsonWriterTest::objectKeyButDocumentEndExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.write("hi");

    Containers::String out;
    Error redirectError{&out};
    json.writeKey("hello")
        .writeJsonKey("\"hello?\"");
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::writeKey(): expected document end\n"
        "Utility::JsonWriter::writeJsonKey(): expected document end\n");
}

void JsonWriterTest::valueButDocumentEndExpected() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Json tokens{{}, {InPlaceInit, {
        JsonTokenData{false},
    }}, {InPlaceInit, {{}}}, {}};

    JsonWriter json;
    json.write("hi");

    Containers::String out;
    Error redirectError{&out};
    json.write("hello")
        .writeArray({5})
        .writeJson("/* HI JSON CAN YOU COMMENT */")
        .writeJson(tokens.root());
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::write(): expected document end\n"
        "Utility::JsonWriter::writeArray(): expected document end\n"
        "Utility::JsonWriter::writeJson(): expected document end\n"
        "Utility::JsonWriter::writeJson(): expected document end\n");
}

void JsonWriterTest::disallowedInCompactArray() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Json tokens{{}, {InPlaceInit, {
        JsonTokenData{false},
    }}, {InPlaceInit, {{}}}, {}};

    JsonWriter json;
    json.beginCompactArray();

    Containers::String out;
    Error redirectError{&out};
    json
        .beginObject()
        .beginArray()
        .beginCompactArray()
        .writeArray({5})
        /* These two could eventually get allowed if a compelling use case is
           found, but the assumption is that JSON strings are inherently
           complex with their own internal indentation etc., which would
           significantly break the formatting here. */
        .writeJson("/* HI JSON CAN YOU COMMENT */")
        .writeJson(tokens.root());
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::beginObject(): expected a compact array value or array end\n"
        "Utility::JsonWriter::beginArray(): expected a compact array value or array end\n"
        "Utility::JsonWriter::beginCompactArray(): expected a compact array value or array end\n"
        "Utility::JsonWriter::writeArray(): expected a compact array value or array end\n"
        "Utility::JsonWriter::writeJson(): expected a compact array value or array end\n"
        "Utility::JsonWriter::writeJson(): expected a compact array value or array end\n");
}

void JsonWriterTest::toStringOrFileNoValue() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::toString(): incomplete JSON, expected a value\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected a value\n");
}

void JsonWriterTest::toStringOrFileIncompleteObject() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginObject();

    Containers::String out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::toString(): incomplete JSON, expected an object key or object end\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an object key or object end\n");
}

void JsonWriterTest::toStringOrFileIncompleteObjectValue() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginObject()
        .writeKey("hi");

    Containers::String out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::toString(): incomplete JSON, expected an object value\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an object value\n");
}

void JsonWriterTest::toStringOrFileIncompleteArray() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;
    json.beginArray();

    Containers::String out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::toString(): incomplete JSON, expected an array value or array end\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an array value or array end\n");
}

void JsonWriterTest::invalidFloat() {
    auto&& data = InvalidFloatDoubleData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.write(data.floatValue);
    CORRADE_COMPARE(out, format(
        "Utility::JsonWriter::write(): invalid floating-point value {}\n",
        data.message));
}

void JsonWriterTest::invalidDouble() {
    auto&& data = InvalidFloatDoubleData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.write(data.doubleValue);
    CORRADE_COMPARE(out, format(
        "Utility::JsonWriter::write(): invalid floating-point value {0}\n",
        data.message));
}

void JsonWriterTest::invalidUnsignedLong() {
    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.write(4503599627370496ull);
    CORRADE_COMPARE(out,
        "Utility::JsonWriter::write(): too large integer value 4503599627370496\n");
}

void JsonWriterTest::invalidLong() {
    auto&& data = InvalidLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_SKIP_IF_NO_ASSERT();

    JsonWriter json;

    Containers::String out;
    Error redirectError{&out};
    json.write(data.value);
    CORRADE_COMPARE(out, format(
        "Utility::JsonWriter::write(): too small or large integer value {}\n",
        data.message));
}

void JsonWriterTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<JsonWriter>{});
    CORRADE_VERIFY(!std::is_copy_assignable<JsonWriter>{});
}

void JsonWriterTest::constructMove() {
    JsonWriter a;
    a.beginArray();

    JsonWriter b = Utility::move(a);
    b.write("hey");
    b.endArray();

    JsonWriter c;
    c = Utility::move(b);
    CORRADE_COMPARE(c.toString(), "[\"hey\"]");

    CORRADE_VERIFY(std::is_nothrow_move_constructible<JsonWriter>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<JsonWriter>::value);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::JsonWriterTest)
