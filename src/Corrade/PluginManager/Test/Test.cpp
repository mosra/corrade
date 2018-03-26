/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Containers/Array.h"
#include "Corrade/PluginManager/Manager.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/Configuration.h"

#include "AbstractAnimal.h"
#include "AbstractFood.h"
#include "AbstractDeletable.h"

#include "../configure.h"
#include "configure.h"

static void initialize() {
    CORRADE_PLUGIN_IMPORT(Canary)
}

namespace Corrade { namespace PluginManager { namespace Test {

struct Test: TestSuite::Tester {
    explicit Test();

    void pluginSearchPathsNotUsed();
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void pluginSearchPathsNotProvided();
    void pluginSearchPathsNotFound();
    #endif

    void nameList();

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void wrongMetadataFile();
    void unresolvedReference();
    void noPluginVersion();
    void wrongPluginVersion();
    void noPluginInterface();
    void wrongPluginInterface();
    void noPluginInitializer();
    void noPluginFinalizer();
    void noPluginInstancer();
    #endif

    void queryNonexistent();
    void loadNonexistent();
    void unloadNonexistent();

    void staticPlugin();
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void dynamicPlugin();
    #endif
    void staticPluginInitFini();
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void dynamicPluginInitFini();
    #endif

    void configuration();
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void configurationImplicit();
    #endif
    void deletable();
    void hierarchy();
    void destructionHierarchy();
    void crossManagerDependencies();
    void unresolvedDependencies();

    void reloadPluginDirectory();

    void staticProvides();
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void dynamicProvides();
    void dynamicProvidesDependency();
    void setPreferredPlugins();
    void setPreferredPluginsUnknownAlias();
    void setPreferredPluginsDoesNotProvide();
    void setPreferredPluginsOverridePrimaryPlugin();
    #endif

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    void utf8Path();
    #endif

    void debug();
};

Test::Test() {
    addTests({&Test::pluginSearchPathsNotUsed,
              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::pluginSearchPathsNotProvided,
              &Test::pluginSearchPathsNotFound,
              #endif

              &Test::nameList,

              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::wrongMetadataFile,
              &Test::unresolvedReference,
              &Test::noPluginVersion,
              &Test::wrongPluginVersion,
              &Test::noPluginInterface,
              &Test::wrongPluginInterface,
              &Test::noPluginInitializer,
              &Test::noPluginFinalizer,
              &Test::noPluginInstancer,
              #endif

              &Test::queryNonexistent,
              &Test::loadNonexistent,
              &Test::unloadNonexistent,

              &Test::staticPlugin,
              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::dynamicPlugin,
              #endif
              &Test::staticPluginInitFini,
              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::dynamicPluginInitFini,
              #endif

              &Test::configuration,
              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::configurationImplicit,
              #endif
              &Test::deletable,
              &Test::hierarchy,
              &Test::destructionHierarchy,
              &Test::crossManagerDependencies,
              &Test::unresolvedDependencies,
              &Test::reloadPluginDirectory,

              &Test::staticProvides,
              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::dynamicProvides,
              &Test::dynamicProvidesDependency,
              &Test::setPreferredPlugins,
              &Test::setPreferredPluginsUnknownAlias,
              &Test::setPreferredPluginsDoesNotProvide,
              &Test::setPreferredPluginsOverridePrimaryPlugin,
              #endif

              #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
              &Test::utf8Path,
              #endif

              &Test::debug});

    initialize();
}

namespace {
    #ifndef CMAKE_INTDIR
    const std::string pluginsDir = PLUGINS_DIR;
    #else
    const std::string pluginsDir = Utility::Directory::join(PLUGINS_DIR, CMAKE_INTDIR);
    #endif
}

void Test::pluginSearchPathsNotUsed() {
    struct SomePlugin: AbstractPlugin {
        static std::string pluginInterface() { return {}; }
    };

    /* Everything okay in this case (no assert) */
    std::ostringstream out;
    Error redirectError{&out};
    {
        PluginManager::Manager<SomePlugin> manager{"someDirectory"};
    }
    CORRADE_COMPARE(out.str(), "");
}

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
void Test::pluginSearchPathsNotProvided() {
    struct SomePlugin: AbstractPlugin {
        static std::string pluginInterface() { return {}; }
    };

    /* Complain that no plugin search path is set */
    std::ostringstream out;
    Error redirectError{&out};
    {
        PluginManager::Manager<SomePlugin> manager;
    }
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::Manager(): either pluginDirectory has to be set or T::pluginSearchPaths() is expected to have at least one entry\n");
}

void Test::pluginSearchPathsNotFound() {
    struct SomePlugin: AbstractPlugin {
        static std::vector<std::string> pluginSearchPaths() {
            return {"nonexistent", "/absolute/but/nonexistent"};
        }

        static std::string pluginInterface() { return {}; }
    };

    /* Complain that no plugin search path is set */
    std::ostringstream out;
    Warning redirectWarning{&out};
    {
        PluginManager::Manager<SomePlugin> manager;
    }
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::Manager(): none of the plugin search paths in {nonexistent, /absolute/but/nonexistent} exists and pluginDirectory was not set, falling back to current working directory\n");
}
#endif

void Test::nameList() {
    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    {
        PluginManager::Manager<AbstractAnimal> manager;

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "PitBull", "Snail"}), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
    }
    #endif

