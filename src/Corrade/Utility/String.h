#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
    CORRADE_UTILITY_EXPORT void ltrimInPlace(std::string& string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT void rtrimInPlace(std::string& string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT void trimInPlace(std::string& string, Containers::ArrayView<const char> characters);

    CORRADE_UTILITY_EXPORT std::string ltrim(std::string string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT std::string rtrim(std::string string, Containers::ArrayView<const char> characters);
    CORRADE_UTILITY_EXPORT std::string trim(std::string string, Containers::ArrayView<const char> characters);

    CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string, Containers::ArrayView<const char> delimiters);

    CORRADE_UTILITY_EXPORT bool beginsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> prefix);
    CORRADE_UTILITY_EXPORT bool endsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> suffix);

    CORRADE_UTILITY_EXPORT std::string stripPrefix(std::string string, Containers::ArrayView<const char> suffix);
    CORRADE_UTILITY_EXPORT std::string stripSuffix(std::string string, Containers::ArrayView<const char> suffix);

    CORRADE_UTILITY_EXPORT std::string replaceFirst(std::string string, Containers::ArrayView<const char> search, Containers::ArrayView<const char> replace);
    CORRADE_UTILITY_EXPORT std::string replaceAll(std::string string, Containers::ArrayView<const char> search, Containers::ArrayView<const char> replace);
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

Implemented using @ref ltrimInPlace().
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
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref ltrimInPlace().
@see @ref rtrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string ltrim(std::string string);

/**
@brief Trim trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Implemented using @ref rtrimInPlace().
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

Equivalent to calling @ref rtrim(std::string, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref trimInPlace().
@see @ref ltrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string rtrim(std::string string);

/**
@brief Trim leading and trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to @cpp ltrim(rtrim(string, characters), characters) @ce.
Implemented using @ref trimInPlace().
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
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref trimInPlace().
*/
CORRADE_UTILITY_EXPORT std::string trim(std::string string);

