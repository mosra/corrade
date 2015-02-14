/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
#include <type_traits>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Arguments.h"

namespace Corrade { namespace Utility { namespace Test {

struct ArgumentsTest: TestSuite::Tester {
    explicit ArgumentsTest();

    void helpArgumentsOnly();
    void helpNamedOnly();
    void helpBoth();
    void helpText();
    void helpEmpty();
    void helpAfterParse();

    void duplicateKey();
    void duplicateShortKey();
    void disallowedCharacter();
    void disallowedCharacterShort();

    void parseHelp();
    void parseArguments();
    void parseMixed();
    void parseCustomType();
    void parseDoubleArgument();

    void parseUnknownArgument();
    void parseUnknownShortArgument();
    void parseSuperfluousArgument();
    void parseArgumentAfterSeparator();
    void parseInvalidLongArgument();

    void parseMissingValue();
    void parseMissingOption();
    void parseMissingArgument();
};

ArgumentsTest::ArgumentsTest() {
    addTests({&ArgumentsTest::helpArgumentsOnly,
              &ArgumentsTest::helpNamedOnly,
              &ArgumentsTest::helpBoth,
              &ArgumentsTest::helpText,
              &ArgumentsTest::helpEmpty,
              &ArgumentsTest::helpAfterParse,

              &ArgumentsTest::duplicateKey,
              &ArgumentsTest::duplicateShortKey,
              &ArgumentsTest::disallowedCharacter,
              &ArgumentsTest::disallowedCharacterShort,

              &ArgumentsTest::parseHelp,
              &ArgumentsTest::parseArguments,
              &ArgumentsTest::parseMixed,
              &ArgumentsTest::parseCustomType,
              &ArgumentsTest::parseDoubleArgument,

              &ArgumentsTest::parseUnknownArgument,
              &ArgumentsTest::parseUnknownShortArgument,
              &ArgumentsTest::parseSuperfluousArgument,
              &ArgumentsTest::parseArgumentAfterSeparator,
              &ArgumentsTest::parseInvalidLongArgument,

              &ArgumentsTest::parseMissingValue,
              &ArgumentsTest::parseMissingOption,
              &ArgumentsTest::parseMissingArgument});
}

void ArgumentsTest::helpArgumentsOnly() {
    Arguments args;
    args.addArgument("foo").setHelp("foo", "which foo to bar")
        .addArgument("bar").setHelpKey("bar", "output.bin")
        .setCommand("foobar");

    const auto expected = R"text(Usage:
  foobar [-h|--help] [--] foo output.bin

Arguments:
  foo         which foo to bar
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpNamedOnly() {
    Arguments args;
    args.addOption('n', "bars", "42").setHelp("bars", "number of bars to foo")
        .addNamedArgument('b', "baz").setHelpKey("baz", "LEVEL")
        .addOption("sanity", "INSANE")
        .addBooleanOption("no-bare-foos").setHelp("no-bare-foos", "don't use bare foos")
        .setCommand("foobar");

    const auto expected = R"text(Usage:
  foobar [-h|--help] [-n|--bars BARS] -b|--baz LEVEL [--sanity SANITY] [--no-bare-foos]

Arguments:
  -h, --help       display this help message and exit
  -n, --bars BARS  number of bars to foo
                   (default: 42)
  --sanity SANITY  (default: INSANE)
  --no-bare-foos   don't use bare foos
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpBoth() {
    Arguments args;
    args.addArgument("foo").setHelp("foo", "which foo to bar with")
        .addBooleanOption('B', "no-bars").setHelp("no-bars", "don't foo with bars");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [-B|--no-bars] [--] foo

Arguments:
  foo            which foo to bar with
  -h, --help     display this help message and exit
  -B, --no-bars  don't foo with bars
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpText() {
    Arguments args;
    args.addArgument("foo").setHelp("foo", "which foo to bar with")
        .setHelp("Bars with given foo.");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--] foo

Bars with given foo.

Arguments:
  foo         which foo to bar with
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpEmpty() {
    const auto expected = R"text(Usage:
  ./app [-h|--help]

Arguments:
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(Arguments().help(), expected);
}

void ArgumentsTest::helpAfterParse() {
    Arguments args;

    const char* argv[] = { "foobar" };
    const int argc = std::extent<decltype(argv)>();

    /* Take command name from argv */
    CORRADE_VERIFY(args.tryParse(argc, argv));
    const auto expected = R"text(Usage:
  foobar [-h|--help]
)text";
    CORRADE_COMPARE(args.usage(), expected);

    /* If set custom command name, don't override */
    args.setCommand("myFoobarApp");
    CORRADE_VERIFY(args.tryParse(argc, argv));
    const auto expected2 = R"text(Usage:
  myFoobarApp [-h|--help]
)text";
    CORRADE_COMPARE(args.usage(), expected2);
}

void ArgumentsTest::duplicateKey() {
    std::ostringstream out;
    Error::setOutput(&out);
    Arguments args;
    args.addArgument("foo")
        .addOption("foo");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addOption(): the key foo or its short version is already used\n");
}

void ArgumentsTest::duplicateShortKey() {
    std::ostringstream out;
    Error::setOutput(&out);
    Arguments args;
    args.addNamedArgument('b', "bar")
        .addBooleanOption('b', "foo");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addBooleanOption(): the key foo or its short version is already used\n");
}

void ArgumentsTest::disallowedCharacter() {
    std::ostringstream out;
    Error::setOutput(&out);
    Arguments args;
    args.addNamedArgument('b', "a mistake");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addNamedArgument(): invalid key a mistake or its short variant\n");
}

void ArgumentsTest::disallowedCharacterShort() {
    std::ostringstream out;
    Error::setOutput(&out);
    Arguments args;
    args.addOption(' ', "bar");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addOption(): invalid key bar or its short variant\n");
}

void ArgumentsTest::parseHelp() {
    Arguments args;
    args.addBooleanOption("no-foo-bars");

    const char* argv[] = { "", "-h", "--no-foo-bars", "error" };
    const int argc = std::extent<decltype(argv)>();

    CORRADE_VERIFY(args.tryParse(argc, argv));
    CORRADE_VERIFY(args.isSet("help"));
    CORRADE_VERIFY(!args.isSet("no-foo-bars"));
}

void ArgumentsTest::parseArguments() {
    Arguments args;
    args.addArgument("name")
        .addArgument("input")
        .addArgument("output");

    const char* argv[] = { "", "hello", "in.txt", "out.bin" };
    const int argc = std::extent<decltype(argv)>();

    CORRADE_VERIFY(args.tryParse(argc, argv));
    CORRADE_COMPARE(args.value("name"), "hello");
    CORRADE_COMPARE(args.value("input"), "in.txt");
    CORRADE_COMPARE(args.value("output"), "out.bin");
}

void ArgumentsTest::parseMixed() {
    Arguments args;
    args.addArgument("file")
        .addNamedArgument('o', "output")
        .addOption("size", "56")
        .addBooleanOption('v', "verbose")
        .addBooleanOption('l', "loud");

    const char* argv[] = { "", "-o", "log.txt", "-v", "input.txt" };
    const int argc = std::extent<decltype(argv)>();

    CORRADE_VERIFY(args.tryParse(argc, argv));
    CORRADE_VERIFY(!args.isSet("help"));
    CORRADE_VERIFY(args.isSet("verbose"));
    CORRADE_COMPARE(args.value("file"), "input.txt");

    /* Default values */
    CORRADE_COMPARE(args.value("size"), "56");
    CORRADE_VERIFY(!args.isSet("loud"));
}

void ArgumentsTest::parseCustomType() {
    Arguments args;
    args.addNamedArgument("pi");

    const char* argv[] = { "", "--pi", "0.3141516e+1" };
    const int argc = std::extent<decltype(argv)>();

    CORRADE_VERIFY(args.tryParse(argc, argv));
    CORRADE_COMPARE(args.value<float>("pi"), 3.141516f);
}

void ArgumentsTest::parseDoubleArgument() {
    Arguments args;
    args.addNamedArgument("arg")
        .addBooleanOption('b', "bool");

    const char* argv[] = { "", "--arg", "first", "-b", "--arg", "second", "-b" };
    const int argc = std::extent<decltype(argv)>();

    CORRADE_VERIFY(args.tryParse(argc, argv));
    CORRADE_COMPARE(args.value("arg"), "second");
    CORRADE_VERIFY(args.isSet("bool"));
}

void ArgumentsTest::parseUnknownArgument() {
    Arguments args;

    const char* argv[] = { "", "--error" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument --error\n");
}

void ArgumentsTest::parseUnknownShortArgument() {
    Arguments args;

    const char* argv[] = { "", "-e" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument -e\n");
}

void ArgumentsTest::parseSuperfluousArgument() {
    Arguments args;

    const char* argv[] = { "", "error" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument error\n");
}

void ArgumentsTest::parseArgumentAfterSeparator() {
    Arguments args;
    args.addBooleanOption('b', "bar");

    const char* argv[] = { "", "--", "-b" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument -b\n");
}

void ArgumentsTest::parseInvalidLongArgument() {
    Arguments args;

    const char* argv[] = { "", "-long-argument" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument -long-argument (did you mean --long-argument?)\n");
}

void ArgumentsTest::parseMissingValue() {
    Arguments args;
    args.addOption("output");

    const char* argv[] = { "", "--output" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Missing value for command-line argument --output\n");
}

void ArgumentsTest::parseMissingOption() {
    Arguments args;
    args.addNamedArgument("output");

    const char* argv[] = { "" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument --output\n");
}

void ArgumentsTest::parseMissingArgument() {
    Arguments args;
    args.addArgument("file").setHelpKey("file", "file.dat");

    const char* argv[] = { "" };
    const int argc = std::extent<decltype(argv)>();

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!args.tryParse(argc, argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument file.dat\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ArgumentsTest)
