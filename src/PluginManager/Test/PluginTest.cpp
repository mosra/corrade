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

#include "PluginTest.h"

#include <QtTest/QTest>
#include <QtCore/QFile>

#include "Utility/Directory.h"
#include "PluginTestConfigure.h"
#include "AbstractPluginManagerConfigure.h"

QTEST_APPLESS_MAIN(Kompas::PluginManager::Test::PluginTest)

using namespace std;
using namespace Kompas::Utility;

void initialize() {
    PLUGIN_IMPORT(Canary)
}

namespace Kompas { namespace PluginManager { namespace Test {

PluginTest::PluginTest() {
    initialize();
    manager = new PluginManager<AbstractAnimal>(PLUGINS_DIR);
    foodManager = new PluginManager<AbstractFood>(Directory::join(PLUGINS_DIR, "food"));
}

void PluginTest::nameList() {
    QStringList expected, actual;
    expected << "Canary" << "Chihuahua" << "Dog" << "Snail";

    vector<string> names = manager->pluginList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        actual.append(QString::fromStdString(*it));
    }

    QCOMPARE(actual, expected);

    /* Check if the list is cleared after destructing */
    delete manager;
    manager = new PluginManager<AbstractAnimal>(Directory::join(PLUGINS_DIR, "inexistent"));

    QStringList expected2;
    expected2 << "Canary";
    actual.clear();
    names = manager->pluginList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        actual.append(QString::fromStdString(*it));
    }

    QCOMPARE(actual, expected2);

    /* Revert manager back */
    delete manager;
    manager = new PluginManager<AbstractAnimal>(PLUGINS_DIR);
}

void PluginTest::errors() {
    /** @todo Wrong plugin version (it would be hard) */
    /** @todo Wrong interface version */

    /* Wrong metadata file */
    QVERIFY(manager->loadState("Snail") == AbstractPluginManager::WrongMetadataFile);
    QVERIFY(manager->load("Snail") == AbstractPluginManager::WrongMetadataFile);
}

void PluginTest::staticPlugin() {
    QVERIFY(manager->loadState("Canary") == AbstractPluginManager::IsStatic);
    QVERIFY(*manager->metadata("Canary")->name() == "I'm allergic to canaries!");
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
    QVERIFY(*manager->metadata("Dog")->name() == "A simple dog plugin");

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
    QVERIFY(*manager->metadata("Chihuahua")->name() == "The smallest dog in the world.");
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

void PluginTest::crossManagerDependencies() {
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(foodManager->loadState("HotDog") == AbstractPluginManager::NotLoaded);

    /* Load HotDog */
    QVERIFY(foodManager->load("HotDog") == AbstractPluginManager::LoadOk);
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::LoadOk);
    QVERIFY(foodManager->metadata("HotDog")->depends().size() == 1);
    QVERIFY(foodManager->metadata("HotDog")->depends()[0] == "Dog");
    QVERIFY(manager->metadata("Dog")->usedBy().size() == 1);
    QVERIFY(manager->metadata("Dog")->usedBy()[0] == "HotDog");

    /* Verify hotdog */
    AbstractFood* hotdog = foodManager->instance("HotDog");
    QVERIFY(!hotdog->isTasty());
    QVERIFY(hotdog->weight() == 6800);

    /* Try to unload dog while dog is used in hotdog */
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::IsRequired);

    /* Destroy hotdog, then try again */
    delete hotdog;
    QVERIFY(foodManager->unload("HotDog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::NotLoaded);
    QVERIFY(manager->metadata("Dog")->usedBy().size() == 0);
}

void PluginTest::usedByZombies() {
    /* HotDogWithSnail depends on Dog and Snail, which cannot be loaded, so the
       loading fails too. Dog plugin then shouldn't have HotDogWithSnail in
       usedBy list. */

    QVERIFY(foodManager->load("HotDogWithSnail") == AbstractPluginManager::UnresolvedDependency);
    QVERIFY(manager->metadata("Dog")->usedBy().size() == 0);

    /* Cleanup after me... */
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::NotLoaded);
}

void PluginTest::reloadPluginDirectory() {
    /* Load Dog and rename the plugin */
    QVERIFY(manager->load("Dog") == AbstractPluginManager::LoadOk);
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, string("Dog") + PLUGIN_FILENAME_SUFFIX)),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, string("LostDog") + PLUGIN_FILENAME_SUFFIX)));
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, "Dog.conf")),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, "LostDog.conf")));

    /* Rename Chihuahua */
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, string("Chihuahua") + PLUGIN_FILENAME_SUFFIX)),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX)));
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, "Chihuahua.conf")),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, "LostChihuahua.conf")));

    /* Reload plugin dir and check new name list */
    manager->reloadPluginDirectory();

    QStringList expected1, actual1;
    expected1 << "Canary" << "Dog" << "LostChihuahua" << "LostDog" << "Snail";

    vector<string> names = manager->pluginList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it)
        actual1.append(QString::fromStdString(*it));

    /* Unload Dog and it should disappear from the list */
    QVERIFY(manager->unload("Dog") == AbstractPluginManager::NotLoaded);

    QStringList expected2 = expected1, actual2;
    expected2.removeAll("Dog");
    names = manager->pluginList();
    for(vector<string>::const_iterator it = names.begin(); it != names.end(); ++it)
        actual2.append(QString::fromStdString(*it));

    /* Rename everything back and clean up */
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, string("LostDog") + PLUGIN_FILENAME_SUFFIX)),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, string("Dog") + PLUGIN_FILENAME_SUFFIX)));
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, "LostDog.conf")),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, "Dog.conf")));

    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX)),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, string("Chihuahua") + PLUGIN_FILENAME_SUFFIX)));
    QFile::rename(QString::fromStdString(Directory::join(PLUGINS_DIR, "LostChihuahua.conf")),
                  QString::fromStdString(Directory::join(PLUGINS_DIR, "Chihuahua.conf")));

    manager->reloadPluginDirectory();

    /* And now we can safely compare */
    QCOMPARE(actual1, expected1);
    QCOMPARE(actual2, expected2);
}

void PluginTest::reload() {
    /* Keep dog sleeping */
    QVERIFY(manager->loadState("Dog") == AbstractPluginManager::NotLoaded);

    /* Rename him */
    Configuration conf(Directory::join(PLUGINS_DIR, "Dog.conf"));
    conf.group("metadata")->setValue<string>("name", "Angry Beast");
    conf.save();

    /* Is dog still sleeping? */
    QVERIFY(manager->reload("Dog") == AbstractPluginManager::NotLoaded);

    /* Clean everything up before parents come home */
    conf.group("metadata")->setValue<string>("name", "A simple dog plugin");
    conf.save();

    /* And silently scare yourself to death */
    QVERIFY(*manager->metadata("Dog")->name() == "Angry Beast");

    /* Nightmare continues, try to really load dog */
    QVERIFY(manager->load("Dog") == AbstractPluginManager::LoadOk);

    /* Hopefully, Angry Beast was only a bad dream */
    QVERIFY(*manager->metadata("Dog")->name() == "A simple dog plugin");
}

}}}