/**
@brief Trim leading characters from string, in place
@param string       String to be trimmed in place
@param characters   Characters which will be trimmed

@see @ref ltrim(), @ref rtrimInPlace(), @ref trimInPlace()
*/
inline void ltrimInPlace(std::string& string, const std::string& characters) {
    Implementation::ltrimInPlace(string, {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline void ltrimInPlace(std::string& string, const char(&characters)[size]) {
    Implementation::ltrimInPlace(string, {characters, size - 1});
}

/**
@brief Trim leading whitespace from string

Equivalent to calling @ref ltrimInPlace(std::string&, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref ltrim(), @ref rtrimInPlace(), @ref trimInPlace()
*/
CORRADE_UTILITY_EXPORT void ltrimInPlace(std::string& string);

/**
@brief Trim trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

@see @ref rtrim(), @ref ltrimInPlace(), @ref trimInPlace()
*/
inline void rtrimInPlace(std::string& string, const std::string& characters) {
    Implementation::rtrimInPlace(string, {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline void rtrimInPlace(std::string& string, const char(&characters)[size]) {
    Implementation::rtrimInPlace(string, {characters, size - 1});
}

/**
@brief Trim trailing whitespace from string

Equivalent to calling @ref rtrimInPlace(std::string&, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref rtrim(), @ref ltrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT void rtrimInPlace(std::string& string);

/**
@brief Trim leading and trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to calling both @ref ltrimInPlace() and @ref rtrimInPlace().
@see @ref trim()
*/
inline void trimInPlace(std::string& string, const std::string& characters) {
    return Implementation::trimInPlace(string, {characters.data(), characters.size()});
}

/** @overload */
template<std::size_t size> inline void trimInPlace(std::string& string, const char(&characters)[size]) {
    return Implementation::trimInPlace(string, {characters, size - 1});
}

/**
@brief Trim leading and trailing whitespace from string

Equivalent to calling @ref trimInPlace(std::string&, const char(&)[size]) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref trim()
*/
CORRADE_UTILITY_EXPORT void trimInPlace(std::string& string);

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

In particular, returns @cpp true @ce for empty @p string only if @p prefix is
empty as well.
@see @ref viewBeginsWith(), @ref stripPrefix()
*/
inline bool beginsWith(const std::string& string, const std::string& prefix) {
    return Implementation::beginsWith({string.data(), string.size()}, {prefix.data(), prefix.size()});
}

/** @overload */
template<std::size_t size> inline bool beginsWith(const std::string& string, const char(&prefix)[size]) {
    return Implementation::beginsWith({string.data(), string.size()}, {prefix, size - 1});
}

/** @overload */
inline bool beginsWith(const std::string& string, char prefix) {
    return !string.empty() && string[0] == prefix;
}

/**
@brief Whether string view has given prefix

In particular, returns @cpp true @ce for empty @p string only if @p prefix is
empty as well.
@see @ref beginsWith()
*/
template<std::size_t size> inline bool viewBeginsWith(Containers::ArrayView<const char> string, const char(&prefix)[size]) {
    return Implementation::beginsWith(string, {prefix, size - 1});
}

/** @overload */
inline bool viewBeginsWith(Containers::ArrayView<const char> string, char prefix) {
    return !string.empty() && string[0] == prefix;
}

/**
@brief Whether the string has given suffix

In particular, returns @cpp true @ce for empty @p string only if @p suffix is
empty as well.
@see @ref viewEndsWith(), @ref stripSuffix()
*/
inline bool endsWith(const std::string& string, const std::string& suffix) {
    return Implementation::endsWith({string.data(), string.size()}, {suffix.data(), suffix.size()});
}

/** @overload */
template<std::size_t size> inline bool endsWith(const std::string& string, const char(&suffix)[size]) {
    return Implementation::endsWith({string.data(), string.size()}, {suffix, size - 1});
}

/** @overload */
inline bool endsWith(const std::string& string, char suffix) {
    return !string.empty() && string[string.size() - 1] == suffix;
}

/**
@brief Whether string view has given suffix

In particular, returns @cpp true @ce for empty @p string only if @p suffix is
empty as well.
@see @ref endsWith()
*/
template<std::size_t size> inline bool viewEndsWith(Containers::ArrayView<const char> string, const char(&suffix)[size]) {
    return Implementation::endsWith(string, {suffix, size - 1});
}

/** @overload */
inline bool viewEndsWith(Containers::ArrayView<const char> string, char suffix) {
    return !string.empty() && string[string.size() - 1] == suffix;
}

/**
@brief Strip given prefix from a string

Expects that the string actually begins with given prefix.
@see @ref beginsWith()
*/
inline std::string stripPrefix(std::string string, const std::string& prefix) {
    return Implementation::stripPrefix(std::move(string), {prefix.data(), prefix.size()});
}

/** @overload */
template<std::size_t size> inline std::string stripPrefix(std::string string, const char(&prefix)[size]) {
    return Implementation::stripPrefix(std::move(string), {prefix, size - 1});
}

/** @overload */
inline std::string stripPrefix(std::string string, char prefix) {
    return Implementation::stripPrefix(std::move(string), {&prefix, 1});
}

/**
@brief Strip given suffix from a string

Expects that the string actually ends with given suffix.
@see @ref endsWith()
*/
inline std::string stripSuffix(std::string string, const std::string& suffix) {
    return Implementation::stripSuffix(std::move(string), {suffix.data(), suffix.size()});
}

/** @overload */
template<std::size_t size> inline std::string stripSuffix(std::string string, const char(&suffix)[size]) {
    return Implementation::stripSuffix(std::move(string), {suffix, size - 1});
}

/** @overload */
inline std::string stripSuffix(std::string string, char suffix) {
    return Implementation::stripSuffix(std::move(string), {&suffix, 1});
}

/**
@brief Replace first occurence in a string

Returns @p string unmodified if it doesn't contain @p search. Having empty
@p search causes @p replace to be prepended to @p string.
@see @ref replaceAll()
*/
inline std::string replaceFirst(std::string string, const std::string& search, const std::string& replace) {
    return Implementation::replaceFirst(std::move(string), {search.data(), search.size()}, {replace.data(), replace.size()});
}

/** @overload */
template<std::size_t searchSize, std::size_t replaceSize> inline std::string replaceFirst(std::string string, const char(&search)[searchSize], const char(&replace)[replaceSize]) {
    return Implementation::replaceFirst(std::move(string), {search, searchSize - 1}, {replace, replaceSize - 1});
}

/** @overload */
template<std::size_t searchSize> inline std::string replaceFirst(std::string string, const char(&search)[searchSize], const std::string& replace) {
    return Implementation::replaceFirst(std::move(string), {search, searchSize - 1}, {replace.data(), replace.size()});
}

/** @overload */
template<std::size_t replaceSize> inline std::string replaceFirst(std::string string, const std::string& search, const char(&replace)[replaceSize]) {
    return Implementation::replaceFirst(std::move(string), {search.data(), search.size()}, {replace, replaceSize - 1});
}

/**
@brief Replace all occurences in a string

Returns @p string unmodified if it doesn't contain @p search. Expects that
@p search is not empty, as that would cause an infinite loop.
@see @ref replaceFirst()
*/
inline std::string replaceAll(std::string string, const std::string& search, const std::string& replace) {
    return Implementation::replaceAll(std::move(string), {search.data(), search.size()}, {replace.data(), replace.size()});
}

/** @overload */
template<std::size_t searchSize, std::size_t replaceSize> inline std::string replaceAll(std::string string, const char(&search)[searchSize], const char(&replace)[replaceSize]) {
    return Implementation::replaceAll(std::move(string), {search, searchSize - 1}, {replace, replaceSize - 1});
}

/** @overload */
template<std::size_t searchSize> inline std::string replaceAll(std::string string, const char(&search)[searchSize], const std::string& replace) {
    return Implementation::replaceAll(std::move(string), {search, searchSize - 1}, {replace.data(), replace.size()});
}

/** @overload */
template<std::size_t replaceSize> inline std::string replaceAll(std::string string, const std::string& search, const char(&replace)[replaceSize]) {
    return Implementation::replaceAll(std::move(string), {search.data(), search.size()}, {replace, replaceSize - 1});
}

}}}

#endif
