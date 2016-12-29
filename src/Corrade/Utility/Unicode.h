#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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
 * @brief Namespace @ref Corrade::Utility::Unicode
 */

#include <cstddef>
#include <string>
#include <utility>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility { namespace Unicode {

/**
@brief Next UTF-8 character

Returns Unicode codepoint of character on the cursor and position of the
following character. If an error occurs, returns position of next byte and
`0xffffffffu` as codepoint.
*/
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> nextChar(Containers::ArrayView<const char> text, std::size_t cursor);

/** @overload */
inline std::pair<char32_t, std::size_t> nextChar(const std::string& text, const std::size_t cursor) {
    return nextChar(Containers::ArrayView<const char>{text.data(), text.size()}, cursor);
}

/** @overload */
/* to fix ambiguity when passing char array in */
template<std::size_t size> inline std::pair<char32_t, std::size_t> nextChar(const char(&text)[size], const std::size_t cursor) {
    return nextChar(Containers::ArrayView<const char>{text, size - 1}, cursor);
}

/**
@brief Previous UTF-8 character

Returns Unicode codepoint of character before the cursor and its position. If
an error occurs, returns position of previous byte and `0xffffffffu` as
codepoint.
*/
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> prevChar(Containers::ArrayView<const char> text, std::size_t cursor);

/** @overload */
inline std::pair<char32_t, std::size_t> prevChar(const std::string& text, const std::size_t cursor) {
    return prevChar(Containers::ArrayView<const char>{text.data(), text.size()}, cursor);
}

/** @overload */
/* to fix ambiguity when passing char array in */
template<std::size_t size> inline std::pair<char32_t, std::size_t> prevChar(const char(&text)[size], const std::size_t cursor) {
    return prevChar(Containers::ArrayView<const char>{text, size - 1}, cursor);
}

/** @brief Convert UTF-8 to UTF-32 */
CORRADE_UTILITY_EXPORT std::u32string utf32(const std::string& text);

}}}

#endif
