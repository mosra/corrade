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

#include "PluginMetadata.h"

using namespace std;
using namespace Map2X::Utility;

namespace Map2X { namespace PluginManager {

PluginMetadata::PluginMetadata(const Configuration& conf) {
    /* Dependencies, replacements */
    _depends = conf.values<string>("depends");
    _replaces = conf.values<string>("replaces");

    /* Metadata */
    const ConfigurationGroup* metadata = conf.group("metadata");
    if(!metadata) return;

    /* Original name */
    if(metadata->valueExists("name"))
        names.insert(pair<string, string>("", metadata->value<string>("name")));

    /* Original description */
    if(metadata->valueExists("description"))
        descriptions.insert(pair<string, string>("", metadata->value<string>("description")));

    /* Translated names and descriptions */
    vector<const ConfigurationGroup*> tr = metadata->groups();
    for(vector<const ConfigurationGroup*>::const_iterator it = tr.begin(); it != tr.end(); ++it) {
        if((*it)->valueExists("name"))
            names.insert(pair<string, string>((*it)->name(), (*it)->value<string>("name")));

        if((*it)->valueExists("description"))
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

}}
