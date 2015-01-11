/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include "Corrade/PluginManager/Manager.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/Directory.h"

#include "AbstractAnimal.h"
#include "AbstractFood.h"
#include "AbstractDeletable.h"

#include "../configure.h"
#include "configure.h"

using Corrade::Utility::Directory;

static void initialize() {
    CORRADE_PLUGIN_IMPORT(Canary)
}

namespace Corrade { namespace PluginManager { namespace Test {

struct Test: TestSuite::Tester {
    explicit Test();

    void nameList();
    void wrongPluginVersion();
    void wrongPluginInterface();
    void wrongMetadataFile();
    void loadNonexistent();
    void unloadNonexistent();

    void staticPlugin();
    #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    void dynamicPlugin();
    #endif
    void staticPluginInitFini();
    #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    void dynamicPluginInitFini();
    #endif

    void deletable();
    void hierarchy();
    void crossManagerDependencies();
    void unresolvedDependencies();

    void reloadPluginDirectory();

    void staticProvides();
    #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    void dynamicProvides();
    void dynamicProvidesDependency();
    #endif

    void debug();
};

Test::Test() {
    addTests({&Test::nameList,
              &Test::wrongPluginVersion,
              &Test::wrongPluginInterface,
              &Test::wrongMetadataFile,
              &Test::loadNonexistent,
              &Test::unloadNonexistent,

              &Test::staticPlugin,
              #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
              &Test::dynamicPlugin,
              #endif
              &Test::staticPluginInitFini,
              #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
              &Test::dynamicPluginInitFini,
              #endif

              &Test::deletable,
              &Test::hierarchy,
              &Test::crossManagerDependencies,
              &Test::unresolvedDependencies,
              &Test::reloadPluginDirectory,

              &Test::staticProvides,
              #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
              &Test::dynamicProvides,
              &Test::dynamicProvidesDependency,
              #endif

              &Test::debug});

    initialize();
}

namespace {
    #ifndef CMAKE_INTDIR
    const std::string pluginsDir = PLUGINS_DIR;
    const std::string foodPluginsDir = Directory::join(PLUGINS_DIR, "food");
    const std::string deletablePluginsDir = Directory::join(PLUGINS_DIR, "deletable");
    #else
    const std::string pluginsDir = Directory::join(PLUGINS_DIR, CMAKE_INTDIR);
    const std::string foodPluginsDir = Directory::join(Directory::join(PLUGINS_DIR, "food"), CMAKE_INTDIR);
    const std::string deletablePluginsDir = Directory::join(Directory::join(PLUGINS_DIR, "deletable"), CMAKE_INTDIR);
    #endif
}

void Test::nameList() {
    #if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
    {
        PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Chihuahua", "Dog", "Snail"}), TestSuite::Compare::Container);
    }
    #endif

    /* Check if the list of dynamic plugins is cleared after destructing */
    PluginManager::Manager<AbstractAnimal> manager("nonexistent");

    CORRADE_COMPARE_AS(manager.pluginList(), std::vector<std::string>{
        "Canary"}, TestSuite::Compare::Container);
}

void Test::wrongPluginVersion() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Can't test plugin version of static plugins");
    #else
    std::ostringstream out;
    Error::setOutput(&out);

    PluginManager::Manager<AbstractFood> foodManager(foodPluginsDir);
    CORRADE_COMPARE(foodManager.load("OldBread"), PluginManager::LoadState::WrongPluginVersion);
    CORRADE_COMPARE(foodManager.loadState("OldBread"), PluginManager::LoadState::NotLoaded);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong version of plugin OldBread, expected 4 but got 0\n");
    #endif
}

void Test::wrongPluginInterface() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Can't test plugin interface of static plugins");
    #else
    std::ostringstream out;
    Error::setOutput(&out);

    PluginManager::Manager<AbstractFood> foodManager(foodPluginsDir);
    CORRADE_COMPARE(foodManager.load("RottenTomato"), PluginManager::LoadState::WrongInterfaceVersion);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong interface string of plugin RottenTomato, expected cz.mosra.Corrade.PluginManager.Test.AbstractFood/1.0 but got cz.mosra.Corrade.PluginManager.Test.AbstractFood/0.1\n");
    #endif
}

void Test::wrongMetadataFile() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Can't test metadata file of static plugins");
    #else
    std::ostringstream out;
    Error::setOutput(&out);

    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);
    CORRADE_COMPARE(manager.loadState("Snail"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(manager.load("Snail"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(out.str(),
        "Utility::Configuration::Configuration(): key/value pair without '=' character\n"
        "PluginManager::Manager::load(): plugin Snail is not ready to load: PluginManager::LoadState::WrongMetadataFile\n");
    #endif
}

void Test::loadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_COMPARE(manager.load("Nonexistent"), LoadState::NotFound);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): plugin Nonexistent is not static and was not found in " + pluginsDir + "\n");
}

void Test::unloadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_COMPARE(manager.unload("Nonexistent"), LoadState::NotFound);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Nonexistent was not found\n");
}

