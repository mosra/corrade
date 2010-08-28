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

#include "utilities.h"

using namespace std;

namespace Map2X { namespace Utility {

string trim(string str, const string& characters) {
    size_t found = str.find_first_not_of(characters);
    str.erase(0, found);

    found = str.find_last_not_of(characters);
    str.erase(found+1);

    return str;
}

vector<string> split(const string& str, char delim, bool keepEmptyParts) {
    vector<string> parts;
    size_t oldpos = 0, pos = string::npos;

    do {
        pos = str.find(delim, oldpos);
        string part = str.substr(oldpos, pos-oldpos);

        if(!part.empty() || keepEmptyParts)
            parts.push_back(part);

        oldpos = pos+1;
    } while(pos != string::npos);

    return parts;
}

}}
