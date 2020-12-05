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

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/StlMath.h"
#include "Corrade/Utility/String.h"

#include "configure.h"

namespace Corrade { namespace TestSuite {

class StringLength;

template<> class Comparator<StringLength> {
    public:
        Comparator(int epsilon = 0): epsilon(epsilon) {}

        ComparisonStatusFlags operator()(const std::string& actual, const std::string& expected) {
            return std::abs(int(actual.size()) - int(expected.size())) <= epsilon ? ComparisonStatusFlags{} : ComparisonStatusFlag::Failed;
        }

        void printMessage(ComparisonStatusFlags, Utility::Debug& out, const char* actual, const char* expected) const {
            out << "Length of actual" << actual << "doesn't match length of expected" << expected << "with epsilon" << epsilon;
        }

    private:
        int epsilon;
};

class StringLength {
    public:
        explicit StringLength(int epsilon = 0): c(epsilon) {}

        Comparator<StringLength> comparator() { return c; }

    private:
        Comparator<StringLength> c;
};

struct MessageDiagnostic;

template<> class Comparator<MessageDiagnostic> {
    public:
        explicit Comparator(ComparisonStatusFlags flags): _flags{flags} {}

        ComparisonStatusFlags operator()(const std::string&, const std::string&) {
            return _flags;
        }

        void printMessage(ComparisonStatusFlags flags, Utility::Debug& out, const char* actual, const char* expected) const {
            if(flags & ComparisonStatusFlag::Failed) {
                out << "Files" << actual << "and" << expected << "are not the same, actual ABC but expected abc";
                return;
            }

            if(flags & ComparisonStatusFlag::Warning)
                out << "This is a warning";
            else if(flags & ComparisonStatusFlag::Message)
                out << "This is a message";
            else if(flags & ComparisonStatusFlag::Verbose)
                out << "This is a verbose note";
            else CORRADE_INTERNAL_ASSERT_UNREACHABLE();

            out << "when comparing" << actual << "and" << expected;
        }

        void saveDiagnostic(ComparisonStatusFlags flags, Utility::Debug& out, const std::string& path) {
            out << "->" << Utility::Directory::join(path,
                flags & ComparisonStatusFlag::VerboseDiagnostic ? "b.verbose.txt" : "b.txt");
        }

    private:
        ComparisonStatusFlags _flags;
};

struct MessageDiagnostic {
    static bool xfail;
    static ComparisonStatusFlags flags;

    explicit MessageDiagnostic(ComparisonStatusFlags flags): c(flags) {}
    explicit MessageDiagnostic(): MessageDiagnostic{flags} {}

    Comparator<MessageDiagnostic> comparator() { return c; }
    Comparator<MessageDiagnostic> c;
};

bool MessageDiagnostic::xfail;
ComparisonStatusFlags MessageDiagnostic::flags;

namespace Test { namespace {

struct Test: Tester {
    Test(std::ostream* out, const TesterConfiguration& configuration = TesterConfiguration{});

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
    void compareWarning();
    void compareMessage();
    void compareSaveDiagnostic();

    void skip();
    void iteration();
    void iterationScope();

    void exception();
    void exceptionNoTestCaseLine();

