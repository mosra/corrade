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

#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */

namespace Corrade { namespace TestSuite { namespace Compare { namespace Test { namespace {

struct StringTest: TestSuite::Tester {
    explicit StringTest();

    void diff();
    void diffMessageFailed();
    void diffMessageFailedReverse();

    void hasPrefix();
    void hasPrefixMessageFailed();
    void hasPrefixMessageVerbose();

    void hasSuffix();
    void hasSuffixMessageFailed();
    void hasSuffixMessageVerbose();

    void contains();
    void containsMessageFailed();
    void containsMessageVerbose();

    void notContains();
    void notContainsMessageFailed();
    void notContainsMessageVerbose();
};

const struct {
    const char* name;
    Containers::StringView actual;
    Containers::StringView expected;
    const char* message;
    const char* messageReverse;
} DiffData[]{
    {"different in the middle",
        "hello world\n"
        "this is cool\n"
        "yes",
        "hello world\n"
        "this isn't cool\n"
        "\n"
        "   at\n"
        "  a l l\n"
        "\n"
        "yes",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this isn't cool\n"
        "       -\n"
        "       -   at\n"
        "       -  a l l\n"
        "       -\n"
        "       +this is cool\n"
        "        yes\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this is cool\n"
        "       +this isn't cool\n"
        "       +\n"
        "       +   at\n"
        "       +  a l l\n"
        "       +\n"
        "        yes\n"},
    {"small single-line difference in the middle",
        "hello world\n"
        "this is cool\n"
        "yes",
        "hello world\n"
        "this isn't cool\n"
        "yes",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this isn't cool\n"
        "       +this is cool\n"
        "        yes\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this is cool\n"
        "       +this isn't cool\n"
        "        yes\n"},
    {"difference in the middle of a UTF-8 character",
        "média",
        "mèdia",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "       -mèdia\n"
        "       +média\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "       -média\n"
        "       +mèdia\n"},
    {"difference next to a UTF-8 character",
        "média",
        "mědia",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "       -mědia\n"
        "       +média\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "       -média\n"
        "       +mědia\n"},
    {"large single-line difference in the middle",
        "hello world\n"
        "this is cool\n"
        "yes",
        "hello world\n"
        "That's awful\n"
        "yes",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -That's awful\n"
        "       +this is cool\n"
        "        yes\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this is cool\n"
        "       +That's awful\n"
        "        yes\n"},
    {"different at the start",
        "Hello\n"
        "world!\n"
        "this is cool",
        "hello world\n"
        "this is cool",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "       -hello world\n"
        "       +Hello\n"
        "       +world!\n"
        "        this is cool\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "       -Hello\n"
        "       -world!\n"
        "       +hello world\n"
        "        this is cool\n"},
    {"different at the end",
        "hello world\n"
        "this is\n"
        "very cool!",
        "hello world\n"
        "this is cool",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this is cool\n"
        "       +this is\n"
        "       +very cool!\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "        hello world\n"
        "       -this is\n"
        "       -very cool!\n"
        "       +this is cool\n"},
    {"only additions / deletions",
        "",
        "hello world\n"
        "this is cool",
        "Strings a and b are different. Actual (+) vs expected (-):\n"
        "       -hello world\n"
        "       -this is cool\n",
        "Strings b and a are different. Actual (+) vs expected (-):\n"
        "       +hello world\n"
        "       +this is cool\n"}
};

StringTest::StringTest() {
    addTests({&StringTest::diff});

    addInstancedTests({&StringTest::diffMessageFailed,
                       &StringTest::diffMessageFailedReverse},
        Containers::arraySize(DiffData));

    addTests({&StringTest::hasPrefix,
              &StringTest::hasPrefixMessageFailed,
              &StringTest::hasPrefixMessageVerbose,

              &StringTest::hasSuffix,
              &StringTest::hasSuffixMessageFailed,
              &StringTest::hasSuffixMessageVerbose,

              &StringTest::contains,
              &StringTest::containsMessageFailed,
              &StringTest::containsMessageVerbose,

              &StringTest::notContains,
              &StringTest::notContainsMessageFailed,
              &StringTest::notContainsMessageVerbose});
}

void StringTest::diff() {
    Containers::StringView a = "hello world";
    Containers::StringView b = "hell";

    CORRADE_COMPARE(Comparator<String>{}(a, a), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<String>{}(a, b), ComparisonStatusFlag::Failed);
}

void StringTest::diffMessageFailed() {
    auto&& data = DiffData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* The diff algorithm is tested thoroughly in DiffTest, this verifies just
       the printing */

    Comparator<String> compare;
    ComparisonStatusFlags flags = compare(data.actual, data.expected);
    CORRADE_COMPARE(flags, TestSuite::ComparisonStatusFlag::Failed);

    CORRADE_INFO("Visual color verification:");
    {
        Debug out;
        compare.printMessage(flags, out, "a", "b");
    }

    std::ostringstream out;
    {
        Debug dc{&out, Debug::Flag::DisableColors};
        compare.printMessage(flags, dc, "a", "b");
    }
    CORRADE_COMPARE(out.str(), data.message);
}

void StringTest::diffMessageFailedReverse() {
    auto&& data = DiffData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* The diff algorithm is tested thoroughly in DiffTest, this verifies just
       the printing */

    Comparator<String> compare;
    ComparisonStatusFlags flags = compare(data.expected, data.actual);
    CORRADE_COMPARE(flags, TestSuite::ComparisonStatusFlag::Failed);

    CORRADE_INFO("Visual color verification:");
    {
        Debug out;
        compare.printMessage(flags, out, "b", "a");
    }

    std::ostringstream out;
    {
        Debug dc{&out, Debug::Flag::DisableColors};
        compare.printMessage(flags, dc, "b", "a");
    }
    CORRADE_COMPARE(out.str(), data.messageReverse);
}

void StringTest::hasPrefix() {
    Containers::StringView a = "hello world";
    Containers::StringView b = "hell";
    Containers::StringView c = "world";

    /* If the strings are not the same, it can print a verbose message */
    CORRADE_COMPARE(Comparator<StringHasPrefix>{}(a, a), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<StringHasPrefix>{}(a, b), ComparisonStatusFlag::Verbose);
    CORRADE_COMPARE(Comparator<StringHasPrefix>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<StringHasPrefix>{}(b, a), ComparisonStatusFlag::Failed);
}

void StringTest::hasPrefixMessageFailed() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringHasPrefix> compare;
        ComparisonStatusFlags flags = compare("hello world", "world");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a isn't prefixed with b, actual is\n"
"        hello world\n"
"        but expected prefix\n"
"        world\n");
}

void StringTest::hasPrefixMessageVerbose() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringHasPrefix> compare;
        ComparisonStatusFlags flags = compare("hello world", "hell");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Verbose);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a is prefixed with b, the actual string\n"
"        hello world\n"
"        has expected prefix\n"
"        hell\n");
}

