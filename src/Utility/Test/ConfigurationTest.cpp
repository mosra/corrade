/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "TestSuite/Tester.h"
#include "TestSuite/Compare/Container.h"
#include "TestSuite/Compare/File.h"
#include "TestSuite/Compare/FileToString.h"
#include "Utility/Configuration.h"
#include "Utility/Directory.h"

#include "testConfigure.h"

namespace Corrade { namespace Utility { namespace Test {

class ConfigurationTest: public TestSuite::Tester {
    public:
        ConfigurationTest();

        void parse();
        void parseHierarchic();

        void groupIndex();
        void valueIndex();

        void names();

        void invalid();
        void readonly();
        void inexistentFile();
        void truncate();

        void whitespaces();
        void types();
        void eol();
        void stripComments();

        void multiLineValue();
        void multiLineValueCrlf();

        /** @todo Merge into parse() and uniqueGroups() */

        void copy();
};

ConfigurationTest::ConfigurationTest() {
    addTests({&ConfigurationTest::parse,
              &ConfigurationTest::parseHierarchic,

              &ConfigurationTest::groupIndex,
              &ConfigurationTest::valueIndex,

              &ConfigurationTest::names,

              &ConfigurationTest::invalid,
              &ConfigurationTest::readonly,
              &ConfigurationTest::inexistentFile,
              &ConfigurationTest::truncate,

              &ConfigurationTest::whitespaces,
              &ConfigurationTest::types,
              &ConfigurationTest::eol,
              &ConfigurationTest::stripComments,

              &ConfigurationTest::multiLineValue,
              &ConfigurationTest::multiLineValueCrlf,

              &ConfigurationTest::copy});

    /* Create testing dir */
    Directory::mkpath(CONFIGURATION_WRITE_TEST_DIR);

    /* Remove everything there */
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "new.conf"));
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "types.conf"));
}

void ConfigurationTest::parse() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "parse.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));

    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());

    /* Groups */
    CORRADE_VERIFY(conf.hasGroups());
    CORRADE_COMPARE(conf.groupCount(), 4);
    CORRADE_VERIFY(!conf.hasGroup("groupInexistent"));
    CORRADE_COMPARE(conf.groupCount("group"), 2);
    CORRADE_COMPARE(conf.groupCount("emptyGroup"), 1);
    CORRADE_COMPARE_AS(conf.groups("group"),
        (std::vector<ConfigurationGroup*>{conf.group("group", 0), conf.group("group", 1)}),
        TestSuite::Compare::Container);

    std::string tmp;

    /* Values */
    CORRADE_VERIFY(conf.hasValues());
    CORRADE_COMPARE(conf.valueCount(), 1);
    CORRADE_VERIFY(conf.hasValue("key"));
    CORRADE_VERIFY(!conf.hasValue("keyInexistent"));
    CORRADE_COMPARE(conf.value("key"), "value");
    CORRADE_COMPARE(conf.group("group", 1)->value("c", 1), "value5");
    CORRADE_COMPARE_AS(conf.group("group", 1)->values("c"),
        (std::vector<std::string>{"value4", "value5"}), TestSuite::Compare::Container);

    /* Default-constructed inexistent values */
    CORRADE_COMPARE(conf.value("inexistent"), "");
    CORRADE_COMPARE(conf.value<int>("inexistent"), 0);
    CORRADE_COMPARE(conf.value<double>("inexistent"), 0.0);

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
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "parse-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::parseHierarchic() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"));
    CORRADE_VERIFY(conf.isValid());

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
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"),
                       TestSuite::Compare::File);

    /* Modify */
    conf.group("z")->group("x")->clear();
    conf.group("a", 1)->addGroup("b")->setValue("key2", "val6");
    conf.addGroup("q")->addGroup("w")->addGroup("e")->addGroup("r")->setValue("key4", "val7");

    /* Verify changes */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "hierarchic-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::groupIndex() {
    std::istringstream in("[a]\n[a]\n");
    Configuration conf(in);

    CORRADE_VERIFY(conf.hasGroup("a", 0));
    CORRADE_VERIFY(conf.hasGroup("a", 1));
    CORRADE_VERIFY(!conf.hasGroup("a", 2));
}

void ConfigurationTest::valueIndex() {
    std::istringstream in("a=\na=\n");
    Configuration conf(in);

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
    std::ostringstream out;
    Error::setOutput(&out);
    Configuration conf;

    conf.addGroup("");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addGroup(): empty group name\n");

    out.str({});
    conf.addGroup("a/b/c");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addGroup(): disallowed character in group name\n");

    out.str({});
    conf.setValue("", "foo");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::setValue(): empty key\n");

    out.str({});
    conf.addValue("a=", "foo");
    CORRADE_COMPARE(out.str(), "Utility::ConfigurationGroup::addValue(): disallowed character in key\n");
}

void ConfigurationTest::invalid() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "invalid.conf"));

    /* Nothing remains, filename is empty and valid bit is not set */
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
}

void ConfigurationTest::readonly() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::ReadOnly);

    /* Filename for readonly configuration is empty */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.isEmpty());
    CORRADE_VERIFY(conf.filename().empty());
}

void ConfigurationTest::inexistentFile() {
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "inexistent.conf"));
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "inexistent.conf"));

    /* Everything okay if the file doesn't exist */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_COMPARE(conf.filename(), Directory::join(CONFIGURATION_WRITE_TEST_DIR, "inexistent.conf"));

    conf.setValue("key", "value");
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "inexistent.conf"),
                       "key=value\n", TestSuite::Compare::FileToString);
}

void ConfigurationTest::truncate() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::ReadOnly|Configuration::Flag::Truncate);

    /* File is truncated on saving */
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(conf.isEmpty());
    CORRADE_VERIFY(conf.save(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf")));
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"),
                       "", TestSuite::Compare::FileToString);
}

