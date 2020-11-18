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

#include <algorithm>
#include <sstream>

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/Arguments.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/String.h"

namespace Corrade { namespace Utility {

namespace {

enum class UsingContainersString: int { Value = 3 };

}

template<> struct ConfigurationValue<UsingContainersString> {
    ConfigurationValue() = delete;

    static UsingContainersString fromString(const Containers::StringView& stringValue, ConfigurationValueFlags) {
        return stringValue == "three" ? UsingContainersString::Value : UsingContainersString{};
    }
};

namespace Test { namespace {

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
    void helpFinalOptionalArgument();
    void helpFinalOptionalArgumentDefaultValueOnly();
    void setHelpNotFound();
    void setHelpKeyForBoolean();

    void duplicateKey();
    void duplicateShortKey();
    void emptyKey();
    void disallowedCharacter();
    void disallowedCharacterShort();
    void disallowedIgnoreUnknown();
    void arrayArgumentTwice();
    void finalOptionalArgumentTwice();
    void finalOptionalArgumentWithArray();
    void argumentAfterFinalOptionalArgument();
    void arrayArgumentAfterFinalOptionalArgument();

    void parseNullptr();
    void parseHelp();
    void parseArguments();
    void parseMixed();
    void parseStringView();
    void parseCustomType();
    void parseCustomTypeFlags();
    void parseCustomTypeUsingContainersString();
    void parseEnvironment();
    void parseEnvironmentUtf8();
    void parseFinalOptionalArgument();
    void parseFinalOptionalArgumentDefault();

    void parseShortOptionValuePack();
    void parseShortOptionValuePackEmpty();
    void parseShortBooleanOptionPack();
    void parseShortBooleanOptionValuePack();

    void parseArrayArguments();
    void parseArrayOptions();

    void parseUnknownArgument();
    void parseUnknownShortArgument();
    void parseSuperfluousArgument();
    void parseSingleDash();
    void parseArgumentAfterSeparator();
    void parseInvalidShortArgument();
    void parseInvalidLongArgument();
    void parseInvalidLongArgumentDashes();

    void parseMissingValue();
    void parseMissingOption();
    void parseMissingArgument();
    void parseMissingArrayArgumentMiddle();
    void parseMissingArrayArgumentLast();

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
    void prefixedIgnoreUnknown();
    void prefixedIgnoreUnknownInvalidPrefixedName();

    void notParsedYet();
    void notParsedYetOnlyHelp();
    void valueNotFound();
    void valueMismatchedUse();
    void arrayValueOutOfBounds();

    void parseErrorCallback();
    void parseErrorCallbackIgnoreAll();
    void parseErrorCallbackIgnoreAll2();

    void debugParseError();
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
              &ArgumentsTest::helpFinalOptionalArgument,
              &ArgumentsTest::helpFinalOptionalArgumentDefaultValueOnly,
              &ArgumentsTest::setHelpNotFound,
              &ArgumentsTest::setHelpKeyForBoolean,

              &ArgumentsTest::duplicateKey,
              &ArgumentsTest::duplicateShortKey,
              &ArgumentsTest::emptyKey,
              &ArgumentsTest::disallowedCharacter,
              &ArgumentsTest::disallowedCharacterShort,
              &ArgumentsTest::disallowedIgnoreUnknown,
              &ArgumentsTest::arrayArgumentTwice,
              &ArgumentsTest::finalOptionalArgumentTwice,
              &ArgumentsTest::finalOptionalArgumentWithArray,
              &ArgumentsTest::argumentAfterFinalOptionalArgument,
              &ArgumentsTest::arrayArgumentAfterFinalOptionalArgument,

              &ArgumentsTest::parseNullptr,
              &ArgumentsTest::parseHelp,
              &ArgumentsTest::parseArguments,
              &ArgumentsTest::parseMixed,
              &ArgumentsTest::parseStringView,
              &ArgumentsTest::parseCustomType,
              &ArgumentsTest::parseCustomTypeFlags,
              &ArgumentsTest::parseCustomTypeUsingContainersString,
              &ArgumentsTest::parseEnvironment,
              &ArgumentsTest::parseEnvironmentUtf8,
              &ArgumentsTest::parseFinalOptionalArgument,
              &ArgumentsTest::parseFinalOptionalArgumentDefault,

              &ArgumentsTest::parseShortOptionValuePack,
              &ArgumentsTest::parseShortOptionValuePackEmpty,
              &ArgumentsTest::parseShortBooleanOptionPack,
              &ArgumentsTest::parseShortBooleanOptionValuePack,

              &ArgumentsTest::parseArrayArguments,
              &ArgumentsTest::parseArrayOptions,

