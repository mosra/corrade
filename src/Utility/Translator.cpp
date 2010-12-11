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

#include "Translator.h"

using namespace std;

namespace Kompas { namespace Utility {

set<Translator*>* Translator::instances() {
    static set<Translator*>* _instances = new set<Translator*>;
    return _instances;
}

string* Translator::_locale() {
    static string* locale = new string;
    return locale;
}

void Translator::setLocale(const std::string& locale) {
    *_locale() = locale;

    /* Reload dynamically set languages */
    for(set<Translator*>::const_iterator it = instances()->begin(); it != instances()->end(); ++it) {
        /* Dynamic filename */
        if(!(*it)->primaryDynamicFilename.empty())
            /* primaryDynamicFilename is cleared, pass a copy to avoid loading
               empty filename */
            (*it)->setPrimary((*it)->primaryDynamicFilename.substr(0));

        /* Dynamic group */
        else if((*it)->primaryDynamicGroup)
            (*it)->setPrimary((*it)->primaryDynamicGroup, true);
    }
}

Translator::~Translator() {
    for(map<string, string*>::const_iterator it = localizations.begin(); it != localizations.end(); ++it)
        delete it->second;

    /* Destroy configuration files, if present. Translations loaded directly
        from groups should be deleted by caller. */
    delete primaryFile;
    delete fallbackFile;

    instances()->erase(this);
}

void Translator::setPrimary(const std::string& file) {
    /* Cleaning primary translation */
    if(file.empty()) return setPrimary(0);

    Configuration* c = new Configuration(replaceLocale(file), Configuration::ReadOnly);
    setPrimary(c);
    primaryFile = c;

    /* Dynamic translations */
    primaryDynamicGroup = 0;
    if(file.find_first_of('#') != string::npos)
        primaryDynamicFilename = file;
    else
        primaryDynamicFilename.clear();
}

void Translator::setFallback(const std::string& file) {
    /* Cleaning fallback translation */
    if(file.empty()) return setFallback(0);

    Configuration* c = new Configuration(file, Configuration::ReadOnly);
    setFallback(c);
    fallbackFile = c;
}

void Translator::setPrimary(const Kompas::Utility::ConfigurationGroup* group, bool dynamic) {
    /* Dynamic translations */
    primaryDynamicFilename.clear();
    if(dynamic && group) {
        primaryDynamicGroup = group;
        primary = group->group(locale());
    } else {
        primaryDynamicGroup = 0;
        primary = group;
    }

    /* Delete previous file */
    delete primaryFile;
    primaryFile = 0;

    /* Reload all localizations from new files */
    for(map<string, string*>::const_iterator it = localizations.begin(); it != localizations.end(); ++it)
        get(it->first, it->second, 0);
}

void Translator::setFallback(const Kompas::Utility::ConfigurationGroup* group) {
    /* Delete previous file */
    delete fallbackFile;
    fallbackFile = 0;

    fallback = group;

    /* If primary is dynamic */
    if(primaryDynamicGroup) setPrimary(primaryDynamicGroup, true);

    /* Reload all localizations from new files */
    for(map<string, string*>::const_iterator it = localizations.begin(); it != localizations.end(); ++it)
        get(it->first, it->second, 0);
}

const string* Translator::get(const string& key) {
    /* First try to find existing localization */
    map<string, string*>::const_iterator found = localizations.find(key);
    if(found != localizations.end()) return found->second;

    /* If not found, load from configuration */
    string* text = new string;
    localizations.insert(pair<string, string*>(key, text));
    get(key, text, 0);

    return text;
}

bool Translator::get(const string& key, string* text, int level) const {
    const ConfigurationGroup* g;
    switch(level) {
        case 0: g = primary; break;
        case 1: g = fallback; break;
        default: text->clear(); return false;
    }

    /* If not in current level, try next */
    if(!g || !g->value(key, text)) return get(key, text, ++level);

    return true;
}

string Translator::replaceLocale(const std::string& filename) const {
    size_t pos = filename.find_first_of('#');
    if(pos == string::npos) return filename;

    return filename.substr(0, pos) + locale() + filename.substr(pos+1);
}

}}
