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

#include <algorithm>
#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ArgumentsTest: TestSuite::Tester {
    explicit ArgumentsTest();

    void environment();
    void environmentUtf8();

    void copy();
    void move();

    void helpArgumentsOnly();
    void helpNamedOnly();
    void helpBoth();
    void helpText();
    void helpEmpty();
    void helpEnvironment();
    void helpEnvironmentPrefixed();
    void helpAfterParse();
    void helpLongKeys();
    void helpLongKeyNotPrinted();

    void duplicateKey();
    void duplicateShortKey();
    void disallowedCharacter();
    void disallowedCharacterShort();

    void notParsedYet();
    void notParsedYetOnlyHelp();

    void parseNullptr();
    void parseHelp();
    void parseArguments();
    void parseMixed();
    void parseCustomType();
    void parseCustomTypeFlags();
    void parseDoubleArgument();

    void parseUnknownArgument();
    void parseUnknownShortArgument();
    void parseSuperfluousArgument();
    void parseArgumentAfterSeparator();
    void parseInvalidShortArgument();
    void parseInvalidLongArgument();
    void parseInvalidLongArgumentDashes();
    void parseEnvironment();
    void parseEnvironmentUtf8();

    void parseMissingValue();
    void parseMissingOption();
    void parseMissingArgument();

    void prefixedParse();
    void prefixedParseMinus();
    void prefixedParseMinusMinus();
    void prefixedParseHelpArgument();
    void prefixedHelpWithoutPrefix();
    void prefixedHelpWithPrefix();
    void prefixedHelpLongPrefix();
    void prefixedDisallowedCalls();
    void prefixedDisallowedWithPrefix();
    void prefixedDisallowedWithPrefixAfterSkipPrefix();
    void prefixedUnknownWithPrefix();
    void prefixedInvalidPrefixedName();
    void prefixedInvalidUnprefixedName();
};

ArgumentsTest::ArgumentsTest() {
    addTests({&ArgumentsTest::environment,
              &ArgumentsTest::environmentUtf8,

              &ArgumentsTest::copy,
              &ArgumentsTest::move,

              &ArgumentsTest::helpArgumentsOnly,
              &ArgumentsTest::helpNamedOnly,
              &ArgumentsTest::helpBoth,
              &ArgumentsTest::helpText,
              &ArgumentsTest::helpEmpty,
              &ArgumentsTest::helpEnvironment,
              &ArgumentsTest::helpEnvironmentPrefixed,
              &ArgumentsTest::helpAfterParse,
              &ArgumentsTest::helpLongKeys,
              &ArgumentsTest::helpLongKeyNotPrinted,

              &ArgumentsTest::duplicateKey,
              &ArgumentsTest::duplicateShortKey,
              &ArgumentsTest::disallowedCharacter,
              &ArgumentsTest::disallowedCharacterShort,

              &ArgumentsTest::notParsedYet,
              &ArgumentsTest::notParsedYetOnlyHelp,

              &ArgumentsTest::parseNullptr,
              &ArgumentsTest::parseHelp,
              &ArgumentsTest::parseArguments,
              &ArgumentsTest::parseMixed,
              &ArgumentsTest::parseCustomType,
              &ArgumentsTest::parseCustomTypeFlags,
              &ArgumentsTest::parseDoubleArgument,
              &ArgumentsTest::parseEnvironment,
              &ArgumentsTest::parseEnvironmentUtf8,

              &ArgumentsTest::parseUnknownArgument,
              &ArgumentsTest::parseUnknownShortArgument,
              &ArgumentsTest::parseSuperfluousArgument,
              &ArgumentsTest::parseArgumentAfterSeparator,
              &ArgumentsTest::parseInvalidShortArgument,
              &ArgumentsTest::parseInvalidLongArgument,
              &ArgumentsTest::parseInvalidLongArgumentDashes,

              &ArgumentsTest::parseMissingValue,
              &ArgumentsTest::parseMissingOption,
              &ArgumentsTest::parseMissingArgument,

              &ArgumentsTest::prefixedParse,
              &ArgumentsTest::prefixedParseMinus,
              &ArgumentsTest::prefixedParseMinusMinus,
              &ArgumentsTest::prefixedParseHelpArgument,
              &ArgumentsTest::prefixedHelpWithoutPrefix,
              &ArgumentsTest::prefixedHelpWithPrefix,
              &ArgumentsTest::prefixedHelpLongPrefix,
              &ArgumentsTest::prefixedDisallowedCalls,
              &ArgumentsTest::prefixedDisallowedWithPrefix,
              &ArgumentsTest::prefixedDisallowedWithPrefixAfterSkipPrefix,
              &ArgumentsTest::prefixedUnknownWithPrefix,
              &ArgumentsTest::prefixedInvalidPrefixedName,
              &ArgumentsTest::prefixedInvalidUnprefixedName});
}

