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

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StringStl.h" /** @todo drop once Debug is stream-free */
#include "Corrade/Containers/StringView.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/StringToFile.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/StlMath.h"
#include "Corrade/Utility/String.h"

#include "configure.h"

namespace Corrade { namespace TestSuite {

using namespace Containers::Literals;

class StringLength;

template<> class Comparator<StringLength> {
    public:
        Comparator(int epsilon = 0): epsilon(epsilon) {}

        ComparisonStatusFlags operator()(Containers::StringView actual, Containers::StringView expected) {
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

        ComparisonStatusFlags operator()(Containers::StringView, Containers::StringView) {
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

        void saveDiagnostic(ComparisonStatusFlags flags, Utility::Debug& out, Containers::StringView path) {
            out << "->" << Utility::Path::join(path,
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

    void noMacros();
    void trueExpression();
    void falseExpression();
    void equal();
    void nonEqual();
    void expectFail();
    void expectFailNoChecks();
    void expectFailIfNoChecks();
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

    void info();
    void warn();
    void infoWarnNoChecks();
    void fail();
    void failNot();
    void failExpected();
    void failUnexpectedlyNot();
    void skip();
    void iteration();
    void iterationScope();
    void iterationNoChecks();

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

    void macrosInALambda();
    void benchmarkMacrosInALambda();
    void macrosInASingleExpressionBlock();
    /* CORRADE_BENCHMARK() doesn't work in a single expression block */

    std::ostream* _out;
    int _i = 0;
};

Test::Test(std::ostream* const out, const TesterConfiguration& configuration): Tester{configuration}, _out{out} {
    addTests({&Test::noMacros,
              &Test::trueExpression,
              &Test::falseExpression,
              &Test::equal,
              &Test::nonEqual,
              &Test::expectFail,
              &Test::expectFailNoChecks,
              &Test::expectFailIfNoChecks,
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

              &Test::info,
              &Test::warn,
              &Test::infoWarnNoChecks,
              &Test::fail,
              &Test::failNot,
              &Test::failExpected,
              &Test::failUnexpectedlyNot,
              &Test::skip,
              &Test::iteration,
              &Test::iterationScope,
              &Test::iterationNoChecks,

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

    addTests({&Test::macrosInALambda});

    addCustomBenchmarks({&Test::benchmarkMacrosInALambda}, 1,
                         &Test::benchmarkOnceBegin,
                         &Test::benchmarkOnceEnd,
                         BenchmarkUnits::Bytes);

    addTests({&Test::macrosInASingleExpressionBlock});
}

void Test::noMacros() {}

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
        const char* world = "world"; /* should behave like Debug */
        CORRADE_EXPECT_FAIL("The" << world << "is not mad yet.");
        CORRADE_COMPARE(2 + 2, 5);
        CORRADE_VERIFY(false == true);
    }

    CORRADE_VERIFY(true);

    {
        const char* our = "our"; /* should behave like Debug */
        CORRADE_EXPECT_FAIL_IF(6*7 == 49, "This is not" << our << "universe");
        CORRADE_VERIFY(true);
    }

    CORRADE_EXPECT_FAIL("This shouldn't affect the output in any way.");
    CORRADE_EXPECT_FAIL_IF(true, "Neither this.");
}

void Test::expectFailNoChecks() {
    /* Should still be reported as the test case having no checks, but the name
       should be recorded */
    CORRADE_EXPECT_FAIL("Does nothing.");
}

void Test::expectFailIfNoChecks() {
    /* Should still be reported as the test case having no checks, but the name
       should be recorded */
    CORRADE_EXPECT_FAIL("Does nothing.");
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
    Containers::StringView hello = "hello";
    CORRADE_COMPARE("holla", hello);
}

void Test::compareMessage() {
    CORRADE_EXPECT_FAIL_IF(MessageDiagnostic::xfail, "Welp.");

    /* Let the flags be overridden by TesterTest::compareMessage*() later */
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic(MessageDiagnostic::flags ? MessageDiagnostic::flags : ComparisonStatusFlag::Message));
}

void Test::compareWarning() {
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic{ComparisonStatusFlag::Warning});
}

void Test::compareSaveDiagnostic() {
    CORRADE_EXPECT_FAIL_IF(MessageDiagnostic::xfail, "Welp.");

    /* Let the flags be overridden by TesterTest::saveDiagnostic*() later */
    CORRADE_COMPARE_WITH("a.txt", "b.txt", MessageDiagnostic(MessageDiagnostic::flags ? MessageDiagnostic::flags : ComparisonStatusFlag::Diagnostic));
}

void Test::info() {
    int value = 7;
    CORRADE_INFO("The value is" << value);
    CORRADE_VERIFY(value != 5);
}

void Test::warn() {
    int value = 7;
    if(value > 5)
        CORRADE_WARN("The value" << value << "is higher than 5");
    CORRADE_VERIFY(value != 5);
}

void Test::infoWarnNoChecks() {
    /* For a lack of better way, this manifests in output as an INFO line,
       followed by a WARN line and finally a ? line, indicating that there are
       no actual checks. */
    CORRADE_INFO("This test talks");
    CORRADE_WARN("Instead of testing!!!");
}

void Test::fail() {
    int value = 7;
    CORRADE_FAIL_IF(value > 5, "The value" << value << "is higher than 5");
}


void Test::failNot() {
    int value = 5;
    CORRADE_FAIL_IF(value > 5, "The value" << value << "is higher than 5");
}

void Test::failExpected() {
    int value = 7;

    CORRADE_EXPECT_FAIL("Our values are high.");
    CORRADE_FAIL_IF(value > 5, "The value" << value << "is higher than 5");
}

void Test::failUnexpectedlyNot() {
    int value = 5;

    CORRADE_EXPECT_FAIL("Our values are high.");
    CORRADE_FAIL_IF(value > 5, "The value" << value << "is higher than 5");
}

void Test::skip() {
    const char* is = "is"; /* should behave like Debug */
    CORRADE_SKIP("This testcase" << is << "skipped.");
    CORRADE_VERIFY(false);
}

void Test::iteration() {
    for(Containers::StringView name: {"Lucy", "JOHN", "Ed"}) {
        CORRADE_ITERATION(name << "is the name"); /* should behave like Debug */
        for(std::size_t i = 1; i != name.size(); ++i) {
            CORRADE_ITERATION(i);
            CORRADE_VERIFY(!std::isupper(name[i]));
        }
    }

    CORRADE_ITERATION("This shouldn't affect the output in any way.");
}

void Test::iterationScope() {
    {
        CORRADE_ITERATION("ahah");

        /* Second occurrence of the same macro in the same scope should work
           too, using a different variable name */
        CORRADE_ITERATION("yay");

        CORRADE_COMPARE(3 + 3, 6);
    }

    /* Shouldn't print any iteration info */
    CORRADE_COMPARE(2 + 2, 5);
}

void Test::iterationNoChecks() {
    /* Should still be reported as the test case having no checks, but the name
       should be recorded */
    CORRADE_ITERATION(3);
}

void Test::exception() {
    CORRADE_VERIFY(true);
    throw std::out_of_range{"YOU ARE FORBIDDEN FROM ACCESSING ID 7!!!"};
}

void Test::exceptionNoTestCaseLine() {
    throw std::out_of_range{"AGAIN?! NO!! ID 7 IS NOT THERE!!!"};
}

void Test::testCaseName() {
    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
    setTestCaseName("testCaseName<15>");
    CORRADE_VERIFY(true);
}

void Test::testCaseNameNoChecks() {
    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
    setTestCaseName("testCaseName<27>");
}

void Test::testCaseTemplateName() {
    setTestCaseTemplateName("15");
    CORRADE_VERIFY(true);
}

void Test::testCaseTemplateNameList() {
    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
    setTestCaseTemplateName({"15", "int"});
    CORRADE_VERIFY(true);
}

void Test::testCaseTemplateNameNoChecks() {
    setTestCaseTemplateName("27");
}

void Test::testCaseDescription() {
    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
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

    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
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
    CORRADE_BENCHMARK(4000000000u) break; /* nice hack, isn't it */
}

void Test::benchmarkSkip() {
    Containers::StringView a = "hello";
    Containers::StringView b = "world";
    CORRADE_BENCHMARK(100) {
        Containers::String c = a + b + b + a + a + b; /* slow AF!! */
    }

    CORRADE_SKIP("Can't verify the measurements anyway.");
}

void Test::macrosInALambda() {
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
        CORRADE_INFO("Expected here to test CORRADE_INFO().");
        CORRADE_WARN("Expected here to test CORRADE_WARN().");
        {
            /* Cannot fail, otherwise the SKIP wouldn't be tested */
            CORRADE_EXPECT_FAIL("Expected here to test CORRADE_FAIL_IF().");
            CORRADE_FAIL_IF(true, "Yes.");
        }

        /* Has to be last! */
        CORRADE_SKIP("Expected here to test CORRADE_SKIP().");
    }();
}

void Test::benchmarkMacrosInALambda() {
    setTestCaseName(CORRADE_FUNCTION);

    []() {
        CORRADE_BENCHMARK(1) {}
    }();
}

void Test::macrosInASingleExpressionBlock() {
    /* All this should compile fine even though the ITERATION and EXPECT_FAIL
       variants make no sense this way and are basically no-ops */

    if(true)
        CORRADE_VERIFY(true);

    if(true)
        CORRADE_COMPARE(true, true);

    if(true)
        CORRADE_COMPARE_AS(true, true, bool);

    if(true)
        CORRADE_COMPARE_WITH("You rather GTFO", "hello", StringLength(10));

    if(true)
        CORRADE_EXPECT_FAIL("This makes no sense.");

    if(true)
        CORRADE_EXPECT_FAIL_IF(true, "This makes no sense either.");

    if(true)
        CORRADE_ITERATION("This is a no-op.");

    if(true)
        CORRADE_INFO("Expected here to test CORRADE_INFO().");
    if(true)
        CORRADE_WARN("Expected here to test CORRADE_WARN().");
    {
        /* Cannot fail, otherwise the SKIP wouldn't be tested */
        CORRADE_EXPECT_FAIL("Expected here to test CORRADE_FAIL_IF().");
        if(true)
            CORRADE_FAIL_IF(true, "Yes.");
    }

    /* Has to be last! */
    if(true)
        CORRADE_SKIP("Expected here to test CORRADE_SKIP().");
}

struct TesterTest: Tester {
    explicit TesterTest();

    template<class T> void configurationSetSkippedArgumentPrefixes();
    void configurationCopy();
    void configurationMove();
    void configurationSkippedPrefixesGlobalLiterals();
    #ifdef __linux__
    void configurationCpuScalingGovernorFileGlobalLiterals();
    #endif

    void test();
    void emptyTest();

    void skipOnly();
    void skipOnlyInvalid();
    void skipAll();
    void skipTests();
    void skipBenchmarks();
    void skipTestsNothingElse();
    void skipBenchmarksNothingElse();
    void skipTestsBenchmarks();

    void shuffleOne();
    void repeatEvery();
    void repeatAll();
    void repeatInvalid();

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
    void benchmarkInvalid();

    void testName();
    void namesGlobalLiterals();

    void verifyVarargs();
    void verifyExplicitBool();

    void compareNoCommonType();
    void compareAsOverload();
    void compareAsVarargs();
    void compareWithDereference();
    void compareNonCopyable();
    void expectFailIfExplicitBool();
    void failIfExplicitBool();
};

class EmptyTest: public Tester {};

TesterTest::TesterTest() {
    addTests({&TesterTest::configurationSetSkippedArgumentPrefixes<Containers::StringView>,
              &TesterTest::configurationSetSkippedArgumentPrefixes<const char*>,
              &TesterTest::configurationCopy,
              &TesterTest::configurationMove,
              &TesterTest::configurationSkippedPrefixesGlobalLiterals,
              #ifdef __linux
              &TesterTest::configurationCpuScalingGovernorFileGlobalLiterals,
              #endif

              &TesterTest::test,
              &TesterTest::emptyTest,

              &TesterTest::skipOnly,
              &TesterTest::skipOnlyInvalid,
              &TesterTest::skipAll,
              &TesterTest::skipTests,
              &TesterTest::skipBenchmarks,
              &TesterTest::skipTestsNothingElse,
              &TesterTest::skipBenchmarksNothingElse,
              &TesterTest::skipTestsBenchmarks,

              &TesterTest::shuffleOne,
              &TesterTest::repeatEvery,
              &TesterTest::repeatAll,
              &TesterTest::repeatInvalid,

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
              &TesterTest::benchmarkInvalid,

              &TesterTest::testName,
              &TesterTest::namesGlobalLiterals,

              &TesterTest::verifyVarargs,
              &TesterTest::verifyExplicitBool,

              &TesterTest::compareNoCommonType,
              &TesterTest::compareAsOverload,
              &TesterTest::compareAsVarargs,
              &TesterTest::compareWithDereference,
              &TesterTest::compareNonCopyable,
              &TesterTest::expectFailIfExplicitBool,
              &TesterTest::failIfExplicitBool});
}

template<class T> void TesterTest::configurationSetSkippedArgumentPrefixes() {
    setTestCaseTemplateName(std::is_same<T, const char*>::value ? "const char*" : "Containers::StringView");

    TesterConfiguration a;
    a.setSkippedArgumentPrefixes({static_cast<T>("eyy"), static_cast<T>("bla")});
    a.setSkippedArgumentPrefixes({static_cast<T>("welp")});

    CORRADE_COMPARE_AS(a.skippedArgumentPrefixes(),
        Containers::arrayView<Containers::StringView>({"eyy", "bla", "welp"}),
        TestSuite::Compare::Container);
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
    CORRADE_VERIFY(a.skippedArgumentPrefixes().isEmpty());
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(b.skippedArgumentPrefixes()[1], "bla");

    TesterConfiguration c;
    c.setSkippedArgumentPrefixes({"welp"});
    c = std::move(b);
    CORRADE_COMPARE(b.skippedArgumentPrefixes().size(), 1);
    CORRADE_COMPARE(c.skippedArgumentPrefixes().size(), 2);
    CORRADE_COMPARE(c.skippedArgumentPrefixes()[1], "bla");
}

void TesterTest::configurationSkippedPrefixesGlobalLiterals() {
    TesterConfiguration a;

    /* Setting them to plain C strings will cause copies to be made, global
       string literals should get referenced */
    const char* eyyC = "eyy";
    Containers::StringView blaV = "bla"_s;
    a.setSkippedArgumentPrefixes({eyyC, blaV});
    Containers::Array<Containers::StringView> prefixes = a.skippedArgumentPrefixes();
    CORRADE_COMPARE(prefixes.size(), 2);

    /* Can't test for flags() as those are always just NullTerminated because
       internally it's stored as a String and so the Global flag gets lost */
    CORRADE_VERIFY(prefixes[0].data() != eyyC);
    CORRADE_VERIFY(prefixes[1].data() == blaV);
}

#ifdef __linux__
void TesterTest::configurationCpuScalingGovernorFileGlobalLiterals() {
    TesterConfiguration a;

    /* By default it should be a global string literal */
    CORRADE_COMPARE(a.cpuScalingGovernorFile().flags(), Containers::StringViewFlag::Global|Containers::StringViewFlag::NullTerminated);

    /* Setting it to a plain C string will cause a copy to be made */
    {
        const char* eyyC = "eyy";
        a.setCpuScalingGovernorFile(eyyC);
        /* Can't test for flags() as those are always just NullTerminated
           because internally it's stored as a String and so the Global flag
           gets lost */
        CORRADE_VERIFY(a.cpuScalingGovernorFile().data() != eyyC);

    /* A global string literal should get referenced */
    } {
        Containers::StringView blaV = "bla"_s;
        a.setCpuScalingGovernorFile(blaV);
        /* Can't test for flags() as those are always just NullTerminated
           because internally it's stored as a String and so the Global flag
           gets lost */
        CORRADE_VERIFY(a.cpuScalingGovernorFile().data() == blaV);
    }
}
#endif

void TesterTest::test() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");
    if(std::getenv("CORRADE_TEST_NO_CATCH"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_CATCH set.");
    if(std::getenv("CORRADE_TEST_SAVE_DIAGNOSTIC"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SAVE_DIAGNOSTIC set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

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
        Utility::Path::join(TESTER_TEST_DIR, "test.txt"),
        Compare::StringToFile);
}

void TesterTest::emptyTest() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");

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
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "11-13;26,4", "--skip", "20-28,12" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "skipOnly.txt"),
        Compare::StringToFile);
}

void TesterTest::skipOnlyInvalid() {
    for(const char* arg: {"--skip", "--only"}) {
        CORRADE_ITERATION(arg);

        const char* argv[] = { "", "--color", "off", arg, "13,0x13" };
        int argc = Containers::arraySize(argv);
        Tester::registerArguments(argc, argv);

        std::stringstream out;
        Error redirectError{&out};
        Test t{&out};
        t.registerTest("here.cpp", "TesterTest::Test");
        CORRADE_COMPARE(t.exec(this, &out, &out), 2);
        CORRADE_COMPARE(out.str(), "Utility::parseNumberSequence(): unrecognized character x in 13,0x13\n");
    }
}

void TesterTest::skipAll() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "26", "--skip", "26" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 2);
    CORRADE_COMPARE(out.str(), "No test cases to run in TesterTest::Test!\n");
}

