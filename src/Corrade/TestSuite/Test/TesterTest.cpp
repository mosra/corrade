/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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
#include <cstdlib>
#include <sstream>

#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace TestSuite {

class StringLength;

template<> class Comparator<StringLength> {
    public:
        Comparator(int epsilon = 0): epsilon(epsilon) {}

        bool operator()(const std::string& actual, const std::string& expected) {
            return std::abs(int(actual.size()) - int(expected.size())) <= epsilon;
        }

        void printErrorMessage(Utility::Error& e, const std::string& actual, const std::string& expected) const {
            e << "Length of actual" << actual << "doesn't match length of expected" << expected << "with epsilon" << epsilon;
        }

    private:
        int epsilon;
};

class StringLength {
    public:
        StringLength(int epsilon = 0): c(epsilon) {}

        Comparator<StringLength> comparator() { return c; }

    private:
        Comparator<StringLength> c;
};

namespace Test {

struct Test: Tester {
    Test(std::ostream* out);

    void noChecks();
    void trueExpression();
    void falseExpression();
    void equal();
    void nonEqual();
    void expectFail();
    void unexpectedPassExpression();
    void unexpectedPassEqual();

    void compareAs();
    void compareAsFail();
    void compareWith();
    void compareWithFail();
    void compareImplicitConversionFail();

    void skip();

    void testCaseName();
    void testCaseNameNoChecks();
    void testCaseDescription();

    void setupTeardown();
    void setupTeardownEmpty();
    void setupTeardownFail();
    void setupTeardownSkip();

    void setup();
    void teardown();

    void instancedTest();

    void repeatedTest();
    void repeatedTestEmpty();
    void repeatedTestFail();
    void repeatedTestSkip();

    void repeatedTestSetupTeardown();
    void repeatedTestSetupTeardownEmpty();
    void repeatedTestSetupTeardownFail();
    void repeatedTestSetupTeardownSkip();

    void benchmarkTime();
    void benchmarkCount();

    void benchmarkTimeBegin();
    std::uint64_t benchmarkTimeEnd();

    void benchmarkCountBegin();
    std::uint64_t benchmarkCountEnd();

    void benchmarkSkip();

    std::ostream* _out;
    int _i = 0;
    int _benchmarkMultiplier = 0;
};

Test::Test(std::ostream* const out): _out{out} {
    addTests({&Test::noChecks,
              &Test::trueExpression,
              &Test::falseExpression,
              &Test::equal,
              &Test::nonEqual,
              &Test::expectFail,
              &Test::unexpectedPassExpression,
              &Test::unexpectedPassEqual,

              &Test::compareAs,
              &Test::compareAsFail,
              &Test::compareWith,
              &Test::compareWithFail,
              &Test::compareImplicitConversionFail,

              &Test::skip,

              &Test::testCaseName,
              &Test::testCaseNameNoChecks,
              &Test::testCaseDescription});

    addTests({&Test::setupTeardown,
              &Test::setupTeardownEmpty,
              &Test::setupTeardownFail,
              &Test::setupTeardownSkip},
              &Test::setup,
              &Test::teardown);

    addInstancedTests({&Test::instancedTest}, 5);

    addRepeatedTests({&Test::repeatedTest}, 5);

    addRepeatedTests({&Test::repeatedTestEmpty,
                      &Test::repeatedTestFail,
                      &Test::repeatedTestSkip}, 50);

    addRepeatedTests({&Test::repeatedTestSetupTeardown,
                      &Test::repeatedTestSetupTeardownEmpty,
                      &Test::repeatedTestSetupTeardownFail,
                      &Test::repeatedTestSetupTeardownSkip}, 2,
                      &Test::setup, &Test::teardown);

    addCustomBenchmarks({&Test::benchmarkTime}, 3,
                         &Test::benchmarkTimeBegin,
                         &Test::benchmarkTimeEnd,
                         BenchmarkUnits::Time);

    addCustomBenchmarks({&Test::benchmarkCount}, 5,
                         &Test::benchmarkCountBegin,
                         &Test::benchmarkCountEnd,
                         BenchmarkUnits::Cycles);

    addBenchmarks({&Test::benchmarkSkip}, 3);
}

void Test::noChecks() {
    return;
}

void Test::trueExpression() {
    CORRADE_VERIFY(true); // #1
}

void Test::falseExpression() {
    CORRADE_VERIFY(5 != 5); // #2
}

void Test::equal() {
    CORRADE_COMPARE(3, 3); // #3
}

void Test::nonEqual() {
    int a = 5;
    int b = 3;
    CORRADE_COMPARE(a, b); // #4
}

void Test::expectFail() {
    {
        CORRADE_EXPECT_FAIL("The world is not mad yet.");
        CORRADE_COMPARE(2 + 2, 5); // #5
        CORRADE_VERIFY(false == true); // #6
    }

    CORRADE_VERIFY(true); // #7

    {
        CORRADE_EXPECT_FAIL_IF(6*7 == 49, "This is not our universe");
        CORRADE_VERIFY(true); // #8
    }
}

void Test::unexpectedPassExpression() {
    CORRADE_EXPECT_FAIL("Not yet implemented.");
    CORRADE_VERIFY(true == true); // #9
}

void Test::unexpectedPassEqual() {
    CORRADE_EXPECT_FAIL("Cannot get it right.");
    CORRADE_COMPARE(2 + 2, 4); // #10
}

void Test::compareAs() {
    CORRADE_COMPARE_AS("kill!", "hello", StringLength); // #11
}

void Test::compareAsFail() {
    CORRADE_COMPARE_AS("meh", "hello", StringLength); // #12
}

void Test::compareWith() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10)); // #13
}

