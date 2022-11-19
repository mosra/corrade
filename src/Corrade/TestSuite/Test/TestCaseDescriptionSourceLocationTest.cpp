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

#include <iostream>
#include <sstream>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace TestSuite { namespace Test { namespace {

using namespace Containers::Literals;

struct Test: Tester {
    explicit Test();

    /* It's separate from TesterTest because the output is compiler-dependent
       and thus would be too annoying to handle in the general test */

    void test();
    void somethingElse();
    void resetWithoutLine();
};

const struct {
    TestCaseDescriptionSourceLocation name;
    int value;
} TestData[]{
    {"three", 3},
    {"five", 5},
    {"seventy", 70}
};

Test::Test() {
    addInstancedTests({&Test::test},
        Containers::arraySize(TestData));

    addTests({&Test::somethingElse});

    addInstancedTests({&Test::resetWithoutLine},
        Containers::arraySize(TestData));
}

void Test::test() {
    auto&& data = TestData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_FAIL_IF(data.value == 5, "This message should have data location info");
}

void Test::somethingElse() {
    CORRADE_WARN("This message shouldn't have any stale info about data location");
    CORRADE_VERIFY(true);
}

void Test::resetWithoutLine() {
    auto&& data = TestData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_INFO("This message has the data location info");

    /* Now call setTestCaseDescription() again, but without the line info. The
       output shouldn't have it after anymore. */
    setTestCaseDescription(testCaseDescription());

    CORRADE_FAIL_IF(data.value == 5, "This message shouldn't have data location info anymore");
}

struct TestCaseDescriptionSourceLocationTest: Tester {
    explicit TestCaseDescriptionSourceLocationTest();

    void stringConversion();
    void test();
};

TestCaseDescriptionSourceLocationTest::TestCaseDescriptionSourceLocationTest() {
    addTests({&TestCaseDescriptionSourceLocationTest::stringConversion,
              &TestCaseDescriptionSourceLocationTest::test});
}

void TestCaseDescriptionSourceLocationTest::stringConversion() {
    TestCaseDescriptionSourceLocation a = "yello\0world"_s;

    Containers::StringView b = a;
    CORRADE_COMPARE(b, "yello\0world"_s);
    CORRADE_COMPARE(b.size(), 11);
    CORRADE_COMPARE(b.flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);
}

void TestCaseDescriptionSourceLocationTest::test() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");

    /* Print to visually verify coloring */
    {
        Debug{} << "======================== visual color verification start =======================";
        const char* argv[] = { "" };
        int argc = Containers::arraySize(argv);
        Tester::registerArguments(argc, argv);
        Test t;
        t.registerTest("here.cpp", "TestCaseDescriptionSourceLocationTest::Test");
        t.exec(this, Debug::defaultOutput(), Error::defaultOutput());
        Debug{} << "======================== visual color verification end =========================";
    }

    std::stringstream out;

    /* Disable automatic colors to ensure we have the same behavior
      everywhere */
    const char* argv[] = { "", "--color", "off" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);
    Test t;
    t.registerTest("here.cpp", "TestCaseDescriptionSourceLocationTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    #ifdef CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 12
    CORRADE_EXPECT_FAIL("GCC 11 and older reports all lines the same, pointing to the closing bracket, which is less useful.");
    #endif
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TEST_DIR, "TestCaseDescriptionSourceLocationTestFiles/test.txt"),
        Compare::StringToFile);
    #else
    CORRADE_INFO("CORRADE_SOURCE_LOCATION_BUILTINS_SUPPORTED not available");
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TEST_DIR, "TestCaseDescriptionSourceLocationTestFiles/noSupport.txt"),
        Compare::StringToFile);
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TestCaseDescriptionSourceLocationTest)
