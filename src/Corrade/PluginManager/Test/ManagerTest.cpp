/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/Containers/Optional.h"
#include "Corrade/PluginManager/Manager.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/Utility/DebugStl.h" /** @todo remove when <sstream> is gone */
#include "Corrade/Utility/Directory.h"
#include "Corrade/Utility/FormatStl.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/System.h"

#include "AbstractAnimal.h"
#include "AbstractCustomSuffix.h"
#include "AbstractDeletable.h"
#include "AbstractDisabledMetadata.h"
#include "AbstractFood.h"

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#include "Corrade/PluginManager/Test/wrong-metadata/WrongMetadata.h"
#include "configure.h"
#endif

static void importPlugin() {
    CORRADE_PLUGIN_IMPORT(Canary)
    CORRADE_PLUGIN_IMPORT(CustomSuffixStatic)
    CORRADE_PLUGIN_IMPORT(DisabledMetadataStatic)
}

namespace Corrade { namespace PluginManager { namespace Test { namespace {

struct ManagerTest: TestSuite::Tester {
    explicit ManagerTest();

    void pluginSearchPathsNotUsed();
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void pluginSearchPathsNotProvided();
    void pluginSearchPathsNotFound();
    #endif

    void nameList();

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void wrongMetadataFile();
    void missingMetadataFile();
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
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void dynamicPlugin();
    void dynamicPluginLoadAndInstantiate();
    void dynamicPluginFilePath();
    void dynamicPluginFilePathLoadAndInstantiate();
    void dynamicPluginFilePathConflictsWithLoadedPlugin();
    void dynamicPluginFilePathRemoveOnFail();
    #endif

    void configurationGlobal();
    void configurationLocal();
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void configurationImplicit();
    #endif
    void deletable();
    void hierarchy();
    void destructionHierarchy();
    void crossManagerDependencies();
    void crossManagerDependenciesWrongDestructionOrder();
    void unresolvedDependencies();

    void reloadPluginDirectory();
    void restoreAliasesAfterPluginDirectoryChange();

    void staticProvides();
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void dynamicProvides();
    void dynamicProvidesDependency();
    void setPreferredPlugins();
    void setPreferredPluginsUnknownAlias();
    void setPreferredPluginsDoesNotProvide();
    void setPreferredPluginsOverridePrimaryPlugin();
    #endif

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    void utf8Path();
    #endif

    void twoManagerInstances();

    void customSuffix();
    void disabledMetadata();

