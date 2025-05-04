#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
 * @brief Namespace @ref Corrade::Utility::String
 */

#include <cstddef>
#include <cstdint>

#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief String utilities

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
namespace String {

/**
@brief Safely construct string from char array

If @p string is @cpp nullptr @ce, returns empty string.
*/
CORRADE_UTILITY_EXPORT std::string fromArray(const char* string);

/**
@brief Safely construct string from char array with explicit length

If @p string is @cpp nullptr @ce, returns empty string. Otherwise takes also
@p length into account.
*/
CORRADE_UTILITY_EXPORT std::string fromArray(const char* string, std::size_t length);

/**
@brief Trim leading characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Implemented using @ref ltrimInPlace().
@see @ref rtrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string ltrim(std::string string, const std::string& characters);

/**
@brief Trim leading whitespace from string

Equivalent to calling @ref ltrim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref ltrimInPlace().
@see @ref rtrim(), @ref trim()
*/
CORRADE_UTILITY_EXPORT std::string ltrim(std::string string);

/**
@brief Trim trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Implemented using @ref rtrimInPlace().
@see @ref ltrim(), @ref trim(),
    @ref Containers::StringView::trimmedSuffix(StringView) const
*/
CORRADE_UTILITY_EXPORT std::string rtrim(std::string string, const std::string& characters);

/**
@brief Trim trailing whitespace from string

Equivalent to calling @ref rtrim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref trimInPlace().
@see @ref ltrim(), @ref trim(),
    @ref Containers::StringView::trimmedSuffix() const
*/
CORRADE_UTILITY_EXPORT std::string rtrim(std::string string);

/**
@brief Trim leading and trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to @cpp ltrim(rtrim(string, characters), characters) @ce.
Implemented using @ref trimInPlace().
@see @ref Containers::StringView::trimmed(StringView) const
*/
CORRADE_UTILITY_EXPORT std::string trim(std::string string, const std::string& characters);

/**
@brief Trim leading and trailing whitespace from string

Equivalent to calling @ref trim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using
@ref trimInPlace().
@see @ref Containers::StringView::trimmed() const
*/
CORRADE_UTILITY_EXPORT std::string trim(std::string string);

/**
@brief Trim leading characters from a string, in place
@param string       String to be trimmed in place
@param characters   Characters which will be trimmed

@see @ref ltrim(), @ref rtrimInPlace(), @ref trimInPlace(),
    @ref Containers::StringView::trimmedPrefix(StringView) const
*/
CORRADE_UTILITY_EXPORT void ltrimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim leading whitespace from a string, in place

Equivalent to calling @ref ltrimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref ltrim(), @ref rtrimInPlace(), @ref trimInPlace(),
    @ref Containers::StringView::trimmedPrefix() const
*/
CORRADE_UTILITY_EXPORT void ltrimInPlace(std::string& string);

/**
@brief Trim trailing characters from a string, in place
@param string       String to be trimmed
@param characters   Characters which will be trimmed

@see @ref rtrim(), @ref ltrimInPlace(), @ref trimInPlace(),
    @ref Containers::StringView::trimmedSuffix(StringView) const
*/
CORRADE_UTILITY_EXPORT void rtrimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim trailing whitespace from a string, in place

Equivalent to calling @ref rtrimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref rtrim(), @ref ltrim(), @ref trim(),
    @ref Containers::StringView::trimmedSuffix() const
*/
CORRADE_UTILITY_EXPORT void rtrimInPlace(std::string& string);

/**
@brief Trim leading and trailing characters from a string, in place
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to calling both @ref ltrimInPlace() and @ref rtrimInPlace().
@see @ref trim(), @ref Containers::StringView::trimmed(StringView) const
*/
CORRADE_UTILITY_EXPORT void trimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim leading and trailing whitespace from a string, in place

Equivalent to calling @ref trimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@see @ref trim(), @ref Containers::StringView::trimmed() const
*/
CORRADE_UTILITY_EXPORT void trimInPlace(std::string& string);

/**
@brief Split a string on given character
@param string       String to split
@param delimiter    Delimiter

@see @ref Containers::StringView::split(char) const
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> split(const std::string& string, char delimiter);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::split(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::split() instead") Containers::Array<Containers::StringView> split(Containers::StringView string, char delimiter);
#endif

/**
@brief Split a string on given character and remove empty parts
@param string       String to split
@param delimiter    Delimiter

@see @ref Containers::StringView::splitWithoutEmptyParts(char) const
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string, char delimiter);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitWithoutEmptyParts(char) const instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(Containers::StringView string, char delimiter);
#endif

/**
@brief Split a string on any character from given set and remove empty parts
@param string       String to split
@param delimiters   Delimiter characters

@see @ref Containers::StringView::splitOnAnyWithoutEmptyParts(StringView) const
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const std::string& delimiters);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnAnyWithoutEmptyParts(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnAnyWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(Containers::StringView string, Containers::StringView delimiters);
#endif

/**
@brief Split a string on whitespace and remove empty parts

Equivalent to calling @ref splitWithoutEmptyParts(const std::string&, const std::string&)
with @cpp " \t\f\v\r\n" @ce as second parameter.

@see @ref Containers::StringView::splitOnAnyWithoutEmptyParts()
*/
CORRADE_UTILITY_EXPORT std::vector<std::string> splitWithoutEmptyParts(const std::string& string);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnWhitespaceWithoutEmptyParts() const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnWhitespaceWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(const Containers::StringView string);
#endif

/**
@brief Partition a string
@m_since{2019,10}

Equivalent to Python's @m_class{m-doc-external} [str.partition()](https://docs.python.org/3/library/stdtypes.html#str.partition).
Splits @p string at the first occurrence of @p separator. First returned value
is the part before the separator, second the separator, third a part after the
separator. If the separator is not found, returns the input string followed by
two empty strings.
@see @ref rpartition(), @ref Path::splitExtension(),
    @ref Containers::StringView::partition()
*/
CORRADE_UTILITY_EXPORT Containers::StaticArray<3, std::string> partition(const std::string& string, char separator);

/**
@overload
@m_since{2019,10}
*/
CORRADE_UTILITY_EXPORT Containers::StaticArray<3, std::string> partition(const std::string& string, const std::string& separator);

/**
@brief Right-partition a string
@m_since{2019,10}

Equivalent to Python's @m_class{m-doc-external} [str.rpartition()](https://docs.python.org/3/library/stdtypes.html#str.rpartition).
Splits @p string at the last occurrence of @p separator. First returned value is
the part before the separator, second the separator, third a part after the
separator. If the separator is not found, returns two empty strings followed by
the input string.
@see @ref partition(), @ref Path::splitExtension()
*/
CORRADE_UTILITY_EXPORT Containers::StaticArray<3, std::string> rpartition(const std::string& string, char separator);

/**
@overload
@m_since{2019,10}
*/
CORRADE_UTILITY_EXPORT Containers::StaticArray<3, std::string> rpartition(const std::string& string, const std::string& separator);

/**
@brief Join strings with given character
@param strings      Strings to join
@param delimiter    Delimiter

@see @ref Containers::StringView::join()
*/
CORRADE_UTILITY_EXPORT std::string join(const std::vector<std::string>& strings, char delimiter);

/**
@overload
@m_since{2019,10}
*/
CORRADE_UTILITY_EXPORT std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

/**
@brief Join strings with given character and remove empty parts
@param strings      Strings to join
@param delimiter    Delimiter

@see @ref Containers::StringView::joinWithoutEmptyParts()
*/
CORRADE_UTILITY_EXPORT std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, char delimiter);