void StringTest::hasSuffix() {
    Containers::StringView a = "hello world";
    Containers::StringView b = "world";
    Containers::StringView c = "hell";

    /* If the strings are not the same, it can print a verbose message */
    CORRADE_COMPARE(Comparator<StringHasSuffix>{}(a, a), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<StringHasSuffix>{}(a, b), ComparisonStatusFlag::Verbose);
    CORRADE_COMPARE(Comparator<StringHasSuffix>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<StringHasSuffix>{}(b, a), ComparisonStatusFlag::Failed);
}

void StringTest::hasSuffixMessageFailed() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringHasSuffix> compare;
        ComparisonStatusFlags flags = compare("hello world", "hell");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a isn't suffixed with b, actual is\n"
"        hello world\n"
"        but expected suffix\n"
"        hell\n");
}

void StringTest::hasSuffixMessageVerbose() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringHasSuffix> compare;
        ComparisonStatusFlags flags = compare("hello world", "world");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Verbose);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a is suffixed with b, the actual string\n"
"        hello world\n"
"        has expected suffix\n"
"        world\n");
}

void StringTest::contains() {
    Containers::StringView a = "what a hell world";
    Containers::StringView b = "hell";
    Containers::StringView c = "hello";

    /* If the strings are not the same, it can print a verbose message */
    CORRADE_COMPARE(Comparator<StringContains>{}(a, a), ComparisonStatusFlags{});
    CORRADE_COMPARE(Comparator<StringContains>{}(a, b), ComparisonStatusFlag::Verbose);
    CORRADE_COMPARE(Comparator<StringContains>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<StringContains>{}(b, a), ComparisonStatusFlag::Failed);
}

void StringTest::containsMessageFailed() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringContains> compare;
        ComparisonStatusFlags flags = compare("what a hell world", "hello");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a doesn't contain b, actual is\n"
"        what a hell world\n"
"        but expected to contain\n"
"        hello\n");
}

void StringTest::containsMessageVerbose() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringContains> compare;
        ComparisonStatusFlags flags = compare("what a hell world", "hell");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Verbose);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a contains b at position 7, the actual string\n"
"        what a hell world\n"
"        expectedly contains\n"
"        hell\n");
}

void StringTest::notContains() {
    Containers::StringView a = "what a hell world";
    Containers::StringView b = "hello";
    Containers::StringView c = "hell";

    /* If the strings are not the same, it can print a verbose message */
    CORRADE_COMPARE(Comparator<StringNotContains>{}(a, a), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<StringNotContains>{}(a, b), ComparisonStatusFlag::Verbose);
    CORRADE_COMPARE(Comparator<StringNotContains>{}(a, c), ComparisonStatusFlag::Failed);
    CORRADE_COMPARE(Comparator<StringNotContains>{}(b, a), ComparisonStatusFlag::Verbose);
}

void StringTest::notContainsMessageFailed() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringNotContains> compare;
        ComparisonStatusFlags flags = compare("what a hell world", "hell");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Failed);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a contains b at position 7, actual is\n"
"        what a hell world\n"
"        but expected to not contain\n"
"        hell\n");
}

void StringTest::notContainsMessageVerbose() {
    std::ostringstream out;

    {
        Debug redirectOutput{&out};
        Comparator<StringNotContains> compare;
        ComparisonStatusFlags flags = compare("what a hell world", "hello");
        CORRADE_COMPARE(flags, ComparisonStatusFlag::Verbose);
        compare.printMessage(flags, redirectOutput, "a", "b");
    }

    CORRADE_COMPARE(out.str(), "String a doesn't contain b, the actual string\n"
"        what a hell world\n"
"        expectedly doesn't contain\n"
"        hello\n");
}

}}}}}

CORRADE_TEST_MAIN(Corrade::TestSuite::Compare::Test::StringTest)