void ConfigurationTest::whitespaces() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "whitespaces.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "whitespaces.conf"));
    conf.save();

    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "whitespaces.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "whitespaces-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::types() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "types.conf"), Configuration::Flag::ReadOnly);

    /* String */
    CORRADE_COMPARE(conf.value("string"), "value");
    CORRADE_VERIFY(conf.setValue("string", "value"));
    CORRADE_COMPARE(conf.value("quotes"), " value ");
    CORRADE_VERIFY(conf.setValue("quotes", " value "));

    /* Int */
    CORRADE_COMPARE(conf.value<int>("int"), 5);
    CORRADE_VERIFY(conf.setValue("int", 5));
    CORRADE_COMPARE(conf.value<int>("intNeg"), -10);
    CORRADE_VERIFY(conf.setValue("intNeg", -10));

    /* Bool */
    CORRADE_COMPARE(conf.value<bool>("bool", 0), true);
    CORRADE_VERIFY(conf.setValue("bool", true, 0));
    CORRADE_COMPARE(conf.value<bool>("bool", 1), true);
    CORRADE_COMPARE(conf.value<bool>("bool", 2), true);
    CORRADE_COMPARE(conf.value<bool>("bool", 3), true);
    CORRADE_COMPARE(conf.value<bool>("bool", 4), false);
    CORRADE_VERIFY(conf.setValue("bool", false, 4));

    /* Double */
    CORRADE_COMPARE(conf.value<double>("double"), 3.78);
    CORRADE_VERIFY(conf.setValue("double", 3.78));
    CORRADE_COMPARE(conf.value<double>("doubleNeg"), -2.14);
    CORRADE_VERIFY(conf.setValue("doubleNeg", -2.14));

    /* Double scientific */
    CORRADE_COMPARE(conf.value<double>("exp"), 2.1e7);
    CORRADE_COMPARE(conf.value<double>("expPos"), 2.1e+7);
    conf.setValue("expPos", 2.1e+7, 0, ConfigurationValueFlag::Scientific);
    CORRADE_COMPARE(conf.value<double>("expNeg"), -2.1e7);
    CORRADE_COMPARE(conf.value<double>("expNeg2"), 2.1e-7);
    CORRADE_COMPARE(conf.value<double>("expBig"), 2.1E7);
    conf.setValue<double>("expBig", 2.1E7, 0, ConfigurationValueFlag::Scientific|ConfigurationValueFlag::Uppercase);

    /* Flags */
    CORRADE_COMPARE(conf.value<int>("oct", 0, ConfigurationValueFlag::Oct), 0773);
    conf.setValue("oct", 0773, 0, ConfigurationValueFlag::Oct);
    CORRADE_COMPARE(conf.value<int>("hex", 0, ConfigurationValueFlag::Hex), 0x6ecab);
    conf.setValue("hex", 0x6ecab, 0, ConfigurationValueFlag::Hex);
    CORRADE_COMPARE(conf.value<int>("hex2", 0, ConfigurationValueFlag::Hex), 0x5462FF);
    CORRADE_COMPARE(conf.value<int>("hexUpper", 0, ConfigurationValueFlag::Hex|ConfigurationValueFlag::Uppercase), 0xF00D);
    conf.setValue("hexUpper", 0xF00D, 0, ConfigurationValueFlag::Hex|ConfigurationValueFlag::Uppercase);

    /* Nothing should be changed after saving */
    CORRADE_VERIFY(conf.save(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "types.conf")));
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "types.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "types.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::eol() {
    {
        /* Autodetect Unix */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-unix.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.save(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf")));
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect Windows */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-windows.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.save(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf")));
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect mixed (both \r and \r\n) */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-mixed.conf"), Configuration::Flag::ReadOnly);
        CORRADE_VERIFY(conf.save(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf")));
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf"),
            "key=value\r\nkey=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Force Unix */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceUnixEol);
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Force Windows */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceWindowsEol);
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Default */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate);
        CORRADE_VERIFY(conf.setValue("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    }
}

void ConfigurationTest::stripComments() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "comments.conf"), Configuration::Flag::SkipComments);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "comments.conf"));

    /* Verify that comments were removed */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "comments.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "comments-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::multiLineValue() {
    /* Remove previous saved file */
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf"));

    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "multiLine.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf"));
    CORRADE_VERIFY(conf.isValid());

    /* Check parsing */
    CORRADE_COMPARE(conf.value("value"), " Hello\n people how\n are you?");
    CORRADE_COMPARE(conf.value("empty"), "");

    /* Expect change only in empty value */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "multiLine-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::multiLineValueCrlf() {
    /* Remove previous saved file */
    Directory::rm(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf"));

    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "multiLine-crlf.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf"));
    CORRADE_VERIFY(conf.isValid());

    /* Check parsing */
    CORRADE_COMPARE(conf.value("value"), " Hello\n people how\n are you?");

    /* Expect change only in lines without CR */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "multiLine-crlf.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "multiLine-crlf-saved.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::copy() {
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "copy.conf"));

    ConfigurationGroup* original = conf.addGroup("group");
    original->addGroup("descendent")->setValue<int>("value", 42);

    ConfigurationGroup* constructedCopy = new ConfigurationGroup(*original);
    ConfigurationGroup* assignedCopy = conf.addGroup("another");
    *assignedCopy = *original;

    original->group("descendent")->setValue<int>("value", 666);

    CORRADE_COMPARE(original->group("descendent")->value<int>("value"), 666);
    CORRADE_COMPARE(constructedCopy->group("descendent")->value<int>("value"), 42);
    CORRADE_COMPARE(assignedCopy->group("descendent")->value<int>("value"), 42);
}

}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::ConfigurationTest)
