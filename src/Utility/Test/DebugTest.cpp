/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

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

#include <sstream>
#include <QtTest/QTest>

#include "Utility/Debug.h"

using namespace std;

QTEST_APPLESS_MAIN(Corrade::Utility::Test::DebugTest)

namespace Corrade { namespace Utility { namespace Test {

void DebugTest::debug() {
    ostringstream debug, warning, error;

    Debug::setOutput(&debug);
    Warning::setOutput(&warning);
    Error::setOutput(&error);
    Debug() << "a" << 33 << 0.567f;
    Warning() << "w" << 42 << 'c';
    Error() << "e";

    QCOMPARE(QString::fromStdString(debug.str()), QString("a 33 0.567\n"));
    QCOMPARE(QString::fromStdString(warning.str()), QString("w 42 c\n"));
    QCOMPARE(QString::fromStdString(error.str()), QString("e\n"));

    /* Don't add newline at the end of empty output */
    debug.str("");
    Debug();
    QCOMPARE(QString::fromStdString(debug.str()), QString(""));
}

struct Foo {
    int value;
};

Debug& operator<<(Debug debug, const Foo& value) {
    return debug << value.value;
}

void DebugTest::custom() {
    ostringstream out;
    Debug::setOutput(&out);

    Foo f = { 42 };
    {
        Debug() << "The answer is" << f;
    }
    QCOMPARE(QString::fromStdString(out.str()), QString("The answer is 42\n"));
}

void DebugTest::flags() {
    ostringstream out;
    Debug::setOutput(&out);

    {
        /* Don't allow to set/reset the reserved flag */
        Debug debug;
        debug.setFlag(static_cast<Debug::Flag>(0x01), false);
        QVERIFY(debug.flag(static_cast<Debug::Flag>(0x01)));
    } {
        Debug debug;
        debug.setFlag(Debug::SpaceAfterEachValue, false);
        debug << 'a' << 'b' << 'c';
    }
    QCOMPARE(QString::fromStdString(out.str()), QString("abc\n"));
    out.str("");
    {
        Debug debug;
        debug.setFlag(Debug::NewLineAtTheEnd, false);
        debug << 'a' << 'b' << 'c';
    }
    QCOMPARE(QString::fromStdString(out.str()), QString("a b c"));
}

}}}
