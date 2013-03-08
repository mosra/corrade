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

#include "TestSuite/Tester.h"
#include "TestSuite/Compare/Container.h"
#include "Utility/String.h"

namespace Corrade { namespace Utility { namespace Test {

class StringTest: public TestSuite::Tester {
    public:
        StringTest();

        void trim();
        void split();
        void lowercase();
        void whitespace();
};

StringTest::StringTest() {
    addTests({&StringTest::trim,
              &StringTest::split,
              &StringTest::lowercase,
              &StringTest::whitespace});
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
    /* No delimiters */
    CORRADE_COMPARE_AS(String::split("abcdef", '/'),
        std::vector<std::string>{"abcdef"}, TestSuite::Compare::Container);

    /* Common case */
    CORRADE_COMPARE_AS(String::split("ab/c/def", '/'),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);

    /* Empty parts */
    CORRADE_COMPARE_AS(String::split("ab//c/def//", '/'),
        (std::vector<std::string>{"ab", "", "c", "def", "", ""}), TestSuite::Compare::Container);

    /* Skip empty parts */
    CORRADE_COMPARE_AS(String::split("ab//c/def//", '/', false),
        (std::vector<std::string>{"ab", "c", "def"}), TestSuite::Compare::Container);
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

void StringTest::whitespace() {
    for(char i: String::Whitespace)
        CORRADE_VERIFY(std::isspace(i));
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::StringTest)