void Test::compareWithFail() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(9)); // #14
}

void Test::compareImplicitConversionFail() {
    std::string hello{"hello"};
    CORRADE_COMPARE("holla", hello); // #15
}

void Test::skip() {
    CORRADE_SKIP("This testcase is skipped.");
    CORRADE_VERIFY(false); // (not called)
}

void Test::testCaseName() {
    setTestCaseName("testCaseName<15>");
    CORRADE_VERIFY(true);
}

void Test::testCaseNameNoChecks() {
    setTestCaseName("testCaseName<27>");
}

void Test::testCaseDescription() {
    setTestCaseDescription("hello");
    CORRADE_VERIFY(true);
}

void Test::setup() {
    Debug{_out} << "       [" << Debug::nospace << testCaseId() << Debug::nospace << "] setting up...";
}

void Test::teardown() {
    Debug{_out} << "       [" << Debug::nospace << testCaseId() << Debug::nospace << "] tearing down...";
}

void Test::setupTeardown() {
    CORRADE_VERIFY(true);
}

void Test::setupTeardownEmpty() {}

void Test::setupTeardownFail() {
    CORRADE_VERIFY(false);
}

void Test::setupTeardownSkip() {
    CORRADE_SKIP("Skipped.");
}

namespace {
    constexpr struct {
        const char* desc;
        int value;
        int result;
    } InstanceData[] = {
        {"zero",   3,   27},
        {nullptr,  1,    1},
        {"two",    5,  122},
        {nullptr, -6, -216},
        {"last",   0,    0}
    };
}

void Test::instancedTest() {
    const auto& data = InstanceData[testCaseInstanceId()];
    if(data.desc) setTestCaseDescription(data.desc);

    CORRADE_COMPARE(data.value*data.value*data.value, data.result);
}

void Test::repeatedTest() {
    Debug{_out} << testCaseRepeatId();
    CORRADE_VERIFY(true);
}

void Test::repeatedTestEmpty() {}

void Test::repeatedTestFail() {
    CORRADE_VERIFY(_i++ < 17);
}

void Test::repeatedTestSkip() {
    if(_i++ > 45) CORRADE_SKIP("Too late.");
}

void Test::repeatedTestSetupTeardown() {
    CORRADE_VERIFY(true);
}

void Test::repeatedTestSetupTeardownEmpty() {}

void Test::repeatedTestSetupTeardownFail() {
    CORRADE_VERIFY(false);
}

