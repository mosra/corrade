/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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
#include <string>
#include <utility>
#include <vector>

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/File.h"
#include "Corrade/TestSuite/Compare/FileToString.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Path.h"

#include "configure.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct ConfigurationTest: TestSuite::Tester {
    explicit ConfigurationTest();

    void parse();
    void parseMissingEquals();
    void parseMissingQuote();
    void parseMissingMultiLineQuote();
    void parseHierarchic();
    void parseHierarchicShortcuts();
    void parseHierarchicEmptyGroup();
    void parseHierarchicEmptySubgroup();
    void parseHierarchicMissingBracket();
    void utf8Filename();

    void groupIndex();
    void valueIndex();

    void names();

    void readonly();
    void readError();
    void nonexistentFile();
    void truncate();

    void whitespaces();
    void bom();
    void eol();
    void stripComments();

    void multiLineValue();
    void multiLineValueCrlf();

    void standaloneGroup();
    void copy();
    void move();

    void iterateGroups();
    void iterateGroupsRangeFor();
    void iterateGroupsMutable();
    void iterateGroupsEmpty();
    void iterateValues();
    void iterateValuesRangeFor();
    void iterateValuesEmpty();
    void iterateValuesCommentsOnly();
    void iterateValuesComments();
};

using namespace Containers::Literals;

ConfigurationTest::ConfigurationTest() {
    addTests({&ConfigurationTest::parse,
              &ConfigurationTest::parseMissingEquals,
              &ConfigurationTest::parseMissingQuote,
              &ConfigurationTest::parseMissingMultiLineQuote,
              &ConfigurationTest::parseHierarchic,
              &ConfigurationTest::parseHierarchicShortcuts,
              &ConfigurationTest::parseHierarchicEmptyGroup,
              &ConfigurationTest::parseHierarchicEmptySubgroup,
              &ConfigurationTest::parseHierarchicMissingBracket,
              &ConfigurationTest::utf8Filename,

              &ConfigurationTest::groupIndex,
              &ConfigurationTest::valueIndex,

              &ConfigurationTest::names,

              &ConfigurationTest::readonly,
              &ConfigurationTest::readError,
              &ConfigurationTest::nonexistentFile,
              &ConfigurationTest::truncate,

              &ConfigurationTest::whitespaces,
              &ConfigurationTest::bom,
              &ConfigurationTest::eol,
              &ConfigurationTest::stripComments,

              &ConfigurationTest::multiLineValue,
              &ConfigurationTest::multiLineValueCrlf,

              &ConfigurationTest::standaloneGroup,
              &ConfigurationTest::copy,
              &ConfigurationTest::move,

              &ConfigurationTest::iterateGroups,
              &ConfigurationTest::iterateGroupsRangeFor,
              &ConfigurationTest::iterateGroupsMutable,
              &ConfigurationTest::iterateGroupsEmpty,
              &ConfigurationTest::iterateValues,
              &ConfigurationTest::iterateValuesRangeFor,
              &ConfigurationTest::iterateValuesEmpty,
              &ConfigurationTest::iterateValuesCommentsOnly,
              &ConfigurationTest::iterateValuesComments});

    /* Create testing dir */
    Path::make(CONFIGURATION_WRITE_TEST_DIR);

    /* Remove everything there */
    if(Path::exists(Path::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf")))
        Path::remove(Path::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));
    if(Path::exists(Path::join(CONFIGURATION_WRITE_TEST_DIR, "new.conf")))
        Path::remove(Path::join(CONFIGURATION_WRITE_TEST_DIR, "new.conf"));
}

void ConfigurationTest::parse() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "parse.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));
    CORRADE_VERIFY(conf.configuration() == &conf);
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Groups */
    CORRADE_VERIFY(conf.hasGroups());
    CORRADE_COMPARE(conf.groupCount(), 4);
    CORRADE_VERIFY(!conf.hasGroup("groupNonexistent"));
    CORRADE_COMPARE(conf.groupCount("group"), 2);
    CORRADE_COMPARE(conf.groupCount("emptyGroup"), 1);
    CORRADE_VERIFY(conf.group("group")->configuration() == &conf);
    CORRADE_COMPARE_AS(conf.groups("group"),
        (std::vector<ConfigurationGroup*>{conf.group("group", 0), conf.group("group", 1)}),
        TestSuite::Compare::Container);

    std::string tmp;

    /* Values */
    CORRADE_VERIFY(conf.hasValues());
    CORRADE_COMPARE(conf.valueCount(), 1);
    CORRADE_VERIFY(conf.hasValue("key"));
    CORRADE_VERIFY(!conf.hasValue("keyNonexistent"));
    CORRADE_COMPARE(conf.value("key"), "value");
    CORRADE_COMPARE(conf.group("group", 1)->value("c", 1), "value5");
    CORRADE_COMPARE_AS(conf.group("group", 1)->values("c"),
        (std::vector<std::string>{"value4", "value5"}), TestSuite::Compare::Container);

    /* Default-constructed nonexistent values */
    CORRADE_COMPARE(conf.value("nonexistent"), "");
    CORRADE_COMPARE(conf.value<int>("nonexistent"), 0);
    CORRADE_COMPARE(conf.value<double>("nonexistent"), 0.0);

    /* Save file back - expecting no change */
    CORRADE_VERIFY(conf.save());

    /* Modify */
    conf.addValue("new", "value");
    conf.removeAllGroups("group");
    conf.group("thirdGroup")->clear();
    CORRADE_VERIFY(conf.removeGroup("emptyGroup"));
    CORRADE_VERIFY(conf.addGroup("newGroup"));
    conf.group("newGroup")->addValue("another", "value");
    conf.addGroup("newGroupCopy", new ConfigurationGroup(*conf.group("newGroup")));
    conf.removeAllValues("key");

    /* Save again, verify changes */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "parse-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::parseMissingEquals() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "missing-equals.conf"));

    /* Nothing remains, filename is empty and valid bit is not set */
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): missing equals for a value\n");
}

