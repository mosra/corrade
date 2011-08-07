/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "AbstractHashTest.h"

#include <QtTest/QTest>

#include "Utility/AbstractHash.h"
#include "Utility/Debug.h"

QTEST_APPLESS_MAIN(Kompas::Utility::Test::AbstractHashTest)

namespace Kompas { namespace Utility { namespace Test {

void AbstractHashTest::toHexString() {
    const unsigned char rawDigest[4] = { 0xCA, 0xFE, 0x90, 0xfa };
    Debug() << "+" + AbstractHash<4>::Digest::fromByteArray(reinterpret_cast<const char*>(rawDigest)).hexString() + "+";
    QVERIFY(AbstractHash<4>::Digest::fromByteArray(reinterpret_cast<const char*>(rawDigest)).hexString() == "cafe90fa");
}

void AbstractHashTest::fromHexString() {
    QVERIFY(AbstractHash<4>::Digest::fromHexString("cafe90fa").hexString() == "cafe90fa");
    QVERIFY(AbstractHash<4>::Digest::fromHexString("1234abcdef").hexString() == "00000000");
    QVERIFY(AbstractHash<4>::Digest::fromHexString("babe").hexString() == "00000000");
    QVERIFY(AbstractHash<4>::Digest::fromHexString("bullshit").hexString() == "00000000");
}

}}}
