/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility { namespace Test {

class DebugTest: public TestSuite::Tester {
    public:
        DebugTest();

        void debug();
        void boolean();
        void chars();
        void unicode();
        void custom();
        void ostream_fallback();
        void prefer_not_to_use_fallback();
        void flags();

        void iterable();
};

DebugTest::DebugTest() {
    addTests({&DebugTest::debug,
              &DebugTest::boolean,
              &DebugTest::chars,
              &DebugTest::unicode,
              &DebugTest::custom,
              &DebugTest::flags,
              &DebugTest::iterable,
              &DebugTest::ostream_fallback,
              &DebugTest::prefer_not_to_use_fallback});
}

void DebugTest::debug() {
    std::ostringstream debug, warning, error;

    Debug::setOutput(&debug);
    Warning::setOutput(&warning);
    Error::setOutput(&error);
    Debug() << "a" << 33 << 0.567f;
    Warning() << "w" << 42 << "meh";
    Error() << "e";

    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");
    CORRADE_COMPARE(warning.str(), "w 42 meh\n");
    CORRADE_COMPARE(error.str(), "e\n");

    /* Multiple times used instance */
    debug.str("");
    {
        Debug d;
        d << "a";
        d << 33;
        d << 0.567f;
    }
    CORRADE_COMPARE(debug.str(), "a 33 0.567\n");

    /* Don't add newline at the end of empty output */
    debug.str("");
    Debug();
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

Debug operator<<(Debug debug, const Foo& value) {
    return debug << value.value;
}

}

void DebugTest::custom() {
    std::ostringstream out;
    Debug::setOutput(&out);

    Foo f = { 42 };
    {
        Debug() << "The answer is" << f;
    }
    CORRADE_COMPARE(out.str(), "The answer is 42\n");
}

namespace {

struct Bar { };

inline std::ostream& operator<<(std::ostream& o, const Bar&) {
    return o << "bar";
}

}

void DebugTest::ostream_fallback() {
    std::ostringstream out;
    Debug::setOutput(&out);

    Bar bar;
    Debug() << bar;

    CORRADE_COMPARE(out.str(), "bar\n");
}

namespace {

struct Baz { };

inline std::ostream& operator<<(std::ostream& o, const Baz&) {
    return o << "wrong baz";
}

inline Debug operator<<(Debug debug, const Baz&) {
    return debug << "baz";
}

}

void DebugTest::prefer_not_to_use_fallback() {
    // Suppress unused function warning by using the otherwise-unused
    // std::ostream overload.
    std::ostringstream{} << Baz{};

    std::ostringstream out;
    Debug::setOutput(&out);

    Baz baz;
    Debug() << baz;

    CORRADE_COMPARE(out.str(), "baz\n");
}

void DebugTest::flags() {
    std::ostringstream out;
    Debug::setOutput(&out);

    {
        /* Don't allow to set/reset the reserved flag */
        Debug debug;
        debug.setFlag(static_cast<Debug::Flag>(0x01), false);
        CORRADE_VERIFY(debug.flag(static_cast<Debug::Flag>(0x01)));
    } {
        Debug debug;
        debug.setFlag(Debug::SpaceAfterEachValue, false);
        debug << "a" << "b" << "c";
    }
    CORRADE_COMPARE(out.str(), "abc\n");
    out.str("");
    {
        Debug debug;
        debug.setFlag(Debug::NewLineAtTheEnd, false);
        debug << "a" << "b" << "c";
    }
    CORRADE_COMPARE(out.str(), "a b c");
}

void DebugTest::iterable() {
    std::ostringstream out;
    Debug::setOutput(&out);
    Debug() << std::vector<int>{1, 2, 3};
    CORRADE_COMPARE(out.str(), "{1, 2, 3}\n");

    out.str({});
    Debug() << std::set<std::string>{"a", "b", "c"};
    CORRADE_COMPARE(out.str(), "{a, b, c}\n");

    out.str({});
    Debug() << std::map<int, std::string>{{1, "a"}, {2, "b"}, {3, "c"}};
    CORRADE_COMPARE(out.str(), "{(1, a), (2, b), (3, c)}\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DebugTest)
