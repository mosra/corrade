/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "Corrade/Containers/Optional.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/StlMath.h"

#include "configure.h"

namespace Corrade { namespace TestSuite {

class StringLength;

template<> class Comparator<StringLength> {
    public:
        Comparator(int epsilon = 0): epsilon(epsilon) {}

        bool operator()(const std::string& actual, const std::string& expected) {
            return std::abs(int(actual.size()) - int(expected.size())) <= epsilon;
        }

        void printErrorMessage(Utility::Error& e, const char* actual, const char* expected) const {
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

class SaveFailed {};

template<> class Comparator<SaveFailed> {
    public:
        bool operator()(const std::string&, const std::string&) {
            return false;
        }

        void printErrorMessage(Utility::Error& e, const char* actual, const char* expected) const {
            e << "Files" << actual << "and" << expected << "are not the same, actual ABC but expected abc";
        }

        void saveActualFile(Utility::Debug& out, const std::string& path) {
            out << "->" << Utility::Directory::join(path, "b.txt");
        }
};

namespace Test { namespace {

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
    void saveFailed();

    void skip();

    void testCaseName();
    void testCaseNameNoChecks();
    void testCaseTemplateName();
    void testCaseTemplateNameNoChecks();
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

    void benchmarkDefault();

    void benchmark();
    void benchmarkBegin();
    std::uint64_t benchmarkEnd();

    void benchmarkOnce();
    void benchmarkZero();
    void benchmarkNoMacro();
    void benchmarkOnceBegin();
    std::uint64_t benchmarkOnceEnd();

    void benchmarkSkip();

    std::ostream* _out;
    int _i = 0;
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
              &Test::saveFailed,

              &Test::skip,

              &Test::testCaseName,
              &Test::testCaseNameNoChecks,
              &Test::testCaseTemplateName,
              &Test::testCaseTemplateNameNoChecks,
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

    addBenchmarks({&Test::benchmarkDefault}, 10);

    addCustomBenchmarks({&Test::benchmark}, 3,
                         &Test::benchmarkBegin,
                         &Test::benchmarkEnd,
                         BenchmarkUnits::Nanoseconds);

    addCustomBenchmarks({&Test::benchmarkOnce,
                         &Test::benchmarkZero,
                         &Test::benchmarkNoMacro}, 1,
                         &Test::benchmarkOnceBegin,
                         &Test::benchmarkOnceEnd,
                         BenchmarkUnits::Bytes);

    addBenchmarks({&Test::benchmarkSkip}, 10);
}

void Test::noChecks() {
    return;
}

void Test::trueExpression() {
    CORRADE_VERIFY(true);
}

void Test::falseExpression() {
    CORRADE_VERIFY(5 != 5);
}

void Test::equal() {
    CORRADE_COMPARE(3, 3);
}

void Test::nonEqual() {
    int a = 5;
    int b = 3;
    CORRADE_COMPARE(a, b);
}

void Test::expectFail() {
    {
        CORRADE_EXPECT_FAIL("The world is not mad yet.");
        CORRADE_COMPARE(2 + 2, 5);
        CORRADE_VERIFY(false == true);
    }

    CORRADE_VERIFY(true);

    {
        CORRADE_EXPECT_FAIL_IF(6*7 == 49, "This is not our universe");
        CORRADE_VERIFY(true);
    }
}

void Test::unexpectedPassExpression() {
    CORRADE_EXPECT_FAIL("Not yet implemented.");
    CORRADE_VERIFY(true == true);
}

void Test::unexpectedPassEqual() {
    CORRADE_EXPECT_FAIL("Cannot get it right.");
    CORRADE_COMPARE(2 + 2, 4);
}

void Test::compareAs() {
    CORRADE_COMPARE_AS("kill!", "hello", StringLength);
}

void Test::compareAsFail() {
    CORRADE_COMPARE_AS("meh", "hello", StringLength);
}

void Test::compareWith() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10));
}

void Test::compareWithFail() {
    CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(9));
}

void Test::compareImplicitConversionFail() {
    std::string hello{"hello"};
    CORRADE_COMPARE("holla", hello);
}

void Test::saveFailed() {
    CORRADE_COMPARE_AS("a.txt", "b.txt", SaveFailed);
}

void Test::skip() {
    CORRADE_SKIP("This testcase is skipped.");
    CORRADE_VERIFY(false);
}

void Test::testCaseName() {
    setTestCaseName("testCaseName<15>");
    CORRADE_VERIFY(true);
}

void Test::testCaseNameNoChecks() {
    setTestCaseName("testCaseName<27>");
}

void Test::testCaseTemplateName() {
    setTestCaseTemplateName("15");
    CORRADE_VERIFY(true);
}

void Test::testCaseTemplateNameNoChecks() {
    setTestCaseTemplateName("27");
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

void Test::benchmark() {
    CORRADE_BENCHMARK(2) {
        Debug{_out} << "Benchmark iteration";
    }
}

void Test::benchmarkBegin() {
    Debug{_out} << "Benchmark begin";
}

std::uint64_t Test::benchmarkEnd() {
    std::uint64_t time = 300 + testCaseRepeatId()*100;
    Debug{_out} << "Benchmark end:" << time;
    return time;
}

void Test::benchmarkOnce() {
    CORRADE_BENCHMARK(1) {}
}

void Test::benchmarkZero() {
    CORRADE_BENCHMARK(0) {}

    setBenchmarkName("bytes in millibits");
}

void Test::benchmarkNoMacro() {
    CORRADE_VERIFY(true);

    setTestCaseDescription("this is gonna fail");
}

void Test::benchmarkOnceBegin() {}

std::uint64_t Test::benchmarkOnceEnd() {
    return 356720;
}

void Test::benchmarkDefault() {
    CORRADE_BENCHMARK(1000000000) break; /* nice hack, isn't it */
}

void Test::benchmarkSkip() {
    const std::string a = "hello";
    const std::string b = "world";
    CORRADE_BENCHMARK(100) {
        std::string c = a + b + b + a + a + b;
    }

    CORRADE_SKIP("Can't verify the measurements anyway.");
}

struct TesterTest: Tester {
    explicit TesterTest();

    void configurationCopy();
    void configurationMove();

    void test();
    void emptyTest();

    void skipOnly();
    void skipAll();
    void skipTests();
    void skipBenchmarks();
    void skipTestsNothingElse();
    void skipBenchmarksNothingElse();
    void skipTestsBenchmarks();

    void shuffleOne();
    void repeatEvery();
    void repeatAll();

    void abortOnFail();
    void abortOnFailSkip();
    void noXfail();
    void saveFailed();
    void saveFailedAbortOnFail();

    void benchmarkWallClock();
    void benchmarkCpuClock();
    void benchmarkCpuCycles();
    void benchmarkDiscardAll();

    void testName();

    void compareNoCommonType();
    void compareAsOverload();
    void compareAsVarargs();
    void compareWithDereference();
    void compareNonCopyable();
    void verifyExplicitBool();
    void expectFailIfExplicitBool();
};

class EmptyTest: public Tester {};

TesterTest::TesterTest() {
    addTests({&TesterTest::configurationCopy,
              &TesterTest::configurationMove,

              &TesterTest::test,
              &TesterTest::emptyTest,

              &TesterTest::skipOnly,
              &TesterTest::skipAll,
              &TesterTest::skipTests,
              &TesterTest::skipBenchmarks,
              &TesterTest::skipTestsNothingElse,
              &TesterTest::skipBenchmarksNothingElse,
              &TesterTest::skipTestsBenchmarks,

              &TesterTest::shuffleOne,
              &TesterTest::repeatEvery,
              &TesterTest::repeatAll,

              &TesterTest::abortOnFail,
              &TesterTest::abortOnFailSkip,
              &TesterTest::noXfail,
              &TesterTest::saveFailed,
              &TesterTest::saveFailedAbortOnFail,

              &TesterTest::benchmarkWallClock,
              &TesterTest::benchmarkCpuClock,
              &TesterTest::benchmarkCpuCycles,
              &TesterTest::benchmarkDiscardAll,

              &TesterTest::testName,

              &TesterTest::compareNoCommonType,
              &TesterTest::compareAsOverload,
              &TesterTest::compareAsVarargs,
              &TesterTest::compareWithDereference,
              &TesterTest::compareNonCopyable,
              &TesterTest::verifyExplicitBool,
              &TesterTest::expectFailIfExplicitBool});
}

void TesterTest::configurationCopy() {
    TesterConfiguration a;
    a.setSkippedArgumentPrefixes({"eyy", "bla"});

    TesterConfiguration b{a};
    CORRADE_COMPARE(a.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(a.skippedArgumentPrefixes()[1], "bla");
    CORRADE_COMPARE(b.skippedArgumentPrefixes()[1], "bla");

    TesterConfiguration c;
    c.setSkippedArgumentPrefixes({"welp"});
    c = b;
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(c.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(b.skippedArgumentPrefixes()[1], "bla");
    CORRADE_COMPARE(c.skippedArgumentPrefixes()[1], "bla");
}

void TesterTest::configurationMove() {
    TesterConfiguration a;
    a.setSkippedArgumentPrefixes({"eyy", "bla"});

    TesterConfiguration b{std::move(a)};
    CORRADE_VERIFY(a.skippedArgumentPrefixes().empty());
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(b.skippedArgumentPrefixes()[1], "bla");

    TesterConfiguration c;
    c.setSkippedArgumentPrefixes({"welp"});
    c = std::move(b);
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 1);
    CORRADE_COMPARE(c.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(c.skippedArgumentPrefixes()[1], "bla");
}

void TesterTest::test() {
    /* Print to visually verify coloring */
    {
        Debug{} << "======================== visual color verification start =======================";
        const char* argv[] = { "", "--save-failed", "/some/path" };
        int argc = std::extent<decltype(argv)>();
        Tester::registerArguments(argc, argv);
        Test t{&std::cout};
        t.registerTest("here.cpp", "TesterTest::Test");
        t.exec();
        Debug{} << "======================== visual color verification end =========================";
    }

    std::stringstream out;

    /* Disable automatic colors to ensure we have the same behavior everywhere */
    const char* argv[] = { "", "--color", "off" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);
    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "test.txt"),
        Compare::StringToFile);
}

void TesterTest::emptyTest() {
    std::stringstream out;

    /* Disable automatic colors to ensure we have the same behavior everywhere */
    const char* argv[] = { "", "--color", "off" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);
    EmptyTest t;
    t.registerTest("here.cpp", "TesterTest::EmptyTest");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::EmptyTest!\n");
}

void TesterTest::skipOnly() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 15 4 9", "--skip", "15" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipOnly.txt"),
        Compare::StringToFile);
}

