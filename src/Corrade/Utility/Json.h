#ifndef Corrade_Utility_Json_h
#define Corrade_Utility_Json_h
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
 * @brief Class @ref Corrade::Utility::Json, @ref Corrade::Utility::JsonToken, @ref Corrade::Utility::JsonTokenData, @ref Corrade::Utility::JsonObjectItem, @ref Corrade::Utility::JsonArrayItem, @ref Corrade::Utility::JsonIterator, @ref Corrade::Utility::JsonObjectIterator, @ref Corrade::Utility::JsonArrayIterator, @ref Corrade::Utility::JsonView, @ref Corrade::Utility::JsonObjectView, @ref Corrade::Utility::JsonArrayView
 * @m_since_latest
 */

#include <cstdint>

#include "Corrade/Containers/Pointer.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation { struct JsonData; }

/**
@brief JSON parser
@m_since_latest

Tokenizes a JSON file together with optional parsing of selected token
subtrees. Supports files over 4 GB and parsing of numeric values into 32-bit
floating-point, 32-bit and 52-/53-bit unsigned and signed integer types in
addition to the general 64-bit floating-point representation.

To optimize for parsing performance and minimal memory usage, the parsed tokens
are contained in a single contiguous allocation and form an immutable view on
the input JSON string. As the intended usage is sequential processing of chosen
parts of the file, there's no time spent building any acceleration structures
for fast lookup of keys and array indices --- if that's desired, users are
encouraged to build them on top of the parsed output.

The @ref JsonWriter class provides a write-only counterpart for saving a JSON
file.

@experimental

@section Utility-Json-usage Usage

The following snippet opens a very minimal
[glTF](https://www.khronos.org/gltf/) file, parses it including unescaping
strings and converting numbers to floats, and accesses the known properties:

@m_class{m-row m-container-inflate}

@parblock

@m_div{m-col-l-4}
@code{.json}
{
  "asset": {
    "version": "2.0"
  },
  "nodes": [
    …,
    {
      "name": "Fox",
      "mesh": 5
    }
  ]
}
@endcode
@m_enddiv

@m_div{m-col-l-8}
@snippet Utility.cpp Json-usage
@m_enddiv

@endparblock

The above --- apart from handling a parsing failure --- assumes that the node
`i` we're looking for exists in the file and contains the properties we want.
But that might not always be the case and so it could assert if we'd ask for
something that's not there. This is how it would look with
@ref JsonToken::find() used instead of @ref JsonToken::operator[]():

@snippet Utility.cpp Json-usage-find

@subsection Utility-Json-usage-selective-parsing Selective parsing

As shown above, the @ref Option values passed to @ref fromFile() or
@ref fromString() cause the file to get fully parsed upfront including number
conversion and string unescaping. While that's a convenient behavior that
makes sense when consuming the whole file, doing all that might be unnecessary
when you only need to access a small portion of a large file. In the following
snippet, all literals and string keys get parsed upfront so objects and arrays
can be searched, but then we parse only contents of the particular node using
@ref parseStrings() and  @ref parseFloats():

@snippet Utility.cpp Json-usage-selective-parsing

This way we also have a greater control over parsed numeric types. Instead of
parsing everything as a float using @ref Option::ParseFloats or
@ref parseFloats(), we can parse values that are meant to be integers using
@ref parseUnsignedInts(), @ref parseInts() etc., enforcing a desired numeric
type on a particular subset of the document. Which means, if the mesh ID in the
following snippet has a fractional part or is negative, we get an error:

@snippet Utility.cpp Json-usage-selective-parsing-numeric-types

Finally, while a JSON number is defined to be of a @cpp double @ce type, often
the full precision is not needed and so we parsed everything as floats. If the
precision indeed is important, you can switch to @ref Option::ParseDoubles /
@ref parseDoubles() instead where needed.

@subsection Utility-Json-usage-checked-selective-parsing Checked selective parsing

At this point, the code can handle variations in *valid* glTF files, but isn't
immune from files that don't follow the spec --- for example where the root
element is not an object, or a name isn't a string. Those would still cause the
`find()` and `as*()` APIs to assert.

If we omit all `Parse*` @ref Options in @ref fromString() / @ref fromFile(),
the instance will behave as if nothing was parsed yet. Thus any call to
@ref JsonToken::operator[](), @ref JsonToken::asString(),
@ref JsonToken::asFloat() etc. will assert unless given token was explicitly
previously checked and parsed using @ref parseObject(), @ref parseArray(),
@ref parseString(), @ref parseFloat() etc. That ensures there are no accidental
hardcoded assumptions about the layout of the file:

@snippet Utility.cpp Json-usage-checked-selective-parsing

A possibly non-obvious side-effect of using various @cpp parse*() @ce APIs
instead of checking @ref JsonToken::type() yourself is that if they fail, they
will print a detailed error message about what exactly failed and where ---
including file, line and column info.

@subsection Utility-Json-usage-iteration Iterating objects and arrays

So far, we only accessed object and array elements by a concrete key or an
index. Because the internal representation is optimized for linear consumption
rather than lookup by keys or values, those are @f$ \mathcal{O}(n) @f$
operations. If the whole file is being consumed, a more efficient way may be
iterating over object and array contents using @ref JsonToken::asObject() and
@relativeref{JsonToken,asArray()} and building your own representation from
these instead:

@snippet Utility.cpp Json-usage-iteration

Both @ref JsonObjectItem and @ref JsonArrayItem is implicitly convertible to a
@ref JsonToken reference, giving back either the object key (of which the value
is @ref JsonToken::firstChild()) or array value.

@snippet Utility.cpp Json-usage-iteration-values

@subsection Utility-Json-usage-direct-array-access Direct access to arrays

Besides high-level array iteration, there's also a set of functions for
accessing homogeneous arrays. Coming back to the glTF format, for example a
node translation vector and child indices:

@code{.json}
{
  "translation": [1.5, -0.5, 2.3],
  "children": [2, 3, 4, 17, 399]
}
@endcode

We'll check, parse and access the first property with @ref parseFloatArray()
and the other with @ref parseUnsignedIntArray() --- those will check that it's
indeed an array and that it contains either all floats or all unsigned
integers. If everything passes, we get back a @ref Containers::StridedArrayView,
pointing to parsed data stored inside the token elements, or we get an error
message and @ref Containers::NullOpt if not. A view to the parsed array can be
also retrieved later using @ref JsonToken::asFloatArray() and others, assuming
the parsed type matches.

@snippet Utility.cpp Json-usage-direct-array-access

This feature isn't limited to just numeric arrays --- for booleans there's
@ref parseBitArray() returning a @ref Containers::BasicStridedBitArrayView "Containers::StridedBitArrayView1D"
and for strings there's @ref parseStringArray() returning a
@ref Containers::StringIterable. Both again point to data stored elsewhere
without unnecessary copies.

@section Utility-Json-tokenization Tokenization and parsing process

The class expects exactly one top-level JSON value, be it an object, array,
literal, number or a string.

The tokenization process is largely inspired by [jsmn](https://github.com/zserge/jsmn)
--- the file gets processed to a flat list of @ref JsonToken instances, where
each literal, number, string (or a string object key), object and array is one
token, ordered in a depth-first manner. Whitespace is skipped and not present
in the parsed token list. @ref JsonToken::data() is a view on the input string
that defines the token, with the range containing also all nested tokens for
objects and arrays. @ref JsonToken::type() is then implicitly inferred from the
first byte of the token, but no further parsing or validation of actual token
values is done during the initial tokenization.

This allows the application to reduce the initial processing time and memory
footprint. Token parsing is a subsequent step, parsing the string range of a
token as a literal, converting it to a number or interpreting string escape
sequences. This step can then fail on its own, for example when an invalid
literal value is encountered, when a Unicode escape is out of range or when a
parsed integer doesn't fit into the output type size.

Token hierarchy is defined as the following --- object tokens have string keys
as children, string keys have object values as children, arrays have array
values as children and values themselves have no children. As implied by the
depth-first ordering, the first child token (if any) is ordered right after
its parent token, and together with @ref JsonToken::childCount(), which is the
count of all nested tokens, it's either possible to dive into the child token
tree using @ref JsonToken::firstChild() or @ref JsonToken::children() or skip
after the child token tree using @ref JsonToken::next(). These APIs return
@ref JsonView and @ref JsonIterator instances allowing for convenient use in
both range-for and manual loops.

@section Utility-Json-representation Internal representation

If the string passed to @ref fromString() is
@ref Containers::StringViewFlag::Global, it's just referenced without an
internal copy, and all token data will point to it as well. Otherwise, or if
@ref fromFile() is used, a local copy is made, and tokens point to the copy
instead.

A @ref JsonToken is an opaque reference type pointing to the originating
@ref Json instance and a concrete position in an array of @ref JsonTokenData.
The @ref JsonTokenData is 16 bytes on 32-bit systems and 24 bytes on 64-bit
systems, containing view pointer, size and child count. When a literal or
numeric value is parsed, it's stored inside. Simply put, the representation
exploits the fact that a token either has children or is a value, but never
both. For strings the general assumption is that most of them (and especially
object keys) don't contain any escape characters and thus can be returned as
views on the input string. Strings containing escape characters are allocated
separately, either upfront if @ref Option::ParseStrings is set (or if
@ref Option::ParseStringKeys is set and object keys contain escaped values), or
on-demand if @ref parseStrings() / @ref parseStringKeys() / @ref parseString()
is used.

@see @ref JsonArrayItem, @ref JsonObjectItem, @ref JsonIterator,
    @ref JsonArrayIterator, @ref JsonObjectIterator, @ref JsonView,
    @ref JsonObjectView, @ref JsonArrayView
*/
class CORRADE_UTILITY_EXPORT Json {
    public:
        /**
         * @brief Parsing option
         *
         * @see @ref Options, @ref fromString(), @ref fromFile()
         */
        enum class Option {
            /**
             * Parse objects, arrays, @cb{.json} null @ce, @cb{.json} true @ce
             * and @cb{.json} false @ce values. Causes all @ref JsonToken
             * instances of @ref JsonToken::Type::Object,
             * @relativeref{JsonToken::Type,Array},
             * @relativeref{JsonToken::Type,Null} and
             * @relativeref{JsonToken::Type,Bool} to have
             * @ref JsonToken::isParsed() set and be accessible through
             * @ref JsonToken::asObject(), @relativeref{JsonToken,asArray()},
             * @relativeref{JsonToken,operator[](std::size_t) const},
             * @relativeref{JsonToken,find(std::size_t) const},
             * @relativeref{JsonToken,asNull()} and
             * @relativeref{JsonToken,asBool()}.
             *
             * Invalid values will cause @ref fromString() / @ref fromFile()
             * to print an error and return @ref Containers::NullOpt. This
             * operation can be also performed selectively later using
             * @ref parseLiterals(), or directly for particular tokens using
             * @ref parseObject(), @ref parseArray(), @ref parseNull() or
             * @ref parseBool().
             *
             * Note that using @ref JsonToken::operator[](Containers::StringView) const,
             * @ref JsonToken::find(Containers::StringView) const or accessing
             * @ref JsonObjectItem::key() requires object keys to be parsed as
             * well --- either with @ref parseStringKeys(),
             * @ref Option::ParseStringKeys during the initial call or with
             * @ref parseObject() that parses both an object and its keys.
             */
            ParseLiterals = 1 << 0,

