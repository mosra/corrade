/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "ConfigurationTest.h"

#include <string>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtTest/QTest>

#include "Utility/Configuration.h"
#include "testConfigure.h"
#include "Wgs84Coords.h"

using namespace std;

QTEST_APPLESS_MAIN(Kompas::Utility::Test::ConfigurationTest)

namespace Kompas { namespace Utility { namespace Test {

ConfigurationTest::ConfigurationTest() {
    /* Create testing dir */
    QDir testDir;
    Q_ASSERT(testDir.mkpath(CONFIGURATION_WRITE_TEST_DIR));

    /* Copy files for testing */
    QDir dir(CONFIGURATION_TEST_DIR);
    QStringList filters;
    filters << "*.conf";
    QStringList list = dir.entryList(filters, QDir::Files);
    foreach(QString file, list) {
        /* Remove file */
        QFile::remove(CONFIGURATION_WRITE_TEST_DIR + file);

        Q_ASSERT(QFile::copy(CONFIGURATION_TEST_DIR + file, CONFIGURATION_WRITE_TEST_DIR + file));
    }
}

void ConfigurationTest::parse() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("parse.conf"));
    QVERIFY(conf.isValid());

    /* Groups */
    QVERIFY(conf.groupCount() == 3);
    QVERIFY(conf.groups().size() == 3);
    QVERIFY(conf.groupCount("group") == 2);
    QVERIFY(conf.groupCount("empty_group") == 1);
    QVERIFY(!conf.groupExists("group_inexistent"));

    vector<ConfigurationGroup*> expectedGroups;
    expectedGroups.push_back(conf.group("group", 0));
    expectedGroups.push_back(conf.group("group", 1));
    QVERIFY(expectedGroups == conf.groups("group"));

    string tmp;

    /* Keys */
    QVERIFY(conf.value("key", &tmp));
    QVERIFY(tmp == "value");

    QVERIFY(conf.group("group", 1)->value("c", &tmp, 1));
    QVERIFY(tmp == "value5");

    vector<string> expectedValues;
    expectedValues.push_back("value4");
    expectedValues.push_back("value5");
    QVERIFY(conf.group("group", 1)->values<string>("c") == expectedValues);

    QVERIFY(conf.keyExists("key"));
    QVERIFY(!conf.keyExists("key_inexistent"));

    /* Save file back */
    QVERIFY(conf.save());

    /* Expecting no change */
    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("parse.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();
    fileActual.close();
    fileOrig.close();

    QCOMPARE(actual, original);

    /* Modify */
    QVERIFY(conf.addValue<string>("new", "value"));
    QVERIFY(conf.removeAllGroups("group"));
    QVERIFY(conf.removeGroup("empty_group"));
    QVERIFY(conf.addGroup("new_group"));
    QVERIFY(conf.group("new_group")->addValue<string>("another", "value"));
    QVERIFY(conf.removeAllValues("key"));

    QVERIFY(conf.save());

    /* Verify changes */
    fileOrig.setFileName(CONFIGURATION_TEST_DIR + QString("parse-modified.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    original = fileOrig.readAll();
    actual = fileActual.readAll();

    QCOMPARE(actual, original);
}

void ConfigurationTest::parseDirect() {
    /* Configuration created directly from istream should be readonly */
    istringstream contents("[group]\nkey=value");
    Configuration conf(contents);
    QVERIFY(conf.isValid());
    QVERIFY(!conf.addValue<string>("key2", "value2"));
    QVERIFY(!conf.save());
}

void ConfigurationTest::empty() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("new.conf"));
    QVERIFY(conf.isValid());
    QVERIFY(conf.save());
}

void ConfigurationTest::invalid() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("invalid.conf"));

    /* The group is there */
    QEXPECT_FAIL("", "Currently on invalid row whole group is dropped", Continue);
    QVERIFY(conf.groupCount("group") == 1);

    /* Everything should be disabled */
    QVERIFY(conf.addGroup("new") == 0);
    QVERIFY(!conf.removeGroup("group"));
    QVERIFY(!conf.removeAllGroups("group"));
    QVERIFY(!conf.addValue<string>("new", "value"));
/* TODO: enable after fix of XFAIL
    QVERIFY(!conf.group("group")->setValue<string>("key", "newValue"));
    QVERIFY(!conf.group("group")->removeValue("key"));
    QVERIFY(!conf.group("group")->removeAllValues("key"));
*/
    QVERIFY(!conf.save());
}