void Test::repeatedTestSetupTeardownSkip() {
    CORRADE_SKIP("Skipped.");
}

void Test::benchmarkTime() {
    CORRADE_BENCHMARK(2) {
        Debug{_out} << "Benchmark iteration";
    }
}

void Test::benchmarkTimeBegin() {
    setBenchmarkName("Wall clock time");
    Debug{_out} << "Benchmark begin";
}

std::uint64_t Test::benchmarkTimeEnd() {
    std::uint64_t time = 300 + _benchmarkMultiplier++*100;
    Debug{_out} << "Benchmark end:" << time;
    return time;
}

void Test::benchmarkCount() {
    const std::string a = "hello";
    const std::string b = "world";
    CORRADE_BENCHMARK(100) {
        std::string c = a + b + b + a + a + b;
    }
}

void Test::benchmarkCountBegin() {
    setBenchmarkName("CPU cycles");
}

std::uint64_t Test::benchmarkCountEnd() {
    std::uint64_t count = 5 + _benchmarkMultiplier++*5;
    Debug{_out} << "Benchmark end:" << count;
    return count;
}

void Test::Test::benchmarkSkip() {
    CORRADE_SKIP("Can't verify the measurements anyway.");

    const std::string a = "hello";
    const std::string b = "world";
    CORRADE_BENCHMARK(100) {
        std::string c = a + b + b + a + a + b;
    }
}

struct TesterTest: Tester {
    explicit TesterTest();

    void test();
    void emptyTest();

    void skipOnly();
    void skipAll();
    void skipTests();
    void skipBenchmarks();
    void skipTestsNothingElse();
    void skipBenchmarksNothingElse();
    void skipTestsBenchmarks();

    void repeatEvery();
    void repeatAll();

    void abortOnFail();
    void noXfail();

    void compareNoCommonType();
    void compareAsOverload();
    void compareAsVarargs();
    void compareNonCopyable();
    void verifyExplicitBool();
    void expectFailIfExplicitBool();
};

class EmptyTest: public Tester {};

TesterTest::TesterTest() {
    addTests({&TesterTest::test,
              &TesterTest::emptyTest,

              &TesterTest::skipOnly,
              &TesterTest::skipAll,
              &TesterTest::skipTests,
              &TesterTest::skipBenchmarks,
              &TesterTest::skipTestsNothingElse,
              &TesterTest::skipBenchmarksNothingElse,
              &TesterTest::skipTestsBenchmarks,

              &TesterTest::repeatEvery,
              &TesterTest::repeatAll,

              &TesterTest::abortOnFail,
              &TesterTest::noXfail,

              &TesterTest::compareNoCommonType,
              &TesterTest::compareAsOverload,
              &TesterTest::compareAsVarargs,
              &TesterTest::compareNonCopyable,
              &TesterTest::verifyExplicitBool,
              &TesterTest::expectFailIfExplicitBool});
}

namespace {
    /* Disable automatic colors to ensure we have the same behavior everywhere */
    const char* noColorArgv[] = { "", "--color", "off" };
    const int noColorArgc = std::extent<decltype(noColorArgv)>();
}

