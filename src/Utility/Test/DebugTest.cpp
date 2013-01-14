/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "DebugTest.h"

#include <map>
#include <set>
#include <sstream>

#include "Utility/Debug.h"

CORRADE_TEST_MAIN(Corrade::Utility::Test::DebugTest)

namespace Corrade { namespace Utility { namespace Test {

DebugTest::DebugTest() {
    addTests(&DebugTest::debug,
             &DebugTest::boolean,
             &DebugTest::chars,
             &DebugTest::custom,
             &DebugTest::flags,
             &DebugTest::iterable);
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

struct Foo {
    int value;
};

Debug operator<<(Debug debug, const Foo& value) {
    return debug << value.value;
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
    CORRADE_COMPARE(out.str(), "[1, 2, 3]\n");

    out.str("");
    Debug() << std::set<std::string>{"a", "b", "c"};
    CORRADE_COMPARE(out.str(), "[a, b, c]\n");

    out.str("");
    Debug() << std::map<int, std::string>{{1, "a"}, {2, "b"}, {3, "c"}};
    CORRADE_COMPARE(out.str(), "[(1, a), (2, b), (3, c)]\n");
}

}}}