bool hasEnv(const std::string& value) {
    if(std::getenv(value.data())) return true;

    std::vector<std::string> list = Arguments::environment();
    return std::find_if(list.begin(), list.end(),
    [&value](const std::string& v){ return String::beginsWith(v, value); }) != list.end();
}

void ArgumentsTest::environment() {
    #ifdef CORRADE_TARGET_WINDOWS_RT
    CORRADE_SKIP("No environment on this platform.");
    #endif

    /* Verify that it doesn't crash, at least */
    std::vector<std::string> list = Arguments::environment();
    if(!list.empty()) Debug()
        << "Environment variables found:" << list.size() << Debug::newline
        << "One environment variable:" << list[list.size()/2];

    CORRADE_VERIFY(!list.empty());
}

void ArgumentsTest::environmentUtf8() {
    #ifdef CORRADE_TARGET_WINDOWS_RT
    CORRADE_SKIP("No environment on this platform.");
    #endif

    if(!hasEnv("ARGUMENTSTEST_UNICODE"))
        CORRADE_SKIP("Environment not set. Call the test with ARGUMENTSTEST_UNICODE=hýždě to enable this test case.");

    /* Verify that it doesn't crash, at least */
    std::vector<std::string> list = Arguments::environment();
    auto found = std::find_if(list.begin(), list.end(),
        [](const std::string& v){ return String::beginsWith(v, "ARGUMENTSTEST_UNICODE="); });
    CORRADE_VERIFY(found != list.end());
    CORRADE_COMPARE(*found, "ARGUMENTSTEST_UNICODE=hýždě");
}

void ArgumentsTest::copy() {
    CORRADE_VERIFY(!std::is_copy_constructible<Arguments>::value);
    CORRADE_VERIFY(!std::is_copy_assignable<Arguments>::value);
}

void ArgumentsTest::move() {
    const char* argv[] = {"", "--prefix-bar", "hey"};

    Arguments args{"prefix"};
    args.addOption("bar");

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));

    Arguments args2{std::move(args)};
    CORRADE_COMPARE(args2.value("bar"), "hey");

    CORRADE_VERIFY(!args.isParsed());
    CORRADE_COMPARE(args.prefix(), "");

    Arguments args3{"another"};
    args3 = std::move(args2);
    CORRADE_VERIFY(!args2.isParsed());
    CORRADE_COMPARE(args2.prefix(), "another");
    CORRADE_COMPARE(args3.value("bar"), "hey");

    /* Everything should work well even after two moves */
    CORRADE_VERIFY(args3.tryParse(Containers::arraySize(argv), argv));
}