            /**
             * Parse all numbers as 64-bit floating-point values. Causes all
             * @ref JsonToken instances of @ref JsonToken::Type::Number to
             * become @ref JsonToken::ParsedType::Double and be accessible
             * through @ref JsonToken::asDouble(). If both
             * @ref Option::ParseDoubles and @ref Option::ParseFloats is
             * specified, @ref Option::ParseDoubles gets a precedence.
             *
             * Invalid values will cause @ref fromString() / @ref fromFile()
             * to exit with a parse error. This operation can be also performed
             * selectively later using @ref parseDoubles(), or directly for
             * particular tokens using @ref parseDouble().
             *
             * While this option is guaranteed to preserve the full precision
             * of JSON numeric literals, often you may need only 32-bit
             * precision --- use @ref Option::ParseFloats in that case instead.
             * It's also possible to selectively parse certain values as
             * integers using @ref parseUnsignedInts(), @ref parseInts(),
             * @ref parseUnsignedLongs(), @ref parseLongs() or
             * @ref parseSizes(), then the parsing will also check that the
             * value is an (unsigned) integer and can be represented in given
             * type size. There's no corresponding @ref Option for those, as
             * JSON files rarely contain just integer numbers alone. If you
             * indeed have an integer-only file, call those directly on
             * @ref root().
             */
            ParseDoubles = 1 << 1,

            /**
             * Parse all numbers as 32-bit floating-point values. Causes all
             * @ref JsonToken instances of @ref JsonToken::Type::Number to
             * become @ref JsonToken::ParsedType::Float and be accessible
             * through @ref JsonToken::asFloat(). If both
             * @ref Option::ParseDoubles and @ref Option::ParseFloats is
             * specified, @ref Option::ParseDoubles gets a precedence.
             *
             * Invalid values will cause @ref fromString() / @ref fromFile()
             * to exit with a parse error. This operation can be also performed
             * selectively later using @ref parseFloats(), or directly for
             * particular tokens using @ref parseFloat().
             *
             * While 32-bit float precision is often enough, sometimes you
             * might want to preserve the full precision of JSON numeric
             * literals --- use @ref Option::ParseDoubles in that case instead.
             * It's also possible to selectively parse certain values as
             * integers using @ref parseUnsignedInts(), @ref parseInts(),
             * @ref parseUnsignedLongs(), @ref parseLongs() or
             * @ref parseSizes(), then the parsing will also check that the
             * value is an (unsigned) integer and can be represented in given
             * type size. There's no corresponding @ref Option for those, as
             * JSON files rarely contain just integer numbers alone. If you
             * indeed have an integer-only file, call those directly on
             * @ref root().
             */
            ParseFloats = 1 << 2,

            /**
             * Parse object key strings by processing all escape sequences and
             * caching the parsed result (or marking the original string as
             * parsed in-place, if it has no escape sequences). Causes
             * @ref JsonToken instances of @ref JsonToken::Type::String that
             * are children of a @ref JsonToken::Type::Object to have
             * @ref JsonToken::isParsed() set and be accessible through
             * @ref JsonToken::asString(). String values (as opposed to
             * keys) are left untouched, thus this is useful if you need to
             * perform a key-based search, but don't need to have also all
             * other strings unescaped.
             *
             * Invalid values will cause @ref fromString() / @ref fromFile()
             * to exit with a parse error. This operation can be also performed
             * selectively later using @ref parseStringKeys(), or directly
             * for particular tokens using @ref parseString().
             */
            ParseStringKeys = 1 << 3,

            /**
             * Parse string values by processing all escape sequences and
             * caching the parsed result (or marking the original string as
             * parsed in-place, if it has no escape sequences). Causes all
             * @ref JsonToken instances of @ref JsonToken::Type::String to have
             * @ref JsonToken::isParsed() set and be accessible through
             * @ref JsonToken::asString(). Implies
             * @ref Option::ParseStringKeys.
             *
             * Invalid values will cause @ref fromString() / @ref fromFile()
             * to exit with a parse error. This operation can be also performed
             * selectively later using @ref parseStrings(), or directly for
             * particular tokens using @ref parseString().
             */
            ParseStrings = ParseStringKeys|(1 << 4)
        };

        /**
         * @brief Parsing options
         *
         * @see @ref fromString(), @ref fromFile()
         */
        typedef Containers::EnumSet<Option> Options;
        CORRADE_ENUMSET_FRIEND_OPERATORS(Options)

        /**
         * @brief Parse a JSON string
         * @param string        JSON string to parse
         * @param options       Parsing options
         * @param filename      Filename to use for error reporting. If not
         *      set, `<in>` is printed in error messages.
         * @param lineOffset    Initial line offset in the file for error
         *      reporting
         * @param columnOffset  Initial column offset in the file for error
         *      reporting
         *
         * By default performs only tokenization, not parsing any literals. Use
         * @p options to enable parsing of particular token types as well. If
         * a tokenization or parsing error happens, prints a message to
         * @ref Error and returns @ref Containers::NullOpt.
         *
         * If the @p string is @ref Containers::StringViewFlag::Global, the
         * parsed tokens will reference it, returning also global string
         * literals. Otherwise a copy is made internally.
         *
         * The @p filename, @p lineOffset and @p columnOffset parameters have
         * no effect on actual parsing and are used only to enhance error
         * messages. Line offset is added to the reported line always but
         * column offset is used only if the happens on the first line of the
         * JSON string, errors on subsequent lines are reported without the
         * initial column offset.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        static Containers::Optional<Json> fromString(Containers::StringView string, Options options = {}, Containers::StringView filename = {}, std::size_t lineOffset = 0, std::size_t columnOffset = 0);
        #else
        /* For binary size savings -- the base implementation does only minimal
           tokenization and thus the parsing code can be DCE'd if unused.
           Besides that, the other overloads are explicit to avoid having to
           include a StringView in the header. */
        static Containers::Optional<Json> fromString(Containers::StringView string);
        static Containers::Optional<Json> fromString(Containers::StringView string, Options options);
        static Containers::Optional<Json> fromString(Containers::StringView string, Options options, Containers::StringView filename, std::size_t lineOffset = 0, std::size_t columnOffset = 0);
        #endif

        /** @overload */
        static Containers::Optional<Json> fromString(Containers::StringView string, Containers::StringView filename, std::size_t lineOffset = 0, std::size_t columnOffset = 0);

        /**
         * @brief Parse a JSON file
         *
         * By default performs only tokenization, not parsing any literals. Use
         * @p options to enable parsing of particular token types as well. If
         * the file can't be read, or a tokenization or parsing error happens,
         * prints a message to @ref Error and returns @ref Containers::NullOpt.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        static Containers::Optional<Json> fromFile(Containers::StringView filename, Options options = {});
        #else
        /* These contain the same code and delegate to either the base
           implementation or the complex one, allowing the extra code to be
           DCE'd */
        static Containers::Optional<Json> fromFile(Containers::StringView filename);
        static Containers::Optional<Json> fromFile(Containers::StringView filename, Options options);
        #endif

        /** @brief Copying is not allowed */
        Json(const Json&) = delete;

        /** @brief Move constructor */
        Json(Json&&) noexcept;

        ~Json();

        /** @brief Copying is not allowed */
        Json& operator=(const Json&) = delete;

        /** @brief Move assignment */
        Json& operator=(Json&&) noexcept;

        /**
         * @brief Parsed JSON tokens
         *
         * The first token is the root token (also accessible via @ref root())
         * and is always present, the rest is ordered in a depth-first manner
         * as described in @ref Utility-Json-tokenization.
         */
        JsonView tokens() const;

        /**
         * @brief Root JSON token
         *
         * Always present. Its @ref JsonToken::children() (if any) contain the
         * whole document ordered in a depth-first manner as described in
         * @ref Utility-Json-tokenization.
         */
        JsonToken root() const;

