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

#include "TestSuite/Tester.h"
#include "Utility/Sha1.h"

namespace Corrade { namespace Utility { namespace Test {

class Sha1Test: public TestSuite::Tester {
    public:
        Sha1Test();

        void emptyString();
        void exact64bytes();
        void exactOneBlockPadding();
        void twoBlockPadding();
};

Sha1Test::Sha1Test() {
    addTests({&Sha1Test::emptyString,
              &Sha1Test::exact64bytes,
              &Sha1Test::exactOneBlockPadding,
              &Sha1Test::twoBlockPadding});
}

void Sha1Test::emptyString() {
    CORRADE_COMPARE(Sha1::digest(""),
                    Sha1::Digest::fromHexString("da39a3ee5e6b4b0d3255bfef95601890afd80709"));
}

void Sha1Test::exact64bytes() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456789f1234"),
                    Sha1::Digest::fromHexString("d9aa447706df8797b4f5fe94caa9f6ea723a87c8"));
}

void Sha1Test::exactOneBlockPadding() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e12345"),
                    Sha1::Digest::fromHexString("4cc8d5cfacbb575ddeeed504dd4f7cc09a9d49a3"));
}

void Sha1Test::twoBlockPadding() {
    CORRADE_COMPARE(Sha1::digest("123456789a123456789b123456789c123456789d123456789e123456"),
                    Sha1::Digest::fromHexString("40e94c62ada5dc762f3e9c472001ca64a67d2cbb"));
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::Sha1Test)
