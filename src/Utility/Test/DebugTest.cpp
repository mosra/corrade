/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "DebugTest.h"

#include <sstream>
#include <QtTest/QTest>

#include "Utility/Debug.h"

using namespace std;

QTEST_APPLESS_MAIN(Kompas::Utility::Test::DebugTest)

namespace Kompas { namespace Utility { namespace Test {

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

}}}
