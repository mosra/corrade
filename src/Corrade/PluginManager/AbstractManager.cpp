/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
#define dlsym GetProcAddress
#define dlerror GetLastError
#define dlclose FreeLibrary
#include "Corrade/Utility/Unicode.h"
using Corrade::Utility::Unicode::widen;
#endif

#include "Corrade/PluginManager/configure.h"
#endif

#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC) && !defined(CORRADE_TARGET_WINDOWS_RT)
#include "Corrade/Utility/Implementation/WindowsWeakSymbol.h"
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
    /* Is NullOpt only for static plugins without an assigned manager */
    Containers::Optional<PluginMetadata> metadata;

    /* If set to nullptr, the plugin has not any associated plugin manager and
       cannot be loaded. */
    AbstractManager* manager;

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

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Constructor for dynamic plugins */
    explicit Plugin(std::string name, const std::string& metadata, AbstractManager* manager);
    #endif

    /* Constructor for static plugins */
    explicit Plugin(const Implementation::StaticPlugin& staticPlugin);

    /* Ensure that we don't delete staticPlugin twice */
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;

    ~Plugin() = default;
};

struct AbstractManager::State {
    explicit State(std::string&& pluginInterface): pluginInterface{std::move(pluginInterface)} {}

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    std::string pluginDirectory;
    #endif
    std::string pluginInterface;
    std::map<std::string, Plugin&> aliases;
    std::map<std::string, std::vector<AbstractPlugin*>> instances;
};

const int AbstractManager::Version = CORRADE_PLUGIN_VERSION;

#if !defined(CORRADE_BUILD_STATIC) || defined(CORRADE_TARGET_WINDOWS)
/* (Of course) can't be in an unnamed namespace in order to export it below
   (except for Windows, where we do extern "C" so this doesn't matter) */
namespace {
#endif

#if !defined(CORRADE_BUILD_STATIC) || (defined(CORRADE_BUILD_STATIC) && !defined(CORRADE_TARGET_WINDOWS)) || defined(CORRADE_TARGET_WINDOWS_RT)
#ifdef CORRADE_BUILD_STATIC
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

#ifdef CORRADE_BUILD_MULTITHREADED
CORRADE_THREAD_LOCAL
#endif
#if defined(CORRADE_BUILD_STATIC) && !defined(CORRADE_TARGET_WINDOWS)
/* See above for details why is this here. We need two separate globals because
   static plugin registration *can't be* thread-local (it's written to from one
   thread but then only read from multiple) while the global plugin storage
   *has to be* thread-local as there's dependency management, refcounting etc.
   going on. Windows handled differently below. */
CORRADE_VISIBILITY_EXPORT
    #ifdef __GNUC__
    __attribute__((weak))
    #else
    /* uh oh? the test will fail, probably */
    #endif
#endif
/* A map of plugins. Gets allocated by a manager on construction (if not
   already), deallocated on manager destruction in case there are no plugins
   left in it anymore.

   Beware: having std::map<std::string, Containers::Pointer> is an IMPOSSIBLE
   feat on GCC 4.7, as it will fails with tons of compiler errors because
   std::pair is trying to copy itself. So calm down and ignore those few delete
   calls. Please. Last tried: March 2018.

   The value of this variable is guaranteed to be zero-filled even before any
   static plugin initializers are executed, which means we don't hit any static
   initialization order fiasco. */
std::map<std::string, AbstractManager::Plugin*>* globalPlugins = nullptr;

#if !defined(CORRADE_BUILD_STATIC) || defined(CORRADE_TARGET_WINDOWS)
}
#endif

/* Windows can't have a symbol both thread-local and exported, moreover there
   isn't any concept of weak symbols. Exporting thread-local symbols can be
   worked around by exporting a function that then returns a reference to a
   non-exported thread-local symbol; and finally GetProcAddress() on
   GetModuleHandle(nullptr) "emulates" the weak linking as it's guaranteed to
   pick up the same symbol of the final exe independently of the DLL it was
   called from. To avoid #ifdef hell in code below, the globalStaticPlugins /
   globalPlugins are redefined to return a value from this uniqueness-ensuring
   function. */