void ConfigurationTest::parseMissingQuote() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "missing-quote.conf"));

    /* Nothing remains, filename is empty and valid bit is not set */
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): missing closing quote for a value\n");
}

void ConfigurationTest::parseMissingMultiLineQuote() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "missing-multiline-quote.conf"));

    /* Nothing remains, filename is empty and valid bit is not set */
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): missing closing quotes for a multi-line value\n");
}

void ConfigurationTest::parseHierarchic() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Check parsing */
    CORRADE_VERIFY(conf.hasGroup("z"));
    CORRADE_COMPARE(conf.group("z")->group("x")->group("c")->group("v")->value("key1"), "val1");
    CORRADE_COMPARE(conf.groupCount("a"), 2);
    CORRADE_COMPARE(conf.group("a")->groupCount("b"), 2);
    CORRADE_COMPARE(conf.group("a")->group("b", 0)->value("key2"), "val2");
    CORRADE_COMPARE(conf.group("a")->group("b", 1)->value("key2"), "val3");
    CORRADE_COMPARE(conf.group("a", 1)->value("key3"), "val4");
    CORRADE_COMPARE(conf.group("a", 1)->group("b")->value("key2"), "val5");

    /* Expect no change */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"),
                       TestSuite::Compare::File);

    /* Modify */
    conf.group("z")->group("x")->clear();
    conf.group("a", 1)->addGroup("b")->setValue("key2", "val6");
    conf.addGroup("q")->addGroup("w")->addGroup("e")->addGroup("r")->setValue("key4", "val7");

    /* Verify changes */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "hierarchic-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::parseHierarchicShortcuts() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hierarchic-shortcuts.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic-shortcuts.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Should not be parsed as a/b/c */
    CORRADE_VERIFY(!conf.hasGroup("c/d/e"));
    CORRADE_VERIFY(conf.hasGroup("c"));
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->value("hello"), "there");
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->group("f")->group("g")->value("hi"), "again");

    /* Second g group */
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->group("f")->groupCount("g"), 2);
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->group("f")->group("g", 1)->value("hey"), "hiya");

    /* First g group in second f group */
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->groupCount("f"), 2);
    CORRADE_COMPARE(conf.group("c")->group("d")->group("e")->group("f", 1)->group("g")->value("hola"), "hallo");

    /* A group with explicitly enumerated parents */
    CORRADE_COMPARE(conf.group("q")->group("w")->group("e")->group("r")->value("key4"), "val7");

    /* Verify that nothing changed except for the last squashed group */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic-shortcuts.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "hierarchic-shortcuts-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::parseHierarchicEmptyGroup() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hierarchic-empty-group.conf"));
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): empty group name\n");
}