void ArgumentsTest::helpArgumentsOnly() {
    Arguments args;
    args.addArgument("foo").setHelp("foo", "which foo to bar")
        .addArgument("bar").setHelp("bar", {}, "output.bin")
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
        .addNamedArgument('b', "baz").setHelp("baz", {}, "LEVEL")
        .addOption("sanity-level", "INSANE").setHelp("sanity-level", {}, "SANITY")
        .addBooleanOption("no-bare-foos").setHelp("no-bare-foos", "don't use bare foos")
        .setCommand("foobar");

    const auto expected = R"text(Usage:
  foobar [-h|--help] [-n|--bars BARS] -b|--baz LEVEL [--sanity-level SANITY] [--no-bare-foos]

Arguments:
  -h, --help             display this help message and exit
  -n, --bars BARS        number of bars to foo
                         (default: 42)
  --sanity-level SANITY  (default: INSANE)
  --no-bare-foos         don't use bare foos
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
        .setGlobalHelp("Bars with given foo.");

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

void ArgumentsTest::helpEnvironment() {
    Arguments args;
    args.addOption("use-FOO").setFromEnvironment("use-FOO")
        .addBooleanOption("avoid-bars").setFromEnvironment("avoid-bars");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--use-FOO USE_FOO] [--avoid-bars]

Arguments:
  -h, --help         display this help message and exit
  --use-FOO USE_FOO  (environment: USE_FOO)
  --avoid-bars       (environment: AVOID_BARS=ON|OFF)
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpEnvironmentPrefixed() {
    Arguments args{"well"};
    args.addOption("use-foo").setHelp("use-foo", "well, use foo", "BAR").setFromEnvironment("use-foo");

    const auto expected = R"text(Usage:
  ./app [--well-help] [--well-use-foo BAR] ...

Arguments:
  ...                 main application arguments
                      (see -h or --help for details)
  --well-help         display this help message and exit
  --well-use-foo BAR  well, use foo
                      (environment: WELL_USE_FOO)
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpAfterParse() {
    Arguments args;

    const char* argv[] = { "foobar" };

    /* Take command name from argv */
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    const auto expected = R"text(Usage:
  foobar [-h|--help]
)text";
    CORRADE_COMPARE(args.usage(), expected);

    /* If set custom command name, don't override */
    args.setCommand("myFoobarApp");
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    const auto expected2 = R"text(Usage:
  myFoobarApp [-h|--help]
)text";
    CORRADE_COMPARE(args.usage(), expected2);
}

void ArgumentsTest::helpLongKeys() {
    Arguments args;
    args.addArgument("some-insanely-long-argument").setHelp("some-insanely-long-argument", "this is long, right?")
        .addBooleanOption("some-crazy-long-option-ya").setHelp("some-crazy-long-option-ya", "long is the new short")
        .addOption('X', "another-long-option").setHelp("another-long-option", "loooong", "F");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--some-crazy-long-option-ya] [-X|--another-long-option F] [--] some-insanely-long-argument

Arguments:
  some-insanely-long-argument  this is long, right?
  -h, --help                  display this help message and exit
  --some-crazy-long-option-ya  long is the new short
  -X, --another-long-option F  loooong
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpLongKeyNotPrinted() {
    Arguments args;
    args.addArgument("some-really-long-option-that-will-not-get-printed-anyway");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--] some-really-long-option-that-will-not-get-printed-anyway

Arguments:
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::duplicateKey() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addArgument("foo")
        .addOption("foo");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addOption(): the key foo or its short version is already used\n");
}

void ArgumentsTest::duplicateShortKey() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addNamedArgument('b', "bar")
        .addBooleanOption('b', "foo");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addBooleanOption(): the key foo or its short version is already used\n");
}

void ArgumentsTest::disallowedCharacter() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addNamedArgument('b', "a mistake");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addNamedArgument(): invalid key a mistake or its short variant\n");
}

void ArgumentsTest::disallowedCharacterShort() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addOption(' ', "bar");

    CORRADE_COMPARE(out.str(), "Utility::Arguments::addOption(): invalid key bar or its short variant\n");
}

