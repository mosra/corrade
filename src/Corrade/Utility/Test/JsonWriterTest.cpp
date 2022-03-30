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

#include <cmath> /* NAN, INFINITY */
#include <sstream>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StringStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/Utility/FormatStl.h" /** @todo remove once Debug is stream-free */
#include "Corrade/Utility/JsonWriter.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

using namespace Containers::Literals;

struct JsonWriterTest: TestSuite::Tester {
    public:
        explicit JsonWriterTest();

        void singleObject();
        void singleArray();
        void singleNull();
        void singleBoolean();
        template<class T> void singleNumber();
        void singleString();
        void singleRawJson();

        void simpleObject();
        void simpleArray();
        void nested();

        void objectScope();
        void arrayScope();

        void escapedString();
        template<class T> void negativeZero();
        void minMaxInteger();
        void unclosedObjectOrArrayOnDestruction();
        void rawJsonInObjectKey();
        void rawJsonInObjectValue();
        void rawJsonInArray();

        void toStringFlags();
        void toFile();
        void toFileFailed();

        void tooBigIndent();
        void objectEndButNoObject();
        void arrayEndButNoArray();
        void arrayEndButObjectEndExpected();
        void objectEndButArrayEndExpected();
        void valueButObjectKeyExpected();
        void objectKeyButValueExpected();
        void objectKeyButDocumentEndExpected();
        void valueButDocumentEndExpected();
        void rawJsonButDocumentEndExpected();
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
    std::uint32_t indentation, spacing;
    const char* finalNewline;
} SingleValueData[]{
    {"", {}, 0, 0, ""},
    {"wrap & indent", JsonWriter::Option::Wrap, 4, 1, "\n"}
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation;
    const char* expected;
} SimpleObjectData[]{
    {"", {}, 0,
        R"({"key":true,"anotherObject":{},"number":-35.765,"nestedArray":[],"that":null})"},
    {"no wrapping, non-zero indent", {}, 8,
        /* Indent should get ignored */
        R"({"key":true,"anotherObject":{},"number":-35.765,"nestedArray":[],"that":null})"},
    {"typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7,
        /* Indent should get ignored */
        R"({"key": true, "anotherObject": {}, "number": -35.765, "nestedArray": [], "that": null})"},
    {"two-space indent",
        JsonWriter::Option::Wrap, 2,
        R"({
  "key":true,
  "anotherObject":{},
  "number":-35.765,
  "nestedArray":[],
  "that":null
}
)"},
    {"four-space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 4,
        R"({
    "key": true,
    "anotherObject": {},
    "number": -35.765,
    "nestedArray": [],
    "that": null
}
)"},
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation;
    const char* expected;
} SimpleArrayData[]{
    {"", {}, 0,
        R"([true,"hello",{},-35.765,[],null])"},
    {"non-zero indent", {}, 8,
        /* Indent should get ignored */
        R"([true,"hello",{},-35.765,[],null])"},
    {"typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7,
        /* Indent should get ignored */
        R"([true, "hello", {}, -35.765, [], null])"},
    {"two-space indent",
        JsonWriter::Option::Wrap, 2,
        R"([
  true,
  "hello",
  {},
  -35.765,
  [],
  null
]
)"},
    {"four-space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 4,
        /* No change in expected output compared to above */
        R"([
    true,
    "hello",
    {},
    -35.765,
    [],
    null
]
)"},
};

