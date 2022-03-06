/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void fromArray();
    void trim();
    void trimInPlace();
    void split();
    void splitMultipleCharacters();
    void partition();
    void join();
    void lowercaseUppercase();
    void lowercaseUppercaseString();
    void lowercaseUppercaseStringSmall();
    void lowercaseUppercaseStringNotOwned();
    void lowercaseUppercaseStl();

    void beginsWith();
    void beginsWithEmpty();
    #ifdef CORRADE_BUILD_DEPRECATED
    void viewBeginsWith();
    #endif
    void endsWith();
    void endsWithEmpty();
    #ifdef CORRADE_BUILD_DEPRECATED
    void viewEndsWith();
    #endif

    void stripPrefix();
    void stripPrefixInvalid();
    void stripSuffix();
    void stripSuffixInvalid();

    void replaceFirst();
    void replaceFirstNotFound();
    void replaceFirstEmptySearch();
    void replaceFirstEmptyReplace();
    void replaceAll();
    void replaceAllNotFound();
    void replaceAllEmptySearch();
    void replaceAllEmptyReplace();
    void replaceAllCycle();

    void parseNumberSequence();
    void parseNumberSequenceOverflow();
    void parseNumberSequenceError();
};

const struct {
    const char* name;
    Containers::StringView string;
    Containers::Array<std::uint32_t> expected;
} ParseNumberSequenceData[]{
    {"empty",
        "", {InPlaceInit, {}}},
    {"single number",
        "5", {InPlaceInit, {5}}},
    {"random delimiters",
        "1,3\n8 5;9", {InPlaceInit, {1, 3, 8, 5, 9}}},
    {"duplicate numbers and delimiters",
        "1,\t\v5;;7  ,9\n3\r \f5,9", {InPlaceInit, {1, 5, 7, 9, 3, 5, 9}}},
    {"delimiters at start and end",
        "\t\v;;17,34,;;;", {InPlaceInit, {17, 34}}},
    {"just delimiters",
        "\t\v;;\n, ,;;;", {InPlaceInit, {}}},
    {"range",
        "7-11", {InPlaceInit, {7, 8, 9, 10, 11}}},
    {"range start == end",
        "11-11", {InPlaceInit, {11}}},
    {"range start < end",
        "11-7", {InPlaceInit, {}}},
    {"ranges and single numbers combined",
        "3-5,2,44,789-791", {InPlaceInit, {3, 4, 5, 2, 44, 789, 790, 791}}},
    {"zeros",
        "0,0-5,0-0", {InPlaceInit, {0, 0, 1, 2, 3, 4, 5, 0}}}
};

const struct {
    const char* name;
    std::uint32_t min, max;
    Containers::StringView string;
    Containers::Array<std::uint32_t> expected;
} ParseNumberSequenceOverflowData[]{
    {"zero min and max", 0, 0,
        "1,5,7", {InPlaceInit, {}}},
    {"min > max", 7, 1,
        "1,5,7", {InPlaceInit, {}}},
    {"less than min or larger than max", 3, 50,
        "2,34,55,1,17", {InPlaceInit, {34, 17}}},
    {"parse overflow in the middle", 0, ~std::uint32_t{},
        "14,9999999999,27", {InPlaceInit, {14, 27}}},
    {"parse overflow at the end", 0, ~std::uint32_t{},
        "14,27,9999999999", {InPlaceInit, {14, 27}}},
    {"0xfffffffe", 0, ~std::uint32_t{},
        "4294967294", {InPlaceInit, {0xfffffffe}}},
    {"0xffffffff", 0, ~std::uint32_t{},
        "4294967295", {InPlaceInit, {}}},
    {"range start underflow", 3, 50,
        "17,1-5,25", {InPlaceInit, {17, 3, 4, 5, 25}}},
    {"range end underflow", 3, 50,
        "17,0-2,25", {InPlaceInit, {17, 25}}},
    {"range start overflow", 3, 50,
        "17,55-60,25", {InPlaceInit, {17, 25}}},
    {"range end overflow", 3, 50,
        "17,45-60,25", {InPlaceInit, {17, 45, 46, 47, 48, 49, 25}}},
    {"range missing start", 3, 50,
        "17,-7,25", {InPlaceInit, {17, 3, 4, 5, 6, 7, 25}}},
    {"range missing end", 3, 50,
        "17,48-,25", {InPlaceInit, {17, 48, 49, 25}}},
    {"range missing both", 40, 45,
        "43,-,41", {InPlaceInit, {43, 40, 41, 42, 43, 44, 41}}},
    {"range missing start, 0xffffffff", 0xfffffffe, ~std::uint32_t{},
        "17,-4294967295,25", {InPlaceInit, {4294967294}}},
    {"range missing end, 0xfffffffe", 0, ~std::uint32_t{},
        "17,4294967294-,25", {InPlaceInit, {17, 4294967294, 25}}},
};