/** @overload */
CORRADE_UTILITY_EXPORT std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, const std::string& delimiter);

namespace Implementation {
    CORRADE_UTILITY_EXPORT extern const char* CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(commonPrefix)(const char* a, const char* b, std::size_t sizeA, std::size_t sizeB);
    CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(commonPrefix)
}

/**
@brief Longest common prefix of two strings
@m_since_latest

The returned view is a prefix of @p a.
@see @ref Containers::StringView::prefix()
*/
inline Containers::StringView commonPrefix(Containers::StringView a, Containers::StringView b) {
    return a.prefix(Implementation::commonPrefix(a.data(), b.data(), a.size(), b.size()));
}

namespace Implementation {
    CORRADE_UTILITY_EXPORT extern void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(lowercaseInPlace)(char* data, std::size_t size);
    CORRADE_UTILITY_EXPORT extern void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(uppercaseInPlace)(char* data, std::size_t size);
    CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(lowercaseInPlace)
    CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(uppercaseInPlace)
}

/**
@brief Convert ASCII characters in a string to lowercase, in place
@m_since_latest

Replaces any character from `ABCDEFGHIJKLMNOPQRSTUVWXYZ` with a corresponding
character from `abcdefghijklmnopqrstuvwxyz`. Deliberately supports only ASCII
as Unicode-aware case conversion is a much more complex topic.
@see @ref lowercase()
*/
inline void lowercaseInPlace(Containers::MutableStringView string) {
    Implementation::lowercaseInPlace(string.data(), string.size());
}