    {
        /* Check if the list of dynamic plugins is cleared after destructing */
        PluginManager::Manager<AbstractAnimal> manager("nonexistent");

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Canary"}), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
            "Canary", "JustSomeBird"}), TestSuite::Compare::Container);
    }

    #if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
    /* Check that explicitly specifying the same plugin path does the same */
    {
        PluginManager::Manager<AbstractAnimal> manager{pluginsDir};

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "PitBull", "Snail"}), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
    }
    #endif
}

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
namespace {

struct WrongPlugin: AbstractPlugin {
    static std::string pluginInterface() { return {}; }

    static std::vector<std::string> pluginSearchPaths() {
        return {
            #ifndef CMAKE_INTDIR
            Utility::Directory::join(PLUGINS_DIR, "wrong")
            #else
            Utility::Directory::join(Utility::Directory::join(PLUGINS_DIR, "wrong"), CMAKE_INTDIR)
            #endif
        };
    }

    explicit WrongPlugin(AbstractManager& manager, const std::string& plugin): AbstractPlugin{manager, plugin} {}
};

}

void Test::wrongMetadataFile() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<AbstractAnimal> manager;
    CORRADE_COMPARE(manager.loadState("Snail"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(manager.load("Snail"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(out.str(),
        "Utility::Configuration::Configuration(): key/value pair without '=' character\n"
        "PluginManager::Manager::load(): plugin Snail is not ready to load: PluginManager::LoadState::WrongMetadataFile\n");
}

void Test::unresolvedReference() {
    #ifdef CORRADE_TARGET_WINDOWS
    CORRADE_SKIP("At the moment, plugins are not compiled as modules on Windows, so this is not possible to test.");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("UnresolvedReference"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("UnresolvedReference"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot load plugin UnresolvedReference from \"";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void Test::noPluginVersion() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginVersion"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginVersion"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get version of plugin NoPluginVersion: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void Test::wrongPluginVersion() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<AbstractFood> foodManager;
    CORRADE_COMPARE(foodManager.load("OldBread"), PluginManager::LoadState::WrongPluginVersion);
    CORRADE_COMPARE(foodManager.loadState("OldBread"), PluginManager::LoadState::NotLoaded);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong version of plugin OldBread, expected 5 but got 0\n");
}

void Test::noPluginInterface() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInterface"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInterface"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix =  "PluginManager::Manager::load(): cannot get interface string of plugin NoPluginInterface: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void Test::wrongPluginInterface() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<AbstractFood> foodManager;
    CORRADE_COMPARE(foodManager.load("RottenTomato"), PluginManager::LoadState::WrongInterfaceVersion);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong interface string of plugin RottenTomato, expected cz.mosra.corrade.PluginManager.Test.AbstractFood/1.0 but got cz.mosra.corrade.PluginManager.Test.AbstractFood/0.1\n");
}

void Test::noPluginInitializer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInitializer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInitializer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get initializer of plugin NoPluginInitializer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void Test::noPluginFinalizer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginFinalizer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginFinalizer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get finalizer of plugin NoPluginFinalizer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void Test::noPluginInstancer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInstancer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInstancer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get instancer of plugin NoPluginInstancer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}
#endif

void Test::queryNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;
    CORRADE_VERIFY(!manager.metadata("Nonexistent"));
    CORRADE_COMPARE(manager.loadState("Nonexistent"), LoadState::NotFound);
}

void Test::loadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(manager.load("Nonexistent"), LoadState::NotFound);
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): plugin Nonexistent was not found\n");
    #else
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): plugin Nonexistent is not static and was not found in " + pluginsDir + "\n");
    #endif
}

void Test::unloadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(manager.unload("Nonexistent"), LoadState::NotFound);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Nonexistent was not found\n");
}