void ConfigurationTest::parseHierarchicEmptySubgroup() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hierarchic-empty-subgroup.conf"));
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): empty subgroup name\n");
}

void ConfigurationTest::parseHierarchicMissingBracket() {
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hierarchic-missing-bracket.conf"));
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
    CORRADE_COMPARE(out.str(), "Utility::Configuration::Configuration(): missing closing bracket for a group header\n");
}

void ConfigurationTest::utf8Filename() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "hýždě.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hýždě.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());
    CORRADE_COMPARE(conf.value("unicode"), "supported");
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "hýždě.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "hýždě.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::groupIndex() {
    std::istringstream in("[a]\n[a]\n");
    Configuration conf(in);
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    CORRADE_VERIFY(conf.hasGroup("a", 0));
    CORRADE_VERIFY(conf.hasGroup("a", 1));
    CORRADE_VERIFY(!conf.hasGroup("a", 2));
}

void ConfigurationTest::valueIndex() {
    std::istringstream in("a=\na=\n");
    Configuration conf(in);
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    CORRADE_VERIFY(conf.hasValue("a", 0));
    CORRADE_VERIFY(conf.hasValue("a", 1));
    CORRADE_VERIFY(!conf.hasValue("a", 2));

    /* Setting third value when there are two present is the same as adding
       another value. However, setting fourth value is not possible, as there
       is no third one. */
    CORRADE_VERIFY(!conf.setValue("a", "foo", 3));
    CORRADE_VERIFY(conf.setValue("a", "foo", 2));
}

void ConfigurationTest::names() {
    CORRADE_SKIP_IF_NO_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf;

    {
        /* With CORRADE_GRACEFUL_ASSERT the groups are leaked */
        auto g = conf.addGroup("");
        CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addGroup(): empty group name\n");
        delete g;
    }

    {
        /* With CORRADE_GRACEFUL_ASSERT the groups are leaked */
        out.str({});
        auto g = conf.addGroup("a/b/c");
        CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addGroup(): disallowed character in group name\n");
        delete g;
    }

    out.str({});
    conf.setValue("", "foo");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::setValue(): empty key\n");

    out.str({});
    conf.addValue("a=", "foo");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addValue(): disallowed character in key\n");
}

void ConfigurationTest::readonly() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::ReadOnly);

    /* Filename for readonly configuration is empty */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
}

void ConfigurationTest::readError() {
    #if defined(CORRADE_TARGET_IOS) && defined(CORRADE_TESTSUITE_TARGET_XCTEST)
    if(std::getenv("SIMULATOR_MAINSCREEN_SCALE"))
        CORRADE_SKIP("iOS (in a simulator) thinks all paths are files, can't abuse a directory to simulate a read error.");
    #endif

    /* A directory, yes. At first I thought this would be a nice & quick way to
       check, but given the amount of OS-specific workarounds all around I'm
       not so sure anymore. */
    std::ostringstream out;
    Error redirectError{&out};
    Configuration conf(CONFIGURATION_TEST_DIR);
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());

    /* On Windows the opening itself fails, on Unix we have an explicit check.
       On other systems no idea, so let's say we expect the same message as on
       Unix. */
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_COMPARE_AS(out.str(),
        formatString("Utility::Path::read(): can't open {}: error ", CONFIGURATION_TEST_DIR),
        TestSuite::Compare::StringHasPrefix);
    #else
    CORRADE_COMPARE(out.str(), formatString("Utility::Path::read(): {} is a directory\n", CONFIGURATION_TEST_DIR));
    #endif
}