    void debugLoadState();
    void debugLoadStates();
};

ManagerTest::ManagerTest() {
    addTests({&ManagerTest::pluginSearchPathsNotUsed,
              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::pluginSearchPathsNotProvided,
              &ManagerTest::pluginSearchPathsNotFound,
              #endif

              &ManagerTest::nameList,

              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::wrongMetadataFile,
              &ManagerTest::missingMetadataFile,
              &ManagerTest::unresolvedReference,
              &ManagerTest::noPluginVersion,
              &ManagerTest::wrongPluginVersion,
              &ManagerTest::noPluginInterface,
              &ManagerTest::wrongPluginInterface,
              &ManagerTest::noPluginInitializer,
              &ManagerTest::noPluginFinalizer,
              &ManagerTest::noPluginInstancer,
              #endif

              &ManagerTest::queryNonexistent,
              &ManagerTest::loadNonexistent,
              &ManagerTest::unloadNonexistent,

              &ManagerTest::staticPlugin,
              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::dynamicPlugin,
              &ManagerTest::dynamicPluginLoadAndInstantiate,
              &ManagerTest::dynamicPluginFilePath,
              &ManagerTest::dynamicPluginFilePathLoadAndInstantiate,
              &ManagerTest::dynamicPluginFilePathConflictsWithLoadedPlugin,
              &ManagerTest::dynamicPluginFilePathRemoveOnFail,
              #endif

              &ManagerTest::configurationGlobal,
              &ManagerTest::configurationLocal,
              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::configurationImplicit,
              #endif
              &ManagerTest::deletable,
              &ManagerTest::hierarchy,
              &ManagerTest::destructionHierarchy,
              &ManagerTest::crossManagerDependencies,
              &ManagerTest::crossManagerDependenciesWrongDestructionOrder,
              &ManagerTest::unresolvedDependencies,

              &ManagerTest::reloadPluginDirectory,
              &ManagerTest::restoreAliasesAfterPluginDirectoryChange,

              &ManagerTest::staticProvides,
              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::dynamicProvides,
              &ManagerTest::dynamicProvidesDependency,
              &ManagerTest::setPreferredPlugins,
              &ManagerTest::setPreferredPluginsUnknownAlias,
              &ManagerTest::setPreferredPluginsDoesNotProvide,
              &ManagerTest::setPreferredPluginsOverridePrimaryPlugin,
              #endif

              #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
              &ManagerTest::utf8Path,
              #endif

              &ManagerTest::twoManagerInstances,

              &ManagerTest::customSuffix,
              &ManagerTest::disabledMetadata,

              &ManagerTest::debugLoadState,
              &ManagerTest::debugLoadStates});

    importPlugin();
}

void ManagerTest::pluginSearchPathsNotUsed() {
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

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
void ManagerTest::pluginSearchPathsNotProvided() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

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

void ManagerTest::pluginSearchPathsNotFound() {
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
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::Manager(): none of the plugin search paths in {nonexistent, /absolute/but/nonexistent} exists and pluginDirectory was not set, skipping plugin discovery\n");
}
#endif

void ManagerTest::nameList() {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    {
        PluginManager::Manager<AbstractAnimal> manager;

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "PitBull", "Snail"}), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
            "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
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

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Check that explicitly specifying the same plugin path does the same */
    {
        PluginManager::Manager<AbstractAnimal> manager{Utility::Directory::join(PLUGINS_DIR, "animals")};

        CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
            "Bulldog", "Canary", "Dog", "PitBull", "Snail"}), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
            "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
    }
    #endif
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
struct WrongPlugin: AbstractPlugin {
    static std::string pluginInterface() { return {}; }

