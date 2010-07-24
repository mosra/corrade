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

#include "Configuration.h"

#include <fstream>

#include "utilities.h"

using namespace std;

namespace Map2X { namespace Utility {

Configuration::Configuration(const string& _filename, int _flags): filename(_filename), flags(_flags) {
    /* Open file with requested flags */
    ifstream::openmode openmode = ifstream::in;
    if(flags & Truncate) openmode |= ifstream::trunc;
    ifstream file(filename.c_str(), openmode);

    /* File doesn't exist yet */
    if(!file.is_open()) {
        /** @todo check better */

        /* Empty global group */
        _groups.push_back(ConfigurationGroup("", vector<ConfigurationGroup::Item>(), this));
        flags |= IsValid;
        return;
    }

    if(!file.good()) {
        /** @todo error to stderr */
        return;
    }

    /* It looks like BOM */
    if(file.peek() == '\xEF') {
        char* bom = new char[4];
        file.get(bom, 4);

        /* This is not a BOM, rewind back */
        if(string(bom) != "\xEF\xBB\xBF") file.seekg(0);

        /* Or set flag */
        else flags |= HasBom;

        delete[] bom;
    }

    /* Line buffer, current group name, vector of configuration items */
    string buffer, group;
    vector<ConfigurationGroup::Item> items;

    /* Parse file */
    while(file.good()) {
        getline(file, buffer);

        /* Windows EOL */
        if(buffer[buffer.size()-1] == '\r')
            flags |= WindowsEol;

        buffer = trim(buffer);

        /* Empty line */
        if(buffer.size() == 0) {
            if(flags & (SkipComments|ReadOnly)) continue;

            items.push_back(ConfigurationGroup::Item());

        /* Group header */
        } else if(buffer[0] == '[') {

            /* Check ending bracket */
            /** @todo Error to stderr */
            if(buffer[buffer.size()-1] != ']') return;

            /* Finish current group and start new */

            /* If unique groups are set, check whether current group is unique */
            bool save = true;
            if(flags & UniqueGroups) {
                for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
                    if(it->name() == group) {
                        save = false;
                        break;
                    }
            }

            if(save) _groups.push_back(ConfigurationGroup(group, items, this));
            group = trim(buffer.substr(1, buffer.size()-2));
            /** @todo Error to stderr */
            if(group.empty()) return;
            items.clear();

        /* Comment */
        } else if(buffer[0] == '#' || buffer[0] == ';') {
            if(flags & (SkipComments|ReadOnly)) continue;

            ConfigurationGroup::Item item;
            item.value = buffer;
            items.push_back(item);

        /* Key/value pair */
        } else {
            size_t splitter = buffer.find_first_of('=');
            /** @todo Error to stderr */
            if(splitter == string::npos) return;

            ConfigurationGroup::Item item;
            item.key = trim(buffer.substr(0, splitter));
            item.value = trim(buffer.substr(splitter+1), " \t\v\f\r\n");

            /* Remove quotes, if present */
            /** @todo Check @c '"' characters better */
            if(item.value.size() != 0 && item.value[0] == '"') {
                /** @todo Error to stderr */
                if(item.value.size() < 2 || item.value[item.value.size()-1] != '"') return;

                item.value = item.value.substr(1, item.value.size()-2);
            }

            /* If unique keys are set, check whether current key is unique */
            if(flags & UniqueKeys) {
                bool unique = true;
                for(vector<ConfigurationGroup::Item>::const_iterator it = items.begin(); it != items.end(); ++it) {
                    if(it->key == item.key) unique = false;
                }
                if(!unique) continue;
            }

            items.push_back(item);
        }
    }

    /* Remove last empty line, if present (will be written automatically) */
    if(items.size() != 0 && items[items.size()-1].key == "" && items[items.size()-1].value == "")
        items.pop_back();

    /* If unique groups are set, check whether last group is unique */
    bool save = true;
    if(flags & UniqueGroups) {
        for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
            if(it->name() == group) {
                save = false;
                break;
            }
    }

    /* Finish last group */
    if(save) _groups.push_back(ConfigurationGroup(group, items, this));

    /* Close file */
    file.close();

    /* Everything went fine */
    flags |= IsValid;
}

bool Configuration::save() {
    /* File is readonly or invalid, don't save anything */
    if(flags & ReadOnly || !(flags & IsValid)) return false;

    ofstream file(filename.c_str(), ofstream::out|ofstream::trunc|ofstream::binary);
    if(!file.good()) {
        /** @todo Error to stderr */
        return false;
    }

    /* BOM, if user explicitly wants that crap */
    if((flags & PreserveBom) && (flags & HasBom))
        file.write("\xEF\xBB\xBF", 3);

    /* EOL character */
    string eol;
    if(flags & (ForceWindowsEol|WindowsEol) && !(flags & ForceUnixEol)) eol = "\r\n";
    else eol = "\n";

    string buffer;

    /** @todo Checking file.good() after every operation */
    /** @todo Backup file */

    /* Foreach all groups */
    for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it) {
        /* Group header (if this is not global group) */
        if(!it->name().empty()) {
            buffer = '[' + it->name() + ']' + eol;
            file.write(buffer.c_str(), buffer.size());
        }

        const vector<ConfigurationGroup::Item>& items = it->items();

        /* Foreach all items in the group */
        for(vector<ConfigurationGroup::Item>::const_iterator git = items.begin(); git != items.end(); ++git) {
            /* Key/value pair */
            if(!git->key.empty())
                buffer = git->key + '=' + git->value + eol;

            /* Comment / empty line */
            else buffer = git->value + eol;

            file.write(buffer.c_str(), buffer.size());
        }
    }