const struct {
    const char* name;
    JsonWriter::Options options;
    std::uint32_t indentation;
    const char* expected;
} NestedData[]{
    {"", {}, 0,
        R"([{"hello":5,"yes":true,"matrix":[[0,1],[2,3]],"braces":{"again":{}}},-15.75,"bye!",[]])"},
    {"non-zero indent", {}, 8,
        /* Indent should get ignored */
        R"([{"hello":5,"yes":true,"matrix":[[0,1],[2,3]],"braces":{"again":{}}},-15.75,"bye!",[]])"},
    {"typographical space, non-zero indent",
        JsonWriter::Option::TypographicalSpace, 7,
        /* Indent should get ignored */
        R"([{"hello": 5, "yes": true, "matrix": [[0, 1], [2, 3]], "braces": {"again": {}}}, -15.75, "bye!", []])"},
    {"two-space indent",
        JsonWriter::Option::Wrap, 2,
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
    "braces":{
      "again":{}
    }
  },
  -15.75,
  "bye!",
  []
]
)"},
    {"four-space indent and a typographical space",
        JsonWriter::Option::Wrap|JsonWriter::Option::TypographicalSpace, 4,
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
        "braces": {
            "again": {}
        }
    },
    -15.75,
    "bye!",
    []
]
)"},
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
        #if defined(CORRADE_TARGET_MSVC) && (_MSC_VER < 1920 /* MSVC <2019 */ || defined(CORRADE_TARGET_CLANG_CL))
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

JsonWriterTest::JsonWriterTest() {
    addInstancedTests({&JsonWriterTest::singleObject,
                       &JsonWriterTest::singleArray,
                       &JsonWriterTest::singleNull,
                       &JsonWriterTest::singleBoolean,
                       &JsonWriterTest::singleNumber<float>,
                       &JsonWriterTest::singleNumber<double>,
                       &JsonWriterTest::singleNumber<std::uint32_t>,
                       &JsonWriterTest::singleNumber<std::int32_t>,
                       &JsonWriterTest::singleNumber<std::uint64_t>,
                       &JsonWriterTest::singleNumber<std::int64_t>,
                       &JsonWriterTest::singleString,
                       &JsonWriterTest::singleRawJson},
        Containers::arraySize(SingleValueData));

    addInstancedTests({&JsonWriterTest::simpleObject},
        Containers::arraySize(SimpleObjectData));

    addInstancedTests({&JsonWriterTest::simpleArray},
        Containers::arraySize(SimpleArrayData));

    addInstancedTests({&JsonWriterTest::nested},
        Containers::arraySize(NestedData));

    addTests({&JsonWriterTest::objectScope,
              &JsonWriterTest::arrayScope,

              &JsonWriterTest::escapedString,
              &JsonWriterTest::negativeZero<float>,
              &JsonWriterTest::negativeZero<double>,
              &JsonWriterTest::minMaxInteger,
              &JsonWriterTest::unclosedObjectOrArrayOnDestruction,
              &JsonWriterTest::rawJsonInObjectKey,
              &JsonWriterTest::rawJsonInObjectValue,
              &JsonWriterTest::rawJsonInArray,

              &JsonWriterTest::toStringFlags,
              &JsonWriterTest::toFile,
              &JsonWriterTest::toFileFailed,

              &JsonWriterTest::tooBigIndent,
              &JsonWriterTest::objectEndButNoObject,
              &JsonWriterTest::arrayEndButNoArray,
              &JsonWriterTest::arrayEndButObjectEndExpected,
              &JsonWriterTest::objectEndButArrayEndExpected,
              &JsonWriterTest::valueButObjectKeyExpected,
              &JsonWriterTest::objectKeyButValueExpected,
              &JsonWriterTest::objectKeyButDocumentEndExpected,
              &JsonWriterTest::valueButDocumentEndExpected,
              &JsonWriterTest::rawJsonButDocumentEndExpected,
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

void JsonWriterTest::singleObject() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .beginObject()
        .endObject()
        .toString(), format("{{}}{}", data.finalNewline));
}

void JsonWriterTest::singleArray() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .beginArray()
        .endArray()
        .toString(), format("[]{}", data.finalNewline));
}

void JsonWriterTest::singleNull() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .write(nullptr)
        .toString(), format("null{}", data.finalNewline));
}

void JsonWriterTest::singleBoolean() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .write(true)
        .toString(), format("true{}", data.finalNewline));
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
#undef _c

template<class T> void JsonWriterTest::singleNumber() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(NameTraits<T>::name());

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .write(T{35})
        .toString(), format("35{}", data.finalNewline));
}

void JsonWriterTest::singleString() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .write("hello")
        .toString(), format("\"hello\"{}", data.finalNewline));
}