void ArgumentsTest::notParsedYet() {
    std::ostringstream out;
    Error redirectError{&out};

    Arguments args;
    args.addOption("value")
        .addBooleanOption("boolean");

    args.value("value");
    args.isSet("boolean");

    CORRADE_VERIFY(!args.isParsed());
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::value(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::isSet(): arguments were not successfully parsed yet\n");
}

void ArgumentsTest::notParsedYetOnlyHelp() {
    std::ostringstream out;
    Error redirectError{&out};

    const char* argv[] = { "", "--help" };

    Arguments args;
    args.addArgument("value")
        .addBooleanOption("boolean");

    /* parse() should not succeed if there is --help but some arguments were
       not specified */
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));

    args.value("value");
    args.isSet("boolean");

    CORRADE_VERIFY(!args.isParsed());
    CORRADE_COMPARE(out.str(),
        "Missing command-line argument value\n"
        "Utility::Arguments::value(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::isSet(): arguments were not successfully parsed yet\n");
}

void ArgumentsTest::parseNullptr() {
    Arguments args;

    CORRADE_VERIFY(args.tryParse(0, nullptr));
}

void ArgumentsTest::parseHelp() {
    Arguments args;
    args.addBooleanOption("no-foo-bars");

    const char* argv[] = { "", "-h", "--no-foo-bars", "error" };

    /* The parse() will not exit if help is set, but tryParse() should indicate
       the error */
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
}

void ArgumentsTest::parseArguments() {
    Arguments args;
    args.addArgument("name")
        .addArgument("input")
        .addArgument("output");

    const char* argv[] = { "", "hello", "in.txt", "out.bin" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
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

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
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

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value<float>("pi"), 3.141516f);
}

void ArgumentsTest::parseCustomTypeFlags() {
    Arguments args;
    args.addNamedArgument("key");

    const char* argv[] = { "", "--key", "0xdeadbeef" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value<unsigned int>("key", ConfigurationValueFlag::Hex), 0xdeadbeef);
}

void ArgumentsTest::parseDoubleArgument() {
    Arguments args;
    args.addNamedArgument("arg")
        .addBooleanOption('b', "bool");

    const char* argv[] = { "", "--arg", "first", "-b", "--arg", "second", "-b" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("arg"), "second");
    CORRADE_VERIFY(args.isSet("bool"));
}