void TesterTest::skipAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "15", "--skip", "15" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::skipTests() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 40 9", "--skip-tests" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipTests.txt"),
        Compare::StringToFile);
}

void TesterTest::skipBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 39 9", "--skip-benchmarks" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipBenchmarks.txt"),
        Compare::StringToFile);
}

void TesterTest::skipTestsNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 9", "--skip-tests" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining benchmarks to run in TesterTest::Test.\n");
}

void TesterTest::skipBenchmarksNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "39", "--skip-benchmarks" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining tests to run in TesterTest::Test.\n");
}

void TesterTest::skipTestsBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--skip-tests", "--skip-benchmarks" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::shuffleOne() {
    /* Shuffling just one test to quickly verify it doesn't blow up, at least */

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "4", "--shuffle" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "shuffleOne.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatEvery() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "30 4", "--repeat-every", "2" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "repeatEvery.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "30 4", "--repeat-all", "2" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "repeatAll.txt"),
        Compare::StringToFile);
}

void TesterTest::abortOnFail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "1 2 3 4", "--abort-on-fail" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "abortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::abortOnFailSkip() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "15 2 3 4", "--abort-on-fail" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "abortOnFailSkip.txt"),
        Compare::StringToFile);
}

void TesterTest::noXfail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "6", "--no-xfail" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "noXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveFailed() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "14 14", "--save-failed", "/some/path" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveFailed.txt"),
        Compare::StringToFile);
}

