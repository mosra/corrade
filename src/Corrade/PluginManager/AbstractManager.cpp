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

#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/PluginManager/AbstractPlugin.h"
#include "Corrade/PluginManager/PluginMetadata.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/Configuration.h"
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

using namespace Corrade::Utility;

namespace Corrade { namespace PluginManager {

struct AbstractManager::StaticPlugin  {
    std::string plugin;
    std::string interface;
    Instancer instancer;
    void(*initializer)();
    void(*finalizer)();
};

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
        StaticPlugin* staticPlugin;

        /* For dynamic plugins */
        #ifndef CORRADE_TARGET_WINDOWS
        void* module;
        #else
        HMODULE module;
        #endif
    };
    #else
    StaticPlugin* staticPlugin;
    #endif

    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    /* Constructor for dynamic plugins */
    explicit Plugin(std::string name, const std::string& metadata, AbstractManager* manager);
    #endif

    /* Constructor for static plugins */
    explicit Plugin(StaticPlugin* staticPlugin);

    /* Ensure that we don't delete staticPlugin twice */
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;

    ~Plugin();
};

struct AbstractManager::GlobalPluginStorage {
    /* Beware: having std::map<std::string, Containers::Pointer> is an
       IMPOSSIBLE feat on GCC 4.7, as it will fails with tons of compiler
       errors because std::pair is trying to copy itself. So calm down and
       ignore those few delete calls. Please. Last tried: March 2018. */
    std::map<std::string, Plugin*> plugins;
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

auto AbstractManager::initializeGlobalPluginStorage() -> GlobalPluginStorage& {
    static GlobalPluginStorage* const plugins = new GlobalPluginStorage;

    /* If there are unprocessed static plugins for this manager, add them */
    if(staticPlugins()) {
        for(StaticPlugin* staticPlugin: *staticPlugins()) {
            /* Insert plugin to the list. The metadata are parsed only when the
               plugin gets actually attached to a concrete manager. This is
               needed in order to have properly reset its mutable configuration
               on manager destruction and also avoid unnecessary work when
               nothing actually used the plugin. */
            const auto result = plugins->plugins.insert(std::make_pair(staticPlugin->plugin, new Plugin{staticPlugin}));
            CORRADE_INTERNAL_ASSERT(result.second);
        }

        /** @todo Assert dependencies of static plugins */

        /* Delete the array to mark them as processed */
        delete staticPlugins();
        staticPlugins() = nullptr;
    }

    return *plugins;
}

std::vector<AbstractManager::StaticPlugin*>*& AbstractManager::staticPlugins() {
    static std::vector<StaticPlugin*>* _staticPlugins = new std::vector<StaticPlugin*>();

    return _staticPlugins;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
void AbstractManager::importStaticPlugin(std::string plugin, int _version, std::string interface, Instancer instancer, void(*initializer)(), void(*finalizer)()) {
    CORRADE_ASSERT(_version == Version,
        "PluginManager: wrong version of static plugin" << plugin + ", got" << _version << "but expected" << Version, );
    CORRADE_ASSERT(staticPlugins(),
        "PluginManager: too late to import static plugin" << plugin, );

    staticPlugins()->push_back(new StaticPlugin{std::move(plugin), std::move(interface), instancer, initializer, finalizer});
}
#endif

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
AbstractManager::AbstractManager(std::string pluginInterface, const std::vector<std::string>& pluginSearchPaths, std::string pluginDirectory):
#else
AbstractManager::AbstractManager(std::string pluginInterface):
#endif
    _plugins(initializeGlobalPluginStorage()),
    _state{Containers::InPlaceInit, std::move(pluginInterface)}
{
    /* Find static plugins which have the same interface and have not
       assigned manager to them */
    for(auto p: _plugins.plugins) {
        if(p.second->loadState != LoadState::Static || p.second->manager != nullptr || p.second->staticPlugin->interface != _state->pluginInterface)
            continue;

        /* Assign the plugin to this manager, parse its metadata and initialize
           it */
        Resource r("CorradeStaticPlugin_" + p.first);
        std::istringstream metadata(r.get(p.first + ".conf"));
        p.second->configuration = Utility::Configuration{metadata, Utility::Configuration::Flag::ReadOnly};
        p.second->metadata.emplace(p.first, p.second->configuration);
        p.second->manager = this;
        p.second->staticPlugin->initializer();

        /* The plugin is the best version of itself. If there was already an
           alias for this name, replace it. */
        {
            const auto alias = _state->aliases.find(p.first);
            if(alias != _state->aliases.end()) _state->aliases.erase(alias);
            CORRADE_INTERNAL_ASSERT_OUTPUT(_state->aliases.insert({p.first, *p.second}).second);
        }

        /* Add aliases to the list (only the ones that aren't already there are
           added) */
        for(const std::string& alias: p.second->metadata->_provides) {
            /* Libc++ frees the passed Plugin& reference when using emplace(),
               causing double-free memory corruption later. Everything is okay
               with insert(). */
            /** @todo put back emplace() here when libc++ is fixed */
            _state->aliases.insert({alias, *p.second});
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
           CorradeUtility.dll as a plugin. */
        if(_state->pluginDirectory.empty())
            Warning{} << "PluginManager::Manager::Manager(): none of the plugin search paths in" << pluginSearchPaths << "exists and pluginDirectory was not set, skipping plugin discovery";
    }
    #endif
}

AbstractManager::~AbstractManager() {
    /* Unload all plugins associated with this plugin manager */
    auto it = _plugins.plugins.begin();
    while(it != _plugins.plugins.end()) {
        /* Plugin doesn't belong to this manager */
        if(it->second->manager != this) {
            ++it;
            continue;
        }

        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        /* Try to unload the plugin (and all plugins that depend on it) */
        const LoadState loadState = unloadRecursiveInternal(*it->second);

        /* If the plugin is static, reset the metadata so any changes to the
           plugin-specific configuration are discarded. For dynamic plugins
           fully erase them from the container. */
        if(loadState == LoadState::Static) {
            it->second->metadata = Containers::NullOpt;
        } else {
            delete it->second;
            it = _plugins.plugins.erase(it);
            continue;
        }
        #endif

        /* Otherwise just disconnect this manager from the plugin and finalize
           it, so another manager can take over it in the future. */
        it->second->manager = nullptr;
        it->second->staticPlugin->finalizer();
        ++it;
    }
}

#ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
LoadState AbstractManager::unloadRecursive(const std::string& plugin) {
    const auto found = _plugins.plugins.find(plugin);
    CORRADE_INTERNAL_ASSERT(found != _plugins.plugins.end());
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
    auto it = _plugins.plugins.cbegin();
    while(it != _plugins.plugins.cend()) {
        if(it->second->manager == this && it->second->loadState & (LoadState::NotLoaded|LoadState::WrongMetadataFile)) {
            delete it->second;
            it = _plugins.plugins.erase(it);
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
        if(_plugins.plugins.find(name) != _plugins.plugins.end()) continue;

        registerDynamicPlugin(name, new Plugin{name, Directory::join(_state->pluginDirectory, name + ".conf"), this});
    }

    /* If some of the currently loaded plugins aliased plugins that werre in
       the old plugin directory, these are no longer there. Refresh the alias
       list with the new plugins. */
    for(auto p: _plugins.plugins) {
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
        auto foundPlugin = _plugins.plugins.find(plugin);
        if(foundPlugin == _plugins.plugins.end() || foundPlugin->second->manager != this)
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
    for(const std::pair<std::string, Plugin*>& plugin: _plugins.plugins) {
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
        const auto found = _plugins.plugins.find(name);
        if(found != _plugins.plugins.end() && (found->second->loadState & LoadState::Loaded)) {
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
            if(found != _plugins.plugins.end()) {
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
                _plugins.plugins.erase(found);
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
        const auto foundDependency = _plugins.plugins.find(dependency);

        if(foundDependency == _plugins.plugins.end() || !foundDependency->second->manager ||
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
        auto mit = _plugins.plugins.find(*it);
        if(mit == _plugins.plugins.end()) continue;

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
    const auto result = _plugins.plugins.insert({name, plugin});
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

void AbstractManager::unregisterInstance(const std::string& plugin, AbstractPlugin& instance) {
    auto found = _state->aliases.find(plugin);

    CORRADE_INTERNAL_ASSERT(found != _state->aliases.end() && found->second.manager == this);

    auto foundInstance = _state->instances.find(found->second.metadata->name());
    CORRADE_INTERNAL_ASSERT(foundInstance != _state->instances.end());
    std::vector<AbstractPlugin*>& instancesForPlugin = foundInstance->second;

    auto pos = std::find(instancesForPlugin.begin(), instancesForPlugin.end(), &instance);
    CORRADE_INTERNAL_ASSERT(pos != instancesForPlugin.end());

    instancesForPlugin.erase(pos);

    if(instancesForPlugin.empty()) _state->instances.erase(foundInstance);
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
    loadState = configuration.isValid() ? LoadState::NotLoaded : LoadState::WrongMetadataFile;
}
#endif

AbstractManager::Plugin::Plugin(StaticPlugin* staticPlugin): loadState{LoadState::Static}, manager{nullptr}, instancer{staticPlugin->instancer}, staticPlugin{staticPlugin} {}

AbstractManager::Plugin::~Plugin() {
    #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
    if(loadState == LoadState::Static)
    #endif
        delete staticPlugin;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
Utility::Debug& operator<<(Utility::Debug& debug, PluginManager::LoadState value) {
    switch(value) {
        /* LCOV_EXCL_START */
        #define ls(state) case PluginManager::LoadState::state: return debug << "PluginManager::LoadState::" #state;
        ls(NotFound)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        ls(WrongPluginVersion)
        ls(WrongInterfaceVersion)
        ls(WrongMetadataFile)
        ls(UnresolvedDependency)
        ls(LoadFailed)
        ls(Loaded)
        ls(NotLoaded)
        ls(UnloadFailed)
        ls(Required)
        #endif
        ls(Static)
        #ifndef CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT
        ls(Used)
        #endif
        #undef ls
        /* LCOV_EXCL_STOP */
    }

    return debug << "PluginManager::LoadState(" << Debug::nospace << reinterpret_cast<void*>(std::uint16_t(value)) << Debug::nospace << ")";
}
#endif

}}