/**
@brief Convert ASCII characters in a string to lowercase
@m_since_latest

Allocates a copy and replaces any character from `ABCDEFGHIJKLMNOPQRSTUVWXYZ`
with a corresponding character from `abcdefghijklmnopqrstuvwxyz`. Deliberately
supports only ASCII as Unicode-aware case conversion is a much more complex
topic.
@see @ref lowercaseInPlace()
*/
CORRADE_UTILITY_EXPORT Containers::String lowercase(Containers::StringView string);

/** @overload
@m_since_latest

Compared to @ref lowercase(Containers::StringView) is able to perform the
operation in-place if @p string is owned, transferring the data ownership to
the returned instance. Makes a owned copy first if not.
*/
CORRADE_UTILITY_EXPORT Containers::String lowercase(Containers::String string);

/**
@brief Convert ASCII characters in a string to uppercase, in place
@m_since_latest

Replaces any character from `abcdefghijklmnopqrstuvwxyz` with a corresponding
character from `ABCDEFGHIJKLMNOPQRSTUVWXYZ`. Deliberately supports only ASCII
as Unicode-aware case conversion is a much more complex topic.
@see @ref uppercase()
*/
inline void uppercaseInPlace(Containers::MutableStringView string) {
    Implementation::uppercaseInPlace(string.data(), string.size());
}

/**
@brief Convert ASCII characters in a string to uppercase, in place
@m_since_latest

Allocates a copy and replaces any character from `abcdefghijklmnopqrstuvwxyz`
with a corresponding character from `ABCDEFGHIJKLMNOPQRSTUVWXYZ`. Deliberately
supports only ASCII as Unicode-aware case conversion is a much more complex
topic.
@see @ref uppercaseInPlace()
*/
CORRADE_UTILITY_EXPORT Containers::String uppercase(Containers::StringView string);

/** @overload
@m_since_latest

Compared to @ref uppercase(Containers::StringView) is able to perform the
operation in-place if @p string is owned, transferring the data ownership to
the returned instance. Makes a owned copy first if not.
*/
CORRADE_UTILITY_EXPORT Containers::String uppercase(Containers::String string);

/**
@brief Whether the string has given prefix

In particular, returns @cpp true @ce for empty @p string only if @p prefix is
empty as well.
@see @ref stripPrefix(), @ref Containers::StringView::hasPrefix()
*/
CORRADE_UTILITY_EXPORT bool beginsWith(const std::string& string, const std::string& prefix);

/** @overload */
CORRADE_UTILITY_EXPORT bool beginsWith(const std::string& string, char prefix);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Whether string view has given prefix
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool viewBeginsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> prefix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool viewBeginsWith(Containers::ArrayView<const char> string, char prefix);
#endif

/**
@brief Whether the string has given suffix

In particular, returns @cpp true @ce for empty @p string only if @p suffix is
empty as well.
@see @ref stripSuffix(), @ref Containers::StringView::hasSuffix()
*/
CORRADE_UTILITY_EXPORT bool endsWith(const std::string& string, const std::string& suffix);

/** @overload */
CORRADE_UTILITY_EXPORT bool endsWith(const std::string& string, char suffix);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Whether string view has given suffix
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasSuffix() instead") bool viewEndsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> suffix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasSuffix() instead") bool viewEndsWith(Containers::ArrayView<const char> string, char suffix);
#endif

/**
@brief Strip given prefix from a string

Expects that the string actually begins with given prefix.
@see @ref beginsWith(), @ref Containers::StringView::exceptPrefix()
*/
CORRADE_UTILITY_EXPORT std::string stripPrefix(std::string string, const std::string& prefix);

/** @overload */
CORRADE_UTILITY_EXPORT std::string stripPrefix(std::string string, char prefix);

