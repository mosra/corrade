#ifndef Corrade_Containers_StringStl_h
#define Corrade_Containers_StringStl_h
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
@brief STL @ref std::string compatibility for @ref Corrade::Containers::String and @ref Corrade::Containers::StringView
@m_since_latest

Including this header allows you to convert a
@ref Corrade::Containers::String / @ref Corrade::Containers::StringView from
and to @ref std::string. See @ref Containers-String-stl "String STL compatibility"
and @ref Containers-BasicStringView-stl "StringView STL compatibility" for more
information.
*/

#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/StlForwardString.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

template<> struct CORRADE_UTILITY_EXPORT StringConverter<std::string> {
    static String from(const std::string& other);
    static std::string to(const String& other);
};

template<> struct CORRADE_UTILITY_EXPORT StringViewConverter<const char, std::string> {
    static StringView from(const std::string& other);
    static std::string to(StringView other);
};

template<> struct CORRADE_UTILITY_EXPORT StringViewConverter<char, std::string> {
    static MutableStringView from(std::string& other);
    static std::string to(MutableStringView other);
};

}}}
#endif

#endif
