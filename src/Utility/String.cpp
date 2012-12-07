/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "String.h"

#include <algorithm>

namespace Corrade { namespace Utility {

const std::string String::Whitespace(" \t\f\v\r\n");
const std::string String::Bom("\xEF\xBB\xBF");

std::string String::ltrim(std::string str, const std::string& characters) {
    return str.erase(0, str.find_first_not_of(characters));
}

std::string String::rtrim(std::string str, const std::string& characters) {
    return str.erase(str.find_last_not_of(characters)+1);
}

std::vector<std::string> String::split(const std::string& str, char delim, bool keepEmptyParts) {
    std::vector<std::string> parts;
    std::size_t oldpos = 0, pos = std::string::npos;

    do {
        pos = str.find(delim, oldpos);
        std::string part = str.substr(oldpos, pos-oldpos);

        if(!part.empty() || keepEmptyParts)
            parts.push_back(part);

        oldpos = pos+1;
    } while(pos != std::string::npos);

    return parts;
}

std::string String::lowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), std::ptr_fun<int, int>(std::tolower));
    return str;
}

}}
