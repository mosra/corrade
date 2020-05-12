/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/Endianness.h"
#include "Corrade/Utility/EndiannessBatch.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct EndiannessTest: TestSuite::Tester {
    explicit EndiannessTest();

    void endianness();
    void floats();
    void inPlace();
    void inPlaceUnaligned();
    void inPlaceList();
    void inPlaceListUnaligned();
    void enumClass();
};

EndiannessTest::EndiannessTest() {
    addTests({&EndiannessTest::endianness,
              &EndiannessTest::floats,
              &EndiannessTest::inPlace,
              &EndiannessTest::inPlaceUnaligned,
              &EndiannessTest::inPlaceList,
              &EndiannessTest::inPlaceListUnaligned,
              &EndiannessTest::enumClass});
}

void EndiannessTest::endianness() {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
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

    CORRADE_COMPARE(Endianness::swap<std::uint8_t>(0x40), 0x40);
    CORRADE_COMPARE(Endianness::swap<std::uint32_t>(0x11223344), 0x44332211);
    CORRADE_COMPARE(Endianness::swap<std::int32_t>(0x77665544), 0x44556677);
    CORRADE_COMPARE(Endianness::swap<std::int16_t>(0x7F00), 0x007F);
    CORRADE_COMPARE(Endianness::swap<std::uint64_t>(0x1122334455667788ull), 0x8877665544332211ull);

    CORRADE_COMPARE(Endianness::other<std::uint8_t>(0x40), 0x40);
    CORRADE_COMPARE(Endianness::other<std::uint32_t>(0x11223344), 0x44332211);
    CORRADE_COMPARE(Endianness::other<std::int32_t>(0x77665544), 0x44556677);
    CORRADE_COMPARE(Endianness::other<std::int16_t>(0x7F00), 0x007F);
    CORRADE_COMPARE(Endianness::other<std::uint64_t>(0x1122334455667788ull), 0x8877665544332211ull);

    #undef current
    #undef other
}

void EndiannessTest::floats() {
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

void EndiannessTest::inPlace() {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
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

    Endianness::currentInPlace(a, b, c, d);
    CORRADE_COMPARE(a, 0x70);
    CORRADE_COMPARE(b, 0x11223344);
    CORRADE_COMPARE(c, 0x7F00);
    CORRADE_COMPARE(d, 0x1122334455667788ull);

    Endianness::swapInPlace(a, b, c, d);
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

void EndiannessTest::inPlaceUnaligned() {
    CORRADE_ALIGNAS(4) char data[] = {
        '\x11', '\x22', '\x33', '\x44', '\x55', '\x66', '\x77', '\x88'
    };

    Endianness::swapInPlace(*reinterpret_cast<float*>(data + 3));
    CORRADE_COMPARE(data[3], '\x77');
    CORRADE_COMPARE(data[4], '\x66');
    CORRADE_COMPARE(data[5], '\x55');
    CORRADE_COMPARE(data[6], '\x44');
}

void EndiannessTest::inPlaceList() {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
    #define currentInPlace bigEndianInPlace
    #define otherInPlace littleEndianInPlace
    #else
    #define currentInPlace littleEndianInPlace
    #define otherInPlace bigEndianInPlace
    #endif

    std::int8_t a[]{0x11, 0x22, 0x33, 0x44};
    std::uint16_t b[]{0x1122, 0x3344};
    std::int32_t c[]{0x11223344, 0x55667700};
    std::uint64_t d[]{0x1122334455667700ull, 0x00aabbccddeeff11ull};

    Endianness::currentInPlace(Containers::arrayView(a));
    Endianness::currentInPlace(Containers::arrayView(b));
    Endianness::currentInPlace(Containers::arrayView(c));
    Endianness::currentInPlace(Containers::arrayView(d));
    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView<std::int8_t>({
            0x11, 0x22, 0x33, 0x44
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(b),
        Containers::arrayView<std::uint16_t>({
            0x1122, 0x3344
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(c),
        Containers::arrayView<std::int32_t>({
            0x11223344, 0x55667700
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(d),
        Containers::arrayView<std::uint64_t>({
            0x1122334455667700ull, 0x00aabbccddeeff11ull
        }), TestSuite::Compare::Container);

    Endianness::swapInPlace(Containers::arrayView(a));
    Endianness::swapInPlace(Containers::arrayView(b));
    Endianness::swapInPlace(Containers::arrayView(c));
    Endianness::swapInPlace(Containers::arrayView(d));
    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView<std::int8_t>({
            0x11, 0x22, 0x33, 0x44
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(b),
        Containers::arrayView<std::uint16_t>({
            0x2211, 0x4433
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(c),
        Containers::arrayView<std::int32_t>({
            0x44332211, 0x00776655
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(d),
        Containers::arrayView<std::uint64_t>({
            0x0077665544332211ull, 0x11ffeeddccbbaa00ull
        }), TestSuite::Compare::Container);

    Endianness::otherInPlace(Containers::arrayView(a));
    Endianness::otherInPlace(Containers::arrayView(b));
    Endianness::otherInPlace(Containers::arrayView(c));
    Endianness::otherInPlace(Containers::arrayView(d));
    CORRADE_COMPARE_AS(Containers::arrayView(a),
        Containers::arrayView<std::int8_t>({
            0x11, 0x22, 0x33, 0x44
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(b),
        Containers::arrayView<std::uint16_t>({
            0x1122, 0x3344
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(c),
        Containers::arrayView<std::int32_t>({
            0x11223344, 0x55667700
        }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(Containers::arrayView(d),
        Containers::arrayView<std::uint64_t>({
            0x1122334455667700ull, 0x00aabbccddeeff11ull
        }), TestSuite::Compare::Container);

    #undef currentInPlace
    #undef otherInPlace
}

void EndiannessTest::inPlaceListUnaligned() {
    CORRADE_ALIGNAS(4) char data[] = {
        '\x11', '\x22', '\x33', '\x44', '\x55', '\x66', '\x77', '\x88', '\x99'
    };

    Endianness::swapInPlace(Containers::arrayCast<int>(Containers::arrayView(data).suffix(1)));
    CORRADE_COMPARE(data[1], '\x55');
    CORRADE_COMPARE(data[2], '\x44');
    CORRADE_COMPARE(data[3], '\x33');
    CORRADE_COMPARE(data[4], '\x22');
    CORRADE_COMPARE(data[5], '\x99');
    CORRADE_COMPARE(data[6], '\x88');
    CORRADE_COMPARE(data[7], '\x77');
    CORRADE_COMPARE(data[8], '\x66');
}

void EndiannessTest::enumClass() {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
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

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::EndiannessTest)
