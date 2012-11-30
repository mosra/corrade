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

#include "ContainerTest.h"

#include <sstream>

#include "TestSuite/Compare/Container.h"

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::ContainerTest)

using namespace std;
using namespace Corrade::Utility;

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

ContainerTest::ContainerTest() {
    addTests(&ContainerTest::same,
             &ContainerTest::outputActualSmaller,
             &ContainerTest::outputExpectedSmaller,
             &ContainerTest::output,
             &ContainerTest::sorted);
}

void ContainerTest::same() {
    std::vector<int> a{1, 2, 3, 4};
    CORRADE_VERIFY(Comparator<Compare::Container<std::vector<int>>>()(a, a));
}

void ContainerTest::outputActualSmaller() {
    stringstream out;

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
    stringstream out;

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
    stringstream out;

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

    CORRADE_VERIFY(Comparator<Compare::Container<std::vector<int>>>(Compare::ContainerMethod::Sorted)(a, b));
    CORRADE_VERIFY(Comparator<Compare::Container<std::vector<int>>>(Compare::ContainerMethod::Sorted)(b, a));
    CORRADE_VERIFY(!Comparator<Compare::Container<std::vector<int>>>(Compare::ContainerMethod::Sorted)(a, c));
}

}}}}