void ArgumentsTest::parseUnknownArgument() {
    Arguments args;

    const char* argv[] = { "", "--error" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument --error\n");
}

void ArgumentsTest::parseUnknownShortArgument() {
    Arguments args;

    const char* argv[] = { "", "-e" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument -e\n");
}

void ArgumentsTest::parseSuperfluousArgument() {
    Arguments args;

    const char* argv[] = { "", "error" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument error\n");
}

void ArgumentsTest::parseArgumentAfterSeparator() {
    Arguments args;
    args.addBooleanOption('b', "bar");

    const char* argv[] = { "", "--", "-b" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument -b\n");
}

void ArgumentsTest::parseInvalidShortArgument() {
    Arguments args;

    const char* argv[] = { "", "-?" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument -?\n");
}

void ArgumentsTest::parseInvalidLongArgument() {
    Arguments args;

    const char* argv[] = { "", "--?" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument --?\n");
}

void ArgumentsTest::parseInvalidLongArgumentDashes() {
    Arguments args;

    const char* argv[] = { "", "-long-argument" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument -long-argument (did you mean --long-argument?)\n");
}

void ArgumentsTest::parseMissingValue() {
    Arguments args;
    args.addOption("output");

    const char* argv[] = { "", "--output" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing value for command-line argument --output\n");
}

void ArgumentsTest::parseMissingOption() {
    Arguments args;
    args.addNamedArgument("output");

    const char* argv[] = { "" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument --output\n");
}

void ArgumentsTest::parseMissingArgument() {
    Arguments args;
    args.addArgument("file").setHelp("file", "", "file.dat");

    const char* argv[] = { "" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument file.dat\n");
}

void ArgumentsTest::parseEnvironment() {
    #ifdef CORRADE_TARGET_WINDOWS_RT
    CORRADE_SKIP("No environment on this platform.");
    #else
    if(!hasEnv("ARGUMENTSTEST_SIZE") || !hasEnv("ARGUMENTSTEST_VERBOSE") || !hasEnv("ARGUMENTSTEST_COLOR"))
        CORRADE_SKIP("Environment not set. Call the test with ARGUMENTSTEST_SIZE=1337 ARGUMENTSTEST_VERBOSE=ON ARGUMENTTEST_COLOR=OFF to enable this test case.");

    Arguments args;
    args.addOption("size").setFromEnvironment("size", "ARGUMENTSTEST_SIZE")
        .addBooleanOption("verbose").setFromEnvironment("verbose", "ARGUMENTSTEST_VERBOSE")
        .addBooleanOption("color").setFromEnvironment("color", "ARGUMENTSTEST_COLOR");

    const char* argv[] = { "" };

    /* Set from environment by CTest */
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("size"), "1337");
    CORRADE_VERIFY(args.isSet("verbose"));
    CORRADE_VERIFY(!args.isSet("color"));
    #endif
}

void ArgumentsTest::parseEnvironmentUtf8() {
    #ifdef CORRADE_TARGET_WINDOWS_RT
    CORRADE_SKIP("No environment on this platform.");
    #else
    if(!hasEnv("ARGUMENTSTEST_UNICODE"))
        CORRADE_SKIP("Environment not set. Call the test with ARGUMENTSTEST_UNICODE=hýždě to enable this test case.");

    Arguments args;
    args.addOption("unicode").setFromEnvironment("unicode", "ARGUMENTSTEST_UNICODE");

    const char* argv[] = { "" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("unicode"), "hýždě");
    #endif
}

void ArgumentsTest::prefixedParse() {
    Arguments arg1;
    arg1.addArgument("file")
        .addBooleanOption('b', "binary")
        .addOption("speed")
        .addSkippedPrefix("read");

    Arguments arg2{"read"};
    arg2.addOption("behavior")
        .addOption("buffer-size");

    CORRADE_COMPARE(arg1.prefix(), "");
    CORRADE_COMPARE(arg2.prefix(), "read");

    const char* argv[] = { "", "-b", "--read-behavior", "buffered", "--speed", "fast", "--binary", "--read-buffer-size", "4K", "file.dat" };

    CORRADE_VERIFY(arg1.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(arg1.isSet("binary"));
    CORRADE_COMPARE(arg1.value("speed"), "fast");
    CORRADE_COMPARE(arg1.value("file"), "file.dat");

    CORRADE_VERIFY(arg2.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg2.value("behavior"), "buffered");
    CORRADE_COMPARE(arg2.value("buffer-size"), "4K");
}

void ArgumentsTest::prefixedParseMinus() {
    Arguments arg1;
    arg1.addNamedArgument("offset")
        .addSkippedPrefix("read");

    Arguments arg2{"read"};
    arg2.addOption("behavior")
        .addOption("buffer-size");

    const char* argv[] = { "", "--read-behavior", "buffered", "--offset", "-50" };

    CORRADE_VERIFY(arg1.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg1.value("offset"), "-50");

    CORRADE_VERIFY(arg2.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg2.value("behavior"), "buffered");
}

void ArgumentsTest::prefixedParseMinusMinus() {
    Arguments arg1;
    arg1.addNamedArgument("offset")
        .addSkippedPrefix("read");

    Arguments arg2{"read"};
    arg2.addOption("behavior")
        .addOption("buffer-size");

    const char* argv[] = { "", "--read-behavior", "buffered", "--offset", "--50" };

    CORRADE_VERIFY(arg1.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg1.value("offset"), "--50");

    CORRADE_VERIFY(arg2.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg2.value("behavior"), "buffered");
}

void ArgumentsTest::prefixedParseHelpArgument() {
    /* Prefixed can be only non-boolean options except for help, test that the
       actual argument doesn't get skipped if immediately after the help
       boolean option */

    Arguments arg1;
    arg1.addBooleanOption('b', "binary")
        .addSkippedPrefix("reader");

    const char* argv[] = { "", "--reader-help", "-b" };

    CORRADE_VERIFY(arg1.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(arg1.isSet("binary"));
}

void ArgumentsTest::prefixedHelpWithoutPrefix() {
    Arguments args;
    args.addArgument("file").setHelp("file", "file to read")
        .addBooleanOption('b', "binary").setHelp("binary", "read as binary")
        .addSkippedPrefix("read", "reader options")
        .addSkippedPrefix("write");

    const auto expected = R"text(Usage:
  ./app [--read-...] [--write-...] [-h|--help] [-b|--binary] [--] file

Arguments:
  file          file to read
  -h, --help    display this help message and exit
  -b, --binary  read as binary
  --read-...    reader options
                (see --read-help for details)
  --write-...   (see --write-help for details)
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::prefixedHelpWithPrefix() {
    Arguments args{"read"};
    args.addOption("behavior", "buffered").setHelp("behavior", "reader behavior")
        .addOption("buffer-size").setHelp("buffer-size", "buffer size", "SIZE");

    const auto expected = R"text(Usage:
  ./app [--read-help] [--read-behavior BEHAVIOR] [--read-buffer-size SIZE] ...

Arguments:
  ...                       main application arguments
                            (see -h or --help for details)
  --read-help               display this help message and exit
  --read-behavior BEHAVIOR  reader behavior
                            (default: buffered)
  --read-buffer-size SIZE   buffer size
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::prefixedHelpLongPrefix() {
    Arguments args;
    args.addSkippedPrefix("a-kinda-longer-prefix", "this is long, right?");

    const auto expected = R"text(Usage:
  ./app [--a-kinda-longer-prefix-...] [-h|--help]

Arguments:
  -h, --help                  display this help message and exit
  --a-kinda-longer-prefix-...  this is long, right?
                              (see --a-kinda-longer-prefix-help for details)
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::prefixedDisallowedCalls() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args{"reader"};
    args.addArgument("foo")
        .addNamedArgument("bar")
        .addOption('a', "baz")
        .addBooleanOption("eh")
        .setGlobalHelp("global help");

    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addArgument(): argument foo not allowed in prefixed version\n"
        "Utility::Arguments::addNamedArgument(): argument bar not allowed in prefixed version\n"
        "Utility::Arguments::addOption(): short option a not allowed in prefixed version\n"
        "Utility::Arguments::addBooleanOption(): boolean option eh not allowed in prefixed version\n"
        "Utility::Arguments::setGlobalHelp(): global help text only allowed in unprefixed version\n");
}

void ArgumentsTest::prefixedDisallowedWithPrefix() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addOption("reader-flush")
        .addSkippedPrefix("reader");

    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addSkippedPrefix(): skipped prefix reader conflicts with existing keys\n");
}

void ArgumentsTest::prefixedDisallowedWithPrefixAfterSkipPrefix() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addSkippedPrefix("reader")
        .addOption("reader-flush");

    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addOption(): key reader-flush conflicts with skipped prefixes\n");
}

void ArgumentsTest::prefixedUnknownWithPrefix() {
    std::ostringstream out;
    Error redirectError{&out};
    Arguments args{"reader"};
    args.addOption("bar");

    const char* argv[] = { "", "--reader-foo", "hello", "--something", "other" };

    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument --reader-foo\n");
}

void ArgumentsTest::prefixedInvalidPrefixedName() {
    Arguments args;
    args.addSkippedPrefix("reader")
        .addOption("foo");

    /* The prefixed options might be parsed with something that's more
       forgiving about what is valid in an argument, so be cool about that */
    const char* argv[] = { "", "--reader-?", "hello", "--foo", "yes" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("foo"), "yes");
}

void ArgumentsTest::prefixedInvalidUnprefixedName() {
    Arguments args{"reader"};
    args.addOption("foo");

    /* The unprefixed options might be parsed with something that's more
       forgiving about what is valid in an argument, so be cool about that */
    const char* argv[] = { "", "--?", "hello", "--reader-foo", "yes" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("foo"), "yes");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ArgumentsTest)