void TesterTest::test() {
    /* Print to visually verify coloring */
    {
        Debug{} << "======================== visual color verification start =======================";
        Test t{&std::cout};
        t.registerTest("here.cpp", "TesterTest::Test");
        t.exec(0, nullptr);
        Debug{} << "======================== visual color verification end =========================";
    }

    std::stringstream out;
    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(noColorArgc, noColorArgv, &out, &out);

    CORRADE_VERIFY(result == 1);

    std::string expected =
        "Starting TesterTest::Test with 37 test cases...\n"
        "     ? [01] <unknown>()\n"
        "    OK [02] trueExpression()\n"
        "  FAIL [03] falseExpression() at here.cpp on line 189\n"
        "        Expression 5 != 5 failed.\n"
        "    OK [04] equal()\n"
        "  FAIL [05] nonEqual() at here.cpp on line 199\n"
        "        Values a and b are not the same, actual is\n"
        "        5\n"
        "        but expected\n"
        "        3\n"
        " XFAIL [06] expectFail() at here.cpp on line 205\n"
        "        The world is not mad yet. 2 + 2 and 5 are not equal.\n"
        " XFAIL [06] expectFail() at here.cpp on line 206\n"
        "        The world is not mad yet. Expression false == true failed.\n"
        "    OK [06] expectFail()\n"
        " XPASS [07] unexpectedPassExpression() at here.cpp on line 219\n"
        "        Expression true == true was expected to fail.\n"
        " XPASS [08] unexpectedPassEqual() at here.cpp on line 224\n"
        "        2 + 2 and 4 are not expected to be equal.\n"
        "    OK [09] compareAs()\n"
        "  FAIL [10] compareAsFail() at here.cpp on line 232\n"
        "        Length of actual \"meh\" doesn't match length of expected \"hello\" with epsilon 0\n"
        "    OK [11] compareWith()\n"
        "  FAIL [12] compareWithFail() at here.cpp on line 240\n"
        "        Length of actual \"You rather GTFO\" doesn't match length of expected \"hello\" with epsilon 9\n"
        "  FAIL [13] compareImplicitConversionFail() at here.cpp on line 245\n"
        "        Values \"holla\" and hello are not the same, actual is\n"
        "        holla\n"
        "        but expected\n"
        "        hello\n"
        "  SKIP [14] skip()\n"
        "        This testcase is skipped.\n"
        "    OK [15] testCaseName<15>()\n"
        "     ? [16] testCaseName<27>()\n"
        "    OK [17] testCaseDescription(hello)\n"
        "       [18] setting up...\n"
        "       [18] tearing down...\n"
        "    OK [18] setupTeardown()\n"
        "       [19] setting up...\n"
        "       [19] tearing down...\n"
        "     ? [19] <unknown>()\n"
        "       [20] setting up...\n"
        "  FAIL [20] setupTeardownFail() at here.cpp on line 282\n"
        "        Expression false failed.\n"
        "       [20] tearing down...\n"
        "       [21] setting up...\n"
        "  SKIP [21] setupTeardownSkip()\n"
        "        Skipped.\n"
        "       [21] tearing down...\n"
        "    OK [22] instancedTest(zero)\n"
        "    OK [23] instancedTest(1)\n"
        "  FAIL [24] instancedTest(two) at here.cpp on line 307\n"
        "        Values data.value*data.value*data.value and data.result are not the same, actual is\n"
        "        125\n"
        "        but expected\n"
        "        122\n"
        "    OK [25] instancedTest(3)\n"
        "    OK [26] instancedTest(last)\n"
        "0\n"
        "1\n"
        "2\n"
        "3\n"
        "4\n"
        "    OK [27] repeatedTest()@5\n"
        "     ? [28] <unknown>()@50\n"
        "  FAIL [29] repeatedTestFail()@18 at here.cpp on line 318\n"
        "        Expression _i++ < 17 failed.\n"
        "  SKIP [30] repeatedTestSkip()@29\n"
        "        Too late.\n"
        "       [31] setting up...\n"
        "       [31] tearing down...\n"
        "       [31] setting up...\n"
        "       [31] tearing down...\n"
        "    OK [31] repeatedTestSetupTeardown()@2\n"
        "       [32] setting up...\n"
        "       [32] tearing down...\n"
        "       [32] setting up...\n"
        "       [32] tearing down...\n"
        "     ? [32] <unknown>()@2\n"
        "       [33] setting up...\n"
        "  FAIL [33] repeatedTestSetupTeardownFail()@1 at here.cpp on line 332\n"
        "        Expression false failed.\n"
        "       [33] tearing down...\n"
        "       [34] setting up...\n"
        "  SKIP [34] repeatedTestSetupTeardownSkip()@1\n"
        "        Skipped.\n"
        "       [34] tearing down...\n"
        "Benchmark begin\n"
        "Benchmark iteration\n"
        "Benchmark iteration\n"
        "Benchmark end: 300\n"
        "Benchmark begin\n"
        "Benchmark iteration\n"
        "Benchmark iteration\n"
        "Benchmark end: 400\n"
        "Benchmark begin\n"
        "Benchmark iteration\n"
        "Benchmark iteration\n"
        "Benchmark end: 500\n"
        " BENCH [35] benchmarkTime()@3\n"
        "        2 iterations per repeat. Wall clock time per iteration:\n"
        "        Min: 150.00 ns       Max: 250.00 ns       Avg: 200.00 ns      \n"
        "Benchmark end: 20\n"
        "Benchmark end: 25\n"
        "Benchmark end: 30\n"
        "Benchmark end: 35\n"
        "Benchmark end: 40\n"
        " BENCH [36] benchmarkCount()@5\n"
        "        100 iterations per repeat. CPU cycles per iteration:\n"
        "        Min:   0.20  cycles  Max:   0.40  cycles  Avg:   0.30  cycles \n"
        "  SKIP [37] benchmarkSkip()@1\n"
        "        Can't verify the measurements anyway.\n"
        "Finished TesterTest::Test with 11 errors out of 50 checks. 5 test cases didn't contain any checks!\n";

    //CORRADE_COMPARE(out.str().length(), expected.length());
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::emptyTest() {
    std::stringstream out;

    EmptyTest t;
    t.registerTest("here.cpp", "TesterTest::EmptyTest");
    int result = t.exec(noColorArgc, noColorArgv, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::EmptyTest!\n");
}

void TesterTest::skipOnly() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 14 4 9", "--skip", "14" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 0);

    std::string expected =
        "Starting TesterTest::Test with 3 test cases...\n"
        "    OK [11] compareWith()\n"
        "    OK [04] equal()\n"
        "    OK [09] compareAs()\n"
        "Finished TesterTest::Test with 0 errors out of 3 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::skipAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "14", "--skip", "14" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::skipTests() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 36 9", "--skip-tests" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 0);

    std::string expected =
        "Starting TesterTest::Test with 1 test cases...\n"
        "Benchmark end: 5\n"
        "Benchmark end: 10\n"
        "Benchmark end: 15\n"
        "Benchmark end: 20\n"
        "Benchmark end: 25\n"
        " BENCH [36] benchmarkCount()@5\n"
        "        100 iterations per repeat. CPU cycles per iteration:\n"
        "        Min:   0.05  cycles  Max:   0.25  cycles  Avg:   0.15  cycles \n"
        "Finished TesterTest::Test with 0 errors out of 0 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::skipBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 36 9", "--skip-benchmarks" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 0);

    std::string expected =
        "Starting TesterTest::Test with 2 test cases...\n"
        "    OK [11] compareWith()\n"
        "    OK [09] compareAs()\n"
        "Finished TesterTest::Test with 0 errors out of 2 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::skipTestsNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 9", "--skip-tests" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining benchmarks to run in TesterTest::Test.\n");
}