              &ArgumentsTest::parseUnknownArgument,
              &ArgumentsTest::parseUnknownShortArgument,
              &ArgumentsTest::parseSuperfluousArgument,
              &ArgumentsTest::parseSingleDash,
              &ArgumentsTest::parseArgumentAfterSeparator,
              &ArgumentsTest::parseInvalidShortArgument,
              &ArgumentsTest::parseInvalidLongArgument,
              &ArgumentsTest::parseInvalidLongArgumentDashes,

              &ArgumentsTest::parseMissingValue,
              &ArgumentsTest::parseMissingOption,
              &ArgumentsTest::parseMissingArgument,
              &ArgumentsTest::parseMissingArrayArgumentMiddle,
              &ArgumentsTest::parseMissingArrayArgumentLast,

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
              &ArgumentsTest::prefixedInvalidUnprefixedName,
              &ArgumentsTest::prefixedIgnoreUnknown,
              &ArgumentsTest::prefixedIgnoreUnknownInvalidPrefixedName,

              &ArgumentsTest::notParsedYet,
              &ArgumentsTest::notParsedYetOnlyHelp,
              &ArgumentsTest::valueNotFound,
              &ArgumentsTest::valueMismatchedUse,
              &ArgumentsTest::arrayValueOutOfBounds,

              &ArgumentsTest::parseErrorCallback,
              &ArgumentsTest::parseErrorCallbackIgnoreAll,
              &ArgumentsTest::parseErrorCallbackIgnoreAll2,

              &ArgumentsTest::debugParseError});
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
    args.addArgument("foo")
        .addArrayArgument("input").setHelp("input", "one or more inputs", "files")
        .addArgument("bar").setHelp("bar", "where to put things", "output.bin.gz")
        .setCommand("foobar");

    const auto expected = R"text(Usage:
  foobar [-h|--help] [--] foo files... output.bin.gz

Arguments:
  files          one or more inputs
  output.bin.gz  where to put things
  -h, --help     display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpNamedOnly() {
    Arguments args;
    args.addOption('n', "bars", "42").setHelp("bars", "number of bars to foo")
        .addNamedArgument('b', "baz").setHelp("baz", {}, "LEVEL")
        .addOption("sanity-level", "INSANE").setHelp("sanity-level", {}, "SANITY")
        .addArrayOption("name").setHelp("name", "all names to use", "Ni")
        .addBooleanOption("no-bare-foos").setHelp("no-bare-foos", "don't use bare foos")
        .setCommand("foobar");

    const auto expected = R"text(Usage:
  foobar [-h|--help] [-n|--bars BARS] -b|--baz LEVEL [--sanity-level SANITY] [--name Ni]... [--no-bare-foos]

Arguments:
  -h, --help             display this help message and exit
  -n, --bars BARS        number of bars to foo
                         (default: 42)
  --sanity-level SANITY  (default: INSANE)
  --name Ni              all names to use
  --no-bare-foos         don't use bare foos
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpBoth() {
    Arguments args;
    args.addArgument("foo").setHelp("foo", "which foo to bar with")
        .addArrayOption("name").setHelp("name", "name(s) to use")
        .addBooleanOption('B', "no-bars").setHelp("no-bars", "don't foo with bars");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--name NAME]... [-B|--no-bars] [--] foo

Arguments:
  foo            which foo to bar with
  -h, --help     display this help message and exit
  --name NAME    name(s) to use
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

void ArgumentsTest::helpFinalOptionalArgument() {
    Arguments args;
    args.addArgument("undocumented")
        .addFinalOptionalArgument("optional", "42").setHelp("optional", "the help", "answer");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--] undocumented [answer]

Arguments:
  answer      the help
              (default: 42)
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::helpFinalOptionalArgumentDefaultValueOnly() {
    Arguments args;
    args.addFinalOptionalArgument("optional", "42");

    const auto expected = R"text(Usage:
  ./app [-h|--help] [--] [optional]

Arguments:
  optional    (default: 42)
  -h, --help  display this help message and exit
)text";
    CORRADE_COMPARE(args.help(), expected);
}

void ArgumentsTest::setHelpNotFound() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;

    std::ostringstream out;
    Error redirectError{&out};
    args.setHelp("opt", "this is an option");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::setHelp(): key opt not found\n");
}

void ArgumentsTest::setHelpKeyForBoolean() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;

    std::ostringstream out;
    Error redirectError{&out};
    args.setHelp("help", "this very thing", "HALP");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::setHelp(): help key can't be set for boolean option help\n");
}

