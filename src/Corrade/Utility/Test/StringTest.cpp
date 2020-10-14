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

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StringView.h"
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
    void lowercase();
    void uppercase();

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
};

StringTest::StringTest() {
    addTests({&StringTest::fromArray,
              &StringTest::trim,
              &StringTest::trimInPlace,
              &StringTest::split,
              &StringTest::splitMultipleCharacters,
              &StringTest::partition,
              &StringTest::join,
              &StringTest::lowercase,
              &StringTest::uppercase,

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
}

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

    /* Two occurences */
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

void StringTest::lowercase() {
    /* Lowecase */
    CORRADE_COMPARE(String::lowercase("hello"), "hello");

    /* Uppercase */
    CORRADE_COMPARE(String::lowercase("QWERTZUIOP"), "qwertzuiop");

    /* Special chars */
    CORRADE_COMPARE(String::lowercase(".,?- \"!/(98765%"), ".,?- \"!/(98765%");

    /* UTF-8 */
    CORRADE_EXPECT_FAIL("UTF-8 lowercasing is not supported.");
    CORRADE_COMPARE(String::lowercase("ĚŠČŘŽÝÁÍÉÚŮĎŤŇ"), "ěščřžýáíéúůďťň");
}

void StringTest::uppercase() {
    /* Lowecase */
    CORRADE_COMPARE(String::uppercase("hello"), "HELLO");

    /* Uppercase */
    CORRADE_COMPARE(String::uppercase("QWERTZUIOP"), "QWERTZUIOP");

    /* Special chars */
    CORRADE_COMPARE(String::uppercase(".,?- \"!/(98765%"), ".,?- \"!/(98765%");

    /* UTF-8 */
    CORRADE_EXPECT_FAIL("UTF-8 uppercasing is not supported.");
    CORRADE_COMPARE(String::uppercase("ěščřžýáíéúůďťň"), "ĚŠČŘŽÝÁÍÉÚŮĎŤŇ");
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

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
