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

#include <tuple>

#include "TestSuite/Tester.h"
#include "Utility/Unicode.h"

namespace Corrade { namespace Utility { namespace Test {

class UnicodeTest: public TestSuite::Tester {
    public:
        explicit UnicodeTest();

        void nextUtf8();
        void nextUtf8Error();
};

UnicodeTest::UnicodeTest() {
    addTests(&UnicodeTest::nextUtf8,
             &UnicodeTest::nextUtf8Error);
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

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::UnicodeTest)
