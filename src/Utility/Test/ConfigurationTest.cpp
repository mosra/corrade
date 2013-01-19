/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
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
        void parseDirect();
        void empty();
        void invalid();
        void readonly();
        void readonlyWithoutFile();
        void truncate();
        void whitespaces();
        void types();
        void eol();
        void uniqueGroups();
        void uniqueKeys();
        void stripComments();

        void autoCreation();
        void directValue();

        /** @todo Merge into parse() and uniqueGroups() */
        void hierarchic();
        void hierarchicUnique();

        void copy();
};

ConfigurationTest::ConfigurationTest() {
    addTests(&ConfigurationTest::parse,
             &ConfigurationTest::parseDirect,
             &ConfigurationTest::empty,
             &ConfigurationTest::invalid,
             &ConfigurationTest::readonly,
             &ConfigurationTest::readonlyWithoutFile,
             &ConfigurationTest::truncate,
             &ConfigurationTest::whitespaces,
             &ConfigurationTest::types,
             &ConfigurationTest::eol,
             &ConfigurationTest::uniqueGroups,
             &ConfigurationTest::uniqueKeys,
             &ConfigurationTest::stripComments,
             &ConfigurationTest::autoCreation,
             &ConfigurationTest::directValue,
             &ConfigurationTest::hierarchic,
             &ConfigurationTest::hierarchicUnique,
             &ConfigurationTest::copy);

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

    /* Groups */
    CORRADE_COMPARE(conf.groupCount(), 4);
    CORRADE_COMPARE(conf.groups().size(), 4);
    CORRADE_COMPARE(conf.groupCount("group"), 2);
    CORRADE_COMPARE(conf.groupCount("empty_group"), 1);
    CORRADE_VERIFY(!conf.groupExists("group_inexistent"));
    CORRADE_COMPARE_AS((std::vector<ConfigurationGroup*>{conf.group("group", 0), conf.group("group", 1)}),
        conf.groups("group"), TestSuite::Compare::Container);

    std::string tmp;

    /* Keys */
    CORRADE_VERIFY(conf.value("key", &tmp));
    CORRADE_COMPARE(tmp, "value");
    CORRADE_VERIFY(conf.group("group", 1)->value("c", &tmp, 1));
    CORRADE_COMPARE(tmp, "value5");
    CORRADE_COMPARE_AS(conf.group("group", 1)->values<std::string>("c"),
        (std::vector<std::string>{"value4", "value5"}), TestSuite::Compare::Container);
    CORRADE_VERIFY(conf.keyExists("key"));
    CORRADE_VERIFY(!conf.keyExists("key_inexistent"));

    /* Save file back - expecting no change */
    CORRADE_VERIFY(conf.save());

    /* Modify */
    CORRADE_VERIFY(conf.addValue<std::string>("new", "value"));
    CORRADE_VERIFY(conf.removeAllGroups("group"));
    CORRADE_VERIFY(conf.group("third_group")->clear());
    CORRADE_VERIFY(conf.removeGroup("empty_group"));
    CORRADE_VERIFY(conf.addGroup("new_group"));
    CORRADE_VERIFY(conf.group("new_group")->addValue<std::string>("another", "value"));
    CORRADE_VERIFY(conf.addGroup("new_group_copy", new ConfigurationGroup(*conf.group("new_group"))));
    CORRADE_VERIFY(conf.removeAllValues("key"));

    /* Save again, verify changes */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "parse-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::parseDirect() {
    /* Configuration created directly from istream should be readonly */
    std::istringstream contents("[group]\nkey=value");
    Configuration conf(contents);
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(!conf.addValue<std::string>("key2", "value2"));
    CORRADE_VERIFY(!conf.save());
}

void ConfigurationTest::empty() {
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "new.conf"));
    CORRADE_VERIFY(conf.isValid());
    CORRADE_VERIFY(conf.save());
}