    static std::vector<std::string> pluginSearchPaths() {
        return {Utility::Directory::join(PLUGINS_DIR, "wrong")};
    }
};

void ManagerTest::wrongMetadataFile() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongMetadata> manager;
    CORRADE_COMPARE(manager.loadState("WrongMetadata"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(manager.load("WrongMetadata"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(out.str(),
        "Utility::Configuration::Configuration(): missing equals for a value\n"
        "PluginManager::Manager::load(): plugin WrongMetadata is not ready to load: PluginManager::LoadState::WrongMetadataFile\n");
}

void ManagerTest::missingMetadataFile() {
    std::string dir = Utility::Directory::join(PLUGINS_DIR, "missing-metadata");
    CORRADE_VERIFY(Utility::Directory::mkpath(dir));
    CORRADE_VERIFY(Utility::Directory::writeString(Utility::Directory::join(dir, "MissingMetadata" + AbstractPlugin::pluginSuffix()), "this is not a binary"));

    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongMetadata> manager{dir};
    CORRADE_COMPARE(manager.loadState("MissingMetadata"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(manager.load("MissingMetadata"), LoadState::WrongMetadataFile);
    CORRADE_COMPARE(out.str(), Utility::formatString(
        "PluginManager::Manager: {} was not found\n"
        "PluginManager::Manager::load(): plugin MissingMetadata is not ready to load: PluginManager::LoadState::WrongMetadataFile\n",
        Utility::Directory::join(dir, "MissingMetadata.conf")));
}

void ManagerTest::unresolvedReference() {
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

void ManagerTest::noPluginVersion() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginVersion"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginVersion"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get version of plugin NoPluginVersion: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void ManagerTest::wrongPluginVersion() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<AbstractFood> foodManager;
    CORRADE_COMPARE(foodManager.load("OldBread"), PluginManager::LoadState::WrongPluginVersion);
    CORRADE_COMPARE(foodManager.loadState("OldBread"), PluginManager::LoadState::NotLoaded);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong version of plugin OldBread, expected 6 but got 0\n");
}

void ManagerTest::noPluginInterface() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInterface"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInterface"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix =  "PluginManager::Manager::load(): cannot get interface string of plugin NoPluginInterface: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void ManagerTest::wrongPluginInterface() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<AbstractFood> foodManager;
    CORRADE_COMPARE(foodManager.load("RottenTomato"), PluginManager::LoadState::WrongInterfaceVersion);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): wrong interface string of plugin RottenTomato, expected cz.mosra.corrade.PluginManager.Test.AbstractFood/1.0 but got cz.mosra.corrade.PluginManager.Test.AbstractFood/0.1\n");
}

void ManagerTest::noPluginInitializer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInitializer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInitializer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get initializer of plugin NoPluginInitializer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void ManagerTest::noPluginFinalizer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginFinalizer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginFinalizer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get finalizer of plugin NoPluginFinalizer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}

void ManagerTest::noPluginInstancer() {
    std::ostringstream out;
    Error redirectError{&out};

    PluginManager::Manager<WrongPlugin> manager;
    CORRADE_COMPARE(manager.load("NoPluginInstancer"), PluginManager::LoadState::LoadFailed);
    CORRADE_COMPARE(manager.loadState("NoPluginInstancer"), PluginManager::LoadState::NotLoaded);
    const std::string expectedPrefix = "PluginManager::Manager::load(): cannot get instancer of plugin NoPluginInstancer: ";
    CORRADE_COMPARE(out.str().substr(0, expectedPrefix.size()), expectedPrefix);
}
#endif

void ManagerTest::queryNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;
    const PluginManager::Manager<AbstractAnimal>& cmanager = manager;
    CORRADE_VERIFY(!manager.metadata("Nonexistent"));
    CORRADE_VERIFY(!cmanager.metadata("Nonexistent"));
    CORRADE_COMPARE(cmanager.loadState("Nonexistent"), LoadState::NotFound);
}

void ManagerTest::loadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(manager.load("Nonexistent"), LoadState::NotFound);
    #if defined(CORRADE_TARGET_EMSCRIPTEN) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_ANDROID)
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): plugin Nonexistent was not found\n");
    #else
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::load(): plugin Nonexistent is not static and was not found in " PLUGINS_DIR "/animals\n");
    #endif
}

void ManagerTest::unloadNonexistent() {
    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(manager.unload("Nonexistent"), LoadState::NotFound);
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::unload(): plugin Nonexistent was not found\n");
}

void ManagerTest::staticPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);
    CORRADE_COMPARE(manager.metadata("Canary")->data().value("description"), "I'm allergic to canaries!");

    Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Canary");
    CORRADE_VERIFY(animal);
    CORRADE_VERIFY(animal->hasTail());
    CORRADE_COMPARE(animal->name(), "Achoo");
    CORRADE_COMPARE(animal->legCount(), 2);

    CORRADE_COMPARE(manager.unload("Canary"), LoadState::Static);
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
void ManagerTest::dynamicPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_COMPARE(manager.metadata("Dog")->data().value("description"), "A simple dog plugin.");

    {
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Dog");
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

void ManagerTest::dynamicPluginLoadAndInstantiate() {
    PluginManager::Manager<AbstractAnimal> manager;
    Containers::Pointer<AbstractAnimal> animal = manager.loadAndInstantiate("Dog");
    CORRADE_VERIFY(animal);
    CORRADE_COMPARE(animal->name(), "Doug");
}

void ManagerTest::dynamicPluginFilePath() {
    PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};

    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotFound);
    CORRADE_COMPARE(manager.load(DOG_PLUGIN_FILENAME), LoadState::Loaded);
    CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"Canary", "Dog"}));
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);

    Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Dog");
    CORRADE_VERIFY(animal);
    CORRADE_COMPARE(animal->name(), "Doug");
    CORRADE_COMPARE(animal->metadata()->data().value("description"), "A simple dog plugin.");
}