void ConfigurationTest::nonexistentFile() {
    if(Path::exists(Path::join(CONFIGURATION_WRITE_TEST_DIR, "nonexistent.conf")))
        CORRADE_VERIFY(Path::remove(Path::join(CONFIGURATION_WRITE_TEST_DIR, "nonexistent.conf")));
    Configuration conf(Path::join(CONFIGURATION_WRITE_TEST_DIR, "nonexistent.conf"));

    /* Everything okay if the file doesn't exist */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_COMPARE(conf.filename(), Path::join(CONFIGURATION_WRITE_TEST_DIR, "nonexistent.conf"));

    conf.setValue("key", "value");
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "nonexistent.conf"),
                       "key=value\n", TestSuite::Compare::FileToString);
}

void ConfigurationTest::truncate() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::ReadOnly|Configuration::Flag::Truncate);

    /* File is truncated on saving */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "truncate.conf")));
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "truncate.conf"),
                       "", TestSuite::Compare::FileToString);
}

void ConfigurationTest::whitespaces() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "whitespaces.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "whitespaces.conf"));
    conf.save();

    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "whitespaces.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "whitespaces-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::bom() {
    {
        /* Stripped by default */
        Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "bom.conf"));
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "bom.conf")));
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "bom.conf"),
            "", TestSuite::Compare::FileToString);
    } {
        /* Explicitly preserved */
        Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "bom.conf"), Configuration::Flag::PreserveBom);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "bom-preserve.conf")));
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "bom-preserve.conf"),
            "\xEF\xBB\xBF", TestSuite::Compare::FileToString);
    }
}