void ArgumentsTest::duplicateKey() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    args.addArgument("foo");

    std::ostringstream out;
    Error redirectError{&out};
    args.addArgument("foo")
        .addArrayArgument("foo")
        .addNamedArgument("foo")
        .addOption("foo")
        .addArrayOption("foo")
        .addBooleanOption("foo")
        .addFinalOptionalArgument("foo");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addArgument(): the key foo is already used\n"
        "Utility::Arguments::addArrayArgument(): the key foo is already used\n"
        "Utility::Arguments::addNamedArgument(): the key foo or its short variant is already used\n"
        "Utility::Arguments::addOption(): the key foo or its short variant is already used\n"
        "Utility::Arguments::addArrayOption(): the key foo or its short variant is already used\n"
        "Utility::Arguments::addBooleanOption(): the key foo or its short variant is already used\n"
        "Utility::Arguments::addFinalOptionalArgument(): the key foo is already used\n");
}

void ArgumentsTest::duplicateShortKey() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    args.addNamedArgument('b', "bar");

    std::ostringstream out;
    Error redirectError{&out};
    args.addNamedArgument('b', "foo")
        .addOption('b', "fig")
        .addArrayOption('b', "plop")
        .addBooleanOption('b', "bur");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addNamedArgument(): the key foo or its short variant is already used\n"
        "Utility::Arguments::addOption(): the key fig or its short variant is already used\n"
        "Utility::Arguments::addArrayOption(): the key plop or its short variant is already used\n"
        "Utility::Arguments::addBooleanOption(): the key bur or its short variant is already used\n");
}

void ArgumentsTest::emptyKey() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;

    std::ostringstream out;
    Error redirectError{&out};
    args.addArgument("")
        .addArrayArgument("")
        .addNamedArgument("")
        .addOption("")
        .addArrayOption("")
        .addBooleanOption("")
        .addFinalOptionalArgument("");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addArgument(): key can't be empty\n"
        "Utility::Arguments::addArrayArgument(): key can't be empty\n"
        "Utility::Arguments::addNamedArgument(): invalid key  or its short variant\n"
        "Utility::Arguments::addOption(): invalid key  or its short variant\n"
        "Utility::Arguments::addArrayOption(): invalid key  or its short variant\n"
        "Utility::Arguments::addBooleanOption(): invalid key  or its short variant\n"
        "Utility::Arguments::addFinalOptionalArgument(): key can't be empty\n");
}

void ArgumentsTest::disallowedCharacter() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    /* It's fine here (even though confusing) -- the user won't be typing this
       on the terminal */
    args.addArgument("well, actually")
        .addFinalOptionalArgument("i'm saying");

    std::ostringstream out;
    Error redirectError{&out};
    args
        .addNamedArgument("a mistake")
        .addOption("it is")
        .addArrayOption("tru ly")
        .addBooleanOption("really!");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addNamedArgument(): invalid key a mistake or its short variant\n"
        "Utility::Arguments::addOption(): invalid key it is or its short variant\n"
        "Utility::Arguments::addArrayOption(): invalid key tru ly or its short variant\n"
        "Utility::Arguments::addBooleanOption(): invalid key really! or its short variant\n");
}

void ArgumentsTest::disallowedCharacterShort() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args
        .addNamedArgument('-', "dash")
        .addOption(' ', "bar")
        .addArrayOption('#', "hash")
        .addBooleanOption('?', "halp");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addNamedArgument(): invalid key dash or its short variant\n"
        "Utility::Arguments::addOption(): invalid key bar or its short variant\n"
        "Utility::Arguments::addArrayOption(): invalid key hash or its short variant\n"
        "Utility::Arguments::addBooleanOption(): invalid key halp or its short variant\n");
}

void ArgumentsTest::disallowedIgnoreUnknown() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args{Arguments::Flag::IgnoreUnknownOptions};
    CORRADE_COMPARE(out.str(), "Utility::Arguments: Flag::IgnoreUnknownOptions allowed only in the prefixed variant\n");
}

void ArgumentsTest::arrayArgumentTwice() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addArrayArgument("first")
        .addArrayArgument("second");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::addArrayArgument(): there's already an array argument first\n");
}

void ArgumentsTest::finalOptionalArgumentTwice() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addFinalOptionalArgument("first")
        .addFinalOptionalArgument("second");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::addFinalOptionalArgument(): there's already a final optional argument first\n");
}

void ArgumentsTest::finalOptionalArgumentWithArray() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addArrayArgument("first")
        .addFinalOptionalArgument("second");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::addFinalOptionalArgument(): there's already an array argument first\n");
}

void ArgumentsTest::argumentAfterFinalOptionalArgument() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addFinalOptionalArgument("arg")
        .addArgument("bla");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::addArgument(): can't add more arguments after the final optional one\n");
}

void ArgumentsTest::arrayArgumentAfterFinalOptionalArgument() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addFinalOptionalArgument("arg")
        .addArrayArgument("bla");
    CORRADE_COMPARE(out.str(), "Utility::Arguments::addArrayArgument(): can't add more arguments after the final optional one\n");
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