void Test::staticPlugin() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);
    CORRADE_COMPARE(manager.metadata("Canary")->data().value("description"), "I'm allergic to canaries!");

    std::unique_ptr<AbstractAnimal> animal = manager.instance("Canary");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail());
    CORRADE_COMPARE(animal->name(), "Achoo");
    CORRADE_COMPARE(animal->legCount(), 2);

    CORRADE_COMPARE(manager.unload("Canary"), LoadState::Static);
}

#if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
void Test::dynamicPlugin() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.metadata("Dog")->data().value("description"), "A simple dog plugin.");

    {
        std::unique_ptr<AbstractAnimal> animal = manager.instance("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_VERIFY(animal->hasTail());
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);

        /* Try to unload plugin when instance is used */
        std::ostringstream out;
        Error::setOutput(&out);
        CORRADE_COMPARE(manager.unload("Dog"), LoadState::Used);
        CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Dog is currently used and cannot be deleted\n");
        CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    }

    /* Plugin can be unloaded after destroying all instances in which
       canBeDeleted() returns false. */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
}
#endif

void Test::staticPluginInitFini() {
    std::ostringstream out;
    Debug::setOutput(&out);

    {
        /* Initialization is right after manager assigns them to itself */
        out.str({});
        PluginManager::Manager<AbstractAnimal> manager{"inexistentDir"};
        CORRADE_COMPARE_AS(manager.pluginList(), std::vector<std::string>{
            "Canary"}, TestSuite::Compare::Container);
        CORRADE_COMPARE(out.str(), "Canary initialized\n");

        /* Finalization is right before manager frees them */
        out.str({});
    }

    CORRADE_COMPARE(out.str(), "Canary finalized\n");
}

#if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
void Test::dynamicPluginInitFini() {
    std::ostringstream out;
    Debug::setOutput(&out);

    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    /* Initialization is right after manager loads them */
    out.str({});
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(out.str(), "Dog initialized\n");

    /* Finalization is right before manager unloads them */
    out.str({});
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(out.str(), "Dog finalized\n");
}
#endif

void Test::deletable() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Can't test because static plugins can't be unloaded");
    #else
    PluginManager::Manager<AbstractDeletable> deletableManager(deletablePluginsDir);

    /* Load plugin where canBeDeleted() returns true */
    CORRADE_COMPARE(deletableManager.load("Deletable"), LoadState::Loaded);

    unsigned int var = 0;

    /* create an instance and connect it to local variable, which will be
       changed on destruction */
    AbstractDeletable* deletable = deletableManager.instance("Deletable").release();
    deletable->set(&var);

    /* plugin destroys all instances on deletion => the variable will be changed */
    CORRADE_COMPARE(var, 0);
    CORRADE_COMPARE(deletableManager.unload("Deletable"), LoadState::NotLoaded);
    CORRADE_COMPARE(var, 0xDEADBEEF);
    #endif
}

