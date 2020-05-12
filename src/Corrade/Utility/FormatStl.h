#ifndef Corrade_Utility_FormatStl_h
#define Corrade_Utility_FormatStl_h
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

/** @file
 * @brief Function @ref Corrade::Utility::formatString(), @ref Corrade::Utility::formatInto()
 * @m_since{2019,10}
 * @experimental
 */

#include <string>

#include "Corrade/Utility/Format.h"

namespace Corrade { namespace Utility {

/**
@brief Format a string

Same as @ref format(), but returning a @ref std::string instead of
@ref Containers::Array.
*/
template<class ...Args> std::string formatString(const char* format, const Args&... args);

/**
@brief Format a string into an existing string

Takes an existing @p string and writes the formatted content starting at
@p offset. If the string is not large enough, does at most one reallocation
(by calling @p std::string::resize()). Returns final written size (which might
be less than the string size if inserting in the middle). *Does not* write any
terminating @cpp '\0' @ce character. Example usage:

@snippet Utility.cpp formatInto-string

See @ref formatString() for more information about usage and templating
language.

@experimental
*/
template<class ...Args> std::size_t formatInto(std::string& string, std::size_t offset, const char* format, const Args&... args);

namespace Implementation {

template<> struct Formatter<std::string> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::ArrayView<char>& buffer, const std::string& value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, const std::string& value, int precision, FormatType type);
};

CORRADE_UTILITY_EXPORT std::size_t formatInto(std::string& buffer, std::size_t offset, const char* format, BufferFormatter* formatters, std::size_t formattersCount);

}

template<class ...Args> std::string formatString(const char* format, const Args&... args) {
    std::string buffer;
    formatInto(buffer, 0, format, args...);
    return buffer;
}

template<class ...Args> std::size_t formatInto(std::string& buffer, std::size_t offset, const char* format, const Args&... args) {
    Implementation::BufferFormatter formatters[sizeof...(args) + 1] { Implementation::BufferFormatter{args}..., {} };
    return Implementation::formatInto(buffer, offset, format, formatters, sizeof...(args));
}

}}

#endif