void Test::staticPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);
    CORRADE_COMPARE(manager.metadata("Canary")->data().value("description"), "I'm allergic to canaries!");

    std::unique_ptr<AbstractAnimal> animal = manager.instantiate("Canary");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail());
    CORRADE_COMPARE(animal->name(), "Achoo");
    CORRADE_COMPARE(animal->legCount(), 2);

    CORRADE_COMPARE(manager.unload("Canary"), LoadState::Static);
}

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
void Test::dynamicPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.metadata("Dog")->data().value("description"), "A simple dog plugin.");

    {
        std::unique_ptr<AbstractAnimal> animal = manager.instantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_VERIFY(animal->hasTail());
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);

        /* Try to unload plugin when instance is used */
        std::ostringstream out;
        Error redirectError{&out};
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
    Debug redirectDebug{&out};

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

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
void Test::dynamicPluginInitFini() {
    std::ostringstream out;
    Debug redirectDebug{&out};

    PluginManager::Manager<AbstractAnimal> manager;

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

void Test::configuration() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);

    std::unique_ptr<AbstractAnimal> animal = manager.instantiate("Canary");
    CORRADE_COMPARE(animal->name(), "Achoo");

    CORRADE_COMPARE(manager.metadata("Canary")->configuration().value("name"), "Achoo");
    CORRADE_COMPARE(animal->configuration().value("name"), "Achoo");

    animal->configuration().setValue("name", "Bird!!");
    CORRADE_COMPARE(animal->name(), "Bird!!");

    /* Other instances are not affected */
    std::unique_ptr<AbstractAnimal> animal2 = manager.instantiate("Canary");
    CORRADE_COMPARE(animal2->name(), "Achoo");
}

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
void Test::configurationImplicit() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::unique_ptr<AbstractAnimal> animal = manager.loadAndInstantiate("Dog");
    CORRADE_VERIFY(animal);

    /* The plugin should get an implicitly created configuration */
    CORRADE_COMPARE(manager.metadata("Dog")->configuration().valueCount(), 0);
    CORRADE_COMPARE(animal->configuration().valueCount(), 0);

    /* And a modifiable one */
    animal->configuration().setValue("name", "UPDOG");
    CORRADE_COMPARE(animal->configuration().value("name"), "UPDOG");
}
#endif

void Test::deletable() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_SKIP("Can't test because static plugins can't be unloaded");
    #else
    PluginManager::Manager<AbstractDeletable> deletableManager;

    /* Load plugin where canBeDeleted() returns true */
    CORRADE_COMPARE(deletableManager.load("Deletable"), LoadState::Loaded);

    unsigned int var = 0;

    /* create an instance and connect it to local variable, which will be
       changed on destruction */
    AbstractDeletable* deletable = deletableManager.instantiate("Deletable").release();
    deletable->set(&var);

    /* plugin destroys all instances on deletion => the variable will be changed */
    CORRADE_COMPARE(var, 0);
    CORRADE_COMPARE(deletableManager.unload("Deletable"), LoadState::NotLoaded);
    CORRADE_COMPARE(var, 0xDEADBEEF);
    #endif
}

void Test::hierarchy() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_SKIP("Dependency hierarchy is meaningful only for dynamic plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.load("PitBull"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.metadata("PitBull")->data().value("description"), "I'M ANGRY!!");
    CORRADE_COMPARE(manager.metadata("PitBull")->depends(),
        std::vector<std::string>{"Dog"});
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{"PitBull"});

    {
        std::unique_ptr<AbstractAnimal> animal = manager.instantiate("PitBull");
        CORRADE_VERIFY(animal->hasTail()); // inherited from dog
        CORRADE_COMPARE(animal->legCount(), 4); // this too
        CORRADE_COMPARE(animal->name(), "Rodriguez");

        /* Try to unload plugin when another is depending on it */
        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_COMPARE(manager.unload("Dog"), LoadState::Required);
        CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Dog is required by other plugins: {PitBull}\n");
    }

    /* After deleting instance, unload PitBull plugin, then try again */
    CORRADE_COMPARE(manager.unload("PitBull"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Dog")->usedBy().empty());
    #endif
}

void Test::destructionHierarchy() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_SKIP("Dependency hierarchy is meaningful only for dynamic plugins");
    #else
    /* Dog needs to be ordered first in the map for this test case to work.
       Basically I'm testing that the unload of plugins happens in the
       right order and that I'm not using invalid iterators at any point. */
    CORRADE_VERIFY(std::string{"Dog"} < std::string{"PitBull"});

    {
        PluginManager::Manager<AbstractAnimal> manager;
        CORRADE_COMPARE(manager.load("PitBull"), LoadState::Loaded);
        CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    }

    /* It should not crash, assert or fire an exception on destruction */
    CORRADE_VERIFY(true);
    #endif
}

