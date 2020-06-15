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

#include "AbstractManager.h"

#include <cstring>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <utility>

#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/Implementation/RawForwardList.h"
#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Configuration.h"
#include "Corrade/Utility/Resource.h"
#include "Corrade/Utility/String.h"

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
#include "Corrade/Utility/Directory.h"

#ifndef CORRADE_TARGET_WINDOWS
#include <dlfcn.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN /* Otherwise `#define interface struct` breaks everything */
#endif
#include <windows.h>
#include "Corrade/Utility/Unicode.h"
using Corrade::Utility::Unicode::widen;
#endif
#endif

#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Implementation/WindowsWeakSymbol.h"
#endif
#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
#include "Corrade/Utility/Implementation/WindowsError.h"
#endif

using namespace Corrade::Utility;

namespace Corrade { namespace PluginManager {

struct AbstractManager::Plugin {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    LoadState loadState;
    #else
    const LoadState loadState; /* Always LoadState::Static */
    #endif
    Utility::Configuration configuration;
    PluginMetadata metadata;

    Instancer instancer;
    void(*finalizer)();

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    union {
        /* For static plugins */
        const Implementation::StaticPlugin* staticPlugin;

        /* For dynamic plugins */
        #ifndef CORRADE_TARGET_WINDOWS
        void* module;
        #else
        HMODULE module;
        #endif
    };
    #else
    const Implementation::StaticPlugin* staticPlugin;
    #endif

    std::vector<AbstractPlugin*> instances;

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Constructor for dynamic plugins */
    explicit Plugin(std::string name, const std::string& metadata);
    #endif

    /* Constructor for static plugins */
    explicit Plugin(const Implementation::StaticPlugin& staticPlugin, Utility::Configuration&& configuration_);

    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;

    ~Plugin() = default;
};

struct AbstractManager::State {
    explicit State(std::string&& pluginInterface,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        std::string&& pluginSuffix,
        #endif
        std::string&& pluginMetadataSuffix):
        pluginInterface{std::move(pluginInterface)},
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        pluginSuffix{std::move(pluginSuffix)},
        #endif
        pluginMetadataSuffix{std::move(pluginMetadataSuffix)}
    {}

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    std::string pluginDirectory;
    #endif
    std::string pluginInterface;
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    std::string pluginSuffix;
    #endif
    std::string pluginMetadataSuffix;

    std::map<std::string, Containers::Pointer<Plugin>> plugins;
    std::map<std::string, Plugin&> aliases;

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    std::set<AbstractManager*> externalManagers;
    #ifndef CORRADE_NO_ASSERT
    std::set<AbstractManager*> externalManagerUsedBy;
    #endif
    #endif
};

const int AbstractManager::Version = CORRADE_PLUGIN_VERSION;

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
/* (Of course) can't be in an unnamed namespace in order to export it below
   (except for Windows, where we do extern "C" so this doesn't matter) */
namespace {
#endif

/* What the hell is going on here with the #ifdefs?! */
#if !defined(CORRADE_BUILD_STATIC) || !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || (defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS)) || defined(CORRADE_TARGET_WINDOWS_RT)
#ifdef CORRADE_BUILD_STATIC_UNIQUE_GLOBALS
/* On static builds that get linked to multiple shared libraries and then used
   in a single app we want to ensure there's just one global symbol. On Linux
   it's apparently enough to just export, macOS needs the weak attribute. */
CORRADE_VISIBILITY_EXPORT
    #ifdef __GNUC__
    __attribute__((weak))
    #else
    /* uh oh? the test will fail, probably */
    #endif
#endif
/* A linked list of static plugins. Managed using utilities from
   Containers/Implementation/RawForwardList.h, look there for more info.

   The value of this variable is guaranteed to be zero-filled even before any
   static plugin initializers are executed, which means we don't hit any static
   initialization order fiasco. */
Implementation::StaticPlugin* globalStaticPlugins = nullptr;
#else
/* On Windows the symbol is exported unmangled and then fetched via
   GetProcAddress() to emulate weak linking. Using an extern "C" block instead
   of just a function annotation because otherwise MinGW prints a warning:
   '...' initialized and declared 'extern' (uh?) */
extern "C" {
    CORRADE_VISIBILITY_EXPORT Implementation::StaticPlugin* corradePluginManagerUniqueGlobalStaticPlugins = nullptr;
}
#endif

#if !defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) || defined(CORRADE_TARGET_WINDOWS)
}
#endif

/* Windows don't have any concept of weak symbols, instead GetProcAddress() on
   GetModuleHandle(nullptr) "emulates" the weak linking as it's guaranteed to
   pick up the same symbol of the final exe independently of the DLL it was
   called from. To avoid #ifdef hell in code below, the globalStaticPlugins are
   redefined to return a value from this uniqueness-ensuring function. */
#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC_UNIQUE_GLOBALS) && !defined(CORRADE_TARGET_WINDOWS_RT)
namespace {

Implementation::StaticPlugin*& windowsGlobalStaticPlugins() {
    /* A function-local static to ensure it's only initialized once without any
       race conditions among threads */
    static Implementation::StaticPlugin** const uniqueGlobals = reinterpret_cast<Implementation::StaticPlugin**>(Utility::Implementation::windowsWeakSymbol("corradePluginManagerUniqueGlobalStaticPlugins", &corradePluginManagerUniqueGlobalStaticPlugins));
    return *uniqueGlobals;
}

}

#define globalStaticPlugins windowsGlobalStaticPlugins()
#endif

static_assert(std::is_standard_layout<Implementation::StaticPlugin>::value && std::is_trivial<Implementation::StaticPlugin>::value,
    "static plugins shouldn't cause any global initialization / finalization to happen on their own");

void AbstractManager::importStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Debug::nospace << ", got" << version << "but expected" << Version, );
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(version);
    #endif
    Containers::Implementation::forwardListInsert(globalStaticPlugins, plugin);
}