void JsonWriterTest::singleRawJson() {
    auto&& data = SingleValueData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Except for the final newline, the result should be same regardless of
       the indentation setting */
    JsonWriter json{data.options, data.indentation};
    CORRADE_COMPARE(json
        .writeJson("{\"key\": none, /* HEY JSON HOW ARE YA */ }")
        .toString(), format("{{\"key\": none, /* HEY JSON HOW ARE YA */ }}{}", data.finalNewline));
}

void JsonWriterTest::simpleObject() {
    auto&& data = SimpleObjectData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation};

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

    JsonWriter json{data.options, data.indentation};

    Containers::StringView out = json
        .beginArray()
            .write(true)
            .write("hello")
            .beginObject().endObject()
            .write(-35.765f)
            .beginArray().endArray()
            .write(nullptr)
        .endArray()
        .toString();
    CORRADE_COMPARE(out, data.expected);
}

void JsonWriterTest::nested() {
    auto&& data = NestedData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    JsonWriter json{data.options, data.indentation};

    Containers::StringView out = json
        .beginArray()
            .beginObject()
                .writeKey("hello").write(5)
                .writeKey("yes").write(true)
                .writeKey("matrix")
                    .beginArray()
                        .beginArray()
                            .write(0).write(1)
                        .endArray()
                        .beginArray()
                            .write(2).write(3)
                        .endArray()
                    .endArray()
                .writeKey("braces")
                    .beginObject()
                        .writeKey("again").beginObject().endObject()
                    .endObject()
            .endObject()
            .write(-15.75)
            .write("bye!")
            .beginArray().endArray()
        .endArray()
        .toString();
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

void JsonWriterTest::escapedString() {
    JsonWriter json;

    /* UTF-8 doesn't get escaped */
    CORRADE_COMPARE(json.write("\"a\\h/o\bj\r \fs\nv\tě\"te!").toString(),
        "\"\\\"a\\\\h\\/o\\bj\\r \\fs\\nv\\tě\\\"te!\"");
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
    /* This could eventually get allowed if a compelling use case is found, but
       right now it's not as it makes the implementation of writeJson()
       simpler. Or maybe it would have to be writeJsonKey() or some such
       because why have the distinction in write() vs writeKey() but not in
       writeJson()? */

    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject();

    std::ostringstream out;
    Error redirectError{&out};
    json.writeJson("/* A comment*/ \"key\"");
    CORRADE_COMPARE(out.str(),
        "Utility::JsonWriter::writeJson(): expected an object key or object end\n");
}

void JsonWriterTest::rawJsonInObjectValue() {
    JsonWriter json;
    json.beginObject()
        .writeKey("key")
        .writeJson("/* A comment */ false")
        .endObject();
    CORRADE_COMPARE(json.toString(), "{\"key\":/* A comment */ false}");
}

void JsonWriterTest::rawJsonInArray() {
    JsonWriter json;
    json.beginArray()
        .writeJson("/* A comment */ 6776")
        .endArray();
    CORRADE_COMPARE(json.toString(), "[/* A comment */ 6776]");
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

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!json.toFile(JSONWRITER_TEST_DIR));
    /* There's an error from Path::write() before */
    CORRADE_COMPARE_AS(out.str(),
        "\nUtility::JsonWriter::toFile(): can't write to " JSONWRITER_TEST_DIR "\n",
        TestSuite::Compare::StringHasSuffix);
}

void JsonWriterTest::tooBigIndent() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    JsonWriter{{}, 9};
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter: indentation can be at most 8 characters, got 9\n");
}

void JsonWriterTest::objectEndButNoObject() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.endObject();
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::endObject(): expected a value\n");
}

void JsonWriterTest::arrayEndButNoArray() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.endArray();
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::endArray(): expected a value\n");
}

void JsonWriterTest::arrayEndButObjectEndExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject();

    std::ostringstream out;
    Error redirectError{&out};
    json.endArray();
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::endArray(): expected an object key or object end\n");
}

void JsonWriterTest::objectEndButArrayEndExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginArray();

    std::ostringstream out;
    Error redirectError{&out};
    json.endObject();
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::endObject(): expected an array value or array end\n");
}

