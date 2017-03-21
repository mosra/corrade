/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include <limits>
#include <sstream>

#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace TestSuite { namespace Test {

struct FloatingPointTest: Tester {
    explicit FloatingPointTest();

    void smallDelta();
    void largeDelta();
    void nan();
    void infinity();
    void output();
};

FloatingPointTest::FloatingPointTest() {
    addTests({&FloatingPointTest::smallDelta,
              &FloatingPointTest::largeDelta,
              &FloatingPointTest::nan,
              &FloatingPointTest::infinity,
              &FloatingPointTest::output});
}

void FloatingPointTest::smallDelta() {
    CORRADE_VERIFY(Comparator<float>()(3.202122f,
                                       3.202123f));
    CORRADE_VERIFY(Comparator<double>()(3.202122232425,
                                        3.202122232426));
}

void FloatingPointTest::largeDelta() {
    CORRADE_VERIFY(!Comparator<float>()(3.20212f,
                                        3.20213f));
    CORRADE_VERIFY(!Comparator<double>()(3.20212223242,
                                         3.20212223243));
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

    CORRADE_COMPARE(out.str(), "Floating-point values a and b are not the same, actual 3 but 8 expected (delta -5).\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::FloatingPointTest)
