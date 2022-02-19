#ifndef Corrade_Utility_FormatStlStringView_h
#define Corrade_Utility_FormatStlStringView_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

/** @file
STL @ref std::string_view compatibility for @ref Utility::format()
@m_since_latest

Including this header allows you to use a C++17 @ref std::string_view in
arguments to @ref Utility::format(). A separate @ref Corrade/Utility/FormatStl.h
header provides compatibility with @ref std::string.
@experimental
*/

#include <string_view>

#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Format.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Utility { namespace Implementation {

template<> struct Formatter<std::string_view> {
    static std::size_t format(const Containers::MutableStringView& buffer, std::string_view value, int precision, FormatType type) {
        /* Not using the StringView STL compatibility to avoid including
           StringStlStringView.h which also drags in String.h */
        return Formatter<Containers::StringView>::format(buffer, {value.data(), value.size()}, precision, type);
    }
    static void format(std::FILE* file, std::string_view value, int precision, FormatType type) {
        /* Not using the StringView STL compatibility to avoid including
           StringStlStringView.h which also drags in String.h */
        return Formatter<Containers::StringView>::format(file, {value.data(), value.size()}, precision, type);
    }
};

}}}
#endif

#endif
