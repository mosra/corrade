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
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/StlMath.h" /* NAN */

namespace Corrade { namespace Utility { namespace Test { namespace {

struct MathTest: TestSuite::Tester {
    explicit MathTest();

    void minMax();
    void minMaxNanPropagation();
};

MathTest::MathTest() {
    addTests({&MathTest::minMax,
              &MathTest::minMaxNanPropagation});
}

void MathTest::minMax() {
    CORRADE_COMPARE(Utility::min(5, 9), 5);
    CORRADE_COMPARE(Utility::min(9, 5), 5);
    CORRADE_COMPARE(Utility::max(5, 9), 9);
    CORRADE_COMPARE(Utility::max(9, 5), 9);

    constexpr int cmin = Utility::min(5, 9);
    constexpr int cmax = Utility::max(5, 9);
    CORRADE_COMPARE(cmin, 5);
    CORRADE_COMPARE(cmax, 9);
}

void MathTest::minMaxNanPropagation() {
    /* MinGW is stupid and has NAN as a double, while it's said to be a float
       by the standard and all other compilers */
    CORRADE_COMPARE(Utility::min(float(NAN), 5.0f), float(NAN));
    CORRADE_COMPARE(Utility::max(float(NAN), 5.0f), float(NAN));
    CORRADE_COMPARE(Utility::min(5.0f, float(NAN)), 5.0f);
    CORRADE_COMPARE(Utility::max(5.0f, float(NAN)), 5.0f);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::MathTest)
