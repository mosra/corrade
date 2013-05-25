/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
              Vladimír Vondruš <mosra@centrum.cz>

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

#include "ConfigurationGroup.h"

#include "Configuration.h"
#include "Debug.h"

namespace Corrade { namespace Utility {

ConfigurationGroup::ConfigurationGroup(const ConfigurationGroup& other): items(other.items), _groups(other._groups) {
    /* Deep copy groups */
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        it->group = new ConfigurationGroup(*it->group);
}

ConfigurationGroup& ConfigurationGroup::operator=(const ConfigurationGroup& other) {
    if(&other == this)
        return *this;

    /* Delete current groups */
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        delete it->group;

    items.assign(other.items.begin(), other.items.end());
    _groups.assign(other._groups.begin(), other._groups.end());

    /* Deep copy groups */
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        it->group = new ConfigurationGroup(*it->group);

    return *this;
}

ConfigurationGroup::~ConfigurationGroup() {
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        delete it->group;
}

ConfigurationGroup* ConfigurationGroup::group(const std::string& name, unsigned int number) {
    unsigned int foundNumber = 0;
    for(auto it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->name == name && foundNumber++ == number)
            return it->group;
    }

    /* Automatic group creation is enabled and user wants first group,
        try to create new group */
    if((configuration->flags & Configuration::InternalFlag::AutoCreateGroups) && number == 0) return addGroup(name);

    return nullptr;
}

const ConfigurationGroup* ConfigurationGroup::group(const std::string& name, unsigned int number) const {
    unsigned int foundNumber = 0;
    for(auto it = _groups.cbegin(); it != _groups.cend(); ++it) {
        if(it->name == name && foundNumber++ == number)
            return it->group;
    }

    return nullptr;
}

std::vector<ConfigurationGroup*> ConfigurationGroup::groups(const std::string& name) {
    std::vector<ConfigurationGroup*> found;

    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        if(name.empty() || it->name == name) found.push_back(it->group);

    return found;
}

std::vector<const ConfigurationGroup*> ConfigurationGroup::groups(const std::string& name) const {
    std::vector<const ConfigurationGroup*> found;

    for(auto it = _groups.cbegin(); it != _groups.cend(); ++it)
        if(name.empty() || it->name == name) found.push_back(it->group);

    return found;
}

unsigned int ConfigurationGroup::groupCount(const std::string& name) const {
    /** @todo How the hell is THIS more efficient??? */
    if(name.empty()) return _groups.size();
    return groups(name).size();
}

bool ConfigurationGroup::groupExists(const std::string& name) const {
    /** @todo How the hell is THIS more efficient??? */
    if(name.empty()) return !_groups.empty();
    return group(name) != nullptr;
}

bool ConfigurationGroup::addGroup(const std::string& name, ConfigurationGroup* group) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    /* Set configuration pointer to actual */
    group->configuration = configuration;

    /* Name must not be empty and must not contain slash character */
    if(name.empty() || name.find('/') != std::string::npos) {
        Error() << "Slash in group name!";
        return false;
    }

    /* Check for unique groups */
    if(configuration->flags & Configuration::InternalFlag::UniqueGroups) {
        for(auto it = _groups.cbegin(); it != _groups.cend(); ++it)
            if(it->name == name) return false;
    }

    configuration->flags |= Configuration::InternalFlag::Changed;

    Group g;
    g.name = name;
    g.group = group;
    _groups.push_back(g);
    return true;
}

ConfigurationGroup* ConfigurationGroup::addGroup(const std::string& name) {
    ConfigurationGroup* group = new ConfigurationGroup(configuration);
    if(!addGroup(name, group)) {
        delete group;
        group = nullptr;
    }
    return group;
}

bool ConfigurationGroup::removeGroup(const std::string& name, unsigned int number) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid)) return false;

    /* Find group with given number and name */
    unsigned int foundNumber = 0;
    for(auto it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->name == name && foundNumber++ == number) {
            delete it->group;
            _groups.erase(it);
            configuration->flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeGroup(ConfigurationGroup* group) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid)) return false;

    for(auto it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->group == group) {
            delete it->group;
            _groups.erase(it);
            configuration->flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeAllGroups(const std::string& name) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid)) return false;

    for(int i = _groups.size()-1; i >= 0; --i) {
        if(_groups[i].name != name) continue;
        delete (_groups.begin()+i)->group;
        _groups.erase(_groups.begin()+i);
    }

    configuration->flags |= Configuration::InternalFlag::Changed;
    return true;
}