void ManagerTest::dynamicPluginFilePathLoadAndInstantiate() {
    PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};
    Containers::Pointer<AbstractAnimal> animal = manager.loadAndInstantiate(DOG_PLUGIN_FILENAME);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    CORRADE_VERIFY(animal);
    CORRADE_COMPARE(animal->name(), "Doug");
    CORRADE_COMPARE(animal->metadata()->data().value("description"), "A simple dog plugin.");
}

void ManagerTest::dynamicPluginFilePathConflictsWithLoadedPlugin() {
    PluginManager::Manager<AbstractAnimal> manager; /* Use the path that has Dog plugin */

    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);

    /* Fails when Dog is loaded */
    {
        std::ostringstream out;
        Error redirectError{&out};
        CORRADE_COMPARE(manager.load(DOG_PLUGIN_FILENAME), LoadState::Used);
        CORRADE_COMPARE(out.str(), "PluginManager::load(): Dog"  + AbstractPlugin::pluginSuffix() + " conflicts with currently loaded plugin of the same name\n");
    }

    /* AGoodBoy is provided by (the currently loaded) Dog plugin */
    CORRADE_COMPARE(manager.metadata("AGoodBoy")->name(), "Dog");
    CORRADE_COMPARE(manager.loadState("AGoodBoy"), LoadState::Loaded);
    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.loadState("AGoodBoy"), LoadState::NotLoaded);

    /* Succeeds when it's unloaded */
    CORRADE_COMPARE(manager.load(DOG_PLUGIN_FILENAME), LoadState::Loaded);
    {
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->metadata()->data().value("description"), "A simple dog plugin.");
    }

    /* AGoodBoy is loaded again, different plugin. Test that it
       instantiates properly. */
    CORRADE_COMPARE(manager.loadState("AGoodBoy"), LoadState::Loaded);
    {
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("AGoodBoy");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->metadata()->data().value("description"), "A simple dog plugin.");
    }
}

void ManagerTest::dynamicPluginFilePathRemoveOnFail() {
    PluginManager::Manager<AbstractAnimal> manager{"nonexistent"};

    /* Sure, PitBull needs a Dog */
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotFound);
    CORRADE_COMPARE(manager.load(PITBULL_PLUGIN_FILENAME), LoadState::UnresolvedDependency);

    /* No internal statie is modified, even through PitBull provides a Dog */
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotFound);

    /* Now load the Dog and test it */
    CORRADE_COMPARE(manager.load(DOG_PLUGIN_FILENAME), LoadState::Loaded);
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::Loaded);
    {
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Doug");
    }

    /* Now it's available and we can finally load PitBull */
    CORRADE_COMPARE(manager.load(PITBULL_PLUGIN_FILENAME), LoadState::Loaded);
}

#endif

void ManagerTest::configurationGlobal() {
    {
        PluginManager::Manager<AbstractAnimal> manager;

        CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);

        /* Change the global config, the instance then gets a copy */
        PluginMetadata& metadata = *manager.metadata("Canary");
        metadata.configuration().setValue("name", "BIRD UP!!");

        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Canary");
        CORRADE_COMPARE(animal->name(), "BIRD UP!!");
        CORRADE_COMPARE(animal->configuration().value("name"), "BIRD UP!!");
    } {
        /* When constructing the manager next time, the configuration should go
           back to its initial state */
        PluginManager::Manager<AbstractAnimal> manager;
        CORRADE_COMPARE(manager.metadata("Canary")->configuration().value("name"), "Achoo");
    }
}

