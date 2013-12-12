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

#include "String.h"

#include <cctype>
#include <algorithm>

namespace Corrade { namespace Utility {

const std::string String::Whitespace(" \t\f\v\r\n");
const std::string String::Bom("\xEF\xBB\xBF");

std::string String::ltrim(std::string string, const std::string& characters) {
    return string.erase(0, string.find_first_not_of(characters));
}

std::string String::rtrim(std::string string, const std::string& characters) {
    return string.erase(string.find_last_not_of(characters)+1);
}

std::string String::trim(std::string string, const std::string& characters) {
    return ltrim(rtrim(std::move(string)), characters);
}

std::vector<std::string> String::split(const std::string& string, const char delimiter) {
    std::vector<std::string> parts;
    std::size_t oldpos = 0, pos = std::string::npos;

    while((pos = string.find(delimiter, oldpos)) != std::string::npos) {
        parts.push_back(string.substr(oldpos, pos-oldpos));
        oldpos = pos+1;
    }

    if(!string.empty())
        parts.push_back(string.substr(oldpos));

    return parts;
}

std::vector<std::string> String::splitWithoutEmptyParts(const std::string& string, const char delimiter) {
    std::vector<std::string> parts;
    std::size_t oldpos = 0, pos = std::string::npos;

    while((pos = string.find(delimiter, oldpos)) != std::string::npos) {
        if(pos != oldpos)
            parts.push_back(string.substr(oldpos, pos-oldpos));

        oldpos = pos+1;
    }

    if(!string.empty() && (oldpos < string.size()))
        parts.push_back(string.substr(oldpos));

    return parts;
}

std::string String::join(const std::vector<std::string>& strings, const char delimiter) {
    /* Compute size of resulting string, count also delimiters */
    std::size_t size = 0;
    for(auto it = strings.begin(); it != strings.end(); ++it)
        size += it->size() + 1;
    if(size) --size;

    /* Reserve memory for resulting string */
    std::string result;
    result.reserve(size);

    /* Join strings */
    for(auto it = strings.begin(); it != strings.end(); ++it) {
        result += *it;
        if(result.size() != size) result += delimiter;
    }

    return result;
}

std::string String::joinWithoutEmptyParts(const std::vector<std::string>& strings, const char delimiter) {
    /* Compute size of resulting string, count also delimiters */
    std::size_t size = 0;
    for(auto it = strings.begin(); it != strings.end(); ++it)
        if(!it->empty()) size += it->size() + 1;
    if(size) --size;

    /* Reserve memory for resulting string */
    std::string result;
    result.reserve(size);

    /* Join strings */
    for(auto it = strings.begin(); it != strings.end(); ++it) {
        if(it->empty()) continue;

        result += *it;
        if(result.size() != size) result += delimiter;
    }

    return result;
}


std::string String::lowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(std::tolower));
    return std::move(str);
}

std::string String::uppercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(std::toupper));
    return std::move(str);
}

}}
