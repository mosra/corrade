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

#include "PluginManager/Manager.h"
#include "TestSuite/Tester.h"
#include "TestSuite/Compare/Container.h"
#include "Utility/Directory.h"

#include "AbstractAnimal.h"
#include "AbstractFood.h"
#include "AbstractDeletable.h"

#include "TestConfigure.h"
#include "corradePluginManagerConfigure.h"

using Corrade::Utility::Directory;

static void initialize() {
    PLUGIN_IMPORT(Canary)
}

namespace Corrade { namespace PluginManager { namespace Test {

class Test: public TestSuite::Tester {
    public:
        Test();

        void nameList();
        void errors();

        void staticPlugin();
        void dynamicPlugin();
        void staticPluginInitFini();
        void dynamicPluginInitFini();

        void deletable();
        void hierarchy();
        void crossManagerDependencies();
        void usedByZombies();

        void reloadPluginDirectory();

        void debug();
};

Test::Test() {
    addTests({&Test::nameList,
              &Test::errors,

              &Test::staticPlugin,
              &Test::dynamicPlugin,
              &Test::staticPluginInitFini,
              &Test::dynamicPluginInitFini,

              &Test::deletable,
              &Test::hierarchy,
              &Test::crossManagerDependencies,
              &Test::usedByZombies,
              &Test::reloadPluginDirectory,

              &Test::debug});

    initialize();
}

void Test::nameList() {
    {
        PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Canary", "Chihuahua", "Dog", "Snail"}), TestSuite::Compare::Container);
    }

    /* Check if the list of dynamic plugins is cleared after destructing */
    PluginManager::Manager<AbstractAnimal> manager(Directory::join(PLUGINS_DIR, "inexistent"));

    CORRADE_COMPARE_AS(manager.pluginList(), std::vector<std::string>{
        "Canary"}, TestSuite::Compare::Container);
}

void Test::errors() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    /** @todo Wrong plugin version (it would be hard) */
    /** @todo Wrong interface version */

    /* Wrong metadata file */
    CORRADE_COMPARE(manager.loadState("Snail"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(manager.load("Snail"), LoadState::WrongMetadataFile);
}

void Test::staticPlugin() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);
    CORRADE_COMPARE(*manager.metadata("Canary")->name(), "I'm allergic to canaries!");
    CORRADE_COMPARE(manager.metadata("Canary")->authors()[0], "Vladimír Vondruš <mosra@centrum.cz>");
    CORRADE_COMPARE(manager.metadata("Canary")->version(), "1.0");

    AbstractAnimal* animal = manager.instance("Canary");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail());
    CORRADE_COMPARE(animal->name(), "Achoo");
    CORRADE_COMPARE(animal->legCount(), 2);

    CORRADE_COMPARE(manager.unload("Canary"), LoadState::Static);
}

void Test::dynamicPlugin() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(*manager.metadata("Dog")->name(), "A simple dog plugin");

    AbstractAnimal* animal = manager.instance("Dog");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail());
    CORRADE_COMPARE(animal->name(), "Doug");
    CORRADE_COMPARE(animal->legCount(), 4);

    /* Try to unload plugin when instance is used */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::Used);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);

    /* Plugin can be unloaded after destroying all instances in which
       canBeDeleted() returns false. */
    delete animal;
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
}

void Test::staticPluginInitFini() {
    std::ostringstream out;
    Debug::setOutput(&out);

    {
        /* Initialization is right after manager assigns them to itself */
        out.str({});
        PluginManager::Manager<AbstractAnimal> manager{std::string()};
        CORRADE_COMPARE_AS(manager.pluginList(), std::vector<std::string>{
            "Canary"}, TestSuite::Compare::Container);
        CORRADE_COMPARE(out.str(), "Canary initialized\n");

        /* Finalization is right before manager frees them */
        out.str({});
    }

    CORRADE_COMPARE(out.str(), "Canary finalized\n");
}

void Test::dynamicPluginInitFini() {
    std::ostringstream out;
    Debug::setOutput(&out);

    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    /* Initialization is right after manager loads them */
    out.str({});
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(out.str(), "Dog initialized\n");

    /* Finalization is right before manager unloads them */
    out.str({});
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(out.str(), "Dog finalized\n");
}

void Test::deletable() {
    PluginManager::Manager<AbstractDeletable> deletableManager(Directory::join(PLUGINS_DIR, "deletable"));

    /* Load plugin where canBeDeleted() returns true */
    CORRADE_COMPARE(deletableManager.load("Deletable"), LoadState::Loaded);

    unsigned int var = 0;

    /* create an instance and connect it to local variable, which will be
       changed on destruction */
    AbstractDeletable* deletable = deletableManager.instance("Deletable");
    deletable->set(&var);

    /* plugin destroys all instances on deletion => the variable will be changed */
    CORRADE_COMPARE(var, 0);
    CORRADE_COMPARE(deletableManager.unload("Deletable"), LoadState::NotLoaded);
    CORRADE_COMPARE(var, 0xDEADBEEF);
}