    void testCaseName();
    void testCaseNameNoChecks();
    void testCaseTemplateName();
    void testCaseTemplateNameList();
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

Test::Test(std::ostream* const out, const TesterConfiguration& configuration): Tester{configuration}, _out{out} {
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
              &Test::compareWarning,
              &Test::compareMessage,
              &Test::compareSaveDiagnostic,

              &Test::skip,
              &Test::iteration,
              &Test::iterationScope,

              &Test::exception,
              &Test::exceptionNoTestCaseLine,

              &Test::testCaseName,
              &Test::testCaseNameNoChecks,
              &Test::testCaseTemplateName,
              &Test::testCaseTemplateNameList,
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

void Test::compareMessage() {
    CORRADE_EXPECT_FAIL_IF(MessageDiagnostic::xfail, "Welp.");

    /* Let the flags be overriden by TesterTest::compareMessage*() later */
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic(MessageDiagnostic::flags ? MessageDiagnostic::flags : ComparisonStatusFlag::Message));
}

void Test::compareWarning() {
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic{ComparisonStatusFlag::Warning});
}

void Test::compareSaveDiagnostic() {
    CORRADE_EXPECT_FAIL_IF(MessageDiagnostic::xfail, "Welp.");

    /* Let the flags be overriden by TesterTest::saveDiagnostic*() later */
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic(MessageDiagnostic::flags ? MessageDiagnostic::flags : ComparisonStatusFlag::Diagnostic));
}

void Test::skip() {
    CORRADE_SKIP("This testcase is skipped.");
    CORRADE_VERIFY(false);
}

void Test::iteration() {
    for(std::string name: {"Lucy", "JOHN", "Ed"}) {
        CORRADE_ITERATION(name);
        for(std::size_t i = 1; i != name.size(); ++i) {
            CORRADE_ITERATION(i);
            CORRADE_VERIFY(!std::isupper(name[i]));
        }
    }
}

void Test::iterationScope() {
    {
        CORRADE_ITERATION("ahah");

        /* Second occurence of the same macro in the same scope should work
           too, using a different variable name */
        CORRADE_ITERATION("yay");

        CORRADE_COMPARE(3 + 3, 6);
    }

    /* Shouldn't print any iteration info */
    CORRADE_COMPARE(2 + 2, 5);
}

void Test::exception() {
    CORRADE_VERIFY(true);
    throw std::out_of_range{"YOU ARE FORBIDDEN FROM ACCESSING ID 7!!!"};
}

void Test::exceptionNoTestCaseLine() {
    throw std::out_of_range{"AGAIN?! NO!! ID 7 IS NOT THERE!!!"};
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

void Test::testCaseTemplateNameList() {
    setTestCaseTemplateName({"15", "int"});
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
    void noCatch();

    /* warning and message verified in test() already */
    void compareMessageVerboseDisabled();
    void compareMessageVerboseEnabled();
    void compareMessageFailed();
    void compareMessageXfail();

    void saveDiagnosticVerboseDisabled();
    void saveDiagnosticVerboseEnabled();
    void saveDiagnosticFailedDisabled();
    void saveDiagnosticFailedEnabled();
    void saveDiagnosticXfailDisabled();
    void saveDiagnosticXfailEnabled();
    void saveDiagnosticXpassDisabled();
    void saveDiagnosticXpassEnabled();
    void saveDiagnosticSucceededDisabled();
    void saveDiagnosticSucceededEnabled();
    void saveDiagnosticAbortOnFail();

    void benchmarkWallClock();
    void benchmarkCpuClock();
    void benchmarkCpuCycles();
    void benchmarkDiscardAll();
    void benchmarkDebugBuildNote();
    #ifdef __linux__
    void benchmarkCpuScalingNoWarning();
    void benchmarkCpuScalingWarning();
    void benchmarkCpuScalingWarningVerbose();
    #endif

    void testName();

    void compareNoCommonType();
    void compareAsOverload();
    void compareAsVarargs();
    void compareWithDereference();
    void compareNonCopyable();
    void verifyExplicitBool();
    void expectFailIfExplicitBool();

    void macrosInALambda();
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
              &TesterTest::noCatch,

              &TesterTest::compareMessageVerboseDisabled,
              &TesterTest::compareMessageVerboseEnabled,
              &TesterTest::compareMessageFailed,
              &TesterTest::compareMessageXfail,

              &TesterTest::saveDiagnosticVerboseDisabled,
              &TesterTest::saveDiagnosticVerboseEnabled,
              &TesterTest::saveDiagnosticFailedDisabled,
              &TesterTest::saveDiagnosticFailedEnabled,
              &TesterTest::saveDiagnosticXfailDisabled,
              &TesterTest::saveDiagnosticXfailEnabled,
              &TesterTest::saveDiagnosticXpassDisabled,
              &TesterTest::saveDiagnosticXpassEnabled,
              &TesterTest::saveDiagnosticSucceededDisabled,
              &TesterTest::saveDiagnosticSucceededEnabled,
              &TesterTest::saveDiagnosticAbortOnFail,

              &TesterTest::benchmarkWallClock,
              &TesterTest::benchmarkCpuClock,
              &TesterTest::benchmarkCpuCycles,
              &TesterTest::benchmarkDiscardAll,
              &TesterTest::benchmarkDebugBuildNote,
              #ifdef __linux__
              &TesterTest::benchmarkCpuScalingNoWarning,
              &TesterTest::benchmarkCpuScalingWarning,
              &TesterTest::benchmarkCpuScalingWarningVerbose,
              #endif

              &TesterTest::testName,

              &TesterTest::compareNoCommonType,
              &TesterTest::compareAsOverload,
              &TesterTest::compareAsVarargs,
              &TesterTest::compareWithDereference,
              &TesterTest::compareNonCopyable,
              &TesterTest::verifyExplicitBool,
              &TesterTest::expectFailIfExplicitBool,

              &TesterTest::macrosInALambda});
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
    /* Reset global state for custom comparators (gets modified by other cases
       below */
    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = {};

