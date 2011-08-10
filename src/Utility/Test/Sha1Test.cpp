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

#include "Sha1Test.h"

#include <QtTest/QTest>

#include "Utility/Sha1.h"

QTEST_APPLESS_MAIN(Kompas::Utility::Test::Sha1Test)

namespace Kompas { namespace Utility { namespace Test {

void Sha1Test::emptyString() {
    QVERIFY(Sha1::digest("").hexString() == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}

void Sha1Test::exact64bytes() {
    QVERIFY(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456789f1234").hexString() == "d9aa447706df8797b4f5fe94caa9f6ea723a87c8");
}

void Sha1Test::exactOneBlockPadding() {
    QVERIFY(Sha1::digest("123456789a123456789b123456789c123456789d123456789e12345").hexString() == "4cc8d5cfacbb575ddeeed504dd4f7cc09a9d49a3");
}

void Sha1Test::twoBlockPadding() {
    QVERIFY(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456").hexString() == "40e94c62ada5dc762f3e9c472001ca64a67d2cbb");
}

}}}