void AbstractManager::ejectStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Debug::nospace << ", got" << version << "but expected" << Version, );
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(version);
    #endif
    Containers::Implementation::forwardListRemove(globalStaticPlugins, plugin);
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
AbstractManager::AbstractManager(std::string pluginInterface, const std::vector<std::string>& pluginSearchPaths, std::string pluginSuffix, std::string pluginMetadataSuffix, std::string pluginDirectory):
#else
AbstractManager::AbstractManager(std::string pluginInterface, std::string pluginMetadataSuffix):
#endif
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    _state{Containers::InPlaceInit, std::move(pluginInterface), std::move(pluginSuffix), std::move(pluginMetadataSuffix)}
    #else
    _state{Containers::InPlaceInit, std::move(pluginInterface), std::move(pluginMetadataSuffix)}
    #endif
{
    /* Add static plugins which have the same interface and don't have a
       manager assigned to them (i.e, aren't in the map yet). */
    for(const Implementation::StaticPlugin* staticPlugin = globalStaticPlugins; staticPlugin; staticPlugin = Containers::Implementation::forwardListNext(*staticPlugin)) {
        /* The plugin doesn't belong to this manager, skip it */
        if(staticPlugin->interface != _state->pluginInterface) continue;

        /* Assign the plugin to this manager, parse its metadata and
           initialize it (unless the plugin is metadata-less) */
        Utility::Configuration configuration;
        if(!_state->pluginMetadataSuffix.empty()) {
            Resource rs{std::string{"CorradeStaticPlugin_"} + staticPlugin->plugin};
            std::istringstream metadata(rs.get(staticPlugin->plugin + _state->pluginMetadataSuffix));
            configuration = Utility::Configuration{metadata, Utility::Configuration::Flag::ReadOnly};
        }

        /* Insert the plugin into our list. The names should be globally
           unique, so the insertion is expected to always succeed (if it
           wouldn't, we would have a leak here). */
        const auto inserted = _state->plugins.emplace(staticPlugin->plugin, Containers::pointer(new Plugin{*staticPlugin, std::move(configuration)}));
        CORRADE_INTERNAL_ASSERT(inserted.second);
        Plugin& p = *inserted.first->second;

        p.staticPlugin->initializer();

        /* The plugin is the best version of itself. If there was already
           an alias for this name, replace it. */
        {
            const auto alias = _state->aliases.find(inserted.first->first);
            if(alias != _state->aliases.end()) _state->aliases.erase(alias);
            CORRADE_INTERNAL_ASSERT_OUTPUT(_state->aliases.insert({inserted.first->first, p}).second);
        }

        /* Add aliases to the list (only the ones that aren't already there
           are added) */
        for(const std::string& alias: p.metadata._provides) {
            /* Libc++ frees the passed Plugin& reference when using
               emplace(), causing double-free memory corruption later.
               Everything is okay with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            _state->aliases.insert({alias, p});
        }
    }

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* If plugin directory is set, use it, otherwise loop through */
    if(!pluginDirectory.empty()) setPluginDirectory(std::move(pluginDirectory));
    else {
        CORRADE_ASSERT(!pluginSearchPaths.empty(),
            "PluginManager::Manager::Manager(): either pluginDirectory has to be set or T::pluginSearchPaths() is expected to have at least one entry", );

        const std::string executableDir = Utility::Directory::path(Utility::Directory::executableLocation());
        for(const std::string& path: pluginSearchPaths) {
            std::string fullPath = Utility::Directory::join(executableDir, path);
            if(!Utility::Directory::exists(fullPath)) continue;

            setPluginDirectory(std::move(fullPath));
            break;
        }

        /* If no hardcoded path exists and plugin directory is "", disable
           plugin discovery as searching in the current directory would almost
           never be what the user wants -- e.g., it would treat
           CorradeUtility.dll as a plugin. Don't print the warning in case
           we have static plugins (the aliases are non-empty) -- in that case
           assume the user might want to only use static plugins. */
        if(_state->pluginDirectory.empty() && _state->aliases.empty())
            Warning{} << "PluginManager::Manager::Manager(): none of the plugin search paths in" << pluginSearchPaths << "exists and pluginDirectory was not set, skipping plugin discovery";
    }
    #endif
}

AbstractManager::~AbstractManager() {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    #ifndef CORRADE_NO_ASSERT
    /* Check that this instance is not used for external dependencies anywhere */
    if(!_state->externalManagerUsedBy.empty()) {
        #ifdef CORRADE_GRACEFUL_ASSERT
        for(AbstractManager* manager: _state->externalManagerUsedBy)
            CORRADE_INTERNAL_ASSERT_OUTPUT(manager->_state->externalManagers.erase(this) == 1);
        #endif

        CORRADE_ASSERT(false,
            "PluginManager::Manager: wrong destruction order," << _state->pluginInterface << "plugins still needed by" << _state->externalManagerUsedBy.size() << "other managers for external dependencies", );
    }

    /* Let the external managers know that we're not using them anymore. This
       is done only if asserts are enabled, otherwise the backreference isn't
       used in any way. */
    for(AbstractManager* manager: _state->externalManagers)
        CORRADE_INTERNAL_ASSERT_OUTPUT(manager->_state->externalManagerUsedBy.erase(this) == 1);
    #endif
    #endif

    /* Unload all plugins */
    for(std::pair<const std::string, Containers::Pointer<Plugin>>& plugin: _state->plugins) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /* Try to unload the plugin (and all plugins that depend on it) */
        unloadRecursiveInternal(*plugin.second);
        #endif

        /* Finalize static plugins before they get removed from the list */
        if(plugin.second->loadState == LoadState::Static)
            plugin.second->staticPlugin->finalizer();
    }
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadRecursiveInternal(Plugin& plugin) {
    /* If the plugin is not static and is used by others, try to unload these
       first so it can be unloaded too. This function is called from the
       destructor after we verified there are no other managers using this for
       external dependencies, so everything the usedBy list is in this manager
       as well */
    while(!plugin.metadata._usedBy.empty()) {
        const auto found = _state->plugins.find(plugin.metadata._usedBy.front());
        CORRADE_INTERNAL_ASSERT(found != _state->plugins.end());
        return unloadRecursiveInternal(*found->second);
    }

    /* Unload the plugin */
    const LoadState after = unloadInternal(plugin);
    CORRADE_ASSERT(after & (LoadState::Static|LoadState::NotLoaded|LoadState::WrongMetadataFile),
        "PluginManager::Manager: cannot unload plugin" << plugin.metadata._name << "on manager destruction:" << after, {});

    return after;
}

std::string AbstractManager::pluginInterface() const {
    return _state->pluginInterface;
}

std::string AbstractManager::pluginDirectory() const {
    return _state->pluginDirectory;
}

void AbstractManager::setPluginDirectory(std::string directory) {
    _state->pluginDirectory = std::move(directory);

    /* Remove aliases for unloaded plugins from the container. They need to be
       removed before plugins themselves */
    auto ait = _state->aliases.cbegin();
    while(ait != _state->aliases.cend()) {
        if(ait->second.loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile))
            ait = _state->aliases.erase(ait);
        else ++ait;
    }

    /* Remove all unloaded plugins from the container */
    auto it = _state->plugins.cbegin();
    while(it != _state->plugins.cend()) {
        if(it->second->loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile))
            it = _state->plugins.erase(it);
        else ++it;
    }

    /* Find plugin files in the directory. Sort the list so we have predictable
       plugin preference behavior for aliases on systems that have random
       directory listing order. */
    const std::vector<std::string> d = Directory::list(_state->pluginDirectory,
        Directory::Flag::SkipDirectories|Directory::Flag::SkipDotAndDotDot|
        Directory::Flag::SortAscending);
    for(const std::string& filename: d) {
        /* File doesn't have module suffix, continue to next */
        if(!Utility::String::endsWith(filename, _state->pluginSuffix))
            continue;

        /* Dig plugin name from filename */
        const std::string name = filename.substr(0, filename.length() - _state->pluginSuffix.size());

        /* Skip the plugin if it is among loaded */
        if(_state->plugins.find(name) != _state->plugins.end()) continue;

        registerDynamicPlugin(name, Containers::pointer(new Plugin{name,
            _state->pluginMetadataSuffix.empty() ? std::string{} :
                Directory::join(_state->pluginDirectory, name + _state->pluginMetadataSuffix)}));
    }

    /* If some of the currently loaded plugins aliased plugins that were in the
       old plugin directory, these are no longer there. Refresh the alias list
       with the new plugins. */
    for(std::pair<const std::string, Containers::Pointer<Plugin>>& p: _state->plugins) {
        /* Add aliases to the list (only the ones that aren't already there are
           added, calling insert() won't overwrite the existing value) */
        for(const std::string& alias: p.second->metadata._provides) {
            /* Libc++ frees the passed Plugin& reference when using emplace(),
               causing double-free memory corruption later. Everything is okay
               with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            _state->aliases.insert({alias, *p.second});
        }
    }
}

void AbstractManager::reloadPluginDirectory() {
    setPluginDirectory(pluginDirectory());
}
#endif

void AbstractManager::setPreferredPlugins(const std::string& alias, const std::initializer_list<std::string> plugins) {
    auto foundAlias = _state->aliases.find(alias);
    CORRADE_ASSERT(foundAlias != _state->aliases.end(),
        "PluginManager::Manager::setPreferredPlugins():" << alias << "is not a known alias", );

    /* Replace the alias with the first candidate that exists */
    for(const std::string& plugin: plugins) {
        auto foundPlugin = _state->plugins.find(plugin);
        if(foundPlugin == _state->plugins.end())
            continue;

        CORRADE_ASSERT(std::find(foundPlugin->second->metadata.provides().begin(), foundPlugin->second->metadata.provides().end(), alias) != foundPlugin->second->metadata.provides().end(),
            "PluginManager::Manager::setPreferredPlugins():" << plugin << "does not provide" << alias, );
        _state->aliases.erase(foundAlias);
        _state->aliases.insert({alias, *foundPlugin->second});
        break;
    }
}

std::vector<std::string> AbstractManager::pluginList() const {
    std::vector<std::string> names;
    for(const std::pair<const std::string, Containers::Pointer<Plugin>>& plugin: _state->plugins)
        names.push_back(plugin.first);
    return names;
}

std::vector<std::string> AbstractManager::aliasList() const {
    std::vector<std::string> names;
    for(const auto& alias: _state->aliases) names.push_back(alias.first);
    return names;
}

const PluginMetadata* AbstractManager::metadata(const std::string& plugin) const {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) return &found->second.metadata;

    return nullptr;
}

PluginMetadata* AbstractManager::metadata(const std::string& plugin) {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) return &found->second.metadata;

    return nullptr;
}

