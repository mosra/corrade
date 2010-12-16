/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

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
#include <dlfcn.h>

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

void AbstractPluginManager::importStaticPlugin(const string& name, int _version, const std::string& interface, void* (*instancer)(AbstractPluginManager*, const std::string&)) {
    if(_version != version) return;

    /* Load static plugin metadata */
    Resource r("plugins");
    std::istringstream metadata(r.get(name + ".conf"));

    /* Insert plugin to list */
    plugins()->insert(pair<string, PluginObject*>(name, new PluginObject(metadata, interface, instancer)));
}

AbstractPluginManager::AbstractPluginManager(const string& pluginDirectory): _pluginDirectory(pluginDirectory) {
    /* Foreach all files in plugin directory */
    Directory d(_pluginDirectory, Directory::SkipDirectories|Directory::SkipSpecial);
    for(Directory::const_iterator i = d.begin(); i != d.end(); ++i) {

        /* Search for module filename prefix and suffix in current file */
        size_t begin;
        if(!string(PLUGIN_FILENAME_PREFIX).empty())
            begin = (*i).find(PLUGIN_FILENAME_PREFIX);
        else
            begin = 0;
        size_t end = (*i).find(PLUGIN_FILENAME_SUFFIX);

        /* File is not plugin, continue to next */
        if(begin != 0 || end == string::npos) continue;

        /* Dig plugin name from filename */
        string name = (*i).substr(begin+string(PLUGIN_FILENAME_PREFIX).size(),
                                  end-string(PLUGIN_FILENAME_PREFIX).size());

        /* Insert plugin to list */
        plugins()->insert(pair<string, PluginObject*>(name, new PluginObject(Directory::join(pluginDirectory, name + ".conf"), this)));
    }
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

vector<string> AbstractPluginManager::nameList() const {
    vector<string> names;
    for(map<string, PluginObject*>::const_iterator i = plugins()->begin(); i != plugins()->end(); ++i) {

        /* Plugin doesn't belong to this manager */
        if(i->second->manager != this) continue;

        names.push_back(i->first);
    }
    return names;
}

const PluginMetadata* AbstractPluginManager::metadata(const string& name) const {
    map<string, PluginObject*>::const_iterator found = plugins()->find(name);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return 0;

    /* Plugin doesn't belong to this manager */
    if(found->second->manager != this) return 0;

    return &found->second->metadata;
}

AbstractPluginManager::LoadState AbstractPluginManager::loadState(const std::string& name) const {
    map<string, PluginObject*>::const_iterator found = plugins()->find(name);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    /* Plugin doesn't belong to this manager */
    if(found->second->manager != this) return NotFound;

    return found->second->loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::load(const string& name) {
    map<string, PluginObject*>::iterator found = plugins()->find(name);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    PluginObject& plugin = *found->second;

    /* Plugin doesn't belong to this manager */
    if(plugin.manager != this) return NotFound;

    /* Plugin is not ready to load */
    if(!(plugin.loadState & (NotLoaded)))
        return plugin.loadState;

    /* Load dependencies and add this plugin to their "used by" list */
    for(vector<string>::const_iterator it = plugin.metadata.depends().begin(); it != plugin.metadata.depends().end(); ++it) {
        /* Find manager which is associated to this plugin and load the plugin
           with it */
        map<string, PluginObject*>::iterator found = plugins()->find(*it);
        if(found == plugins()->end() || !found->second->manager || !(found->second->manager->load(*it) & (LoadOk|IsStatic)))
            return UnresolvedDependency;

        found->second->metadata.addUsedBy(name);
    }

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    void* handle = dlopen(Directory::join(_pluginDirectory, PLUGIN_FILENAME_PREFIX + name + PLUGIN_FILENAME_SUFFIX).c_str(),
                          RTLD_NOW|RTLD_GLOBAL);
    if(!handle) {
        cerr << "Cannot open plugin file \""
             << _pluginDirectory + PLUGIN_FILENAME_PREFIX + name + PLUGIN_FILENAME_SUFFIX
             << "\": " << dlerror() << endl;
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Check plugin version */
    int (*_version)(void) = reinterpret_cast<int(*)()>(dlsym(handle, "pluginVersion"));
    if(_version == 0) {
        cerr << "Cannot get version of plugin '" << name << "': " << dlerror() << endl;
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
    string (*interface)() = reinterpret_cast<string (*)()>(dlsym(handle, "pluginInterface"));
    if(interface == 0) {
        cerr << "Cannot get interface string of plugin '" << name << "': " << dlerror() << endl;
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
    void* (*instancer)(AbstractPluginManager*, const std::string&) = reinterpret_cast<void* (*)(AbstractPluginManager*, const std::string&)>(dlsym(handle, "pluginInstancer"));
    if(instancer == 0) {
        cerr << "Cannot get instancer of plugin '" << name << "': " << dlerror() << endl;
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    plugin.loadState = LoadOk;
    plugin.module = handle;
    plugin.instancer = instancer;
    return plugin.loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::unload(const string& name) {
    map<string, PluginObject*>::iterator found = plugins()->find(name);

    /* Plugin with given name doesn't exist */
    if(found == plugins()->end()) return NotFound;

    PluginObject& plugin = *found->second;

    /* Plugin doesn't belong to this manager */
    if(plugin.manager != this) return NotFound;

    /* Plugin is not loaded or is static, nothing to do */
    if(plugin.loadState & ~(LoadOk|UnloadFailed)) return plugin.loadState;

    /* Plugin has active instance, don't unload */
    if(instances.find(name) != instances.end()) return IsUsed;

    /* Plugin is used by another plugin, don't unload */
    if(!plugin.metadata.usedBy().empty()) return IsRequired;

    /* Remove this plugin from "used by" column of dependencies */
    for(vector<string>::const_iterator it = plugin.metadata.depends().begin(); it != plugin.metadata.depends().end(); ++it)
        plugins()->find(*it)->second->metadata.removeUsedBy(name);

    if(dlclose(plugin.module) != 0) {
        cerr << "Cannot unload plugin '" << name << "': " << dlerror() << endl;
        plugin.loadState = UnloadFailed;
        return plugin.loadState;
    }

    plugin.loadState = NotLoaded;
    return plugin.loadState;
}

void AbstractPluginManager::registerInstance(const string& name, Plugin* instance, const Configuration** configuration, const PluginMetadata** metadata) {
    map<string, PluginObject*>::const_iterator foundPlugin = plugins()->find(name);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    /* Plugin doesn't belong to this manager, nothing to do */
    if(foundPlugin->second->manager != this) return;

    map<string, vector<Plugin*> >::iterator foundInstance = instances.find(name);

    if(foundInstance == instances.end()) {
        foundInstance = instances.insert(pair<string, vector<Plugin* > >
            (name, vector<Plugin*>())).first;
    }

    foundInstance->second.push_back(instance);

    *configuration = &foundPlugin->second->configuration;
    *metadata = &foundPlugin->second->metadata;
}

void AbstractPluginManager::unregisterInstance(const string& name, Plugin* instance) {
    map<string, PluginObject*>::const_iterator foundPlugin = plugins()->find(name);

    /* Given plugin doesn't exist, nothing to do */
    if(foundPlugin == plugins()->end()) return;

    /* Plugin doesn't belong to this manager, nothing to do */
    if(foundPlugin->second->manager != this) return;

    /** @todo Emit error when unregistering nonexistent instance */
    map<string, vector<Plugin*> >::iterator foundInstance = instances.find(name);
    if(foundInstance == instances.end()) return;
    vector<Plugin*>& _instances = foundInstance->second;

    vector<Plugin*>::iterator pos = find(_instances.begin(), _instances.end(), instance);
    if(pos == _instances.end()) return;

    _instances.erase(pos);

    if(_instances.size() == 0) instances.erase(name);
}

}}