    /* Print to visually verify coloring */
    {
        Debug{} << "======================== visual color verification start =======================";
        const char* argv[] = { "", "--save-diagnostic", "/some/path" };
        int argc = Containers::arraySize(argv);
        Tester::registerArguments(argc, argv);
        Test t{&std::cout, TesterConfiguration{}
            #ifdef __linux__
            .setCpuScalingGovernorFile("")
            #endif
        };
        t.registerTest("here.cpp", "TesterTest::Test");
        t.exec(this, Debug::defaultOutput(), Error::defaultOutput());
        Debug{} << "======================== visual color verification end =========================";
    }

    std::stringstream out;

    /* Disable automatic colors to ensure we have the same behavior everywhere.
       Unlike with the color test above we DO NOT test the --save-diagnostic
       or --verbose options here as we want to see how it behaves by default. */
    const char* argv[] = { "", "--color", "off" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);
    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(
        /* Replace mangled name of std::out_of_range from the exception() tests
           with a placeholder to remove platform-specific differences */
        Utility::String::replaceAll(out.str(), typeid(const std::out_of_range&).name(), "[mangled std::out_of_range]"),
        Utility::Directory::join(TESTER_TEST_DIR, "test.txt"),
        Compare::StringToFile);
}

void TesterTest::emptyTest() {
    std::stringstream out;

    /* Disable automatic colors to ensure we have the same behavior everywhere */
    const char* argv[] = { "", "--color", "off" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);
    EmptyTest t;
    t.registerTest("here.cpp", "TesterTest::EmptyTest");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::EmptyTest!\n");
}

void TesterTest::skipOnly() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 17 4 9", "--skip", "17" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipOnly.txt"),
        Compare::StringToFile);
}

void TesterTest::skipAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "15", "--skip", "15" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::skipTests() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 47 9", "--skip-tests" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipTests.txt"),
        Compare::StringToFile);
}

void TesterTest::skipBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 46 9", "--skip-benchmarks" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "skipBenchmarks.txt"),
        Compare::StringToFile);
}

void TesterTest::skipTestsNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11 9", "--skip-tests" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining benchmarks to run in TesterTest::Test.\n");
}

void TesterTest::skipBenchmarksNothingElse() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "46", "--skip-benchmarks" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE(out.str(), "No remaining tests to run in TesterTest::Test.\n");
}

void TesterTest::skipTestsBenchmarks() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--skip-tests", "--skip-benchmarks" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::shuffleOne() {
    /* Shuffling just one test to quickly verify it doesn't blow up, at least */

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "4", "--shuffle" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "shuffleOne.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatEvery() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "37 4", "--repeat-every", "2" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "repeatEvery.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "37 4", "--repeat-all", "2" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "repeatAll.txt"),
        Compare::StringToFile);
}

void TesterTest::abortOnFail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "1 2 3 4", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "abortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::abortOnFailSkip() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "17 2 3 4", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "abortOnFailSkip.txt"),
        Compare::StringToFile);
}

