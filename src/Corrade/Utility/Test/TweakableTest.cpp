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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Format.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/String.h"
#include "Corrade/Utility/Tweakable.h"

#include "Corrade/Utility/Implementation/tweakable.h"

#define _ CORRADE_TWEAKABLE

namespace Corrade { namespace Utility { namespace Test { namespace {

struct TweakableTest: TestSuite::Tester {
    explicit TweakableTest();

    void constructCopy();
    void constructMove();

    void findTweakableAlias();
    void findTweakableAliasDefinedEmpty();

    void parseTweakables();
    void parseTweakablesError();

    void parseSpecials();
    void parseSpecialsError();

    void benchmarkBase();
    void benchmarkDisabled();
    void benchmarkEnabled();

    void debugState();
};

constexpr struct {
    const char* name;
    const char* data;
    const char* alias;
    const char* expectFail;
} TweakableAliasData[]{
    {"usual", "// a comment\n#define T3 CORRADE_TWEAKABLE\n", "T3", nullptr},
    {"right after another define", "// a comment\n#define something foo\n#define T3 CORRADE_TWEAKABLE\n", "T3", nullptr},
    {"right after an empty define", "// a comment\n#define something\n#define T3 CORRADE_TWEAKABLE\n", "T3", nullptr},
    {"first line", "#define _ CORRADE_TWEAKABLE\n", "_", nullptr},
    {"windows newlines", "#define T3 CORRADE_TWEAKABLE\r\n", "T3", nullptr},
    {"as a last in the file", "#define t3 CORRADE_TWEAKABLE", "t3", nullptr},
    {"spaces", "#define \t t \tCORRADE_TWEAKABLE   \n", "t", nullptr},
    {"spaces before", "   #define _ CORRADE_TWEAKABLE\n", "_", nullptr},
    {"spaces after #", "#  define _ CORRADE_TWEAKABLE\n", "_",
        "Spaces after # not supported."},
    {"junk after", "#define _ CORRADE_TWEAKABLEs", "CORRADE_TWEAKABLE", nullptr},
    {"junk after #2", "#define _ CORRADE_TWEAKABLE \tabc", "CORRADE_TWEAKABLE", nullptr},
    {"commented out", "//#define _ CORRADE_TWEAKABLE\n", "CORRADE_TWEAKABLE", nullptr},
    {"commented out multiline", "/*\n#define _ CORRADE_TWEAKABLE\n*/", "CORRADE_TWEAKABLE",
        "Multi-line comments are not handled properly."}
};

constexpr struct {
    const char* name;
    const char* data;
    TweakableState(*parser)(Containers::StringView, Containers::StaticArrayView<Implementation::TweakableStorageSize, char>);
    TweakableState state;
    const char* error;
} ParseErrorData[]{
    {"unterminated before", "_(    ", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unterminated _( in a.cpp:1\n"},
    {"unterminated after", "_(3   ", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unterminated _(3    in a.cpp:1\n"},
    {"garbage after", "_(3(", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unterminated _(3 in a.cpp:1\n"},
    {"unterminated string", "_( \"", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unterminated string _( \" in a.cpp:1\n"},
    {"unterminated char", "_(\t'", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unterminated char _(\t' in a.cpp:1\n"},
    {"wide char", "_(L' ')", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unsupported wide char/string literal _(L in a.cpp:1\n"},
    {"unicode 1", "_(U' ')", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(U in a.cpp:1\n"},
    {"unicode 2", "_(u\" \")", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(u in a.cpp:1\n"},
    {"unicode 3", "_(u8\" \")", nullptr, TweakableState::Error,
        "Utility::Tweakable::update(): unsupported unicode/raw char/string literal _(u in a.cpp:1\n"},
    {"raw", "_(R\"( )\")", nullptr, TweakableState::Error,
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

constexpr struct {
    const char* name;
    const char* data;
    int line;
} ParseSpecialsData[]{
    {"tweakable in a line comment", "// TW(42)\nTW(1337)", 2},
    {"tweakable in a block comment", R"CPP(/*
   this is
   a TW(42)
   comment */
TW(1337)
)CPP", 5},
    {"tweakable in a nested block comment", R"CPP(/* this is
a /* nested comment */
which TW(1337)
// should work */
)CPP", 3},
    {"tweakable in a 4-char", "'TW()' TW(1337)", 1},
    {"tweakable in a string", "\"TW(42)\" TW(1337)", 1},
    {"tweakable in a string with escapes", "\"hello \\\"TW(42)\\\" there\"\nTW(1337)", 2},
    {"tweakable in a raw string with no delimiter", R"CPP(R"(TW(42))"
TW(1337)
)CPP", 2},
    {"tweakable in a raw string with a delimiter", R"CPP(R"string(TW(42))string"
TW(1337)
)CPP", 2},
    {"tweakable in a raw string with a 16-char delimiter", R"CPP(R"0123456789abcdef(TW(42))0123456789abcdef"
    TW(1337)
    )CPP", 2},
    {"tweakable in a nested raw string", R"CPP(R"outer(R"inner(TW(42))inner")outer"
    TW(1337)
    )CPP", 2},
    {"tweakable with the same initial char", R"CPP(namespace Tw {
TW(1337)
})CPP", 2},
};

constexpr struct {
    const char* name;
    const char* data;
    const char* error;
} ParseSpecialsErrorData[]{
    {"unterminated block comment", "/* you know, this\n  is all very\nnice but",
        "Utility::Tweakable::update(): unterminated block comment in a.cpp:3\n"},
    {"unterminated char", "\n'a",
        "Utility::Tweakable::update(): unterminated character literal in a.cpp:2\n"},
    {"multiline char", "\n\'\n",
        "Utility::Tweakable::update(): unterminated character literal in a.cpp:2\n"},
    {"unterminated string", "\n\n\"oh but i wanted to sa",
        "Utility::Tweakable::update(): unterminated string literal in a.cpp:3\n"},
    {"multiline non-raw string", "\n\"oh but\nthis is a newline\"",
        "Utility::Tweakable::update(): unterminated string literal in a.cpp:2\n"},
    {"unterminated raw string delimiter", "\n\nR\"\nbut",
        "Utility::Tweakable::update(): unterminated raw string delimiter in a.cpp:3\n"},
    {"too long raw string delimiter", "\n\nR\"0123456789abcdefg(haha)0123456789abcdefg\"",
        "Utility::Tweakable::update(): unterminated raw string delimiter in a.cpp:3\n"},
    {"unterminated raw string", "R\"boo(and this goes until \nthe EOF\n)boo \"",
        "Utility::Tweakable::update(): unterminated raw string literal in a.cpp:3\n"}
};

TweakableTest::TweakableTest() {
    addTests({&TweakableTest::constructCopy,
              &TweakableTest::constructMove});

    addInstancedTests({&TweakableTest::findTweakableAlias},
        Containers::arraySize(TweakableAliasData));

    addTests({&TweakableTest::findTweakableAliasDefinedEmpty,
              &TweakableTest::parseTweakables});

    addInstancedTests({&TweakableTest::parseTweakablesError},
        Containers::arraySize(ParseErrorData));

    addInstancedTests({&TweakableTest::parseSpecials},
        Containers::arraySize(ParseSpecialsData));

    addInstancedTests({&TweakableTest::parseSpecialsError},
        Containers::arraySize(ParseSpecialsErrorData));

    addBenchmarks({&TweakableTest::benchmarkBase,
                   &TweakableTest::benchmarkDisabled,
                   &TweakableTest::benchmarkEnabled}, 200);

    addTests({&TweakableTest::debugState});
}

void TweakableTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<Tweakable>{});
    CORRADE_VERIFY(!std::is_copy_assignable<Tweakable>{});
}

void TweakableTest::constructMove() {
    /* For a move we would need some NoCreate state and the destructor not
       checking for globalInstance == this */
    CORRADE_VERIFY(!std::is_move_constructible<Tweakable>{});
    CORRADE_VERIFY(!std::is_move_assignable<Tweakable>{});
}

void TweakableTest::findTweakableAlias() {
    auto&& data = TweakableAliasData[testCaseInstanceId()];
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
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables("_", "a.cpp", data, variables, scopes);
        CORRADE_COMPARE(out.str(),
            "Utility::Tweakable::update(): updating _( 3) in a.cpp:3\n"
            "Utility::Tweakable::update(): updating _(true) in a.cpp:5\n"
            "Utility::Tweakable::update(): updating _( -1.1 ) in a.cpp:6\n"
            "Utility::Tweakable::update(): ignoring unknown new value _(\"some \\\"thing\\\"\") in a.cpp:8\n"
            "Utility::Tweakable::update(): updating _(    'a' ) in a.cpp:11\n"
            "Utility::Tweakable::update(): ignoring unknown new value _('\\'') in a.cpp:13\n");
        CORRADE_COMPARE(state, TweakableState::Success);
        CORRADE_COMPARE(scopes.size(), 1);
        CORRADE_VERIFY(std::get<0>(*scopes.begin()) == lambda2);
    }
    CORRADE_COMPARE(*reinterpret_cast<int*>(variables[0].storage), 3);
    CORRADE_COMPARE(*reinterpret_cast<float*>(variables[1].storage), 4.0f);
    CORRADE_COMPARE(*reinterpret_cast<bool*>(variables[2].storage), true);
    CORRADE_COMPARE(*reinterpret_cast<double*>(variables[3].storage), -1.1);
    CORRADE_COMPARE(*reinterpret_cast<char*>(variables[5].storage), 'a');

    /* Second pass should report no change */
    {
        std::ostringstream out;
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        TweakableState state = Implementation::parseTweakables("_", "a.cpp", data, variables, scopes);
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
    auto&& data = ParseErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::vector<Implementation::TweakableVariable> variables{1};
    variables[0].line = 1;
    variables[0].parser = data.parser;

    {
        std::ostringstream out;
        Warning redirectWarning{&out};
        Error redirectError{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables("_", "a.cpp", data.data, variables, scopes);
        CORRADE_COMPARE(out.str(), data.error);
        CORRADE_COMPARE(state, data.state);
    }
}

void TweakableTest::parseSpecials() {
    auto&& data = ParseSpecialsData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::vector<Implementation::TweakableVariable> variables{2};
    variables[0].line = data.line;
    variables[0].parser = Implementation::TweakableTraits<int>::parse;
    variables[1].line = 100;
    variables[1].parser = nullptr;

    {
        std::ostringstream out;
        Debug redirectOutput{&out};
        Warning redirectWarning{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables("TW", "a.cpp", data.data, variables, scopes);
        CORRADE_COMPARE(out.str(), formatString(
            "Utility::Tweakable::update(): updating TW(1337) in a.cpp:{}\n", data.line));
        CORRADE_COMPARE(state, TweakableState::Success);
        CORRADE_COMPARE(scopes.size(), 0);
    }
    CORRADE_COMPARE(*reinterpret_cast<int*>(variables[0].storage), 1337);
}

void TweakableTest::parseSpecialsError() {
    auto&& data = ParseSpecialsErrorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::vector<Implementation::TweakableVariable> variables{1};
    variables[0].line = 1;
    variables[0].parser = Implementation::TweakableTraits<int>::parse;

    {
        std::ostringstream out;
        Warning redirectWarning{&out};
        Error redirectError{&out};
        std::set<std::tuple<void(*)(void(*)(), void*), void(*)(), void*>> scopes;
        TweakableState state = Implementation::parseTweakables("_", "a.cpp", data.data, variables, scopes);
        CORRADE_COMPARE(out.str(), data.error);
        CORRADE_COMPARE(state, TweakableState::Error);
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

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::TweakableTest)