void ConfigurationTest::eol() {
    {
        /* Autodetect Unix */
        Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "eol-unix.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(!conf.isEmpty());
        CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf")));
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect Windows */
        Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "eol-windows.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(!conf.isEmpty());
        CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf")));
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect mixed (both \r and \r\n) */
        Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "eol-mixed.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(!conf.isEmpty());
        CORRADE_VERIFY(conf.save(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf")));
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf"),
            "key=value\r\nkey=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Force Unix */
        Configuration conf(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceUnixEol);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Force Windows */
        Configuration conf(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceWindowsEol);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Default */
        Configuration conf(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate);
        CORRADE_VERIFY(conf.isValid());
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    }
}

void ConfigurationTest::stripComments() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "comments.conf"), Configuration::Flag::SkipComments);
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "comments.conf"));

    /* Verify that comments were removed */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "comments.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "comments-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::multiLineValue() {
    /* Remove previous saved file */
    if(Path::exists(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf")))
        CORRADE_VERIFY(Path::remove(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf")));

    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "multiLine.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Check parsing */
    CORRADE_COMPARE(conf.value("value"), " Hello\n people how\n are you?");
    CORRADE_COMPARE(conf.value("empty"), "");

    /* Expect change only in empty value */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "multiLine-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::multiLineValueCrlf() {
    /* Remove previous saved file */
    if(Path::exists(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf")))
        CORRADE_VERIFY(Path::remove(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf")));

    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "multiLine-crlf.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Check parsing */
    CORRADE_COMPARE(conf.value("value"), " Hello\n people how\n are you?");

    /* Expect change only in lines without CR */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf"),
                       Path::join(CONFIGURATION_TEST_DIR, "multiLine-crlf-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::standaloneGroup() {
    ConfigurationGroup* group = new ConfigurationGroup;

    group->setValue("value", "hello");
    ConfigurationGroup* descendent = group->addGroup("descendent");
    descendent->setValue("number", 42);
    descendent->addGroup("deep");

    CORRADE_COMPARE(group->value("value"), "hello");
    CORRADE_COMPARE(group->group("descendent")->value<int>("number"), 42);

    /* No configuration is assigned initially */
    CORRADE_VERIFY(!group->configuration());
    CORRADE_VERIFY(!group->group("descendent")->configuration());
    CORRADE_VERIFY(!group->group("descendent")->group("deep")->configuration());

    /* But it gets assigned once it's added to a Configuration */
    Configuration conf;
    conf.addGroup("group", group);
    CORRADE_COMPARE(group->configuration(), &conf);
    CORRADE_COMPARE(group->group("descendent")->configuration(), &conf);
    CORRADE_COMPARE(group->group("descendent")->group("deep")->configuration(), &conf);
}

void ConfigurationTest::copy() {
    Configuration conf;

    ConfigurationGroup* original = conf.addGroup("group");
    ConfigurationGroup* descendent = original->addGroup("descendent");
    descendent->setValue("value", 42);
    descendent->addGroup("deep");
    CORRADE_COMPARE(original->configuration(), &conf);
    CORRADE_COMPARE(original->group("descendent")->configuration(), &conf);
    CORRADE_COMPARE(original->group("descendent")->group("deep")->configuration(), &conf);

    /* The configuration isn't preserved on copy construction */
    ConfigurationGroup* constructedCopy = new ConfigurationGroup(*original);
    CORRADE_VERIFY(!constructedCopy->configuration());
    CORRADE_VERIFY(!constructedCopy->group("descendent")->configuration());
    CORRADE_VERIFY(!constructedCopy->group("descendent")->group("deep")->configuration());

    /* But on assignment it inherits the configuration used in the assigned to
       instance */
    ConfigurationGroup* assignedCopy = conf.addGroup("another");
    CORRADE_COMPARE(assignedCopy->configuration(), &conf);
    *assignedCopy = *constructedCopy;
    CORRADE_COMPARE(assignedCopy->configuration(), &conf);
    CORRADE_COMPARE(assignedCopy->group("descendent")->configuration(), &conf);
    CORRADE_COMPARE(assignedCopy->group("descendent")->group("deep")->configuration(), &conf);

    original->group("descendent")->setValue<int>("value", 666);

    CORRADE_COMPARE(original->group("descendent")->value<int>("value"), 666);
    CORRADE_COMPARE(constructedCopy->group("descendent")->value<int>("value"), 42);
    CORRADE_COMPARE(assignedCopy->group("descendent")->value<int>("value"), 42);

    delete constructedCopy;
}

void ConfigurationTest::move() {
    Configuration conf;
    ConfigurationGroup* original = conf.addGroup("group");
    ConfigurationGroup* descendent = original->addGroup("descendent");
    descendent->setValue("value", 42);
    descendent->addGroup("deep");

    /* Move constructor for ConfigurationGroup */
    ConfigurationGroup* constructedMove = new ConfigurationGroup(std::move(*original));
    CORRADE_VERIFY(original->isEmpty());
    CORRADE_VERIFY(!constructedMove->configuration());
    CORRADE_VERIFY(!constructedMove->group("descendent")->configuration());
    CORRADE_VERIFY(!constructedMove->group("descendent")->group("deep")->configuration());

    /* Move assignment for ConfigurationGroup */
    ConfigurationGroup* assignedMove = conf.addGroup("another");
    CORRADE_COMPARE(assignedMove->configuration(), &conf);
    *assignedMove = std::move(*constructedMove);
    CORRADE_VERIFY(constructedMove->isEmpty());
    CORRADE_COMPARE(assignedMove->configuration(), &conf);
    CORRADE_COMPARE(assignedMove->group("descendent")->configuration(), &conf);
    CORRADE_COMPARE(assignedMove->group("descendent")->group("deep")->configuration(), &conf);

    delete constructedMove;

    /* Move constructor for Configuration */
    Configuration confConstructedMove(std::move(conf));
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_COMPARE(confConstructedMove.configuration(), &confConstructedMove);
    CORRADE_COMPARE(confConstructedMove.group("group")->configuration(), &confConstructedMove);
    CORRADE_COMPARE(confConstructedMove.group("another")->group("descendent")->configuration(), &confConstructedMove);

    /* Move assignment for Configuration */
    Configuration confAssignedMove;
    confAssignedMove = std::move(confConstructedMove);
    CORRADE_VERIFY(confConstructedMove.isEmpty());
    CORRADE_COMPARE(confAssignedMove.configuration(), &confAssignedMove);
    CORRADE_COMPARE(confAssignedMove.group("group")->configuration(), &confAssignedMove);
    CORRADE_COMPARE(confAssignedMove.group("another")->group("descendent")->configuration(), &confAssignedMove);
}

void ConfigurationTest::iterateGroups() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    /* No matter whether the originating ConfigurationGroup is const or not,
       it should be possible to use the immutable type */
    const ConfigurationGroup* groupsOnly = conf.group("groupsOnly");
    CORRADE_VERIFY(groupsOnly);
    ConfigurationGroup::Groups groups = groupsOnly->groups();
    CORRADE_VERIFY(groups.begin() == groups.cbegin());
    CORRADE_VERIFY(groups.end() == groups.cend());
    CORRADE_VERIFY(groups.begin() != groups.end());

    ConfigurationGroup::GroupIterator it = groups.begin();

    /* Test post-increment, dereference and return value */
    Containers::Pair<Containers::StringView, Containers::Reference<const ConfigurationGroup>> a = *(it++);
    CORRADE_COMPARE(a.first(), "a");
    CORRADE_COMPARE(a.second()->value("yes"), "yes");

    Containers::Pair<Containers::StringView, Containers::Reference<const ConfigurationGroup>> b = *it;
    CORRADE_COMPARE(b.first(), "b");
    CORRADE_COMPARE(b.second()->value("yes"), "no");

    /* Test pre-increment */
    CORRADE_VERIFY(++it == groups.end());
}

void ConfigurationTest::iterateGroupsRangeFor() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    /* No matter whether the originating ConfigurationGroup is const or not,
       it should be possible to use the immutable type */
    const ConfigurationGroup* mixed = conf.group("mixed");
    CORRADE_VERIFY(mixed);
    Containers::Array<Containers::StringView> names;
    for(Containers::Pair<Containers::StringView, Containers::Reference<const ConfigurationGroup>> g: mixed->groups()) {
        arrayAppend(names, g.first());
    }

    CORRADE_COMPARE_AS(names,
        Containers::arrayView({"first"_s, "subgroup"_s, "subgroup"_s, "last"_s}),
        TestSuite::Compare::Container);
}

void ConfigurationTest::iterateGroupsMutable() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));
    conf.setFilename(Path::join(CONFIGURATION_WRITE_TEST_DIR, "iterate.conf"));

    ConfigurationGroup* mixed = conf.group("mixed");
    CORRADE_VERIFY(mixed);
    CORRADE_VERIFY(mixed->groups().begin() != mixed->groups().end());
    CORRADE_COMPARE((*mixed->groups().begin()).first(), "first"_s);
    (*mixed->groups().begin()).second()->setValue("psot", "frist!");

    conf.save();

    CORRADE_COMPARE_AS(Path::join(CONFIGURATION_WRITE_TEST_DIR, "iterate.conf"),
        Path::join(CONFIGURATION_TEST_DIR, "iterate-modified.conf"),
        TestSuite::Compare::File);
}

void ConfigurationTest::iterateGroupsEmpty() {
    const Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* valuesOnly = conf.group("valuesOnly");
    CORRADE_VERIFY(valuesOnly);
    CORRADE_VERIFY(valuesOnly->groups().begin() == valuesOnly->groups().end());
}

void ConfigurationTest::iterateValues() {
    const Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* valuesOnly = conf.group("valuesOnly");
    CORRADE_VERIFY(valuesOnly);
    ConfigurationGroup::Values values = valuesOnly->values();
    CORRADE_VERIFY(values.begin() == values.cbegin());
    CORRADE_VERIFY(values.end() == values.cend());
    CORRADE_VERIFY(values.begin() != values.end());

    ConfigurationGroup::ValueIterator it = values.begin();

    /* Test pre-increment, dereference and return value */
    Containers::Pair<Containers::StringView, Containers::StringView> a = *(it++);
    CORRADE_COMPARE(a, pair("a"_s, "42"_s));

    CORRADE_COMPARE(*it, pair("duplicate"_s, "this should be first"_s));
    /* Test pre-increment */
    CORRADE_COMPARE(*(++it), pair("duplicate"_s, "this second"_s));
    CORRADE_COMPARE(*(++it), pair("multiline"_s, "ah\nwell"_s));

    CORRADE_VERIFY(++it == values.end());
}

void ConfigurationTest::iterateValuesRangeFor() {
    Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* mixed = conf.group("mixed");
    CORRADE_VERIFY(mixed);
    Containers::Array<Containers::Pair<Containers::StringView, Containers::StringView>> values;
    for(Containers::Pair<Containers::StringView, Containers::StringView> g: mixed->values()) {
        arrayAppend(values, InPlaceInit, g.first(), g.second());
    }

    CORRADE_COMPARE_AS(values,
        Containers::arrayView({pair("b"_s, "value"_s), pair("a"_s, "also"_s)}),
        TestSuite::Compare::Container);
}

void ConfigurationTest::iterateValuesEmpty() {
    const Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* groupsOnly = conf.group("groupsOnly");
    CORRADE_VERIFY(groupsOnly);
    CORRADE_VERIFY(groupsOnly->values().begin() == groupsOnly->values().end());
}

void ConfigurationTest::iterateValuesCommentsOnly() {
    const Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* commentsOnly = conf.group("commentsOnly");
    CORRADE_VERIFY(commentsOnly);
    CORRADE_VERIFY(commentsOnly->values().begin() == commentsOnly->values().end());
}

void ConfigurationTest::iterateValuesComments() {
    const Configuration conf(Path::join(CONFIGURATION_TEST_DIR, "iterate.conf"));

    const ConfigurationGroup* mixed = conf.group("mixed");
    CORRADE_VERIFY(mixed);
    ConfigurationGroup::Values valuesComments = mixed->valuesComments();
    ConfigurationGroup::ValueIterator it = valuesComments.begin();

    /* The begin() should not skip past comments */
    CORRADE_VERIFY(it != valuesComments.end());
    CORRADE_COMPARE(*it++, pair(""_s, "# A comment"_s));
    CORRADE_VERIFY(it != valuesComments.end());
    CORRADE_COMPARE(*it++, pair("b"_s, "value"_s));
    CORRADE_VERIFY(it != valuesComments.end());
    CORRADE_COMPARE(*it++, pair(""_s, "; Another, which gets its leading whitespace trimmed"_s));
    CORRADE_VERIFY(it != valuesComments.end());
    /* Test that pre-increment also does the right thing */
    CORRADE_COMPARE(*it, pair("a"_s, "also"_s));
    CORRADE_VERIFY(++it != valuesComments.end());
    CORRADE_COMPARE(*it, pair(""_s, ""_s));
    CORRADE_VERIFY(++it != valuesComments.end());
    CORRADE_COMPARE(*it, pair(""_s, "# Another comment, and empty line after"_s));
    CORRADE_VERIFY(++it != valuesComments.end());
    CORRADE_COMPARE(*it, pair(""_s, ""_s));
    CORRADE_VERIFY(++it == valuesComments.end());
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ConfigurationTest)
