#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_BUILD_DEPRECATED
/* For APIs that used to return / take a std::[w]string. Not ideal for the
   return types and does nothing for std::wstring that's now an Array (though
   there you might get it working just from the implicit pointer conversion
   alone), but at least something. */
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/StringStl.h>
/* For APIs that used to return a std::pair */
#include <Corrade/Containers/PairStl.h>
#endif

namespace Corrade { namespace Utility {

/**
@brief Unicode utilities

This library is built if `CORRADE_WITH_UTILITY` is enabled when building
Corrade. To use this library with CMake, request the `Utility` component of the
`Corrade` package and link to the `Corrade::Utility` target.

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

Returns a Unicode codepoint of a character at @p cursor and position of the
following character. Expects that @p cursor is less than @p text size. If an
error occurs, returns position of the next byte and @cpp 0xffffffffu @ce as the
codepoint, it's then up to the caller whether it gets treated as a fatal error
or if the invalid character is simply skipped or replaced.
@see @ref utf8()
*/
CORRADE_UTILITY_EXPORT Containers::Pair<char32_t, std::size_t> nextChar(Containers::StringView text, std::size_t cursor);

/**
@brief Previous UTF-8 character

Returns a Unicode codepoint of a character before @p cursor and its position.
Expects that @p cursor is greater than @cpp 0 @ce and less than or equal to
@p text size. If an error occurs, returns position of the previous byte and
@cpp 0xffffffffu @ce as the codepoint, it's then up to the caller whether it
gets treated as a fatal error or if the invalid character is simply skipped or
replaced.
@see @ref utf8()
*/
CORRADE_UTILITY_EXPORT Containers::Pair<char32_t, std::size_t> prevChar(Containers::StringView text, std::size_t cursor);

/**
@brief Convert a UTF-8 string to UTF-32

If an error occurs, returns @ref Containers::NullOpt. Iterate over the string
with @ref nextChar() instead if you need custom handling for invalid
characters.
*/
/* The returned value is an Array, which compared to a std::u32string means we
   can't use SSO to our advantage. On the other hand, assuming the string being
   no more than three pointers large, it means there's at most ~19 bytes to
   store if we exclude four bytes for a null terminator and one byte for size,
   thus 4 codepoints at most. Which is pretty much useless. */
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<char32_t>> utf32(Containers::StringView text);

/**
@brief Convert a UTF-32 character to UTF-8
@param[in]  character   UTF-32 character to convert
@param[out] result      Where to put the UTF-8 result

Returns length of the encoding (1, 2, 3 or 4). If @p character is outside of
the UTF-32 range, returns 0.
@see @ref nextChar(), @ref prevChar(), @ref utf32()
*/
CORRADE_UTILITY_EXPORT std::size_t utf8(char32_t character, Containers::ArrayView4<char> result);

#if defined(CORRADE_TARGET_WINDOWS) || defined(DOXYGEN_GENERATING_OUTPUT)
/**
@brief Widen a UTF-8 string for use with Windows Unicode APIs

Converts a UTF-8 string to a wide-string (UTF-16) representation. The primary
purpose of this API is easy interaction with Windows Unicode APIs, thus the
function doesn't return @cpp char16_t @ce but rather a @cpp wchar_t @ce. If the
text is not empty, the returned array contains a sentinel null terminator
(i.e., not counted into its size).
@partialsupport Available only on @ref CORRADE_TARGET_WINDOWS "Windows" to be
    used when dealing directly with Windows Unicode APIs. Other code should
    always use UTF-8, see http://utf8everywhere.org for more information.
*/
/* Not named utf16() in order to avoid clashes with potential portable utf16()
   implementation in the future. */
/* The returned value is an Array, which compared to a std::wstring means we
   can't use SSO to our advantage. On the other hand, assuming the string being
   no more than three pointers large, it means there's at most 21 bytes to
   store if we exclude two bytes for a null terminator and one byte for size,
   thus 10 characters at most. Which is two characters less than what 8.3
   filenames had, thus unlikely to help us in any real world scenario. And
   practically maybe only 7 instead of 10, as only libc++ is able to use 22
   bytes out of 24, the others just 15: https://stackoverflow.com/a/28003328 */
CORRADE_UTILITY_EXPORT Containers::Array<wchar_t> widen(Containers::StringView text);

/**
@brief Narrow a string to UTF-8 for use with Windows Unicode APIs

Converts a wide-string (UTF-16) to a UTF-8 representation. The primary purpose
is easy interaction with Windows Unicode APIs, thus the function doesn't take
@cpp char16_t @ce but rather a @cpp wchar_t @ce.
@partialsupport Available only on @ref CORRADE_TARGET_WINDOWS "Windows" to be
    used when dealing directly with Windows Unicode APIs. Other code should
    always use UTF-8, see http://utf8everywhere.org for more information.
*/
CORRADE_UTILITY_EXPORT Containers::String narrow(Containers::ArrayView<const wchar_t> text);

namespace Implementation {
    CORRADE_UTILITY_EXPORT Containers::String narrow(const wchar_t* text, int size);
}

/** @overload
Expects that @p text is null-terminated.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
Containers::String narrow(const wchar_t* text);
#else
/* Needed to avoid ambiguity in calls to narrow() with types that are
   convertible both to a pointer and to an StringView.

   Matches wchar_t*, const wchar_t*, wchar_t[] or const wchar_t[] but not
   ArrayView<const wchar_t> or any other class convertible to an ArrayView. The
   array is matched because usually when a static array is used for character
   storage, its size isn't size of the actual string there.

   The return type is templated to avoid unconditionally including String.h. */
template<class T, class R = Containers::String, class = typename std::enable_if<std::is_same<typename std::decay<T>::type, const wchar_t*>::value || std::is_same<typename std::decay<T>::type, wchar_t*>::value>::type> inline R narrow(T&& text) {
    return Implementation::narrow(text, -1);
}
#endif
#endif

}}}

#endif