LoadState AbstractManager::loadState(const std::string& plugin) const {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) return found->second.loadState;

    return LoadState::NotFound;
}

LoadState AbstractManager::load(const std::string& plugin) {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* File path passed, load directly */
    if(Utility::String::endsWith(plugin, _state->pluginSuffix)) {
        /* Dig plugin name from filename and verify it's not loaded at the moment */
        const std::string filename = Utility::Directory::filename(plugin);
        const std::string name = filename.substr(0, filename.length() - _state->pluginSuffix.size());
        const auto found = _state->plugins.find(name);
        if(found != _state->plugins.end() && (found->second->loadState & LoadState::Loaded)) {
            Error{} << "PluginManager::load():" << filename << "conflicts with currently loaded plugin of the same name";
            return LoadState::Used;
        }

        /* Load the plugin and register it only if loading succeeded so we
           don't crap the alias state. If there's already a registered
           plugin of this name, replace it. */
        Containers::Pointer<Plugin> data{new Plugin{name,
            _state->pluginMetadataSuffix.empty() ? std::string{} :
                Directory::join(Utility::Directory::path(plugin), name + _state->pluginMetadataSuffix)}};
        const LoadState state = loadInternal(*data, plugin);
        if(state & LoadState::Loaded) {
            /* Remove the potential plugin with the same name (we already
               checked above that it's *not* loaded) */
            if(found != _state->plugins.end()) {
                /* Erase all aliases that reference this plugin, as they would
                   be dangling now. */
                auto ait = _state->aliases.cbegin();
                while(ait != _state->aliases.cend()) {
                    if(&ait->second == found->second.get())
                        ait = _state->aliases.erase(ait);
                    else ++ait;
                }

                /* Erase the plugin from the plugin map */
                _state->plugins.erase(found);
            }

            registerDynamicPlugin(name, std::move(data));
        }
        return state;
    }
    #endif

    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        return loadInternal(found->second);
        #else
        return found->second.loadState;
        #endif
    }

    Error() << "PluginManager::Manager::load(): plugin" << plugin
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        << "is not static and was not found in" << _state->pluginDirectory;
        #else
        << "was not found";
        #endif
    return LoadState::NotFound;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::loadInternal(Plugin& plugin) {
    return loadInternal(plugin, Directory::join(_state->pluginDirectory, plugin.metadata._name + _state->pluginSuffix));
}

