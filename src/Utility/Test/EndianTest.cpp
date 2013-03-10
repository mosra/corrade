/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
    addTests({&EndianTest::endianness});
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