void Test::crossManagerDependencies() {
    PluginManager::Manager<AbstractAnimal> manager;
    PluginManager::Manager<AbstractFood> foodManager;

    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
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
        std::unique_ptr<AbstractFood> hotdog = foodManager.instantiate("HotDog");
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
    CORRADE_VERIFY(manager.instantiate("Canary"));

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!foodManager.instantiate("Canary"));
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::instance(): plugin Canary is not loaded\n");
}

void Test::unresolvedDependencies() {
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_SKIP("UsedBy list is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;
    PluginManager::Manager<AbstractFood> foodManager;

    /* HotDogWithSnail depends on Dog and Snail, which cannot be loaded, so the
       loading fails too. Dog plugin then shouldn't have HotDogWithSnail in
       usedBy list. */

    std::ostringstream out;
    Error redirectError{&out};
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
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_SKIP("Plugin directory is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;

    /* Load PitBull and rename the plugin */
    CORRADE_COMPARE(manager.load("PitBull"), LoadState::Loaded);
    Utility::Directory::move(Utility::Directory::join(pluginsDir, std::string("PitBull") + PLUGIN_FILENAME_SUFFIX),
                             Utility::Directory::join(pluginsDir, std::string("LostPitBull") + PLUGIN_FILENAME_SUFFIX));
    Utility::Directory::move(Utility::Directory::join(pluginsDir, "PitBull.conf"),
                             Utility::Directory::join(pluginsDir, "LostPitBull.conf"));

    /* Rename Snail */
    Utility::Directory::move(Utility::Directory::join(pluginsDir, std::string("Snail") + PLUGIN_FILENAME_SUFFIX),
                             Utility::Directory::join(pluginsDir, std::string("LostSnail") + PLUGIN_FILENAME_SUFFIX));
    Utility::Directory::move(Utility::Directory::join(pluginsDir, "Snail.conf"),
                             Utility::Directory::join(pluginsDir, "LostSnail.conf"));

    /* Reload plugin dir and check new name list */
    manager.reloadPluginDirectory();
    std::vector<std::string> actualPlugins1 = manager.pluginList();
    std::vector<std::string> actualAliases1 = manager.aliasList();

    /* Unload PitBull and it should disappear from the list */
    CORRADE_COMPARE(manager.unload("PitBull"), LoadState::NotLoaded);
    manager.reloadPluginDirectory();
    std::vector<std::string> actualPlugins2 = manager.pluginList();
    std::vector<std::string> actualAliases2 = manager.aliasList();

    /** @todo Also test that "WrongMetadataFile" plugins are reloaded */

    /* Rename everything back and clean up */
    Utility::Directory::move(Utility::Directory::join(pluginsDir, std::string("LostPitBull") + PLUGIN_FILENAME_SUFFIX),
                             Utility::Directory::join(pluginsDir, std::string("PitBull") + PLUGIN_FILENAME_SUFFIX));
    Utility::Directory::move(Utility::Directory::join(pluginsDir, "LostPitBull.conf"),
                             Utility::Directory::join(pluginsDir, "PitBull.conf"));

    Utility::Directory::move(Utility::Directory::join(pluginsDir, std::string("LostSnail") + PLUGIN_FILENAME_SUFFIX),
                             Utility::Directory::join(pluginsDir, std::string("Snail") + PLUGIN_FILENAME_SUFFIX));
    Utility::Directory::move(Utility::Directory::join(pluginsDir, "LostSnail.conf"),
                             Utility::Directory::join(pluginsDir, "Snail.conf"));

    manager.reloadPluginDirectory();

    /* And now we can safely compare */
    CORRADE_COMPARE_AS(actualPlugins1, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "LostPitBull", "LostSnail", "PitBull"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualAliases1, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "LostPitBull", "LostSnail", "PitBull"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualPlugins2, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "LostPitBull", "LostSnail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualAliases2, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "LostPitBull", "LostSnail"}), TestSuite::Compare::Container);
    #endif
}

void Test::staticProvides() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.metadata("Canary")->provides(), std::vector<std::string>{"JustSomeBird"});

    CORRADE_COMPARE(manager.loadState("JustSomeBird"), LoadState::Static);
    CORRADE_VERIFY(manager.metadata("JustSomeBird"));
    CORRADE_COMPARE(manager.metadata("JustSomeBird")->name(), "Canary");

    const auto animal = manager.instantiate("JustSomeBird");
    CORRADE_COMPARE(animal->plugin(), "JustSomeBird");
    CORRADE_COMPARE(animal->metadata()->name(), "Canary");
}

#if !defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(CORRADE_TARGET_WINDOWS_RT) && !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_ANDROID)
void Test::dynamicProvides() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.metadata("Dog")->provides(), std::vector<std::string>{"JustSomeMammal"});

    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("JustSomeMammal"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::Loaded);
    CORRADE_VERIFY(manager.metadata("JustSomeMammal"));
    CORRADE_COMPARE(manager.metadata("JustSomeMammal")->name(), "Dog");

    const auto animal = manager.instantiate("JustSomeMammal");
    CORRADE_COMPARE(animal->plugin(), "JustSomeMammal");
    CORRADE_COMPARE(animal->metadata()->name(), "Dog");

    /* Trying to unload the plugin via any name has to fail as there is an
       instance active */
    CORRADE_COMPARE(manager.unload("JustSomeMammal"), LoadState::Used);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::Used);
}

