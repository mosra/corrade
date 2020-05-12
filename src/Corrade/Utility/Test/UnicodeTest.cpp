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

#include <sstream>
#include <string>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Unicode.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct UnicodeTest: TestSuite::Tester {
    explicit UnicodeTest();

    void nextUtf8();
    void nextUtf8Error();
    void nextUtf8Empty();

    void prevUtf8();
    void prevUtf8Error();
    void prevUtf8Empty();

    void utf8utf32();
    void utf32utf8();
    void utf32utf8Error();

    #ifdef CORRADE_TARGET_WINDOWS
    void widen();
    void narrow();
    #endif
};

UnicodeTest::UnicodeTest() {
    addTests({&UnicodeTest::nextUtf8,
              &UnicodeTest::nextUtf8Error,
              &UnicodeTest::nextUtf8Empty,

              &UnicodeTest::prevUtf8,
              &UnicodeTest::prevUtf8Error,
              &UnicodeTest::prevUtf8Empty,

              &UnicodeTest::utf8utf32,
              &UnicodeTest::utf32utf8,
              &UnicodeTest::utf32utf8Error,

              #ifdef CORRADE_TARGET_WINDOWS
              &UnicodeTest::widen,
              &UnicodeTest::narrow
              #endif
              });
}

void UnicodeTest::nextUtf8() {
    /* One-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \x7f", 3), std::make_pair(127, 4));

    /* Two byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xce\xac", 3), std::make_pair(940, 5));

    /* Three-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xea\xb8\x89", 3), std::make_pair(44553, 6));

    /* Four-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xf4\x85\x98\x80", 3), std::make_pair(1070592, 7));

    /* std::string argument */
    CORRADE_COMPARE(Unicode::nextChar(std::string{"   \xea\xb8\x89"}, 3),
        std::make_pair(44553, 6));
}

void UnicodeTest::nextUtf8Error() {
    /* Wrong start of a sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xb0", 3), std::make_pair(0xffffffffu, 4));

    /* Garbage in multibyte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xea\x40\xb8", 3), std::make_pair(0xffffffffu, 4));

    /* Too small string for mulibyte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xce", 3), std::make_pair(0xffffffffu, 4));
}

void UnicodeTest::nextUtf8Empty() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Unicode::nextChar("", 0);

    CORRADE_COMPARE(out.str(), "Utility::Unicode::nextChar(): cursor out of range\n");
}

void UnicodeTest::prevUtf8() {
    /* One-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x7f", 4), std::make_pair(127, 3));

    /* Two byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xce\xac", 5), std::make_pair(940, 3));

    /* Three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xea\xb8\x89", 6), std::make_pair(44553, 3));

    /* Four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xf4\x85\x98\x80", 7), std::make_pair(1070592, 3));

    /* std::string argument */
    CORRADE_COMPARE(Unicode::prevChar(std::string{"   \xea\xb8\x89"}, 6),
        std::make_pair(44553, 3));
}

void UnicodeTest::prevUtf8Error() {
    /* Wrong start of a sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xb0", 4), std::make_pair(0xffffffffu, 3));

    /* Garbage in two-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\xac", 5), std::make_pair(0xffffffffu, 4));

    /* Garbage in three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\xb8\xb8", 6), std::make_pair(0xffffffffu, 5));

    /* Garbage in four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\x85\x98\x80", 7), std::make_pair(0xffffffffu, 6));

    /* Too small string for two-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\xac", 1), std::make_pair(0xffffffffu, 0));

    /* Too small string for three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\xb8\x89", 2), std::make_pair(0xffffffffu, 1));

    /* Too small string for four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\x85\x98\x80", 3), std::make_pair(0xffffffffu, 2));
}

void UnicodeTest::prevUtf8Empty() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Unicode::prevChar("hello", 0);

    CORRADE_COMPARE(out.str(), "Utility::Unicode::prevChar(): cursor already at the beginning\n");
}

void UnicodeTest::utf8utf32() {
    CORRADE_COMPARE(Unicode::utf32("žluťoučký kůň"),
                    U"\U0000017elu\U00000165ou\U0000010dk\U000000fd k\U0000016f\U00000148");

    /* Empty string shouldn't crash */
    CORRADE_COMPARE(Unicode::utf32(""), U"");
}

void UnicodeTest::utf32utf8() {
    char result[4];
    std::size_t size;

    /* One-byte sequence */
    size = Unicode::utf8(127, result);
    CORRADE_COMPARE(size, 1);
    CORRADE_COMPARE((std::string{result, size}), "\x7f");

    /* Two-byte sequence */
    size = Unicode::utf8(940, result);
    CORRADE_COMPARE(size, 2);
    CORRADE_COMPARE((std::string{result, size}), "\xce\xac");

    /* Three-byte sequence */
    size = Unicode::utf8(44553, result);
    CORRADE_COMPARE(size, 3);
    CORRADE_COMPARE((std::string{result, size}), "\xea\xb8\x89");

    /* Four-byte sequence */
    size = Unicode::utf8(1070592, result);
    CORRADE_COMPARE(size, 4);
    CORRADE_COMPARE((std::string{result, size}), "\xf4\x85\x98\x80");
}

void UnicodeTest::utf32utf8Error() {
    /* Codepoint outside of the range */
    CORRADE_VERIFY(!Unicode::utf8(1594880, nullptr));
}

#ifdef CORRADE_TARGET_WINDOWS
void UnicodeTest::widen() {
    const char text[] = "žluťoučký kůň\0hýždě";
    const wchar_t result[] = L"\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148\u0000h\u00fd\u017ed\u011b";

    CORRADE_COMPARE(Unicode::widen(text),
                    result);
    CORRADE_COMPARE(Unicode::widen(std::string{text, sizeof(text)}),
                    (std::wstring{result, sizeof(result)/sizeof(wchar_t)}));
    CORRADE_COMPARE(Unicode::widen(Containers::ArrayView<const char>{text, sizeof(text)}),
                    (std::wstring{result, sizeof(result)/sizeof(wchar_t)}));

    /* Empty string shouldn't crash */
    CORRADE_COMPARE(Unicode::widen(""), L"");
}

void UnicodeTest::narrow() {
    const wchar_t text[] = L"\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148\u0000h\u00fd\u017ed\u011b";
    const char result[] = "žluťoučký kůň\0hýždě";

    CORRADE_COMPARE(Unicode::narrow(text),
                    result);
    CORRADE_COMPARE(Unicode::narrow(std::wstring{text, sizeof(text)/sizeof(wchar_t)}),
                    (std::string{result, sizeof(result)}));
    CORRADE_COMPARE(Unicode::narrow(Containers::ArrayView<const wchar_t>{text, sizeof(text)/sizeof(wchar_t)}),
                    (std::string{result, sizeof(result)}));

    /* Empty string shouldn't crash */
    CORRADE_COMPARE(Unicode::narrow(L""), "");
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::UnicodeTest)