        /**
         * @brief Parse objects, arrays, `null`, `true` and `false` values in given token tree
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Object,
         * @relativeref{JsonToken::Type,Array},
         * @relativeref{JsonToken::Type,Null} and
         * @relativeref{JsonToken::Type,Bool} to have @ref JsonToken::isParsed()
         * set and be accessible through @ref JsonToken::asObject(),
         * @relativeref{JsonToken,asArray()},
         * @relativeref{JsonToken,operator[](std::size_t) const},
         * @relativeref{JsonToken,find(std::size_t) const},
         * @relativeref{JsonToken,asNull()} and
         * @relativeref{JsonToken,asBool()}. Tokens of other types and tokens
         * that are already parsed are skipped. If an invalid value is
         * encountered, prints a message to @ref Error and returns
         * @cpp false @ce. Expects that @p token references a token owned by
         * this instance.
         *
         * Passing @ref root() as @p token has the same effect as
         * @ref Option::ParseLiterals specified during the initial
         * @ref fromString() or @ref fromFile() call. Checking a single token
         * for an object, array, null or boolean type and parsing it as can be
         * done using @ref parseObject(), @ref parseArray(), @ref parseNull()
         * or @ref parseBool().
         *
         * Note that using @ref JsonToken::operator[](Containers::StringView) const,
         * @ref JsonToken::find(Containers::StringView) const or accessing
         * @ref JsonObjectItem::key() requires object keys to be parsed as well
         * --- either with @ref parseStringKeys(), @ref Option::ParseStringKeys
         * during the initial call or with @ref parseObject() that parses both
         * an object and its keys.
         */
        bool parseLiterals(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as 64-bit floating-point values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::Double and be accessible through
         * @ref JsonToken::asDouble(). Non-numeric tokens and numeric tokens
         * that are already parsed as doubles are skipped, numeric tokens
         * parsed as other types are reparsed. If an invalid value is
         * encountered, prints a message to @ref Error and returns
         * @cpp false @ce. Expects that @p token references a token owned by
         * this instance.
         *
         * Passing @ref root() as @p token has the same effect as
         * @ref Option::ParseDoubles specified during the initial
         * @ref fromString() or @ref fromFile() call. Checking a single token
         * for a numeric type and parsing it as a doouble can be done using
         * @ref parseDouble().
         * @see @ref parseFloats(), @ref parseUnsignedInts(), @ref parseInts()
         */
        bool parseDoubles(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as 32-bit floating-point values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::Float and be accessible through
         * @ref JsonToken::asFloat(). Non-numeric tokens and numeric tokens
         * that are already parsed as floats are skipped, numeric tokens parsed
         * as other types are reparsed. If an invalid value is encountered,
         * prints a message to @ref Error and returns @cpp false @ce. Expects
         * that @p token references a token owned by this instance.
         *
         * Passing @ref root() as @p token has the same effect as
         * @ref Option::ParseFloats specified during the initial
         * @ref fromString() or @ref fromFile() call. Checking a single token
         * for a numeric type and parsing it as a float can be done using
         * @ref parseFloat().
         * @see @ref parseDoubles(), @ref parseUnsignedInts(), @ref parseInts()
         */
        bool parseFloats(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as unsigned 32-bit integer values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::UnsignedInt and be accessible through
         * @ref JsonToken::asUnsignedInt(). Non-numeric tokens and numeric
         * tokens that are already parsed as unsigned int are skipped, numeric
         * tokens parsed as other types are reparsed. If an invalid value,
         * a literal with a fractional or exponent part or a negative value is
         * encountered or a value doesn't fit into a 32-bit representation,
         * prints a message to @ref Error and returns @cpp false @ce. Expects
         * that @p token references a token owned by this instance.
         *
         * Checking a single token for a numeric type and parsing it as an
         * unsigned int can be done using @ref parseUnsignedInt().
         * @see @ref parseDoubles(), @ref parseInts(),
         *      @ref parseUnsignedLongs(), @ref parseSizes()
         */
        bool parseUnsignedInts(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as signed 32-bit integer values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::Int and be accessible through
         * @ref JsonToken::asInt(). Non-numeric tokens and numeric tokens that
         * are already parsed as int are skipped, numeric tokens parsed as
         * other types are reparsed. If an invalid value, a literal with a
         * fractional or exponent part is encountered or a value doesn't fit
         * into a 32-bit representation, prints a message to @ref Error and
         * returns @cpp false @ce. Expects that @p token references a token
         * owned by this instance.
         *
         * Checking a single token for a numeric type and parsing it as an int
         * can be done using @ref parseUnsignedInt().
         * @see @ref parseDoubles(), @ref parseUnsignedInts(),
         *      @ref parseLongs(), @ref parseSizes()
         */
        bool parseInts(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as unsigned 52-bit integer values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::UnsignedLong and be accessible through
         * @ref JsonToken::asUnsignedLong(). Non-numeric tokens and numeric
         * tokens that are already parsed as unsigned long are skipped, numeric
         * tokens parsed as other types are reparsed. If an invalid value, a
         * literal with a fractional or exponent part or a negative value is
         * encountered or a value doesn't fit into 52 bits (which is the
         * representable unsigned integer range in a JSON), prints a message to
         * @ref Error and returns @cpp false @ce. Expects that @p token
         * references a token owned by this instance.
         *
         * Checking a single token for a numeric type and parsing it as an
         * unsigned long can be done using @ref parseUnsignedInt().
         * @see @ref parseDoubles(), @ref parseLongs(),
         *      @ref parseUnsignedInts(), @ref parseSizes()
         */
        bool parseUnsignedLongs(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as signed 53-bit integer values
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::Number
         * in @p token and its children to become
         * @ref JsonToken::ParsedType::Long and be accessible through
         * @ref JsonToken::asLong(). Non-numeric tokens and numeric tokens that
         * are already parsed as long are skipped, numeric tokens parsed as
         * other types are reparsed. If an invalid value, a literal with a
         * fractional or exponent part is encountered or a value doesn't fit
         * into 53 bits (which is the representable signed integer range in a
         * JSON), prints a message to @ref Error and returns @cpp false @ce.
         * Expects that @p token references a token owned by this instance.
         *
         * Checking a single token for a numeric type and parsing it as a long
         * can be done using @ref parseLong().
         * @see @ref parseDoubles(), @ref parseUnsignedLongs(),
         *      @ref parseInts(), @ref parseSizes()
         */
        bool parseLongs(JsonToken token);

        /**
         * @brief Parse numbers in given token tree as size values
         *
         * Convenience function that calls into @ref parseUnsignedInts() on
         * @ref CORRADE_TARGET_32BIT "32-bit targets" and into
         * @ref parseUnsignedLongs() on 64-bit. Besides being available under
         * the concrete types as documented in these functions, @ref JsonToken
         * instances of @ref JsonToken::Type::Number in @p token and its
         * children will alias to @ref JsonToken::ParsedType::Size and be also
         * accessible through @ref JsonToken::asSize().
         *
         * Checking a single token for a numeric type and parsing it as a size
         * can be done using @ref parseSize().
         */
        bool parseSizes(JsonToken token);

        /**
         * @brief Parse string keys in given token tree
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::String
         * that are children of a @ref JsonToken::Type::Object in @p token and
         * its children to have @ref JsonToken::isParsed() set and be
         * accessible through @ref JsonToken::asString(). The operation is a
         * subset of @ref parseStringKeys(). Non-string tokens, string tokens
         * that are not object keys and string tokens that are already parsed
         * are skipped. If an invalid value is encountered, prints a message to
         * @ref Error and returns @cpp false @ce. Expects that @p token
         * references a token owned by this instance.
         *
         * Passing @ref root() as @p token has the same effect as
         * @ref Option::ParseStringKeys specified during the initial
         * @ref fromString() or @ref fromFile() call. Checking a single token
         * for a string type and parsing it can be done using
         * @ref parseString().
         */
        bool parseStringKeys(JsonToken token);

        /**
         * @brief Parse strings in given token tree
         *
         * Causes all @ref JsonToken instances of @ref JsonToken::Type::String
         * in @p token and its children to have @ref JsonToken::isParsed() set
         * and be accessible through @ref JsonToken::asString(). The operation
         * is a superset of @ref parseStringKeys(). Non-string tokens and
         * string tokens that are already parsed are skipped. If an invalid
         * value is encountered, prints a message to @ref Error and returns
         * @cpp false @ce. Expects that @p token references a token owned by
         * this instance.
         *
         * Passing @ref root() as @p token has the same effect as
         * @ref Option::ParseStrings specified during the initial
         * @ref fromString() or @ref fromFile() call. Checking a single token
         * for a string type and parsing it can be done using
         * @ref parseString().
         */
        bool parseStrings(JsonToken token);

        /**
         * @brief Check and parse an object token
         *
         * If @p token is not a @ref JsonToken::Type::Object or does not have
         * valid string values in its keys, prints a message to @ref Error and
         * returns @ref Containers::NullOpt. If @ref JsonToken::isParsed() is
         * already set on the object and all its keys, returns the object view
         * directly, otherwise caches the parsed results first. Expects that
         * @p token references a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asObject()
         */
        Containers::Optional<JsonObjectView> parseObject(JsonToken token);

        /**
         * @brief Check and parse an array token
         *
         * If @p token is not a @ref JsonToken::Type::Array, prints a message
         * to @ref Error and returns @cpp false @ce. If
         * @ref JsonToken::isParsed() is already set on the array, returns the
         * array view directly, otherwise marks it as parsed first. Expects
         * that @p token references a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref JsonToken::asArray(), @ref parseBitArray(),
         *      @ref parseDoubleArray(), @ref parseFloatArray(),
         *      @ref parseUnsignedIntArray(), @ref parseIntArray(),
         *      @ref parseUnsignedLongArray(), @ref parseLongArray(),
         *      @ref parseSizeArray(), @ref parseStringArray()
         */
        Containers::Optional<JsonArrayView> parseArray(JsonToken token);

        /**
         * @brief Check and parse a null token
         *
         * If @p token is not a @ref JsonToken::Type::Null or is not a valid
         * null value, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set, returns the cached value, otherwise caches the parsed result.
         * Expects that @p token references a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asNull()
         */
        Containers::Optional<std::nullptr_t> parseNull(JsonToken token);

        /**
         * @brief Check and parse a boolean token
         *
         * If @p token is not a @ref JsonToken::Type::Bool or is not a valid
         * boolean value, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set, returns the cached value, otherwise caches the parsed result.
         * Expects that @p token references a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asBool(),
         *      @ref parseBoolArray()
         */
        Containers::Optional<bool> parseBool(JsonToken token);

        /**
         * @brief Check and parse a 64-bit floating-point token
         *
         * If @p token is not a @ref JsonToken::Type::Number or is not a valid
         * numeric value, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If the token is already parsed as
         * @ref JsonToken::ParsedType::Double, returns the cached value,
         * otherwise caches the parsed result. Expects that @p token references
         * a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asDouble(),
         *      @ref parseDoubleArray()
         */
        Containers::Optional<double> parseDouble(JsonToken token);

        /**
         * @brief Check and parse a 32-bit floating-point token
         *
         * If @p token is not a @ref JsonToken::Type::Number or is not a valid
         * numeric value, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. Precision that doesn't fit into the 32-bit
         * floating-point representation is truncated, use @ref parseDouble()
         * to get the full precision. If the token is already parsed as
         * @ref JsonToken::ParsedType::Float, returns the cached value,
         * otherwise caches the parsed result. Expects that @p token references
         * a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asFloat(),
         *      @ref parseFloatArray()
         */
        Containers::Optional<float> parseFloat(JsonToken token);

        /**
         * @brief Check and parse an unsigned 32-bit integer token
         *
         * If @p token is not a @ref JsonToken::Type::Number, is not a valid
         * numeric value, has a fractional or exponent part, is negative or
         * doesn't fit into a 32-bit representation, prints a message to
         * @ref Error and returns @ref Containers::NullOpt. If the token is
         * already parsed as @ref JsonToken::ParsedType::UnsignedInt, returns
         * the cached value, otherwise caches the parsed result. Expects that
         * @p token references a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asUnsignedInt(),
         *      @ref parseSize(), @ref parseUnsignedIntArray()
         */
        Containers::Optional<std::uint32_t> parseUnsignedInt(JsonToken token);

        /**
         * @brief Check and parse a signed 32-bit integer token
         *
         * If @p token is not a @ref JsonToken::Type::Number, is not a valid
         * numeric value, has a fractional or exponent part or doesn't fit into
         * a 32-bit representation, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If the token is already parsed as
         * @ref JsonToken::ParsedType::Int, returns the cached value, otherwise
         * caches the parsed result. Expects that @p token references a token
         * owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asInt(),
         *      @ref parseIntArray()
         */
        Containers::Optional<std::int32_t> parseInt(JsonToken token);

        /**
         * @brief Check and parse an unsigned 52-bit integer token
         *
         * If @p token is not a @ref JsonToken::Type::Number, is not a valid
         * numeric value, has a fractional or exponent part, is negative or
         * doesn't fit into 52 bits (which is the representable unsigned
         * integer range in a JSON), prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If the token is already parsed as
         * @ref JsonToken::ParsedType::UnsignedLong, returns the cached value,
         * otherwise caches the parsed result. Expects that @p token references
         * a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asUnsignedLong(),
         *      @ref parseSize(), @ref parseUnsignedLongArray()
         */
        Containers::Optional<std::uint64_t> parseUnsignedLong(JsonToken token);

        /**
         * @brief Check and parse a signed 53-bit integer token
         *
         * If @p token is not a @ref JsonToken::Type::Number,  is not a valid
         * nnumeric value, has a fractional or exponent part or doesn't fit
         * into 53 bits (which is the representable signed integer range in a
         * JSON), prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If the token is already parsed as
         * @ref JsonToken::ParsedType::Long, returns the cached value,
         * otherwise caches the parsed result. Expects that @p token references
         * a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asLong(),
         *      @ref parseLongArray()
         */
        Containers::Optional<std::int64_t> parseLong(JsonToken token);

        /**
         * @brief Check and parse a size token
         *
         * Convenience function that calls into @ref parseUnsignedInt() on
         * @ref CORRADE_TARGET_32BIT "32-bit targets" and into
         * @ref parseUnsignedLong() on 64-bit. If the token is already parsed
         * as @ref JsonToken::ParsedType::Size, returns the cached value,
         * otherwise caches the parsed result. Expects that @p token references
         * a token owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asSize(),
         *      @ref parseSizeArray()
         */
        Containers::Optional<std::size_t> parseSize(JsonToken token);

        /**
         * @brief Check and parse a string token
         *
         * If @p token is not a @ref JsonToken::Type::String or is not a valid
         * string value, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set, returns the cached value, otherwise caches the parsed result.
         * Expects that @p token references a token owned by this instance. If
         * @ref fromString() was called with a global literal and the string
         * didn't contain any escape sequences, the returned view has
         * @ref Containers::StringViewFlag::Global set. If not, the view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::asString(),
         *      @ref parseStringArray()
         */
        Containers::Optional<Containers::StringView> parseString(JsonToken token);

        /**
         * @brief Check and parse a bit array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Bool tokens, the tokens are not valid
         * boolean values, or @p expectedSize is not @cpp 0 @ce and the array
         * has different size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set for the array and all tokens inside, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref Json::Option::ParseLiterals, @ref parseLiterals(),
         *      @ref parseBool(), @ref parseArray(),
         *      @ref JsonToken::asBitArray()
         */
        Containers::Optional<Containers::StridedBitArrayView1D> parseBitArray(JsonToken token, std::size_t expectedSize = 0);

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief Check and parse a boolean array
         * @m_deprecated_since_latest Use @ref parseBitArray() instead.
         */
        CORRADE_DEPRECATED("use parseBitArray() instead") Containers::Optional<Containers::StridedArrayView1D<const bool>> parseBoolArray(JsonToken token, std::size_t expectedSize = 0);
        #endif

        /**
         * @brief Check and parse a 64-bit floating-point array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, or @p expectedSize is not @cpp 0 @ce and the array
         * has different size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set for the array and all tokens inside are already parsed as
         * @ref JsonToken::ParsedType::Double, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref Json::Option::ParseDoubles, @ref parseDoubles(),
         *      @ref parseDouble(), @ref parseArray(),
         *      @ref JsonToken::asDoubleArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const double>> parseDoubleArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse a 32-bit floating-point array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, or @p expectedSize is not @cpp 0 @ce and the array
         * has different size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. Precision that doesn't fit into the 32-bit
         * floating-point representation is truncated, use
         * @ref parseDoubleArray() to get the full precision. If
         * @ref JsonToken::isParsed() is already set for the array and all
         * tokens inside are already parsed as
         * @ref JsonToken::ParsedType::Float, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref Json::Option::ParseFloats, @ref parseFloats(),
         *      @ref parseFloat(), @ref parseArray(),
         *      @ref JsonToken::asFloatArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const float>> parseFloatArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse an unsigned 32-bit integer array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, have a fractional or exponent part, are negative or
         * don't fit into a 32-bit representation, or @p expectedSize is not
         * @cpp 0 @ce and the array has different size, prints a message to
         * @ref Error and returns @ref Containers::NullOpt. If
         * @ref JsonToken::isParsed() is already set for the array and all
         * tokens inside are already parsed as
         * @ref JsonToken::ParsedType::UnsignedInt, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref parseUnsignedInts(), @ref parseUnsignedInt(),
         *      @ref parseSizeArray(), @ref parseArray(),
         *      @ref JsonToken::asUnsignedIntArray(),
         */
        Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> parseUnsignedIntArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse a signed 32-bit integer array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, have a fractional or exponent part or don't fit into
         * a 32-bit representation, or @p expectedSize is not @cpp 0 @ce and
         * the array has different size, prints a message to @ref Error and
         * returns @ref Containers::NullOpt. If @ref JsonToken::isParsed() is
         * already set for the array and all tokens inside are already parsed
         * as @ref JsonToken::ParsedType::Int, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref parseInts(), @ref parseInt(), @ref parseArray(),
         *      @ref JsonToken::asIntArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const std::int32_t>> parseIntArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse an unsigned 52-bit integer array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, have a fractional or exponent part, are negative or
         * don't fit into 52 bits (which is the representable unsigned integer
         * range in a JSON), or @p expectedSize is not @cpp 0 @ce and the array
         * has different size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set for the array and all tokens inside are already parsed as
         * @ref JsonToken::ParsedType::UnsignedLong, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref parseUnsignedLongs(), @ref parseUnsignedLong(),
         *      @ref parseSizeArray(), @ref parseArray(),
         *      @ref JsonToken::asUnsignedLongArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> parseUnsignedLongArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse a signed 52-bit integer array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::Number tokens, the tokens are not valid
         * numeric values, have a fractional or exponent part or don't fit into
         * 53 bits (which is the representable signed integer range in a JSON),
         * or @p expectedSize is not @cpp 0 @ce and the array has different
         * size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set for the array and all tokens are already parsed as
         * @ref JsonToken::ParsedType::Long, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance, the returned view points
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref parseLongs(), @ref parseLong(), @ref parseArray(),
         *      @ref JsonToken::asLongArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const std::int64_t>> parseLongArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse a size array
         *
         * Convenience function that calls into @ref parseUnsignedIntArray() on
         * @ref CORRADE_TARGET_32BIT "32-bit targets" and into
         * @ref parseUnsignedLongArray() on 64-bit.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref parseSizes(), @ref parseSize(), @ref parseArray(),
         *      @ref JsonToken::asSizeArray()
         */
        Containers::Optional<Containers::StridedArrayView1D<const std::size_t>> parseSizeArray(JsonToken token, std::size_t expectedSize = 0);

        /**
         * @brief Check and parse a string array
         *
         * If @p token is not a @ref JsonToken::Type::Array, doesn't contain
         * just @ref JsonToken::Type::String tokens, the tokens are not valid
         * numeric values, or @p expectedSize is not @cpp 0 @ce and the array
         * has different size, prints a message to @ref Error and returns
         * @ref Containers::NullOpt. If @ref JsonToken::isParsed() is already
         * set for the array and all tokens inside, returns the cached values,
         * otherwise caches the parsed results. Expects that @p token
         * references a token owned by this instance. If @ref fromString() was
         * called with a global literal and the string didn't contain any
         * escape sequences, the returned views have
         * @ref Containers::StringViewFlag::Global set. If not, the views point
         * to data owned by this instance.
         * @see @ref JsonToken::type(), @ref JsonToken::commonArrayType(),
         *      @ref Json::Option::ParseStrings, @ref parseStrings(),
         *      @ref parseString(), @ref JsonToken::asStringArray()
         */
        Containers::Optional<Containers::StringIterable> parseStringArray(JsonToken token, std::size_t expectedSize = 0);

    private:
        friend JsonToken; /* JsonTokenData constructor and data() needs _state access */

        struct State;

        explicit CORRADE_UTILITY_LOCAL Json();

        /* These are here because they need friended JsonToken and/or access to
           State */
        CORRADE_UTILITY_LOCAL void printFilePosition(Debug& out, Containers::StringView string) const;
        CORRADE_UTILITY_LOCAL void printFilePosition(Debug& out, const JsonTokenData& token) const;
        CORRADE_UTILITY_LOCAL static Containers::Optional<Json> tokenize(Containers::StringView filename, std::size_t lineOffset, std::size_t columnOffset, Containers::StringView string);
        CORRADE_UTILITY_LOCAL static Containers::Optional<Json> tokenize(Containers::StringView filename, std::size_t lineOffset, std::size_t columnOffset, Containers::StringView string, Options options);
        /* Used by all parse*Internal() below, is here and not on JsonTokenData
           because it may eventually rely on data outside of given token. Is
           static because JsonToken::data() has no access to the Json
           instance, only to the state. */
        CORRADE_UTILITY_LOCAL static Containers::StringView tokenData(const Implementation::JsonData& json, const JsonTokenData& token);
        /* These are here because they need friended JsonToken, they're not on
           JsonToken in order to print nice file/line info on error (and access
           the string cache in case of parseStringInternal()) */
        CORRADE_UTILITY_LOCAL void parseObjectArrayInternal(JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseNullInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseBoolInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseDoubleInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseFloatInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseUnsignedIntInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseIntInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseUnsignedLongInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseLongInternal(const char* errorPrefix, JsonTokenData& token);
        CORRADE_UTILITY_LOCAL bool parseStringInternal(const char* errorPrefix, JsonTokenData& token);

        Containers::Pointer<State> _state;
};

/**
@brief A single JSON token
@m_since_latest

Represents an object, array, null, boolean, numeric or a string value in a JSON
file. See the @ref Json class documentation for more information.

@experimental
*/
class CORRADE_UTILITY_EXPORT JsonToken {
    public:
        /**
         * @brief Token type
         *
         * @see @ref type()
         */
        enum class Type: std::uint8_t {
            /**
             * An object, @cb{.json} {} @ce. Its immediate children are
             * @ref Type::String keys, values are children of the keys. The
             * keys can be in an arbitrary order and can contain duplicates.
             * @ref isParsed() is set always.
             * @see @ref children(), @ref firstChild(), @ref next()
             */
            Object,

            /**
             * An array, @cb{.json} [] @ce. Its immediate children are values.
             * @ref isParsed() is set always.
             * @see @ref children(), @ref firstChild(), @ref next()
             */
            Array,

            /**
             * A @cb{.json} null @ce value. Unless @ref isParsed() is set, the
             * value is not guaranteed to be valid.
             * @see @ref asNull(), @ref Json::Option::ParseLiterals,
             *      @ref Json::parseLiterals(), @ref Json::parseNull()
             */
            Null,

            /**
             * A @cb{.json} true @ce or @cb{.json} false @ce value. Unless
             * @ref isParsed() is set, the value is not guaranteed to be valid.
             * @see @ref asBool(), @ref Json::Option::ParseLiterals,
             *      @ref Json::parseLiterals(), @ref Json::parseBool()
             */
            Bool,

            /**
             * A number. Unless @ref isParsed() is set, the value is not
             * guaranteed to be valid. JSON numbers are always 64-bit floating
             * point values but you can choose whether to parse them as doubles
             * or floats using @ref Json::parseDoubles() or
             * @ref Json::parseFloats(). If an integer value is expected you
             * can use @ref Json::parseInts(), @ref Json::parseUnsignedInts(),
             * @ref Json::parseLongs(), @ref Json::parseUnsignedLongs() or
             * @ref Json::parseSizes() instead to implicitly check that they
             * have a zero fractional part or additionally that they're also
             * non-negative.
             * @see @ref asDouble(), @ref asFloat(), @ref asUnsignedInt(),
             *      @ref asInt(), @ref asUnsignedLong(), @ref asLong(),
             *      @ref asSize(), @ref Json::Option::ParseDoubles,
             *      @ref Json::Option::ParseFloats, @ref Json::parseDouble(),
             *      @ref Json::parseFloat(), @ref Json::parseUnsignedInt(),
             *      @ref Json::parseInt(), @ref Json::parseUnsignedLong(),
             *      @ref Json::parseLong(), @ref Json::parseSize()
             */
            Number,

            /**
             * A string. Unless @ref isParsed() is set, the value is not
             * guaranteed to be valid.
             * @see @ref asString(), @ref Json::Option::ParseStringKeys,
             *      @ref Json::Option::ParseStrings,
             *      @ref Json::parseStringKeys(), @ref Json::parseStrings(),
             *      @ref Json::parseString()
             */
            String
        };

        /**
         * @brief Parsed type
         *
         * @see @ref parsedType()
         */
        enum class ParsedType: std::uint8_t {
            /** Not parsed yet. */
            None,

            /**
             * 64-bit floating-point value.
             *
             * Set if @ref Json::Option::ParseDoubles is passed to
             * @ref Json::fromString() or @ref Json::fromFile() or if
             * @ref Json::parseDoubles() is called later.
             */
            Double,

            /**
             * 32-bit floating-point value.
             *
             * Set if @ref Json::Option::ParseFloats is passed to
             * @ref Json::fromString() or @ref Json::fromFile() or if
             * @ref Json::parseFloats() is called later. Double-precision
             * values that can't be represented as a float are truncated.
             */
            Float,

            /**
             * 32-bit unsigned integer value.
             *
             * Set if @ref Json::parseUnsignedInts() is called on a particular
             * subtree. Except for invalid values, parsing fails also if any
             * the values have a non-zero fractional part, if they have an
             * exponent, if they're negative or if they can't fit into 32 bits.
             * @see @ref ParsedType::Size, @ref Json::parseSizes()
             */
            UnsignedInt,

            /**
             * 32-bit signed integer value.
             *
             * Set if @ref Json::parseInts() is called on a particular subtree.
             * Except for invalid values, parsing fails also if any the values
             * have a non-zero fractional part, if they have an exponent or if
             * they can't fit into 32 bits.
             */
            Int,

            /**
             * 52-bit unsigned integer value.
             *
             * Set if @ref Json::parseUnsignedLongs() is called on a particular
             * subtree. Except for invalid values, parsing fails also fails if
             * any the values have a non-zero fractional part, if they have an
             * exponent, if they're negative or if they can't fit into 52 bits
             * (which is the representable unsigned integer range in a JSON).
             * @see @ref ParsedType::Size, @ref Json::parseSizes()
             */
            UnsignedLong,

            /**
             * 53-bit signed integer value.
             *
             * Set if @ref Json::parseLongs() is called on a particular
             * subtree. Except for invalid values, parsing fails also fails if
             * any the values have a non-zero fractional part, if they have an
             * exponent, if they're negative or if they can't fit into 53 bits
             * (which is the representable signed integer range in a JSON).
             */
            Long,

            /**
             * Size value. Alias to @ref ParsedType::UnsignedInt or
             * @ref ParsedType::UnsignedLong depending on whether the system is
             * @ref CORRADE_TARGET_32BIT "32-bit" or 64-bit.
             * @see @ref Json::parseSizes()
             */
            Size
                #ifndef DOXYGEN_GENERATING_OUTPUT
                #ifndef CORRADE_TARGET_32BIT
                = UnsignedLong
                #else
                = UnsignedInt
                #endif
                #endif
                ,

            /** An object, array, null, bool or a string value. */
            Other = Long + 1
        };

        /**
         * @brief Construct from an internal token data reference
         *
         * The @p token is expected to be a *reference* previously returned
         * from @ref token() for a @ref JsonToken belonging to given @p json
         * instance.
         */
        explicit JsonToken(const Json& json, const JsonTokenData& token) noexcept;

        /**
         * @brief Internal token data reference
         *
         * The @ref JsonTokenData reference is not useful for anything on its
         * own, can only be passed back to @ref JsonToken(const Json&, const JsonTokenData&)
         * to form a @ref JsonToken instance again. Note that it has to stay a
         * reference, passing it by value will lose its relation to the
         * @ref Json instance that owns it.
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline const JsonTokenData& token() const;

        /**
         * @brief Token data
         *
         * Contains raw unparsed token data, including all child tokens (if
         * any). The first byte implies @ref type():
         *
         * -    `{` is a @ref Type::Object. Spans until and including the
         *      closing `}`. Child token tree is exposed through
         *      @ref children(). Immediate children are keys, second-level
         *      children are values.
         * -    `[` is a @ref Type::Array. Spans until and including the
         *      closing `]`. Child token tree is exposed through
         *      @ref children().
         * -    `n` is a @ref Type::Null. Not guaranteed to be a valid value if
         *      @ref isParsed() is not set.
         * -    `t` or `f` is a @ref Type::Bool. Not guaranteed to be a valid
         *      value if @ref isParsed() is not set.
         * -    `-` or `0` to `9` is a @ref Type::Number. Not guaranteed to be
         *      a valid value if @ref isParsed() is not set.
         * -    `"` is a @ref Type::String. If an object key, @ref children()
         *      contains the value token tree, but the token data always spans
         *      only until and including the closing `"`. Not guaranteed to be
         *      a valid value and may contain escape sequences if
         *      @ref isParsed() is not set.
         *
         * Returned view points to data owned by the originating @ref Json
         * instance, or to the string passed to @ref Json::fromString() if it
         * was called with @ref Containers::StringViewFlag::Global set. The
         * global flag is preserved in the returned value here, same as with
         * @ref asString().
         */
        Containers::StringView data() const;

        /** @brief Token type */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline Type type() const;

        /**
         * @brief Common array type
         *
         * Returns a type of the array tokens or @ref Containers::NullOpt if
         * the array is heterogeneous or empty. Expects that @ref type() is
         * @ref JsonToken::Type::Array, but @ref isParsed() doesn't have to be
         * set.
         * @see @ref commonParsedArrayType(), @ref childCount()
         */
        Containers::Optional<Type> commonArrayType() const;

        /**
         * @brief Whether the token value is parsed
         *
         * If set, the value can be accessed directly by @ref asObject(),
         * @ref asArray(), @ref asNull(), @ref asBool(), @ref asDouble(),
         * @ref asFloat(), @ref asUnsignedInt(), @ref asInt(),
         * @ref asUnsignedLong(), @ref asLong(), @ref asSize() or
         * @ref asString() function based on @ref type() and @ref parsedType()
         * and the call will not fail. If not set, the value has to be parsed
         * first.
         *
         * Tokens can be parsed during the initial run by passing
         * @ref Json::Option::ParseLiterals,
         * @relativeref{Json::Option,ParseDoubles},
         * @relativeref{Json::Option,ParseStringKeys} or
         * @relativeref{Json::Option,ParseStrings} to
         * @ref Json::fromString() or @ref Json::fromFile(); selectively
         * later using @ref Json::parseLiterals(),
         * @relativeref{Json,parseDoubles()}, @relativeref{Json,parseFloats()},
         * @relativeref{Json,parseUnsignedInts()},
         * @relativeref{Json,parseInts()},
         * @relativeref{Json,parseUnsignedLongs()},
         * @relativeref{Json,parseLongs()}, @relativeref{Json,parseSizes()},
         * @relativeref{Json,parseStringKeys()} or
         * @relativeref{Json,parseStrings()}; or on a per-token basis using
         * @ref Json::parseObject(), @relativeref{Json,parseArray()},
         * @relativeref{Json,parseNull()}, @relativeref{Json,parseBool()},
         * @relativeref{Json,parseDouble()}, @relativeref{Json,parseFloat()},
         * @relativeref{Json,parseUnsignedInt()}, @relativeref{Json,parseInt()},
         * @relativeref{Json,parseUnsignedLong()},
         * @relativeref{Json,parseLong()}, @relativeref{Json,parseSize()} or
         * @relativeref{Json,parseString()}.
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline bool isParsed() const;

        /**
         * @brief Parsed token type
         *
         * @see @ref type(), @ref isParsed()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline ParsedType parsedType() const;

        /**
         * @brief Common parsed array type
         *
         * Returns a parsed type of the array tokens or @ref Containers::NullOpt
         * if the array is not parsed at all, only partially, to a
         * heterogeneous @ref ParsedType or is empty. Expects that @ref type()
         * is @ref JsonToken::Type::Array. The returned value is never
         * @ref ParsedType::None, however @ref ParsedType::Other can be
         * returned also if the array is a combination of nulls and bools, for
         * example --- use this function in combination with
         * @ref commonArrayType() to distinguish that case. The returned value
         * is independent from @ref isParsed() being set or not.
         * @see @ref commonArrayType(), @ref childCount()
         */
        Containers::Optional<ParsedType> commonParsedArrayType() const;

        /**
         * @brief Child token count
         *
         * Number of all child tokens, including nested token trees. For
         * @ref Type::Null, @ref Type::Bool, @ref Type::Number and value
         * @ref Type::String always returns @cpp 0 @ce, for a @ref Type::String
         * that's an object key always returns @cpp 1 @ce. For an array with a
         * common type that isn't @ref Type::Object and @ref Type::Array
         * returns the array size.
         * @see @ref commonArrayType()
         */
        std::size_t childCount() const;

        /**
         * @brief Child token tree
         *
         * Contains all child tokens ordered in a depth-first manner as
         * described in @ref Utility-Json-tokenization. Returned view points
         * to data owned by the originating @ref Json instance.
         * @see @ref childCount(), @ref parent()
         */
        JsonView children() const;

        /**
         * @brief First child token
         *
         * Returns first child token or an invalid iterator if there are no
         * child tokens. In particular, for a non-empty @ref Type::Object the
         * first immediate child is a @ref Type::String, which then contains
         * the value as a child token tree. @ref Type::Null, @ref Type::Bool
         * and @ref Type::Number tokens return @cpp nullptr @ce always.
         * Accessing the first child has a @f$ \mathcal{O}(1) @f$ complexity.
         * Returned iterator points to data owned by the originating @ref Json
         * instance.
         * @see @ref JsonIterator::operator bool(), @ref parent(), @ref next()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline JsonIterator firstChild() const;

        /**
         * @brief Next token
         *
         * Return next token at the same or higher level, or an iterator to
         * (one value after) the end. Accessing the next token has a
         * @f$ \mathcal{O}(1) @f$ complexity. Returned iterator points to data
         * owned by the originating @ref Json instance.
         * @see @ref parent()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline JsonIterator next() const;

        /**
         * @brief Parent token
         *
         * Returns parent token or an invalid iterator if the token is the root
         * token. Accessing the parent token is done by traversing the token
         * list backwards and thus has a @f$ \mathcal{O}(n) @f$ complexity ---
         * where possible, it's encouraged to remember the parent instead of
         * using this function. Returned iterator points to data owned by the
         * originating @ref Json instance.
         * @see @ref JsonIterator::operator bool(), @ref firstChild(),
         *      @ref next(), @ref Json::root()
         */
        JsonIterator parent() const;

        /**
         * @brief Get an iterable object
         *
         * Expects that the token is a @ref Type::Object and @ref isParsed() is
         * set, accessing @ref JsonObjectItem::key() then expects that the key
         * token has @ref isParsed() set. See @ref Utility-Json-usage-iteration
         * for more information. Iteration through object keys is performed
         * using @ref next(), which has a @f$ \mathcal{O}(1) @f$ complexity.
         * @see @ref type(), @ref Json::Option::ParseLiterals,
         *      @ref Json::Option::ParseStringKeys, @ref Json::parseLiterals(),
         *      @ref Json::parseStringKeys(), @ref Json::parseObject(),
         *      @ref Json::parseString()
         */
        JsonObjectView asObject() const;

        /**
         * @brief Get an iterable array
         *
         * Expects that the token is a @ref Type::Array and @ref isParsed() is
         * set. See @ref Utility-Json-usage-iteration for more information.
         * Iteration through array values is performed using @ref next(), which
         * has a @f$ \mathcal{O}(1) @f$ complexity.
         * @see @ref type(), @ref commonArrayType(),
         *      @ref Json::Option::ParseLiterals, @ref Json::parseLiterals(),
         *      @ref Json::parseArray(), @ref asBitArray(),
         *      @ref asDoubleArray(), @ref asFloatArray(),
         *      @ref asUnsignedIntArray(), @ref asIntArray(),
         *      @ref asUnsignedLongArray(), @ref asLongArray(),
         *      @ref asSizeArray(), @ref asStringArray()
         */
        JsonArrayView asArray() const;

        /**
         * @brief Find an object value by key
         *
         * Expects that the token is a @ref Type::Object, @ref isParsed() is
         * set and its keys have @ref isParsed() set as well. If @p key is
         * found, returns iterator to the child token corresponding to its
         * value, otherwise returns an invalid iterator.
         *
         * Note that there's no acceleration structure built at parse time and
         * thus the operation has a @f$ \mathcal{O}(n) @f$ complexity, where
         * @f$ n @f$ is the number of keys in given object. When looking up
         * many keys in a larger object, it's thus recommended to iterate
         * through @ref asObject() than to repeatedly call this function.
         * @see @ref JsonIterator::operator bool(), @ref type(),
         *      @ref Json::Option::ParseLiterals,
         *      @ref Json::Option::ParseStringKeys, @ref Json::parseLiterals(),
         *      @ref Json::parseStringKeys(), @ref Json::parseObject()
         */
        JsonIterator find(Containers::StringView key) const;

        /**
         * @brief Find an array value by index
         *
         * Expects that the token is a @ref Type::Array and @ref isParsed() is
         * set. If @p index is found, returns iterator to the corresponding
         * token, otherwise returns an invalid iterator.
         *
         * Note that there's no acceleration structure built at parse time and
         * thus the operation has a @f$ \mathcal{O}(n) @f$ complexity, where
         * @f$ n @f$ is the number of items in given array. When looking up
         * many indices in a larger array, it's thus recommended to iterate
         * through @ref asArray() than to repeatedly call this function.
         * @see @ref JsonIterator::operator bool(), @ref type(),
         *      @ref Json::Option::ParseLiterals, @ref Json::parseLiterals(),
         *      @ref Json::parseArray()
         */
        JsonIterator find(std::size_t index) const;

        /**
         * @brief Access an object value by key
         *
         * Compared to @ref find(Containers::StringView) const expects also
         * that @p key exists.
         */
        JsonToken operator[](Containers::StringView key) const;

        /**
         * @brief Access an array value by index
         *
         * Compared to @ref find(std::size_t) const expects also that @p index
         * exists.
         */
        JsonToken operator[](std::size_t index) const;

        /**
         * @brief Get a parsed null value
         *
         * Expects that the token is @ref Type::Null and @ref isParsed() is
         * set. If not, use @ref Json::parseNull() instead.
         * @see @ref Json::Option::ParseLiterals, @ref Json::parseLiterals()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::nullptr_t asNull() const;

        /**
         * @brief Get a parsed boolean value
         *
         * Expects that the token is @ref Type::Bool and @ref isParsed() is
         * set. If not, use @ref Json::parseBool() instead.
         * @see @ref Json::Option::ParseLiterals, @ref Json::parseLiterals(),
         *      @ref asBitArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline bool asBool() const;

        /**
         * @brief Get a parsed 64-bit floating-point value
         *
         * Expects that the token is already parsed as a
         * @ref ParsedType::Double. If not, use @ref Json::parseDouble()
         * instead.
         * @see @ref Json::Option::ParseDoubles, @ref Json::parseDoubles(),
         *      @ref asDoubleArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline double asDouble() const;

        /**
         * @brief Get a parsed 32-bit floating-point value
         *
         * Expects that the token is already parsed as a
         * @ref ParsedType::Float. If not, use @ref Json::parseFloat() instead.
         * @see @ref Json::Option::ParseFloats, @ref Json::parseFloats(),
         *      @ref asFloatArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline float asFloat() const;

        /**
         * @brief Get a parsed unsigned 32-bit integer value
         *
         * Expects that the token is already parsed as a
         * @ref ParsedType::UnsignedInt. If not, use
         * @ref Json::parseUnsignedInt() instead.
         * @see @ref Json::parseUnsignedInts(), @ref asUnsignedIntArray(),
         *      @ref asSize()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::uint32_t asUnsignedInt() const;

        /**
         * @brief Get a parsed signed 32-bit integer value
         *
         * Expects that the token is already parsed as a
         * @ref ParsedType::Int. If not, use @ref Json::parseInt() instead.
         * @see @ref Json::parseInts(), @ref asIntArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::int32_t asInt() const;

        /**
         * @brief Get a parsed unsigned 52-bit integer value
         *
         * Expects that the value is already parsed as a
         * @ref ParsedType::UnsignedLong. If not, use
         * @ref Json::parseUnsignedLong() instead.
         * @see @ref Json::parseUnsignedLongs(), @ref asUnsignedLongArray(),
         *      @ref asSize()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::uint64_t asUnsignedLong() const;

        /**
         * @brief Get a parsed signed 53-bit integer value
         *
         * Expects that the token is already parsed as a
         * @ref ParsedType::Long. If not, use @ref Json::parseLong() instead.
         * @see @ref Json::parseLongs(), @ref asLongArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::int64_t asLong() const;

        /**
         * @brief Get a parsed size value
         *
         * Expects that the value is already parsed as a
         * @ref ParsedType::Size. If not, use @ref Json::parseSize() instead.
         * @see @ref Json::parseSizes(), @ref asSizeArray()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline std::size_t asSize() const;

        /**
         * @brief Get a parsed string value
         *
         * Expects that the token is a @ref Type::String and @ref isParsed() is
         * set. If not, use @ref Json::parseString() instead. If
         * @ref Json::fromString() was called with a global literal and the
         * string didn't contain any escape sequences, the returned view has
         * @ref Containers::StringViewFlag::Global set. If not, the view points
         * to data owned by the originating @ref Json instance.
         * @see @ref Json::Option::ParseStringKeys,
         *      @ref Json::Option::ParseStrings, @ref Json::parseStringKeys(),
         *      @ref Json::parseStrings()
         */
        Containers::StringView asString() const;

        /**
         * @brief Get a parsed boolean array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * @ref Type::Bool tokens and exactly @p expectedSize elements if it's
         * not @cpp 0 @ce, already parsed. If not, use
         * @ref Json::parseBitArray() instead. The returned view points to data
         * owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref asBool(),
         *      @ref asArray()
         */
        Containers::StridedBitArrayView1D asBitArray(std::size_t expectedSize = 0) const;

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @brief Get a parsed boolean array
         * @m_deprecated_since_latest Use @ref asBitArray() instead.
         */
        CORRADE_DEPRECATED("use asBitArray() instead") Containers::StridedArrayView1D<const bool> asBoolArray(std::size_t expectedSize = 0) const;
        #endif

        /**
         * @brief Get a parsed 64-bit floating-point array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::Double. If not, use
         * @ref Json::parseDoubleArray() instead. The returned view points to
         * data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asDouble(), @ref asArray()
         */
        Containers::StridedArrayView1D<const double> asDoubleArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed 32-bit floating-point array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::Float. If not, use
         * @ref Json::parseFloatArray() instead. The returned view points to
         * data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asFloat(), @ref asArray()
         */
        Containers::StridedArrayView1D<const float> asFloatArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed unsigned 32-bit integer array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::UnsignedInt. If not,
         * use @ref Json::parseUnsignedIntArray() instead. The returned view
         * points to data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asUnsignedInt(), @ref asSizeArray(), @ref asArray()
         */
        Containers::StridedArrayView1D<const std::uint32_t> asUnsignedIntArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed signed 32-bit integer array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::Int. If not, use
         * @ref Json::parseIntArray() instead. The returned view points to data
         * owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asInt(), @ref asArray()
         */
        Containers::StridedArrayView1D<const std::int32_t> asIntArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed unsigned 52-bit integer array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::UnsignedLong. If not,
         * use @ref Json::parseUnsignedLongArray() instead. The returned view
         * points to data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asUnsignedLong(), @ref asSizeArray(), @ref asArray()
         */
        Containers::StridedArrayView1D<const std::uint64_t> asUnsignedLongArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed signed 53-bit integer value array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * numeric tokens and exactly @p expectedSize elements if it's not
         * @cpp 0 @ce, already parsed as @ref ParsedType::Long. If not, use
         * @ref Json::parseLongArray() instead. The returned view points to
         * data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asLong(), @ref asArray()
         */
        Containers::StridedArrayView1D<const std::int64_t> asLongArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed size array
         *
         * Convenience function that calls into @ref asUnsignedIntArray() on
         * @ref CORRADE_TARGET_32BIT "32-bit targets" and into
         * @ref asUnsignedLongArray() on 64-bit.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref parsedType(),
         *      @ref asSize(), @ref asArray()
         */
        Containers::StridedArrayView1D<const std::size_t> asSizeArray(std::size_t expectedSize = 0) const;

        /**
         * @brief Get a parsed string array
         *
         * Expects that the token is a @ref Type::Array consisting of just
         * @ref Type::String tokens and exactly @p expectedSize elements if
         * it's not @cpp 0 @ce, already parsed. If not, use
         * @ref Json::parseStringArray() instead. If @ref Json::fromString()
         * was called with a global literal and the strings didn't contain any
         * escape sequences, the returned views have
         * @ref Containers::StringViewFlag::Global set. If not, the views point
         * to data owned by the originating @ref Json instance.
         *
         * The @p expectedSize parameter is ignored on a @ref CORRADE_NO_ASSERT
         * build.
         * @see @ref type(), @ref commonArrayType(), @ref asString(),
         *      @ref asArray()
         */
        Containers::StringIterable asStringArray(std::size_t expectedSize = 0) const;

    private:
        friend Json;
        friend JsonView;
        friend JsonObjectView;
        friend JsonArrayView;
        friend JsonTokenData; /* uses the constants */
        friend JsonIterator;
        friend JsonObjectIterator;
        friend JsonArrayIterator;

        explicit JsonToken(const Implementation::JsonData& json, std::size_t token) noexcept: _json{&json}, _token{token} {}

        /* Used by asString() as well as find(Containers::StringView), doesn't
           assert token type and whether it's parsed */
        CORRADE_UTILITY_LOCAL Containers::StringView asStringInternal() const;

        enum: std::uint64_t {
            /* If the token has these bits set, type is stored in TypeSmall*
               / TypeLarge* bits, otherwise the type is in the token size and
               stored in TypeTokenSize* bits */
            NanMask = 0xfffull << 52, /* sign bit + NaN, 0b111111111111 */
            Nan = 0x7ffull << 52, /* +NaN, 0b011111111111 */

            /* This bit is set for parsed small or large types. 64-bit types,
               which are if TypeTokenSizeMask is non-zero, are all parsed. */
            TypeSmallLargeIsParsed = 0x1ull << 48,          /* 0b0001____ */

            TypeSmallMask = NanMask|(0xfcull << 44), /* sign, NaN, 6 bits after */
            /* All TypeSmall* use just 32 bits for data out of the remaining 46
               bits. The remaining 14 bits are currently unused. */
            TypeSmallDataMask = 0xffffffffull, /* the final 32 bits */
            /* All TypeSmall* have the first two bits out of five zero */
            /* This bit is set for number types */
            TypeSmallIsNumber = 0x20ull << 44,              /* 0b001000__ */
            TypeSmallNull = Nan|(0x00ull << 44),            /* 0b000?00__ */
            TypeSmallBool = Nan|(0x04ull << 44),            /* 0b000?01__ */
            TypeSmallNumber = Nan|(0x20ull << 44),          /* 0b001000__ */
            /* These have both the IsNumber and IsParsed bit implicitly set */
            TypeSmallFloat = Nan|(0x34ull << 44),           /* 0b001101__ */
            TypeSmallUnsignedInt = Nan|(0x38ull << 44),     /* 0b001110__ */
            TypeSmallInt = Nan|(0x3cull << 44),             /* 0b001111__ */

            TypeLargeMask = NanMask|(0xfull << 48), /* sign, NaN, 4 bits after */
            TypeLargeDataMask = 0xffffffffffffull, /* the final 48 bits */
            /* All TypeSmall* have the first two bits out of five non-zero */
            TypeIsLarge = 0xcull << 48,                     /* 0b1100 */
            /* This bit is set for string types */
            TypeLargeIsString = 0x8ull << 48,               /* 0b1000 */
            /* This bit is set for string types that are a key */
            TypeLargeStringIsKey = 0x4ull << 48,            /* 0b0100 */
            /* This bit is set for string types that are escaped */
            TypeLargeStringIsEscaped = 0x2ull << 48,        /* 0b0010 */
            TypeLargeObject = Nan|(0x4ull << 48),           /* 0b010? */
            TypeLargeArray = Nan|(0x6ull << 48),            /* 0b011? */
            TypeLargeString = Nan|(0x8ull << 48),           /* 0b100? */
            TypeLargeStringEscaped = Nan|(0xaull << 48),    /* 0b101? */
            TypeLargeStringKey = Nan|(0xcull << 48),        /* 0b110? */
            TypeLargeStringKeyEscaped = Nan|(0xeull << 48), /* 0b111? */
        };

        /* The final 2 bits of a 32/64-bit token offset are used to distinguish
           between 64-bit types. If both bits are 0, the token should have
           NanMask set and the type described by the 4 or 5 bits after it. */
        enum: std::size_t {
            TypeTokenSizeMask = 0x3ull << (sizeof(std::size_t)*8 - 2),
            TypeTokenSizeOther = 0x0ull << (sizeof(std::size_t)*8 - 2),
            TypeTokenSizeDouble = 0x1ull << (sizeof(std::size_t)*8 - 2),
            TypeTokenSizeUnsignedLong = 0x2ull << (sizeof(std::size_t)*8 - 2),
            TypeTokenSizeLong = 0x3ull << (sizeof(std::size_t)*8 - 2),
        };

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _token;
};

/** @debugoperatorclassenum{JsonToken,Type} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, JsonToken::Type value);

/** @debugoperatorclassenum{JsonToken,ParsedType} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, JsonToken::ParsedType value);

/**
@brief Data of a single JSON token
@m_since_latest

Storage for the actual JSON token data inside a @ref Json, internally
referenced from @ref JsonToken instances and accessible through
@ref JsonToken::token(). Is not usable on its own, pass it to
@ref JsonToken::JsonToken(const Json&, const JsonTokenData&) to access the data
it references.

@experimental
*/
class CORRADE_UTILITY_EXPORT JsonTokenData {
    private:
        friend Json;
        friend JsonToken;
        friend JsonObjectIterator; /* uses childCount() */
        friend JsonArrayIterator; /* uses childCount() */

        /* This constructor is currently private because user code should have
           no need to create instances of this class directly */
        explicit JsonTokenData(NoInitT) /*nothing*/ {}

        /* These are all private because they may depend on spatial locality
           relative to other tokens, which has to be ensured externally */
        std::size_t childCount() const;
        inline const JsonTokenData& next() const;
        inline bool isNumber() const;
        JsonToken::Type type() const;
        bool isParsed() const;
        JsonToken::ParsedType parsedType() const;

        /* See Json.cpp for detailed layout description */
        const char* _data;
        /* Upper two bits used for TypeTokenSize* values */
        std::size_t _sizeType;
        union {
            /* The upper 12 bits contain a NaN, the next four / five bits
               contain a type. For objects and arrays, the lower 48 bits is
               abused for storing parent token index. After tokenization, they
               contain child count. */
            std::uint64_t _dataTypeNan;
            /* Wouldn't the shorter types clash with NaN on BE? */
            bool _parsedBool;
            double _parsedDouble;
            float _parsedFloat;
            std::uint64_t _parsedUnsignedLong;
            std::int64_t _parsedLong;
            std::uint32_t _parsedUnsignedInt;
            std::int32_t _parsedInt;
        };
};

/**
@brief JSON object item
@m_since_latest

Returned when iterating @ref JsonToken::asObject(). See
@ref Utility-Json-usage-iteration for more information.

@experimental
*/
class CORRADE_UTILITY_EXPORT JsonObjectItem {
    public:
        /**
         * @brief Key
         *
         * Equivalent to calling @ref JsonToken::asString() on the token.
         */
        Containers::StringView key() const;

        /**
         * @brief Value
         *
         * Equvialent to accessing @ref JsonToken::firstChild() on the token.
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline JsonToken value() const;

        /**
         * @brief Key token
         *
         * Key is retrivable as @ref JsonToken::asString() on the returned
         * token, value as @ref JsonToken::firstChild().
         */
        /*implicit*/ operator JsonToken() const { return _token; }

    private:
        friend JsonObjectIterator;

        explicit JsonObjectItem(JsonToken token) noexcept: _token{token} {}

        JsonToken _token;
};

/**
@brief JSON array item
@m_since_latest

Returned when iterating @ref JsonToken::asObject(). See
@ref Utility-Json-usage-iteration for more information.

@experimental
*/
class JsonArrayItem {
    public:
        /** @brief Array index */
        std::size_t index() const { return _index; }

        /** @brief Value */
        JsonToken value() const { return _token; }

        /** @brief Value */
        /*implicit*/ operator JsonToken() const { return _token; }

    private:
        friend JsonArrayIterator;

        explicit JsonArrayItem(std::size_t index, JsonToken token) noexcept: _index{index}, _token{token} {}

        std::size_t _index;
        JsonToken _token;
};

namespace Implementation {

/* Is inherited by Json::State with more members, this contains just enough to
   be able to inline hot paths but not pull in Array etc. headers */
struct JsonData {
    const JsonTokenData* tokens;
    std::size_t tokenCount;

    /* Disallow accidental deletion through the base pointer */
    protected:
        ~JsonData() = default;
};

}

/**
@brief JSON iterator
@m_since_latest

Iterator for @ref JsonView, which is returned from @ref Json::tokens() and
@ref JsonToken::children(), and additionally directly returned from
@ref JsonToken::firstChild(), @ref JsonToken::parent(), @ref JsonToken::find(),
@ref JsonObjectView::find() and @ref JsonArrayView::find(), where it indicates
a potentially invalid value.

Compared to @ref JsonObjectIterator and @ref JsonArrayIterator iterates all
tokens in a depth-first manner instead of just the immediate children, and can
go both ways, not just forward. See @ref Utility-Json-tokenization for more
information about the internal representation.
@experimental
*/
class JsonIterator {
    public:
        /**
         * @brief Default constructor
         *
         * Creates an invalid iterator, i.e. one with @ref operator bool()
         * returning @cpp false @ce.
         */
        /*implicit*/ JsonIterator(): _json{}, _token{} {}

        /** @brief Construct from a token */
        /*implicit*/ JsonIterator(JsonToken token) noexcept: _json{token._json}, _token{token._token} {}

        /** @brief Equality comparison */
        bool operator==(const JsonIterator& other) const {
            return _token == other._token;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const JsonIterator& other) const {
            return _token != other._token;
        }

        /**
         * @brief Whether the iterator is valid
         *
         * Returns @cpp false @ce if the iterator is default-constructed,
         * returned from @ref JsonToken::firstChild() of a token that has no
         * children, from @ref JsonToken::parent() of a token that has no
         * parent, from @ref JsonToken::find(), @ref JsonObjectView::find()
         * or @ref JsonArrayView::find() if given key or index wasn't found, or
         * if it was advanced outside of the bounds of the token stream,
         * @cpp true @ce otherwise.
         */
        explicit operator bool() const {
            return _json && _token < _json->tokenCount;
        }

        /**
         * @brief Advance to previous position
         *
         * The iterator is expected to be valid and not at the begin of the
         * token stream. Note that compared to @ref JsonObjectIterator and
         * @ref JsonArrayIterator it advances in a depth-first manner instead
         * of just the immediate children.
         * @see @ref operator bool()
         */
        JsonIterator& operator--() {
            CORRADE_DEBUG_ASSERT(_json, "Utility::JsonIterator::operator--(): the iterator is invalid", *this);
            CORRADE_DEBUG_ASSERT(_token > 0,
                "Utility::JsonIterator::operator--(): advancing past the begin of the token stream", *this);
            --_token;
            return *this;
        }

        /**
         * @brief Advance to next position
         *
         * The iterator is expected to be valid. Note that compared to
         * @ref JsonObjectIterator and @ref JsonArrayIterator it advances in a
         * depth-first manner instead of just the immediate children.
         * @see @ref operator bool()
         */
        JsonIterator& operator++() {
            CORRADE_DEBUG_ASSERT(_json, "Utility::JsonIterator::operator++(): the iterator is invalid", *this);
            CORRADE_DEBUG_ASSERT(_token < _json->tokenCount,
                "Utility::JsonIterator::operator++(): advancing past the end of the token stream", *this);
            ++_token;
            return *this;
        }

        /**
         * @brief Dereference
         *
         * The iterator is expected to be valid.
         * @see @ref operator bool()
         */
        JsonToken operator*() const {
            CORRADE_DEBUG_ASSERT(_json && _token < _json->tokenCount,
                "Utility::JsonIterator::operator*(): the iterator is invalid", (JsonToken{*_json, _token}));
            return JsonToken{*_json, _token};
        }

        /**
         * @brief Dereference
         *
         * The iterator is expected to be valid.
         * @see @ref operator bool()
         */
        const JsonToken* operator->() const {
            CORRADE_DEBUG_ASSERT(_json && _token < _json->tokenCount,
                "Utility::JsonIterator::operator->(): the iterator is invalid", reinterpret_cast<const JsonToken*>(this));
            return reinterpret_cast<const JsonToken*>(this);
        }

    private:
        friend JsonToken;
        friend JsonView;

        explicit JsonIterator(const Implementation::JsonData* json, std::size_t token) noexcept: _json{json}, _token{token} {
            /* Has to be here because in the class definition the type would
               still be incomplete. Cannot be free-standing in Json.cpp because
               the members are private. */
            static_assert(
                offsetof(JsonToken, _json) == offsetof(JsonIterator, _json) &&
                offsetof(JsonToken, _token) == offsetof(JsonIterator, _token),
                "incompatible JsonToken and JsonIterator class layout");
        }

        /* Compared to other cases, here it can be null. The members need to
           have the same layout as JsonToken for the operator->() to work, see
           the static_assert() above. */
        const Implementation::JsonData* _json;
        std::size_t _token;
};

/**
@brief JSON object iterator
@m_since_latest

Iterator for @ref JsonObjectView, which is returned from
@ref Json::parseObject() and @ref JsonToken::asObject(). See
@ref Utility-Json-usage-iteration for more information.

@experimental
*/
class JsonObjectIterator {
    public:
        /** @brief Equality comparison */
        bool operator==(const JsonObjectIterator& other) const {
            return _token == other._token;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const JsonObjectIterator& other) const {
            return _token != other._token;
        }

        /**
         * @brief Advance to next position
         *
         * Expects that the iterator is not at the end of the token stream.
         * It's however not possible to detect advancing outside of the
         * iterated object, such iterators have undefined behavior.
         * Implemented using @ref JsonToken::next().
         */
        JsonObjectIterator& operator++() {
            CORRADE_DEBUG_ASSERT(_token < _json->tokenCount,
                "Utility::JsonObjectIterator::operator++(): advancing past the end of the token stream", *this);
            _token += _json->tokens[_token].childCount() + 1;
            return *this;
        }

        /**
         * @brief Dereference
         *
         * Expects that the iterator is not at the end of the token stream.
         * It's however not possible to detect advancing outside of the
         * iterated object, dereferencing such iterators has undefined
         * behavior.
         */
        JsonObjectItem operator*() const {
            CORRADE_DEBUG_ASSERT(_token < _json->tokenCount,
                "Utility::JsonObjectIterator::operator*(): dereferencing iterator at the end of the token stream", (JsonObjectItem{JsonToken{*_json, _token}}));
            return JsonObjectItem{JsonToken{*_json, _token}};
        }

    private:
        friend JsonObjectView;

        explicit JsonObjectIterator(const Implementation::JsonData& json, std::size_t token) noexcept: _json{&json}, _token{token} {}

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _token;
};

/**
@brief JSON array iterator
@m_since_latest

Iterator for @ref JsonArrayView, which is returned from @ref Json::parseArray()
and @ref JsonToken::asArray(). See @ref Utility-Json-usage-iteration for more
information.

@experimental
*/
class JsonArrayIterator {
    public:
        /** @brief Equality comparison */
        bool operator==(const JsonArrayIterator& other) const {
            /* _index is implicit, no need to compare */
            return _token == other._token;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const JsonArrayIterator& other) const {
            /* _index is implicit, no need to compare */
            return _token != other._token;
        }

        /**
         * @brief Advance to next position
         *
         * Expects that the iterator is not at the end of the token stream.
         * It's however not possible to detect advancing outside of the
         * iterated array, such iterators have undefined behavior. Implemented
         * using @ref JsonToken::next().
         */
        JsonArrayIterator& operator++() {
            CORRADE_DEBUG_ASSERT(_token < _json->tokenCount,
                "Utility::JsonArrayIterator::operator++(): advancing past the end of the token stream", *this);
            ++_index;
            _token += _json->tokens[_token].childCount() + 1;
            return *this;
        }

        /**
         * @brief Dereference
         *
         * Expects that the iterator is not at the end of the token stream.
         * It's however not possible to detect advancing outside of the
         * iterated array, dereferencing such iterators has undefined behavior.
         */
        JsonArrayItem operator*() const {
            CORRADE_DEBUG_ASSERT(_token < _json->tokenCount,
                "Utility::JsonArrayIterator::operator*(): dereferencing iterator at the end of the token stream", (JsonArrayItem{_index, JsonToken{*_json, _token}}));
            return JsonArrayItem{_index, JsonToken{*_json, _token}};
        }

    private:
        friend JsonArrayView;

        explicit JsonArrayIterator(const Implementation::JsonData& json, std::size_t index, std::size_t token) noexcept: _json{&json}, _index{index}, _token{token} {}

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _index;
        std::size_t _token;
};

/**
@brief JSON token subtree
@m_since_latest

Returned from @ref Json::tokens() and @ref JsonToken::children(). Note that,
unlike with @ref JsonObjectView and @ref JsonArrayView, the iteration is
performed on the whole subtree in a depth-first order, it doesn't iterate just
the immediate children.

@experimental
*/
class JsonView {
    public:
        /**
         * @brief Token count
         *
         * Note that the returned value includes all nested children as well,
         * it isn't just the count of immediate children.
         * @see @ref size()
         */
        std::size_t size() { return _end - _begin; }

        /**
         * @brief Whether the view is empty
         *
         * @see @ref size()
         */
        bool isEmpty() const { return _end == _begin; }

        /**
         * @brief Iterator to the first element
         *
         * If the view is empty, the iterator reports itself as valid but may
         * point outside of the actual token data.
         * @see @ref isEmpty(), @ref JsonIterator::operator bool(),
         *      @ref front()
         */
        JsonIterator begin() const { return JsonIterator{_json, _begin}; }
        /** @overload */
        JsonIterator cbegin() const { return JsonIterator{_json, _begin}; }

        /**
         * @brief Iterator to (one item after) the last element
         *
         * The returned iterator reports itself as valid but may point outside
         * of the actual token data.
         * @see @ref JsonIterator::operator bool(), @ref back()
         */
        JsonIterator end() const { return JsonIterator{_json, _end}; }
        /** @overload */
        JsonIterator cend() const { return JsonIterator{_json, _end}; }

        /**
         * @brief First token
         *
         * Expects there is at least one token.
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        JsonToken front() const {
            CORRADE_DEBUG_ASSERT(_begin != _end,
                "Utility::JsonView::front(): view is empty", (JsonToken{*_json, _begin}));
            return JsonToken{*_json, _begin};
        }

        /**
         * @brief Last token
         *
         * Expects there is at least one token.
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        JsonToken back() const {
            CORRADE_DEBUG_ASSERT(_begin != _end,
                "Utility::JsonView::back(): view is empty", (JsonToken{*_json, _begin}));
            return JsonToken{*_json, _end - 1};
        }

        /**
         * @brief Token access
         *
         * Expects that @p i is less than @ref size().
         */
        JsonToken operator[](std::size_t i) const {
            CORRADE_DEBUG_ASSERT(_begin + i < _end,
                "Utility::JsonView::operator[](): index" << i << "out of range for" << _end - _begin << "elements", (JsonToken{*_json, _begin}));
            return JsonToken{*_json, _begin + i};
        }

    private:
        friend Json;
        friend JsonToken;

        explicit JsonView(const Implementation::JsonData& json, std::size_t begin, std::size_t size) noexcept: _json{&json}, _begin{begin}, _end{begin + size} {}

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _begin, _end;
};

/**
@brief JSON object view
@m_since_latest

Returned from @ref Json::parseObject() and @ref JsonToken::asObject(). See
@ref Utility-Json-usage-iteration for more information.

@experimental
*/
class CORRADE_UTILITY_EXPORT JsonObjectView {
    public:
        /** @brief Iterator to the first element */
        JsonObjectIterator begin() const {
            return JsonObjectIterator{*_json, _begin};
        }
        /** @overload */
        JsonObjectIterator cbegin() const {
            return JsonObjectIterator{*_json, _begin};
        }

        /** @brief Iterator to (one item after) the last element */
        JsonObjectIterator end() const {
            return JsonObjectIterator{*_json, _end};
        }
        /** @overload */
        JsonObjectIterator cend() const {
            return JsonObjectIterator{*_json, _end};
        }

        /**
         * @brief Find an object value by key
         *
         * Calls @ref JsonToken::find(Containers::StringView) const on the
         * enclosing object token. Useful for performing a lookup directly on
         * the value returned from @ref Json::parseObject().
         */
        JsonIterator find(Containers::StringView key) const;

        /**
         * @brief Access an object value by key
         *
         * Calls @ref JsonToken::operator[](Containers::StringView) const on
         * the enclosing object token. Useful for performing a lookup directly
         * on the value returned from @ref Json::parseObject().
         */
        JsonToken operator[](Containers::StringView key) const;

    private:
        friend Json;
        friend JsonToken;

        explicit JsonObjectView(const Implementation::JsonData& json, std::size_t begin, std::size_t size) noexcept: _json{&json}, _begin{begin}, _end{begin + size} {}

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _begin, _end;
};

/**
@brief JSON array view
@m_since_latest

Returned from @ref Json::parseArray() and @ref JsonToken::asArray(). See
@ref Utility-Json-usage-iteration for more information.

@experimental
*/
class CORRADE_UTILITY_EXPORT JsonArrayView {
    public:
        /** @brief Iterator to the first element */
        JsonArrayIterator begin() const {
            return JsonArrayIterator{*_json, 0, _begin};
        }
        /** @overload */
        JsonArrayIterator cbegin() const {
            return JsonArrayIterator{*_json, 0, _begin};
        }

        /** @brief Iterator to (one item after) the last element */
        JsonArrayIterator end() const {
            /* JsonArrayIterator cannot be decremented and thus
               JsonArrayItem::index() cannot be called, thus the index is never
               used for anything and can be 0 for end() as well */
            return JsonArrayIterator{*_json, 0, _end};
        }
        /** @overload */
        JsonArrayIterator cend() const {
            return JsonArrayIterator{*_json, 0, _end};
        }

        /**
         * @brief Find an array value by index
         *
         * Calls @ref JsonToken::find(std::size_t) const on the enclosing array
         * token. Useful for performing a lookup directly on the value returned
         * from @ref Json::parseArray().
         */
        JsonIterator find(std::size_t index) const;

        /**
         * @brief Access an array value by index
         *
         * Calls @ref JsonToken::operator[](std::size_t) const on the enclosing
         * array token. Useful for performing a lookup directly on the value
         * returned from @ref Json::parseArray().
         */
        JsonToken operator[](std::size_t index) const;

    private:
        friend Json;
        friend JsonToken;

        explicit JsonArrayView(const Implementation::JsonData& json, std::size_t begin, std::size_t size) noexcept: _json{&json}, _begin{begin}, _end{begin + size} {}

        /* Is never null, just avoiding a Containers::Reference dependency */
        const Implementation::JsonData* _json;
        std::size_t _begin, _end;
};

inline const JsonTokenData& JsonToken::token() const {
    return _json->tokens[_token];
}

inline JsonToken::Type JsonToken::type() const {
    return _json->tokens[_token].type();
}

inline bool JsonToken::isParsed() const {
    return _json->tokens[_token].isParsed();
}

inline JsonToken::ParsedType JsonToken::parsedType() const {
    return _json->tokens[_token].parsedType();
}

inline JsonIterator JsonToken::firstChild() const {
    /* The only types that can have children are arrays, objects and string
       keys, which are all large types */
    const JsonTokenData& data = _json->tokens[_token];
    if((data._dataTypeNan & JsonToken::NanMask) == JsonToken::Nan && (data._dataTypeNan & JsonToken::TypeIsLarge)) {
        /* String keys have at least one child always */
        if((data._dataTypeNan & JsonToken::TypeLargeIsString) && (data._dataTypeNan & JsonToken::TypeLargeStringIsKey))
            return JsonIterator{_json, _token + 1};

        /* Arrays and objects have a child only if child count is non-zero */
        const std::uint64_t type = data._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed;
        if(type == JsonToken::TypeLargeArray ||
           type == JsonToken::TypeLargeObject)
            return data._dataTypeNan & JsonToken::TypeLargeDataMask ?
                JsonIterator{_json, _token + 1} : JsonIterator{};
    }

    /* Otherwise the type is a non-key string or is small (null, bool, number)
       or it's a 64-bit parsed type, neither of which have children */
    return {};
}

inline const JsonTokenData& JsonTokenData::next() const {
    return *(this + childCount() + 1);
}

inline JsonIterator JsonToken::next() const {
    return JsonIterator{_json, _token + _json->tokens[_token].childCount() + 1};
}

inline std::nullptr_t JsonToken::asNull() const {
    CORRADE_ASSERT((_json->tokens[_token]._dataTypeNan & TypeSmallMask) == (TypeSmallNull|TypeSmallLargeIsParsed),
        /* {} causes a -Wzero-as-null-pointer-constant warning on GCC 4.8 */
        "Utility::JsonToken::asNull(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), nullptr);
    return nullptr;
}

inline bool JsonToken::asBool() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeSmallMask) == (TypeSmallBool|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asBool(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});
    return data._parsedBool;
}

inline double JsonToken::asDouble() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._sizeType & TypeTokenSizeMask) == TypeTokenSizeDouble,
        "Utility::JsonToken::asDouble(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedDouble;
}

inline float JsonToken::asFloat() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeSmallMask) == TypeSmallFloat,
        "Utility::JsonToken::asFloat(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedFloat;
}

