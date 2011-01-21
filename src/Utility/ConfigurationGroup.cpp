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

#include "ConfigurationGroup.h"

#include "Configuration.h"

using namespace std;

namespace Kompas { namespace Utility {

ConfigurationGroup::ConfigurationGroup(const std::string& name, Configuration* _configuration): configuration(_configuration) {
    if(name.find('/') != string::npos)
        throw string("Slash in group name!");

    _name = name;
}

ConfigurationGroup::~ConfigurationGroup() {
    for(vector<ConfigurationGroup*>::iterator it = _groups.begin(); it != _groups.end(); ++it)
        delete *it;
}

ConfigurationGroup* ConfigurationGroup::group(const string& name, unsigned int number) {
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup*>::iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if((*it)->name() == name && foundNumber++ == number)
            return *it;
    }

    /* Automatic group creation is enabled and user wants first group,
        try to create new group */
    if((configuration->flags & Configuration::AutoCreateGroups) && number == 0) return addGroup(name);

    return 0;
}

const ConfigurationGroup* ConfigurationGroup::group(const string& name, unsigned int number) const {
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup*>::const_iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if((*it)->name() == name && foundNumber++ == number)
            return *it;
    }

    return 0;
}

vector<ConfigurationGroup*> ConfigurationGroup::groups(const string& name) {
    if(name.empty()) return _groups;

    vector<ConfigurationGroup*> found;

    for(vector<ConfigurationGroup*>::iterator it = _groups.begin(); it != _groups.end(); ++it)
        if((*it)->name() == name) found.push_back(*it);

    return found;
}

vector<const ConfigurationGroup*> ConfigurationGroup::groups(const string& name) const {
    vector<const ConfigurationGroup*> found;

    for(vector<ConfigurationGroup*>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
        if(name.empty() || (*it)->name() == name) found.push_back(*it);

    return found;
}

ConfigurationGroup* ConfigurationGroup::addGroup(const std::string& name) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid)) return 0;

    /* Name must not be empty and must not contain slash character */
    if(name.empty() || name.find('/') != string::npos) return 0;

    /* Check for unique groups */
    if(configuration->flags & Configuration::UniqueGroups) {
        for(vector<ConfigurationGroup*>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
            if((*it)->name() == name) return 0;
    }

    configuration->flags |= Configuration::Changed;

    ConfigurationGroup* g = new ConfigurationGroup(name, configuration);
    _groups.push_back(g);
    return g;
}

bool ConfigurationGroup::removeGroup(const std::string& name, unsigned int number) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid)) return false;

    /* Global group cannot be removed */
    if(name == "") return false;

    /* Find group with given number and name */
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup*>::iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if((*it)->name() == name && foundNumber++ == number) {
            _groups.erase(it);
            configuration->flags |= Configuration::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeGroup(ConfigurationGroup* group) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid)) return false;

    for(vector<ConfigurationGroup*>::iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if(*it == group) {
            _groups.erase(it);
            configuration->flags |= Configuration::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeAllGroups(const std::string& name) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid)) return false;

    /* Global group cannot be removed */
    if(name == "") return false;

    for(int i = _groups.size()-1; i >= 0; --i) {
        if(_groups[i]->name() == name) _groups.erase(_groups.begin()+i);
    }

    configuration->flags |= Configuration::Changed;
    return true;
}

unsigned int ConfigurationGroup::keyCount(const string& key) const {
    unsigned int count = 0;
    for(vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
        if(it->key == key) count++;

    return count;
}

bool ConfigurationGroup::keyExists(const std::string& key) const {
    for(vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
        if(it->key == key) return true;

    return false;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> vector<string> ConfigurationGroup::values(const string& key, int flags) const {
    vector<string> found;

    for(vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
        if(it->key == key) found.push_back(it->value);

    return found;
}

template<> bool ConfigurationGroup::setValue(const string& key, const string& value, unsigned int number, int flags) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid))
        return false;

    /* Key cannot be empty => this would change comments / empty lines */
    if(key.empty()) return false;

    unsigned int foundNumber = 0;
    for(vector<Item>::iterator it = items.begin(); it != items.end(); ++it) {
        if(it->key == key && foundNumber++ == number) {
            it->value = value;
            configuration->flags |= Configuration::Changed;
            return true;
        }
    }

    /* No value with that name was found, add new */
    Item i;
    i.key = key;
    i.value = value;
    items.push_back(i);

    configuration->flags |= Configuration::Changed;
    return true;
}

template<> bool ConfigurationGroup::addValue(const string& key, const string& value, int flags) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid))
        return false;

    /* Key cannot be empty => empty keys are treated as comments / empty lines */
    if(key.empty()) return false;

    /* Check for unique keys */
    if(configuration->flags & Configuration::UniqueKeys) {
        for(vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it)
            if(it->key == key) return false;
    }

    Item i;
    i.key = key;
    i.value = value;
    items.push_back(i);

    configuration->flags |= Configuration::Changed;
    return true;
}

template<> bool ConfigurationGroup::value(const string& key, string* value, unsigned int number, int flags) {
    const ConfigurationGroup* c = this;
    if(c->value(key, value, number, flags)) return true;

    /* Automatic key/value pair creation is enabled and user wants first key,
        try to create new key/value pair */
    if((configuration->flags & Configuration::AutoCreateKeys) && number == 0)
        return setValue<string>(key, *value, flags);

    return false;
}
template<> bool ConfigurationGroup::value(const string& key, string* value, unsigned int number, int flags) const {
    unsigned int foundNumber = 0;
    for(vector<Item>::const_iterator it = items.begin(); it != items.end(); ++it) {
        if(it->key == key) {
            if(foundNumber++ == number) {
                *value = it->value;
                return true;
            }
        }
    }

    return false;
}
#endif

bool ConfigurationGroup::removeValue(const string& key, unsigned int number) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid))
        return false;

    /* Key cannot be empty => empty keys are treated as comments / empty lines */
    if(key.empty()) return false;

    unsigned int foundNumber = 0;
    for(vector<Item>::iterator it = items.begin(); it != items.end(); ++it) {
        if(it->key == key && foundNumber++ == number) {
            items.erase(it);
            configuration->flags |= Configuration::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeAllValues(const std::string& key) {
    if(configuration->flags & Configuration::ReadOnly || !(configuration->flags & Configuration::IsValid))
        return false;

    /** @todo Do it better & faster */
    for(int i = items.size()-1; i >= 0; --i) {
        if(items[i].key == key) items.erase(items.begin()+i);
    }

    configuration->flags |= Configuration::Changed;
    return true;
}

#ifndef DOXYGEN_GENERATING_OUTPUT

bool ConfigurationValue<bool>::fromString(const std::string& value, int flags) {
    if(value == "1" || value == "yes" || value == "y" || value == "true") return true;
    return false;
}

std::string ConfigurationValue<bool>::toString(const bool& value, int flags) {
    if(value) return "true";
    return "false";
}

#endif

}}