void ArgumentsTest::parseStringView() {
    Arguments args;
    args.addArgument("stuff")
        .addArrayOption('O', "other");

    const char* argv[] = { "", "hello this is a string", "-O", "hello this also", "--other", "it should not be dangling" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value<Containers::StringView>("stuff"), "hello this is a string");
    CORRADE_COMPARE(args.arrayValueCount("other"), 2);
    CORRADE_COMPARE(args.arrayValue<Containers::StringView>("other", 1), "it should not be dangling");
}

void ArgumentsTest::parseCustomType() {
    Arguments args;
    args.addNamedArgument("pi")
        .addArrayOption('F', "fibonacci");

    const char* argv[] = { "", "--pi", "0.3141516e+1", "-F", "0", "--fibonacci", "1", "-F", "1", "-F", "2" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value<float>("pi"), 3.141516f);
    CORRADE_COMPARE(args.arrayValueCount("fibonacci"), 4);
    CORRADE_COMPARE(args.arrayValue<int>("fibonacci", 3), 2);
}

void ArgumentsTest::parseCustomTypeFlags() {
    Arguments args;
    args.addNamedArgument("key")
        .addArrayOption('M', "mod");

    const char* argv[] = { "", "--key", "0xdeadbeef", "-M", "0644"};

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value<unsigned int>("key", ConfigurationValueFlag::Hex), 0xdeadbeef);
    CORRADE_COMPARE(args.arrayValue<int>("mod", 0, ConfigurationValueFlag::Oct), 0644);
}