unsigned int ConfigurationGroup::keyCount(const std::string& key) const {
    unsigned int count = 0;
    for(auto it = items.cbegin(); it != items.cend(); ++it)
        if(it->key == key) count++;

    return count;
}

bool ConfigurationGroup::keyExists(const std::string& key) const {
    for(auto it = items.cbegin(); it != items.cend(); ++it)
        if(it->key == key) return true;

    return false;
}

std::vector<std::string> ConfigurationGroup::valuesInternal(const std::string& key, ConfigurationValueFlags) const {
    std::vector<std::string> found;

    for(auto it = items.cbegin(); it != items.cend(); ++it)
        if(it->key == key) found.push_back(it->value);

    return found;
}

bool ConfigurationGroup::setValueInternal(const std::string& key, std::string value, unsigned int number, ConfigurationValueFlags) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    /* Key cannot be empty => this would change comments / empty lines */
    if(key.empty()) return false;

    unsigned int foundNumber = 0;
    for(auto it = items.begin(); it != items.end(); ++it) {
        if(it->key == key && foundNumber++ == number) {
            it->value = std::move(value);
            configuration->flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    /* No value with that name was found, add new */
    Item i;
    i.key = key;
    i.value = std::move(value);
    items.push_back(i);

    configuration->flags |= Configuration::InternalFlag::Changed;
    return true;
}

bool ConfigurationGroup::addValueInternal(std::string key, std::string value, ConfigurationValueFlags) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    /* Key cannot be empty => empty keys are treated as comments / empty lines */
    if(key.empty()) return false;

    /* Check for unique keys */
    if(configuration->flags & Configuration::InternalFlag::UniqueKeys) {
        for(auto it = items.cbegin(); it != items.cend(); ++it)
            if(it->key == key) return false;
    }

    Item i;
    i.key = std::move(key);
    i.value = std::move(value);
    items.push_back(i);

    configuration->flags |= Configuration::InternalFlag::Changed;
    return true;
}

bool ConfigurationGroup::valueInternal(const std::string& key, std::string* value, unsigned int number, ConfigurationValueFlags flags) {
    const ConfigurationGroup* c = this;
    if(c->value(key, value, number, flags)) return true;

    /* Automatic key/value pair creation is enabled and user wants first key,
        try to create new key/value pair */
    if((configuration->flags & Configuration::InternalFlag::AutoCreateKeys) && number == 0)
        return setValueInternal(key, *value, number, flags);

    return false;
}

bool ConfigurationGroup::valueInternal(const std::string& key, std::string* value, unsigned int number, ConfigurationValueFlags) const {
    unsigned int foundNumber = 0;
    for(auto it = items.cbegin(); it != items.cend(); ++it) {
        if(it->key == key) {
            if(foundNumber++ == number) {
                *value = it->value;
                return true;
            }
        }
    }

    return false;
}

bool ConfigurationGroup::removeValue(const std::string& key, unsigned int number) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    /* Key cannot be empty => empty keys are treated as comments / empty lines */
    if(key.empty()) return false;

    unsigned int foundNumber = 0;
    for(auto it = items.begin(); it != items.end(); ++it) {
        if(it->key == key && foundNumber++ == number) {
            items.erase(it);
            configuration->flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    return false;
}

bool ConfigurationGroup::removeAllValues(const std::string& key) {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    /** @todo Do it better & faster */
    for(int i = items.size()-1; i >= 0; --i) {
        if(items[i].key == key) items.erase(items.begin()+i);
    }

    configuration->flags |= Configuration::InternalFlag::Changed;
    return true;
}

bool ConfigurationGroup::clear() {
    if(configuration->flags & Configuration::InternalFlag::ReadOnly ||
     !(configuration->flags & Configuration::InternalFlag::IsValid))
        return false;

    items.clear();

    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        delete it->group;
    _groups.clear();

    return true;
}

}}