/**
@brief Strip given suffix from a string

Expects that the string actually ends with given suffix.
@see @ref endsWith(), @ref Containers::StringView::exceptSuffix()
*/
CORRADE_UTILITY_EXPORT std::string stripSuffix(std::string string, const std::string& suffix);

/** @overload */
CORRADE_UTILITY_EXPORT std::string stripSuffix(std::string string, char suffix);

/**
@brief Replace first occurrence in a string
@m_since_latest

Returns @p string unmodified if it doesn't contain @p search. Having empty
@p search causes @p replace to be prepended to @p string.
@see @ref replaceAll()
*/
CORRADE_UTILITY_EXPORT Containers::String replaceFirst(Containers::StringView string, Containers::StringView search, Containers::StringView replace);

/**
@brief Replace all occurrences in a string
@m_since_latest

Returns @p string unmodified if it doesn't contain @p search. Expects that
@p search is not empty, as that would cause an infinite loop. For substituting
a single character with another the @ref replaceAll(Containers::String, char, char)
variant is more optimal.
@see @ref replaceFirst()
*/
CORRADE_UTILITY_EXPORT Containers::String replaceAll(Containers::StringView string, Containers::StringView search, Containers::StringView replace);

/**
@brief Replace all occurrences of a character in a string with another character
@m_since_latest

The @p string is passed through unmodified if it doesn't contain @p search.
Otherwise the operation is performed in-place if @p string is owned,
transferring the data ownership to the returned instance. An owned copy is made
if not. See also @ref replaceAllInPlace() for a variant that operates on string
views.
@see @ref replaceAll(Containers::StringView, Containers::StringView, Containers::StringView),
    @ref replaceFirst()
*/
CORRADE_UTILITY_EXPORT Containers::String replaceAll(Containers::String string, char search, char replace);

namespace Implementation {
    CORRADE_UTILITY_EXPORT extern void CORRADE_UTILITY_CPU_DISPATCHED_DECLARATION(replaceAllInPlaceCharacter)(char* data, std::size_t size, char search, char replace);
    CORRADE_UTILITY_CPU_DISPATCHER_DECLARATION(replaceAllInPlaceCharacter)
}

/**
@brief Replace all occurrences of a character in a string with another character in-place
@m_since_latest

@see @ref replaceAll(Containers::String, char, char),
    @ref replaceAll(Containers::StringView, Containers::StringView, Containers::StringView),
    @ref replaceFirst()
*/
inline void replaceAllInPlace(const Containers::MutableStringView string, const char search, const char replace) {
    Implementation::replaceAllInPlaceCharacter(string.data(), string.size(), search, replace);
}

/**
@brief Parse a number sequence
@m_since_latest

Parses a string containing a sequence of numbers, returning them converted to
integers. The numbers can be delimited by commas (`,`), semicolons (`;`) or
an arbitrary whitespace character. Order in which the numbers were specified is
kept in the output including possible duplicates. Empty string results in an
empty array returned.

Additionally it's possible to specify a range using the `-` character, in which case the range will be expanded in the output. The range is inclusive, meaning
`3-6` will result in @cpp {3, 4, 5, 6} @ce in the output. Ranges where the end
is smaller than the start (such as `6-3`) will be treated as empty. If the
number before the `-` is omitted, a @p min is used instead; if the number after
the `-` is omitted, @cpp max - 1 @ce is used instead.

If an unrecognized character is encountered, the function prints an error and
returns a @relativeref{Corrade,Containers::NullOpt}. If any parsed number is
less than @p min, greater than or equal to @p max or doesn't fit into 32 bits,
it's omitted in the output.

Example usage:

-   `4,3 5;5;17` results in @cpp {4, 3, 5, 5, 17} @ce
-   `12-,3-5,1` with @p max set to @cpp 15 @ce results in
    @cpp {12, 13, 14, 3, 4, 5, 1} @ce
-   `-3, 13-` with @p min set to @cpp 0 @ce and @p max to @cpp 15 @ce results
    in @cpp {0, 1, 2, 3, 13, 14} @ce
-   any input with @p min set to @cpp 0 @ce and @p max set to @cpp 0 @ce
    results in an empty output
-   `-` results in a range from @p min to @cpp max - 1 @ce
*/
CORRADE_UTILITY_EXPORT Containers::Optional<Containers::Array<std::uint32_t>> parseNumberSequence(Containers::StringView string, std::uint32_t min, std::uint32_t max);

}}}

#endif
