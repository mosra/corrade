#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
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

#ifdef CORRADE_BUILD_DEPRECATED
/* For narrow() / widen(), which used to return / take a std::[w]string. Not
   ideal for the return types and does nothing for std::wstring that's now an
   Array (though there you might get it working just from the implicit pointer
   conversion alone), but at least something. */
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/StringStl.h>
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
namespace Implementation {
    CORRADE_UTILITY_EXPORT Containers::Array<wchar_t> widen(const char* text, int size);
    CORRADE_UTILITY_EXPORT Containers::String narrow(const wchar_t* text, int size);
}

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
/* Not named utf16() in order to avoid clashes with potential portable
   std::u16string utf16(const std::string&) implementation in the future. */
/* The returned value is an Array, which compared to a std::wstring means we
   can't use SSO to our advantage. On the other hand, assuming the string being
   no more than three pointers large, it means there's at most 21 bytes to
   store if we exclude two bytes for a null terminator and one byte for size,
   thus 10 characters at most. Which is two characters less than what 8.3
   filenames had, thus unlikely to help us in any real world scenario. And
   practically maybe only 7 instead of 10, as only libc++ is able to use 22
   bytes out of 24, the others just 15: https://stackoverflow.com/a/28003328 */
CORRADE_UTILITY_EXPORT Containers::Array<wchar_t> widen(Containers::StringView text);

/** @overload
Expects that @p text is null-terminated.
@todo can be safely removed once the STL overload below is dropped, the
    StringView will do the job on its own
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
Containers::Array<wchar_t> widen(const char* text);
#else
/* Needed to avoid ambiguity in calls to widen() or narrow() with types that
   are convertible both to a pointer and to an StringView (or a std::string).

   Matches char*, const char*, char[] or cibst char[] but not StringView or any
   other class convertible to a StringView (or a std::string). The array is
   matched because usually when a static array is used for character storage,
   its size isn't size of the actual string there.

   The return type is templated to avoid unconditionally including Array.h. */
template<class T, class R = Containers::Array<wchar_t>, class = typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value || std::is_same<typename std::decay<T>::type, char*>::value>::type> inline R widen(T&& text) {
    return Implementation::widen(text, -1);
}
#endif

/** @overload */
CORRADE_UTILITY_EXPORT std::wstring widen(const std::string& text);

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

/** @overload
Expects that @p text is null-terminated.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
Containers::String narrow(const wchar_t* text);
#else
/* Needed to avoid ambiguity in calls to widen() or narrow() with types that
   are convertible both to a pointer and to an StringView (or a std::string).

   Matches wchar_t*, const wchar_t*, wchar_t[] or const wchar_t[] but not
   ArrayView<const wchar_t> or any other class convertible to an ArrayView (or
   a std::wstring). The array is matched because usually when a static array is
   used for character storage, its size isn't size of the actual string
   there.

   The return type is templated to avoid unconditionally including String.h. */
template<class T, class R = Containers::String, class = typename std::enable_if<std::is_same<typename std::decay<T>::type, const wchar_t*>::value || std::is_same<typename std::decay<T>::type, wchar_t*>::value>::type> inline R narrow(T&& text) {
    return Implementation::narrow(text, -1);
}
#endif

/** @overload */
CORRADE_UTILITY_EXPORT std::string narrow(const std::wstring& text);
#endif

}}}

#endif
