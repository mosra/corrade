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

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace TestSuite { namespace Test { namespace {

struct ArgumentsTest: TestSuite::Tester {
    explicit ArgumentsTest();

    void test();

    std::string _value;
};

ArgumentsTest::ArgumentsTest(): TestSuite::Tester{TesterConfiguration{}.setSkippedArgumentPrefixes({"arguments"})}  {
    addTests({&ArgumentsTest::test});

    Utility::Arguments args{"arguments"};
    args.addOption("value").setHelp("value", "value to pass to the test")
        .parse(arguments().first, arguments().second);

    _value = args.value("value");
}

void ArgumentsTest::test() {
    Debug{} << "This test expects that `--arguments-value hello` is passed to it";

    #ifdef CORRADE_TESTSUITE_TARGET_XCTEST
    CORRADE_EXPECT_FAIL("Not supported on Xcode XCTest.");
    #endif
    CORRADE_COMPARE(_value, "hello");
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::ArgumentsTest)