void TesterTest::skipBenchmarksNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "36", "--skip-benchmarks" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining tests to run in TesterTest::Test.\n");
}

void TesterTest::skipTestsBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--skip-tests", "--skip-benchmarks" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::repeatEvery() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "27 4", "--repeat-every", "2" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_VERIFY(result == 0);

    std::string expected =
        "Starting TesterTest::Test with 2 test cases...\n"
        "0\n"
        "1\n"
        "2\n"
        "3\n"
        "4\n"
        "5\n"
        "6\n"
        "7\n"
        "8\n"
        "9\n"
        "    OK [27] repeatedTest()@10\n"
        "    OK [04] equal()@2\n"
        "Finished TesterTest::Test with 0 errors out of 12 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::repeatAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "27 4", "--repeat-all", "2" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_VERIFY(result == 0);

    std::string expected =
        "Starting TesterTest::Test with 4 test cases...\n"
        "0\n"
        "1\n"
        "2\n"
        "3\n"
        "4\n"
        "    OK [27] repeatedTest()@5\n"
        "    OK [04] equal()\n"
        "0\n"
        "1\n"
        "2\n"
        "3\n"
        "4\n"
        "    OK [27] repeatedTest()@5\n"
        "    OK [04] equal()\n"
        "Finished TesterTest::Test with 0 errors out of 12 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::abortOnFail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "1 2 3 4", "--abort-on-fail" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_VERIFY(result == 1);

    std::string expected =
        "Starting TesterTest::Test with 4 test cases...\n"
        "     ? [01] <unknown>()\n"
        "    OK [02] trueExpression()\n"
        "  FAIL [03] falseExpression() at here.cpp on line 189\n"
        "        Expression 5 != 5 failed.\n"
        "Aborted TesterTest::Test after first failure out of 2 checks so far. 1 test cases didn't contain any checks!\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::noXfail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "6", "--no-xfail" };
    const int argc = std::extent<decltype(argv)>();

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(argc, argv, &out, &out);

    CORRADE_COMPARE(result, 1);

    std::string expected =
        "Starting TesterTest::Test with 1 test cases...\n"
        "  FAIL [06] expectFail() at here.cpp on line 205\n"
        "        Values 2 + 2 and 5 are not the same, actual is\n"
        "        4\n"
        "        but expected\n"
        "        5\n"
        "Finished TesterTest::Test with 1 errors out of 1 checks.\n";
    CORRADE_COMPARE(out.str(), expected);
}

