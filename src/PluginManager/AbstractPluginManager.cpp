/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "AbstractPluginManager.h"

#include <algorithm>
#include <dlfcn.h>

#include "PluginManager/AbstractPluginManagerConfigure.h"
#include "Plugin.h"
#include "Utility/Directory.h"

using namespace std;
using namespace Map2X::Utility;

namespace Map2X { namespace PluginManager {

#ifndef DOXYGEN_GENERATING_OUTPUT
vector<AbstractPluginManager::StaticPlugin> AbstractPluginManager::staticPlugins;
#endif

void AbstractPluginManager::importStaticPlugin(const string& name, int _version, void (*metadataCreator)(PluginMetadata*), void* (*instancer)(AbstractPluginManager*, const std::string&)) {
    if(_version != version) return;

    StaticPlugin p;
    p.name = name;
    p.metadataCreator = metadataCreator;
    p.instancer = instancer;
    staticPlugins.push_back(p);
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

        /* If found, add plugin filename part to list */
        if(begin == 0 && end != string::npos) {
            string name = (*i).substr(begin+string(PLUGIN_FILENAME_PREFIX).size(), end-string(PLUGIN_FILENAME_PREFIX).size());

            PluginObject p;
            p.loadState = Unknown;
            plugins.insert(pair<string, PluginObject>(name, p));
        }
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

    /* Unload all plugins */
    /** @todo Checking load state whether unload succeeded? */
    for(map<string, PluginObject>::const_iterator it = plugins.begin(); it != plugins.end(); ++it)
        /** @todo unload() tests whether plugin exists => performance-- */
        unload(it->first);
}

vector<string> AbstractPluginManager::nameList() const {
    vector<string> names;
    for(map<string, PluginObject>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        names.push_back(i->first);
    return names;
}

void AbstractPluginManager::loadAll() {
    for(map<string, PluginObject>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        load(i->first);
}

const PluginMetadata* AbstractPluginManager::metadata(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return 0;

    /* If plugin was not yet loaded, try to load it */
    if(plugins.at(name).loadState == Unknown)
        load(name);

    return &plugins.at(name).metadata;
}

AbstractPluginManager::LoadState AbstractPluginManager::load(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    PluginObject& plugin = plugins.at(name);

    /* Plugin is already loaded or is static */
    if(plugin.loadState & (LoadOk|UnloadFailed|IsStatic))
        return plugin.loadState;

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    /** @todo Portable directory separator or plugindir with separator */
    void* handle = dlopen((_pluginDirectory + PLUGIN_FILENAME_PREFIX + name + PLUGIN_FILENAME_SUFFIX).c_str(),
                          RTLD_NOW|RTLD_GLOBAL);
    if(!handle) {
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Check plugin version */
    /** @todo Check whether the symbol was found! */
    int (*_version)(void) = reinterpret_cast<int(*)()>(dlsym(handle, "pluginVersion"));
    if(_version() == 0) {
        dlclose(handle);
        plugin.loadState = WrongPluginVersion;
        return plugin.loadState;
    }
    if(_version() != version) {
        dlclose(handle);
        plugin.loadState = WrongPluginVersion;
        return plugin.loadState;
    }

    /* Pointer to metadata creator */
    /** @todo Check whether the symbol was found! */
    void (*metadataCreator)(PluginMetadata*) = reinterpret_cast<void (*)(PluginMetadata*)>(dlsym(handle, "pluginMetadataCreator"));
    if(metadataCreator == 0) {
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Load metadata and check interface version */
    metadataCreator(&plugin.metadata);
    if(plugin.metadata.interface != pluginInterface()) {
        dlclose(handle);
        plugin.loadState = WrongInterfaceVersion;
        return plugin.loadState;
    }

    /* Load plugin instancer */
    /** @todo Check whether the symbol was found! */
    void* (*instancer)(AbstractPluginManager*, const std::string&) = reinterpret_cast<void* (*)(AbstractPluginManager*, const std::string&)>(dlsym(handle, "pluginInstancer"));
    if(instancer == 0) {
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    plugin.loadState = LoadOk;
    plugin.handle = handle;
    plugin.instancer = instancer;
    return plugin.loadState;
}

AbstractPluginManager::LoadState AbstractPluginManager::unload(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    PluginObject& plugin = plugins.at(name);

    /* Plugin is not loaded or is static, nothing to do */
    if(plugin.loadState & ~(LoadOk|UnloadFailed)) return plugin.loadState;

    /* Plugin has active instance, don't unload */
    if(instances.find(name) != instances.end()) return IsUsed;

    if(dlclose(plugin.handle) != 0) {
        plugin.loadState = UnloadFailed;
        return plugin.loadState;
    }

    plugin.loadState = NotLoaded;
    return plugin.loadState;
}

void AbstractPluginManager::registerInstance(const string& name, Plugin* instance) {
    /* Given plugin doesn't exist or is static (static plugins cannot be unloaded
        and thus instance counting is useless), nothing to do */
    if(plugins.find(name) == plugins.end() || plugins[name].loadState == IsStatic) return;

    if(instances.find(name) == instances.end()) {
        instances.insert(pair<string, vector<Plugin* > >
            (name, vector<Plugin*>()));
    }

    instances[name].push_back(instance);
}

void AbstractPluginManager::unregisterInstance(const string& name, Plugin* instance) {
    /* Given plugin doesn't exist or is static (static plugins cannot be unloaded
        and thus instance counting is useless), nothing to do */
    if(plugins.find(name) == plugins.end() || plugins[name].loadState == IsStatic) return;

    /** @todo Emit error when unregistering nonexistent instance */
    if(instances.find(name) == instances.end()) return;

    vector<Plugin*>::iterator pos = find(instances[name].begin(), instances[name].end(), instance);
    if(pos == instances[name].end()) return;

    instances[name].erase(pos);

    if(instances[name].size() == 0) instances.erase(name);
}

}}
