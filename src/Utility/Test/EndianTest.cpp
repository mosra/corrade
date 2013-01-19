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
#include "Utility/Endianness.h"
#include "Utility/Debug.h"

namespace Corrade { namespace Utility { namespace Test {

class EndianTest: public TestSuite::Tester {
    public:
        EndianTest();

        void endianness();
};

EndianTest::EndianTest() {
    addTests(&EndianTest::endianness);
}

void EndianTest::endianness() {
    #ifdef CORRADE_BIG_ENDIAN
        CORRADE_VERIFY(Endianness::isBigEndian());
        Debug() << "Big endian system";
        #define current bigEndian
        #define other littleEndian
    #else
        CORRADE_VERIFY(!Endianness::isBigEndian());
        Debug() << "Little endian system";
        #define current littleEndian
        #define other bigEndian
    #endif

    CORRADE_COMPARE(Endianness::current<std::uint32_t>(0x11223344), 0x11223344);
    CORRADE_COMPARE(Endianness::other<std::uint32_t>(0x11223344), 0x44332211);
    CORRADE_COMPARE(Endianness::other<std::int32_t>(0x77665544), 0x44556677);
    CORRADE_COMPARE(Endianness::other<std::int16_t>(0x7F00), 0x007F);
    CORRADE_COMPARE(Endianness::other<std::uint64_t>(0x1122334455667788ull), 0x8877665544332211ull);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::EndianTest)
