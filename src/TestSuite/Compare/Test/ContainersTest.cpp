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

#include "ContainersTest.h"

#include <sstream>

#include "TestSuite/Compare/Containers.h"

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::ContainersTest)

using namespace std;
using namespace Corrade::Utility;

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test {

ContainersTest::ContainersTest() {
    addTests(&ContainersTest::same,
             &ContainersTest::outputActualSmaller,
             &ContainersTest::outputExpectedSmaller,
             &ContainersTest::output);
}

void ContainersTest::same() {
    std::vector<int> a{1, 2, 3, 4};
    CORRADE_VERIFY(Comparator<Compare::Containers<std::vector<int>>>()(a, a));
}

void ContainersTest::outputActualSmaller() {
    stringstream out;

    std::vector<int> a{1, 2, 3};
    std::vector<int> b{1, 2, 3, 4};

    {
        Error e(&out);
        Comparator<Compare::Containers<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different size, actual 3 but 4 expected. Expected has 4 on position 3.\n");
}

void ContainersTest::outputExpectedSmaller() {
    stringstream out;

    std::vector<int> a{1, 2, 3, 4};
    std::vector<int> b{1, 2, 3};

    {
        Error e(&out);
        Comparator<Compare::Containers<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different size, actual 4 but 3 expected. Actual has 4 on position 3.\n");
}

void ContainersTest::output() {
    stringstream out;

    std::vector<int> a{1, 9, 3, 4};
    std::vector<int> b{1, 2, 3, 4};

    {
        Error e(&out);
        Comparator<Compare::Containers<std::vector<int>>> compare;
        CORRADE_VERIFY(!compare(a, b));
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Containers a and b have different contents. Actual 9 but 2 expected on position 1.\n");
}

}}}}