StringTest::StringTest() {
    addTests({&StringTest::fromArray,
              &StringTest::trim,
              &StringTest::trimInPlace,
              &StringTest::split,
              &StringTest::splitMultipleCharacters,
              &StringTest::partition,
              &StringTest::join,
              &StringTest::lowercaseUppercase,
              &StringTest::lowercaseUppercaseString,
              &StringTest::lowercaseUppercaseStringSmall,
              &StringTest::lowercaseUppercaseStringNotOwned,
              &StringTest::lowercaseUppercaseStl,

              &StringTest::beginsWith,
              &StringTest::beginsWithEmpty,
              #ifdef CORRADE_BUILD_DEPRECATED
              &StringTest::viewBeginsWith,
              #endif
              &StringTest::endsWith,
              &StringTest::endsWithEmpty,
              #ifdef CORRADE_BUILD_DEPRECATED
              &StringTest::viewEndsWith,
              #endif

              &StringTest::stripPrefix,
              &StringTest::stripPrefixInvalid,
              &StringTest::stripSuffix,
              &StringTest::stripSuffixInvalid,

              &StringTest::replaceFirst,
              &StringTest::replaceFirstNotFound,
              &StringTest::replaceFirstEmptySearch,
              &StringTest::replaceFirstEmptyReplace,
              &StringTest::replaceAll,
              &StringTest::replaceAllNotFound,
              &StringTest::replaceAllEmptySearch,
              &StringTest::replaceAllEmptyReplace,
              &StringTest::replaceAllCycle});

    addInstancedTests({&StringTest::parseNumberSequence},
        Containers::arraySize(ParseNumberSequenceData));

    addInstancedTests({&StringTest::parseNumberSequenceOverflow},
        Containers::arraySize(ParseNumberSequenceOverflowData));

    addTests({&StringTest::parseNumberSequenceError});
}

using namespace Containers::Literals;

void StringTest::fromArray() {
    CORRADE_COMPARE(String::fromArray(nullptr), "");
    CORRADE_COMPARE(String::fromArray(nullptr, 37), "");

    CORRADE_COMPARE(String::fromArray("abc\0def"), "abc");
    CORRADE_COMPARE(String::fromArray("abc\0def", 7), std::string("abc\0def", 7));
}

void StringTest::trim() {
    /* Spaces at the end */
    CORRADE_COMPARE(String::ltrim("abc  "), "abc  ");
    CORRADE_COMPARE(String::rtrim("abc  "), "abc");

    /* Spaces at the beginning */
    CORRADE_COMPARE(String::ltrim("  abc"), "abc");
    CORRADE_COMPARE(String::rtrim("  abc"), "  abc");

    /* Spaces on both beginning and end */
    CORRADE_COMPARE(String::trim("  abc  "), "abc");

    /* No spaces */
    CORRADE_COMPARE(String::trim("abc"), "abc");

    /* All spaces */
    CORRADE_COMPARE(String::trim("\t\r\n\f\v "), "");

    /* Special characters */
    CORRADE_COMPARE(String::ltrim("oubya", "aeiyou"), "bya");
    CORRADE_COMPARE(String::rtrim("oubya", "aeiyou"), "oub");
    CORRADE_COMPARE(String::trim("oubya", "aeiyou"), "b");

    /* Special characters as a string */
    CORRADE_COMPARE(String::ltrim("oubya", std::string{"aeiyou"}), "bya");
    CORRADE_COMPARE(String::rtrim("oubya", std::string{"aeiyou"}), "oub");
    CORRADE_COMPARE(String::trim("oubya", std::string{"aeiyou"}), "b");
}