LoadState AbstractManager::loadInternal(Plugin& plugin, const std::string& filename) {
    /* Plugin is not ready to load */
    if(plugin.loadState != LoadState::NotLoaded) {
        if(!(plugin.loadState & (LoadState::Static|LoadState::Loaded)))
            Error() << "PluginManager::Manager::load(): plugin" << plugin.metadata._name << "is not ready to load:" << plugin.loadState;
        return plugin.loadState;
    }

    /* Load dependencies and remember their names for later. Their names will
       be added to usedBy list only if everything goes well. */
    std::vector<Containers::Reference<Plugin>> dependencies;
    dependencies.reserve(plugin.metadata._depends.size());
    for(const std::string& dependency: plugin.metadata._depends) {
        /* If the dependency is not in our plugin manager, check the registered
           external managers as well */
        AbstractManager* dependencyManager = nullptr;
        auto foundDependency = _state->plugins.find(dependency);
        if(foundDependency != _state->plugins.end())
            dependencyManager = this;
        else for(AbstractManager* other: _state->externalManagers) {
            foundDependency = other->_state->plugins.find(dependency);
            if(foundDependency != other->_state->plugins.end()) {
                dependencyManager = other;
                break;
            }
        }

        if(!dependencyManager || !(dependencyManager->loadInternal(*foundDependency->second) & LoadState::Loaded)) {
            Error() << "PluginManager::Manager::load(): unresolved dependency" << dependency << "of plugin" << plugin.metadata._name;
            return LoadState::UnresolvedDependency;
        }

        dependencies.emplace_back(*foundDependency->second);
    }

    /* Open plugin file, make symbols globally available for next libs (which
       may depend on this) */
    #ifndef CORRADE_TARGET_WINDOWS
    void* module = dlopen(filename.data(), RTLD_NOW|RTLD_GLOBAL);
    #else
    HMODULE module = LoadLibraryW(widen(filename).data());
    #endif
    if(!module) {
        Error{} << "PluginManager::Manager::load(): cannot load plugin"
            << plugin.metadata._name << "from \"" << Debug::nospace
            << filename << Debug::nospace << "\":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        return LoadState::LoadFailed;
    }

    /* Check plugin version */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    int (*version)() = reinterpret_cast<int(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginVersion"));
    if(version == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get version of plugin"
            << plugin.metadata._name << Debug::nospace << ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }
    if(version() != Version) {
        Error{} << "PluginManager::Manager::load(): wrong version of plugin"
                << plugin.metadata._name << Debug::nospace << ", expected"
                << Version << "but got" << version();
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::WrongPluginVersion;
    }

    /* Check interface string */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    const char* (*interface)() = reinterpret_cast<const char* (*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInterface"));
    if(interface == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get interface string of plugin"
            << plugin.metadata._name << Debug::nospace << ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }
    if(interface() != pluginInterface()) {
        Error() << "PluginManager::Manager::load(): wrong interface string of plugin" << plugin.metadata._name + ", expected" << pluginInterface() << "but got" << interface();
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::WrongInterfaceVersion;
    }

    /* Load plugin initializer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    void(*initializer)() = reinterpret_cast<void(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInitializer"));
    if(initializer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get initializer of plugin"
            << plugin.metadata._name + ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Load plugin finalizer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    void(*finalizer)() = reinterpret_cast<void(*)()>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginFinalizer"));
    if(finalizer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get finalizer of plugin"
            << plugin.metadata._name + ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Load plugin instancer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    Instancer instancer = reinterpret_cast<Instancer>(
        #ifndef CORRADE_TARGET_WINDOWS
        dlsym
        #else
        GetProcAddress
        #endif
        (module, "pluginInstancer"));
    if(instancer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get instancer of plugin"
            << plugin.metadata._name + ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        #ifndef CORRADE_TARGET_WINDOWS
        dlclose(module);
        #else
        FreeLibrary(module);
        #endif
        return LoadState::LoadFailed;
    }

    /* Initialize plugin */
    initializer();

    /* Everything is okay, add this plugin to usedBy list of each dependency */
    for(Plugin& dependency: dependencies)
        dependency.metadata._usedBy.push_back(plugin.metadata._name);

    /* Update plugin object, set state to loaded */
    plugin.loadState = LoadState::Loaded;
    plugin.module = module;
    plugin.instancer = instancer;
    plugin.finalizer = finalizer;
    return LoadState::Loaded;
}
#endif

LoadState AbstractManager::unload(const std::string& plugin) {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) {
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        return unloadInternal(found->second);
        #else
        return found->second.loadState;
        #endif
    }

    Error() << "PluginManager::Manager::unload(): plugin" << plugin << "was not found";
    return LoadState::NotFound;
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadInternal(Plugin& plugin) {
    /* Plugin is not ready to unload, nothing to do. The only thing this can
       happen is when the plugin is static or not loaded (which is fine, so we
       just return that load state) or when its metadata file is broken (which
       is not good, but what can we do). All other states (such as UnloadFailed
       etc.) are transient -- not saved into the local state, only returned. */
    if(plugin.loadState != LoadState::Loaded) {
        CORRADE_INTERNAL_ASSERT(plugin.loadState & (LoadState::Static|LoadState::NotLoaded|LoadState::WrongMetadataFile));
        return plugin.loadState;
    }

    /* Plugin is used by another plugin, don't unload */
    if(!plugin.metadata._usedBy.empty()) {
        Error{} << "PluginManager::Manager::unload(): plugin"
                << plugin.metadata._name << "is required by other plugins:"
                << plugin.metadata._usedBy;
        return LoadState::Required;
    }

    /* Plugin has active instances */
    if(!plugin.instances.empty()) {
        /* Check if all instances can be safely deleted */
        for(AbstractPlugin* const instance: plugin.instances)
            if(!instance->canBeDeleted()) {
                Error{} << "PluginManager::Manager::unload(): plugin"
                        << plugin.metadata._name
                        << "is currently used and cannot be deleted";
                return LoadState::Used;
            }

        /* If they can be, delete them. They remove itself from instances
           list on destruction, thus going backwards */
        for(std::size_t i = plugin.instances.size(); i != 0; --i)
            delete plugin.instances[i-1];
    }

    /* Remove this plugin from "used by" list of dependencies. If not found
       in this manager, try in registered external managers. */
    for(auto it = plugin.metadata.depends().cbegin(); it != plugin.metadata.depends().cend(); ++it) {
        Plugin* dependency = nullptr;
        auto foundDependency = _state->plugins.find(*it);
        if(foundDependency != _state->plugins.end())
            dependency = foundDependency->second.get();
        else for(AbstractManager* other: _state->externalManagers) {
            foundDependency = other->_state->plugins.find(*it);
            if(foundDependency != other->_state->plugins.end()) {
                dependency = foundDependency->second.get();
                break;
            }
        }
        CORRADE_INTERNAL_ASSERT(dependency);

        auto uit = dependency->metadata._usedBy.begin();
        for(; uit != dependency->metadata._usedBy.end(); ++uit)
            if(*uit == plugin.metadata._name) break;
        CORRADE_INTERNAL_ASSERT(uit != dependency->metadata._usedBy.end());
        dependency->metadata._usedBy.erase(uit);
    }

    /* Finalize plugin */
    plugin.finalizer();

    /* Close the module */
    #ifndef CORRADE_TARGET_WINDOWS
    if(dlclose(plugin.module) != 0)
    #else
    if(!FreeLibrary(plugin.module))
    #endif
    {
        /* This is hard to test, the only possibility I can think of is
           dlclose() when a symbol is still needed (by another plugin, e.g.),
           but that's possible only on QNX, on linux dlclose() only unloads the
           library if it's really not needed. Source:
           https://stackoverflow.com/questions/28882298/error-on-dlclose-shared-objects-still-referenced */
        Error{} << "PluginManager::Manager::unload(): cannot unload plugin"
            << plugin.metadata._name << Debug::nospace << ":"
            #ifndef CORRADE_TARGET_WINDOWS
            << dlerror()
            #else
            << Utility::Implementation::windowsErrorString(GetLastError())
            #endif
            ;
        plugin.loadState = LoadState::NotLoaded;
        return LoadState::UnloadFailed;
    }

    /* Update plugin object, set state to not loaded */
    plugin.loadState = LoadState::NotLoaded;
    plugin.module = nullptr;
    plugin.instancer = nullptr;
    plugin.finalizer = nullptr;
    return LoadState::NotLoaded;
}
#endif

void AbstractManager::registerExternalManager(AbstractManager& manager) {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    _state->externalManagers.insert(&manager);
    #ifndef CORRADE_NO_ASSERT
    manager._state->externalManagerUsedBy.insert(this);
    #endif
    #else
    static_cast<void>(manager);
    #endif
}

void AbstractManager::registerDynamicPlugin(const std::string& name, Containers::Pointer<Plugin>&& plugin) {
    /* Insert plugin to list */
    const auto result = _state->plugins.emplace(name, std::move(plugin));
    CORRADE_INTERNAL_ASSERT(result.second);

    /* The plugin is the best version of itself. If there was already an
       alias for this name, replace it. */
    {
        const auto alias = _state->aliases.find(name);
        if(alias != _state->aliases.end()) _state->aliases.erase(alias);
        CORRADE_INTERNAL_ASSERT_OUTPUT(_state->aliases.insert({name, *result.first->second}).second);
    }

    /* Add aliases to the list. Calling insert() won't overwrite the
       existing value, which ensures that the above note is still held. */
    for(const std::string& alias: result.first->second->metadata._provides) {
        /* Libc++ frees the passed Plugin& reference when using emplace(),
           causing double-free memory corruption later. Everything is okay
           with insert(). */
        /** @todo put back emplace() here when libc++ is fixed */
        _state->aliases.insert({alias, *result.first->second});
    }
}

void AbstractManager::registerInstance(const std::string& plugin, AbstractPlugin& instance, const PluginMetadata*& metadata) {
    /** @todo assert proper interface */
    auto found = _state->aliases.find(plugin);
    CORRADE_ASSERT(found != _state->aliases.end(),
        "PluginManager::AbstractPlugin::AbstractPlugin(): attempt to register instance of plugin not known to given manager", );

    found->second.instances.push_back(&instance);
    metadata = &found->second.metadata;
}

void AbstractManager::reregisterInstance(const std::string& plugin, AbstractPlugin& oldInstance, AbstractPlugin* const newInstance) {
    auto found = _state->aliases.find(plugin);
    CORRADE_INTERNAL_ASSERT(found != _state->aliases.end());

    auto pos = std::find(found->second.instances.begin(), found->second.instances.end(), &oldInstance);
    CORRADE_INTERNAL_ASSERT(pos != found->second.instances.end());

    /* If the plugin is being moved, replace the instance pointer. Otherwise
       remove it from the list, and if the list is empty, delete it fully. */
    if(newInstance) *pos = newInstance;
    else found->second.instances.erase(pos);
}

Containers::Pointer<AbstractPlugin> AbstractManager::instantiateInternal(const std::string& plugin) {
    auto found = _state->aliases.find(plugin);

    CORRADE_ASSERT(found != _state->aliases.end() && (found->second.loadState & LoadState::Loaded),
        "PluginManager::Manager::instantiate(): plugin" << plugin << "is not loaded", nullptr);

    return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, plugin)));
}

