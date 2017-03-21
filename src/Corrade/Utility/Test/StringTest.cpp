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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility { namespace Test {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void fromArray();
    void trim();
    void split();
    void splitMultipleCharacters();
    void join();
    void lowercase();
    void uppercase();

    void beginsWith();
    void endsWith();
};

StringTest::StringTest() {
    addTests({&StringTest::fromArray,
              &StringTest::trim,
              &StringTest::split,
              &StringTest::splitMultipleCharacters,
              &StringTest::join,
              &StringTest::lowercase,
              &StringTest::uppercase,

              &StringTest::beginsWith,
              &StringTest::endsWith});
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
    CORRADE_COMPARE(String::trim("ouya", "aeiyou"), "");
}

void StringTest::split() {
    /* Empty */
    CORRADE_COMPARE_AS(String::split({}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts({}, '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiter */
    CORRADE_COMPARE_AS(String::split("/", '/'),
        (std::vector<std::string>{"", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("/", '/'),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::split("abcdef", '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("abcdef", '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::split("ab/c/def", '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("ab/c/def", '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::split("ab//c/def//", '/'),
        (std::vector<std::string>{"ab", "", "c", "def", "", ""}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("ab//c/def//", '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
}

void StringTest::splitMultipleCharacters() {
    const std::string delimiters = ".:;";

    /* Empty */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts({}, delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* Only delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts(".::;", delimiters),
        std::vector<std::string>{}, TestSuite::Compare::Container);

    /* No delimiters */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("abcdef", delimiters),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("ab:c;def", delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::splitWithoutEmptyParts("ab:c;;def.", delimiters),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
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

    /* Common case */
    CORRADE_COMPARE(String::join({"ab", "c", "def"}, '/'),
        "ab/c/def");
    CORRADE_COMPARE(String::joinWithoutEmptyParts({"ab", "c", "def"}, '/'),
        "ab/c/def");

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
    CORRADE_VERIFY(String::beginsWith("overcomplicated", "over"));
    CORRADE_VERIFY(String::beginsWith("overcomplicated", std::string{"over"}));

    CORRADE_VERIFY(!String::beginsWith("overcomplicated", "oven"));
    CORRADE_VERIFY(!String::beginsWith("overcomplicated", std::string{"oven"}));
}

void StringTest::endsWith() {
    CORRADE_VERIFY(String::endsWith("overcomplicated", "complicated"));
    CORRADE_VERIFY(String::endsWith("overcomplicated", std::string{"complicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "somplicated"));
    CORRADE_VERIFY(!String::endsWith("overcomplicated", std::string{"somplicated"}));

    CORRADE_VERIFY(!String::endsWith("overcomplicated", "overcomplicated even more"));
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