void ArgumentsTest::parseCustomTypeUsingContainersString() {
    Arguments args;
    args.addArgument("value");

    const char* argv[] = { "", "three" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(int(args.value<UsingContainersString>("value")), int(UsingContainersString::Value));
}

void ArgumentsTest::parseEnvironment() {
    #ifdef CORRADE_TARGET_WINDOWS_RT
    CORRADE_SKIP("No environment on this platform.");
    #else
    if(!hasEnv("ARGUMENTSTEST_SIZE") || !hasEnv("ARGUMENTSTEST_VERBOSE") || !hasEnv("ARGUMENTSTEST_COLOR"))
        CORRADE_SKIP("Environment not set. Call the test with ARGUMENTSTEST_SIZE=1337 ARGUMENTSTEST_VERBOSE=ON ARGUMENTSTEST_COLOR=OFF to enable this test case.");

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

void ArgumentsTest::parseFinalOptionalArgument() {
    Arguments args;
    args.addArgument("input")
        .addFinalOptionalArgument("output")
        .addOption('x', "language")
        .addBooleanOption("debug");

    const char* argv[] = { "", "main.cpp", "-x", "c++", "a.out", "--debug" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("input"), "main.cpp");
    CORRADE_COMPARE(args.value("output"), "a.out");
    CORRADE_COMPARE(args.value("language"), "c++");
    CORRADE_VERIFY(args.isSet("debug"));
}

void ArgumentsTest::parseFinalOptionalArgumentDefault() {
    Arguments args;
    args.addArgument("input")
        .addFinalOptionalArgument("output", "a.out")
        .addOption('x', "language")
        .addBooleanOption("debug");

    const char* argv[] = { "", "main.cpp", "-x", "c++", "--debug" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("input"), "main.cpp");
    CORRADE_COMPARE(args.value("output"), "a.out");
    CORRADE_COMPARE(args.value("language"), "c++");
    CORRADE_VERIFY(args.isSet("debug"));
}

void ArgumentsTest::parseShortOptionValuePack() {
    Arguments args;
    args.addOption('D', "define")
        .addArgument("input");

    /* The argument after is to test that the short option pack offset got
       reset correctly */
    const char* argv[] = { "", "-DNDEBUG", "main.cpp" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("define"), "NDEBUG");
    CORRADE_COMPARE(args.value("input"), "main.cpp");
}

void ArgumentsTest::parseShortOptionValuePackEmpty() {
    Arguments args;
    args.addOption('D', "define")
        .addArgument("input");

    const char* argv[] = { "", "-D", "main.cpp" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument input\n");
}

void ArgumentsTest::parseShortBooleanOptionPack() {
    Arguments args;
    args.addBooleanOption('S', "sync")
        .addBooleanOption('y', "refresh")
        .addBooleanOption('u', "sysupgrade")
        .addArgument("package");

    /* The argument after is to test that the short option pack offset got
       reset correctly */
    const char* argv[] = { "", "-Syu", "magnum" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(args.isSet("sync"));
    CORRADE_VERIFY(args.isSet("refresh"));
    CORRADE_VERIFY(args.isSet("sysupgrade"));
    CORRADE_COMPARE(args.value("package"), "magnum");
}

void ArgumentsTest::parseShortBooleanOptionValuePack() {
    Arguments args;
    args.addBooleanOption('S', "sync")
        .addBooleanOption('y', "refresh")
        .addOption('s', "search")
        .addArgument("package");

    /* The argument after is to test that the short option pack offset got
       reset correctly */
    const char* argv[] = { "", "-Sysmagnum", "corrade" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(args.isSet("sync"));
    CORRADE_VERIFY(args.isSet("refresh"));
    CORRADE_COMPARE(args.value("search"), "magnum");
    CORRADE_COMPARE(args.value("package"), "corrade");
}

void ArgumentsTest::parseArrayArguments() {
    Arguments args;
    args.addArrayOption("error") /* only to verify the array values are not
                                    overwriting each other */
        .addArgument("mode")
        .addArrayArgument("input")
        .addArgument("output")
        .addArgument("logfile");

    const char* argv[] = { "", "compress", "a.txt", "b.jpg", "c.cpp", "data.zip", "data.log", "--error", "never" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("mode"), "compress");
    CORRADE_COMPARE(args.arrayValueCount("input"), 3);
    CORRADE_COMPARE(args.arrayValue("input", 0), "a.txt");
    CORRADE_COMPARE(args.arrayValue("input", 1), "b.jpg");
    CORRADE_COMPARE(args.arrayValue("input", 2), "c.cpp");
    CORRADE_COMPARE(args.value("output"), "data.zip");
    CORRADE_COMPARE(args.value("logfile"), "data.log");

    CORRADE_COMPARE(args.arrayValueCount("error"), 1);
    CORRADE_COMPARE(args.arrayValue("error", 0), "never");
}

void ArgumentsTest::parseArrayOptions() {
    Arguments args;
    args.addArrayArgument("input") /* only to verify the array values are not
                                      overwriting each other */
        .addNamedArgument("arg")
        .addBooleanOption('b', "bool")
        .addArrayOption('F', "fibonacci");

    /* For --arg and -b / --bool only the last value is taken */
    const char* argv[] = { "", "-F", "0", "--arg", "first", "--fibonacci", "1", "-F", "1", "-b", "--arg", "second", "-F", "2", "-b", "in.txt" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("arg"), "second");
    CORRADE_COMPARE(args.arrayValueCount("fibonacci"), 4);
    CORRADE_COMPARE(args.arrayValue("fibonacci", 0), "0");
    CORRADE_COMPARE(args.arrayValue("fibonacci", 1), "1");
    CORRADE_COMPARE(args.arrayValue("fibonacci", 2), "1");
    CORRADE_COMPARE(args.arrayValue("fibonacci", 3), "2");
    CORRADE_VERIFY(args.isSet("bool"));

    CORRADE_COMPARE(args.arrayValueCount("input"), 1);
    CORRADE_COMPARE(args.arrayValue("input", 0), "in.txt");
}

void ArgumentsTest::parseUnknownArgument() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::UnknownArgument);
        CORRADE_COMPARE(key, "error");
        return false;
    });

    const char* argv[] = { "", "--error" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument --error\n");
}

void ArgumentsTest::parseUnknownShortArgument() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::UnknownShortArgument);
        CORRADE_COMPARE(key, "e");
        return false;
    });

    const char* argv[] = { "", "-e" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Unknown command-line argument -e\n");
}

void ArgumentsTest::parseSuperfluousArgument() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::SuperfluousArgument);
        CORRADE_COMPARE(key, "error");
        return false;
    });

    const char* argv[] = { "", "error" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument error\n");
}

void ArgumentsTest::parseSingleDash() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::SuperfluousArgument);
        /* Compared to parseSuperfluousArgument(), this verifies that the dash
           isn't stripped here */
        CORRADE_COMPARE(key, "-");
        return false;
    });

    const char* argv[] = { "", "-" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Superfluous command-line argument -\n");
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
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::InvalidShortArgument);
        CORRADE_COMPARE(key, "?");
        return false;
    });

    const char* argv[] = { "", "-?" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument -?\n");
}

void ArgumentsTest::parseInvalidLongArgument() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::InvalidArgument);
        CORRADE_COMPARE(key, "??");
        return false;
    });

    const char* argv[] = { "", "--??" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument --??\n");
}

void ArgumentsTest::parseInvalidLongArgumentDashes() {
    Arguments args;
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::InvalidShortArgument);
        CORRADE_COMPARE(key, "long-argument");
        return false;
    });

    const char* argv[] = { "", "-long-argument" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument -long-argument (did you mean --long-argument?)\n");
}