void StringTest::trimInPlace() {
    /* Spaces at the end */
    {
        std::string a = "abc  ";
        String::ltrimInPlace(a);
        CORRADE_COMPARE(a, "abc  ");
    } {
        std::string a = "abc  ";
        String::rtrimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* Spaces at the beginning */
    {
        std::string a = "  abc";
        String::ltrimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    } {
        std::string a = "  abc";
        String::rtrimInPlace(a);
        CORRADE_COMPARE(a, "  abc");
    }

    /* Spaces on both beginning and end */
    {
        std::string a = "  abc  ";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* No spaces */
    {
        std::string a = "abc";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "abc");
    }

    /* All spaces */
    {
        std::string a = "\t\r\n\f\v ";
        String::trimInPlace(a);
        CORRADE_COMPARE(a, "");
    }

    /* Special characters */
    {
        std::string a = "oubya";
        String::ltrimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "bya");
    } {
        std::string a = "oubya";
        String::rtrimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "oub");
    } {
        std::string a = "oubya";
        String::trimInPlace(a, "aeiyou");
        CORRADE_COMPARE(a, "b");
    }

    /* Special characters as a string */
    {
        std::string a = "oubya";
        String::ltrimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "bya");
    } {
        std::string a = "oubya";
        String::rtrimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "oub");
    } {
        std::string a = "oubya";
        String::trimInPlace(a, std::string{"aeiyou"});
        CORRADE_COMPARE(a, "b");
    }
}

void StringTest::split() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated.
       The explicit cast to avoid an ambiguous overload is kinda nasty, but
       since this is eventually getting deprecated, I don't care anymore. */

    /* Empty */
    CORRADE_COMPARE_AS(String::split(std::string{}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiter */
    CORRADE_COMPARE_AS(String::split(std::string{"/"}, '/'),
        (std::vector<std::string>{"", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"/"}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::split(std::string{"abcdef"}, '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"abcdef"}, '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::split(std::string{"ab/c/def"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab/c/def"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::split(std::string{"ab//c/def//"}, '/'),
        (std::vector<std::string>{"ab", "", "c", "def", "", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab//c/def//"}, '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
}

void StringTest::splitMultipleCharacters() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated.
       The explicit cast to avoid an ambiguous overload is kinda nasty, but
       since this is eventually getting deprecated, I don't care anymore. */

    const char delimiters[] = ".:;";

    /* Empty */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{}, delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{".::;"}, delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"abcdef"}, delimiters),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab:c;def"}, delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab:c;;def."}, delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Whitespace */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(std::string{"ab c  \t \ndef\r"}),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
}

void StringTest::partition() {
    /* Happy case */
    CORRADE_COMPARE_AS(String::partition("ab=c", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab=c", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c"}),
        TestSuite::Compare::Container);

    /* Two occurrences */
    CORRADE_COMPARE_AS(String::partition("ab=c=d", '='),
        (Containers::StaticArray<3, std::string>{"ab", "=", "c=d"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab=c=d", '='),
        (Containers::StaticArray<3, std::string>{"ab=c", "=", "d"}),
        TestSuite::Compare::Container);

    /* Not found */
    CORRADE_COMPARE_AS(String::partition("abc", '='),
        (Containers::StaticArray<3, std::string>{"abc", "", ""}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("abc", '='),
        (Containers::StaticArray<3, std::string>{"", "", "abc"}),
        TestSuite::Compare::Container);

    /* Empty input */
    CORRADE_COMPARE_AS(String::partition("", '='),
        (Containers::StaticArray<3, std::string>{"", "", ""}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("", '='),
        (Containers::StaticArray<3, std::string>{"", "", ""}),
        TestSuite::Compare::Container);

    /* More characters */
    CORRADE_COMPARE_AS(String::partition("ab, c, d", ", "),
        (Containers::StaticArray<3, std::string>{"ab", ", ", "c, d"}),
        TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::rpartition("ab, c, d", ", "),
        (Containers::StaticArray<3, std::string>{"ab, c", ", ", "d"}),
        TestSuite::Compare::Container);
}

void StringTest::join() {
    /* Empty */
    CORRADE_COMPARE(String::join({}, '/'), "");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({}, '/'), "");

    /* One empty value */
    CORRADE_COMPARE(String::join({""}, '/'), "");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({""}, '/'), "");

    /* Two empty values */
    CORRADE_COMPARE(String::join({"", ""}, '/'),
        "/");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"", ""}, '/'),
        "");

    /* One value */
    CORRADE_COMPARE(String::join({"abcdef"}, '/'),
        "abcdef");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"abcdef"}, '/'),
        "abcdef");

    /* Common case, also multi-character and std::string joiner */
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, '/'),
        "ab/c/def");
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, ", "),
        "ab, c, def");
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, std::string{", "}),
        "ab, c, def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, '/'),
        "ab/c/def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, ", "),
        "ab, c, def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, std::string{", "}),
        "ab, c, def");

    /* Empty parts */
    CORRADE_COMPARE(String::join({"ab", "", "c", "def", "", ""}, '/'),
        "ab//c/def//");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "", "c", "def", "", ""}, '/'),
        "ab/c/def");
}