void ConfigurationTest::readonly() {
    /* Reload fresh parse.conf */
    QFile::remove(CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf"));
    Q_ASSERT(QFile::copy(CONFIGURATION_TEST_DIR + QString("parse.conf"), CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf")));

    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("parse.conf"), Configuration::ReadOnly);

    /* Everything should be disabled */
    QVERIFY(conf.addGroup("new") == 0);
    QVERIFY(!conf.removeGroup("group"));
    QVERIFY(!conf.removeAllGroups("group"));
    QVERIFY(!conf.addValue<string>("new", "value"));
    QVERIFY(!conf.group("group")->setValue<string>("key", "newValue"));
    QVERIFY(!conf.group("group")->removeValue("b"));
    QVERIFY(!conf.group("group")->removeAllValues("b"));
    QVERIFY(!conf.save());
}

void ConfigurationTest::readonlyWithoutFile() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("inexistent.conf"), Configuration::ReadOnly);
    QVERIFY(!conf.isValid());
}

void ConfigurationTest::truncate() {
    /* Reload fresh parse.conf */
    QFile::remove(CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf"));
    Q_ASSERT(QFile::copy(CONFIGURATION_TEST_DIR + QString("parse.conf"), CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf")));

    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("parse.conf"), Configuration::Truncate);
    conf.save();

    QVERIFY(conf.keyCount("key") == 0);

    QFile file(CONFIGURATION_WRITE_TEST_DIR + QString("parse.conf"));
    file.open(QFile::Text|QFile::ReadOnly);
    QByteArray contents = file.readAll();
    QVERIFY(contents == "");
}

void ConfigurationTest::whitespaces() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("whitespaces.conf"));
    conf.save();

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("whitespaces-saved.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("whitespaces.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();

    QCOMPARE(actual, original);
}

void ConfigurationTest::types() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("types.conf"));

    string tmp;
    conf.value("string", &tmp);
    QVERIFY(tmp == "value");
    QVERIFY(conf.setValue("string", tmp));
    conf.value("quotes", &tmp);
    QVERIFY(tmp == " value ");
    QVERIFY(conf.setValue("quotes", tmp));

    int intTmp;
    conf.value("int", &intTmp);
    QVERIFY(intTmp == 5);
    QVERIFY(conf.setValue("int", intTmp));
    conf.value("intNeg", &intTmp);
    QVERIFY(intTmp == -10);
    QVERIFY(conf.setValue("intNeg", intTmp));

    bool boolTmp;
    conf.value("bool", &boolTmp, 0);
    QVERIFY(boolTmp);
    QVERIFY(conf.setValue("bool", boolTmp, 0));
    boolTmp = false;
    conf.value("bool", &boolTmp, 1);
    QVERIFY(boolTmp);
    boolTmp = false;
    conf.value("bool", &boolTmp, 2);
    QVERIFY(boolTmp);
    boolTmp = false;
    conf.value("bool", &boolTmp, 3);
    QVERIFY(boolTmp);
    conf.value("bool", &boolTmp, 4);
    QVERIFY(!boolTmp);
    QVERIFY(conf.setValue("bool", boolTmp, 4));

    double doubleTmp;
    conf.value("double", &doubleTmp);
    QVERIFY(doubleTmp == 3.78);
    QVERIFY(conf.setValue("double", doubleTmp));
    conf.value("doubleNeg", &doubleTmp);
    QVERIFY(doubleTmp == -2.14);
    QVERIFY(conf.setValue("doubleNeg", doubleTmp));

    /* Flags */
    conf.value("exp", &doubleTmp);
    QVERIFY(doubleTmp == 2.1e7);
    conf.value("expPos", &doubleTmp);
    QVERIFY(doubleTmp == 2.1e+7);
    QVERIFY(conf.setValue("expPos", doubleTmp, 0, ConfigurationGroup::Scientific));
    conf.value("expNeg", &doubleTmp);
    QVERIFY(doubleTmp == -2.1e7);
    conf.value("expNeg2", &doubleTmp);
    QVERIFY(doubleTmp == 2.1e-7);
    conf.value("expBig", &doubleTmp);
    QVERIFY(doubleTmp == 2.1E7);

    conf.value("oct", &intTmp, 0, ConfigurationGroup::Oct);
    QVERIFY(intTmp == 0773);
    QVERIFY(conf.setValue("oct", intTmp, 0, ConfigurationGroup::Oct));
    conf.value("hex", &intTmp, 0, ConfigurationGroup::Hex);
    QVERIFY(intTmp == 0x6ecab);
    QVERIFY(conf.setValue("hex", intTmp, 0, ConfigurationGroup::Hex));
    conf.value("hex2", &intTmp, 0, ConfigurationGroup::Hex);
    QVERIFY(intTmp == 0x5462FF);
    conf.value("color", &intTmp, 0, ConfigurationGroup::Color);
    QVERIFY(intTmp == 0x34f85e);
    QVERIFY(conf.setValue("color", intTmp, 0, ConfigurationGroup::Color));

    /* Wgs84 coordinates */
    Core::Wgs84Coords coordsTmp;
    conf.value("coords", &coordsTmp);
    QVERIFY(coordsTmp == Core::Wgs84Coords(49.1925, 16.602222));
    QVERIFY(conf.setValue("coords", coordsTmp));

    /* Invalid coordinates (load) */
    conf.value("coordsInvalid", &coordsTmp);
    QVERIFY(coordsTmp == Core::Wgs84Coords());
    QVERIFY(conf.value("coordsInvalid2", &coordsTmp));
    QVERIFY(coordsTmp == Core::Wgs84Coords());

    /* Invalid coordinates (save) */
    QVERIFY(conf.setValue("coordsInvalidSave", coordsTmp));

    conf.save();

    /* Check saved values */
    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("types.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("types.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();

    QCOMPARE(actual, original);
}

