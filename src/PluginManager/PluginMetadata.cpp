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

#include "PluginMetadata.h"

using namespace std;
using namespace Kompas::Utility;

namespace Kompas { namespace PluginManager {

PluginMetadata::PluginMetadata(const Configuration& conf) {
    /* Author(s), version */
    _authors = conf.values<string>("author");
    _version = conf.value<string>("version");

    /* Dependencies, replacements */
    _depends = conf.values<string>("depends");
    _replaces = conf.values<string>("replaces");

    /* Metadata */
    const ConfigurationGroup* metadata = conf.group("metadata");
    if(!metadata) return;

    /* Original name */
    if(metadata->keyExists("name"))
        names.insert(pair<string, string>("", metadata->value<string>("name")));

    /* Original description */
    if(metadata->keyExists("description"))
        descriptions.insert(pair<string, string>("", metadata->value<string>("description")));

    /* Translated names and descriptions */
    vector<const ConfigurationGroup*> tr = metadata->groups();
    for(vector<const ConfigurationGroup*>::const_iterator it = tr.begin(); it != tr.end(); ++it) {
        if((*it)->keyExists("name"))
            names.insert(pair<string, string>((*it)->name(), (*it)->value<string>("name")));

        if((*it)->keyExists("description"))
            descriptions.insert(pair<string, string>((*it)->name(), (*it)->value<string>("description")));
    }
}

std::string PluginMetadata::name(const std::string& language) const {
    /* Fallback to original name or return empty string */
    if(names.find(language) == names.end()) {
        if(language.empty()) return "";
        else return name("");
    }

    return names.at(language);
}

std::string PluginMetadata::description(const std::string& language) const {
    /* Fallback to original description or return empty string */
    if(descriptions.find(language) == descriptions.end()) {
        if(language.empty()) return "";
        else return description("");
    }

    return descriptions.at(language);
}

void PluginMetadata::addUsedBy(const std::string& name) {
    for(vector<string>::const_iterator it = _usedBy.begin(); it != _usedBy.end(); ++it)
        if(*it == name) return;

    _usedBy.push_back(name);
}

void PluginMetadata::removeUsedBy(const std::string& name) {
    for(vector<string>::iterator it = _usedBy.begin(); it != _usedBy.end(); ++it) {
        if(*it == name) {
            _usedBy.erase(it);
            return;
        }
    }
}

}}
