#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
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

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/StlForwardString.h"
#include "Corrade/Utility/StlForwardVector.h"
#endif

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
@brief String parse state
@m_since_latest

Returned as part of @ref ParseResult from @ref parseDecimal() and
@ref parseHexadecimal().
*/
enum class ParseState: std::uint8_t {
    /**
     * Parsing succeeded with no information loss, i.e. the value can fit into
     * the desired range without being clamped.
     */
    Success,

    /**
     * Parsing succeeded but the parsed value had to be clamped to fit into the
     * desired range. The output value is set to the appropriate min or max
     * value of given range.
     */
    Clamped,

    /**
     * Parsing the value failed, for example because it contains invalid
     * characters. The output value is left in an unspecified state in this
     * case, @ref ParseResult::index() contains the index of a byte on which a
     * parsing failure happened.
     */
    Failed
};

/**
@debugoperatorenum{ParseState}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ParseState value);

/**
@brief String parse result
@m_since_latest

Stores a @ref ParseState and an optional byte index of where a parsing failure
happened, returned from @ref parseDecimal() and @ref parseHexadecimal(). The
instance is implicitly convertible to a @ref ParseState, allowing you to use
just the enum if details about parsing failure aren't needed for anything:

@snippet Utility.cpp ParseResult-enum-conversion
*/
class ParseResult {
    public:
        /** @brief Constructor */
        /*implicit*/ ParseResult(ParseState state, std::size_t index = 0): _state{state}, _index{index} {}

        /** @brief Parse state */
        ParseState state() const { return _state; }

        /** @brief Parse state */
        /*implicit*/ operator ParseState() const { return _state; }

        /**
         * @brief Parse failure byte index
         *
         * Index of a byte in the parsed string on which a parsing failure
         * happened. Has a meaningful value only for @ref ParseState::Failed,
         * otherwise the value is @cpp 0 @ce.
         */
        std::size_t index() const { return _index; }

    private:
        ParseState _state;
        std::size_t _index;
};

/**
@brief Decimal string parse flag
@m_since_latest

@see @ref ParseDecimalFlags, @ref parseDecimal(), @ref ParseHexadecimalFlag
*/
enum class ParseDecimalFlag: std::uint8_t {
    /**
     * Disallow `-` and `+` sign in front of the number. Note that, unlike with
     * @ref std::strtoull(), which silently accepts negative numbers and wraps
     * them around, for an unsigned output type a `-` is never allowed
     * regardless of this flag being present.
     */
    DisallowSign = 1 << 0
};

/**
@debugoperatorenum{ParseDecimalFlag}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ParseDecimalFlag value);

/**
@brief Decimal string parse flags
@m_since_latest

@see @ref parseDecimal(), @ref ParseHexadecimalFlags
*/
typedef Containers::EnumSet<ParseDecimalFlag> ParseDecimalFlags;

CORRADE_ENUMSET_OPERATORS(ParseDecimalFlags)

