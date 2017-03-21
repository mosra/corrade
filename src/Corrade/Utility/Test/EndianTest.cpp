/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include <cstdint>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Endianness.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility { namespace Test {

struct EndianTest: TestSuite::Tester {
    explicit EndianTest();

    void endianness();
    void floats();
    void inPlace();
    void enumClass();
};

EndianTest::EndianTest() {
    addTests({&EndianTest::endianness,
              &EndianTest::floats,
              &EndianTest::inPlace,
              &EndianTest::enumClass});
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
    CORRADE_COMPARE(Endianness::other<std::uint8_t>(0x40), 0x40);
    CORRADE_COMPARE(Endianness::other<std::uint32_t>(0x11223344), 0x44332211);
    CORRADE_COMPARE(Endianness::other<std::int32_t>(0x77665544), 0x44556677);
    CORRADE_COMPARE(Endianness::other<std::int16_t>(0x7F00), 0x007F);
    CORRADE_COMPARE(Endianness::other<std::uint64_t>(0x1122334455667788ull), 0x8877665544332211ull);

    #undef current
    #undef other
}

void EndianTest::floats() {
    /* Verifies that the swapping operation doesn't involve any
       information-losing type conversion */
    float original = -456.7896713f;
    float swapped = Endianness::swap(original);
    float back = Endianness::swap(swapped);

    /* Compare bitwise (as opposed to fuzzy compare), as the values should be
       exactly the same */
    CORRADE_VERIFY(swapped != original);
    CORRADE_VERIFY(back == original);
}

void EndianTest::inPlace() {
    #ifdef CORRADE_BIG_ENDIAN
    #define currentInPlace bigEndianInPlace
    #define otherInPlace littleEndianInPlace
    #else
    #define currentInPlace littleEndianInPlace
    #define otherInPlace bigEndianInPlace
    #endif

    std::int8_t a = 0x70;
    std::uint32_t b = 0x11223344;
    std::int16_t c = 0x7F00;
    std::uint64_t d = 0x1122334455667788ull;

    Endianness::otherInPlace(a, b, c, d);
    CORRADE_COMPARE(a, 0x70);
    CORRADE_COMPARE(b, 0x44332211);
    CORRADE_COMPARE(c, 0x007F);
    CORRADE_COMPARE(d, 0x8877665544332211ull);

    Endianness::otherInPlace(a, b, c, d);
    CORRADE_COMPARE(a, 0x70);
    CORRADE_COMPARE(b, 0x11223344);
    CORRADE_COMPARE(c, 0x7F00);
    CORRADE_COMPARE(d, 0x1122334455667788ull);

    #undef currentInPlace
    #undef otherInPlace
}

void EndianTest::enumClass() {
    #ifdef CORRADE_BIG_ENDIAN
    #define other littleEndian
    #define otherInPlace littleEndianInPlace
    #else
    #define other bigEndian
    #define otherInPlace bigEndianInPlace
    #endif

    enum class FileType: std::uint32_t {
        PlainText = 0xcafebabe,
        Binary = 0xdeadbeef
    };

    FileType a = FileType(0xbebafeca);
    const FileType b = FileType(0xefbeadde);

    Endianness::otherInPlace(a);
    const FileType c = Endianness::other(b);

    CORRADE_VERIFY(a == FileType::PlainText);
    CORRADE_VERIFY(c == FileType::Binary);

    #undef other
    #undef otherInPlace
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::EndianTest)