void ArgumentsTest::parseMissingValue() {
    Arguments args;
    args.addOption("output");
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Not parsed yet as this is an unrecoverable error */
        CORRADE_VERIFY(!args.isParsed());

        CORRADE_COMPARE(error, Arguments::ParseError::MissingValue);
        CORRADE_COMPARE(key, "output");
        return false;
    });

    const char* argv[] = { "", "--output" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing value for command-line argument --output\n");
}

void ArgumentsTest::parseMissingOption() {
    Arguments args;
    args.addBooleanOption("yes")
        .addNamedArgument("output");
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Everything should be parsed at this point */
        CORRADE_VERIFY(args.isParsed());
        CORRADE_VERIFY(args.isSet("yes"));

        CORRADE_COMPARE(error, Arguments::ParseError::MissingArgument);
        CORRADE_COMPARE(key, "output");
        return false;
    });

    const char* argv[] = { "", "--yes" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument --output\n");
}

void ArgumentsTest::parseMissingArgument() {
    Arguments args;
    args.addBooleanOption("yes")
        .addArgument("file").setHelp("file", "", "file.dat");
    args.setParseErrorCallback([](const Arguments& args, Arguments::ParseError error, const std::string& key) {
        /* Everything should be parsed at this point */
        CORRADE_VERIFY(args.isParsed());
        CORRADE_VERIFY(args.isSet("yes"));

        CORRADE_COMPARE(error, Arguments::ParseError::MissingArgument);
        CORRADE_COMPARE(key, "file");
        return false;
    });

    const char* argv[] = { "", "--yes" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Missing command-line argument file.dat\n");
}

void ArgumentsTest::parseMissingArrayArgumentMiddle() {
    Arguments args;
    args.addArgument("mode")
        .addArrayArgument("input")
        .addArgument("output")
        .addArgument("logfile");

    const char* argv[] = { "", "compress", "data.zip", "data.log" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    /* It's actually the array arguments missing (there has to be at least
       one), but that's impossible to distinguish here */
    CORRADE_COMPARE(out.str(), "Missing command-line argument logfile\n");
}

void ArgumentsTest::parseMissingArrayArgumentLast() {
    Arguments args;
    args.addArgument("mode")
        .addArrayArgument("input");

    const char* argv[] = { "", "compress" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    /* Verify it's correctly printed (and not --input or some such) */
    CORRADE_COMPARE(out.str(), "Missing command-line argument input\n");
}

void ArgumentsTest::prefixedParse() {
    Arguments arg1;
    arg1.addArgument("file")
        .addBooleanOption('b', "binary")
        .addOption("speed")
        .addSkippedPrefix("read");

    Arguments arg2{"read"};
    arg2.addOption("behavior")
        .addOption("buffer-size")
        .addArrayOption("seek");

    CORRADE_COMPARE(arg1.prefix(), "");
    CORRADE_COMPARE(arg2.prefix(), "read");

    const char* argv[] = { "", "-b", "--read-behavior", "buffered", "--speed", "fast", "--binary", "--read-seek", "33", "--read-buffer-size", "4K", "file.dat", "--read-seek", "-0" };

    CORRADE_VERIFY(arg1.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(arg1.isSet("binary"));
    CORRADE_COMPARE(arg1.value("speed"), "fast");
    CORRADE_COMPARE(arg1.value("file"), "file.dat");

    CORRADE_VERIFY(arg2.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(arg2.value("behavior"), "buffered");
    CORRADE_COMPARE(arg2.value("buffer-size"), "4K");
    CORRADE_COMPARE(arg2.arrayValueCount("seek"), 2);
    CORRADE_COMPARE(arg2.arrayValue("seek", 0), "33");
    CORRADE_COMPARE(arg2.arrayValue("seek", 1), "-0");
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
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args{"reader"};
    args.addArgument("foo")
        .addArrayArgument("bizbaz")
        .addNamedArgument("bar")
        .addOption('a', "baz")
        .addArrayOption('X', "booboo")
        .addBooleanOption("eh")
        .setGlobalHelp("global help");

    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addArgument(): argument foo not allowed in prefixed version\n"
        "Utility::Arguments::addArrayArgument(): argument bizbaz not allowed in prefixed version\n"
        "Utility::Arguments::addNamedArgument(): argument bar not allowed in prefixed version\n"
        "Utility::Arguments::addOption(): short option a not allowed in prefixed version\n"
        "Utility::Arguments::addArrayOption(): short option X not allowed in prefixed version\n"
        "Utility::Arguments::addBooleanOption(): boolean option eh not allowed in prefixed version\n"
        "Utility::Arguments::setGlobalHelp(): global help text only allowed in unprefixed version\n");
}

void ArgumentsTest::prefixedDisallowedWithPrefix() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    Arguments args;
    args.addOption("reader-flush")
        .addSkippedPrefix("reader");

    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::addSkippedPrefix(): skipped prefix reader conflicts with existing keys\n");
}

void ArgumentsTest::prefixedDisallowedWithPrefixAfterSkipPrefix() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

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

void ArgumentsTest::prefixedIgnoreUnknown() {
    Arguments args{"reader", Arguments::Flag::IgnoreUnknownOptions};
    args.addOption("foo");

    /* Unknown options should be ignored */
    const char* argv[] = { "", "--reader-foo", "yes", "--reader-is-interested", "not sure" };

    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(args.value("foo"), "yes");
}

void ArgumentsTest::prefixedIgnoreUnknownInvalidPrefixedName() {
    Arguments args{"reader", Arguments::Flag::IgnoreUnknownOptions};
    args.addOption("foo");

    /* Invalid options should be reported, because we can't be sure that it
       doesn't mess up with our assumption of what's an option and what a
       value */
    const char* argv[] = { "", "--reader-foo", "yes", "--reader-?", "what" };

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(out.str(), "Invalid command-line argument --reader-?\n");
}

void ArgumentsTest::notParsedYet() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    args.addOption("value")
        .addArrayOption("array")
        .addBooleanOption("boolean");

    std::ostringstream out;
    Error redirectError{&out};
    args.value("value");
    args.arrayValueCount("array");
    args.arrayValue("array", 0);
    args.isSet("boolean");
    CORRADE_VERIFY(!args.isParsed());
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::value(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::arrayValueCount(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::arrayValue(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::isSet(): arguments were not successfully parsed yet\n");
}

void ArgumentsTest::notParsedYetOnlyHelp() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const char* argv[] = { "", "--help" };

    Arguments args;
    args.addArgument("value")
        .addArrayOption("array")
        .addBooleanOption("boolean");

    std::ostringstream out;
    Error redirectError{&out};
    /* parse() should not succeed if there is --help but some arguments were
       not specified */
    CORRADE_VERIFY(!args.tryParse(Containers::arraySize(argv), argv));
    args.value("value");
    args.arrayValueCount("array");
    args.arrayValue("array", 0);
    args.isSet("boolean");
    CORRADE_VERIFY(!args.isParsed());
    CORRADE_COMPARE(out.str(),
        "Missing command-line argument value\n"
        "Utility::Arguments::value(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::arrayValueCount(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::arrayValue(): arguments were not successfully parsed yet\n"
        "Utility::Arguments::isSet(): arguments were not successfully parsed yet\n");
}

void ArgumentsTest::valueNotFound() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    args.addOption("foobar") /* only so asserts have some reference to return */
        .parse(0, nullptr);

    std::ostringstream out;
    Error redirectError{&out};
    args.value("nonexistent");
    args.arrayValueCount("nonexistent");
    args.arrayValue("nonexistent", 0);
    args.isSet("nonexistent");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::value(): key nonexistent not found\n"
        "Utility::Arguments::arrayValueCount(): key nonexistent not found\n"
        "Utility::Arguments::arrayValue(): key nonexistent not found\n"
        "Utility::Arguments::isSet(): key nonexistent not found\n");
}

void ArgumentsTest::valueMismatchedUse() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    Arguments args;
    args.addOption("value")
        .addArrayOption("array")
        .addBooleanOption("boolean")
        .parse(0, nullptr);

    std::ostringstream out;
    Error redirectError{&out};
    args.value("array");
    args.value("boolean");
    args.arrayValueCount("value");
    args.arrayValueCount("boolean");
    args.arrayValue("value", 0);
    args.arrayValue("boolean", 0);
    args.isSet("value");
    args.isSet("array");
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::value(): cannot use this function for an array/boolean option array\n"
        "Utility::Arguments::value(): cannot use this function for an array/boolean option boolean\n"
        "Utility::Arguments::arrayValueCount(): cannot use this function for a non-array option value\n"
        "Utility::Arguments::arrayValueCount(): cannot use this function for a non-array option boolean\n"
        "Utility::Arguments::arrayValue(): cannot use this function for a non-array option value\n"
        "Utility::Arguments::arrayValue(): cannot use this function for a non-array option boolean\n"
        "Utility::Arguments::isSet(): cannot use this function for a non-boolean option value\n"
        "Utility::Arguments::isSet(): cannot use this function for a non-boolean option array\n");
}