#if defined(CORRADE_TARGET_WINDOWS) && defined(CORRADE_BUILD_STATIC) && !defined(CORRADE_TARGET_WINDOWS_RT)
extern "C" CORRADE_VISIBILITY_EXPORT std::map<std::string, AbstractManager::Plugin*>*& corradePluginManagerUniqueGlobalPlugins();
extern "C" CORRADE_VISIBILITY_EXPORT std::map<std::string, AbstractManager::Plugin*>*& corradePluginManagerUniqueGlobalPlugins() {
    return globalPlugins;
}

namespace {

Implementation::StaticPlugin*& windowsGlobalStaticPlugins() {
    /* A function-local static to ensure it's only initialized once without any
       race conditions among threads */
    static Implementation::StaticPlugin** const uniqueGlobals = reinterpret_cast<Implementation::StaticPlugin**>(Utility::Implementation::windowsWeakSymbol("corradePluginManagerUniqueGlobalStaticPlugins", &corradePluginManagerUniqueGlobalStaticPlugins));
    return *uniqueGlobals;
}

std::map<std::string, AbstractManager::Plugin*>*& windowsGlobalPlugins() {
    /* A function-local static to ensure it's only initialized once without any
       race conditions among threads */
    static std::map<std::string, AbstractManager::Plugin*>*&(*const uniqueGlobals)() = reinterpret_cast<std::map<std::string, AbstractManager::Plugin*>*&(*)()>(Utility::Implementation::windowsWeakSymbol("corradePluginManagerUniqueGlobalPlugins", reinterpret_cast<void*>(&corradePluginManagerUniqueGlobalPlugins)));
    return uniqueGlobals();
}

}

#define globalStaticPlugins windowsGlobalStaticPlugins()
#define globalPlugins windowsGlobalPlugins()
#endif

static_assert(std::is_pod<Implementation::StaticPlugin>::value,
    "static plugins shouldn't cause any global initialization / finalization to happen on their own");

void AbstractManager::importStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Debug::nospace << ", got" << version << "but expected" << Version, );
    Containers::Implementation::forwardListInsert(globalStaticPlugins, plugin);
}