void TesterTest::noXfail() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "6", "--no-xfail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "noXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::noCatch() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "20", "--no-catch" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");

    bool failed = false;
    try {
        t.exec(this, &out, &out);
    } catch(const std::out_of_range& e) {
        failed = true;
        CORRADE_COMPARE(std::string{e.what()}, "YOU ARE FORBIDDEN FROM ACCESSING ID 7!!!");
    }

    CORRADE_VERIFY(failed);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "noCatch.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageVerboseDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Verbose;
    const char* argv[] = { "", "--color", "off", "--only", "15" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message (and not fail) */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "compareMessageVerboseDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageVerboseEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Verbose;
    const char* argv[] = { "", "--color", "off", "--only", "15", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should print a verbose message (and not fail) */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "compareMessageVerboseEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageFailed() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Message;
    const char* argv[] = { "", "--color", "off", "--only", "15", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message, just the failure */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "compareMessageFailed.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageXfail() {
    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Message;
    const char* argv[] = { "", "--color", "off", "--only", "15", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message, just the XFAIL, and succeed */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "compareMessageXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticVerboseDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::VerboseDiagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, not fail and not print anything */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticVerboseDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticVerboseEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::VerboseDiagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save a verbose.txt file, but not fail */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticVerboseEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticFailedDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, only hint about the flag in the final wrap-up */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticFailedDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticFailedEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file and print both the error and SAVED */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticFailedEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXfailDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save any file, just print XFAIL and succeed -- no difference
       if diagnostic is enabled or not */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXfailEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save any file, just print XFAIL and succeed -- no difference
       if diagnostic is enabled or not */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXpassDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, but print XPASS and hint about the flag in the
       final wrap-up */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticXpassDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXpassEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file, print both XPASS and SAVED */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticXpassEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticSucceededDisabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save and shouldn't hint existence of the flag either as
       there's no error */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticSucceededDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticSucceededEnabled() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "16", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file (and print) even though there's no error */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticSucceededEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticAbortOnFail() {
    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "1 16 1", "--save-diagnostic", "/some/path", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "saveDiagnosticAbortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkWallClock() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "--benchmark", "wall-time" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkWallClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuClock() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "--benchmark", "cpu-time" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuCycles() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "--benchmark", "cpu-cycles" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuCycles.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkDiscardAll() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "--benchmark-discard", "100" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkDiscardAll.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkDebugBuildNote() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "-v" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        #ifdef __linux__
        .setCpuScalingGovernorFile("")
        #endif
    };
    t.registerTest("here.cpp", "TesterTest::Test", /*isDebugBuild=*/true);
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    /* Same as wall clock output */
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkDebugBuildNote.txt"),
        Compare::StringToFile);
}

#ifdef __linux__
void TesterTest::benchmarkCpuScalingNoWarning() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Directory::join(TESTER_TEST_DIR, "cpu-governor-performance.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    /* Same as wall clock output */
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkWallClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuScalingWarning() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Directory::join(TESTER_TEST_DIR, "cpu-governor-powersave.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    /* Same as wall clock output */
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuScalingWarning.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuScalingWarningVerbose() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "45 47", "-v" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Directory::join(TESTER_TEST_DIR, "cpu-governor-powersave.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    /* Same as wall clock output */
    CORRADE_COMPARE_AS(out.str(),
        Utility::Directory::join(TESTER_TEST_DIR, "benchmarkCpuScalingWarningVerbose.txt"),
        Compare::StringToFile);
}
#endif

void TesterTest::testName() {
    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.setTestName("MyCustomTestName");
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(t.testName(), "MyCustomTestName");
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

void TesterTest::macrosInALambda() {
    setTestCaseName(CORRADE_FUNCTION);

    []() {
        CORRADE_COMPARE_AS(3, 3, float);
        CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10));
        {
            CORRADE_EXPECT_FAIL_IF(false, "");
            CORRADE_COMPARE(3, 3);
        }
        {
            CORRADE_ITERATION("37");
            CORRADE_EXPECT_FAIL("Expected here to test CORRADE_EXPECT_FAIL().");
            CORRADE_VERIFY(false);
        }
        CORRADE_SKIP("Expected here to test CORRADE_SKIP().");
        CORRADE_BENCHMARK(3) std::puts("a");
    }();
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