void ConfigurationTest::eol_data() {
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("flags");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("autodetect-unix") << "eol-unix.conf" << 0 << QByteArray("key=value\n");
    QTest::newRow("autodetect-windows") << "eol-windows.conf" << 0 << QByteArray("key=value\r\n");
    QTest::newRow("autodetect-mixed") << "eol-mixed.conf" << 0 << QByteArray("key=value\r\nkey=value\r\n");
    QTest::newRow("force-unix") << "" << (int) Configuration::ForceUnixEol << QByteArray("key=value\n");
    QTest::newRow("force-windows") << "" << (int) Configuration::ForceWindowsEol << QByteArray("key=value\r\n");
    QTest::newRow("default") << "" << 0 << QByteArray("key=value\n");
}

void ConfigurationTest::eol() {
    QFETCH(QString, filename);
    QFETCH(int, flags);
    QFETCH(QByteArray, output);

    string file;
    if(!filename.isEmpty()) file = filename.toStdString();
    else {
        file = "temp.conf";
        flags |= Configuration::Truncate;
    }

    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + file, flags);

    /* Add some data to fill the file */
    if(filename.isEmpty()) conf.addValue<string>("key", "value");

    conf.save();

    QFile _file(CONFIGURATION_WRITE_TEST_DIR + QString::fromStdString(file));
    _file.open(QFile::ReadOnly);
    QByteArray actual = _file.readAll();

    QCOMPARE(actual, output);
}

void ConfigurationTest::uniqueGroups() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("unique-groups.conf"), Configuration::UniqueGroups);
    conf.save();

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("unique-groups-saved.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("unique-groups.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();

    QCOMPARE(actual, original);

    /* Try to insert already existing group */
    QVERIFY(conf.addGroup("group") == 0);
}

void ConfigurationTest::uniqueKeys() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("unique-keys.conf"), Configuration::UniqueKeys);
    conf.save();

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("unique-keys-saved.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("unique-keys.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();

    QCOMPARE(actual, original);

    /* Try to insert already existing key */
    QVERIFY(conf.addValue<string>("key", "val") == 0);
}

void ConfigurationTest::stripComments() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("comments.conf"), Configuration::SkipComments);
    conf.save();

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("comments-saved.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("comments.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();

    QCOMPARE(actual, original);
}