void AbstractManager::ejectStaticPlugin(int version, Implementation::StaticPlugin& plugin) {
    CORRADE_ASSERT(version == Version,
        "PluginManager: wrong version of static plugin" << plugin.plugin << Debug::nospace << ", got" << version << "but expected" << Version, );
    Containers::Implementation::forwardListRemove(globalStaticPlugins, plugin);
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
AbstractManager::AbstractManager(std::string pluginInterface, const std::vector<std::string>& pluginSearchPaths, std::string pluginDirectory):
#else
AbstractManager::AbstractManager(std::string pluginInterface):
#endif
    _state{Containers::InPlaceInit, std::move(pluginInterface)}
{
    /* If the global storage doesn't exist yet, allocate it. This gets deleted
       when it's fully empty again on manager destruction. */
    if(!globalPlugins) globalPlugins = new std::map<std::string, AbstractManager::Plugin*>;

    /* Add static plugins which have the same interface and don't have a
       manager assigned to them (i.e, aren't in the map yet). */
    for(const Implementation::StaticPlugin* staticPlugin = globalStaticPlugins; staticPlugin; staticPlugin = Containers::Implementation::forwardListNext(*staticPlugin)) {
        /* The plugin doesn't belong to this manager, skip it */
        if(staticPlugin->interface != _state->pluginInterface) continue;

        /* Attempt to insert the plugin into the global list. If it's
           already there, it's owned by another plugin manager. Skip it. */
        const auto inserted = globalPlugins->insert(std::make_pair(staticPlugin->plugin, nullptr));
        if(!inserted.second) continue;

        /* Only allocate the Plugin in case the insertion happened. */
        Plugin& p = *(inserted.first->second = new Plugin{*staticPlugin});

        /* Assign the plugin to this manager, parse its metadata and
           initialize it */
        Resource r("CorradeStaticPlugin_" + inserted.first->first);
        std::istringstream metadata(r.get(inserted.first->first + ".conf"));
        p.configuration = Utility::Configuration{metadata, Utility::Configuration::Flag::ReadOnly};
        p.metadata.emplace(inserted.first->first, p.configuration);
        p.manager = this;
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
        for(const std::string& alias: p.metadata->_provides) {
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
    /* Unload all plugins associated with this plugin manager */
    auto it = globalPlugins->begin();
    while(it != globalPlugins->end()) {
        /* Plugin doesn't belong to this manager */
        if(it->second->manager != this) {
            ++it;
            continue;
        }

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /* Try to unload the plugin (and all plugins that depend on it). If
           that fails for some reason, it'll blow up with an assert. */
        unloadRecursiveInternal(*it->second);
        #endif

        /* Finalize static plugins before they get removed from the list */
        if(it->second->loadState == LoadState::Static)
            it->second->staticPlugin->finalizer();

        /* Fully erase the plugin from the container, both static and dynamic
           ones. The static ones get re-added next time a manager of matching
           interface is instantiated. */
        delete it->second;
        it = globalPlugins->erase(it);
    }

    /* If there's nothing left, deallocate the storage. If a manager needs it
       again, it will allocate it on its own. */
    if(globalPlugins->empty()) {
        delete globalPlugins;
        globalPlugins = nullptr;
    }
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadRecursive(const std::string& plugin) {
    const auto found = globalPlugins->find(plugin);
    CORRADE_INTERNAL_ASSERT(found != globalPlugins->end());
    return unloadRecursiveInternal(*found->second);
}

LoadState AbstractManager::unloadRecursiveInternal(Plugin& plugin) {
    /* Plugin doesn't belong to this manager, cannot do anything. Caller takes
       care of properly blowing up. */
    if(plugin.manager != this) return LoadState::NotFound;

    /* If the plugin is not static and is used by others, try to unload these
       first so it can be unloaded too. Verification that the child actually
       got unloaded is done by assert for the above return value and the assert
       down below. This is done for both dynamic and static plugins to have
       equivalent behavior on platforms that don't have dynamic plugins. */
    while(!plugin.metadata->_usedBy.empty())
        CORRADE_ASSERT_OUTPUT(unloadRecursive(plugin.metadata->_usedBy.front()) != LoadState::NotFound,
            "PluginManager::Manager: wrong destruction order, cannot unload" << plugin.metadata->_name << "that depends on" << plugin.metadata->_usedBy.front() << "from a different manager instance", {});

    /* Unload the plugin */
    const LoadState after = unloadInternal(plugin);
    CORRADE_ASSERT(after & (LoadState::Static|LoadState::NotLoaded|LoadState::WrongMetadataFile),
        "PluginManager::Manager: cannot unload plugin" << plugin.metadata->_name << "on manager destruction:" << after, {});

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
    auto it = globalPlugins->cbegin();
    while(it != globalPlugins->cend()) {
        if(it->second->manager == this && it->second->loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile)) {
            delete it->second;
            it = globalPlugins->erase(it);
        } else ++it;
    }

    /* Find plugin files in the directory. Sort the list so we have predictable
       plugin preference behavior for aliases on systems that have random
       directory listing order. */
    const std::vector<std::string> d = Directory::list(_state->pluginDirectory,
        Directory::Flag::SkipDirectories|Directory::Flag::SkipDotAndDotDot|
        Directory::Flag::SortAscending);
    for(const std::string& filename: d) {
        /* File doesn't have module suffix, continue to next */
        if(!Utility::String::endsWith(filename, PLUGIN_FILENAME_SUFFIX))
            continue;

        /* Dig plugin name from filename */
        const std::string name = filename.substr(0, filename.length() - sizeof(PLUGIN_FILENAME_SUFFIX) + 1);

        /* Skip the plugin if it is among loaded */
        if(globalPlugins->find(name) != globalPlugins->end()) continue;

        registerDynamicPlugin(name, new Plugin{name, Directory::join(_state->pluginDirectory, name + ".conf"), this});
    }

    /* If some of the currently loaded plugins aliased plugins that werre in
       the old plugin directory, these are no longer there. Refresh the alias
       list with the new plugins. */
    for(auto p: *globalPlugins) {
        if(p.second->manager != this) continue;

        /* Add aliases to the list (only the ones that aren't already there are
           added, calling insert() won't overwrite the existing value) */
        for(const std::string& alias: p.second->metadata->_provides) {
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
        auto foundPlugin = globalPlugins->find(plugin);
        if(foundPlugin == globalPlugins->end() || foundPlugin->second->manager != this)
            continue;

        CORRADE_ASSERT(std::find(foundPlugin->second->metadata->provides().begin(), foundPlugin->second->metadata->provides().end(), alias) != foundPlugin->second->metadata->provides().end(),
            "PluginManager::Manager::setPreferredPlugins():" << plugin << "does not provide" << alias, );
        _state->aliases.erase(foundAlias);
        _state->aliases.insert({alias, *foundPlugin->second});
        break;
    }
}

std::vector<std::string> AbstractManager::pluginList() const {
    std::vector<std::string> names;
    for(const std::pair<std::string, Plugin*>& plugin: *globalPlugins) {
        /* Plugin doesn't belong to this manager */
        if(plugin.second->manager != this) continue;

        names.push_back(plugin.first);
    }
    return names;
}

std::vector<std::string> AbstractManager::aliasList() const {
    std::vector<std::string> names;
    for(const auto& alias: _state->aliases) names.push_back(alias.first);
    return names;
}

const PluginMetadata* AbstractManager::metadata(const std::string& plugin) const {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) return &*found->second.metadata;

    return nullptr;
}

PluginMetadata* AbstractManager::metadata(const std::string& plugin) {
    auto found = _state->aliases.find(plugin);
    if(found != _state->aliases.end()) return &*found->second.metadata;

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
    if(Utility::String::endsWith(plugin, PLUGIN_FILENAME_SUFFIX)) {
        /* Dig plugin name from filename and verify it's not loaded at the moment */
        const std::string filename = Utility::Directory::filename(plugin);
        const std::string name = filename.substr(0, filename.length() - sizeof(PLUGIN_FILENAME_SUFFIX) + 1);
        const auto found = globalPlugins->find(name);
        if(found != globalPlugins->end() && (found->second->loadState & LoadState::Loaded)) {
            Error{} << "PluginManager::load():" << filename << "conflicts with currently loaded plugin of the same name";
            return LoadState::Used;
        }

        /* Load the plugin and register it only if loading succeeded so we
           don't crap the alias state. If there's already a registered
           plugin of this name, replace it. */
        Containers::Pointer<Plugin> data{new Plugin{name, Directory::join(Utility::Directory::path(plugin), name + ".conf"), this}};
        const LoadState state = loadInternal(*data, plugin);
        if(state & LoadState::Loaded) {
            /* Remove the potential plugin with the same name (we already
               checked above that it's *not* loaded) */
            if(found != globalPlugins->end()) {
                /* Erase all aliases that reference this plugin, as they would
                   be dangling now. */
                auto ait = _state->aliases.cbegin();
                while(ait != _state->aliases.cend()) {
                    if(&ait->second == found->second)
                        ait = _state->aliases.erase(ait);
                    else ++ait;
                }

                /* Erase the plugin from the plugin map. It could happen that
                   the original plugin was not owned by this plugin manager --
                   but since we were able to load the new plugin, everything
                   should be fine. */
                delete found->second;
                globalPlugins->erase(found);
            }

            registerDynamicPlugin(name, data.release());
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
    return loadInternal(plugin, Directory::join(_state->pluginDirectory, plugin.metadata->_name + PLUGIN_FILENAME_SUFFIX));
}

LoadState AbstractManager::loadInternal(Plugin& plugin, const std::string& filename) {
    /* Plugin is not ready to load */
    if(plugin.loadState != LoadState::NotLoaded) {
        if(!(plugin.loadState & (LoadState::Static|LoadState::Loaded)))
            Error() << "PluginManager::Manager::load(): plugin" << plugin.metadata->_name << "is not ready to load:" << plugin.loadState;
        return plugin.loadState;
    }

    /* Load dependencies and remember their names for later. Their names will
       be added to usedBy list only if everything goes well. */
    std::vector<Containers::Reference<Plugin>> dependencies;
    dependencies.reserve(plugin.metadata->_depends.size());
    for(const std::string& dependency: plugin.metadata->_depends) {
        /* Find manager which is associated to this plugin and load the plugin
           with it */
        const auto foundDependency = globalPlugins->find(dependency);

        if(foundDependency == globalPlugins->end() || !foundDependency->second->manager ||
           !(foundDependency->second->manager->loadInternal(*foundDependency->second) & LoadState::Loaded))
        {
            Error() << "PluginManager::Manager::load(): unresolved dependency" << dependency << "of plugin" << plugin.metadata->_name;
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
                << plugin.metadata->_name << "from \"" << Debug::nospace
                << filename << Debug::nospace << "\":" << dlerror();
        return LoadState::LoadFailed;
    }

    /* Check plugin version */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    int (*version)() = reinterpret_cast<int(*)()>(dlsym(module, "pluginVersion"));
    if(version == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get version of plugin"
                << plugin.metadata->_name << Debug::nospace << ":" << dlerror();
        dlclose(module);
        return LoadState::LoadFailed;
    }
    if(version() != Version) {
        Error{} << "PluginManager::Manager::load(): wrong version of plugin"
                << plugin.metadata->_name << Debug::nospace << ", expected"
                << Version << "but got" << version();
        dlclose(module);
        return LoadState::WrongPluginVersion;
    }

    /* Check interface string */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    const char* (*interface)() = reinterpret_cast<const char* (*)()>(dlsym(module, "pluginInterface"));
    if(interface == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get interface string of plugin"
                << plugin.metadata->_name << Debug::nospace << ":" << dlerror();
        dlclose(module);
        return LoadState::LoadFailed;
    }
    if(interface() != pluginInterface()) {
        Error() << "PluginManager::Manager::load(): wrong interface string of plugin" << plugin.metadata->_name + ", expected" << pluginInterface() << "but got" << interface();
        dlclose(module);
        return LoadState::WrongInterfaceVersion;
    }

    /* Load plugin initializer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    void(*initializer)() = reinterpret_cast<void(*)()>(dlsym(module, "pluginInitializer"));
    if(initializer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get initializer of plugin"
                << plugin.metadata->_name + ":" << dlerror();
        dlclose(module);
        return LoadState::LoadFailed;
    }

    /* Load plugin finalizer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    void(*finalizer)() = reinterpret_cast<void(*)()>(dlsym(module, "pluginFinalizer"));
    if(finalizer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get finalizer of plugin"
                << plugin.metadata->_name + ":" << dlerror();
        dlclose(module);
        return LoadState::LoadFailed;
    }

    /* Load plugin instancer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    Instancer instancer = reinterpret_cast<Instancer>(dlsym(module, "pluginInstancer"));
    if(instancer == nullptr) {
        Error{} << "PluginManager::Manager::load(): cannot get instancer of plugin"
                << plugin.metadata->_name + ":" << dlerror();
        dlclose(module);
        return LoadState::LoadFailed;
    }

    /* Initialize plugin */
    initializer();

    /* Everything is okay, add this plugin to usedBy list of each dependency */
    for(Plugin& dependency: dependencies)
        dependency.metadata->_usedBy.push_back(plugin.metadata->_name);

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
    if(!plugin.metadata->_usedBy.empty()) {
        Error{} << "PluginManager::Manager::unload(): plugin"
                << plugin.metadata->_name << "is required by other plugins:"
                << plugin.metadata->_usedBy;
        return LoadState::Required;
    }

    /* Plugin has active instances */
    auto foundInstance = _state->instances.find(plugin.metadata->_name);
    if(foundInstance != _state->instances.end()) {
        /* Check if all instances can be safely deleted */
        for(AbstractPlugin* const instance: foundInstance->second)
            if(!instance->canBeDeleted()) {
                Error{} << "PluginManager::Manager::unload(): plugin"
                        << plugin.metadata->_name
                        << "is currently used and cannot be deleted";
                return LoadState::Used;
            }

        /* If they can be, delete them. They remove itself from instances
           list on destruction, thus going backwards */
        for(std::size_t i = foundInstance->second.size(); i != 0; --i)
            delete foundInstance->second[i-1];
    }

    /* Remove this plugin from "used by" list of dependencies */
    for(auto it = plugin.metadata->depends().cbegin(); it != plugin.metadata->depends().cend(); ++it) {
        auto mit = globalPlugins->find(*it);
        if(mit == globalPlugins->end()) continue;

        for(auto uit = mit->second->metadata->_usedBy.begin(); uit != mit->second->metadata->_usedBy.end(); ++uit) {
            if(*uit != plugin.metadata->_name) continue;

            mit->second->metadata->_usedBy.erase(uit);
            break;
        }
    }

    /* Finalize plugin */
    plugin.finalizer();

    /* Close the module */
    #ifndef CORRADE_TARGET_WINDOWS
    if(dlclose(plugin.module) != 0) {
    #else
    if(!FreeLibrary(plugin.module)) {
    #endif
        /* This is hard to test, the only possibility I can think of is
           dlclose() when a symbol is still needed (by another plugin, e.g.),
           but that's possible only on QNX, on linux dlclose() only unloads the
           library if it's really not needed. Source:
           https://stackoverflow.com/questions/28882298/error-on-dlclose-shared-objects-still-referenced */
        Error{} << "PluginManager::Manager::unload(): cannot unload plugin"
                << plugin.metadata->_name << Debug::nospace << ":" << dlerror();
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

void AbstractManager::registerDynamicPlugin(const std::string& name, Plugin* const plugin) {
    /* Insert plugin to list */
    const auto result = globalPlugins->insert({name, plugin});
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
    for(const std::string& alias: result.first->second->metadata->_provides) {
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

    CORRADE_ASSERT(found != _state->aliases.end() && found->second.manager == this,
        "PluginManager::AbstractPlugin::AbstractPlugin(): attempt to register instance of plugin not known to given manager", );

    auto foundInstance = _state->instances.find(plugin);

    if(foundInstance == _state->instances.end())
        foundInstance = _state->instances.insert({found->second.metadata->name(), {}}).first;

    foundInstance->second.push_back(&instance);

    metadata = &*found->second.metadata;
}

void AbstractManager::reregisterInstance(const std::string& plugin, AbstractPlugin& oldInstance, AbstractPlugin* const newInstance) {
    auto found = _state->aliases.find(plugin);

    CORRADE_INTERNAL_ASSERT(found != _state->aliases.end() && found->second.manager == this);

    auto foundInstance = _state->instances.find(found->second.metadata->name());
    CORRADE_INTERNAL_ASSERT(foundInstance != _state->instances.end());
    std::vector<AbstractPlugin*>& instancesForPlugin = foundInstance->second;

    auto pos = std::find(instancesForPlugin.begin(), instancesForPlugin.end(), &oldInstance);
    CORRADE_INTERNAL_ASSERT(pos != instancesForPlugin.end());

    /* If the plugin is being moved, replace the instance pointer. Otherwise
       remove it from the list, and if the list is empty, delete it fully. */
    if(newInstance) *pos = newInstance;
    else {
        instancesForPlugin.erase(pos);
        if(instancesForPlugin.empty()) _state->instances.erase(foundInstance);
    }
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
    if(Utility::String::endsWith(plugin, PLUGIN_FILENAME_SUFFIX)) {
        const std::string filename = Utility::Directory::filename(plugin);
        const std::string name = filename.substr(0, filename.length() - sizeof(PLUGIN_FILENAME_SUFFIX) + 1);
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
AbstractManager::Plugin::Plugin(std::string name, const std::string& metadata, AbstractManager* manager): configuration{metadata, Utility::Configuration::Flag::ReadOnly}, metadata{Containers::InPlaceInit, std::move(name), configuration}, manager{manager}, instancer{nullptr}, module{nullptr} {
    if(configuration.isValid()) {
        if(Utility::Directory::exists(metadata)) {
            loadState = LoadState::NotLoaded;
            return;
        }

        Error{} << "PluginManager::Manager:" << metadata << "was not found";
    }

    loadState = LoadState::WrongMetadataFile;
}
#endif

AbstractManager::Plugin::Plugin(const Implementation::StaticPlugin& staticPlugin): loadState{LoadState::Static}, manager{nullptr}, instancer{staticPlugin.instancer}, staticPlugin{&staticPlugin} {}

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