void StringTest::lowercaseUppercase() {
    /* Because the conversion is done using a bit operation on a range, check
       that the conversion is done on all characters and there's no off-by-one
       error at the bounds */
    {
        Containers::StringView lowercase = "`abcdefghijklmnopqrstuvwxyz{";
        Containers::StringView uppercase = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[";
        CORRADE_COMPARE(lowercase.size(), uppercase.size());
        for(std::size_t i = 0; i != lowercase.size() - 1; ++i) {
            CORRADE_ITERATION(i, lowercase[i], uppercase[i]);
            /* The tested range should be contiguous */
            CORRADE_COMPARE(lowercase[i] + 1, lowercase[i + 1]);
            CORRADE_COMPARE(uppercase[i] + 1, uppercase[i + 1]);
        }

        /* The conversion should NOT change the non-alpha characters before/
           after! Have two checks for this to reduce the possibility of someone
           "cleaning this up" in the future. */
        CORRADE_COMPARE(String::uppercase(lowercase), "`ABCDEFGHIJKLMNOPQRSTUVWXYZ{");
        CORRADE_COMPARE(String::lowercase(uppercase), "@abcdefghijklmnopqrstuvwxyz[");
        CORRADE_VERIFY(String::uppercase(lowercase) != uppercase);
        CORRADE_VERIFY(String::uppercase(uppercase) != lowercase);

    /* No-op */
    } {
        CORRADE_COMPARE(String::lowercase("hello"_s), "hello");
        CORRADE_COMPARE(String::uppercase("YEAH"_s), "YEAH");

    /* Lowercase / uppercase */
    } {
        CORRADE_COMPARE(String::lowercase("YEAh!"_s), "yeah!");
        CORRADE_COMPARE(String::uppercase("Hello!"_s), "HELLO!");

    /* Special chars */
    } {
        Containers::StringView a = ".,?- \"!/(98765%";
        CORRADE_COMPARE(String::lowercase(a), a);
        CORRADE_COMPARE(String::uppercase(a), a);

    /* UTF-8 deliberately not changed in any way */
    } {
        CORRADE_COMPARE(String::lowercase("HÝŽDĚ"_s), "hÝŽdĚ");
        CORRADE_COMPARE(String::uppercase("hýždě"_s), "HýžDě");

    /* In-place. These are called from the copying functions so just verify
       they're exported and callable, everything else is already tested
       above */
    } {
        Containers::String yeah = "YEAh!"_s;
        String::lowercaseInPlace(yeah);
        CORRADE_COMPARE(yeah, "yeah!");

        Containers::String hello = "Hello!"_s;
        String::uppercaseInPlace(hello);
        CORRADE_COMPARE(hello, "HELLO!");
    }
}

void StringTest::lowercaseUppercaseString() {
    /* It should just operate in-place, not allocate a copy */

    {
        Containers::String in{Containers::AllocatedInit, "YEAh!"};
        const char* data = in.data();
        Containers::String out = String::lowercase(std::move(in));
        CORRADE_COMPARE(out, "yeah!");
        CORRADE_VERIFY(out.data() == data);
    } {
        Containers::String in{Containers::AllocatedInit, "Hello!"};
        const char* data = in.data();
        Containers::String out = String::uppercase(std::move(in));
        CORRADE_COMPARE(out, "HELLO!");
        CORRADE_VERIFY(out.data() == data);
    }
}

void StringTest::lowercaseUppercaseStringSmall() {
    /* For SSO there's no allocation to preserve, so just check that it works */

    {
        Containers::String string{"YEAh!"};
        CORRADE_VERIFY(string.isSmall());
        CORRADE_COMPARE(String::lowercase(string), "yeah!");
    } {
        Containers::String string{"Hello!"};        CORRADE_VERIFY(string.isSmall());
        CORRADE_COMPARE(String::uppercase(string), "HELLO!");
    }
}

void StringTest::lowercaseUppercaseStringNotOwned() {
    /* Will make a copy as it can't touch a potentially immutable data */

    {
        const char* data = "YEAh!";
        Containers::String in = Containers::String::nullTerminatedView(data);
        CORRADE_VERIFY(!in.isSmall());
        CORRADE_VERIFY(in.deleter());

        Containers::String out = String::lowercase(std::move(in));
        CORRADE_COMPARE(out, "yeah!");
        CORRADE_VERIFY(out.data() != data);
    } {
        const char* data = "Hello!";
        Containers::String in = Containers::String::nullTerminatedView(data);
        CORRADE_VERIFY(!in.isSmall());
        CORRADE_VERIFY(in.deleter());

        Containers::String out = String::uppercase(std::move(in));
        CORRADE_COMPARE(out, "HELLO!");
        CORRADE_VERIFY(out.data() != data);
    }
}

void StringTest::lowercaseUppercaseStl() {
    /* These just call into the in-place implementations tested above, so
       verify just basic functionality */
    CORRADE_COMPARE(String::lowercase(std::string{"YEAh!"}), "yeah!");
    CORRADE_COMPARE(String::uppercase(std::string{"Hello!"}), "HELLO!");
}

void StringTest::beginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(String::beginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", std::string{"over"}));

    CORRADE_VERIFY(!String::beginsWith("overcomplicated", "oven"));
    CORRADE_VERIFY(!String::beginsWith("overcomplicated", std::string{"oven"}));

    CORRADE_VERIFY(String::beginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::beginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::beginsWith("", 'h'));
}

void StringTest::beginsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(!String::beginsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::beginsWith("", ""));
}