void Test::hierarchy() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    CORRADE_COMPARE(manager.load("Chihuahua"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(*manager.metadata("Chihuahua")->name(), "The smallest dog in the world.");
    CORRADE_COMPARE(manager.metadata("Chihuahua")->depends().size(), 1);
    CORRADE_COMPARE(manager.metadata("Chihuahua")->depends()[0], "Dog");
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy().size(), 1);
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy()[0], "Chihuahua");

    AbstractAnimal* animal = manager.instance("Chihuahua");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail()); // inherited from dog
    CORRADE_COMPARE(animal->legCount(), 4); // this too
    CORRADE_COMPARE(animal->name(), "Rodriguez");

    /* Try to unload plugin when another is depending on it */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::Required);

    /* Unload chihuahua plugin, then try again */
    delete animal;
    CORRADE_COMPARE(manager.unload("Chihuahua"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Dog")->usedBy().empty());
}

void Test::crossManagerDependencies() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);
    PluginManager::Manager<AbstractFood> foodManager(Directory::join(PLUGINS_DIR, "food"));

    /* Load HotDog */
    CORRADE_COMPARE(foodManager.load("HotDog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(foodManager.metadata("HotDog")->depends().size(), 1);
    CORRADE_COMPARE(foodManager.metadata("HotDog")->depends()[0], "Dog");
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy().size(), 1);
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy()[0], "HotDog");

    /* Verify hotdog */
    AbstractFood* hotdog = foodManager.instance("HotDog");
    CORRADE_VERIFY(!hotdog->isTasty());
    CORRADE_COMPARE(hotdog->weight(), 6800);

    /* Try to unload dog while dog is used in hotdog */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::Required);

    /* Destroy hotdog, then try again */
    delete hotdog;
    CORRADE_COMPARE(foodManager.unload("HotDog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Dog")->usedBy().empty());
}

void Test::usedByZombies() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);
    PluginManager::Manager<AbstractFood> foodManager(Directory::join(PLUGINS_DIR, "food"));

    /* HotDogWithSnail depends on Dog and Snail, which cannot be loaded, so the
       loading fails too. Dog plugin then shouldn't have HotDogWithSnail in
       usedBy list. */

    CORRADE_COMPARE(foodManager.load("HotDogWithSnail"), LoadState::UnresolvedDependency);
    CORRADE_COMPARE(foodManager.loadState("HotDogWithSnail"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Dog")->usedBy().empty());
}

void Test::reloadPluginDirectory() {
    PluginManager::Manager<AbstractAnimal> manager(PLUGINS_DIR);

    /* Load Dog and rename the plugin */
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    Directory::move(Directory::join(PLUGINS_DIR, std::string("Dog") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(PLUGINS_DIR, std::string("LostDog") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(PLUGINS_DIR, "Dog.conf"),
                    Directory::join(PLUGINS_DIR, "LostDog.conf"));

    /* Rename Chihuahua */
    Directory::move(Directory::join(PLUGINS_DIR, std::string("Chihuahua") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(PLUGINS_DIR, std::string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(PLUGINS_DIR, "Chihuahua.conf"),
                    Directory::join(PLUGINS_DIR, "LostChihuahua.conf"));

    /* Reload plugin dir and check new name list */
    manager.reloadPluginDirectory();
    std::vector<std::string> actual1 = manager.pluginList();

    /* Unload Dog and it should disappear from the list */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    manager.reloadPluginDirectory();
    std::vector<std::string> actual2 = manager.pluginList();

    /** @todo Also test that "WrongMetadataFile" plugins are reloaded */

    /* Rename everything back and clean up */
    Directory::move(Directory::join(PLUGINS_DIR, std::string("LostDog") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(PLUGINS_DIR, std::string("Dog") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(PLUGINS_DIR, "LostDog.conf"),
                    Directory::join(PLUGINS_DIR, "Dog.conf"));

    Directory::move(Directory::join(PLUGINS_DIR, std::string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(PLUGINS_DIR, std::string("Chihuahua") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(PLUGINS_DIR, "LostChihuahua.conf"),
                    Directory::join(PLUGINS_DIR, "Chihuahua.conf"));

    manager.reloadPluginDirectory();

    /* And now we can safely compare */
    CORRADE_COMPARE_AS(actual1, (std::vector<std::string>{
        "Canary", "Dog", "LostChihuahua", "LostDog", "Snail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actual2, (std::vector<std::string>{
        "Canary", "LostChihuahua", "LostDog", "Snail"}), TestSuite::Compare::Container);
}

void Test::debug() {
    std::ostringstream o;

    Debug(&o) << LoadState::UnresolvedDependency;
    CORRADE_COMPARE(o.str(), "PluginManager::LoadState::UnresolvedDependency\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::Test)
