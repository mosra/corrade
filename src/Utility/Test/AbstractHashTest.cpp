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

#include "AbstractHashTest.h"

#include <sstream>

#include "Utility/AbstractHash.h"

CORRADE_TEST_MAIN(Corrade::Utility::Test::AbstractHashTest)

namespace Corrade { namespace Utility { namespace Test {

AbstractHashTest::AbstractHashTest() {
    addTests(&AbstractHashTest::toHexString,
             &AbstractHashTest::fromHexString,
             &AbstractHashTest::debug);
}

void AbstractHashTest::toHexString() {
    const unsigned char rawDigest[4] = { 0xCA, 0xFE, 0x90, 0xfa };
    CORRADE_COMPARE(AbstractHash<4>::Digest::fromByteArray(reinterpret_cast<const char*>(rawDigest)).hexString(), "cafe90fa");
}

void AbstractHashTest::fromHexString() {
    CORRADE_COMPARE(AbstractHash<4>::Digest::fromHexString("cafe90fa").hexString(), "cafe90fa");
    CORRADE_COMPARE(AbstractHash<4>::Digest::fromHexString("1234abcdef").hexString(), "00000000");
    CORRADE_COMPARE(AbstractHash<4>::Digest::fromHexString("babe").hexString(), "00000000");
    CORRADE_COMPARE(AbstractHash<4>::Digest::fromHexString("bullshit").hexString(), "00000000");
}

void AbstractHashTest::debug() {
    std::ostringstream out;
    Debug(&out) << AbstractHash<4>::Digest::fromHexString("defeca7e");
    CORRADE_COMPARE(out.str(), "defeca7e\n");
}

}}}