void TesterTest::compareNoCommonType() {
    /* Test that this compiles well */
    struct A {
        /*implicit*/ A(int value): value(value) {}
        /*implicit*/ operator int() const { return value; }
        int value;
    };
    CORRADE_COMPARE(A{5}, 5);
}

void TesterTest::compareAsOverload() {
    /* Just test that this compiles well */
    float a = 3.0f;
    double b = 3.0f;
    CORRADE_COMPARE_AS(a, b, float);
    CORRADE_COMPARE_AS(a, b, double);
}

void TesterTest::compareAsVarargs() {
    const std::pair<int, int> a(3, 5);
    const std::pair<float, float> b(3.2f, 5.7f);
    CORRADE_COMPARE_AS(a, b, std::pair<int, int>);
}

namespace {
    struct NonCopyable {
        explicit NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = delete;
    };

    inline bool operator==(const NonCopyable&, const NonCopyable&) { return true; }
    inline Utility::Debug& operator<<(Utility::Debug& debug, const NonCopyable&) {
        return debug << "NonCopyable";
    }
}

void TesterTest::compareNonCopyable() {
    /* Just to verify that there is no need to copy anything anywhere */
    NonCopyable a, b;
    CORRADE_COMPARE(a, b);
}

void TesterTest::verifyExplicitBool() {
    struct ExplicitTrue { explicit operator bool() const { return true; } };
    ExplicitTrue t;
    CORRADE_VERIFY(t);
    CORRADE_VERIFY(ExplicitTrue());

    struct ExplicitTrueNonConst { explicit operator bool() { return true; } };
    ExplicitTrueNonConst tc;
    CORRADE_VERIFY(tc);
    CORRADE_VERIFY(ExplicitTrueNonConst());

    struct ExplicitFalse { explicit operator bool() const { return false; } };
    ExplicitFalse f;
    CORRADE_VERIFY(!f);
}

void TesterTest::expectFailIfExplicitBool() {
    struct ExplicitFalse { explicit operator bool() const { return false; } };
    {
        ExplicitFalse t;
        CORRADE_EXPECT_FAIL_IF(t, "");
        CORRADE_EXPECT_FAIL_IF(ExplicitFalse{}, "");
        CORRADE_VERIFY(true);
    }

    struct ExplicitFalseNonConst { explicit operator bool() { return false; } };
    {
        ExplicitFalseNonConst t;
        CORRADE_EXPECT_FAIL_IF(t, "");
        CORRADE_EXPECT_FAIL_IF(ExplicitFalseNonConst{}, "");
        CORRADE_VERIFY(true);
    }

    struct ExplicitTrue { explicit operator bool() const { return true; } };
    {
        CORRADE_EXPECT_FAIL_IF(ExplicitTrue{}, "");
        CORRADE_VERIFY(false);
    }
}

}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
