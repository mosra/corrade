/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Unicode.h"

#ifdef CORRADE_TARGET_WINDOWS
#include "Corrade/TestSuite/Compare/Container.h"
#endif

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
    void widenEmpty();
    void narrow();
    void narrowEmpty();
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
              &UnicodeTest::widenEmpty,
              &UnicodeTest::narrow,
              &UnicodeTest::narrowEmpty,
              #endif
              });
}

using namespace Containers::Literals;

void UnicodeTest::nextUtf8() {
    /* One-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \x7f", 3),
        (Containers::Pair<char32_t, std::size_t>{127, 4}));

    /* Two byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xce\xac", 3),
        (Containers::Pair<char32_t, std::size_t>{940, 5}));

    /* Three-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xea\xb8\x89", 3),
        (Containers::Pair<char32_t, std::size_t>{44553, 6}));

    /* Four-byte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xf4\x85\x98\x80", 3),
        (Containers::Pair<char32_t, std::size_t>{1070592, 7}));
}

void UnicodeTest::nextUtf8Error() {
    /* Wrong start of a sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xb0", 3),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 4}));

    /* Garbage in multibyte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xea\x40\xb8", 3),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 4}));

    /* Too small string for mulibyte sequence */
    CORRADE_COMPARE(Unicode::nextChar("   \xce", 3),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 4}));
}

void UnicodeTest::nextUtf8Empty() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    Unicode::nextChar("", 0);
    CORRADE_COMPARE(out.str(), "Utility::Unicode::nextChar(): cursor out of range\n");
}

void UnicodeTest::prevUtf8() {
    /* One-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x7f", 4),
        (Containers::Pair<char32_t, std::size_t>{127, 3}));

    /* Two byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xce\xac", 5),
        (Containers::Pair<char32_t, std::size_t>{940, 3}));

    /* Three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xea\xb8\x89", 6),
        (Containers::Pair<char32_t, std::size_t>{44553, 3}));

    /* Four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xf4\x85\x98\x80", 7),
        (Containers::Pair<char32_t, std::size_t>{1070592, 3}));
}

void UnicodeTest::prevUtf8Error() {
    /* Wrong start of a sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \xb0", 4),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 3}));

    /* Garbage in two-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\xac", 5),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 4}));

    /* Garbage in three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\xb8\xb8", 6),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 5}));

    /* Garbage in four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("   \x40\x85\x98\x80", 7),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 6}));

    /* Too small string for two-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\xac", 1),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 0}));

    /* Too small string for three-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\xb8\x89", 2),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 1}));

    /* Too small string for four-byte sequence */
    CORRADE_COMPARE(Unicode::prevChar("\x85\x98\x80", 3),
        (Containers::Pair<char32_t, std::size_t>{0xffffffffu, 2}));
}

void UnicodeTest::prevUtf8Empty() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    Unicode::prevChar("hello", 0);
    CORRADE_COMPARE(out.str(), "Utility::Unicode::prevChar(): cursor already at the beginning\n");
}

void UnicodeTest::utf8utf32() {
    {
        Containers::Optional<Containers::Array<char32_t>> utf32 = Unicode::utf32("žluťoučký kůň");
        CORRADE_VERIFY(utf32);
        CORRADE_COMPARE_AS(*utf32,
            Containers::arrayView(U"\U0000017elu\U00000165ou\U0000010dk\U000000fd k\U0000016f\U00000148").exceptSuffix(1),
            TestSuite::Compare::Container);

    /* Invalid characters return a null Optional */
    } {
        Containers::Optional<Containers::Array<char32_t>> utf32 = Unicode::utf32("he\xff\xffo");
        CORRADE_VERIFY(!utf32);

    /* Empty string shouldn't crash */
    } {
        Containers::Optional<Containers::Array<char32_t>> utf32 = Unicode::utf32("");
        CORRADE_VERIFY(utf32);
        CORRADE_COMPARE_AS(*utf32,
            Containers::ArrayView<const char32_t>{},
            TestSuite::Compare::Container);
    }
}

