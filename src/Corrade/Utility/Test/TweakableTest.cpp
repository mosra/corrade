/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Tweakable.h"

#include "Corrade/Utility/Implementation/tweakable.h"

#define _ CORRADE_TWEAKABLE

namespace Corrade { namespace Utility { namespace Test {

struct TweakableTest: TestSuite::Tester {
    explicit TweakableTest();

    void findTweakableAlias();
    void findTweakableAliasDefinedEmpty();

    void parseTweakables();
    void parseTweakablesError();

    void benchmarkBase();
    void benchmarkDisabled();
    void benchmarkEnabled();

    void debugState();
};

namespace {
    constexpr struct {
        const char* name;
        const char* data;
        const char* alias;
        const char* expectFail;
    } TweakableAliasData[]{
        {"usual", "// a comment\n#define T3 CORRADE_TWEAKABLE\n", "T3", {}},
        {"right after another define", "// a comment\n#define something foo\n#define T3 CORRADE_TWEAKABLE\n", "T3", {}},
        {"right after an empty define", "// a comment\n#define something\n#define T3 CORRADE_TWEAKABLE\n", "T3", {}},
        {"first line", "#define _ CORRADE_TWEAKABLE\n", "_", {}},
        {"windows newlines", "#define T3 CORRADE_TWEAKABLE\r\n", "T3", {}},
        {"as a last in the file", "#define t3 CORRADE_TWEAKABLE", "t3", {}},
        {"spaces", "#define \t t \tCORRADE_TWEAKABLE   \n", "t", {}},
        {"spaces before", "   #define _ CORRADE_TWEAKABLE\n", "_", {}},
        {"spaces after #", "#  define _ CORRADE_TWEAKABLE\n", "_",
            "Spaces after # not supported."},
        {"junk after", "#define _ CORRADE_TWEAKABLEs", "CORRADE_TWEAKABLE", {}},
        {"junk after #2", "#define _ CORRADE_TWEAKABLE \tabc", "CORRADE_TWEAKABLE", {}},
        {"commented out", "//#define _ CORRADE_TWEAKABLE\n", "CORRADE_TWEAKABLE", {}},
        {"commented out multiline", "/*\n#define _ CORRADE_TWEAKABLE\n*/", "CORRADE_TWEAKABLE",
            "Multi-line comments are not handled properly."}
    };