void TesterTest::skipTests() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "13 57 11", "--skip-tests" };
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
        Utility::Path::join(TESTER_TEST_DIR, "skipTests.txt"),
        Compare::StringToFile);
}

void TesterTest::skipBenchmarks() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "13 57 11", "--skip-benchmarks" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "skipBenchmarks.txt"),
        Compare::StringToFile);
}

void TesterTest::skipTestsNothingElse() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "13 11", "--skip-tests" };
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

    const char* argv[] = { "", "--color", "off", "--only", "57", "--skip-benchmarks" };
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
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

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
        Utility::Path::join(TESTER_TEST_DIR, "shuffleOne.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatEvery() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "47 4", "--repeat-every", "2" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "repeatEvery.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatAll() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "47 4", "--repeat-all", "2" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "repeatAll.txt"),
        Compare::StringToFile);
}

void TesterTest::repeatInvalid() {
    for(const char* arg: {"--repeat-every", "--repeat-all"}) {
        CORRADE_ITERATION(arg);

        const char* argv[] = { "", "--color", "off", arg, "0" };
        int argc = Containers::arraySize(argv);
        Tester::registerArguments(argc, argv);

        std::stringstream out;
        Error redirectError{&out};
        Test t{&out};
        t.registerTest("here.cpp", "TesterTest::Test");
        CORRADE_COMPARE(t.exec(this, &out, &out), 1);
        CORRADE_COMPARE(out.str(), "TestSuite::Tester::exec(): you have to repeat at least once\n");
    }
}

