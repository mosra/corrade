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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Implementation/Diff.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct DiffTest: TestSuite::Tester {
    explicit DiffTest();

    void longestMatchingSlice();
    void matchingSlicesInto();
};

DiffTest::DiffTest() {
    addTests({&DiffTest::longestMatchingSlice,
              &DiffTest::matchingSlicesInto});
}

void DiffTest::longestMatchingSlice() {
    typedef Containers::Triple<std::size_t, std::size_t, size_t> Triple;

    /* Empty inputs */
    {
        int a[]{3};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(nullptr, nullptr),
            Triple{});
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, nullptr),
            Triple{});
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(nullptr, a),
            Triple{});

    /* No match */
    } {
        int a[]{3, 17, 5};
        int b[]{26, 4, 13};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            Triple{});
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            Triple{});

    /* Full match */
    }{
        int a[]{17, 3, 5, -1776};
        int b[]{17, 3, 5, -1776};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            (Triple{0, 0, 4}));
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            (Triple{0, 0, 4}));

    /* Prefix match */
    } {
        int a[]{17, 3, 5, -1776};
        int b[]{17, 3, 5};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            (Triple{0, 0, 3}));
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            (Triple{0, 0, 3}));

    /* Suffix match */
    } {
        int a[]{17, 3, 5, -1776};
        int b[]{3, 5, -1776};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            (Triple{1, 0, 3}));
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            (Triple{0, 1, 3}));

    /* Partial match from both */
    } {
        int a[]{17, 3, 21, 5, -1776, 24};
        int b[]{26, 5, -1776, 22};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            (Triple{3, 1, 2}));
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            (Triple{1, 3, 2}));

    /* Multiple matches */
    } {
        int a[]{17, 3, 0, 5, -1776, 24, -1776, 8, 26, 5, -1776, 22, 26, 5, 23};
        int b[]{22, 26, 5, -1776, 22};
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(a, b),
            (Triple{8, 1, 4}));
        CORRADE_COMPARE(Implementation::longestMatchingSlice<int>(b, a),
            (Triple{1, 8, 4}));
    }
}

void DiffTest::matchingSlicesInto() {
    typedef Containers::Triple<std::size_t, std::size_t, size_t> Triple;

    /*      0  1   2  3  4               5   6          7  8  9  10  11 */
    int a[]{0, 1, 56, 2, 3,             23, 11,         7, 8, 9, 11, 12};
    int b[]{   1,     2, 3, 4, 5, 7, 8,         10, 12, 7, 8, 9,     12, 23};
    /*         0      1  2  3  4  5  6           7   8  9 10 11      12 */

    /* With the arguments swapped it should just swap first() and second() */
    {
        Containers::Array<Triple> out;
        Implementation::matchingSlicesInto<int>(out, a, 0, b, 0);
        CORRADE_COMPARE_AS(out, Containers::arrayView<Triple>({
            {1, 0, 1},
            {3, 1, 2},
            {7, 9, 3},
            {11, 12, 1}
        }), TestSuite::Compare::Container);
    } {
        Containers::Array<Triple> out;
        Implementation::matchingSlicesInto<int>(out, b, 0, a, 0);
        CORRADE_COMPARE_AS(out, Containers::arrayView<Triple>({
            {0, 1, 1},
            {1, 3, 2},
            {9, 7, 3},
            {12, 11, 1}
        }), TestSuite::Compare::Container);
    }
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::DiffTest)
