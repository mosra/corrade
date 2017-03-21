/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Corrade { namespace Utility { namespace String {

namespace Implementation {

std::string ltrim(std::string string, const Containers::ArrayView<const char> characters) {
    return std::move(string.erase(0, string.find_first_not_of(characters, 0, characters.size())));
}

std::string rtrim(std::string string, const Containers::ArrayView<const char> characters) {
    return std::move(string.erase(string.find_last_not_of(characters, std::string::npos, characters.size())+1));
}

std::string trim(std::string string, const Containers::ArrayView<const char> characters) {
    return ltrim(rtrim(std::move(string), characters), characters);
}

std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const Containers::ArrayView<const char> delimiters) {
    std::vector<std::string> parts;
    std::size_t oldpos = 0, pos = std::string::npos;

    while((pos = string.find_first_of(delimiters, oldpos, delimiters.size())) != std::string::npos) {
        if(pos != oldpos)
            parts.push_back(string.substr(oldpos, pos-oldpos));

        oldpos = pos+1;
    }

    if(!string.empty() && (oldpos < string.size()))
        parts.push_back(string.substr(oldpos));

    return parts;
}

bool beginsWith(const std::string& string, const Containers::ArrayView<const char> prefix) {
    return string.compare(0, prefix.size(), prefix, prefix.size()) == 0;
}

bool endsWith(const std::string& string, const Containers::ArrayView<const char> suffix) {
    if(string.size() < suffix.size()) return false;

    return string.compare(string.size() - suffix.size(), suffix.size(), suffix, suffix.size()) == 0;
}

}

namespace {
    constexpr const char Whitespace[] = " \t\f\v\r\n";
}

std::string ltrim(std::string string) { return ltrim(std::move(string), Whitespace); }

std::string rtrim(std::string string) { return rtrim(std::move(string), Whitespace); }

std::string trim(std::string string) { return trim(std::move(string), Whitespace); }

std::vector<std::string> splitWithoutEmptyParts(const std::string& string) {
    return splitWithoutEmptyParts(string, Whitespace);
}

std::vector<std::string> split(const std::string& string, const char delimiter) {
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

std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const char delimiter) {
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

std::string join(const std::vector<std::string>& strings, const char delimiter) {
    /* Compute size of resulting string, count also delimiters */
    std::size_t size = 0;
    for(const auto& s: strings) size += s.size() + 1;
    if(size) --size;

    /* Reserve memory for resulting string */
    std::string result;
    result.reserve(size);

    /* Join strings */
    for(const auto& s: strings) {
        result += s;
        if(result.size() != size) result += delimiter;
    }

    return result;
}

std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, const char delimiter) {
    /* Compute size of resulting string, count also delimiters */
    std::size_t size = 0;
    for(const auto& s: strings) if(!s.empty()) size += s.size() + 1;
    if(size) --size;

    /* Reserve memory for resulting string */
    std::string result;
    result.reserve(size);

    /* Join strings */
    for(const auto& s: strings) {
        if(s.empty()) continue;

        result += s;
        if(result.size() != size) result += delimiter;
    }

    return result;
}

std::string lowercase(std::string string) {
    std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::tolower));
    return string;
}

std::string uppercase(std::string string) {
    std::transform(string.begin(), string.end(), string.begin(), static_cast<int (*)(int)>(std::toupper));
    return string;
}

}}}
