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

#include <vector>

#include "Corrade/Containers/Array.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/SortedContainer.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct SortedContainerTest: Tester {
    explicit SortedContainerTest();

    void test();
};

SortedContainerTest::SortedContainerTest() {
    addTests({&SortedContainerTest::test});
}

/* The rest is tested in ContainerTest, this is just a derived class */

void SortedContainerTest::test() {
    std::vector<int> a{1, 2, 4, 3};
    std::vector<int> b{1, 4, 3, 2};
    std::vector<int> c{1, 4, 3, 3};

    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>()(a, b)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>()(b, a)), ComparisonStatusFlags{});
    CORRADE_COMPARE((Comparator<Compare::SortedContainer<std::vector<int>>>()(a, c)), ComparisonStatusFlag::Failed);
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::SortedContainerTest)