/**
@debugoperatorenum{ParseDecimalFlags}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ParseDecimalFlags value);

/**
@brief Parse a string containing an unsigned decimal number
@param[in]  string      Input string
@param[out] value       Output value
@param[in]  min         Minimal allowed value
@param[in]  max         Maximal allowed value
@param[in]  flags       Flags
@return Parse state
@m_since_latest

If the @p string is a decimal numeric value, optionally prepended with a `+`
sign unless @ref ParseDecimalFlag::DisallowSign is set, parses it into
@p value and returns @ref ParseState::Success if the value fits into the range
defined by @p min and @p max. If the value doesn't fit into the range defined
by @p min and @p max, returns @ref ParseState::Clamped and @p value is set to
either @p min or @p max as appropriate. If the string isn't a valid number or
has non-numeric characters before or after, returns @ref ParseState::Failed,
with @ref ParseResult::index() pointing to the byte at which a parsing failure
happened, and @p value left in an unspecified state.

The string can have any number of leading zeros after the sign, unlike
@ref std::strtoull() a leading zero *never* causes the number to be interpreted
as octal, and a `0x` or `0X` prefix is treated as a parsing failure.

Expects that @p min is less or equal to @p max. Common usage is through one of
the type-specific overloads such as @ref parseDecimal(Containers::StringView, std::uint32_t&, ParseDecimalFlags)
which have the min and max values implicit based on the type. Example usage:

@snippet Utility.cpp parseDecimal-unsigned

If clamping / overflow doesn't need to be handled, it's enough to check that
the function doesn't return @ref ParseState::Failed.

Note that in comparison to @ref std::strtoull(), which accepts negative numbers
and wraps them around, this function returns @ref ParseState::Failed for any
number with a `-` sign. Furthermore, the function *does not* discard any
whitespace characters around the number --- if you need to do so, pass the
@p string as @relativeref{Containers::BasicStringView,trimmed()}, with
@relativeref{Containers::BasicStringView,trimmedPrefix()} or with
@relativeref{Containers::BasicStringView,trimmedSuffix()}:

@snippet Utility.cpp parseDecimal-unsigned-trimmed

@see @ref parseHexadecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseHexadecimalFlags)
*/
CORRADE_UTILITY_EXPORT ParseResult parseDecimal(Containers::StringView string, std::uint64_t& value, std::uint64_t min, std::uint64_t max, ParseDecimalFlags flags = {});

/**
@brief Parse a string containing a signed decimal number
@param[in]  string      Input string
@param[out] value       Output value
@param[in]  min         Minimal allowed value
@param[in]  max         Maximal allowed value
@param[in]  flags       Flags
@return Parse state
@m_since_latest

If the @p string is a decimal numeric value, optionally prepended with a `+` or
`-` sign unless @ref ParseDecimalFlag::DisallowSign is set, parses it into
@p value and returns @ref ParseState::Success if the value fits into the range
defined by @p min and @p max. If the value doesn't fit into the range defined
by @p min and @p max, returns @ref ParseState::Clamped and @p value is set to
either @p min or @p max as appropriate. If the string isn't a valid number or
has non-numeric characters before or after, returns @ref ParseState::Failed,
with @ref ParseResult::index() pointing to the byte at which a parsing failure
happened, and @p value left in an unspecified state.

The string can have any number of leading zeros after the sign, if any, unlike
@ref std::strtoull() a leading zero *never* causes the number to be interpreted
as octal, and a `0x` or `0X` prefix is treated as a parsing failure.

Expects that @p min is less or equal to @p max. Common usage is through one of
the type-specific overloads such as @ref parseDecimal(Containers::StringView, std::int32_t&, ParseDecimalFlags)
which have the min and max values implicit based on the type. Example usage:

@snippet Utility.cpp parseDecimal-signed

If clamping / overflow doesn't need to be handled, it's enough to check that
the function doesn't return @ref ParseState::Failed.

Note that in comparison to @ref std::strtoll(), the function *does not* discard
any whitespace characters around the number --- if you need to do so, pass the
@p string as @relativeref{Containers::BasicStringView,trimmed()}, with
@relativeref{Containers::BasicStringView,trimmedPrefix()} or with
@relativeref{Containers::BasicStringView,trimmedSuffix()}:

@snippet Utility.cpp parseDecimal-signed-trimmed

@see @ref parseHexadecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseHexadecimalFlags)
*/
CORRADE_UTILITY_EXPORT ParseResult parseDecimal(Containers::StringView string, std::int64_t& value, std::int64_t min, std::int64_t max, ParseDecimalFlags flags = {});

