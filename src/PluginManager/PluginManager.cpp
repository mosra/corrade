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

#define Map2X_PluginManager_PluginManager_cpp

#include "PluginManager.h"

#include <algorithm>
#include <dlfcn.h>

#include "Utility/Directory.h"

using namespace std;
using Map2X::Utility::Directory;

namespace Map2X { namespace PluginManager {

/** @todo Support for other systems' module filenames (Mac OS, Symbian at least) */
#ifdef _WIN32
#define PLUGIN_FILENAME_PREFIX ""
#define PLUGIN_FILENAME_SUFFIX ".dll"
#else /* linux, bsd */
#define PLUGIN_FILENAME_PREFIX "lib"
#define PLUGIN_FILENAME_SUFFIX ".so"
#endif

template<class T> PluginManager<T>::PluginManager(const string& _pluginDirectory): pluginDirectory(_pluginDirectory) {
    /* Load static plugins which use the same API */
    for(vector<StaticPlugin>::const_iterator i = staticPlugins.begin(); i != staticPlugins.end(); ++i) {
        Plugin p;
        p.loadState = IsStatic;
        p.instancer = i->instancer;
        i->metadataCreator(&p.metadata);

        if(p.metadata.interface == T::pluginInterface())
            plugins.insert(pair<string, Plugin>(i->name, p));
    }

    /* Plugin directory contents */
    Directory d(_pluginDirectory, Directory::SkipDirectories|Directory::SkipSpecial);

    /* Get all dynamic plugin filenames */
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

            Plugin p;
            p.loadState = Unknown;
            plugins.insert(pair<string, Plugin>(name, p));
        }
    }
}

template<class T> vector<string> PluginManager<T>::nameList() const {
    vector<string> names;
    for(map<string, Plugin>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        names.push_back(i->first);
    return names;
}

template<class T> void PluginManager<T>::loadAll() {
    for(map<string, Plugin>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        load(i->first);
}

template<class T> const PluginMetadata* PluginManager<T>::metadata(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return 0;

    /* If plugin was not yet loaded, try to load it */
    if(plugins.at(name).loadState == NotLoaded)
        load(name);

    return &plugins.at(name).metadata;
}

template<class T> typename PluginManager<T>::LoadState PluginManager<T>::loadState(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    return plugins.at(name).loadState;
}

template<class T> typename PluginManager<T>::LoadState PluginManager<T>::load(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    Plugin& plugin = plugins.at(name);

    /* Plugin is already loaded or is static */
    if(plugin.loadState & (LoadOk|UnloadFailed|IsStatic))
        return plugin.loadState;

    /* Open plugin file, make symbols available for next libs (which depends on this) */
    /** @todo Portable directory separator or plugindir with separator */
    void* handle = dlopen((pluginDirectory + PLUGIN_FILENAME_PREFIX + name + PLUGIN_FILENAME_SUFFIX).c_str(),
                          RTLD_NOW|RTLD_GLOBAL);
    if(!handle) {
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Check plugin version */
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
    void (*metadataCreator)(PluginMetadata*) = reinterpret_cast<void (*)(PluginMetadata*)>(dlsym(handle, "pluginMetadataCreator"));
    if(metadataCreator == 0) {
        dlclose(handle);
        plugin.loadState = LoadFailed;
        return plugin.loadState;
    }

    /* Load metadata and check interface version */
    metadataCreator(&plugin.metadata);
    if(plugin.metadata.interface != T::pluginInterface()) {
        dlclose(handle);
        plugin.loadState = WrongInterfaceVersion;
        return plugin.loadState;
    }

    /* Load plugin instancer */
    void* (*instancer)() = reinterpret_cast<void* (*)()>(dlsym(handle, "pluginInstancer"));
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

template<class T> typename PluginManager<T>::LoadState PluginManager<T>::unload(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    Plugin& plugin = plugins.at(name);

    /* Plugin is not loaded or is static, nothing to do */
    if(!(plugin.loadState & (LoadOk|UnloadFailed|IsStatic))) return plugin.loadState;

    if(dlclose(plugin.handle) != 0) {
        plugin.loadState = UnloadFailed;
        return plugin.loadState;
    }

    plugin.loadState = NotLoaded;
    return plugin.loadState;
}

template<class T> T* PluginManager<T>::instance(const std::string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return 0;

    Plugin& plugin = plugins.at(name);

    /* Plugin is not successfully loaded */
    if(!(plugin.loadState & (LoadOk|IsStatic))) return 0;

    return static_cast<T*>(plugin.instancer());
}

}}