#ifdef CORRADE_BUILD_DEPRECATED
void StringTest::viewBeginsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_IGNORE_DEPRECATED_PUSH
    CORRADE_VERIFY(String::viewBeginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(!String::viewBeginsWith("overcomplicated", "oven"));

    CORRADE_VERIFY(String::viewBeginsWith("hello", 'h'));
    CORRADE_VERIFY(!String::viewBeginsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewBeginsWith("", 'h'));
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif

void StringTest::endsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(String::endsWith("overcomplicated", "complicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", std::string{"complicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "somplicated"));
    CORRADE_VERIFY(!String::endsWith("overcomplicated", std::string{"somplicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "overcomplicated even more"));

    CORRADE_VERIFY(!String::endsWith("hello", 'h'));
    CORRADE_VERIFY(String::endsWith("hello", 'o'));
    CORRADE_VERIFY(!String::endsWith("", 'h'));
}

void StringTest::endsWithEmpty() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_VERIFY(!String::endsWith("", "overcomplicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", ""));
    CORRADE_VERIFY(String::endsWith("", ""));
}

#ifdef CORRADE_BUILD_DEPRECATED
void StringTest::viewEndsWith() {
    /* These delegate into the StringView implementation and the tests are
       kept just for archival purposes, until the whole thing is deprecated. */

    CORRADE_IGNORE_DEPRECATED_PUSH
    CORRADE_VERIFY(String::viewEndsWith({"overcomplicated", 15}, "complicated"));
    CORRADE_VERIFY(!String::viewEndsWith("overcomplicated", "complicated"));

    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "somplicated"));
    CORRADE_VERIFY(!String::viewEndsWith({"overcomplicated", 15}, "overcomplicated even more"));

    CORRADE_VERIFY(!String::viewEndsWith({"hello", 5}, 'h'));
    CORRADE_VERIFY(String::viewEndsWith({"hello", 5}, 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("hello", 'o'));
    CORRADE_VERIFY(!String::viewEndsWith("", 'h'));
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif

void StringTest::stripPrefix() {
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", "over"), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", std::string{"over"}), "complicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", 'o'), "vercomplicated");
    CORRADE_COMPARE(String::stripPrefix("overcomplicated", ""), "overcomplicated");
}

void StringTest::stripPrefixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectOutput{&out};
    String::stripPrefix("overcomplicated", "complicated");
    CORRADE_COMPARE(out.str(), "Utility::String::stripPrefix(): string doesn't begin with given prefix\n");
}

void StringTest::stripSuffix() {
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", "complicated"), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", std::string{"complicated"}), "over");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", 'd'), "overcomplicate");
    CORRADE_COMPARE(String::stripSuffix("overcomplicated", ""), "overcomplicated");
}

