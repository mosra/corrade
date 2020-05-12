/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Tweakable.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TweakableParserTest: TestSuite::Tester {
    explicit TweakableParserTest();

    template<class T> void integral();
    template<class T> void integralUppercase();
    template<class T> void integralError();
    template<class T> void floatingPoint();
    template<class T> void floatingPointUppercase();
    template<class T> void floatingPointError();

    void character();
    void characterError();

    void boolean();
    void booleanError();
};

template<class> struct TypeTraits;
template<> struct TypeTraits<int> {
    static const char* name() { return "int"; }
    static const char* suffix() { return ""; }
};
template<> struct TypeTraits<unsigned int> {
    static const char* name() { return "unsigned int"; }
    static const char* suffix() { return "u"; }
};
template<> struct TypeTraits<long> {
    static const char* name() { return "long"; }
    static const char* suffix() { return "l"; }
};
template<> struct TypeTraits<unsigned long> {
    static const char* name() { return "unsigned long"; }
    static const char* suffix() { return "ul"; }
};
template<> struct TypeTraits<long long> {
    static const char* name() { return "long long"; }
    static const char* suffix() { return "ll"; }
};
template<> struct TypeTraits<unsigned long long> {
    static const char* name() { return "unsigned long long"; }
    static const char* suffix() { return "ull"; }
};
template<> struct TypeTraits<float> {
    static const char* name() { return "float"; }
    static const char* suffix() { return "f"; }
};
template<> struct TypeTraits<double> {
    static const char* name() { return "double"; }
    static const char* suffix() { return ""; }
};
#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct TypeTraits<long double> {
    static const char* name() { return "long double"; }
    static const char* suffix() { return "l"; }
};
#endif

constexpr struct {
    const char* name;
    const char* data;
    int result;
} IntegralData[] {
    {"dec", "42", 42},
    {"hex", "0x2a", 42},
    {"oct", "052", 42},
    {"bin", "0b101010", 42},
    {"positive", "+42", 42},
    {"negative", "-42", -42} /* not for unsigned */
};

enum: std::size_t { IntegralDataUnsignedCount = Containers::arraySize(IntegralData) - 1 };

constexpr struct {
    const char* name;
    const char* data;
    TweakableState state;
    const char* error;
} IntegralErrorData[] {
    {"empty", "", TweakableState::Recompile,
        "Utility::TweakableParser:  is not an integer literal\n"},
    {"char", "'a'", TweakableState::Recompile,
        "Utility::TweakableParser: 'a' is not an integer literal\n"},
    {"garbage after", "42.{}", TweakableState::Recompile,
        "Utility::TweakableParser: unexpected characters .{} after an integer literal\n"},
    {"different suffix", "0x2af", TweakableState::Recompile, /* not for int */
        "Utility::TweakableParser: 0x2af has an unexpected suffix, expected {}\n"}
};

enum: std::size_t { IntegralErrorDataNoSuffixCount = Containers::arraySize(IntegralErrorData) - 1 };

constexpr struct {
    const char* name;
    const char* data;
    float result;
} FloatingPointData[] {
    {"fixed", "35.0", 35.0f},
    {"no zero before", ".5", 0.5f},
    {"no zero after", "35.", 35.0f},
    {"exponential positive", "3.5e+1", 35.0f},
    {"exponential negative", "350.0e-1", 35.0f},
    {"positive", "+35.0", 35.0f},
    {"negative", "-35.0", -35.0f}
};

constexpr struct {
    const char* name;
    const char* data;
    TweakableState state;
    const char* error;
} FloatingPointErrorData[] {
    {"empty", "", TweakableState::Recompile,
        "Utility::TweakableParser:  is not a floating-point literal\n"},
    {"integral", "42{}", TweakableState::Recompile,
        "Utility::TweakableParser: 42{} is not a floating-point literal\n"},
    {"garbage after", "42.b{}", TweakableState::Recompile,
        "Utility::TweakableParser: unexpected characters b{} after a floating-point literal\n"},
    {"different suffix", "42.0u", TweakableState::Recompile, /* not for double */
        "Utility::TweakableParser: 42.0u has an unexpected suffix, expected {}\n"}
};