void ConfigurationTest::autoCreation() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("autoCreation.conf"), Configuration::Truncate);

    QVERIFY(conf.group("newGroup") == 0);

    conf.setAutomaticGroupCreation(true);
    QVERIFY(conf.group("newGroup") != 0);
    conf.setAutomaticGroupCreation(false);

    string value1 = "defaultValue1";
    QVERIFY(!conf.group("newGroup")->value<string>("key", &value1));

    conf.setAutomaticKeyCreation(true);
    QVERIFY(conf.group("newGroup")->value<string>("key", &value1));
    QVERIFY(conf.group("newGroup")->keyCount("key") == 1);
    QVERIFY(value1 == "defaultValue1");

    conf.setAutomaticGroupCreation(true);
    string value2 = "defaultValue2";
    QVERIFY(conf.group("group")->value<string>("key", &value2));
    QVERIFY(conf.group("group")->keyCount("key") == 1);
    QVERIFY(value2 == "defaultValue2");

    /* Auto-creation of non-string values */
    int value3 = 42;
    QVERIFY(conf.group("group")->value<int>("integer", &value3));
    conf.setAutomaticKeyCreation(false);
    QVERIFY(conf.group("group")->value<int>("integer", &value3));
    QVERIFY(value3 == 42);
}

void ConfigurationTest::directValue() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("directValue.conf"), Configuration::Truncate);

    /* Fill values */
    conf.setValue<string>("string", "value");
    conf.setValue<int>("key", 23);

    /* Test direct return */
    QVERIFY(conf.value<string>("string") == "value");
    QVERIFY(conf.value<int>("key") == 23);

    /* Default-configured values */
    QVERIFY(conf.value<string>("inexistent") == "");
    QVERIFY(conf.value<int>("inexistent") == 0);
    QVERIFY(conf.value<double>("inexistent") == 0.0);
}

void ConfigurationTest::hierarchic() {
    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("hierarchic.conf"));
    QVERIFY(conf.isValid());

    /* Check parsing */
    QVERIFY(conf.group("z")->group("x")->group("c")->group("v")->value<string>("key1") == "val1");
    QVERIFY(conf.groupCount("a") == 2);
    QVERIFY(conf.group("a")->groupCount("b") == 2);
    QVERIFY(conf.group("a")->group("b", 0)->value<string>("key2") == "val2");
    QVERIFY(conf.group("a")->group("b", 1)->value<string>("key2") == "val3");
    QVERIFY(conf.group("a", 1)->value<string>("key3") == "val4");
    QVERIFY(conf.group("a", 1)->group("b")->value<string>("key2") == "val5");

    /* Expect no change */
    QVERIFY(conf.save());

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("hierarchic.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("hierarchic.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();
    fileOrig.close();
    fileActual.close();

    QCOMPARE(actual, original);

    /* Modify */
    conf.group("a", 1)->addGroup("b")->setValue<string>("key2", "val6");
    conf.addGroup("q")->addGroup("w")->addGroup("e")->addGroup("r")->setValue<string>("key4", "val7");

    /* Cannot add group with '/' character */
    QVERIFY(!conf.addGroup("a/b/c"));

    conf.save();

    /* Verify changes */
    fileOrig.setFileName(CONFIGURATION_TEST_DIR + QString("hierarchic-modified.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    original = fileOrig.readAll();
    actual = fileActual.readAll();

    QCOMPARE(actual, original);
}

void ConfigurationTest::hierarchicUnique() {
    /* Reload fresh hierarchic.conf */
    QFile::remove(CONFIGURATION_WRITE_TEST_DIR + QString("hierarchic.conf"));
    Q_ASSERT(QFile::copy(CONFIGURATION_TEST_DIR + QString("hierarchic.conf"), CONFIGURATION_WRITE_TEST_DIR + QString("hierarchic.conf")));

    Configuration conf(CONFIGURATION_WRITE_TEST_DIR + string("hierarchic.conf"), Configuration::UniqueGroups);
    conf.save();

    QFile fileOrig(CONFIGURATION_TEST_DIR + QString("hierarchic-unique.conf"));
    QFile fileActual(CONFIGURATION_WRITE_TEST_DIR + QString("hierarchic.conf"));
    fileOrig.open(QFile::Text|QFile::ReadOnly);
    fileActual.open(QFile::Text|QFile::ReadOnly);
    QByteArray original = fileOrig.readAll();
    QByteArray actual = fileActual.readAll();
    fileOrig.close();
    fileActual.close();

    QCOMPARE(actual, original);
}

}}}