void Test::hierarchy() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Dependency hierarchy is meaningful only for dynamic plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    CORRADE_COMPARE(manager.load("Chihuahua"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.metadata("Chihuahua")->data().value("description"), "The smallest dog in the world.");
    CORRADE_COMPARE(manager.metadata("Chihuahua")->depends(),
        std::vector<std::string>{"Dog"});
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{"Chihuahua"});

    {
        std::unique_ptr<AbstractAnimal> animal = manager.instance("Chihuahua");
        CORRADE_VERIFY(animal);
        CORRADE_VERIFY(animal->hasTail()); // inherited from dog
        CORRADE_COMPARE(animal->legCount(), 4); // this too
        CORRADE_COMPARE(animal->name(), "Rodriguez");

        /* Try to unload plugin when another is depending on it */
        std::ostringstream out;
        Error::setOutput(&out);
        CORRADE_COMPARE(manager.unload("Dog"), LoadState::Required);
        CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Dog is required by other plugins: {Chihuahua}\n");
    }

    /* After deleting instance, unload chihuahua plugin, then try again */
    CORRADE_COMPARE(manager.unload("Chihuahua"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Dog")->usedBy().empty());
    #endif
}

void Test::crossManagerDependencies() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);
    PluginManager::Manager<AbstractFood> foodManager(foodPluginsDir);

    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Cross-manager dependencies are meaningful only for dynamic plugins");
    #else
    /* Load HotDog */
    CORRADE_COMPARE(foodManager.load("HotDog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(foodManager.metadata("HotDog")->depends(),
        std::vector<std::string>{"Dog"});
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{"HotDog"});

    {
        /* Verify hotdog */
        std::unique_ptr<AbstractFood> hotdog = foodManager.instance("HotDog");
        CORRADE_VERIFY(hotdog);
        CORRADE_VERIFY(!hotdog->isTasty());
        CORRADE_COMPARE(hotdog->weight(), 6800);

        /* Try to unload dog while dog is used in hotdog */
        CORRADE_COMPARE(manager.unload("Dog"), LoadState::Required);
    }

    /* After destroying hotdog try again */
    CORRADE_COMPARE(foodManager.unload("HotDog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{});
    #endif

    /* Verify that the plugin can be instanced only through its own manager */
    CORRADE_VERIFY(manager.instance("Canary"));

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_VERIFY(!foodManager.instance("Canary"));
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::instance(): plugin Canary is not loaded\n");
}

void Test::unresolvedDependencies() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("UsedBy list is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);
    PluginManager::Manager<AbstractFood> foodManager(foodPluginsDir);

    /* HotDogWithSnail depends on Dog and Snail, which cannot be loaded, so the
       loading fails too. Dog plugin then shouldn't have HotDogWithSnail in
       usedBy list. */

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_COMPARE(foodManager.load("HotDogWithSnail"), LoadState::UnresolvedDependency);
    CORRADE_COMPARE(out.str(),
        "PluginManager::Manager::load(): plugin Snail is not ready to load: PluginManager::LoadState::WrongMetadataFile\n"
        "PluginManager::Manager::load(): unresolved dependency Snail of plugin HotDogWithSnail\n");
    CORRADE_COMPARE(foodManager.loadState("HotDogWithSnail"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{});
    #endif
}

void Test::reloadPluginDirectory() {
    #if defined(CORRADE_TARGET_NACL_NEWLIB) || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_SKIP("Plugin directory is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    /* Load Dog and rename the plugin */
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    Directory::move(Directory::join(pluginsDir, std::string("Dog") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(pluginsDir, std::string("LostDog") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(pluginsDir, "Dog.conf"),
                    Directory::join(pluginsDir, "LostDog.conf"));

    /* Rename Chihuahua */
    Directory::move(Directory::join(pluginsDir, std::string("Chihuahua") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(pluginsDir, std::string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(pluginsDir, "Chihuahua.conf"),
                    Directory::join(pluginsDir, "LostChihuahua.conf"));

    /* Reload plugin dir and check new name list */
    manager.reloadPluginDirectory();
    std::vector<std::string> actual1 = manager.pluginList();

    /* Unload Dog and it should disappear from the list */
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    manager.reloadPluginDirectory();
    std::vector<std::string> actual2 = manager.pluginList();

    /** @todo Also test that "WrongMetadataFile" plugins are reloaded */

    /* Rename everything back and clean up */
    Directory::move(Directory::join(pluginsDir, std::string("LostDog") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(pluginsDir, std::string("Dog") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(pluginsDir, "LostDog.conf"),
                    Directory::join(pluginsDir, "Dog.conf"));

    Directory::move(Directory::join(pluginsDir, std::string("LostChihuahua") + PLUGIN_FILENAME_SUFFIX),
                    Directory::join(pluginsDir, std::string("Chihuahua") + PLUGIN_FILENAME_SUFFIX));
    Directory::move(Directory::join(pluginsDir, "LostChihuahua.conf"),
                    Directory::join(pluginsDir, "Chihuahua.conf"));

    manager.reloadPluginDirectory();

    /* And now we can safely compare */
    CORRADE_COMPARE_AS(actual1, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "LostChihuahua", "LostDog", "Snail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actual2, (std::vector<std::string>{
        "Bulldog", "Canary", "LostChihuahua", "LostDog", "Snail"}), TestSuite::Compare::Container);
    #endif
}

void Test::staticProvides() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    CORRADE_COMPARE(manager.metadata("Canary")->provides(), std::vector<std::string>{"JustSomeBird"});

    CORRADE_COMPARE(manager.loadState("JustSomeBird"), LoadState::Static);
    CORRADE_VERIFY(manager.metadata("JustSomeBird"));
    CORRADE_COMPARE(manager.metadata("JustSomeBird")->name(), "Canary");

    const auto animal = manager.instance("JustSomeBird");
    CORRADE_COMPARE(animal->metadata()->name(), "Canary");
}

#if !defined(CORRADE_TARGET_NACL_NEWLIB) && !defined(CORRADE_TARGET_EMSCRIPTEN)
void Test::dynamicProvides() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    CORRADE_COMPARE(manager.metadata("Dog")->provides(), std::vector<std::string>{"JustSomeMammal"});

    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("JustSomeMammal"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::Loaded);
    CORRADE_VERIFY(manager.metadata("JustSomeMammal"));
    CORRADE_COMPARE(manager.metadata("JustSomeMammal")->name(), "Dog");

    const auto animal = manager.instance("JustSomeMammal");
    CORRADE_COMPARE(animal->metadata()->name(), "Dog");
}

void Test::dynamicProvidesDependency() {
    PluginManager::Manager<AbstractAnimal> manager(pluginsDir);

    /* The plugin JustSomeMammal exists, but is an alias and cannot be used as
       a dependency */
    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Bulldog"));
    CORRADE_COMPARE(manager.metadata("Bulldog")->depends(), std::vector<std::string>{"JustSomeMammal"});

    std::ostringstream out;
    Error::setOutput(&out);
    CORRADE_COMPARE(manager.load("Bulldog"), LoadState::UnresolvedDependency);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): unresolved dependency JustSomeMammal of plugin Bulldog\n");
}
#endif

void Test::debug() {
    std::ostringstream o;

    Debug(&o) << LoadState::Static;
    CORRADE_COMPARE(o.str(), "PluginManager::LoadState::Static\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::Test)