void TesterTest::abortOnFail() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "1 2 3 4", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "abortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::abortOnFailSkip() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "26 2 3 4", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_VERIFY(result == 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "abortOnFailSkip.txt"),
        Compare::StringToFile);
}

void TesterTest::noXfail() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "6 24", "--no-xfail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "noXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::noCatch() {
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "30", "--no-catch" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");

    bool failed = false;
    try {
        t.exec(this, &out, &out);
    } catch(const std::out_of_range& e) {
        failed = true;
        CORRADE_COMPARE(e.what(), "YOU ARE FORBIDDEN FROM ACCESSING ID 7!!!"_s);
    }

    CORRADE_VERIFY(failed);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "noCatch.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageVerboseDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_VERBOSE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_VERBOSE set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Verbose;
    const char* argv[] = { "", "--color", "off", "--only", "17" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message (and not fail) */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "compareMessageVerboseDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageVerboseEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Verbose;
    const char* argv[] = { "", "--color", "off", "--only", "17", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should print a verbose message (and not fail) */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "compareMessageVerboseEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageFailed() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Message;
    const char* argv[] = { "", "--color", "off", "--only", "17", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message, just the failure */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "compareMessageFailed.txt"),
        Compare::StringToFile);
}

void TesterTest::compareMessageXfail() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Message;
    const char* argv[] = { "", "--color", "off", "--only", "17", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not print any message, just the XFAIL, and succeed */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "compareMessageXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticVerboseDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_VERBOSE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_VERBOSE set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::VerboseDiagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, not fail and not print anything */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticVerboseDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticVerboseEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::VerboseDiagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path", "--verbose" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save a verbose.txt file, but not fail */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticVerboseEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticFailedDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");
    if(std::getenv("CORRADE_TEST_SAVE_DIAGNOSTIC"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SAVE_DIAGNOSTIC set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, only hint about the flag in the final wrap-up */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticFailedDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticFailedEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file and print both the error and SAVED */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticFailedEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXfailDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save any file, just print XFAIL and succeed -- no difference
       if diagnostic is enabled or not */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXfailEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save any file, just print XFAIL and succeed -- no difference
       if diagnostic is enabled or not */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticXfail.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXpassDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");
    if(std::getenv("CORRADE_TEST_SAVE_DIAGNOSTIC"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SAVE_DIAGNOSTIC set.");

    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should not save any file, but print XPASS and hint about the flag in the
       final wrap-up */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticXpassDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticXpassEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_ABORT_ON_FAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_ABORT_ON_FAIL set.");
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = true;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file, print both XPASS and SAVED */
    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticXpassEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticSucceededDisabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_SAVE_DIAGNOSTIC"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SAVE_DIAGNOSTIC set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Shouldn't save and shouldn't hint existence of the flag either as
       there's no error */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticSucceededDisabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticSucceededEnabled() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "18", "--save-diagnostic", "/some/path" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    /* Should save the file (and print) even though there's no error */
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticSucceededEnabled.txt"),
        Compare::StringToFile);
}

void TesterTest::saveDiagnosticAbortOnFail() {
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    MessageDiagnostic::xfail = false;
    MessageDiagnostic::flags = ComparisonStatusFlag::Failed|ComparisonStatusFlag::Diagnostic;
    const char* argv[] = { "", "--color", "off", "--only", "1 18 1", "--save-diagnostic", "/some/path", "--abort-on-fail" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 1);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "saveDiagnosticAbortOnFail.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkWallClock() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "--benchmark", "wall-time" };
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
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkWallClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuClock() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "--benchmark", "cpu-time" };
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
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkCpuClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuCycles() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "--benchmark", "cpu-cycles" };
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
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkCpuCycles.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkDiscardAll() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "--benchmark-discard", "100" };
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
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkDiscardAll.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkDebugBuildNote() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "-v" };
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
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkDebugBuildNote.txt"),
        Compare::StringToFile);
}

