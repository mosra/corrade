#ifndef Corrade_Utility_DebugStlStringView_h
#define Corrade_Utility_DebugStlStringView_h
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
@brief STL @ref std::string_view compatibility for @ref Corrade::Utility::Debug
@m_since_latest

Including this header allows you to use a C++17 @ref std::string_view with
@ref Corrade::Utility::Debug. A separate @ref Corrade/Utility/DebugStl.h header
provides compatibility with @ref std::string or @ref std::tuple. See
@ref Utility-Debug-stl for more information.
*/

#include <string_view>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility {

/** @relatesalso Debug
@brief Print a @ref std::string_view to debug output
@m_since_latest
*/
inline Debug& operator<<(Debug& debug, std::string_view value) {
    /* Not using the StringView STL compatibility to avoid including
       StringStlStringView.h which also drags in String.h */
    return debug << Containers::StringView{value.data(), value.size()};
}

/** @relatesalso Debug
@brief Print a @ref std::basic_string_view to debug output
@m_since_latest

All other types than exactly @ref std::string_view are printed as containers.
*/
template<class T> Debug& operator<<(Debug& debug, std::basic_string_view<T> value) {
    return debug << Containers::ArrayView<const T>{value.data(), value.size()};
}

}}

#endif