void Test::dynamicProvidesDependency() {
    PluginManager::Manager<AbstractAnimal> manager;

    /* The plugin JustSomeMammal exists, but is an alias and cannot be used as
       a dependency */
    CORRADE_COMPARE(manager.loadState("JustSomeMammal"), LoadState::NotLoaded);
    CORRADE_VERIFY(manager.metadata("Bulldog"));
    CORRADE_COMPARE(manager.metadata("Bulldog")->depends(), std::vector<std::string>{"JustSomeMammal"});

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(manager.load("Bulldog"), LoadState::UnresolvedDependency);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): unresolved dependency JustSomeMammal of plugin Bulldog\n");
}

void Test::setPreferredPlugins() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.metadata("Dog")->provides(), std::vector<std::string>{"JustSomeMammal"});
    CORRADE_COMPARE(manager.metadata("PitBull")->provides(), (std::vector<std::string>{"JustSomeMammal", "Dog"}));

    /* Implicit state */
    CORRADE_COMPARE(manager.metadata("JustSomeMammal")->name(), "Dog");

    /* Override */
    manager.setPreferredPlugins("JustSomeMammal", {"Chihuahua", "PitBull"});
    CORRADE_COMPARE(manager.metadata("JustSomeMammal")->name(), "PitBull");

    /* Reloading plugin directory resets the mapping back */
    manager.reloadPluginDirectory();
    CORRADE_COMPARE(manager.metadata("JustSomeMammal")->name(), "Dog");
}

void Test::setPreferredPluginsUnknownAlias() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    manager.setPreferredPlugins("Chihuahua", {"PitBull"});
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::setPreferredPlugins(): Chihuahua is not a known alias\n");
}

void Test::setPreferredPluginsDoesNotProvide() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    manager.setPreferredPlugins("Dog", {"Snail"});
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::setPreferredPlugins(): Snail does not provide Dog\n");
}

void Test::setPreferredPluginsOverridePrimaryPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.metadata("PitBull")->provides(), (std::vector<std::string>{"JustSomeMammal", "Dog"}));

    /* Implicit state */
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "Dog");

    /* Override */
    manager.setPreferredPlugins("Dog", {"PitBull"});
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "PitBull");

    /* Reloading plugin directory resets the mapping back */
    manager.reloadPluginDirectory();
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "Dog");
}

void Test::utf8Path() {
    /* Copy the dog plugin to a new UTF-8 path */
    const std::string utf8PluginsDir = Utility::Directory::join(pluginsDir, "hýždě");
    CORRADE_VERIFY(Utility::Directory::mkpath(utf8PluginsDir));
    Utility::Directory::write(
        Utility::Directory::join(utf8PluginsDir, std::string("Dog") + PLUGIN_FILENAME_SUFFIX),
        Utility::Directory::mapRead(Utility::Directory::join(pluginsDir, std::string("Dog") + PLUGIN_FILENAME_SUFFIX)));
    Utility::Directory::write(
        Utility::Directory::join(utf8PluginsDir, "Dog.conf"),
        Utility::Directory::mapRead(Utility::Directory::join(pluginsDir, "Dog.conf")));

    PluginManager::Manager<AbstractAnimal> manager{utf8PluginsDir};
    /* One static plugin always present */
    CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"Canary", "Dog"}));
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);

    {
        std::unique_ptr<AbstractAnimal> animal = manager.instantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_VERIFY(animal->hasTail());
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);
    }

    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
}
#endif

void Test::debug() {
    std::ostringstream o;

    Debug(&o) << LoadState::Static << LoadState(0x3f);
    CORRADE_COMPARE(o.str(), "PluginManager::LoadState::Static PluginManager::LoadState(0x3f)\n");
}

}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::Test)
