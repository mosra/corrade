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

#include "AbstractPluginManager.h"

#include <algorithm>
#include <iostream>

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#undef interface
#define dlsym GetProcAddress
#define dlerror GetLastError
#define dlclose FreeLibrary
#endif

#include "AbstractPluginManagerConfigure.h"
#include "Plugin.h"
#include "Utility/Directory.h"
#include "Utility/Configuration.h"

using namespace std;
using namespace Kompas::Utility;

namespace Kompas { namespace PluginManager {

map<string, AbstractPluginManager::PluginObject*>* AbstractPluginManager::plugins() {
    static map<string, PluginObject*>* _plugins = new map<string, PluginObject*>();

    return _plugins;
}

void AbstractPluginManager::importStaticPlugin(const string& plugin, int _version, const std::string& interface, void* (*instancer)(AbstractPluginManager*, const std::string&)) {
    if(_version != version) return;

    /* Load static plugin metadata */
    Resource r("plugins");
    std::istringstream metadata(r.get(plugin + ".conf"));

    /* Insert plugin to list */
    plugins()->insert(pair<string, PluginObject*>(plugin, new PluginObject(metadata, interface, instancer)));
}

AbstractPluginManager::~AbstractPluginManager() {
    /* Destroying all plugin instances. Every instance removes itself from
        instances array on destruction, so going carefully backwards and
        reloading iterator for every plugin name */
    map<string, vector<Plugin*> >::const_iterator it;
    while((it = instances.begin()) != instances.end()) {
        for(int i = it->second.size()-1; i >= 0; --i) {
            delete it->second[i];
        }
    }

    /* Unload all plugins associated with this plugin manager */
    vector<string> removed;
    for(map<string, PluginObject*>::const_iterator it = plugins()->begin(); it != plugins()->end(); ++it) {
        /* Plugin doesn't belong to this manager */
        if(it->second->manager != this) continue;

        /* Schedule the plugin for deletion, if it is not static */
        if(unload(it->first) != IsStatic) {
            delete it->second;
            removed.push_back(it->first);
        }
    }

    /* Remove the plugins from global container */
    for(vector<string>::const_iterator it = removed.begin(); it != removed.end(); ++it)
        plugins()->erase(*it);
}

void AbstractPluginManager::reloadPluginDirectory() {
    /* Get all unloaded plugins and schedule them for deletion */
    vector<string> removed;
    for(map<string, PluginObject*>::const_iterator it = plugins()->begin(); it != plugins()->end(); ++it) {
        if(it->second->manager != this || it->second->loadState != NotLoaded)
            continue;

        delete it->second;
        removed.push_back(it->first);
    }

    /* Remove the plugins from global container */
    for(vector<string>::const_iterator it = removed.begin(); it != removed.end(); ++it)
        plugins()->erase(*it);

    /* Foreach all files in plugin directory */
    size_t suffixSize = string(PLUGIN_FILENAME_SUFFIX).size();
    Directory d(_pluginDirectory, Directory::SkipDirectories|Directory::SkipDotAndDotDot);
    for(Directory::const_iterator i = d.begin(); i != d.end(); ++i) {
        /* Search for module filename suffix in current file */
        size_t end = (*i).find(PLUGIN_FILENAME_SUFFIX);

        /* File doesn't have module suffix, continue to next */
        if(end == string::npos || end + suffixSize != i->size())
            continue;

        /* Dig plugin name from filename */
        string name = (*i).substr(0, end);

        /* Skip the plugin if it is among loaded */
        if(plugins()->find(name) != plugins()->end()) continue;

        /* Insert plugin to list */
        plugins()->insert(pair<string, PluginObject*>(name, new PluginObject(Directory::join(_pluginDirectory, name + ".conf"), this)));
    }
}

vector<string> AbstractPluginManager::pluginList() const {
    vector<string> names;
    for(map<string, PluginObject*>::const_iterator i = plugins()->begin(); i != plugins()->end(); ++i) {

        /* Plugin doesn't belong to this manager */
        if(i->second->manager != this) continue;

        names.push_back(i->first);
    }
    return names;
}

const PluginMetadata* AbstractPluginManager::metadata(const string& plugin) const {
    map<string, PluginObject*>::const_iterator found = plugins()->find(plugin);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return 0;

    /* Plugin doesn't belong to this manager */
    if(found->second->manager != this) return 0;

    return &found->second->metadata;
}

AbstractPluginManager::LoadState AbstractPluginManager::loadState(const std::string& plugin) const {
    map<string, PluginObject*>::const_iterator found = plugins()->find(plugin);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    /* Plugin doesn't belong to this manager */
    if(found->second->manager != this) return NotFound;

    return found->second->loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::load(const string& _plugin) {
    map<string, PluginObject*>::iterator found = plugins()->find(_plugin);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    /* Plugin doesn't belong to this manager */
    if(found->second->manager != this) return NotFound;

    /* Before loading reload its metadata and if it is not found,
        remove it from the list */
    if(!reloadPluginMetadata(found)) {
        delete found->second;
        plugins()->erase(found);
        return NotFound;
    }

    PluginObject& plugin = *found->second;

    /* Plugin is not ready to load */
    if(!(plugin.loadState & (NotLoaded)))
        return plugin.loadState;

    /* Vector of found dependencies. If everything goes well, this plugin will
       be added to each dependency usedBy list. */
    vector<PluginObject*> dependencies;

    /* Load dependencies and add this plugin to their "used by" list */
    for(vector<string>::const_iterator it = plugin.metadata.depends().begin(); it != plugin.metadata.depends().end(); ++it) {
        /* Find manager which is associated to this plugin and load the plugin
           with it */
        map<string, PluginObject*>::iterator found = plugins()->find(*it);

        if(found == plugins()->end() || !found->second->manager || !(found->second->manager->load(*it) & (LoadOk|IsStatic)))
            return UnresolvedDependency;

        dependencies.push_back(found->second);
    }

    string filename = Directory::join(_pluginDirectory, _plugin + PLUGIN_FILENAME_SUFFIX);

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    #ifndef _WIN32
    void* handle = dlopen(filename.c_str(), RTLD_NOW|RTLD_GLOBAL);
    #else
    HMODULE handle = LoadLibraryA(filename.c_str());
    #endif
    if(!handle) {
        cerr << "Cannot open plugin file \""
             << _pluginDirectory + _plugin + PLUGIN_FILENAME_SUFFIX
             << "\": " << dlerror() << endl;
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Check plugin version */

    /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    #ifdef __GNUC__
    __extension__
    #endif
    int (*_version)(void) = reinterpret_cast<int(*)()>(dlsym(handle, "pluginVersion"));
    if(_version == 0) {
        cerr << "Cannot get version of plugin '" << _plugin << "': " << dlerror() << endl;
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }
    if(_version() != version) {
        dlclose(handle);
        plugin.loadState = WrongPluginVersion;
        return plugin.loadState;
    }

    /* Check interface string */

    /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    #ifdef __GNUC__
    __extension__
    #endif
    string (*interface)() = reinterpret_cast<string (*)()>(dlsym(handle, "pluginInterface"));
    if(interface == 0) {
        cerr << "Cannot get interface string of plugin '" << _plugin << "': " << dlerror() << endl;
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }
    if(interface() != pluginInterface()) {
        dlclose(handle);
        plugin.loadState = WrongInterfaceVersion;
        return plugin.loadState;
    }

    /* Load plugin instancer */

    /* http://www.mr-edd.co.uk/blog/supressing_gcc_warnings */
    #ifdef __GNUC__
    __extension__
    #endif
    void* (*instancer)(AbstractPluginManager*, const std::string&) = reinterpret_cast<void* (*)(AbstractPluginManager*, const std::string&)>(dlsym(handle, "pluginInstancer"));
    if(instancer == 0) {
        cerr << "Cannot get instancer of plugin '" << _plugin << "': " << dlerror() << endl;
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Everything is okay, add this plugin to usedBy list of each dependency */
    for(vector<PluginObject*>::const_iterator it = dependencies.begin(); it != dependencies.end(); ++it)
        (*it)->metadata.addUsedBy(_plugin);

    plugin.loadState = LoadOk;
    plugin.module = handle;
    plugin.instancer = instancer;
    return plugin.loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::unload(const string& _plugin) {
    map<string, PluginObject*>::iterator found = plugins()->find(_plugin);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    PluginObject& plugin = *found->second;

    /* Plugin doesn't belong to this manager */
    if(plugin.manager != this) return NotFound;

    /* Only unload loaded plugin */
    if(plugin.loadState & (LoadOk|UnloadFailed)) {
        /* Plugin has active instance, don't unload */
        if(instances.find(_plugin) != instances.end()) return IsUsed;

        /* Plugin is used by another plugin, don't unload */
        if(!plugin.metadata.usedBy().empty()) return IsRequired;

        /* Remove this plugin from "used by" column of dependencies */
        for(vector<string>::const_iterator it = plugin.metadata.depends().begin(); it != plugin.metadata.depends().end(); ++it)
            plugins()->find(*it)->second->metadata.removeUsedBy(_plugin);

        #ifndef _WIN32
        if(dlclose(plugin.module) != 0) {
        #else
        if(!FreeLibrary(plugin.module)) {
        #endif
            cerr << "Cannot unload plugin '" << _plugin << "': " << dlerror() << endl;
            plugin.loadState = UnloadFailed;
            return plugin.loadState;
        }

        plugin.loadState = NotLoaded;
    }

    /* After successful unload, reload its metadata and if it is not found,
        remove it from the list */
    if(!reloadPluginMetadata(found)) {
        delete found->second;
        plugins()->erase(found);
        return NotLoaded;
    }

    /* Return directly the load state, as 'plugin' reference was deleted by
        reloadPluginMetadata() */
    return found->second->loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::reload(const std::string& plugin) {
    /* If the plugin is not loaded, just reload its metadata */
    if(loadState(plugin) == NotLoaded) {
        map<string, PluginObject*>::iterator found = plugins()->find(plugin);

        /* If the plugin is not found, remove it from the list */
        if(!reloadPluginMetadata(found)) {
            delete found->second;
            plugins()->erase(found);
        }

        return NotLoaded;

    /* Else try unload and load */
    } else {
        LoadState l = unload(plugin);
        if(l != NotLoaded) return l;
        return load(plugin);
    }
}

void AbstractPluginManager::registerInstance(const string& plugin, Plugin* instance, const Configuration** configuration, const PluginMetadata** metadata) {
    map<string, PluginObject*>::const_iterator foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    /* Plugin doesn't belong to this manager, nothing to do */
    if(foundPlugin->second->manager != this) return;

    map<string, vector<Plugin*> >::iterator foundInstance = instances.find(plugin);

    if(foundInstance == instances.end()) {
        foundInstance = instances.insert(pair<string, vector<Plugin* > >
            (plugin, vector<Plugin*>())).first;
    }

    foundInstance->second.push_back(instance);

    *configuration = &foundPlugin->second->configuration;
    *metadata = &foundPlugin->second->metadata;
}

void AbstractPluginManager::unregisterInstance(const string& plugin, Plugin* instance) {
    map<string, PluginObject*>::const_iterator foundPlugin = plugins()->find(plugin);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    /* Plugin doesn't belong to this manager, nothing to do */
    if(foundPlugin->second->manager != this) return;

    map<string, vector<Plugin*> >::iterator foundInstance = instances.find(plugin);
    if(foundInstance == instances.end()) return;
    vector<Plugin*>& _instances = foundInstance->second;

    vector<Plugin*>::iterator pos = find(_instances.begin(), _instances.end(), instance);
    if(pos == _instances.end()) return;

    _instances.erase(pos);

    if(_instances.size() == 0) instances.erase(plugin);
}

bool AbstractPluginManager::reloadPluginMetadata(map<string, PluginObject*>::iterator it) {
    /* Don't reload metadata of alien or loaded plugins */
    if(it->second->manager != this || (it->second->loadState & (LoadOk|IsStatic)))
        return true;

    /* If plugin binary doesn't exist, schedule the entry for deletion */
    if(!Directory::fileExists(Directory::join(_pluginDirectory, it->first + PLUGIN_FILENAME_SUFFIX)))
        return false;

    /* Reload plugin metadata */
    delete it->second;
    it->second = new PluginObject(Directory::join(_pluginDirectory, it->first + ".conf"), this);

    return true;
}

} namespace Utility {

using namespace PluginManager;

Debug& operator<<(Debug debug, AbstractPluginManager::LoadState value) {
    string _value;
    switch(value) {
        #define ls(state) case AbstractPluginManager::state: _value = #state; break;
        ls(NotFound)
        ls(WrongPluginVersion)
        ls(WrongInterfaceVersion)
        ls(WrongMetadataFile)
        ls(UnresolvedDependency)
        ls(LoadFailed)
        ls(LoadOk)
        ls(NotLoaded)
        ls(UnloadFailed)
        ls(IsRequired)
        ls(IsStatic)
        ls(IsUsed)
        #undef ls

        default:
            ostringstream o;
            o << value;
            _value = o.str();
    }

    return debug << "LoadState(" + _value + ')';
}

}}