void TesterTest::saveFailedAbortOnFail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "1 14 14", "--save-failed", "/some/path", "--abort-on-fail" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveFailedAbortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkWallClock() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "38 40", "--benchmark", "wall-time" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkWallClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuClock() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "38 40", "--benchmark", "cpu-time" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuCycles() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "38 40", "--benchmark", "cpu-cycles" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuCycles.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkDiscardAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "38 40", "--benchmark-discard", "100" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkDiscardAll.txt"),
        Compare::StringToFile);
}

void TesterTest::testName() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11" };
    int argc = std::extent<decltype(argv)>();
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.setTestName("MyCustomTestName");
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(&out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "testName.txt"),
        Compare::StringToFile);
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
    double b = 3.0;
    CORRADE_COMPARE_AS(a, b, float);
    CORRADE_COMPARE_AS(a, b, double);
}

void TesterTest::compareAsVarargs() {
    const std::pair<int, int> a(3, 5);
    const std::pair<float, float> b(3.2f, 5.7f);
    CORRADE_COMPARE_AS(a, b, std::pair<int, int>);
}

void TesterTest::compareWithDereference() {
    Containers::Optional<StringLength> comparator{Containers::InPlaceInit};

    CORRADE_COMPARE_WITH("hello", "olleh", *comparator);
}

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

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