void ConfigurationTest::invalid() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "invalid.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "invalid.conf"));

    /* The group is there */
    {
        CORRADE_EXPECT_FAIL("Currently on invalid row whole group is dropped");
        CORRADE_COMPARE(conf.groupCount("group"), 1);
        /* TODO: enable after fix of XFAIL */
        // CORRADE_VERIFY(!conf.group("group")->setValue<string>("key", "newValue"));
        // CORRADE_VERIFY(!conf.group("group")->removeValue("key"));
        // CORRADE_VERIFY(!conf.group("group")->removeAllValues("key"));
    }

    /* Everything should be disabled */
    CORRADE_VERIFY(!conf.isValid());
    CORRADE_VERIFY(!conf.addGroup("new"));
    CORRADE_VERIFY(!conf.removeGroup("group"));
    CORRADE_VERIFY(!conf.removeAllGroups("group"));
    CORRADE_VERIFY(!conf.addValue<std::string>("new", "value"));
    CORRADE_VERIFY(!conf.save());
}

void ConfigurationTest::readonly() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::ReadOnly);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));

    /* Everything should be disabled */
    CORRADE_VERIFY(!conf.addGroup("new"));
    CORRADE_VERIFY(!conf.removeGroup("group"));
    CORRADE_VERIFY(!conf.removeAllGroups("group"));
    CORRADE_VERIFY(!conf.group("third_group")->clear());
    CORRADE_VERIFY(!conf.addValue<std::string>("new", "value"));
    CORRADE_VERIFY(!conf.group("group")->setValue<std::string>("key", "newValue"));
    CORRADE_VERIFY(!conf.group("group")->removeValue("b"));
    CORRADE_VERIFY(!conf.group("group")->removeAllValues("b"));
    CORRADE_VERIFY(!conf.save());
}

void ConfigurationTest::readonlyWithoutFile() {
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "inexistent.conf"), Configuration::Flag::ReadOnly);
    CORRADE_VERIFY(!conf.isValid());
}