enum: std::size_t { FloatingPointErrorDataNoSuffixCount = Containers::arraySize(FloatingPointErrorData) - 1 };

constexpr struct {
    const char* name;
    const char* data;
    char result;
    bool expectFail;
} CharacterData[] {
    {"ascii", "'a'", 'a', false},
    {"escaped '", "'\\\''", '\'', true}
};

constexpr struct {
    const char* name;
    const char* data;
    TweakableState state;
    const char* error;
} CharacterErrorData[] {
    {"empty", "", TweakableState::Recompile,
        "Utility::TweakableParser:  is not a character literal\n"},
    {"garbage after", "'a'_foo", TweakableState::Recompile,
        "Utility::TweakableParser: 'a'_foo is not a character literal\n"},
    {"integer", "42",  TweakableState::Recompile,
        "Utility::TweakableParser: 42 is not a character literal\n"}
    /* not testing unterminated, as that is handled in Tweakable already */
};

constexpr struct {
    const char* name;
    const char* data;
    bool result;
} BooleanData[] {
    {"true", "true", true},
    {"false", "false", false}
};

constexpr struct {
    const char* name;
    const char* data;
    TweakableState state;
    const char* error;
} BooleanErrorData[] {
    {"empty", "", TweakableState::Recompile,
        "Utility::TweakableParser:  is not a boolean literal\n"},
    {"garbage after", "true_foo", TweakableState::Recompile,
        "Utility::TweakableParser: true_foo is not a boolean literal\n"}
};

TweakableParserTest::TweakableParserTest() {
    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::integral<int>,
        &TweakableParserTest::integral<long>,
        &TweakableParserTest::integral<long long>,
        &TweakableParserTest::integralUppercase<int>,
        &TweakableParserTest::integralUppercase<long>,
        &TweakableParserTest::integralUppercase<long long>},
        Containers::arraySize(IntegralData));

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::integralError<int>},
        IntegralErrorDataNoSuffixCount);

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::integralError<long>,
        &TweakableParserTest::integralError<long long>},
        Containers::arraySize(IntegralErrorData));

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::integral<unsigned int>,
        &TweakableParserTest::integral<unsigned long>,
        &TweakableParserTest::integral<unsigned long long>,
        &TweakableParserTest::integralUppercase<unsigned int>,
        &TweakableParserTest::integralUppercase<unsigned long>,
        &TweakableParserTest::integralUppercase<unsigned long long>},
        IntegralDataUnsignedCount);

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::integralError<unsigned int>,
        &TweakableParserTest::integralError<unsigned long>,
        &TweakableParserTest::integralError<unsigned long long>},
        Containers::arraySize(IntegralErrorData));

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::floatingPoint<float>,
        &TweakableParserTest::floatingPoint<double>,
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        &TweakableParserTest::floatingPoint<long double>,
        #endif
        &TweakableParserTest::floatingPointUppercase<float>,
        &TweakableParserTest::floatingPointUppercase<double>,
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        &TweakableParserTest::floatingPointUppercase<long double>
        #endif
        }, Containers::arraySize(FloatingPointData));

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::floatingPointError<float>},
        Containers::arraySize(FloatingPointErrorData));

    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::floatingPointError<double>},
        FloatingPointErrorDataNoSuffixCount);

    #ifndef CORRADE_TARGET_EMSCRIPTEN
    addInstancedTests<TweakableParserTest>({
        &TweakableParserTest::floatingPointError<long double>},
        Containers::arraySize(FloatingPointErrorData));
    #endif

    addInstancedTests({&TweakableParserTest::character},
        Containers::arraySize(CharacterData));

    addInstancedTests({&TweakableParserTest::characterError},
        Containers::arraySize(CharacterErrorData));

    addInstancedTests({&TweakableParserTest::boolean},
        Containers::arraySize(BooleanData));

    addInstancedTests({&TweakableParserTest::booleanError},
        Containers::arraySize(BooleanErrorData));
}

