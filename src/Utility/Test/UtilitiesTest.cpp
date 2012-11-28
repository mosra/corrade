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

#include "UtilitiesTest.h"

#include "Utility/utilities.h"

CORRADE_TEST_MAIN(Corrade::Utility::Test::UtilitiesTest)

namespace Corrade { namespace Utility { namespace Test {

UtilitiesTest::UtilitiesTest() {
    addTests(&UtilitiesTest::pow2,
             &UtilitiesTest::log2,
             &UtilitiesTest::trim,
             &UtilitiesTest::split,
             &UtilitiesTest::lowercase);
}

void UtilitiesTest::pow2() {
    CORRADE_COMPARE(Utility::pow2(10), 1024);
}

void UtilitiesTest::log2() {
    CORRADE_COMPARE(Utility::log2(2153), 11);
}

void UtilitiesTest::trim() {
    /* Spaces at the end */
    CORRADE_COMPARE(Utility::trim("abc  "), "abc");

    /* Spaces at the beginning */
    CORRADE_COMPARE(Utility::trim("  abc"), "abc");

    /* Spaces on both beginning and end */
    CORRADE_COMPARE(Utility::trim("  abc  "), "abc");

    /* No spaces */
    CORRADE_COMPARE(Utility::trim("abc"), "abc");

    /* All spaces */
    CORRADE_COMPARE(Utility::trim("\t\r\n\f\v "), "");
}

void UtilitiesTest::split() {
    /* No delimiters */
    CORRADE_COMPARE(Utility::split("abcdef", '/'),
                    std::vector<std::string>{"abcdef"});

    /* Common case */
    CORRADE_COMPARE(Utility::split("ab/c/def", '/'),
                    (std::vector<std::string>{"ab", "c", "def"}));

    /* Empty parts */
    CORRADE_COMPARE(Utility::split("ab//c/def//", '/'),
                    (std::vector<std::string>{"ab", "", "c", "def", "", ""}));

    /* Skip empty parts */
    CORRADE_COMPARE(Utility::split("ab//c/def//", '/', false),
                    (std::vector<std::string>{"ab", "c", "def"}));
}

void UtilitiesTest::lowercase() {
    /* Lowecase */
    CORRADE_COMPARE(Utility::lowercase("hello"), "hello");

    /* Uppercase */
    CORRADE_COMPARE(Utility::lowercase("QWERTZUIOP"), "qwertzuiop");

    /* Special chars */
    CORRADE_COMPARE(Utility::lowercase(".,?- \"!/(98765%"), ".,?- \"!/(98765%");

    /* UTF-8 */
    CORRADE_EXPECT_FAIL("UTF-8 lowercasing is not supported.");
    CORRADE_COMPARE(Utility::lowercase("ĚŠČŘŽÝÁÍÉÚŮĎŤŇ"), "ěščřžýáíéúůďťň");
}

}}}