void JsonWriterTest::valueButObjectKeyExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject();

    std::ostringstream out;
    Error redirectError{&out};
    json.write("hello");
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::write(): expected an object key or object end\n");
}

void JsonWriterTest::objectKeyButValueExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject()
        .writeKey("hi");

    std::ostringstream out;
    Error redirectError{&out};
    json.writeKey("hello");
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::writeKey(): expected an object value\n");
}

void JsonWriterTest::objectKeyButDocumentEndExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.write("hi");

    std::ostringstream out;
    Error redirectError{&out};
    json.writeKey("hello");
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::writeKey(): expected document end\n");
}

void JsonWriterTest::valueButDocumentEndExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.write("hi");

    std::ostringstream out;
    Error redirectError{&out};
    json.write("hello");
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::write(): expected document end\n");
}

void JsonWriterTest::rawJsonButDocumentEndExpected() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.write("hi");

    std::ostringstream out;
    Error redirectError{&out};
    json.writeJson("/* HI JSON CAN YOU COMMENT */");
    CORRADE_COMPARE(out.str(), "Utility::JsonWriter::writeJson(): expected document end\n");
}

void JsonWriterTest::toStringOrFileNoValue() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out.str(),
        "Utility::JsonWriter::toString(): incomplete JSON, expected a value\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected a value\n");
}

void JsonWriterTest::toStringOrFileIncompleteObject() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject();

    std::ostringstream out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out.str(),
        "Utility::JsonWriter::toString(): incomplete JSON, expected an object key or object end\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an object key or object end\n");
}

void JsonWriterTest::toStringOrFileIncompleteObjectValue() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginObject()
        .writeKey("hi");

    std::ostringstream out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out.str(),
        "Utility::JsonWriter::toString(): incomplete JSON, expected an object value\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an object value\n");
}

void JsonWriterTest::toStringOrFileIncompleteArray() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;
    json.beginArray();

    std::ostringstream out;
    Error redirectError{&out};
    json.toString();
    json.toFile(Path::join(JSONWRITER_TEST_DIR, "file.json"));
    CORRADE_COMPARE(out.str(),
        "Utility::JsonWriter::toString(): incomplete JSON, expected an array value or array end\n"
        "Utility::JsonWriter::toFile(): incomplete JSON, expected an array value or array end\n");
}

void JsonWriterTest::invalidFloat() {
    auto&& data = InvalidFloatDoubleData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.write(data.floatValue);
    CORRADE_COMPARE(out.str(), formatString("Utility::JsonWriter::write(): invalid floating-point value {}\n", data.message));
}

void JsonWriterTest::invalidDouble() {
    auto&& data = InvalidFloatDoubleData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.write(data.doubleValue);
    CORRADE_COMPARE(out.str(), formatString("Utility::JsonWriter::write(): invalid floating-point value {}\n", data.message));
}

void JsonWriterTest::invalidUnsignedLong() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.write(4503599627370496ull);
    CORRADE_COMPARE(out.str(), formatString("Utility::JsonWriter::write(): too large integer value 4503599627370496\n"));
}

void JsonWriterTest::invalidLong() {
    auto&& data = InvalidLongData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    JsonWriter json;

    std::ostringstream out;
    Error redirectError{&out};
    json.write(data.value);
    CORRADE_COMPARE(out.str(), formatString("Utility::JsonWriter::write(): too small or large integer value {}\n", data.message));
}

void JsonWriterTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<JsonWriter>{});
    CORRADE_VERIFY(!std::is_copy_assignable<JsonWriter>{});
}

void JsonWriterTest::constructMove() {
    JsonWriter a;
    a.beginArray();

    JsonWriter b = std::move(a);
    b.write("hey");
    b.endArray();

    JsonWriter c;
    c = std::move(b);
    CORRADE_COMPARE(c.toString(), "[\"hey\"]");

    CORRADE_VERIFY(std::is_nothrow_move_constructible<JsonWriter>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<JsonWriter>::value);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::JsonWriterTest)