    file.close();

    return true;
}

ConfigurationGroup* Configuration::group(const string& name, unsigned int number) {
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup>::iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->name() == name && foundNumber++ == number)
            return &*it;
    }

    return 0;
}

const ConfigurationGroup* Configuration::group(const string& name, unsigned int number) const {
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->name() == name && foundNumber++ == number)
            return it.base();
    }

    return 0;
}

vector<ConfigurationGroup*> Configuration::groups(const string& name) {
    vector<ConfigurationGroup*> found;

    for(vector<ConfigurationGroup>::iterator it = _groups.begin(); it != _groups.end(); ++it)
        if(it->name() == name) found.push_back(it.base());

    return found;
}

vector<const ConfigurationGroup*> Configuration::groups(const string& name) const {
    vector<const ConfigurationGroup*> found;

    for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
        if(it->name() == name) found.push_back(it.base());

    return found;
}

unsigned int Configuration::groupCount(const string& name) const {
    unsigned int count = 0;
    for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
        if(it->name() == name) count++;

    return count;
}

ConfigurationGroup* Configuration::addGroup(const std::string& name) {
    if(flags & ReadOnly || !(flags & IsValid)) return 0;

    /* Global group can be only one */
    if(name.empty()) return 0;

    /* Check for unique groups */
    if(flags & UniqueGroups) {
        for(vector<ConfigurationGroup>::const_iterator it = _groups.begin(); it != _groups.end(); ++it)
            if(it->name() == name) return 0;
    }

    flags |= Changed;

    ConfigurationGroup g(name, vector<ConfigurationGroup::Item>(), this);
    _groups.push_back(g);
    return &_groups.back();
}

bool Configuration::removeGroup(const std::string& name, unsigned int number) {
    if(flags & ReadOnly || !(flags & IsValid)) return false;

    /* Global group cannot be removed */
    if(name == "") return false;

    /* Find group with given number and name */
    unsigned int foundNumber = 0;
    for(vector<ConfigurationGroup>::iterator it = _groups.begin(); it != _groups.end(); ++it) {
        if(it->name() == name && foundNumber++ == number) {
            _groups.erase(it);
            flags |= Changed;
            return true;
        }
    }

    return false;
}

bool Configuration::removeAllGroups(const std::string& name) {
    if(flags & ReadOnly || !(flags & IsValid)) return false;

    /* Global group cannot be removed */
    if(name == "") return false;

    for(int i = _groups.size()-1; i >= 0; --i) {
        if(_groups[i].name() == name) _groups.erase(_groups.begin()+i);
    }

    flags |= Changed;
    return true;
}

}}
