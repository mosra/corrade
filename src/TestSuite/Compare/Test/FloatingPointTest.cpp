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

#include "FloatingPointTest.h"

#include <limits>
#include <sstream>

using namespace Corrade::Utility;

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::FloatingPointTest)

namespace Corrade { namespace TestSuite { namespace Test {

FloatingPointTest::FloatingPointTest() {
    addTests(&FloatingPointTest::smallDelta,
             &FloatingPointTest::largeDelta,
             &FloatingPointTest::nan,
             &FloatingPointTest::infinity,
             &FloatingPointTest::output);
}

void FloatingPointTest::smallDelta() {
    CORRADE_VERIFY(Comparator<float>()(3.2021220f,
                                       3.2021225f));
    CORRADE_VERIFY(Comparator<double>()(3.2021222324250,
                                        3.2021222324255));
}

void FloatingPointTest::largeDelta() {
    CORRADE_VERIFY(!Comparator<float>()(3.202120f,
                                        3.202125f));
    CORRADE_VERIFY(!Comparator<double>()(3.202122232420,
                                         3.202122232425));
}

void FloatingPointTest::nan() {
    CORRADE_VERIFY(Comparator<float>()(std::numeric_limits<float>::quiet_NaN(),
                                       std::numeric_limits<float>::quiet_NaN()));
    CORRADE_VERIFY(!Comparator<float>()(std::numeric_limits<float>::quiet_NaN(), 0));
    CORRADE_VERIFY(!Comparator<float>()(0, std::numeric_limits<float>::quiet_NaN()));
}

void FloatingPointTest::infinity() {
    CORRADE_VERIFY(Comparator<float>()(std::numeric_limits<float>::infinity(),
                                       std::numeric_limits<float>::infinity()));
    CORRADE_VERIFY(Comparator<float>()(-std::numeric_limits<float>::infinity(),
                                       -std::numeric_limits<float>::infinity()));
    CORRADE_VERIFY(!Comparator<float>()(std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::infinity()));
}

void FloatingPointTest::output() {
    std::stringstream out;

    {
        Error e(&out);
        Comparator<float> compare;
        compare(3.0f, 8.0f);
        compare.printErrorMessage(e, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "Floating-point values a and b are not the same, actual 3 but 8 expected (delta 5).\n");
}

}}}
