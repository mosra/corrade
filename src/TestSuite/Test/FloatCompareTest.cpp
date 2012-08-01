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

#include "FloatCompareTest.h"

#include <limits>

using namespace std;

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::FloatCompareTest)

namespace Corrade { namespace TestSuite { namespace Test {

FloatCompareTest::FloatCompareTest() {
    addTests(&FloatCompareTest::smallDelta,
             &FloatCompareTest::largeDelta,
             &FloatCompareTest::nan,
             &FloatCompareTest::infinity);
}

void FloatCompareTest::smallDelta() {
    CORRADE_VERIFY(Compare<float>()(3.2021220f,
                                    3.2021225f));
    CORRADE_VERIFY(Compare<double>()(3.2021222324250,
                                     3.2021222324255));
}

void FloatCompareTest::largeDelta() {
    CORRADE_VERIFY(!Compare<float>()(3.202120f,
                                     3.202125f));
    CORRADE_VERIFY(!Compare<double>()(3.202122232420,
                                      3.202122232425));
}

void FloatCompareTest::nan() {
    CORRADE_VERIFY(Compare<float>()(numeric_limits<float>::quiet_NaN(),
                                    numeric_limits<float>::quiet_NaN()));
    CORRADE_VERIFY(!Compare<float>()(numeric_limits<float>::quiet_NaN(), 0));
    CORRADE_VERIFY(!Compare<float>()(0, numeric_limits<float>::quiet_NaN()));
}

void FloatCompareTest::infinity() {
    CORRADE_VERIFY(Compare<float>()(numeric_limits<float>::infinity(),
                                    numeric_limits<float>::infinity()));
    CORRADE_VERIFY(Compare<float>()(-numeric_limits<float>::infinity(),
                                    -numeric_limits<float>::infinity()));
    CORRADE_VERIFY(!Compare<float>()(numeric_limits<float>::quiet_NaN(),
                                     numeric_limits<float>::infinity()));
}

}}}
