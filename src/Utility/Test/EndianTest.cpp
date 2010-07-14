/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "EndianTest.h"

#include <QtTest/QTest>

#include "Utility/Endianness.h"

QTEST_APPLESS_MAIN(Map2X::Utility::Test::EndianTest)

namespace Map2X { namespace Utility { namespace Test {

void EndianTest::endianness() {
    #ifdef ENDIANNESS_BIG_ENDIAN
        QVERIFY(Endianness::isBigEndian());
        qDebug("Big endian system");
        #define current bigEndian
        #define other littleEndian
    #else
        QVERIFY(!Endianness::isBigEndian());
        qDebug("Little endian system");
        #define current littleEndian
        #define other bigEndian
    #endif

    QVERIFY(Endianness::current<unsigned int>(0x11223344) == 0x11223344);
    QVERIFY(Endianness::other<unsigned int>(0x11223344) == 0x44332211);
    QVERIFY(Endianness::other<int>(0x77665544) == 0x44556677);
    QVERIFY(Endianness::other<short>(0x7F00) == 0x007F);
    QVERIFY(Endianness::other<quint64>(0x1122334455667788) == 0x8877665544332211);
}

}}}