void StringTest::stripSuffixInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectOutput{&out};
    String::stripSuffix("overcomplicated", "over");
    CORRADE_COMPARE(out.str(), "Utility::String::stripSuffix(): string doesn't end with given suffix\n");
}

void StringTest::replaceFirst() {
    CORRADE_COMPARE(String::replaceFirst(
        "this part will get replaced and this will get not",
        "will get", "got"),
        "this part got replaced and this will get not");
    CORRADE_COMPARE(String::replaceFirst(
        "this part will get replaced and this will get not",
        "will get", std::string{"got"}),
        "this part got replaced and this will get not");
    CORRADE_COMPARE(String::replaceFirst(
        "this part will get replaced and this will get not",
        std::string{"will get"}, "got"),
        "this part got replaced and this will get not");
    CORRADE_COMPARE(String::replaceFirst(
        "this part will get replaced and this will get not",
        std::string{"will get"}, std::string{"got"}),
        "this part got replaced and this will get not");
}

void StringTest::replaceFirstNotFound() {
    CORRADE_COMPARE(String::replaceFirst("this part will not get replaced",
        "will get", "got"), "this part will not get replaced");
}

void StringTest::replaceFirstEmptySearch() {
    CORRADE_COMPARE(String::replaceFirst("this completely messed up",
        "", "got "), "got this completely messed up");
}

void StringTest::replaceFirstEmptyReplace() {
    CORRADE_COMPARE(String::replaceFirst("this completely messed up",
        "completely ", ""), "this messed up");
}

void StringTest::replaceAll() {
    CORRADE_COMPARE(String::replaceAll(
        "this part will get replaced and this will get replaced also",
        "will get", "got"),
        "this part got replaced and this got replaced also");
    CORRADE_COMPARE(String::replaceAll(
        "this part will get replaced and this will get replaced also",
        "will get", std::string{"got"}),
        "this part got replaced and this got replaced also");
    CORRADE_COMPARE(String::replaceAll(
        "this part will get replaced and this will get replaced also",
        std::string{"will get"}, "got"),
        "this part got replaced and this got replaced also");
    CORRADE_COMPARE(String::replaceAll(
        "this part will get replaced and this will get replaced also",
        std::string{"will get"}, std::string{"got"}),
        "this part got replaced and this got replaced also");
}

void StringTest::replaceAllNotFound() {
    CORRADE_COMPARE(String::replaceAll("this part will not get replaced",
        "will get", "got"), "this part will not get replaced");
}

void StringTest::replaceAllEmptySearch() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectOutput{&out};
    String::replaceAll("this completely messed up", "", "got ");
    CORRADE_COMPARE(out.str(), "Utility::String::replaceAll(): empty search string would cause an infinite loop\n");
}

void StringTest::replaceAllEmptyReplace() {
    CORRADE_COMPARE(String::replaceAll("lalalalala!",
        "la", ""), "!");
}

void StringTest::replaceAllCycle() {
    CORRADE_COMPARE(String::replaceAll("lalala",
        "la", "lala"), "lalalalalala");
}

void StringTest::parseNumberSequence() {
    auto&& data = ParseNumberSequenceData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Containers::Array<std::uint32_t>> out = String::parseNumberSequence(data.string, 0, ~std::uint32_t{});
    CORRADE_VERIFY(out);
    CORRADE_COMPARE_AS(*out, data.expected, TestSuite::Compare::Container);
}

void StringTest::parseNumberSequenceOverflow() {
    auto&& data = ParseNumberSequenceOverflowData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Optional<Containers::Array<std::uint32_t>> out = String::parseNumberSequence(data.string, data.min, data.max);
    CORRADE_VERIFY(out);
    CORRADE_COMPARE_AS(*out, data.expected, TestSuite::Compare::Container);
}

void StringTest::parseNumberSequenceError() {
    std::ostringstream out;
    Error redirectError{&out};
    String::parseNumberSequence("3,5y7,x,25", 0, ~std::uint32_t{});
    CORRADE_COMPARE(out.str(), "Utility::parseNumberSequence(): unrecognized character y in 3,5y7,x,25\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
