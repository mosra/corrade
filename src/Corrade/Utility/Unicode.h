#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
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
 * @brief Namespace @ref Corrade::Utility::Unicode
 */

#include <cstddef>
#include <utility>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_TARGET_LIBCXX
#include <string> /* Libc++ doesn't have std::u32string in the fwdecl */
#else
#include "Corrade/Utility/StlForwardString.h"
#endif

namespace Corrade { namespace Utility {

/**
@brief Unicode utilities

This library is built if `WITH_UTILITY` is enabled when building Corrade. To
use this library with CMake, request the `Utility` component of the `Corrade`
package and link to the `Corrade::Utility` target.

@code{.cmake}
find_package(Corrade REQUIRED Utility)

# ...
target_link_libraries(your-app PRIVATE Corrade::Utility)
@endcode

See also @ref building-corrade and @ref corrade-cmake for more information.
*/
namespace Unicode {

/**
@brief Next UTF-8 character

Returns Unicode codepoint of character on the cursor and position of the
following character. If an error occurs, returns position of next byte and
@cpp 0xffffffffu @ce as codepoint.
@see @ref utf8()
*/
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> nextChar(Containers::ArrayView<const char> text, std::size_t cursor);

/** @overload */
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> nextChar(const std::string& text, const std::size_t cursor);

/** @overload */
/* to fix ambiguity when passing char array in */
template<std::size_t size> inline std::pair<char32_t, std::size_t> nextChar(const char(&text)[size], const std::size_t cursor) {
    return nextChar(Containers::ArrayView<const char>{text, size - 1}, cursor);
}

/**
@brief Previous UTF-8 character

Returns Unicode codepoint of character before the cursor and its position. If
an error occurs, returns position of previous byte and @cpp 0xffffffffu @ce as
codepoint.
@see @ref utf8()
*/
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> prevChar(Containers::ArrayView<const char> text, std::size_t cursor);

/** @overload */
CORRADE_UTILITY_EXPORT std::pair<char32_t, std::size_t> prevChar(const std::string& text, const std::size_t cursor);

/** @overload */
/* to fix ambiguity when passing char array in */
template<std::size_t size> inline std::pair<char32_t, std::size_t> prevChar(const char(&text)[size], const std::size_t cursor) {
    return prevChar(Containers::ArrayView<const char>{text, size - 1}, cursor);
}

/** @brief Convert UTF-8 to UTF-32 */
CORRADE_UTILITY_EXPORT std::u32string utf32(const std::string& text);

/**
@brief Convert UTF-32 character to UTF-8
@param[in]  character   UTF-32 character to convert
@param[out] result      Where to put the UTF-8 result

Returns length of the encoding (1, 2, 3 or 4). If @p character is outside of
UTF-32 range, returns 0.
@see @ref nextChar(), @ref prevChar(), @ref utf32()
*/
CORRADE_UTILITY_EXPORT std::size_t utf8(char32_t character, Containers::StaticArrayView<4, char> result);

#if defined(CORRADE_TARGET_WINDOWS) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief Widen UTF-8 string for use with Windows Unicode APIs

Converts UTF-8 string to wide-string (UTF-16) representation. Primary purpose
is easy interaction with Windows Unicode APIs, thus the function doesn't
return @ref std::u16string but a @ref std::wstring.
@partialsupport Available only on @ref CORRADE_TARGET_WINDOWS "Windows" to be
    used when dealing directly with Windows Unicode APIs. Other code should
    always use UTF-8, see http://utf8everywhere.org for more information.
*/
/* Not named utf16() in order to avoid clashes with potential portable
   std::u16string utf16(const std::string&) implementation in the future */
CORRADE_UTILITY_EXPORT std::wstring widen(const std::string& text);

/** @overload */
CORRADE_UTILITY_EXPORT std::wstring widen(Containers::ArrayView<const char> text);

/** @overload
Expects that @p text is null-terminated.
*/
CORRADE_UTILITY_EXPORT std::wstring widen(const char* text);

/**
@brief Narrow string to UTF-8 for use with Windows Unicode APIs

Converts wide-string (UTF-16) to UTF-8 representation. Primary purpose
is easy interaction with Windows Unicode APIs, thus the function doesn't
accept @ref std::u16string but a @ref std::wstring.
@partialsupport Available only on @ref CORRADE_TARGET_WINDOWS "Windows" to be
    used when dealing directly with Windows Unicode APIs. Other code should
    always use UTF-8, see http://utf8everywhere.org for more information.
*/
CORRADE_UTILITY_EXPORT std::string narrow(const std::wstring& text);

/** @overload */
CORRADE_UTILITY_EXPORT std::string narrow(Containers::ArrayView<const wchar_t> text);

/** @overload
Expects that @p text is null-terminated.
*/
CORRADE_UTILITY_EXPORT std::string narrow(const wchar_t* text);
#endif

}}}

#endif
