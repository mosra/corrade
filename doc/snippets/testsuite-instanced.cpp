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

#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/StlMath.h>

using namespace Corrade;

using std::round;

namespace {

/** [0] */
struct RoundTest: TestSuite::Tester {
    explicit RoundTest();

    void test();
};

enum: std::size_t { RoundDataCount = 5 };

constexpr const struct {
    const char* name;
    float input;
    float expected;
} RoundData[RoundDataCount] {
    {"positive down", 3.3f, 3.0f},
    {"positive up", 3.5f, 4.0f},
    {"zero", 0.0f, 0.0f},
    {"negative down", -3.5f, -4.0f},
    {"negative up", -3.3f, -3.0f}
};

RoundTest::RoundTest() {
    addInstancedTests({&RoundTest::test}, RoundDataCount);
}

void RoundTest::test() {
    auto&& data = RoundData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_COMPARE(round(data.input), data.expected);
}
/** [0] */

}

CORRADE_TEST_MAIN(RoundTest)
