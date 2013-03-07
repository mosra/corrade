/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "AbstractPluginManager.h"

#include <algorithm>
#include <sstream>

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#undef interface
#define dlsym GetProcAddress
#define dlerror GetLastError
#define dlclose FreeLibrary
#endif

#include "Plugin.h"
#include "Utility/Directory.h"
#include "Utility/Configuration.h"

#include "corradePluginManagerConfigure.h"

using namespace Corrade::Utility;

namespace Corrade { namespace PluginManager {

const int AbstractPluginManager::version = PLUGIN_VERSION;

std::map<std::string, AbstractPluginManager::PluginObject*>* AbstractPluginManager::plugins() {
    static std::map<std::string, PluginObject*>* _plugins = new std::map<std::string, PluginObject*>();

    /* If there are unprocessed static plugins for this manager, add them */
    if(staticPlugins()) {
        for(auto it = staticPlugins()->cbegin(); it != staticPlugins()->cend(); ++it) {
            /* Load static plugin metadata */
            Resource r("plugins");
            std::istringstream metadata(r.get(it->plugin + ".conf"));

            /* Insert plugin to list */
            if(!_plugins->insert(std::make_pair(it->plugin, new PluginObject(metadata, it->interface, it->instancer))).second)
                Warning() << "PluginManager: static plugin" << '\'' + it->plugin + '\'' << "is already imported!";
        }

        /* Delete the array to mark them as processed */
        delete staticPlugins();
        staticPlugins() = nullptr;
    }

    return _plugins;
}

std::vector<AbstractPluginManager::StaticPluginObject>*& AbstractPluginManager::staticPlugins() {
    static std::vector<StaticPluginObject>* _staticPlugins = new std::vector<StaticPluginObject>();

    return _staticPlugins;
}

void AbstractPluginManager::importStaticPlugin(const std::string& plugin, int _version, const std::string& interface, Instancer instancer) {
    if(_version != version) {
        Error() << "PluginManager: wrong version of static plugin" << '\'' + plugin + '\'';
        return;
    }
    if(!staticPlugins()) {
        Error() << "PluginManager: too late to import static plugin" << '\'' + plugin + '\'';
        return;
    }

    StaticPluginObject o = {plugin, interface, instancer};
    staticPlugins()->push_back(o);
}

AbstractPluginManager::AbstractPluginManager(const std::string& pluginDirectory): _pluginDirectory(pluginDirectory) {
    reloadPluginDirectory();
}

AbstractPluginManager::~AbstractPluginManager() {
    /* Destroying all plugin instances. Every instance removes itself from
       instance array on destruction, so going carefully backwards and
       reloading iterator for every plugin name */
    std::map<std::string, std::vector<Plugin*> >::const_iterator it;
    while((it = instances.begin()) != instances.end()) {
        for(int i = it->second.size()-1; i >= 0; --i)
            delete it->second[i];
    }

    /* Unload all plugins associated with this plugin manager */
    std::vector<std::map<std::string, PluginObject*>::iterator> removed;
    for(auto it = plugins()->begin(); it != plugins()->end(); ++it) {

        /* Plugin doesn't belong to this manager */
        if(it->second->manager != this) continue;

        /* Unload the plugin and schedule it for deletion, if it is not static.
           Otherwise just disconnect this manager from the plugin, so another
           manager can take over it in the future. */
        if(unload(it->first) == LoadState::Static)
            it->second->manager = nullptr;
        else
            removed.push_back(it);
    }

    /* Remove the plugins from global container */
    for(auto it = removed.cbegin(); it != removed.cend(); ++it) {
        delete (*it)->second;
        plugins()->erase(*it);
    }
}

std::string AbstractPluginManager::pluginDirectory() const {
    return _pluginDirectory;
}

void AbstractPluginManager::setPluginDirectory(const std::string& directory) {
    _pluginDirectory = directory;
    reloadPluginDirectory();
}

void AbstractPluginManager::reloadPluginDirectory() {
    /* Get all unloaded plugins and schedule them for deletion */
    std::vector<std::string> removed;
    for(auto it = plugins()->cbegin(); it != plugins()->cend(); ++it) {
        if(it->second->manager != this || it->second->loadState != LoadState::NotLoaded)
            continue;

        delete it->second;
        removed.push_back(it->first);
    }

    /* Remove the plugins from global container */
    for(auto it = removed.cbegin(); it != removed.cend(); ++it)
        plugins()->erase(*it);

    /* Foreach all files in plugin directory */
    std::size_t suffixSize = std::string(PLUGIN_FILENAME_SUFFIX).size();
    Directory d(_pluginDirectory, Directory::SkipDirectories|Directory::SkipDotAndDotDot);
    for(Directory::const_iterator i = d.begin(); i != d.end(); ++i) {
        /* Search for module filename suffix in current file */
        std::size_t end = (*i).find(PLUGIN_FILENAME_SUFFIX);

        /* File doesn't have module suffix, continue to next */
        if(end == std::string::npos || end + suffixSize != i->size())
            continue;

        /* Dig plugin name from filename */
        std::string name = (*i).substr(0, end);

        /* Skip the plugin if it is among loaded */
        if(plugins()->find(name) != plugins()->end()) continue;

        /* Insert plugin to list */
        plugins()->insert(std::make_pair(name, new PluginObject(Directory::join(_pluginDirectory, name + ".conf"), this)));
    }
}

std::vector<std::string> AbstractPluginManager::pluginList() const {
    std::vector<std::string> names;
    for(auto i = plugins()->cbegin(); i != plugins()->cend(); ++i) {

        /* Plugin doesn't belong to this manager */
        if(i->second->manager != this) continue;

        names.push_back(i->first);
    }
    return names;
}

const PluginMetadata* AbstractPluginManager::metadata(const std::string& plugin) const {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return nullptr;

    return &foundPlugin->second->metadata;
}

LoadState AbstractPluginManager::loadState(const std::string& plugin) const {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return LoadState::NotFound;

    return foundPlugin->second->loadState;
}

LoadState AbstractPluginManager::load(const std::string& _plugin) {
    auto foundPlugin = plugins()->find(_plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return LoadState::NotFound;

    /* Before loading reload its metadata and if it is not found,
        remove it from the list */
    if(!reloadPluginMetadata(foundPlugin)) {
        delete foundPlugin->second;
        plugins()->erase(foundPlugin);
        return LoadState::NotFound;
    }

    PluginObject& plugin = *foundPlugin->second;

    /* Plugin is not ready to load */
    if(plugin.loadState != LoadState::NotLoaded)
        return plugin.loadState;

    /* Vector of found dependencies. If everything goes well, this plugin will
       be added to each dependency usedBy list. */
    std::vector<std::pair<std::string, PluginObject*>> dependencies;

    /* Load dependencies and add this plugin to their "used by" list */
    for(auto it = plugin.metadata.depends().cbegin(); it != plugin.metadata.depends().cend(); ++it) {
        /* Find manager which is associated to this plugin and load the plugin
           with it */
        auto foundDependency = plugins()->find(*it);

        if(foundDependency == plugins()->end() || !foundDependency->second->manager || !(foundDependency->second->manager->load(*it) & (LoadState::Loaded|LoadState::Static)))
            return LoadState::UnresolvedDependency;

        dependencies.push_back(*foundDependency);
    }

    std::string filename = Directory::join(_pluginDirectory, _plugin + PLUGIN_FILENAME_SUFFIX);

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    #ifndef _WIN32
    void* handle = dlopen(filename.c_str(), RTLD_NOW|RTLD_GLOBAL);
    #else
    HMODULE handle = LoadLibraryA(filename.c_str());
    #endif
    if(!handle) {
        Error() << "PluginManager: cannot open plugin file"
                << '"' + _pluginDirectory + _plugin + PLUGIN_FILENAME_SUFFIX + "\":"
                << dlerror();
        plugin.loadState = LoadState::LoadFailed;
        return plugin.loadState;
    }

    /* Check plugin version */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    int (*_version)(void) = reinterpret_cast<int(*)()>(dlsym(handle, "pluginVersion"));
    if(_version == nullptr) {
        Error() << "PluginManager: cannot get version of plugin" << '\'' + _plugin + "':" << dlerror();
        dlclose(handle);
        plugin.loadState = LoadState::LoadFailed;
        return plugin.loadState;
    }
    if(_version() != version) {
        Error() << "PluginManager: wrong plugin version, expected" << _version() << "got" << version;
        dlclose(handle);
        plugin.loadState = LoadState::WrongPluginVersion;
        return plugin.loadState;
    }

    /* Check interface string */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    const char* (*interface)() = reinterpret_cast<const char* (*)()>(dlsym(handle, "pluginInterface"));
    if(interface == nullptr) {
        Error() << "PluginManager: cannot get interface string of plugin" << '\'' + _plugin + "':" << dlerror();
        dlclose(handle);
        plugin.loadState = LoadState::LoadFailed;
        return plugin.loadState;
    }
    if(interface() != pluginInterface()) {
        Error() << "PluginManager: wrong plugin interface, expected" << '\'' + pluginInterface() + ", got '" + interface() + "'";
        dlclose(handle);
        plugin.loadState = LoadState::WrongInterfaceVersion;
        return plugin.loadState;
    }

    /* Load plugin instancer */
    #ifdef __GNUC__ /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    __extension__
    #endif
    Instancer instancer = reinterpret_cast<Instancer>(dlsym(handle, "pluginInstancer"));
    if(instancer == nullptr) {
        Error() << "PluginManager: cannot get instancer of plugin" << '\'' + _plugin + "':" << dlerror();
        dlclose(handle);
        plugin.loadState = LoadState::LoadFailed;
        return plugin.loadState;
    }

    /* Everything is okay, add this plugin to usedBy list of each dependency */
    for(auto it = dependencies.cbegin(); it != dependencies.cend(); ++it) {
        /* If the plugin is not static with no associated manager, use its
           manager for adding this plugin */
        if(it->second->manager)
            it->second->manager->addUsedBy(it->first, _plugin);

        /* Otherwise add this plugin manually */
        else it->second->metadata._usedBy.push_back(_plugin);
    }

    plugin.loadState = LoadState::Loaded;
    plugin.module = handle;
    plugin.instancer = instancer;
    return plugin.loadState;
}

LoadState AbstractPluginManager::unload(const std::string& _plugin) {
    auto foundPlugin = plugins()->find(_plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return LoadState::NotFound;

    PluginObject& plugin = *foundPlugin->second;

    /* Only unload loaded plugin */
    if(plugin.loadState & (LoadState::Loaded|LoadState::UnloadFailed)) {
        /* Plugin is used by another plugin, don't unload */
        if(!plugin.metadata.usedBy().empty()) return LoadState::Required;

        /* Plugin has active instances */
        auto foundInstance = instances.find(_plugin);
        if(foundInstance != instances.end()) {
            /* Check if all instances can be safely deleted */
            for(auto it = foundInstance->second.cbegin(); it != foundInstance->second.cend(); ++it)
                if(!(*it)->canBeDeleted())
                    return LoadState::Used;

            /* If they can be, delete them. They remove itself from instances
               list on destruction, thus going backwards */
            for(std::size_t i = foundInstance->second.size(); i != 0; --i)
                delete foundInstance->second[i-1];
        }

        /* Remove this plugin from "used by" column of dependencies */
        for(auto it = plugin.metadata.depends().cbegin(); it != plugin.metadata.depends().cend(); ++it) {
            auto mit = plugins()->find(*it);
            /** @bug FIXME: use plugin hierarchy for destruction */

            if(mit != plugins()->end()) {
                /* If the plugin is not static with no associated manager, use
                   its manager for removing this plugin */
                if(mit->second->manager)
                    mit->second->manager->removeUsedBy(mit->first, _plugin);

                /* Otherwise remove this plugin manually */
                else for(auto it = mit->second->metadata._usedBy.begin(); it != mit->second->metadata._usedBy.end(); ++it) {
                    if(*it == _plugin) {
                        mit->second->metadata._usedBy.erase(it);
                        break;
                    }
                }
            }
        }

        #ifndef _WIN32
        if(dlclose(plugin.module) != 0) {
        #else
        if(!FreeLibrary(plugin.module)) {
        #endif
            Error() << "PluginManager: cannot unload plugin" << '\'' + _plugin + "':" << dlerror();
            plugin.loadState = LoadState::UnloadFailed;
            return plugin.loadState;
        }

        plugin.loadState = LoadState::NotLoaded;
    }

    /* After successful unload, reload its metadata and if it is not found,
       remove it from the list */
    if(!reloadPluginMetadata(foundPlugin)) {
        delete foundPlugin->second;
        plugins()->erase(foundPlugin);
        return LoadState::NotLoaded;
    }

    /* Return directly the load state, as 'plugin' reference was deleted by
       reloadPluginMetadata() */
    return foundPlugin->second->loadState;
}

LoadState AbstractPluginManager::reload(const std::string& plugin) {
    /* If the plugin is not loaded, just reload its metadata */
    if(loadState(plugin) == LoadState::NotLoaded) {
        auto foundPlugin = plugins()->find(plugin);

        /* If the plugin is not found, remove it from the list */
        if(!reloadPluginMetadata(foundPlugin)) {
            delete foundPlugin->second;
            plugins()->erase(foundPlugin);
        }

        return LoadState::NotLoaded;

    /* Else try unload and load */
    } else {
        LoadState l = unload(plugin);
        if(l != LoadState::NotLoaded) return l;
        return load(plugin);
    }
}

void AbstractPluginManager::registerInstance(const std::string& plugin, Plugin* instance, const Configuration** configuration, const PluginMetadata** metadata) {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return;

    auto foundInstance = instances.find(plugin);

    if(foundInstance == instances.end())
        foundInstance = instances.insert(std::make_pair(plugin, std::vector<Plugin*>())).first;

    foundInstance->second.push_back(instance);

    *configuration = &foundPlugin->second->configuration;
    *metadata = &foundPlugin->second->metadata;
}

void AbstractPluginManager::unregisterInstance(const std::string& plugin, Plugin* instance) {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist or doesn't belong to this manager, nothing to do */
    if(foundPlugin == plugins()->end() || foundPlugin->second->manager != this)
        return;

    auto foundInstance = instances.find(plugin);
    if(foundInstance == instances.end()) return;
    std::vector<Plugin*>& _instances = foundInstance->second;

    auto pos = std::find(_instances.begin(), _instances.end(), instance);
    if(pos == _instances.end()) return;

    _instances.erase(pos);

    if(_instances.empty()) instances.erase(plugin);
}

bool AbstractPluginManager::reloadPluginMetadata(std::map<std::string, PluginObject*>::iterator it) {
    /* Don't reload metadata of alien or loaded plugins */
    if(it->second->manager != this || (it->second->loadState & (LoadState::Loaded|LoadState::Static)))
        return true;

    /* If plugin binary doesn't exist, schedule the entry for deletion */
    if(!Directory::fileExists(Directory::join(_pluginDirectory, it->first + PLUGIN_FILENAME_SUFFIX)))
        return false;

    /* Reload plugin metadata */
    delete it->second;
    it->second = new PluginObject(Directory::join(_pluginDirectory, it->first + ".conf"), this);

    return true;
}

void AbstractPluginManager::addUsedBy(const std::string& plugin, const std::string& usedBy) {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    foundPlugin->second->metadata._usedBy.push_back(usedBy);
}

void AbstractPluginManager::removeUsedBy(const std::string& plugin, const std::string& usedBy) {
    auto foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    for(auto it = foundPlugin->second->metadata._usedBy.begin(); it != foundPlugin->second->metadata._usedBy.end(); ++it) {
        if(*it == usedBy) {
            foundPlugin->second->metadata._usedBy.erase(it);
            return;
        }
    }
}

AbstractPluginManager::PluginObject::PluginObject(const std::string& _metadata, AbstractPluginManager* _manager): configuration(_metadata, Utility::Configuration::Flag::ReadOnly), metadata(configuration), manager(_manager), instancer(nullptr), module(nullptr) {
    loadState = configuration.isValid() ? LoadState::NotLoaded : LoadState::WrongMetadataFile;
}

AbstractPluginManager::PluginObject::PluginObject(std::istream& _metadata, std::string _interface, Instancer _instancer): loadState(LoadState::Static), interface(_interface), configuration(_metadata, Utility::Configuration::Flag::ReadOnly), metadata(configuration), manager(nullptr), instancer(_instancer), module(nullptr) {}

} namespace Utility {

#ifndef DOXYGEN_GENERATING_OUTPUT
Debug operator<<(Debug debug, PluginManager::LoadState value) {
    switch(value) {
        #define ls(state) case PluginManager::LoadState::state: return debug << "PluginManager::LoadState::" #state;
        ls(NotFound)
        ls(WrongPluginVersion)
        ls(WrongInterfaceVersion)
        ls(WrongMetadataFile)
        ls(UnresolvedDependency)
        ls(LoadFailed)
        ls(Loaded)
        ls(NotLoaded)
        ls(UnloadFailed)
        ls(Required)
        ls(Static)
        ls(Used)
        #undef ls
    }

    return debug << "PluginManager::LoadState::(invalid)";
}
#endif

}}