inline std::uint32_t JsonToken::asUnsignedInt() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeSmallMask) == TypeSmallUnsignedInt,
        "Utility::JsonToken::asUnsignedInt(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedUnsignedInt;
}

inline std::int32_t JsonToken::asInt() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeSmallMask) == TypeSmallInt,
        "Utility::JsonToken::asInt(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedInt;
}

inline std::uint64_t JsonToken::asUnsignedLong() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._sizeType & TypeTokenSizeMask) == TypeTokenSizeUnsignedLong,
        "Utility::JsonToken::asUnsignedLong(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedUnsignedLong;
}

inline std::int64_t JsonToken::asLong() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._sizeType & TypeTokenSizeMask) == TypeTokenSizeLong,
        "Utility::JsonToken::asLong(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedLong;
}

inline std::size_t JsonToken::asSize() const {
    const JsonTokenData& data = _json->tokens[_token];
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_ASSERT((data._sizeType & TypeTokenSizeMask) == TypeTokenSizeUnsignedLong,
        "Utility::JsonToken::asSize(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedUnsignedLong;
    #else
    CORRADE_ASSERT((data._dataTypeNan & TypeSmallMask) == TypeSmallUnsignedInt,
        "Utility::JsonToken::asSize(): token is a" << type() << "parsed as" << parsedType(), {});
    return data._parsedUnsignedInt;
    #endif
}

inline JsonToken JsonObjectItem::value() const {
    return *_token.firstChild();
}

}}

#endif
