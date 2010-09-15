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

#include "PluginTestConfigure.h"

QTEST_APPLESS_MAIN(Map2X::PluginManager::Test::PluginTest)

using namespace std;

void initialize() {
    PLUGIN_IMPORT(Kangaroo)
    PLUGIN_IMPORT(Canary)
}

namespace Map2X { namespace PluginManager { namespace Test {

PluginTest::PluginTest() {
    initialize();
    manager = new PluginManager<AbstractAnimal>(PLUGINS_DIR);
}

void PluginTest::nameList() {
    QStringList expected, actual;
    expected << "Canary" << "Chihuahua" << "Dog" << "Kangaroo" << "Snail";

    vector<string> names = manager->nameList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        actual.append(QString::fromStdString(*it));
    }

    QCOMPARE(actual, expected);
}

void PluginTest::errors() {
    /** @todo Wrong plugin version (it would be hard) */
    /** @todo Wrong interface version */

    /* Wrong metadata file */
    QVERIFY(manager->loadState("Snail") == AbstractPluginManager::WrongMetadataFile);
    QVERIFY(manager->loadState("Kangaroo") == AbstractPluginManager::WrongMetadataFile);
    QVERIFY(manager->load("Snail") == AbstractPluginManager::WrongMetadataFile);
    QVERIFY(manager->load("Kangaroo") == AbstractPluginManager::WrongMetadataFile);
}

void PluginTest::staticPlugin() {
    QVERIFY(manager->loadState("Canary") == AbstractPluginManager::IsStatic);
    QVERIFY(manager->metadata("Canary")->name() == "I'm allergic to canaries!");
    QVERIFY(manager->metadata("Canary")->authors()[0] == "Vladimír Vondruš <mosra@centrum.cz>");
    QVERIFY(manager->metadata("Canary")->version() == "1.0");

    AbstractAnimal* animal = manager->instance("Canary");

    QVERIFY(animal != 0);
    QVERIFY(animal->hasTail() == true);
    QVERIFY(animal->name() == "Achoo");
    QVERIFY(animal->legCount() == 2);

    QVERIFY(manager->unload("Canary") == AbstractPluginManager::IsStatic);
}

void PluginTest::dynamicPlugin() {
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->load("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->metadata("Dog")->name() == "A simple dog plugin");

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

void PluginTest::hierarchy() {
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->loadState("Chihuahua") == AbstractPluginManager::NotLoaded);

    QVERIFY(manager->load("Chihuahua") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->metadata("Chihuahua")->name() == "The smallest dog in the world.");
    QVERIFY(manager->metadata("Chihuahua")->depends().size() == 1);
    QVERIFY(manager->metadata("Chihuahua")->depends()[0] == "Dog");
    QVERIFY(manager->metadata("Dog")->usedBy().size() == 1);
    QVERIFY(manager->metadata("Dog")->usedBy()[0] == "Chihuahua");

    AbstractAnimal* animal = manager->instance("Chihuahua");

    QVERIFY(animal != 0);
    QVERIFY(animal->hasTail() == true); // inherited from dog
    QVERIFY(animal->legCount() == 4); // this too
    QVERIFY(animal->name() == "Rodriguez");

    /* Try to unload plugin when another is depending on it */
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::IsRequired);

    /* Unload chihuahua plugin, then try again */
    delete animal;
    QVERIFY(manager->unload("Chihuahua") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->metadata("Dog")->usedBy().size() == 0);
}

}}}