void ConfigurationTest::truncate() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "parse.conf"), Configuration::Flag::Truncate);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "parse.conf"));

    CORRADE_COMPARE(conf.keyCount("key"), 0);

    CORRADE_VERIFY(conf.save());
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
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "types.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "types.conf"));

    std::string tmp;
    CORRADE_VERIFY(conf.value("string", &tmp));
    CORRADE_COMPARE(tmp, "value");
    CORRADE_VERIFY(conf.setValue("string", tmp));
    CORRADE_VERIFY(conf.value("quotes", &tmp));
    CORRADE_COMPARE(tmp, " value ");
    CORRADE_VERIFY(conf.setValue("quotes", tmp));

    int intTmp;
    CORRADE_VERIFY(conf.value("int", &intTmp));
    CORRADE_COMPARE(intTmp, 5);
    CORRADE_VERIFY(conf.setValue("int", intTmp));
    CORRADE_VERIFY(conf.value("intNeg", &intTmp));
    CORRADE_COMPARE(intTmp, -10);
    CORRADE_VERIFY(conf.setValue("intNeg", intTmp));

    bool boolTmp;
    CORRADE_VERIFY(conf.value("bool", &boolTmp, 0));
    CORRADE_VERIFY(boolTmp);
    CORRADE_VERIFY(conf.setValue("bool", boolTmp, 0));
    boolTmp = false;
    CORRADE_VERIFY(conf.value("bool", &boolTmp, 1));
    CORRADE_VERIFY(boolTmp);
    boolTmp = false;
    CORRADE_VERIFY(conf.value("bool", &boolTmp, 2));
    CORRADE_VERIFY(boolTmp);
    boolTmp = false;
    CORRADE_VERIFY(conf.value("bool", &boolTmp, 3));
    CORRADE_VERIFY(boolTmp);
    CORRADE_VERIFY(conf.value("bool", &boolTmp, 4));
    CORRADE_VERIFY(!boolTmp);
    CORRADE_VERIFY(conf.setValue("bool", boolTmp, 4));

    double doubleTmp;
    CORRADE_VERIFY(conf.value("double", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, 3.78);
    CORRADE_VERIFY(conf.setValue("double", doubleTmp));
    CORRADE_VERIFY(conf.value("doubleNeg", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, -2.14);
    CORRADE_VERIFY(conf.setValue("doubleNeg", doubleTmp));

    /* Flags */
    CORRADE_VERIFY(conf.value("exp", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, 2.1e7);
    CORRADE_VERIFY(conf.value("expPos", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, 2.1e+7);
    CORRADE_VERIFY(conf.setValue("expPos", doubleTmp, 0, ConfigurationValueFlag::Scientific));
    CORRADE_VERIFY(conf.value("expNeg", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, -2.1e7);
    CORRADE_VERIFY(conf.value("expNeg2", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, 2.1e-7);
    CORRADE_VERIFY(conf.value("expBig", &doubleTmp));
    CORRADE_COMPARE(doubleTmp, 2.1E7);

    CORRADE_VERIFY(conf.value("oct", &intTmp, 0, ConfigurationValueFlag::Oct));
    CORRADE_COMPARE(intTmp, 0773);
    CORRADE_VERIFY(conf.setValue("oct", intTmp, 0, ConfigurationValueFlag::Oct));
    CORRADE_VERIFY(conf.value("hex", &intTmp, 0, ConfigurationValueFlag::Hex));
    CORRADE_COMPARE(intTmp, 0x6ecab);
    CORRADE_VERIFY(conf.setValue("hex", intTmp, 0, ConfigurationValueFlag::Hex));
    CORRADE_VERIFY(conf.value("hex2", &intTmp, 0, ConfigurationValueFlag::Hex));
    CORRADE_COMPARE(intTmp, 0x5462FF);
    CORRADE_VERIFY(conf.value("color", &intTmp, 0, ConfigurationValueFlag::Color));
    CORRADE_COMPARE(intTmp, 0x34f85e);
    CORRADE_VERIFY(conf.setValue("color", intTmp, 0, ConfigurationValueFlag::Color));

    /* Check saved values */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "types.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "types.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::eol() {
    {
        /* Autodetect Unix */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-unix.conf"));
        conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-unix.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect Windows */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-windows.conf"));
        conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-windows.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Autodetect mixed (both \r and \r\n) */
        Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "eol-mixed.conf"));
        conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-mixed.conf"),
            "key=value\r\nkey=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Force Unix */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceUnixEol);
        CORRADE_VERIFY(conf.setValue<std::string>("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    } {
        /* Force Windows */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate|Configuration::Flag::ForceWindowsEol);
        CORRADE_VERIFY(conf.setValue<std::string>("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\r\n", TestSuite::Compare::FileToString);
    } {
        /* Default */
        Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            Configuration::Flag::Truncate);
        CORRADE_VERIFY(conf.setValue<std::string>("key", "value"));
        CORRADE_VERIFY(conf.save());
        CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "eol-temp.conf"),
            "key=value\n", TestSuite::Compare::FileToString);
    }
}

void ConfigurationTest::uniqueGroups() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "unique-groups.conf"), Configuration::Flag::UniqueGroups);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "unique-groups.conf"));

    /* Verify that non-unique groups were removed */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "unique-groups.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "unique-groups-saved.conf"),
                       TestSuite::Compare::File);

    /* Try to insert already existing group */
    CORRADE_VERIFY(!conf.addGroup("group"));
}

void ConfigurationTest::uniqueKeys() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "unique-keys.conf"), Configuration::Flag::UniqueKeys);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "unique-keys.conf"));

    /* Verify that non-unique keys were removed */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "unique-keys.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "unique-keys-saved.conf"),
                       TestSuite::Compare::File);

    /* Try to insert already existing key */
    CORRADE_VERIFY(!conf.addValue<std::string>("key", "val"));
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