void UnicodeTest::utf32utf8() {
    char result[4];
    std::size_t size;

    /* One-byte sequence */
    size = Unicode::utf8(127, result);
    CORRADE_COMPARE(size, 1);
    CORRADE_COMPARE((Containers::StringView{result, size}), "\x7f");

    /* Two-byte sequence */
    size = Unicode::utf8(940, result);
    CORRADE_COMPARE(size, 2);
    CORRADE_COMPARE((Containers::StringView{result, size}), "\xce\xac");

    /* Three-byte sequence */
    size = Unicode::utf8(44553, result);
    CORRADE_COMPARE(size, 3);
    CORRADE_COMPARE((Containers::StringView{result, size}), "\xea\xb8\x89");

    /* Four-byte sequence */
    size = Unicode::utf8(1070592, result);
    CORRADE_COMPARE(size, 4);
    CORRADE_COMPARE((Containers::StringView{result, size}), "\xf4\x85\x98\x80");
}

void UnicodeTest::utf32utf8Error() {
    /* Codepoint outside of the range */
    CORRADE_VERIFY(!Unicode::utf8(1594880, nullptr));
}

#ifdef CORRADE_TARGET_WINDOWS
const Containers::StringView TextNarrow = "žluťoučký kůň\0hýždě"_s;
const Containers::ArrayView<const wchar_t> TextWide = Containers::arrayView(L"\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148\u0000h\u00fd\u017ed\u011b").exceptSuffix(1);

void UnicodeTest::widen() {
    Containers::Array<wchar_t> a = Unicode::widen(TextNarrow);
    CORRADE_COMPARE_AS(a, TextWide,
        TestSuite::Compare::Container);
    /* There should be an explicit null terminator. Not using operator[]() but
       rather raw memory access, since operator[]() has a range-checking debug
       assertion. */
    CORRADE_COMPARE(a.data()[a.size()], 0);

    /* With implicit size gets cut off after the first \0 */
    Containers::Array<wchar_t> b = Unicode::widen(TextNarrow.data());
    CORRADE_COMPARE_AS(b, TextWide.prefix(13),
        TestSuite::Compare::Container);
    /* There should be an explicit null terminator. Not using operator[]() but
       rather raw memory access, since operator[]() has a range-checking debug
       assertion. */
    CORRADE_COMPARE(b.data()[b.size()], 0);
}

void UnicodeTest::widenEmpty() {
    Containers::Array<wchar_t> a = Unicode::widen(Containers::StringView{});
    CORRADE_COMPARE_AS(a,
        Containers::ArrayView<const wchar_t>{},
        TestSuite::Compare::Container);
    /* There should be an explicit null terminator. Not using operator[]() but
       rather raw memory access, since operator[]() has a range-checking debug
       assertion. */
    CORRADE_VERIFY(a.data());
    CORRADE_COMPARE(a.data()[0], 0);

    /* With implicit size */
    Containers::Array<wchar_t> b = Unicode::widen("");
    CORRADE_COMPARE_AS(b,
        Containers::ArrayView<const wchar_t>{},
        TestSuite::Compare::Container);
    /* There should be an explicit null terminator. Not using operator[]() but
       rather raw memory access, since operator[]() has a range-checking debug
       assertion. */
    CORRADE_VERIFY(b.data());
    CORRADE_COMPARE(b.data()[0], 0);
}

void UnicodeTest::narrow() {
    CORRADE_COMPARE(Unicode::narrow(TextWide), TextNarrow);
    /* Containers::String takes care of null termination, no need to test */

    /* With implicit size gets cut off after the first \0 */
    CORRADE_COMPARE(Unicode::narrow(TextWide.data()), TextNarrow.prefix(19));
}

void UnicodeTest::narrowEmpty() {
    CORRADE_COMPARE(Unicode::narrow(Containers::ArrayView<const wchar_t>{}), "");
    /* Containers::String takes care of null termination, no need to test */

    /* With implicit size */
    CORRADE_COMPARE(Unicode::narrow(L""), "");
}
#endif

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::UnicodeTest)