void ArgumentsTest::arrayValueOutOfBounds() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    const char* argv[] = { "", "-X", "first", "--opt", "second", "-X", "last" };

    Arguments args;
    args.addOption("foobar") /* only so asserts have some reference to return */
        .addArrayOption('X', "opt")
        .parse(Containers::arraySize(argv), argv);

    std::ostringstream out;
    Error redirectError{&out};
    args.arrayValue("opt", 3);
    CORRADE_COMPARE(out.str(),
        "Utility::Arguments::arrayValue(): id 3 out of range for 3 values with key opt\n");
}

void ArgumentsTest::parseErrorCallback() {
    Utility::Arguments args;
    args.addArgument("input")
        .addArgument("output")
        .addBooleanOption('i', "info")
            .setHelp("info", "print info about the input file and exit")
        .setParseErrorCallback([](const Utility::Arguments& args, Utility::Arguments::ParseError error, const std::string& key) {
            /* If --info is passed, we don't need the output argument */
            if(error == Arguments::ParseError::MissingArgument &&
            key == "output" &&
            args.isSet("info")) return true;

            /* Handle all other errors as usual */
            return false;
        });

    /* Parsing should succeed */
    const char* argv[] = { "", "file.in", "-i" };
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_VERIFY(args.isSet("info"));
    CORRADE_COMPARE(args.value("input"), "file.in");
    CORRADE_COMPARE(args.value("output"), ""); /* default-constructed */
}

