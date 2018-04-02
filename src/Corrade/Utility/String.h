#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Namespace @ref Corrade::Utility::String
 */

#include <cstddef>
#include <string>
#include <vector>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief String utilities

This library is built if `WITH_UTILITY` is enabled when building Corrade. To
use this library with CMake, you need to request the `Utility` component of the
`Corrade` package and link to the `Corrade::Utility` target.

@code{.cmake}
find_package(Corrade REQUIRED Utility)

# ...
target_link_libraries(your-app Corrade::Utility)
@endcode

See also @ref building-corrade and @ref corrade-cmake for more information.
*/
namespace String {

namespace Implementation {
    CORRADE_UTILITY_EXPORT std::string ltrim(std::string string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT std::string rtrim(std::string string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT std::string trim(std::string string, Containers::ArrayView<const char> characters);

    CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string, Containers::ArrayView<const char> delimiters);

    CORRADE_UTILITY_EXPORT bool beginsWith(const std::string& string, Containers::ArrayView<const char> prefix);
    CORRADE_UTILITY_EXPORT bool endsWith(const std::string& string, Containers::ArrayView<const char> suffix);
}

/**
@brief Safely construct string from char array

If @p string is @cpp nullptr @ce, returns empty string.
*/
inline std::string fromArray(const char* string) {
    return string ? std::string{string} : std::string{};
}

/**
@brief Safely construct string from char array with explicit length

If @p string is @cpp nullptr @ce, returns empty string. Otherwise takes also
@p length into account.
*/
inline std::string fromArray(const char* string, std::size_t length) {
    return string ? std::string{string, length} : std::string{};
}

/**
@brief Trim leading characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

@see @ref rtrim(), @ref trim()
*/
inline std::string ltrim(std::string string, const std::string& characters) {
    return Implementation::ltrim(std::move(string), {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline std::string ltrim(std::string string, const char(&characters)[size]) {
    return Implementation::ltrim(std::move(string), {characters, size - 1});
}

/**
@brief Trim leading whitespace from string

Equivalent to calling @ref ltrim(std::string, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref rtrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string ltrim(std::string string);

/**
@brief Trim trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

@see @ref ltrim(), @ref trim()
*/
inline std::string rtrim(std::string string, const std::string& characters) {
    return Implementation::rtrim(std::move(string), {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline std::string rtrim(std::string string, const char(&characters)[size]) {
    return Implementation::rtrim(std::move(string), {characters, size - 1});
}

/**
@brief Trim trailing whitespace from string

Equivalent to calling @ref rtrim(std::string, const char(&)[size] with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref ltrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string rtrim(std::string string);

/**
@brief Trim leading and trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to @cpp ltrim(rtrim(string)) @ce.
*/
inline std::string trim(std::string string, const std::string& characters) {
    return Implementation::trim(std::move(string), {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline std::string trim(std::string string, const char(&characters)[size]) {
    return Implementation::trim(std::move(string), {characters, size - 1});
}

/**
@brief Trim leading and trailing whitespace from string

Equivalent to calling @ref trim(std::string, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter.
*/
CORRADE_UTILITY_EXPORT std::string trim(std::string string);

/**
@brief Split string on given character
@param string       String to split
@param delimiter    Delimiter
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> split(const std::string& string, char delimiter);

/**
@brief Split string on given character and remove empty parts
@param string       String to split
@param delimiter    Delimiter
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string, char delimiter);

/**
@brief Split string on any character from given set and remove empty parts
@param string       String to split
@param delimiters   Delimiter characters
*/
inline std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const std::string& delimiters) {
    return Implementation::splitWithoutEmptyParts(string, {delimiters.data(), delimiters.size()});
}

/** @overload */
template<std::size_t size> inline std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const char(&delimiters)[size]) {
    return Implementation::splitWithoutEmptyParts(string, {delimiters, size - 1});
}

/**
@brief Split string on whitespaces and remove empty parts

Equivalent to calling @ref splitWithoutEmptyParts(const std::string&, const char(&)[size])
with @cpp " \t\f\v\r\n" @ce as second parameter.
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string);

/**
@brief Join strings with given character
@param strings      Strings to join
@param delimiter    Delimiter
*/
CORRADE_UTILITY_EXPORT std::string join(const std::vector<std::string>& strings, char delimiter);

/**
@brief Join strings with given character and remove empty parts
@param strings      Strings to join
@param delimiter    Delimiter
*/
CORRADE_UTILITY_EXPORT std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, char delimiter);

/**
@brief Convert string to lowercase

@attention Doesn't work with UTF-8.
*/
CORRADE_UTILITY_EXPORT std::string lowercase(std::string string);

/**
@brief Convert string to uppercase

@attention Doesn't work with UTF-8.
*/
CORRADE_UTILITY_EXPORT std::string uppercase(std::string string);

/**
@brief Whether the string has given prefix

In particular, returns @cpp false @ce also if @p string is empty.
*/
inline bool beginsWith(const std::string& string, const std::string& prefix) {
    return Implementation::beginsWith(string, {prefix.data(), prefix.size()});
}

/** @overload */
template<std::size_t size> inline bool beginsWith(const std::string& string, const char(&prefix)[size]) {
    return Implementation::beginsWith(string, {prefix, size - 1});
}

/** @overload */
inline bool beginsWith(const std::string& string, char prefix) {
    return !string.empty() && string[0] == prefix;
}

/**
@brief Whether the string has given suffix

In particular, returns @cpp false @ce also if @p string is empty.
*/
inline bool endsWith(const std::string& string, const std::string& suffix) {
    return Implementation::endsWith(string, {suffix.data(), suffix.size()});
}

/** @overload */
template<std::size_t size> inline bool endsWith(const std::string& string, const char(&suffix)[size]) {
    return Implementation::endsWith(string, {suffix, size - 1});
}

/** @overload */
inline bool endsWith(const std::string& string, char suffix) {
    return !string.empty() && string[string.size() - 1] == suffix;
}

}}}

#endif