    constexpr struct {
        const char* name;
        const char* data;
        TweakableState(*parser)(Containers::ArrayView<const char>, Containers::StaticArrayView<Implementation::TweakableStorageSize, char>);
        TweakableState state;
        const char* error;
    } ParseErrorData[]{
        {"unterminated before", "_(    ", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unterminated _( in a.cpp:1\n"},
        {"unterminated after", "_(3   ", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unterminated _(3    in a.cpp:1\n"},
        {"garbage after", "_(3(", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unterminated _(3 in a.cpp:1\n"},
        {"unterminated string", "_( \"", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unterminated string _( \" in a.cpp:1\n"},
        {"unterminated char", "_(\t'", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unterminated char _(\t' in a.cpp:1\n"},
        {"wide char", "_(L' ')", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unsupported wide char/string literal _(L in a.cpp:1\n"},
        {"unicode 1", "_(U' ')", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(U in a.cpp:1\n"},
        {"unicode 2", "_(u\" \")", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(u in a.cpp:1\n"},
        {"unicode 3", "_(u8\" \")", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(u in a.cpp:1\n"},
        {"raw", "_(R\"( )\")", {}, TweakableState::Error,
            "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(R in a.cpp:1\n"},
        {"char escape error", "_('\\o')",
            Implementation::TweakableTraits<char>::parse, TweakableState::Error,
            /** @todo this message will change once escapes *are* implemented */
            "Utility::TweakableParser: escape sequences in char literals are not implemented, sorry\n"
            "Utility::Tweakable::update(): error parsing _('\\o') in a.cpp:1\n"},
        {"different type", "_(42.0f)",
            Implementation::TweakableTraits<long>::parse, TweakableState::Recompile,
            "Utility::TweakableParser: 42.0f has an unexpected suffix, expected l\n"
            "Utility::Tweakable::update(): change of _(42.0f) in a.cpp:1 requested a recompile\n"},
        {"unexpected line number", "\n_(false)",
            Implementation::TweakableTraits<bool>::parse, TweakableState::Recompile,
            "Utility::Tweakable::update(): code changed around _(false) in a.cpp:2, requesting a recompile\n"}
    };
}

TweakableTest::TweakableTest() {
    addInstancedTests({&TweakableTest::findTweakableAlias},
        Containers::arraySize(TweakableAliasData));

    addTests({&TweakableTest::findTweakableAliasDefinedEmpty,
              &TweakableTest::parseTweakables});

    addInstancedTests({&TweakableTest::parseTweakablesError},
        Containers::arraySize(ParseErrorData));

    addBenchmarks({&TweakableTest::benchmarkBase,
                   &TweakableTest::benchmarkDisabled,
                   &TweakableTest::benchmarkEnabled}, 200);

    addTests({&TweakableTest::debugState});
}

void TweakableTest::findTweakableAlias() {
    auto& data = TweakableAliasData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    {
        CORRADE_EXPECT_FAIL_IF(data.expectFail, String::fromArray(data.expectFail));
        CORRADE_COMPARE(Implementation::findTweakableAlias(data.data), data.alias);
    }
}

void TweakableTest::findTweakableAliasDefinedEmpty() {
    /* This doesn't match and so the default is returned. If the preprocessor
       works correctly, there should be no calls to the Tweakable instance and
       so no file gets ever registered, thus nothing gets searched for the CORRADE_TWEAKABLE macro. */
    CORRADE_COMPARE(Implementation::findTweakableAlias("#define CORRADE_TWEAKABLE"), "CORRADE_TWEAKABLE");
}

void TweakableTest::parseTweakables() {
    const std::string data = R"(/* line 1 */

int a = _( 3);
// comment
foo(_(4.0f), _(true));
int b = bar3_()+__() / _( -1.1 ); // lots of false matches

_("some \"thing\"") // doesn't have a parser

unordered_map<>;
return _(    'a' );

_('\'') // also no parser
)";

    auto lambda1 = [](void(*)(), void* out) {
        ++*static_cast<int*>(out);
    };
    auto lambda2 = [](void(*)(), void* out) {
        *static_cast<bool*>(out) = true;
    };

    std::vector<Implementation::TweakableVariable> variables{6};
    variables[0].line = 3;
    variables[0].parser = Implementation::TweakableTraits<int>::parse;
    variables[1].line = 5;
    variables[1].parser = Implementation::TweakableTraits<float>::parse;
    *reinterpret_cast<float*>(variables[1].storage) = 4.0f;
    variables[1].scopeLambda = lambda1;
    variables[2].line = 5;
    variables[2].parser = Implementation::TweakableTraits<bool>::parse;
    variables[3].scopeLambda = lambda2;
    variables[3].line = 6;
    variables[3].parser = Implementation::TweakableTraits<double>::parse;
    variables[3].scopeLambda = lambda2;
    variables[4].line = 8;
    variables[4].parser = nullptr; /* doesn't have a parser */
    variables[5].line = 11;
    variables[5].parser = Implementation::TweakableTraits<char>::parse;

    {
        std::string name = "_";
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables(name, "a.cpp", data, variables, scopes);
        CORRADE_COMPARE(out.str(),
            "Utility::Tweakable::update(): updating _( 3) in a.cpp:3\n"
            "Utility::Tweakable::update(): updating _(true) in a.cpp:5\n"
            "Utility::Tweakable::update(): updating _( -1.1 ) in a.cpp:6\n"
            "Utility::Tweakable::update(): ignoring unknown new value _(\"some \\\"thing\\\"\") in a.cpp:8\n"
            "Utility::Tweakable::update(): updating _(    'a' ) in a.cpp:11\n"
            "Utility::Tweakable::update(): ignoring unknown new value _('\\'') in a.cpp:13\n");
        CORRADE_COMPARE(state, TweakableState::Success);
        CORRADE_COMPARE(scopes.size(), 1);
        CORRADE_COMPARE(std::get<0>(*scopes.begin()), lambda2);
    }
    CORRADE_COMPARE(*reinterpret_cast<int*>(variables[0].storage), 3);
    CORRADE_COMPARE(*reinterpret_cast<float*>(variables[1].storage), 4.0f);
    CORRADE_COMPARE(*reinterpret_cast<bool*>(variables[2].storage), true);
    CORRADE_COMPARE(*reinterpret_cast<double*>(variables[3].storage), -1.1);
    CORRADE_COMPARE(*reinterpret_cast<char*>(variables[5].storage), 'a');

    /* Second pass should report no change */
    {
        std::string name = "_";
        std::ostringstream out;
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        TweakableState state = Implementation::parseTweakables(name, "a.cpp", data, variables, scopes);
        CORRADE_COMPARE(out.str(),
            "Utility::Tweakable::update(): ignoring unknown new value _(\"some \\\"thing\\\"\") in a.cpp:8\n"
            "Utility::Tweakable::update(): ignoring unknown new value _('\\'') in a.cpp:13\n");
        CORRADE_COMPARE(state, TweakableState::NoChange);
    }
    CORRADE_COMPARE(*reinterpret_cast<int*>(variables[0].storage), 3);
    CORRADE_COMPARE(*reinterpret_cast<float*>(variables[1].storage), 4.0f);
    CORRADE_COMPARE(*reinterpret_cast<bool*>(variables[2].storage), true);
    CORRADE_COMPARE(*reinterpret_cast<double*>(variables[3].storage), -1.1);
    CORRADE_COMPARE(*reinterpret_cast<char*>(variables[5].storage), 'a');
}

void TweakableTest::parseTweakablesError() {
    auto& data = ParseErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;

    std::vector<Implementation::TweakableVariable> variables{1};
    variables[0].line = 1;
    variables[0].parser = data.parser;

    {
        std::string name = "_";
        std::ostringstream out;
        Warning redirectWarning{&out};
        Error redirectError{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables(name, "a.cpp", data.data, variables, scopes);
        CORRADE_COMPARE(out.str(), data.error);
        CORRADE_COMPARE(state, data.state);
    }
}

void TweakableTest::benchmarkBase() {
    float dt = 1/60.0f;
    float velocity = 0.0f;
    struct {
        float x{}, y{};
    } position;

    CORRADE_BENCHMARK(120) {
        velocity += 9.81f*dt;
        position.x += 2.2f*dt;
        position.y += velocity*dt;
    }

    CORRADE_COMPARE(position.x, 4.4f);
    CORRADE_COMPARE(position.y, 19.7835f);
}

void TweakableTest::benchmarkDisabled() {
    Tweakable tweakable;

    float dt = 1/60.0f;
    float velocity = 0.0f;
    struct {
        float x{}, y{};
    } position;

    CORRADE_BENCHMARK(120) {
        velocity += _(9.81f)*dt;
        position.x += _(2.2f)*dt;
        position.y += velocity*dt;
    }

    CORRADE_COMPARE(position.x, 4.4f);
    CORRADE_COMPARE(position.y, 19.7835f);
}

void TweakableTest::benchmarkEnabled() {
    Tweakable tweakable;
    tweakable.enable();

    float dt = 1/60.0f;
    float velocity;
    {
        /* Disable the watch message */
        Debug redirectOutput{nullptr};
        Error redirectError{nullptr};
        velocity = _(0.0f);
    }
    struct {
        float x{}, y{};
    } position;

    CORRADE_BENCHMARK(120) {
        velocity += _(9.81f)*dt;
        position.x += _(2.2f)*dt;
        position.y += velocity*dt;
    }

    CORRADE_COMPARE(position.x, 4.4f);
    CORRADE_COMPARE(position.y, 19.7835f);
}

void TweakableTest::debugState() {
    std::ostringstream out;
    Debug{&out} << TweakableState::NoChange << TweakableState(0xde);
    CORRADE_COMPARE(out.str(), "Utility::TweakableState::NoChange Utility::TweakableState(0xde)\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TweakableTest)