void ManagerTest::configurationLocal() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.loadState("Canary"), LoadState::Static);

    /* Verify everything is accessible through const& */
    const PluginMetadata& metadata = *const_cast<const PluginManager::Manager<AbstractAnimal>&>(manager).metadata("Canary");
    CORRADE_COMPARE(metadata.configuration().value("name"), "Achoo");

    Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Canary");
    CORRADE_COMPARE(animal->name(), "Achoo");
    CORRADE_COMPARE(animal->configuration().value("name"), "Achoo");

    /* Local config is also mutable */
    animal->configuration().setValue("name", "Bird!!");
    CORRADE_COMPARE(animal->name(), "Bird!!");

    /* Global config and other instances are not affected by it */
    CORRADE_COMPARE(metadata.configuration().value("name"), "Achoo");
    CORRADE_COMPARE(manager.instantiate("Canary")->name(), "Achoo");
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
void ManagerTest::configurationImplicit() {
    PluginManager::Manager<AbstractAnimal> manager;

    Containers::Pointer<AbstractAnimal> animal = manager.loadAndInstantiate("Dog");
    CORRADE_VERIFY(animal);

    /* The plugin should get an implicitly created configuration */
    CORRADE_COMPARE(manager.metadata("Dog")->configuration().valueCount(), 0);
    CORRADE_COMPARE(animal->configuration().valueCount(), 0);

    /* And a modifiable one */
    animal->configuration().setValue("name", "UPDOG");
    CORRADE_COMPARE(animal->configuration().value("name"), "UPDOG");
}
#endif

void ManagerTest::deletable() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
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

void ManagerTest::hierarchy() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
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
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("PitBull");
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

void ManagerTest::destructionHierarchy() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
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

void ManagerTest::crossManagerDependencies() {
    PluginManager::Manager<AbstractAnimal> manager;
    PluginManager::Manager<AbstractFood> foodManager;
    foodManager.registerExternalManager(manager);

    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
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
        Containers::Pointer<AbstractFood> hotdog = foodManager.instantiate("HotDog");
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

    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't fully test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!foodManager.instantiate("Canary"));
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::instantiate(): plugin Canary is not loaded\n");
}

void ManagerTest::crossManagerDependenciesWrongDestructionOrder() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_SKIP("Cross-manager dependencies are meaningful only for dynamic plugins");
    #endif
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't fully test assertions");
    #endif

    Containers::Optional<PluginManager::Manager<AbstractAnimal>> manager{Containers::InPlaceInit};
    PluginManager::Manager<AbstractFood> foodManager;
    foodManager.registerExternalManager(*manager);

    std::ostringstream out;
    Error redirectError{&out};
    manager = Containers::NullOpt;
    CORRADE_COMPARE(out.str(), "PluginManager::Manager: wrong destruction order, cz.mosra.corrade.PluginManager.Test.AbstractAnimal/1.0 plugins still needed by 1 other managers for external dependencies\n");
}