void ArgumentsTest::parseErrorCallbackIgnoreAll() {
    int count = 0;

    Utility::Arguments args;
    args.addArgument("input")
        .addOption("output")
        .addBooleanOption("hello")
        .setParseErrorCallback([](const Utility::Arguments& args, Utility::Arguments::ParseError error, const std::string& key) {
            ++*reinterpret_cast<int*>(args.parseErrorCallbackState());

            switch(error) {
                case Arguments::ParseError::InvalidShortArgument:
                    if(key == "?") CORRADE_COMPARE(key, "?");
                    else CORRADE_COMPARE(key, "help");
                    return true;
                case Arguments::ParseError::InvalidArgument:
                    CORRADE_COMPARE(key, "!!");
                    return true;
                case Arguments::ParseError::UnknownShortArgument:
                    CORRADE_COMPARE(key, "v");
                    return true;
                case Arguments::ParseError::UnknownArgument:
                    CORRADE_COMPARE(key, "halp");
                    return true;
                case Arguments::ParseError::MissingValue:
                    CORRADE_COMPARE(key, "output");
                    return true;
                case Arguments::ParseError::MissingArgument:
                    CORRADE_COMPARE(key, "input");
                    return true;

                /* Not handled here (mutually exclusive with MissingArgument) */
                case Arguments::ParseError::SuperfluousArgument:
                    break;
            }

            CORRADE_ITERATION(error);
            CORRADE_ITERATION(key);
            CORRADE_VERIFY(!"this shouldn't get here");
            return true;
        }, &count);

    const char* argv[] = { "", "-?", "--!!", "-v", "--halp", "-help", "--hello", "--output" };
    /* The parsing should ignore the errors, not die where it shouldn't, but
       still extracting the valid optionas */
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(count, 7);
    CORRADE_VERIFY(args.isSet("hello"));
}

void ArgumentsTest::parseErrorCallbackIgnoreAll2() {
    int count = 0;

    Utility::Arguments args;
    args.addBooleanOption("hello")
        .setParseErrorCallback([](const Utility::Arguments& args, Utility::Arguments::ParseError error, const std::string& key) {
            ++*reinterpret_cast<int*>(args.parseErrorCallbackState());

            switch(error) {
                /* All those handled above */
                case Arguments::ParseError::InvalidShortArgument:
                case Arguments::ParseError::InvalidArgument:
                case Arguments::ParseError::UnknownShortArgument:
                case Arguments::ParseError::UnknownArgument:
                case Arguments::ParseError::MissingValue:
                case Arguments::ParseError::MissingArgument:
                    break;

                case Arguments::ParseError::SuperfluousArgument:
                    CORRADE_COMPARE(key, "/dev/null 3");
                    return true;
            }

            CORRADE_ITERATION(error);
            CORRADE_ITERATION(key);
            CORRADE_VERIFY(!"this shouldn't get here");
            return true;
        }, &count);

    const char* argv[] = { "", "/dev/null 3", "--hello" };
    /* The parsing should ignore the errors, not die where it shouldn't, but
       still extracting the valid optionas */
    CORRADE_VERIFY(args.tryParse(Containers::arraySize(argv), argv));
    CORRADE_COMPARE(count, 1);
    CORRADE_VERIFY(args.isSet("hello"));
}

void ArgumentsTest::debugParseError() {
    std::ostringstream out;

    Debug{&out} << Arguments::ParseError::MissingArgument << Arguments::ParseError(0xf0);
    CORRADE_COMPARE(out.str(), "Utility::Arguments::ParseError::MissingArgument Utility::Arguments::ParseError(0xf0)\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ArgumentsTest)
