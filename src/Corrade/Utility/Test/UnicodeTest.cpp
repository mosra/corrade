/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <sstream>
#include <tuple>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Unicode.h"

namespace Corrade { namespace Utility { namespace Test {

struct UnicodeTest: TestSuite::Tester {
    explicit UnicodeTest();

    void nextUtf8();
    void nextUtf8Error();
    void nextUtf8Empty();

    void prevUtf8();
    void prevUtf8Error();
    void prevUtf8Empty();

    void utf8utf32();
};

UnicodeTest::UnicodeTest() {
    addTests({&UnicodeTest::nextUtf8,
              &UnicodeTest::nextUtf8Error,
              &UnicodeTest::nextUtf8Empty,

              &UnicodeTest::prevUtf8,
              &UnicodeTest::prevUtf8Error,
              &UnicodeTest::prevUtf8Empty,

              &UnicodeTest::utf8utf32});
}

void UnicodeTest::nextUtf8() {
    std::uint32_t codepoint;
    std::size_t next;

    /* One-byte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \x7f", 3);
    CORRADE_COMPARE(next, 4);
    CORRADE_COMPARE(codepoint, 127);

    /* Two byte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xce\xac", 3);
    CORRADE_COMPARE(next, 5);
    CORRADE_COMPARE(codepoint, 940);

    /* Three-byte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xea\xb8\x89", 3);
    CORRADE_COMPARE(next, 6);
    CORRADE_COMPARE(codepoint, 44553);

    /* Four-byte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xf6\x85\x98\x80", 3);
    CORRADE_COMPARE(next, 7);
    CORRADE_COMPARE(codepoint, 1594880);
}

void UnicodeTest::nextUtf8Error() {
    std::uint32_t codepoint;
    std::size_t next;

    /* Wrong start of a sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xb0", 3);
    CORRADE_COMPARE(next, 4);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Garbage in multibyte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xea\x40\xb8", 3);
    CORRADE_COMPARE(next, 4);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Too small string for mulibyte sequence */
    std::tie(codepoint, next) = Unicode::nextChar("   \xce", 3);
    CORRADE_COMPARE(next, 4);
    CORRADE_COMPARE(codepoint, 0xffffffffu);
}

void UnicodeTest::nextUtf8Empty() {
    std::ostringstream out;
    Error redirectError{&out};
    Unicode::nextChar("", 0);

    CORRADE_COMPARE(out.str(), "Utility::Unicode::nextChar(): cursor out of range\n");
}

void UnicodeTest::prevUtf8() {
    std::uint32_t codepoint;
    std::size_t prev;

    /* One-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \x7f", 4);
    CORRADE_COMPARE(prev, 3);
    CORRADE_COMPARE(codepoint, 127);

    /* Two byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \xce\xac", 5);
    CORRADE_COMPARE(prev, 3);
    CORRADE_COMPARE(codepoint, 940);

    /* Three-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \xea\xb8\x89", 6);
    CORRADE_COMPARE(prev, 3);
    CORRADE_COMPARE(codepoint, 44553);

    /* Four-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \xf6\x85\x98\x80", 7);
    CORRADE_COMPARE(prev, 3);
    CORRADE_COMPARE(codepoint, 1594880u);
}

void UnicodeTest::prevUtf8Error() {
    std::uint32_t codepoint;
    std::size_t prev;

    /* Wrong start of a sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \xb0", 4);
    CORRADE_COMPARE(prev, 3);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Garbage in two-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \x40\xac", 5);
    CORRADE_COMPARE(prev, 4);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Garbage in three-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \x40\xb8\xb8", 6);
    CORRADE_COMPARE(prev, 5);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Garbage in four-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("   \x40\x85\x98\x80", 7);
    CORRADE_COMPARE(prev, 6);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Too small string for two-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("\xac", 1);
    CORRADE_COMPARE(prev, 0);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Too small string for three-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("\xb8\x89", 2);
    CORRADE_COMPARE(prev, 1);
    CORRADE_COMPARE(codepoint, 0xffffffffu);

    /* Too small string for four-byte sequence */
    std::tie(codepoint, prev) = Unicode::prevChar("\x85\x98\x80", 3);
    CORRADE_COMPARE(prev, 2);
    CORRADE_COMPARE(codepoint, 0xffffffffu);
}

void UnicodeTest::prevUtf8Empty() {
    std::ostringstream out;
    Error redirectError{&out};
    Unicode::prevChar("hello", 0);

    CORRADE_COMPARE(out.str(), "Utility::Unicode::prevChar(): cursor already at the beginning\n");
}

void UnicodeTest::utf8utf32() {
    CORRADE_COMPARE(Unicode::utf32("žluťoučký kůň"),
                    U"\u017Elu\u0165ou\u010Dk\u00FD k\u016F\u0148");

    /* Empty string shouldn't crash */
    CORRADE_COMPARE(Unicode::utf32(""), U"");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::UnicodeTest)
