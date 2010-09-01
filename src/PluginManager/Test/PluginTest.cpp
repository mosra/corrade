/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "PluginTest.h"

#include <QtTest/QTest>

#include "Canary.cpp"
#include "PluginTestConfigure.h"

QTEST_APPLESS_MAIN(Map2X::PluginManager::Test::PluginTest)

using namespace std;

namespace Map2X { namespace PluginManager { namespace Test {

PluginTest::PluginTest() {
    PLUGIN_IMPORT_STATIC(Canary)

    manager = new PluginManager<AbstractAnimal>(PLUGINS_DIR);
}

void PluginTest::nameList() {
    QStringList expected, actual;
    expected << "Canary" << "Dog";

    vector<string> names = manager->nameList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        actual.append(QString::fromStdString(*it));
    }

    QCOMPARE(actual, expected);
}

void PluginTest::staticPlugin() {
    QVERIFY(manager->loadState("Canary") == AbstractPluginManager::IsStatic);
    QVERIFY(manager->metadata("Canary")->name == "I'm allergic to canaries!");

    AbstractAnimal* animal = manager->instance("Canary");

    QVERIFY(animal != 0);
    QVERIFY(animal->hasTail() == true);
    QVERIFY(animal->name() == "Achoo");
    QVERIFY(animal->legCount() == 2);

    QVERIFY(manager->unload("Canary") == AbstractPluginManager::IsStatic);
}

void PluginTest::dynamicPlugin() {
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::Unknown);
    QVERIFY(manager->load("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->metadata("Dog")->name == "A simple dog plugin");

    AbstractAnimal* animal = manager->instance("Dog");

    QVERIFY(animal != 0);
    QVERIFY(animal->hasTail() == true);
    QVERIFY(animal->name() == "Doug");
    QVERIFY(animal->legCount() == 4);

    /* Try to unload plugin when instance is used */
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::IsUsed);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::LoadOk);

    /* Plugin can be unloaded after destroying all instances */
    delete animal;
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::NotLoaded);
}

}}}