void ManagerTest::unresolvedDependencies() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_SKIP("UsedBy list is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;
    PluginManager::Manager<AbstractFood> foodManager;
    foodManager.registerExternalManager(manager);

    /* HotDogWithSnail depends on Dog and Snail, which cannot be loaded, so the
       loading fails too. Dog plugin then shouldn't have HotDogWithSnail in
       usedBy list. */

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_COMPARE(foodManager.load("HotDogWithSnail"), LoadState::UnresolvedDependency);
    CORRADE_COMPARE(out.str(),
        "PluginManager::Manager::load(): unresolved dependency SomethingThatDoesNotExist of plugin Snail\n"
        "PluginManager::Manager::load(): unresolved dependency Snail of plugin HotDogWithSnail\n");
    CORRADE_COMPARE(foodManager.loadState("HotDogWithSnail"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.metadata("Dog")->usedBy(),
        std::vector<std::string>{});
    #endif
}

void ManagerTest::reloadPluginDirectory() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_SKIP("Plugin directory is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;

    /* Load PitBull and rename the plugin */
    CORRADE_COMPARE(manager.load("PitBull"), LoadState::Loaded);
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "PitBull"  + AbstractPlugin::pluginSuffix()}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostPitBull" + AbstractPlugin::pluginSuffix()}));
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "PitBull.conf"}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostPitBull.conf"}));

    /* Rename Snail */
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "Snail" + AbstractPlugin::pluginSuffix()}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostSnail" + AbstractPlugin::pluginSuffix()}));
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "Snail.conf"}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostSnail.conf"}));

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
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostPitBull" + AbstractPlugin::pluginSuffix()}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "PitBull" + AbstractPlugin::pluginSuffix()}));
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostPitBull.conf"}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "PitBull.conf"}));

    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostSnail" + AbstractPlugin::pluginSuffix()}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "Snail" + AbstractPlugin::pluginSuffix()}));
    Utility::Directory::move(
        Utility::Directory::join({PLUGINS_DIR, "animals", "LostSnail.conf"}),
        Utility::Directory::join({PLUGINS_DIR, "animals", "Snail.conf"}));

    manager.reloadPluginDirectory();

    /* And now we can safely compare */
    CORRADE_COMPARE_AS(actualPlugins1, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "LostPitBull", "LostSnail", "PitBull"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualAliases1, (std::vector<std::string>{
        "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "LostPitBull", "LostSnail", "PitBull"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualPlugins2, (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "LostPitBull", "LostSnail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(actualAliases2, (std::vector<std::string>{
        "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "LostPitBull", "LostSnail"}), TestSuite::Compare::Container);
    #endif
}

void ManagerTest::restoreAliasesAfterPluginDirectoryChange() {
    #ifdef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_SKIP("Plugin directory is irrelevant for static plugins");
    #else
    PluginManager::Manager<AbstractAnimal> manager;
    CORRADE_COMPARE(manager.load(DOGGO_PLUGIN_FILENAME), LoadState::Loaded);

    CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
        "Bulldog", "Canary", "Dog", "Doggo", "PitBull", "Snail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
        "AGoodBoy", "Bulldog", "Canary", "Dog", "Doggo", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);

    /* Set plugin directory to an empty idr -- there should stay the Doggo
       plugin and its Dog alias */
    manager.setPluginDirectory("nonexistent");
    CORRADE_COMPARE_AS(manager.pluginList(), (std::vector<std::string>{
        "Canary", "Doggo"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(manager.aliasList(), (std::vector<std::string>{
        "Canary", "Dog", "Doggo", "JustSomeBird"}), TestSuite::Compare::Container);
    #endif
}

void ManagerTest::staticProvides() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_COMPARE(manager.metadata("Canary")->provides(), std::vector<std::string>{"JustSomeBird"});

    CORRADE_COMPARE(manager.loadState("JustSomeBird"), LoadState::Static);
    CORRADE_VERIFY(manager.metadata("JustSomeBird"));
    CORRADE_COMPARE(manager.metadata("JustSomeBird")->name(), "Canary");

    const auto animal = manager.instantiate("JustSomeBird");
    CORRADE_COMPARE(animal->plugin(), "JustSomeBird");
    CORRADE_COMPARE(animal->metadata()->name(), "Canary");
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
void ManagerTest::dynamicProvides() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_VERIFY(manager.metadata("Dog"));
    CORRADE_COMPARE(manager.metadata("Dog")->provides(), (std::vector<std::string>{"JustSomeMammal", "AGoodBoy"}));

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

void ManagerTest::dynamicProvidesDependency() {
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

void ManagerTest::setPreferredPlugins() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_VERIFY(manager.metadata("Dog"));
    CORRADE_COMPARE(manager.metadata("Dog")->provides(), (std::vector<std::string>{"JustSomeMammal", "AGoodBoy"}));
    CORRADE_VERIFY(manager.metadata("PitBull"));
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

void ManagerTest::setPreferredPluginsUnknownAlias() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    manager.setPreferredPlugins("Chihuahua", {"PitBull"});
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::setPreferredPlugins(): Chihuahua is not a known alias\n");
}

void ManagerTest::setPreferredPluginsDoesNotProvide() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    PluginManager::Manager<AbstractAnimal> manager;

    std::ostringstream out;
    Error redirectError{&out};
    manager.setPreferredPlugins("Dog", {"Snail"});
    CORRADE_COMPARE(out.str(), "PluginManager::Manager::setPreferredPlugins(): Snail does not provide Dog\n");
}

void ManagerTest::setPreferredPluginsOverridePrimaryPlugin() {
    PluginManager::Manager<AbstractAnimal> manager;

    CORRADE_VERIFY(manager.metadata("PitBull"));
    CORRADE_COMPARE(manager.metadata("PitBull")->provides(), (std::vector<std::string>{"JustSomeMammal", "Dog"}));

    /* Implicit state */
    CORRADE_VERIFY(manager.metadata("Dog"));
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "Dog");

    /* Override */
    manager.setPreferredPlugins("Dog", {"PitBull"});
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "PitBull");

    /* Reloading plugin directory resets the mapping back */
    manager.reloadPluginDirectory();
    CORRADE_COMPARE(manager.metadata("Dog")->name(), "Dog");
}

void ManagerTest::utf8Path() {
    #if defined(__has_feature)
    #if __has_feature(address_sanitizer)
    CORRADE_SKIP("Because the same shared object is loaded from two different paths, its globals (the vtable) are loaded twice. Skipping to avoid AddressSanitizer complain about ODR violation.");
    #endif
    #endif

    /* Copy the dog plugin to a new UTF-8 path */
    const std::string utf8PluginsDir = Utility::Directory::join(PLUGINS_DIR, "hýždě");
    CORRADE_VERIFY(Utility::Directory::mkpath(utf8PluginsDir));
    Utility::Directory::write(
        Utility::Directory::join(utf8PluginsDir, "Dog" + AbstractPlugin::pluginSuffix()),
        Utility::Directory::mapRead(Utility::Directory::join({PLUGINS_DIR, "animals", "Dog" + AbstractPlugin::pluginSuffix()})));
    Utility::Directory::write(
        Utility::Directory::join(utf8PluginsDir, "Dog.conf"),
        Utility::Directory::mapRead(Utility::Directory::join({PLUGINS_DIR, "animals", "Dog.conf"})));

    PluginManager::Manager<AbstractAnimal> manager{utf8PluginsDir};
    /* One static plugin always present */
    CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"Canary", "Dog"}));
    CORRADE_COMPARE(manager.loadState("Dog"), LoadState::NotLoaded);
    CORRADE_COMPARE(manager.load("Dog"), LoadState::Loaded);

    {
        Containers::Pointer<AbstractAnimal> animal = manager.instantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_VERIFY(animal->hasTail());
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);
    }

    CORRADE_COMPARE(manager.unload("Dog"), LoadState::NotLoaded);
}
#endif