/**
@brief Parse a string containing an unsigned 8-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseDecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to @cpp 255 @ce and converting the
@p value to a 8-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::uint8_t& value, ParseDecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, 0, UINT8_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 8-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseDecimalFlags)
with @p min set to @cpp -128 @ce and @p max set to @cpp 127 @ce and converting
the @p value to a 8-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::int8_t& value, ParseDecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, INT8_MIN, INT8_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 16-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseDecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to @cpp 65535 @ce and converting
the @p value to a 16-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::uint16_t& value, ParseDecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, 0, UINT16_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 16-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseDecimalFlags)
with @p min set to @cpp -32768 @ce and @p max set to @cpp 32767 @ce and
converting the @p value to a 16-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::int16_t& value, ParseDecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, INT16_MIN, INT16_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 32-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseDecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to a max representable unsigned
32-bit value and converting the @p value to a 32-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::uint32_t& value, ParseDecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, 0, UINT32_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 32-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseDecimalFlags)
with @p min and @p max set to a min and max representable signed 32-bit value
and converting the @p value to a 32-bit type.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::int32_t& value, ParseDecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, INT32_MIN, INT32_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 64-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseDecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to a max representable unsigned
64-bit value.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::uint64_t& value, ParseDecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, 0, UINT64_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 64-bit decimal number
@m_since_latest

Equivalent to calling @ref parseDecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseDecimalFlags)
with @p min and @p max set to a min and max representable signed 64-bit value.
*/
inline ParseResult parseDecimal(Containers::StringView string, std::int64_t& value, ParseDecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseDecimal(string, parsed, INT64_MIN, INT64_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Hexadecimal string parse flag
@m_since_latest

@see @ref ParseHexadecimalFlags, @ref parseHexadecimal(), @ref ParseDecimalFlag
*/
enum class ParseHexadecimalFlag: std::uint8_t {
    /**
     * Disallow `-` and `+` sign in front of the number. Note that, unlike with
     * @ref std::strtoull(), which silently accepts negative numbers and wraps
     * them around, for an unsigned output type a `-` is never allowed
     * regardless of this flag being present.
     */
    DisallowSign = 1 << 0,

    /**
     * Allow also a `0x` or `0X` prefix in front of the number and after the
     * sign, if any. If neither @ref ParseHexadecimalFlag::AllowBasePrefix nor
     * @relativeref{ParseHexadecimalFlag,AllowHashPrefix} is set, no prefix is
     * allowed.
     */
    AllowBasePrefix = 1 << 1,

    /**
     * Allow also a `#` prefix in front of the number and after the sign, if
     * any, such as for a hexadecimal color representation. If neither
     * @ref ParseHexadecimalFlag::AllowBasePrefix nor
     * @relativeref{ParseHexadecimalFlag,AllowHashPrefix} is set, no prefix is
     * allowed.
     */
    AllowHashPrefix = 1 << 2,
};

/**
@debugoperatorenum{ParseHexadecimalFlag}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ParseHexadecimalFlag value);

/**
@brief Hexadecimal string parse flags
@m_since_latest

@see @ref parseHexadecimal(), @ref ParseDecimalFlags
*/
typedef Containers::EnumSet<ParseHexadecimalFlag> ParseHexadecimalFlags;

CORRADE_ENUMSET_OPERATORS(ParseHexadecimalFlags)

/**
@debugoperatorenum{ParseHexadecimalFlags}
@m_since_latest
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, ParseHexadecimalFlags value);

/**
@brief Parse a string containing an unsigned hexadecimal number
@param[in]  string      Input string
@param[out] value       Output value
@param[in]  min         Minimal allowed value
@param[in]  max         Maximal allowed value
@param[in]  flags       Flags
@return Parse state
@m_since_latest

If the @p string is a hexadecimal numeric value, optionally prepended with a
`+` sign unless @ref ParseHexadecimalFlag::DisallowSign is set, parses it into
@p value and returns @ref ParseState::Success if the value fits into the range
defined by @p min and @p max. If the value doesn't fit into the range defined
by @p min and @p max, returns @ref ParseState::Clamped and @p value is set to
either @p min or @p max as appropriate. If the string isn't a valid number or
has non-hexadecimal characters before or after, returns
@ref ParseState::Failed, with @ref ParseResult::index() pointing to the byte at
which a parsing failure happened, and @p value left in an unspecified state.

Both lowercase and uppercase hexadecimal characters are accepted. By default no
prefix is allowed, pass @ref ParseHexadecimalFlag::AllowBasePrefix to accept
also numbers prefixed with `0x` or `0X` after the sign, if any, and
@ref ParseHexadecimalFlag::AllowHashPrefix to accept also numbers prefixed with
a `#` character after the sign, if any, such as for a hexadecimal color
representation. The string can have any number of leading zeros after the sign
and prefix, if any, unlike @ref std::strtoull() an omitted prefix or a leading
zero *never* causes the number to be interpreted as decimal or octal.

Expects that @p min is less or equal to @p max. Common usage is through one of
the type-specific overloads such as @ref parseHexadecimal(Containers::StringView, std::uint32_t&, ParseHexadecimalFlags)
which have the min and max values implicit based on the type. Example usage:

@snippet Utility.cpp parseHexadecimal-unsigned

If clamping / overflow doesn't need to be handled, it's enough to check that
the function doesn't return @ref ParseState::Failed.

Note that in comparison to @ref std::strtoull(), which accepts negative numbers
and wraps them around, this function returns @ref ParseState::Failed for any
number with a `-` sign. Furthermore, the function *does not* discard any
whitespace characters around the number --- if you need to do so, pass the
@p string as @relativeref{Containers::BasicStringView,trimmed()}, with
@relativeref{Containers::BasicStringView,trimmedPrefix()} or with
@relativeref{Containers::BasicStringView,trimmedSuffix()}:

@snippet Utility.cpp parseHexadecimal-unsigned-trimmed

@see @ref parseDecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseDecimalFlags)
*/
CORRADE_UTILITY_EXPORT ParseResult parseHexadecimal(Containers::StringView string, std::uint64_t& value, std::uint64_t min, std::uint64_t max, ParseHexadecimalFlags flags = {});

/**
@brief Parse a string containing a signed hexadecimal number
@param[in]  string      Input string
@param[out] value       Output value
@param[in]  min         Minimal allowed value
@param[in]  max         Maximal allowed value
@param[in]  flags       Flags
@return Parse state
@m_since_latest

If the @p string is a hexadecimal numeric value, optionally prepended with a
`+` or `-` sign unless @ref ParseDecimalFlag::DisallowSign is set, parses it
into @p value and returns @ref ParseState::Success if the value fits into the
range defined by @p min and @p max. If the value doesn't fit into the range
defined by @p min and @p max, returns @ref ParseState::Clamped and @p value is
set to either @p min or @p max as appropriate. If the string isn't a valid
number or has non-hexadecimal characters before or after, returns
@ref ParseState::Failed, with @ref ParseResult::index() pointing to the byte at
which a parsing failure happened, and @p value left in an unspecified state.

Both lowercase and uppercase hexadecimal characters are accepted. By default no
prefix is allowed, pass @ref ParseHexadecimalFlag::AllowBasePrefix to accept
also numbers prefixed with `0x` or `0X` after the sign, and
@ref ParseHexadecimalFlag::AllowHashPrefix to accept also numbers prefixed with
a `#` character after the sign, such as for hexadecimal color representation.
The string can have any number of leading zeros after the sign and prefix,
unlike @ref std::strtoull() an omitted prefix or a leading zero *never* causes
the number to be interpreted as decimal or octal.

Expects that @p min is less or equal to @p max. Common usage is through one of
the type-specific overloads such as @ref parseHexadecimal(Containers::StringView, std::int32_t&, ParseHexadecimalFlags)
which have the min and max values implicit based on the type. Example usage:

@snippet Utility.cpp parseHexadecimal-signed

If clamping / overflow doesn't need to be handled, it's enough to check that
the function doesn't return @ref ParseState::Failed.

Note that in comparison to @ref std::strtoll(), the function *does not* discard
any whitespace characters around the number --- if you need to do so, pass the
@p string as @relativeref{Containers::BasicStringView,trimmed()}, with
@relativeref{Containers::BasicStringView,trimmedPrefix()} or with
@relativeref{Containers::BasicStringView,trimmedSuffix()}:

@snippet Utility.cpp parseHexadecimal-signed-trimmed

@see @ref parseDecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseDecimalFlags)
*/
CORRADE_UTILITY_EXPORT ParseResult parseHexadecimal(Containers::StringView string, std::int64_t& value, std::int64_t min, std::int64_t max, ParseHexadecimalFlags flags = {});

/**
@brief Parse a string containing an unsigned 8-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseHexadecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to @cpp 255 @ce and converting the
@p value to a 8-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::uint8_t& value, ParseHexadecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, 0, UINT8_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 8-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseHexadecimalFlags)
with @p min set to @cpp -128 @ce and @p max set to @cpp 127 @ce and converting
the @p value to a 8-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::int8_t& value, ParseHexadecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, INT8_MIN, INT8_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 16-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseHexadecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to @cpp 65535 @ce and converting
the @p value to a 16-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::uint16_t& value, ParseHexadecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, 0, UINT16_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 16-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseHexadecimalFlags)
with @p min set to @cpp -32768 @ce and @p max set to @cpp 32767 @ce and
converting the @p value to a 16-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::int16_t& value, ParseHexadecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, INT16_MIN, INT16_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 32-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseHexadecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to a max representable unsigned
32-bit value and converting the @p value to a 32-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::uint32_t& value, ParseHexadecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, 0, UINT32_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 32-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseHexadecimalFlags)
with @p min and @p max set to a min and max representable signed 32-bit value
and converting the @p value to a 32-bit type.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::int32_t& value, ParseHexadecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, INT32_MIN, INT32_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing an unsigned 64-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::uint64_t&, std::uint64_t, std::uint64_t, ParseHexadecimalFlags)
with @p min set to @cpp 0 @ce and @p max set to a max representable unsigned
64-bit value.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::uint64_t& value, ParseHexadecimalFlags flags = {}) {
    std::uint64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, 0, UINT64_MAX, flags);
    value = parsed;
    return result;
}

/**
@brief Parse a string containing a signed 64-bit hexadecimal number
@m_since_latest

Equivalent to calling @ref parseHexadecimal(Containers::StringView, std::int64_t&, std::int64_t, std::int64_t, ParseHexadecimalFlags)
with @p min and @p max set to a min and max representable signed 64-bit value.
*/
inline ParseResult parseHexadecimal(Containers::StringView string, std::int64_t& value, ParseHexadecimalFlags flags = {}) {
    std::int64_t parsed;
    const ParseResult result = parseHexadecimal(string, parsed, INT64_MIN, INT64_MAX, flags);
    value = parsed;
    return result;
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

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Safely construct string from char array

If @p string is @cpp nullptr @ce, returns empty string.
@m_deprecated_since_latest Use @ref Containers::StringView instead, it treats
    @cpp nullptr @ce as an empty string on its own
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView instead") std::string fromArray(const char* string);

/**
@brief Safely construct string from char array with explicit length

If @p string is @cpp nullptr @ce, returns empty string. Otherwise takes also
@p length into account.
@m_deprecated_since_latest Use @ref Containers::StringView instead, it treats
    @cpp nullptr @ce as an empty string on its own
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView instead") std::string fromArray(const char* string, std::size_t length);

/**
@brief Trim leading characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Implemented using @ref ltrimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedPrefix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedPrefix() instead") std::string ltrim(std::string string, const std::string& characters);

/**
@brief Trim leading whitespace from string

Equivalent to calling @ref ltrim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref ltrimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedPrefix() instead") std::string ltrim(std::string string);

/**
@brief Trim trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Implemented using @ref rtrimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedSuffix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedSuffix() instead") std::string rtrim(std::string string, const std::string& characters);

/**
@brief Trim trailing whitespace from string

Equivalent to calling @ref rtrim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using @ref trimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedSuffix() instead") std::string rtrim(std::string string);

/**
@brief Trim leading and trailing characters from string
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to @cpp ltrim(rtrim(string, characters), characters) @ce.
Implemented using @ref trimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmed(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmed() instead") std::string trim(std::string string, const std::string& characters);

/**
@brief Trim leading and trailing whitespace from string

Equivalent to calling @ref trim(std::string, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter. Implemented using
@ref trimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmed() instead.
*/
CORRADE_UTILITY_EXPORT  CORRADE_DEPRECATED("use Containers::StringView::trimmed() instead") std::string trim(std::string string);

/**
@brief Trim leading characters from a string, in place
@param string       String to be trimmed in place
@param characters   Characters which will be trimmed

@m_deprecated_since_latest Use @ref Containers::StringView::trimmedPrefix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedPrefix() instead") void ltrimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim leading whitespace from a string, in place

Equivalent to calling @ref ltrimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedPrefix() instead") void ltrimInPlace(std::string& string);

/**
@brief Trim trailing characters from a string, in place
@param string       String to be trimmed
@param characters   Characters which will be trimmed

@m_deprecated_since_latest Use @ref Containers::StringView::trimmedSuffix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedSuffix() instead") void rtrimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim trailing whitespace from a string, in place

Equivalent to calling @ref rtrimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@m_deprecated_since_latest Use @ref Containers::StringView::trimmedSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmedSuffix() instead") void rtrimInPlace(std::string& string);

/**
@brief Trim leading and trailing characters from a string, in place
@param string       String to be trimmed
@param characters   Characters which will be trimmed

Equivalent to calling both @ref ltrimInPlace() and @ref rtrimInPlace().
@m_deprecated_since_latest Use @ref Containers::StringView::trimmed(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmed() instead") void trimInPlace(std::string& string, const std::string& characters);

/**
@brief Trim leading and trailing whitespace from a string, in place

Equivalent to calling @ref trimInPlace(std::string&, const std::string&) with
@cpp " \t\f\v\r\n" @ce as second parameter.
@m_deprecated_since_latest Use @ref Containers::StringView::trimmed() instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::trimmed() instead") void trimInPlace(std::string& string);

/**
@brief Split a string on given character
@param string       String to split
@param delimiter    Delimiter

@m_deprecated_since_latest Use @ref Containers::StringView::split(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::split() instead") std::vector<std::string> split(const std::string& string, char delimiter);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::split(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::split() instead") Containers::Array<Containers::StringView> split(Containers::StringView string, char delimiter);

/**
@brief Split a string on given character and remove empty parts
@param string       String to split
@param delimiter    Delimiter

@m_deprecated_since_latest Use
    @ref Containers::StringView::splitWithoutEmptyParts(char) const instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitWithoutEmptyParts() instead") std::vector<std::string> splitWithoutEmptyParts(const std::string& string, char delimiter);

/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitWithoutEmptyParts(char) const instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(Containers::StringView string, char delimiter);

/**
@brief Split a string on any character from given set and remove empty parts
@param string       String to split
@param delimiters   Delimiter characters

@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnAnyWithoutEmptyParts(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnAnyWithoutEmptyParts() instead") std::vector<std::string> splitWithoutEmptyParts(const std::string& string, const std::string& delimiters);

/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnAnyWithoutEmptyParts(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnAnyWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(Containers::StringView string, Containers::StringView delimiters);

/**
@brief Split a string on whitespace and remove empty parts

Equivalent to calling @ref splitWithoutEmptyParts(const std::string&, const std::string&)
with @cpp " \t\f\v\r\n" @ce as second parameter.
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnAnyWithoutEmptyParts() instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnAnyWithoutEmptyParts() instead") std::vector<std::string> splitWithoutEmptyParts(const std::string& string);

/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::splitOnWhitespaceWithoutEmptyParts() const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::splitOnWhitespaceWithoutEmptyParts() instead") Containers::Array<Containers::StringView> splitWithoutEmptyParts(const Containers::StringView string);

/**
@brief Partition a string

Equivalent to Python's @m_class{m-doc-external} [str.partition()](https://docs.python.org/3/library/stdtypes.html#str.partition).
Splits @p string at the first occurrence of @p separator. First returned value
is the part before the separator, second the separator, third a part after the
separator. If the separator is not found, returns the input string followed by
two empty strings.
@m_deprecated_since_latest Use @ref Containers::StringView::partition(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::partition() instead") Containers::StaticArray<3, std::string> partition(const std::string& string, char separator);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::partition(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::partition() instead") Containers::StaticArray<3, std::string> partition(const std::string& string, const std::string& separator);

/**
@brief Right-partition a string

Equivalent to Python's @m_class{m-doc-external} [str.rpartition()](https://docs.python.org/3/library/stdtypes.html#str.rpartition).
Splits @p string at the last occurrence of @p separator. First returned value is
the part before the separator, second the separator, third a part after the
separator. If the separator is not found, returns two empty strings followed by
the input string.
@m_deprecated_since_latest Use @ref Containers::StringView::partitionLast(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::partitionLast() instead") Containers::StaticArray<3, std::string> rpartition(const std::string& string, char separator);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::partitionLast(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::partitionLast() instead") Containers::StaticArray<3, std::string> rpartition(const std::string& string, const std::string& separator);

/**
@brief Join strings with given character
@param strings      Strings to join
@param delimiter    Delimiter

@m_deprecated_since_latest Use @ref Containers::StringView::join() instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::join() instead") std::string join(const std::vector<std::string>& strings, char delimiter);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::join() instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::join() instead") std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

/**
@brief Join strings with given character and remove empty parts
@param strings      Strings to join
@param delimiter    Delimiter

@m_deprecated_since_latest Use
    @ref Containers::StringView::joinWithoutEmptyParts() instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::joinWithoutEmptyParts() instead") std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, char delimiter);

/**
@overload
@m_deprecated_since_latest Use
    @ref Containers::StringView::joinWithoutEmptyParts() instead
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::joinWithoutEmptyParts() instead") std::string joinWithoutEmptyParts(const std::vector<std::string>& strings, const std::string& delimiter);

/**
@brief Whether the string has given prefix

In particular, returns @cpp true @ce for empty @p string only if @p prefix is
empty as well.
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool beginsWith(const std::string& string, const std::string& prefix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool beginsWith(const std::string& string, char prefix);

/**
@brief Whether string view has given prefix
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool viewBeginsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> prefix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasPrefix(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasPrefix() instead") bool viewBeginsWith(Containers::ArrayView<const char> string, char prefix);

/**
@brief Whether the string has given suffix

In particular, returns @cpp true @ce for empty @p string only if @p suffix is
empty as well.
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::endsWith() instead") bool endsWith(const std::string& string, const std::string& suffix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::endsWith() instead") bool endsWith(const std::string& string, char suffix);

/**
@brief Whether string view has given suffix
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix(StringView) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasSuffix() instead") bool viewEndsWith(Containers::ArrayView<const char> string, Containers::ArrayView<const char> suffix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::hasSuffix(char) const
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::hasSuffix() instead") bool viewEndsWith(Containers::ArrayView<const char> string, char suffix);

/**
@brief Strip given prefix from a string

Expects that the string actually begins with given prefix.
@m_deprecated_since_latest Use @ref Containers::StringView::exceptPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::exceptPrefix() instead") std::string stripPrefix(std::string string, const std::string& prefix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::exceptPrefix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::exceptPrefix() instead") std::string stripPrefix(std::string string, char prefix);

/**
@brief Strip given suffix from a string

Expects that the string actually ends with given suffix.
@m_deprecated_since_latest Use @ref Containers::StringView::exceptSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::exceptSuffix() instead") std::string stripSuffix(std::string string, const std::string& suffix);

/**
@overload
@m_deprecated_since_latest Use @ref Containers::StringView::exceptSuffix()
    instead.
*/
CORRADE_UTILITY_EXPORT CORRADE_DEPRECATED("use Containers::StringView::exceptSuffix() instead") std::string stripSuffix(std::string string, char suffix);
#endif

}}}

#endif
