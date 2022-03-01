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

StringTest::StringTest() {
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
