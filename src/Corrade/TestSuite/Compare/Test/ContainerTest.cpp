/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

class ContainerTest: public Tester {
    public:
        ContainerTest();

        void same();
        void outputActualSmaller();
        void outputExpectedSmaller();
        void output();
        void sorted();
};

ContainerTest::ContainerTest() {
    addTests({&ContainerTest::same,
              &ContainerTest::outputActualSmaller,
              &ContainerTest::outputExpectedSmaller,
              &ContainerTest::output,
              &ContainerTest::sorted});
}

void ContainerTest::same() {
    std::vector<int> a{1, 2, 3, 4};
    CORRADE_VERIFY(Comparator<Compare::Container<std::vector<int>>>()(a, a));
}

void ContainerTest::outputActualSmaller() {
    std::stringstream out;

    std::vector<int> a{1, 2, 3};
    std::vector<int> b{1, 2, 3, 4};

    {
        Error e(&out);
        Comparator<Compare::Container<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different size, actual 3 but 4 expected. Expected has 4 on position 3.\n");
}

void ContainerTest::outputExpectedSmaller() {
    std::stringstream out;

    std::vector<int> a{1, 2, 3, 4};
    std::vector<int> b{1, 2, 3};

    {
        Error e(&out);
        Comparator<Compare::Container<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different size, actual 4 but 3 expected. Actual has 4 on position 3.\n");
}

void ContainerTest::output() {
    std::stringstream out;

    std::vector<int> a{1, 9, 3, 4};
    std::vector<int> b{1, 2, 3, 4};

    {
        Error e(&out);
        Comparator<Compare::Container<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different contents. Actual 9 but 2 expected on position 1.\n");
}

void ContainerTest::sorted() {
    std::vector<int> a{1, 2, 4, 3};
    std::vector<int> b{1, 4, 3, 2};
    std::vector<int> c{1, 4, 3, 3};

    CORRADE_VERIFY((Comparator<Compare::SortedContainer<std::vector<int>>>()(a, b)));
    CORRADE_VERIFY((Comparator<Compare::SortedContainer<std::vector<int>>>()(b, a)));
    CORRADE_VERIFY((!Comparator<Compare::SortedContainer<std::vector<int>>>()(a, c)));
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::ContainerTest)
