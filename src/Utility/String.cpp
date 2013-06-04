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

std::string String::trim(std::string str, const std::string& characters) {
    return ltrim(rtrim(std::move(str)), characters);
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
    std::transform(str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(std::tolower));
    return str;
}

}}