void ManagerTest::twoManagerInstances() {
    PluginManager::Manager<AbstractAnimal> a;
    PluginManager::Manager<AbstractAnimal> b;

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    CORRADE_COMPARE_AS(a.aliasList(), (std::vector<std::string>{
        "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(b.aliasList(), (std::vector<std::string>{
        "AGoodBoy", "Bulldog", "Canary", "Dog", "JustSomeBird", "JustSomeMammal", "PitBull", "Snail"}), TestSuite::Compare::Container);
    #else
    CORRADE_COMPARE_AS(a.aliasList(), (std::vector<std::string>{
        "Canary", "JustSomeBird"}), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(b.aliasList(), (std::vector<std::string>{
        "Canary", "JustSomeBird"}), TestSuite::Compare::Container);
    #endif

    /* Verify that loading a dynamic plugin works the same in both */
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    {
        Containers::Pointer<AbstractAnimal> animal = a.loadAndInstantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);
    } {
        Containers::Pointer<AbstractAnimal> animal = b.loadAndInstantiate("Dog");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Doug");
        CORRADE_COMPARE(animal->legCount(), 4);
    }
    #endif

    /* Verify that loading a static plugin works also */
    {
        Containers::Pointer<AbstractAnimal> animal = a.loadAndInstantiate("Canary");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Achoo");
        CORRADE_COMPARE(animal->legCount(), 2);
    } {
        Containers::Pointer<AbstractAnimal> animal = b.loadAndInstantiate("Canary");
        CORRADE_VERIFY(animal);
        CORRADE_COMPARE(animal->name(), "Achoo");
        CORRADE_COMPARE(animal->legCount(), 2);
    }
}

