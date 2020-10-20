#ifndef Corrade_Utility_TweakableParser_h
#define Corrade_Utility_TweakableParser_h
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
 * @brief Class @ref Corrade::Utility::TweakableParser, enum @ref Corrade::Utility::TweakableState
 */

#include <utility>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/** @relatesalso Tweakable
@brief Parser for @ref Tweakable types

Support for basic types that are expressible with plain literals is implemented
in @ref TweakableParser<int>, @ref TweakableParser<unsigned int>,
@ref TweakableParser<long>, @ref TweakableParser<unsigned long>,
@ref TweakableParser<long long>, @ref TweakableParser<unsigned long long>,
@ref TweakableParser<float>, @ref TweakableParser<double>,
@ref TweakableParser<long double> and @ref TweakableParser<char>.

@section TweakableParser-subclassing Implementing support for custom literals

The parser support is limited to what's expressible with C++11 user-defined
literals, together with an optional unary @cpp + @ce or @cpp - @ce in front.
Current implementation supports only trivially copyable and trivially
destructible types of a limited size --- in particular, saving strings is not
possible.

In order to implement support for a custom type, create a (partial) template
specialization with a @cpp parse() @ce function with the following sigature
for given type `T` --- assuming there's a user-defined C++11 literal that
returns `T` as well:

@snippet Utility.cpp TweakableParser

The function gets a view onto the contents of the annotation, with outer
whitespace stripped (so e.g. for string literals you get also the quotes
around). It returns the parsed value and a parser state. The state should be
one of the following:

-   @ref TweakableState::Success if parsing consumed the whole input and there
    was no error
-   @ref TweakableState::Error if parsing failed and the parser is *sure* that
    this is a compile error (and not suddenly a different type, for example
    --- @cpp 0xa901g0f @ce might look like an error, but it could be also a
    user-defined literal `g0f`). Examples of such errors can be unterminated
    string/char literals or unknown escape characters in them.
-   @ref TweakableState::Recompile if parsing failed and the parser is *not*
    sure that this is a compile error

Returning @ref TweakableState::NoChange is not allowed.

Note that the user-defined literal has to return a custom type that's not
already handled by the implementation. So for example a custom C++11 binary
literal @cpp 110110110_b @ce, returning @cpp int @ce and supplementing the
builtin C++14 literal @cpp 0b110110110 @ce, wouldn't be possible to implement,
since @ref TweakableParser<int> is already defined.

@experimental
*/
template<class T> struct TweakableParser;

/**
@brief Tweakable state

@see @ref Tweakable::update()
@experimental
*/
enum class TweakableState: std::uint8_t {
    /**
     * No source file has any changes that affect tweakable values. Nothing to
     * do.
     */
    NoChange = 0,

    /**
     * Tweakable values in some source files were changed and successfully
     * updated. Values that are neither accessed in the main event loop nor
     * were part of any @ref Tweakable::scope() call should be updated manually
     * on the caller side.
     */
    Success = 1,

    /**
     * Source files were changed in a way that can't be handled by updating
     * just the tweakable values alone. No values were updated, hot-reload the
     * affected code or restart a recompiled version of the app to pick up the
     * changes.
     *
     * Note that this state is optimistic --- it may happen that the changes
     * will lead to a compile error similarly as with @ref TweakableState::Error,
     * but detecting that is out of scope of this utility.
     */
    Recompile = 2,

    /**
     * Source files were changed in a way that caused a parse error. No values
     * were updated, fix the error and save the file again to retry the
     * parsing.
     *
     * This state is returned only when the utility is absolutely sure there is
     * a syntax error (for example when a char literal is not terminated). If
     * not sure, @ref TweakableState::Recompile is returned (and then the
     * compiler might or might not report an error).
     */
    Error = 3
};

/** @debugoperatorenum{TweakableState} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, TweakableState value);

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `int` type

Expects literals in the form @cpp 42 @ce, @cpp 0x2a @ce, @cpp 052 @ce or
@cpp 0b101010 @ce, case-insensitive, with no suffixes. Unary @cpp + @ce or
@cpp - @ce is allowed. C++14 group separator @c ' is not supported at the
moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<int> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, int> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `unsigned int` type

Expects literals in the form @cpp 42u @ce, @cpp 0x2au @ce, @cpp 052u @ce or
@cpp 0b101010u @ce, case-insensitive. The `u` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed. C++14 group separator @c ' is not
supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<unsigned int> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, unsigned int> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `long int` type

Expects literals in the form @cpp 42l @ce, @cpp 0x2al @ce, @cpp 052l @ce or
@cpp 0b101010l @ce, case-insensitive. The `l` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed. C++14 group separator @c ' is not
supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<long> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, long> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `unsigned long int` type

Expects literals in the form @cpp 42ul @ce, @cpp 0x2aul @ce, @cpp 052ul @ce or
@cpp 0b101010ul @ce, case-insensitive. The `ul` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed. C++14 group separator @c ' is not
supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<unsigned long> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, unsigned long> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `long long int` type

Expects literals in the form @cpp 42ll @ce, @cpp 0x2all @ce, @cpp 052ll @ce or
@cpp 0b101010ll @ce, case-insensitive. The `ll` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed. C++14 group separator @c ' is not
supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<long long> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, long long> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `unsigned long long int` type

Expects literals in the form @cpp 42ull @ce, @cpp 0x2aull @ce, @cpp 052ull @ce
or @cpp 0b101010ull @ce, case-insensitive. The `ull` suffix is *not* optional,
unary @cpp + @ce or @cpp - @ce is allowed. C++14 group separator @c ' is not
supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<unsigned long long> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, unsigned long long> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `float` type

Expects literals in the form @cpp 0.42f @ce, @cpp 4.2e-1f @ce, @cpp .42f @ce
and variants, case-insensitive. The `f` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<float> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, float> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `double` type

Expects literals in the form @cpp 0.42 @ce, @cpp 4.2e-1 @ce, @cpp .42 @ce
and variants, case-insensitive, with no suffixes. Unary @cpp + @ce or
@cpp - @ce is allowed.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<double> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, double> parse(Containers::StringView value);
};

#ifndef CORRADE_TARGET_EMSCRIPTEN
/** @relatesalso Tweakable
@brief Tweakable constant parser for the `long double` type

Expects literals in the form @cpp 0.42l @ce, @cpp 4.2e-1l @ce, @cpp .42l @ce
and variants, case-insensitive. The `l` suffix is *not* optional, unary
@cpp + @ce or @cpp - @ce is allowed.
@experimental
@partialsupport Not available in @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"
    as JavaScript doesn't support doubles larger than 64 bits.
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<long double> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, long double> parse(Containers::StringView value);
};
#endif

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `char` type

Expects literals in the form @cpp 'a' @ce. Escape characters other than
<tt>\'</tt> and unicode char literals are not supported at the moment.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<char> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, char> parse(Containers::StringView value);
};

/** @relatesalso Tweakable
@brief Tweakable constant parser for the `bool` type

Expects literals in the form @cpp true @ce or @cpp false @ce.
@experimental
*/
template<> struct CORRADE_UTILITY_EXPORT TweakableParser<bool> {
    TweakableParser() = delete;

    /** @brief Parse the value */
    static std::pair<TweakableState, bool> parse(Containers::StringView value);
};

}}

#endif