template<class T> void TweakableParserTest::integral() {
    auto&& data = IntegralData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = formatString("{}{}", data.data, TypeTraits<T>::suffix());
    CORRADE_COMPARE(TweakableParser<T>::parse({input.data(), input.size()}), std::make_pair(TweakableState::Success, T(data.result)));
}

template<class T> void TweakableParserTest::integralUppercase() {
    auto&& data = IntegralData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = String::uppercase(formatString("{}{}", data.data, TypeTraits<T>::suffix()));
    CORRADE_COMPARE(TweakableParser<T>::parse({input.data(), input.size()}), std::make_pair(TweakableState::Success, T(data.result)));
}

template<class T> void TweakableParserTest::integralError() {
    auto&& data = IntegralErrorData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = Utility::formatString(data.data, TypeTraits<T>::suffix());

    std::ostringstream out;
    Warning redirectWarning{&out};
    Error redirectError{&out};
    TweakableState state = TweakableParser<T>::parse({input.data(), input.size()}).first;
    CORRADE_COMPARE(out.str(), formatString(data.error, TypeTraits<T>::suffix()));
    CORRADE_COMPARE(state, data.state);
}

template<class T> void TweakableParserTest::floatingPoint() {
    auto&& data = FloatingPointData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = formatString("{}{}", data.data, TypeTraits<T>::suffix());
    std::pair<TweakableState, T> parsed = TweakableParser<T>::parse({input.data(), input.size()});
    CORRADE_COMPARE(parsed.first, TweakableState::Success);
    CORRADE_COMPARE(parsed.second, T(data.result));
}

template<class T> void TweakableParserTest::floatingPointUppercase() {
    auto&& data = FloatingPointData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = String::uppercase(formatString("{}{}", data.data, TypeTraits<T>::suffix()));
    std::pair<TweakableState, T> parsed = TweakableParser<T>::parse({input.data(), input.size()});
    CORRADE_COMPARE(parsed.first, TweakableState::Success);
    CORRADE_COMPARE(parsed.second, T(data.result));
}

template<class T> void TweakableParserTest::floatingPointError() {
    auto&& data = FloatingPointErrorData[testCaseInstanceId()];
    setTestCaseTemplateName(TypeTraits<T>::name());
    setTestCaseDescription(data.name);
    std::string input = Utility::formatString(data.data, TypeTraits<T>::suffix());

    std::ostringstream out;
    Warning redirectWarning{&out};
    Error redirectError{&out};
    TweakableState state = TweakableParser<T>::parse({input.data(), input.size()}).first;
    CORRADE_COMPARE(out.str(), formatString(data.error, TypeTraits<T>::suffix()));
    CORRADE_COMPARE(state, data.state);
}

void TweakableParserTest::character() {
    auto&& data = CharacterData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    std::string input = data.data; /* lazy way to get a length */

    auto parsed = TweakableParser<char>::parse({input.data(), input.size()});
    {
        CORRADE_EXPECT_FAIL_IF(data.expectFail, "Not yet implemented.");
        CORRADE_COMPARE(parsed, std::make_pair(TweakableState::Success, data.result));
    }
}

void TweakableParserTest::characterError() {
    auto&& data = CharacterErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    std::string input = data.data; /* lazy way to get a length */

    std::ostringstream out;
    Warning redirectWarning{&out};
    Error redirectError{&out};
    TweakableState state = TweakableParser<char>::parse({input.data(), input.size()}).first;
    CORRADE_COMPARE(out.str(), data.error);
    CORRADE_COMPARE(state, data.state);
}

void TweakableParserTest::boolean() {
    auto&& data = BooleanData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    std::string input = data.data; /* lazy way to get a length */
    CORRADE_COMPARE(TweakableParser<bool>::parse({input.data(), input.size()}), std::make_pair(TweakableState::Success, data.result));
}

void TweakableParserTest::booleanError() {
    auto&& data = BooleanErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    std::string input = data.data; /* lazy way to get a length */

    std::ostringstream out;
    Warning redirectWarning{&out};
    Error redirectError{&out};
    TweakableState state = TweakableParser<bool>::parse({input.data(), input.size()}).first;
    CORRADE_COMPARE(out.str(), data.error);
    CORRADE_COMPARE(state, data.state);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TweakableParserTest)