Containers::Pointer<AbstractPlugin> AbstractManager::loadAndInstantiateInternal(const std::string& plugin) {
    if(!(load(plugin) & LoadState::Loaded)) return nullptr;

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* If file path passed, instantiate extracted name instead */
    if(Utility::String::endsWith(plugin, _state->pluginSuffix)) {
        const std::string filename = Utility::Directory::filename(plugin);
        const std::string name = filename.substr(0, filename.length() - _state->pluginSuffix.size());
        auto found = _state->aliases.find(name);
        CORRADE_INTERNAL_ASSERT(found != _state->aliases.end());
        return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, name)));
    }
    #endif

    auto found = _state->aliases.find(plugin);
    CORRADE_INTERNAL_ASSERT(found != _state->aliases.end());
    return Containers::pointer(static_cast<AbstractPlugin*>(found->second.instancer(*this, plugin)));
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
AbstractManager::Plugin::Plugin(std::string name, const std::string& metadata): configuration{metadata, Utility::Configuration::Flag::ReadOnly}, metadata{std::move(name), configuration}, instancer{nullptr}, module{nullptr} {
    /* If the path is empty, we don't use any plugin configuration file, so
       don't do any checks. */
    if(metadata.empty()) {
        loadState = LoadState::NotLoaded;
    } else {
        if(configuration.isValid()) {
            if(Utility::Directory::exists(metadata)) {
                loadState = LoadState::NotLoaded;
                return;
            }

            Error{} << "PluginManager::Manager:" << metadata << "was not found";
        }

        loadState = LoadState::WrongMetadataFile;
    }
}
#endif