void ConfigurationTest::autoCreation() {
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "autoCreation.conf"), Configuration::Flag::Truncate);

    CORRADE_VERIFY(!conf.group("newGroup"));
    conf.setAutomaticGroupCreation(true);
    CORRADE_VERIFY(conf.group("newGroup"));
    conf.setAutomaticGroupCreation(false);
    CORRADE_VERIFY(!conf.group("newGroup2"));

    std::string value1 = "defaultValue1";
    CORRADE_VERIFY(!conf.group("newGroup")->value<std::string>("key", &value1));

    conf.setAutomaticKeyCreation(true);
    CORRADE_VERIFY(conf.group("newGroup")->value<std::string>("key", &value1));
    CORRADE_COMPARE(conf.group("newGroup")->keyCount("key"), 1);
    CORRADE_COMPARE(value1, "defaultValue1");

    conf.setAutomaticGroupCreation(true);
    std::string value2 = "defaultValue2";
    CORRADE_VERIFY(conf.group("group")->value<std::string>("key", &value2));
    CORRADE_COMPARE(conf.group("group")->keyCount("key"), 1);
    CORRADE_COMPARE(value2, "defaultValue2");

    /* Auto-creation of non-string values */
    int value3 = 42;
    CORRADE_VERIFY(conf.group("group")->value<int>("integer", &value3));
    conf.setAutomaticKeyCreation(false);
    value3 = 45;
    CORRADE_VERIFY(conf.group("group")->value<int>("integer", &value3));
    CORRADE_COMPARE(value3, 42);
}

void ConfigurationTest::directValue() {
    Configuration conf(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "directValue.conf"), Configuration::Flag::Truncate);

    /* Fill values */
    conf.setValue<std::string>("string", "value");
    conf.setValue<int>("key", 23);

    /* Test direct return */
    CORRADE_COMPARE(conf.value<std::string>("string"), "value");
    CORRADE_COMPARE(conf.value<int>("key"), 23);

    /* Default-constructed values */
    CORRADE_COMPARE(conf.value<std::string>("inexistent"), "");
    CORRADE_COMPARE(conf.value<int>("inexistent"), 0);
    CORRADE_COMPARE(conf.value<double>("inexistent"), 0.0);
}

void ConfigurationTest::hierarchic() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"));
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"));
    CORRADE_VERIFY(conf.isValid());

    /* Check parsing */
    CORRADE_COMPARE(conf.group("z")->group("x")->group("c")->group("v")->value<std::string>("key1"), "val1");
    CORRADE_COMPARE(conf.groupCount("a"), 2);
    CORRADE_COMPARE(conf.group("a")->groupCount("b"), 2);
    CORRADE_COMPARE(conf.group("a")->group("b", 0)->value<std::string>("key2"), "val2");
    CORRADE_COMPARE(conf.group("a")->group("b", 1)->value<std::string>("key2"), "val3");
    CORRADE_COMPARE(conf.group("a", 1)->value<std::string>("key3"), "val4");
    CORRADE_COMPARE(conf.group("a", 1)->group("b")->value<std::string>("key2"), "val5");

    /* Expect no change */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"),
                       TestSuite::Compare::File);

    /* Modify */
    conf.group("z")->group("x")->clear();
    conf.group("a", 1)->addGroup("b")->setValue<std::string>("key2", "val6");
    conf.addGroup("q")->addGroup("w")->addGroup("e")->addGroup("r")->setValue<std::string>("key4", "val7");

    /* Cannot add group with '/' character */
    CORRADE_VERIFY(!conf.addGroup("a/b/c"));

    /* Verify changes */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "hierarchic-modified.conf"),
                       TestSuite::Compare::File);
}

void ConfigurationTest::hierarchicUnique() {
    Configuration conf(Directory::join(CONFIGURATION_TEST_DIR, "hierarchic.conf"), Configuration::Flag::UniqueGroups);
    conf.setFilename(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"));

    /* Verify that non-unique groups were removed */
    CORRADE_VERIFY(conf.save());
    CORRADE_COMPARE_AS(Directory::join(CONFIGURATION_WRITE_TEST_DIR, "hierarchic.conf"),
                       Directory::join(CONFIGURATION_TEST_DIR, "hierarchic-unique.conf"),
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