void ManagerTest::customSuffix() {
    {
        PluginManager::Manager<AbstractCustomSuffix> manager;

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"CustomSuffix", "CustomSuffixStatic"}));
        #else
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"CustomSuffixStatic"}));
        #endif

        CORRADE_COMPARE(manager.load("CustomSuffixStatic"), LoadState::Static);
        Containers::Pointer<AbstractCustomSuffix> pluginStatic = manager.instantiate("CustomSuffixStatic");
        CORRADE_VERIFY(pluginStatic);
        CORRADE_COMPARE(pluginStatic->greet(), "Hiya but static!");

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        CORRADE_COMPARE(manager.load("CustomSuffix"), LoadState::Loaded);
        Containers::Pointer<AbstractCustomSuffix> plugin = manager.instantiate("CustomSuffix");
        CORRADE_VERIFY(plugin);
        CORRADE_COMPARE(plugin->greet(), "Hiya!");
        #endif
    }

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Loading by file path accesses the metadata files through a different
       code path, verify that also */
    {
        PluginManager::Manager<AbstractCustomSuffix> manager{"nonexistent"};
        CORRADE_COMPARE(manager.load(Utility::Directory::join({PLUGINS_DIR, "custom-suffix", "CustomSuffix" + AbstractCustomSuffix::pluginSuffix()})), LoadState::Loaded);
        Containers::Pointer<AbstractCustomSuffix> plugin = manager.instantiate("CustomSuffix");
        CORRADE_VERIFY(plugin);
        CORRADE_COMPARE(plugin->greet(), "Hiya!");
    }
    #endif
}

void ManagerTest::disabledMetadata() {
    {
        PluginManager::Manager<AbstractDisabledMetadata> manager;

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"DisabledMetadata", "DisabledMetadataStatic"}));
        #else
        CORRADE_COMPARE(manager.pluginList(), (std::vector<std::string>{"DisabledMetadataStatic"}));
        #endif

        CORRADE_COMPARE(manager.load("DisabledMetadataStatic"), LoadState::Static);
        Containers::Pointer<AbstractDisabledMetadata> pluginStatic = manager.instantiate("DisabledMetadataStatic");
        CORRADE_VERIFY(pluginStatic);
        CORRADE_COMPARE(pluginStatic->greet(), "Olaa but static!");

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        CORRADE_COMPARE(manager.load("DisabledMetadata"), LoadState::Loaded);
        Containers::Pointer<AbstractDisabledMetadata> plugin = manager.instantiate("DisabledMetadata");
        CORRADE_VERIFY(plugin);
        CORRADE_COMPARE(plugin->greet(), "Olaa!");
        #endif
    }

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Loading by file path accesses the metadata files through a different
       code path, verify that also */
    {
        PluginManager::Manager<AbstractDisabledMetadata> manager{"nonexistent"};
        CORRADE_COMPARE(manager.load(Utility::Directory::join({PLUGINS_DIR, "disabled-metadata", "DisabledMetadata" + AbstractDisabledMetadata::pluginSuffix()})), LoadState::Loaded);
        Containers::Pointer<AbstractDisabledMetadata> plugin = manager.instantiate("DisabledMetadata");
        CORRADE_VERIFY(plugin);
        CORRADE_COMPARE(plugin->greet(), "Olaa!");
    }
    #endif
}

void ManagerTest::debugLoadState() {
    std::ostringstream o;

    Debug(&o) << LoadState::Static << LoadState(0x3f);
    CORRADE_COMPARE(o.str(), "PluginManager::LoadState::Static PluginManager::LoadState(0x3f)\n");
}

void ManagerTest::debugLoadStates() {
    std::ostringstream out;

    Debug{&out} << (LoadState::Static|LoadState::NotFound) << LoadStates{};
    CORRADE_COMPARE(out.str(), "PluginManager::LoadState::NotFound|PluginManager::LoadState::Static PluginManager::LoadStates{}\n");
}

}}}}

CORRADE_TEST_MAIN(Corrade::PluginManager::Test::ManagerTest)
