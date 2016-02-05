/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility { namespace Test {

struct DebugTest: TestSuite::Tester {
    explicit DebugTest();

    void debug();
    void boolean();
    void chars();
    void unicode();
    void custom();
    void nospace();
    void newline();
    void noNewlineAtTheEnd();
    void colors();
    void colorsNospace();

    void iterable();
    void tuple();

    void ostreamFallback();
    void ostreamFallbackPriority();

    void scopedOutput();
};

DebugTest::DebugTest() {
    addTests({&DebugTest::debug,
              &DebugTest::boolean,
              &DebugTest::chars,
              &DebugTest::unicode,
              &DebugTest::custom,
              &DebugTest::nospace,
              &DebugTest::newline,
              &DebugTest::noNewlineAtTheEnd,
              &DebugTest::colors,
              &DebugTest::colorsNospace,

              &DebugTest::iterable,
              &DebugTest::tuple,

              &DebugTest::ostreamFallback,
              &DebugTest::ostreamFallbackPriority,

              &DebugTest::scopedOutput});
}

void DebugTest::debug() {
    std::ostringstream debug, warning, error;

    Debug(&debug) << "a" << 33 << 0.567f;
    Warning(&warning) << "w" << 42 << "meh";
    Error(&error) << "e";

    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");
    CORRADE_COMPARE(warning.str(), "w 42 meh\n");
    CORRADE_COMPARE(error.str(), "e\n");

    /* Multiple times used instance */
    debug.str("");
    {
        Debug d{&debug};
        d << "a";
        d << 33;
        d << 0.567f;
    }
    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");

    /* Don't add newline at the end of empty output */
    debug.str("");
    Debug{&debug};
    CORRADE_COMPARE(debug.str(), "");
}

void DebugTest::boolean() {
    std::ostringstream o;
    Debug(&o) << true << false;
    CORRADE_COMPARE(o.str(), "true false\n");
}

void DebugTest::chars() {
    std::ostringstream o;
    Debug(&o) << 'a';
    CORRADE_COMPARE(o.str(), "97\n");
}

void DebugTest::unicode() {
    /* Four-character hex values */
    std::ostringstream o;
    Debug(&o) << U'a';
    CORRADE_COMPARE(o.str(), "U+0061\n");

    /* Longer hex values */
    o.str({});
    Debug(&o) << U'\xBEEF3';
    CORRADE_COMPARE(o.str(), "U+BEEF3\n");

    /* UTF-32 string */
    o.str({});
    Debug(&o) << U"abc";
    CORRADE_COMPARE(o.str(), "{U+0061, U+0062, U+0063}\n");
}

namespace {

struct Foo {
    int value;
};

Debug& operator<<(Debug& debug, const Foo& value) {
    return debug << value.value;
}

}

void DebugTest::custom() {
    std::ostringstream out;

    Foo f = { 42 };
    {
        Debug(&out) << "The answer is" << f;
        Debug(&out) << f << "is the answer";
    }
    CORRADE_COMPARE(out.str(), "The answer is 42\n"
                               "42 is the answer\n");
}

void DebugTest::nospace() {
    std::ostringstream out;
    Debug(&out) << "Value:" << 16 << Debug::nospace << "," << 24;

    CORRADE_COMPARE(out.str(), "Value: 16, 24\n");
}

void DebugTest::newline() {
    std::ostringstream out;
    Debug(&out) << "Value:" << Debug::newline << 16;

    CORRADE_COMPARE(out.str(), "Value:\n16\n");
}

void DebugTest::noNewlineAtTheEnd() {
    std::ostringstream out1, out2, out3;

    Debug(&out1) << "Ahoy";
    Debug{&out1, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    Warning(&out2) << "Ahoy";
    Warning{&out2, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    Error(&out3) << "Ahoy";
    Error{&out3, Debug::Flag::NoNewlineAtTheEnd} << "Hello";

    CORRADE_COMPARE(out1.str(), "Ahoy\nHello");
    CORRADE_COMPARE(out2.str(), "Ahoy\nHello");
    CORRADE_COMPARE(out3.str(), "Ahoy\nHello");
}

void DebugTest::colors() {
    {
        /* Auto-reset at the end */
        std::ostringstream out;
        Debug{&out} << "Hello" << Debug::color(Debug::Color::Green) << "world";
        CORRADE_COMPARE(out.str(), "Hello\033[0;32m world\033[0m\n");
    } {
        /* Don't reset twice */
        std::ostringstream out;
        Debug{&out} << Debug::boldColor(Debug::Color::Red) << "Hello"
            << Debug::resetColor << "world";
        CORRADE_COMPARE(out.str(), "\033[1;31mHello\033[0m world\n");
    } {
        /* Disabled globally */
        std::ostringstream out;
        Debug{&out, Debug::Flag::DisableColors}
            << Debug::boldColor(Debug::Color::Default) << "Hello"
            << Debug::color(Debug::Color::Cyan) << "world"
            << Debug::resetColor;
        CORRADE_COMPARE(out.str(), "Hello world\n");
    }
}

void DebugTest::colorsNospace() {
    std::ostringstream out1, out2;

    /* Order of nospace and color modifiers shouldn't matter and give the same
       output */
    Debug{&out1} << "H"
        << Debug::color(Debug::Color::Blue) << Debug::nospace << "e"
        << Debug::boldColor(Debug::Color::Yellow) << Debug::nospace << "ll"
        << Debug::resetColor << Debug::nospace << "o";
    Debug{&out2} << "H"
        << Debug::nospace << Debug::color(Debug::Color::Blue) << "e"
        << Debug::nospace << Debug::boldColor(Debug::Color::Yellow) << "ll"
        << Debug::nospace << Debug::resetColor << "o";

    CORRADE_COMPARE(out1.str(), "H\033[0;34me\033[1;33mll\033[0mo\n");
    CORRADE_COMPARE(out2.str(), "H\033[0;34me\033[1;33mll\033[0mo\n");
}

void DebugTest::iterable() {
    std::ostringstream out;
    Debug(&out) << std::vector<int>{1, 2, 3};
    CORRADE_COMPARE(out.str(), "{1, 2, 3}\n");

    out.str({});
    Debug(&out) << std::set<std::string>{"a", "b", "c"};
    CORRADE_COMPARE(out.str(), "{a, b, c}\n");

    out.str({});
    Debug(&out) << std::map<int, std::string>{{1, "a"}, {2, "b"}, {3, "c"}};
    CORRADE_COMPARE(out.str(), "{(1, a), (2, b), (3, c)}\n");
}

void DebugTest::tuple() {
    std::ostringstream out;

    Debug(&out) << std::make_tuple();
    CORRADE_COMPARE(out.str(), "()\n");

    out.str({});
    Debug(&out) << std::make_tuple(3, 4.56, std::string{"hello"});
    CORRADE_COMPARE(out.str(), "(3, 4.56, hello)\n");
}

namespace {

struct Bar {};
struct Baz {};

inline std::ostream& operator<<(std::ostream& o, const Bar&) {
    return o << "bar";
}

inline std::ostream& operator<<(std::ostream& o, const Baz&) {
    return o << "baz from ostream";
}

inline Debug& operator<<(Debug& debug, const Baz&) {
    return debug << "baz from Debug";
}

}

void DebugTest::ostreamFallback() {
    std::ostringstream out;
    Debug(&out) << Bar{};
    CORRADE_COMPARE(out.str(), "bar\n");
}

void DebugTest::ostreamFallbackPriority() {
    /* Suppress warning about unused function operator<<(std::ostream&, const Baz&) */
    {
        std::ostringstream o;
        o << Baz{};
    }

    std::ostringstream out;
    Debug(&out) << Baz{};
    CORRADE_COMPARE(out.str(), "baz from Debug\n");
}

void DebugTest::scopedOutput() {
    std::ostringstream debug1, debug2, warning1, warning2, error1, error2;

    Debug muteD{nullptr};
    Warning muteW{nullptr};
    Error muteE{nullptr};

    {
        Debug redirectD1{&debug1};
        Warning redirectW1{&warning1};
        Error redirectE1{&error1};

        Debug() << "hello";
        Warning() << "crazy";
        Error() << "world";

        {
            Debug redirectD2{&debug2};
            Warning redirectW2{&warning2};
            Error redirectE2{&error2};

            Debug() << "well";
            Warning() << "that";
            Error() << "smells";
        }

        Debug() << "how";
        Warning() << "are";
        Error() << "you?";
    }

    Debug() << "anyone";
    Warning() << "hears";
    Error() << "me?";

    CORRADE_COMPARE(debug1.str(), "hello\nhow\n");
    CORRADE_COMPARE(warning1.str(), "crazy\nare\n");
    CORRADE_COMPARE(error1.str(), "world\nyou?\n");

    CORRADE_COMPARE(debug2.str(), "well\n");
    CORRADE_COMPARE(warning2.str(), "that\n");
    CORRADE_COMPARE(error2.str(), "smells\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DebugTest)
