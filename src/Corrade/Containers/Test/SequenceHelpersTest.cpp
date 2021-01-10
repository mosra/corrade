/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include "Corrade/Containers/sequenceHelpers.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct SequenceHelpersTest: TestSuite::Tester {
    explicit SequenceHelpersTest();

    void generateSequenceEmpty();
    void generateSequenceEven();
    void generateSequenceOdd();
};

SequenceHelpersTest::SequenceHelpersTest() {
    addTests({&SequenceHelpersTest::generateSequenceEmpty,
              &SequenceHelpersTest::generateSequenceEven,
              &SequenceHelpersTest::generateSequenceOdd});

    /* Uncomment to benchmark compile times. The following command then shows
       both time and memory use:

        /usr/bin/time --verbose ninja ContainersSequenceHelpersTest
    */
    //static_cast<void>(Implementation::GenerateSequence<899>::Type{});
}

void SequenceHelpersTest::generateSequenceEmpty() {
    CORRADE_VERIFY((std::is_same<
        Implementation::GenerateSequence<0>::Type,
        Implementation::Sequence<>
    >::value));
}

void SequenceHelpersTest::generateSequenceEven() {
    CORRADE_VERIFY((std::is_same<
        Implementation::GenerateSequence<8>::Type,
        Implementation::Sequence<0, 1, 2, 3, 4, 5, 6, 7>
    >::value));
}

void SequenceHelpersTest::generateSequenceOdd() {
    CORRADE_VERIFY((std::is_same<
        Implementation::GenerateSequence<7>::Type,
        Implementation::Sequence<0, 1, 2, 3, 4, 5, 6>
    >::value));
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::SequenceHelpersTest)