AbstractManager::Plugin::Plugin(const Implementation::StaticPlugin& staticPlugin, Utility::Configuration&& configuration_): loadState{LoadState::Static}, configuration{std::move(configuration_)}, metadata{staticPlugin.plugin, configuration}, instancer{staticPlugin.instancer}, staticPlugin{&staticPlugin} {}

#ifndef DOXYGEN_GENERATING_OUTPUT
Utility::Debug& operator<<(Utility::Debug& debug, LoadState value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case LoadState::value: return debug << "PluginManager::LoadState::" #value;
        _c(NotFound)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        _c(WrongPluginVersion)
        _c(WrongInterfaceVersion)
        _c(WrongMetadataFile)
        _c(UnresolvedDependency)
        _c(LoadFailed)
        _c(Loaded)
        _c(NotLoaded)
        _c(UnloadFailed)
        _c(Required)
        #endif
        _c(Static)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        _c(Used)
        #endif
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "PluginManager::LoadState(" << Debug::nospace << reinterpret_cast<void*>(std::uint16_t(value)) << Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, const LoadStates value) {
    return Containers::enumSetDebugOutput(debug, value, "PluginManager::LoadStates{}", {
        LoadState::NotFound,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        LoadState::WrongPluginVersion,
        LoadState::WrongInterfaceVersion,
        LoadState::WrongMetadataFile,
        LoadState::UnresolvedDependency,
        LoadState::LoadFailed,
        LoadState::Loaded,
        LoadState::NotLoaded,
        LoadState::UnloadFailed,
        LoadState::Required,
        #endif
        LoadState::Static,
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        LoadState::Used
        #endif
        });
}

#endif

}}