#ifdef __linux__
void TesterTest::benchmarkCpuScalingNoWarning() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Path::join(TESTER_TEST_DIR, "cpu-governor-performance.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    /* Same as wall clock output */
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkWallClock.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuScalingWarning() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_VERBOSE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_VERBOSE set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Path::join(TESTER_TEST_DIR, "cpu-governor-powersave.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkCpuScalingWarning.txt"),
        Compare::StringToFile);
}

void TesterTest::benchmarkCpuScalingWarningVerbose() {
    if(std::getenv("CORRADE_TEST_SKIP_BENCHMARKS"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SKIP_BENCHMARKS set.");
    if(std::getenv("CORRADE_TEST_SHUFFLE"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_SHUFFLE set.");
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK set.");
    if(std::getenv("CORRADE_TEST_BENCHMARK_DISCARD"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_BENCHMARK_DISCARD set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "55 57", "-v" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out, TesterConfiguration{}
        .setCpuScalingGovernorFile(Utility::Path::join(TESTER_TEST_DIR, "cpu-governor-powersave.txt"))
    };
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(result, 0);
    #ifndef CORRADE_TARGET_ANDROID
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkCpuScalingWarningVerbose.txt"),
        Compare::StringToFile);
    #else
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "benchmarkCpuScalingWarningVerboseAndroid.txt"),
        Compare::StringToFile);
    #endif
}
#endif

void TesterTest::benchmarkInvalid() {
    const char* argv[] = { "", "--color", "off", "--benchmark", "fumes" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    std::stringstream out;
    Error redirectError{&out};
    Test t{&out};
    t.registerTest("here.cpp", "TesterTest::Test");
    CORRADE_COMPARE(t.exec(this, &out, &out), 1);
    CORRADE_COMPARE(out.str(), "TestSuite::Tester::exec(): unknown benchmark type fumes, use one of wall-time, cpu-time or cpu-cycles\n");
}

void TesterTest::testName() {
    if(std::getenv("CORRADE_TEST_REPEAT_EVERY"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_EVERY set.");
    if(std::getenv("CORRADE_TEST_REPEAT_ALL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_REPEAT_ALL set.");

    std::stringstream out;

    const char* argv[] = { "", "--color", "off", "--only", "13" };
    int argc = Containers::arraySize(argv);
    Tester::registerArguments(argc, argv);

    Test t{&out};
    /* Explicitly testing the const char* overload, which then delegates to a
       StringView overload so both are tested */
    t.setTestName("MyCustomTestName");
    t.registerTest("here.cpp", "TesterTest::Test");
    int result = t.exec(this, &out, &out);

    CORRADE_COMPARE(t.testName(), "MyCustomTestName");
    CORRADE_COMPARE(result, 0);
    CORRADE_COMPARE_AS(out.str(),
        Utility::Path::join(TESTER_TEST_DIR, "testName.txt"),
        Compare::StringToFile);
}

void TesterTest::namesGlobalLiterals() {
    /* Implicitly it should be all global literals. However can't test that via
       flags() as those are always just NullTerminated  because internally it's
       stored as a String and so the Global flag gets lost */

    /* Register current test case name, save it and restore everything later to
       avoid messing up the output */
    CORRADE_VERIFY(true);
    struct Current {
        Containers::StringView testName, testCaseName;
    } current{Tester::testName(), Tester::testCaseName()};
    Containers::ScopeGuard exit{&current, [](Current* current) {
        Tester::instance().setTestName(current->testName);
        Tester::instance().setTestCaseName(current->testCaseName);
        Tester::instance().setTestCaseDescription(nullptr);
        Tester::instance().setTestCaseTemplateName(nullptr);
        Tester::instance().setBenchmarkName(nullptr);
    }};

    /* Setting them to plain C strings will cause copies to be made. Try to
       match the original test & test case name to avoid confusion when the
       test fails midway through. */
    {
        const char* testNameC = "Corrade::TestSuite::Test::TesterTest";
        setTestName(testNameC);
        CORRADE_VERIFY(Tester::testName().data() != testNameC);
    } {
        const char* testCaseNameC = "namesGlobalLiterals";
        setTestCaseName(testCaseNameC);
        CORRADE_VERIFY(testCaseName().data() != testCaseNameC);
    } {
        const char* testCaseDescriptionC = "hello";
        setTestCaseDescription(testCaseDescriptionC);
        CORRADE_VERIFY(testCaseDescription().data() != testCaseDescriptionC);
    } {
        const char* testCaseTemplateNameC = "hello";
        setTestCaseTemplateName(testCaseTemplateNameC);
        CORRADE_VERIFY(testCaseTemplateName().data() != testCaseTemplateNameC);
    } {
        const char* benchmarkNameC = "hello";
        setBenchmarkName(benchmarkNameC);
        CORRADE_VERIFY(benchmarkName().data() != benchmarkNameC);
    }

    /* Setting them to global string literals should reference them */
    {
        Containers::StringView testNameV = "Corrade::TestSuite::Test::TesterTest"_s;
        setTestName(testNameV);
        CORRADE_VERIFY(Tester::testName().data() == testNameV);
    } {
        Containers::StringView testCaseNameV = "namesGlobalLiterals"_s;
        setTestCaseName(testCaseNameV);
        CORRADE_VERIFY(testCaseName().data() == testCaseNameV);
    } {
        Containers::StringView testCaseDescriptionV = "hello"_s;
        setTestCaseDescription(testCaseDescriptionV);
        CORRADE_VERIFY(testCaseDescription().data() == testCaseDescriptionV);
    } {
        Containers::StringView testCaseTemplateNameV = "hello"_s;
        setTestCaseTemplateName(testCaseTemplateNameV);
        CORRADE_VERIFY(testCaseTemplateName().data() == testCaseTemplateNameV);
    } {
        Containers::StringView benchmarkNameV = "hello"_s;
        setBenchmarkName(benchmarkNameV);
        CORRADE_VERIFY(benchmarkName().data() == benchmarkNameV);
    }
}

void TesterTest::verifyVarargs() {
    /* Shouldn't need to have braces around */
    CORRADE_VERIFY(std::is_constructible<Containers::StringView, const char*>::value);
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
    Containers::Optional<StringLength> comparator{InPlaceInit};

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

void TesterTest::expectFailIfExplicitBool() {
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

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

void TesterTest::failIfExplicitBool() {
    if(std::getenv("CORRADE_TEST_NO_XFAIL"))
        CORRADE_SKIP("Can't test with CORRADE_TEST_NO_XFAIL set.");

    struct ExplicitFalse { explicit operator bool() const { return false; } };
    {
        ExplicitFalse t;
        CORRADE_FAIL_IF(t, "");
        CORRADE_FAIL_IF(ExplicitFalse{}, "");
    }

    struct ExplicitFalseNonConst { explicit operator bool() { return false; } };
    {
        ExplicitFalseNonConst t;
        CORRADE_FAIL_IF(t, "");
        CORRADE_FAIL_IF(ExplicitFalseNonConst{}, "");
    }

    struct ExplicitTrue { explicit operator bool() const { return true; } };
    {
        CORRADE_EXPECT_FAIL("");
        CORRADE_FAIL_IF(ExplicitTrue{}, "");
    }
}

}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Test::TesterTest)
