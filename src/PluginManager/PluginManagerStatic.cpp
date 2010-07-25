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

#include "PluginManagerStatic.h"

using namespace std;

namespace Map2X { namespace PluginManager {

#ifndef DOXYGEN_GENERATING_OUTPUT
vector<PluginManagerStatic::StaticPlugin> PluginManagerStatic::staticPlugins;
#endif

void PluginManagerStatic::importStaticPlugin(const string& name, int _version, void (*metadataCreator)(PluginMetadata*), void* (*instancer)()) {
    if(_version != version) return;

    StaticPlugin p;
    p.name = name;
    p.metadataCreator = metadataCreator;
    p.instancer = instancer;
    staticPlugins.push_back(p);
}

vector<string> PluginManagerStatic::nameList() const {
    vector<string> names;
    for(map<string, Plugin>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        names.push_back(i->first);
    return names;
}

void PluginManagerStatic::loadAll() {
    for(map<string, Plugin>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
        load(i->first);
}

const PluginMetadata* PluginManagerStatic::metadata(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return 0;

    /* If plugin was not yet loaded, try to load it */
    if(plugins.at(name).loadState == Unknown)
        load(name);

    return &plugins.at(name).metadata;
}

PluginManagerStatic::LoadState PluginManagerStatic::loadState(const string& name) {
    /* Plugin with given name doesn't exist */
    if(plugins.find(name) == plugins.end()) return NotFound;

    return plugins.at(name).loadState;
}

}}
