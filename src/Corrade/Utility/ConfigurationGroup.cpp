/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Configuration.h"

namespace Corrade { namespace Utility {

ConfigurationGroup::ConfigurationGroup(): _configuration(nullptr) {}

ConfigurationGroup::ConfigurationGroup(Configuration* configuration): _configuration(configuration) {}

ConfigurationGroup::ConfigurationGroup(const ConfigurationGroup& other): _values(other._values), _groups(other._groups), _configuration(nullptr) {
    /* Deep copy groups */
    for(Group& group: _groups)
        group.group = new ConfigurationGroup(*group.group);
}

ConfigurationGroup::ConfigurationGroup(ConfigurationGroup&& other): _values(std::move(other._values)), _groups(std::move(other._groups)), _configuration(nullptr) {
    /* Reset configuration pointer for subgroups */
    for(Group& group: _groups)
        group.group->_configuration = nullptr;
}

ConfigurationGroup& ConfigurationGroup::operator=(const ConfigurationGroup& other) {
    /* Delete current groups */
    for(Group& group: _groups)
        delete group.group;

    /* _configuration stays the same */
    _values = other._values;
    _groups = other._groups;

    /* Deep copy groups */
    for(Group& group: _groups) {
        group.group = new ConfigurationGroup(*group.group);
        group.group->_configuration = _configuration;
    }

    return *this;
}

ConfigurationGroup& ConfigurationGroup::operator=(ConfigurationGroup&& other) {
    /* Delete current groups */
    for(Group& group: _groups)
        delete group.group;

    /* _configuration stays the same */
    _values = std::move(other._values);
    _groups = std::move(other._groups);

    /* Redirect configuration pointer for subgroups */
    for(Group& group: _groups)
        group.group->_configuration = _configuration;

    return *this;
}

ConfigurationGroup::~ConfigurationGroup() {
    for(Group& group: _groups)
        delete group.group;
}

auto ConfigurationGroup::findGroup(const std::string& name, const unsigned int index) -> std::vector<Group>::iterator {
    unsigned int foundIndex = 0;
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        if(it->name == name && foundIndex++ == index) return it;

    return _groups.end();
}

auto ConfigurationGroup::findGroup(const std::string& name, const unsigned int index) const -> std::vector<Group>::const_iterator {
    unsigned int foundIndex = 0;
    for(auto it = _groups.begin(); it != _groups.end(); ++it)
        if(it->name == name && foundIndex++ == index) return it;

    return _groups.end();
}

bool ConfigurationGroup::hasGroup(const std::string& name, const unsigned int index) const {
    return findGroup(name, index) != _groups.end();
}

unsigned int ConfigurationGroup::groupCount(const std::string& name) const {
    unsigned int count = 0;
    for(const Group& group: _groups)
        if(group.name == name) ++count;

    return count;
}

ConfigurationGroup* ConfigurationGroup::group(const std::string& name, const unsigned int index) {
    const auto it = findGroup(name, index);
    return it != _groups.end() ? it->group : nullptr;
}

const ConfigurationGroup* ConfigurationGroup::group(const std::string& name, unsigned int index) const {
    const auto it = findGroup(name, index);
    return it != _groups.end() ? it->group : nullptr;
}

std::vector<ConfigurationGroup*> ConfigurationGroup::groups(const std::string& name) {
    std::vector<ConfigurationGroup*> found;

    for(Group& group: _groups)
        if(group.name == name) found.push_back(group.group);

    return found;
}

std::vector<const ConfigurationGroup*> ConfigurationGroup::groups(const std::string& name) const {
    std::vector<const ConfigurationGroup*> found;

    for(const Group& group: _groups)
        if(group.name == name) found.push_back(group.group);

    return found;
}

void ConfigurationGroup::addGroup(const std::string& name, ConfigurationGroup* group) {
    /* Set configuration pointer to actual */
    CORRADE_ASSERT(!group->_configuration,
        "Utility::Configuration::addGroup(): the group is already part of some configuration", );
    group->_configuration = _configuration;

    CORRADE_ASSERT(!name.empty(),
        "Utility::ConfigurationGroup::addGroup(): empty group name", );
    CORRADE_ASSERT(name.find_first_of("\n/[]") == std::string::npos,
        "Utility::ConfigurationGroup::addGroup(): disallowed character in group name", );

    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
    _groups.push_back({name, group});
}

ConfigurationGroup* ConfigurationGroup::addGroup(const std::string& name) {
    ConfigurationGroup* const group = new ConfigurationGroup;
    addGroup(name, group);
    return group;
}

bool ConfigurationGroup::removeGroup(const std::string& name, unsigned int index) {
    const auto it = findGroup(name, index);
    if(it == _groups.end()) return false;

    delete it->group;
    _groups.erase(it);
    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
    return true;
}

bool ConfigurationGroup::removeGroup(ConfigurationGroup* const group) {
    for(auto it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->group == group) {
            delete it->group;
            _groups.erase(it);
            if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    return false;
}

void ConfigurationGroup::removeAllGroups(const std::string& name) {
    for(int i = _groups.size()-1; i >= 0; --i) {
        if(_groups[i].name != name) continue;
        delete (_groups.begin()+i)->group;
        _groups.erase(_groups.begin()+i);
    }

    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
}

auto ConfigurationGroup::findValue(const std::string& key, const unsigned int index) const -> std::vector<Value>::const_iterator {
    unsigned int foundIndex = 0;
    for(auto it = _values.begin(); it != _values.end(); ++it)
        if(it->key == key && foundIndex++ == index) return it;

    return _values.end();
}

auto ConfigurationGroup::findValue(const std::string& key, const unsigned int index) -> std::vector<Value>::iterator {
    unsigned int foundIndex = 0;
    for(auto it = _values.begin(); it != _values.end(); ++it)
        if(it->key == key && foundIndex++ == index) return it;

    return _values.end();
}

bool ConfigurationGroup::hasValues() const {
    for(const Value& value: _values)
        if(!value.key.empty()) return true;

    return false;
}

unsigned int ConfigurationGroup::valueCount() const {
    unsigned int count = 0;
    for(const Value& value: _values)
        if(!value.key.empty()) ++count;

    return count;
}

bool ConfigurationGroup::hasValue(const std::string& key, const unsigned int index) const {
    return findValue(key, index) != _values.end();
}

unsigned int ConfigurationGroup::valueCount(const std::string& key) const {
    unsigned int count = 0;
    for(const Value& value: _values)
        if(value.key == key) ++count;

    return count;
}

const std::string* ConfigurationGroup::valueInternal(const std::string& key, const unsigned int index, ConfigurationValueFlags) const {
    const auto it = findValue(key, index);
    return it != _values.end() ? &it->value : nullptr;
}

std::vector<std::string> ConfigurationGroup::valuesInternal(const std::string& key, ConfigurationValueFlags) const {
    std::vector<std::string> found;

    for(const Value& value: _values)
        if(value.key == key) found.push_back(value.value);

    return found;
}

bool ConfigurationGroup::setValueInternal(const std::string& key, std::string value, const unsigned int index, ConfigurationValueFlags) {
    CORRADE_ASSERT(!key.empty(), "Utility::ConfigurationGroup::setValue(): empty key", false);
    CORRADE_ASSERT(key.find_first_of("\n=") == std::string::npos,
        "Utility::ConfigurationGroup::setValue(): disallowed character in key", false);

    unsigned int foundIndex = 0;
    for(Value& v: _values) {
        if(v.key == key && foundIndex++ == index) {
            v.value = std::move(value);
            if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
            return true;
        }
    }

    /* Wanted to set value with index much larger than what we have */
    if(index > foundIndex) return false;

    /* No value with that name was found, add new */
    _values.push_back({key, std::move(value)});

    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
    return true;
}

void ConfigurationGroup::addValueInternal(std::string key, std::string value, ConfigurationValueFlags) {
    CORRADE_ASSERT(!key.empty(), "Utility::ConfigurationGroup::addValue(): empty key", );
    CORRADE_ASSERT(key.find_first_of("\n=") == std::string::npos,
        "Utility::ConfigurationGroup::addValue(): disallowed character in key", );

    _values.push_back({std::move(key), std::move(value)});

    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
}

bool ConfigurationGroup::removeValue(const std::string& key, const unsigned int index) {
    CORRADE_ASSERT(!key.empty(), "Utility::ConfigurationGroup::removeValue(): empty key", false);

    const auto it = findValue(key, index);
    if(it == _values.end()) return false;

    _values.erase(it);
    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
    return true;
}

void ConfigurationGroup::removeAllValues(const std::string& key) {
    CORRADE_ASSERT(!key.empty(), "Utility::ConfigurationGroup::removeAllValues(): empty key", );

    /** @todo Do it better & faster */
    for(int i = _values.size()-1; i >= 0; --i) {
        if(_values[i].key == key) _values.erase(_values.begin()+i);
    }

    if(_configuration) _configuration->_flags |= Configuration::InternalFlag::Changed;
}

void ConfigurationGroup::clear() {
    _values.clear();

    for(Group& group: _groups)
        delete group.group;
    _groups.clear();
}

}}
