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

#include "Json.h"

#include <climits>
#include <cmath> /* std::isinf(), std::isnan() */
#include <cstring>

#include "Corrade/Containers/Array.h"
#ifndef CORRADE_NO_ASSERT
#include "Corrade/Containers/BitArray.h"
#endif
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Unicode.h"

namespace Corrade { namespace Utility {

/*

### JSON token data layout

At the very least, the JSON token has to contain an offset to the input string
for token begin, a token byte size and, in case of objects or arrays the number
of child tokens to make traversals possible. Token type can be determined
implicitly, as mentioned in the public docs of JsonToken::data().

On 32bit architectures all three (pointer, size and child count) can be 32bit,
thus looking like on the left. On 64bit, the pointer has to be 64bit, and the
size should be larger than 32bit, as a 4 GB limitation wouldn't be wise. Due to
alignment restrictions, this means we have up to 64 bits left for the child
token count as well, as shown on the right:

    +---------+---------+    +-------------------+
    | pointer |  size   |    |      pointer      |
    +---------+---------+    +-------------------+
    | child # |              |       size        |
    +---------+ . . .        +-------------------+
                             |    child #
                             +------------ . . .

A tokenizer alone wouldn't be enough however, as a goal here is to abuse the
(already quite minimal) token layout to also store the actual parsed value:

-   Unescaped strings. These can have an unbounded size and thus have to
    be allocated externally with the token somehow storing some reference (thus
    also 32 bits isn't enough on 64 bit systems) to the unescaped variant.
-   64-bit doubles. Even though such floating-point precision is rarely needed
    and thus we could probably get away with 32-bit floats, doubles can also
    store signed integer values up to 53 bits, for example file offsets
    pointing to glTF buffers. Since we already allow JSON files over 4 GB, we
    should thus allow >32bit integers as well. For binary size savings one
    might also want to not even parse doubles but go with floats instead, or
    perform a much faster integer parsing. It should be possible to directly
    store such values as well, without having to perform a conversion from a
    double on every access.
-   Boolean and null values. Comparatively easy.

As value tokens have no children, a double (or a parsed string reference, or a
boolean value) can be stored in place of the child count. Which is
coincidentally why the diagrams above are both 8-byte aligned. Technically,
string tokens that represent object keys have children -- the object value, and
its potential children -- but such information can be again determined
implicitly.

Another goal is to have numbers parsable on-demand. Thus there needs to be a
way to know whether a token has its value already parsed or not (and for
numbers additionally whether it's a double, an int, etc.), and an ability to
change the parsed state later (turning an unparsed string to a parsed one, or
a parsed float to an int).

Finally, to reduce maintenance burden, apart from the size differences for
token data pointer and size, the representation should be the same on 32-bit
and 64-bit platforms.

### Token size

Because Containers::StringView is used as the input type, on 32-bit platforms
the total data size is limited to 1 GB due to the top two bits of the size used
for storing NullTerminated / Global flags. Which means the (32-bit) token
representation will never have token size over 1 GB either and can reuse the
two top bits for something else.

On 64-bit platforms StringView uses the top two bits as well, but as the
remaining 62 bits are more than enough for storing anything else, no practical
size limitations happen there.

    +---------+-------+-+    +-------------------+
    | pointer |  size |…|    |      pointer      |
    +---------+-------+-+    +-----------------+-+
    .                   .    |       size      |…|
                             +-----------------+-+
                             .                   .

### Storing 64-bit numeric types

A 64-bit double value is a (signed) NaN or (signed) infinity if the 11-bit
exponent is set to all 1s. Since JSON has to way to store NaN or infinity
values, let alone NaNs with custom bit patterns, we can use a NaN-like value to
store anything else in the remaining 53 bits:

    +-+---+-------------+
    |…|1…1|     …       |
    +-+---+-------------+

Additionally, as JSON's native data type is a double, it implies that the
maximum representable integer value is 52 bits for an unsigned integer, and 53
bits for a signed, any larger value will not fit into the mantissa exactly.
With the usual two's complement representation, one of the following scenarios
can happen:

    +-+---+-------------+    +-+---+-------------+
    |0| … |   +double   |    |1| … |   -double   |
    +-+---+-------------+    +-+---+-------------+

    +-+---+-------------+    +-+---+-------------+
    |0|0…0| +int/double |    |1|1…1|    -int     |
    +-+---+-------------+    +-+---+-------------+

While three cases are unambiguously distinguishable to a concrete data type,
in the fourth case with both the sign bit and exponent being all 0s it can be
a positive signed 53-bit int, an unsigned 52-bit int or a double with no
exponent. In order to be able to distinguish among those three, the
above-mentioned two bits from the token size get used.

### Storing object, array, null, bool, 32-bit numeric types and strings

From the above cases it's clear that a bit pattern with the sign bit 0 and the
exponent all 1s will be clearly distinct from all 64-bit values representable
in a JSON. Which makes it a great fit for representing all other token types:

    +-+---+-------------+
    |0|1…1|     …       |
    +-+---+-------------+

Thus there is the remaining 52 bits of the mantissa, which need to store the
following:

 - a token type identifier
 - whether the token had been parsed, and in case of numbers to which type
 - whether a string is an object key, and thus may have nested objects or
   arrays, to be able to figure child count for it
 - and one of:
    - child count for an object token
    - child count for an array token
    - index of an externally allocated string, in case the string has escape
      characters
    - a parsed 32-bit bool / int / float value

I'm picking 48 bits for child count and string index because it's a nice round
value and should be plenty. Could have picked 47 bits or less and it'd make
things slightly simpler, but here we are. Which means there's 4 bits left for
encoding the token type and whether it's parsed. However, there's precisely 20
possible type combinations:

 1. object
 2. object, parsed
 3. array
 4. array, parsed
 5. string
 6. string, parsed
 7. string with escape characters
 8. string with escape characters, parsed
 9. string that's an object key
10. string that's an object key, parsed
11. string that's an object key with escape characters
12. string that's an object key with escape characters, parsed
13. null
14. null, parsed
15. bool
16. bool, parsed
17. number
18. number, parsed as a float
19. number, parsed as an unsigned int
20. number, parsed as an int

While it could be possible to prune the six "raw" unparsed types from this list
and determine them implicitly by looking at the initial token character ({ for
objects, [ for arrays, " for strings, n for null, t/f for a bool and anything
else for a number), arriving at just 12 types that would be encodable in four
bits, doing so isn't very cache-friendly as it'd mean looking at very random
places in the original string every time. It'd also add additonal logic burden
as it won't be easy to filter parsed vs unparsed, numeric vs other types with a
simple bitwise operation.

Instead, the solution here is that the type identifier will be four-bit for the
tokens that need to store 48 bits of information, and six-bit for the tokens
that need only 32 or less, distinguished by the initial bits like with the NaN
case:

    +------+----+-------------+    +----+--------------------+
    |00NP??|    | 32-bit data |    |100P|48-bit esc string ID|
    +------+----+-------------+    +----+--------------------+

    +----+--------------------+    +----+--------------------+
    |01?P| 48-bit child count |    |111P| 48-bit esc key ID  |
    +----+--------------------+    +----+--------------------+

In other words, if the first two bits are zero, the type identifier is
five-bit, with the remaining three bits distinguishing the 8 types (13 to 20
in the above list), and the 32 bits out of the remaining 47 storing data.
Otherwise, if any of the first two bits is set, the type identifier is
four-bit, encoding one of the 12 types that need 48 bits of storage (1 to 12 in
the above list).

Finally, this four-/six-bit encoding allows using the fourth bit (marked with P
in the above diagram) to distinguish parsed types consistently between the
"short data" and "long data" encoding, and in case of short data the N bit is
used to distinguish between numeric and non-numeric types.

While this looks like a complicated logic, it's so only when asking "what type
does this token have". From experience, this kind of query is quite rare. "Is
this token of given type" is a much more common query, and due to how the bit
pattern is organized, it can be determined by a simple bitwise AND followed by
a comparison.

*/

using namespace Containers::Literals;

struct Json::State: Implementation::JsonData {
    /* If the string passed to parseString() was not global, this contains its
       copy, otherwise it's empty */
    Containers::String storage;
    /* Points either to the global string passed to parseString() or to
       the storage above. Used for line/column info in Json::parse*() error
       reporting. */
    Containers::StringView string;
    /* Used for file/line/column info in Json::parse*() error reporting */
    Containers::String filename;
    std::size_t lineOffset;
    std::size_t columnOffset;

    /* Needs to be in sync with the JsonData::tokens and ::tokenOffsetsSizes
       pointers that are used by the implementation in the header */
    Containers::Array<JsonTokenData> tokenStorage;
    Containers::Array<JsonTokenOffsetSize> tokenOffsetSizeStorage;

    Containers::Array<Containers::String> strings;
};

namespace {

/* To avoid having this duplicated in each and every static string */
constexpr const char* ErrorPrefix = "Utility::Json:";

constexpr const char* ExpectingString[]{
    "a value",
    "a value or ]",
    "\"",
    "\" or }",
    ":",
    ", or }",
    ", or ]",
    "document end"
};

enum class Expecting {
    Value,
    ValueOrArrayEnd,
    ObjectKey,
    ObjectKeyOrEnd,
    ObjectKeyColon,
    CommaOrObjectEnd,
    CommaOrArrayEnd,
    DocumentEnd
};

}

void Json::printFilePosition(Debug& out, const Containers::StringView string) const {
    std::size_t i = 0;
    /* Line offset is added always, but column offset only for the first line
       -- if a \n gets encountered, the lastLineBegin gets reset without the
       initial column offset */
    std::size_t line = 1 + _state->lineOffset;
    std::ptrdiff_t lastLineBegin = -std::ptrdiff_t(_state->columnOffset);
    for(; i != string.size(); ++i) {
        if(string[i] == '\n') {
            ++line;
            lastLineBegin = i + 1;
        }
    }

    /** @todo UTF-8 position instead */
    out << _state->filename << Debug::nospace << ":" << Debug::nospace << line << Debug::nospace << ":" << Debug::nospace << (string.size() - lastLineBegin) + 1;
}

void Json::printFilePosition(Debug& out, const JsonTokenData& token) const {
    const State& state = *_state;
    const std::size_t tokenIndex = &token - state.tokenStorage;
    printFilePosition(out, state.string.prefix(state.tokenOffsetSizeStorage[tokenIndex]._offset));
}

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset, const Containers::StringView string_) {
    Json json;
    json._state.emplace();

    /* Make a copy of the input string if not marked as global */
    if(string_.flags() & Containers::StringViewFlag::Global)
        json._state->string = string_;
    else
        json._state->string = json._state->storage = string_;

    /* Save also the filename for subsequent error reporting */
    json._state->filename = Containers::String::nullTerminatedGlobalView(filename ? filename : "<in>"_s);
    json._state->lineOffset = lineOffset;
    json._state->columnOffset = columnOffset;

    /* Remember surrounding object or array token index to update its size,
       child count and check matching braces when encountering } / ]. It's a
       64-bit type even on 32-bit because it's the 48-bit part of the
       mantissa. */
    constexpr std::uint64_t NoObjectOrArrayEndExpected = JsonToken::TypeLargeDataMask;
    std::uint64_t objectOrArrayTokenIndex = NoObjectOrArrayEndExpected;

    /* Remember what token to expect next; error printer utility. Can't be a
       free function in an anonymous namespace because it needs access to
       printFilePosition() in private State, and because it's used only here,
       it's a lambda, saving us some argument passing at least. */
    Expecting expecting = Expecting::Value;
    const auto printError = [&expecting, &json](const char offending, const Containers::StringView string) {
        Error err;
        err << ErrorPrefix << "expected" << ExpectingString[int(expecting)]
            << "but got" << Containers::StringView{&offending, 1} << "at";
        json.printFilePosition(err, string);
        return Containers::NullOpt;
    };

    /* Go through the file byte by byte */
    const std::size_t size = json._state->string.size();
    const char* const data = json._state->string.data();
    for(std::size_t i = 0; i != size; ++i) {
        const char c = data[i];

        switch(c) {
            /* Object / array begin */
            case '{':
            case '[': {
                if(expecting != Expecting::ValueOrArrayEnd &&
                   expecting != Expecting::Value)
                    return printError(c, json._state->string.prefix(i));

                /* Token holding the whole object / array */
                JsonTokenData token{NoInit};
                /* Child count gets filled in once } / ] is encountered. Until
                   then, abuse the _dataTypeNan field to store the previous
                   object / array index and remember this index for when we get
                   to } / ]. */
                token._dataTypeNan =
                    (c == '{' ? JsonToken::TypeLargeObject : JsonToken::TypeLargeArray)|objectOrArrayTokenIndex;
                objectOrArrayTokenIndex = json._state->tokenStorage.size();
                arrayAppend(json._state->tokenStorage, token);
                arrayAppend(json._state->tokenOffsetSizeStorage, InPlaceInit,
                    i,
                    /* Like with child count, size gets filled once } / ] is
                       encountered */
                    0u);

                /* If we're in an object, we're expecting an object key (or
                   end) next, otherwise a value (or end) */
                if(c == '{') expecting = Expecting::ObjectKeyOrEnd;
                else expecting = Expecting::ValueOrArrayEnd;
            } break;

            /* Object / array end */
            case '}':
            case ']': {
                if(expecting != Expecting::ObjectKeyOrEnd &&
                   expecting != Expecting::ValueOrArrayEnd &&
                   expecting != Expecting::CommaOrObjectEnd &&
                   expecting != Expecting::CommaOrArrayEnd)
                    return printError(c, json._state->string.prefix(i));

                /* Get the object / array token, check that the brace matches */
                JsonTokenData& token = json._state->tokenStorage[objectOrArrayTokenIndex];
                JsonTokenOffsetSize& tokenOffsetSize = json._state->tokenOffsetSizeStorage[objectOrArrayTokenIndex];
                const std::uint64_t tokenType = token._dataTypeNan & JsonToken::TypeLargeMask;
                CORRADE_INTERNAL_DEBUG_ASSERT(tokenType == JsonToken::TypeLargeObject || tokenType == JsonToken::TypeLargeArray);
                const bool isObject = tokenType == JsonToken::TypeLargeObject;
                if((c == '}') != isObject) {
                    Error err;
                    err << ErrorPrefix << "unexpected" << json._state->string.slice(i, i + 1) << "at";
                    json.printFilePosition(err, json._state->string.prefix(i));
                    err << "for an" << (c == ']' ? "object" : "array") << "starting at";
                    /* Printing the filename again, because it will make a
                       useful clickable link in terminal even though a bit
                       redundant */
                    json.printFilePosition(err, token);
                    return {};
                }

                /* The child count field was abused to store the previous
                   object / array index. Restore it and put the actual child
                   count there instead. */
                const std::size_t tokenChildCount = json._state->tokenStorage.size() - objectOrArrayTokenIndex - 1;
                objectOrArrayTokenIndex = token._dataTypeNan & JsonToken::TypeLargeDataMask;
                token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeLargeDataMask)|tokenChildCount;

                /* Update the token size to contain everything parsed until
                   now. The upper two bits, used for type, are non-0 only for
                   parsed numbers, so no need for any special handling here. */
                tokenOffsetSize._sizeType = i - tokenOffsetSize._offset + 1;

                /* Next should be a comma or an end depending on what the
                   new parent is */
                if(objectOrArrayTokenIndex == NoObjectOrArrayEndExpected)
                    expecting = Expecting::DocumentEnd;
                else {
                    const std::uint64_t objectOrArrayTokenType = json._state->tokenStorage[objectOrArrayTokenIndex]._dataTypeNan & JsonToken::TypeLargeMask;
                    CORRADE_INTERNAL_DEBUG_ASSERT(objectOrArrayTokenType == JsonToken::TypeLargeObject || objectOrArrayTokenType == JsonToken::TypeLargeArray);
                    expecting = objectOrArrayTokenType == JsonToken::TypeLargeObject ?
                        Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;
                }

            } break;

            /* String. Can be a value or an object key. Eat everything until
               the final unescaped quote so the next loop iteration is after
               the string. */
            case '"': {
                if(expecting != Expecting::Value &&
                   expecting != Expecting::ValueOrArrayEnd &&
                   expecting != Expecting::ObjectKey &&
                   expecting != Expecting::ObjectKeyOrEnd)
                    return printError(c, json._state->string.prefix(i));

                /* At the end of the loop, start points to the initial " and i
                   points to the final ". Remember if we encountered any
                   escape character -- if not, the string can be later accessed
                   directly. */
                const std::size_t start = i++;
                std::uint64_t escapedFlag = 0;
                for(; i != size; ++i) {
                    const char sc = data[i];

                    if(sc == '\"')
                        break;

                    if(sc == '\\') switch(data[++i]) {
                        case '"':
                        case '\\':
                        /* While not clearly said in the spec, this seems to be
                           allowed to be escaped to be able to put strings
                           containing "</script>" into <script> tags without
                           requiring a JavaScript-aware X/HTML parser. Hah. */
                        case '/':
                        case 'b':
                        case 'f':
                        case 'n':
                        case 'r':
                        case 't':
                        case 'u': /* Deliberately not validating Unicode here */
                            escapedFlag = JsonToken::TypeLargeStringIsEscaped;
                            break;
                        default: {
                            Error err;
                            err << ErrorPrefix << "unexpected string escape sequence" << json._state->string.slice(i - 1, i + 1) << "at";
                            json.printFilePosition(err, json._state->string.prefix(i - 1));
                            return {};
                        }
                    }
                }

                if(i == size) {
                    Error err;
                    err << ErrorPrefix << "file too short, unterminated string literal starting at";
                    json.printFilePosition(err, json._state->string.prefix(start));
                    return {};
                }

                /* Token holding the string, size includes the final " as
                   well. The i then gets incremented after the final " by the
                   outer loop. */
                JsonTokenData token{NoInit};
                /* The key flag, if any, gets added below */
                token._dataTypeNan = JsonToken::TypeLargeString|escapedFlag;

                /* Remember if this is an object key. In that case we're
                   expecting the colon next. */
                if(expecting == Expecting::ObjectKey ||
                   expecting == Expecting::ObjectKeyOrEnd)
                {
                    token._dataTypeNan |= JsonToken::TypeLargeStringIsKey;
                    expecting = Expecting::ObjectKeyColon;

                /* Otherwise it's a value and we're expecting a comma or an
                   end depending on what the parent is */
                } else if(expecting == Expecting::Value ||
                          expecting == Expecting::ValueOrArrayEnd)
                {
                    if(objectOrArrayTokenIndex == NoObjectOrArrayEndExpected)
                        expecting = Expecting::DocumentEnd;
                    else {
                        const std::uint64_t objectOrArrayTokenType = json._state->tokenStorage[objectOrArrayTokenIndex]._dataTypeNan & JsonToken::TypeLargeMask;
                        CORRADE_INTERNAL_DEBUG_ASSERT(objectOrArrayTokenType == JsonToken::TypeLargeObject || objectOrArrayTokenType == JsonToken::TypeLargeArray);
                        expecting = objectOrArrayTokenType == JsonToken::TypeLargeObject ?
                            Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;
                    }
                } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */

                arrayAppend(json._state->tokenStorage, token);
                arrayAppend(json._state->tokenOffsetSizeStorage, InPlaceInit,
                    start,
                    /* The upper two bits, used for type, are non-0 only for
                       parsed numbers, so no need for any special handling
                       here */
                    i - start + 1);

            } break;

            /* Number, null, true, false. Eat everything until
               the final unescaped quote so the next loop iteration is after
               the literal. */
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'n':
            case 't':
            case 'f': {
                if(expecting != Expecting::Value &&
                   expecting != Expecting::ValueOrArrayEnd)
                    return printError(c, json._state->string.prefix(i));

                /* At the end of the loop, start points to the initial letter
                   and i points to the end to a character after. */
                const std::size_t start = i;
                for(; i != size; ++i) {
                    const char lc = data[i];

                    /* Optimizing for the simplest check, deliberately not
                       doing any validation here */
                    if(lc == '\t' || lc == '\r' || lc == '\n' || lc == ' ' ||
                       lc == ',' || lc == ']' || lc == '}')
                        break;
                }

                /* Decrement i as it's incremented again by the outer loop */
                --i;

                JsonTokenData token{NoInit};
                if(c == 'n')
                    token._dataTypeNan = JsonToken::TypeSmallNull;
                else if(c == 't' || c == 'f')
                    token._dataTypeNan = JsonToken::TypeSmallBool;
                else
                    token._dataTypeNan = JsonToken::TypeSmallNumber;

                arrayAppend(json._state->tokenStorage, token);
                arrayAppend(json._state->tokenOffsetSizeStorage, InPlaceInit,
                    start,
                    /* The upper two bits, used for type, are non-0 only for
                       parsed numbers, so no need for any special handling
                       here */
                    i - start + 1);

                /* Expecting a comma or end next, depending on what the parent
                   is */
                if(objectOrArrayTokenIndex == NoObjectOrArrayEndExpected)
                    expecting = Expecting::DocumentEnd;
                else {
                    const std::uint64_t objectOrArrayTokenType = json._state->tokenStorage[objectOrArrayTokenIndex]._dataTypeNan & JsonToken::TypeLargeMask;
                    CORRADE_INTERNAL_DEBUG_ASSERT(objectOrArrayTokenType == JsonToken::TypeLargeObject || objectOrArrayTokenType == JsonToken::TypeLargeArray);
                    expecting = objectOrArrayTokenType == JsonToken::TypeLargeObject ?
                        Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;
                }

            } break;

            /* Colon after an object key */
            case ':': {
                if(expecting != Expecting::ObjectKeyColon)
                    return printError(c, json._state->string.prefix(i));

                /* Expecting a value next */
                expecting = Expecting::Value;
            } break;

            /* Comma after a value */
            case ',': {
                if(expecting != Expecting::CommaOrObjectEnd &&
                   expecting != Expecting::CommaOrArrayEnd)
                    return printError(c, json._state->string.prefix(i));

                /* If we're in an object, expecting a key next, otherwise a
                   value next */
                const std::uint64_t objectOrArrayTokenType = json._state->tokenStorage[objectOrArrayTokenIndex]._dataTypeNan & JsonToken::TypeLargeMask;
                CORRADE_INTERNAL_DEBUG_ASSERT(objectOrArrayTokenType == JsonToken::TypeLargeObject || objectOrArrayTokenType == JsonToken::TypeLargeArray);
                expecting = objectOrArrayTokenType == JsonToken::TypeLargeObject ?
                    Expecting::ObjectKey : Expecting::Value;
            } break;

            /* Whitespace, nothing to do */
            case '\t':
            case '\r':
            case '\n': /* JSON, Y U NO \v? */
            case ' ':
                break;

            default: {
                Error err;
                err << ErrorPrefix << "unexpected" << json._state->string.slice(i, i + 1) << "at";
                json.printFilePosition(err, json._state->string.prefix(i));
                return {};
            }
        }
    }

    if(expecting != Expecting::DocumentEnd &&
       /* Don't print this for a missing object/array end, the block below will
          do that with more context */
       expecting != Expecting::CommaOrArrayEnd &&
       expecting != Expecting::CommaOrObjectEnd)
    {
        Error err;
        err << ErrorPrefix << "file too short, expected"
            << ExpectingString[int(expecting)] << "at";
        json.printFilePosition(err, json._state->string);
        return {};
    }

    if(objectOrArrayTokenIndex != NoObjectOrArrayEndExpected) {
        Error err;
        err << ErrorPrefix << "file too short, expected closing";
        const JsonTokenData& token = json._state->tokenStorage[objectOrArrayTokenIndex];
        if(expecting == Expecting::CommaOrObjectEnd)
            err << "} for object";
        else if(expecting == Expecting::CommaOrArrayEnd)
            err << "] for array";
        else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        err << "starting at";
        json.printFilePosition(err, token);
        return {};
    }

    /* Not reserving memory for parsed string instances with the assumption
       that only a subset may ultimately get parsed, and they get appended into
       the array on-demand, without their index fixed */

    /* All good. Fill the token data + size members in the JsonData base and
       return. */
    json._state->tokens = json._state->tokenStorage.data();
    json._state->tokenOffsetsSizes = json._state->tokenOffsetSizeStorage.data();
    json._state->tokenCount = json._state->tokenStorage.size();
    /* GCC 4.8 needs a bit of help here */
    return Containers::optional(Utility::move(json));
}

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset, const Containers::StringView string, const Options options) {
    Containers::Optional<Json> out = tokenize(filename, lineOffset, columnOffset, string);
    if(!out)
        return {};

    if((options & Option::ParseLiterals) && !out->parseLiterals(out->root()))
        return {};

    /* If both ParseDoubles and ParseFloats is specified, doubles get a
       priority */
    if(options & Option::ParseDoubles) {
        if(!out->parseDoubles(out->root()))
            return {};
    } else if(options & Option::ParseFloats) {
        if(!out->parseFloats(out->root()))
            return {};
    }

    /* ParseStrings is a superset of ParseStringKeys, so don't call both */
    if(options >= Option::ParseStrings) {
        if(!out->parseStrings(out->root()))
            return {};
    } else if(options >= Option::ParseStringKeys) {
        if(!out->parseStringKeys(out->root()))
            return {};
    }

    return out;
}

Containers::Optional<Json> Json::fromString(const Containers::StringView string, const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset) {
    return tokenize(filename, lineOffset, columnOffset, string);
}

Containers::Optional<Json> Json::fromString(const Containers::StringView string) {
    return tokenize({}, 0, 0, string);
}

Containers::Optional<Json> Json::fromString(const Containers::StringView string, const Options options, const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset) {
    return tokenize(filename, lineOffset, columnOffset, string, options);
}

Containers::Optional<Json> Json::fromString(const Containers::StringView string, const Options options) {
    return tokenize({}, 0, 0, string, options);
}

Containers::Optional<Json> Json::fromFile(const Containers::StringView filename) {
    Containers::Optional<Containers::String> string = Path::readString(filename);
    if(!string) {
        Error{} << "Utility::Json::fromFile(): can't read" << filename;
        return {};
    }

    return tokenize(filename, 0, 0, *string);
}

Containers::Optional<Json> Json::fromFile(const Containers::StringView filename, const Options options) {
    Containers::Optional<Containers::String> string = Path::readString(filename);
    if(!string) {
        Error{} << "Utility::Json::fromFile(): can't read" << filename;
        return {};
    }

    return tokenize(filename, 0, 0, *string, options);
}

Json::Json() = default;

Json::Json(Json&&) noexcept = default;

Json::~Json() = default;

Json& Json::operator=(Json&&) noexcept = default;

JsonView Json::tokens() const {
    return JsonView{*_state, 0, _state->tokenStorage.size()};
}

JsonToken Json::root() const {
    /* An empty file is not a valid JSON, so there should always be at least
       one token */
    CORRADE_INTERNAL_ASSERT(!_state->tokenStorage.isEmpty());
    return JsonToken{*_state, 0};
}

void Json::parseObjectArrayInternal(JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if(token._dataTypeNan & JsonToken::TypeSmallLargeIsParsed)
        return;

    /* Just mark it as parsed and return. This is not done implicitly in order
       to force users to always explicitly call parse{Object,Array}*() before
       using the values. */
    token._dataTypeNan |= JsonToken::TypeSmallLargeIsParsed;
}

bool Json::parseNullInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed null, the token type could be just about
       anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeSmallMask) == (JsonToken::TypeSmallNull|JsonToken::TypeSmallLargeIsParsed))
        return true;

    const Containers::StringView string = tokenData(*_state, token);
    if(string != "null"_s) {
        Error err;
        err << errorPrefix << "invalid null literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success mark the token as parsed. The value is implicit, so nothing
       to save. */
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeSmallMask)|JsonToken::TypeSmallNull|JsonToken::TypeSmallLargeIsParsed;
    return true;
}

bool Json::parseBoolInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed bool, the token type could be just about
       anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeSmallMask) == (JsonToken::TypeSmallBool|JsonToken::TypeSmallLargeIsParsed))
        return true;

    const Containers::StringView string = tokenData(*_state, token);
    if(string == "true"_s)
        token._parsedBool = true;
    else if(string == "false"_s)
        token._parsedBool = false;
    else {
        Error err;
        err << errorPrefix << "invalid bool literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success save the parsed token type. It aliases the _parsedBool field
       that was set above so be sure to preserve existing contents. */
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeSmallMask)|JsonToken::TypeSmallBool|JsonToken::TypeSmallLargeIsParsed;
    return true;
}

bool Json::parseDoubleInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed double, the token type could be just about
       anything if the condition is false. */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    if((tokenOffsetSize._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeDouble) {
        /* Just a sanity check, as JSON doesn't support NaNs, the NaN bits
           shouldn't match. If they would, type detection elsewhere would fail
           miserably. */
        CORRADE_INTERNAL_DEBUG_ASSERT((token._dataTypeNan & JsonToken::Nan) != JsonToken::Nan);
        return true;
    }

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedDouble directly to avoid a failure corrupting
       the existing value */
    const double out = std::strtod(buffer, &end);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size
        /* Explicitly disallowing NaNs to not clash with the NaN bit pattern
           stuffing. NAN and INF literals in a JSON are non-conforming behavior
           (see all XFAILs in tests) */
        /** @todo drop this once we have a conversion routine with proper
            control */
        || std::isinf(out) || std::isnan(out)
    ) {
        Error err;
        err << errorPrefix << "invalid floating-point literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success save the parsed value and its type, again verify that we
       didn't end up with NaN bits set. */
    token._parsedDouble = out;
    CORRADE_INTERNAL_DEBUG_ASSERT((token._dataTypeNan & JsonToken::Nan) != JsonToken::Nan);
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeDouble;
    return true;
}

bool Json::parseFloatInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed float, the token type could be just about
       anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallFloat)
        return true;

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedFloat directly to avoid a failure corrupting
       the existing value */
    const float out = std::strtof(buffer, &end);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid floating-point literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success save the parsed value and its type. The type aliases the
       _parsedFloat field that was set above so be sure to preserve existing
       contents. */
    token._parsedFloat = out;
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeSmallMask)|JsonToken::TypeSmallFloat;
    /* Reset the 64-bit type identifier in the size field, in case it was
       parsed as a double before, for example */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeOther;
    return true;
}

bool Json::parseUnsignedIntInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed unsigned int, the token type could be just
       about anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallUnsignedInt)
        return true;

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not using strtoul() here as on Windows it's 32-bit and we wouldn't be
       able to detect overflows. Also not saving to token._parsedUnsignedInt
       directly to avoid a failure corrupting the existing value. */
    /** @todo replace with something that can report errors in a non-insane
        way */
    const std::uint64_t outLong = std::strtoull(buffer, &end, 10);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid unsigned integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }
    if(outLong > ~std::uint32_t{}) {
        Error err;
        err << errorPrefix << "too large integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success convert the value to 32 bits and save the parsed token type.
       The type aliases the _parsedUnsignedInt field so be sure to preserve
       its contents. */
    token._parsedUnsignedInt = outLong;
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeSmallMask)|JsonToken::TypeSmallUnsignedInt;
    /* Reset the 64-bit type identifier in the size field, in case it was
       parsed as a double before, for example */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeOther;
    return true;
}

bool Json::parseIntInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed int, the token type could be just about
       anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallInt)
        return true;

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not using strtol() here as on Windows it's 32-bit and we wouldn't be
       able to detect overflows. Also not saving to token._parsedInt directly
       to avoid a failure corrupting the existing value. */
    /** @todo replace with something that can report errors in a non-insane
        way */
    const std::int64_t outLong = std::strtoll(buffer, &end, 10);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }
    if(outLong < INT_MIN || outLong > INT_MAX) {
        Error err;
        err << errorPrefix << "too small or large integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success convert the value to 32 bits and save the parsed token type.
       The type aliases the _parsedInt field so be sure to preserve
       its contents. */
    token._parsedInt = outLong;
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeSmallMask)|JsonToken::TypeSmallInt;
    /* Reset the 64-bit type identifier in the size field, in case it was
       parsed as a double before, for example */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeOther;
    return true;
}

bool Json::parseUnsignedLongInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed unsigned long, the token type could be just
       about anything if the condition is false. */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    if((tokenOffsetSize._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeUnsignedLong) {
        /* Just a sanity check, as only 52-bit unsigned types are supported,
           the NaN bits and the sign bit should be all zero. If they wouldn't,
           type detection elsewhere would fail miserably. */
        CORRADE_INTERNAL_DEBUG_ASSERT((token._dataTypeNan & JsonToken::NanMask) == 0);
        return true;
    }

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedUnsignedLong directly to avoid a failure
       corrupting the existing value */
    const std::uint64_t out = std::strtoull(buffer, &end, 10);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid unsigned integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }
    if(out >= 1ull << 52) {
        Error err;
        err << errorPrefix << "too large integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success save the parsed value and its type, again verify that we
       didn't end up with any NaN bits set. */
    token._parsedUnsignedLong = out;
    CORRADE_INTERNAL_DEBUG_ASSERT((token._dataTypeNan & JsonToken::NanMask) == 0);
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeUnsignedLong;
    return true;
}

bool Json::parseLongInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed long, the token type could be just about
       anything if the condition is false. */
    State& state = *_state;
    JsonTokenOffsetSize& tokenOffsetSize = state.tokenOffsetSizeStorage[&token - state.tokens];
    if((tokenOffsetSize._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeLong) {
        /* Just a sanity check, as only 53-bit signed types are supported, the
           NaN bits and the sign bit should be either all zero or all one. If
           they wouldn't, type detection elsewhere would fail miserably. */
        CORRADE_INTERNAL_DEBUG_ASSERT(
            (token._dataTypeNan & JsonToken::NanMask) == 0 ||
            (token._dataTypeNan & JsonToken::NanMask) == JsonToken::NanMask);
        return true;
    }

    const Containers::StringView string = tokenData(*_state, token);
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedLong directly to avoid a failure corrupting
       the existing value */
    const std::int64_t out = std::strtoll(buffer, &end, 10);
    /* The token being empty (such as with external data) is also an error */
    if(!size || std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }
    if(out < -(1ll << 52) || out >= (1ll << 52)) {
        Error err;
        err << errorPrefix << "too small or large integer literal" << string << "at";
        printFilePosition(err, token);
        return false;
    }

    /* On success save the parsed value and its type, again verify that we
       didn't end up with either none or all NaN and sign bits set. */
    token._parsedLong = out;
    CORRADE_INTERNAL_DEBUG_ASSERT(
        (token._dataTypeNan & JsonToken::NanMask) == 0 ||
        (token._dataTypeNan & JsonToken::NanMask) == JsonToken::NanMask);
    tokenOffsetSize._sizeType = (tokenOffsetSize._sizeType & ~JsonToken::TypeTokenSizeMask)|JsonToken::TypeTokenSizeLong;
    return true;
}

bool Json::parseStringInternal(const char* const errorPrefix, JsonTokenData& token) {
    /* If the token is an already parsed string (or a key, or an escaped
       string, or an escaped key string), nothing to do. Assumes the caller
       checked for correct token type, otherwise, while the condition is true
       only if it's indeed a parsed string, the token type could be just about
       anything if the condition is false. */
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeLargeStringIsEscaped|JsonToken::TypeLargeStringIsKey)) == (JsonToken::TypeLargeString|JsonToken::TypeSmallLargeIsParsed))
        return true;

    /* If a token has no escapes, mark it as parsed and return. This is not
       done implicitly in order to force users to always explicitly call
       parseString*() before using the string values. */
    if(!(token._dataTypeNan & JsonToken::TypeLargeStringIsEscaped)) {
        token._dataTypeNan |= JsonToken::TypeSmallLargeIsParsed;
        return true;
    }

    /* Otherwise parse the escapes */
    const Containers::StringView string = tokenData(*_state, token);
    Containers::String& destination = arrayAppend(_state->strings, InPlaceInit, NoInit, string.size());

    /* Ignore the quotes at the begin/end */
    const char* in = string.data() + 1;
    const char* const end = string.end() - 1;
    char* const outBegin = destination.data();
    char* out = outBegin;
    for(; in != end; ++in, ++out) {
        const char c = *in;

        if(c == '\\') switch(*++in) {
            case '"':
                *out = '"';
                break;
            case '\\':
                *out = '\\';
                break;
            case '/':
                *out = '/';  /* JSON, why, you're weird */
                break;
            case 'b':
                *out = '\b';
                break;
            case 'f':
                *out = '\f';
                break;
            case 'n':
                *out = '\n';
                break;
            case 't':
                *out = '\t';
                break;
            case 'r':
                *out = '\r';
                break;
            case 'u': {
                /* Parse the hexadecimal Unicode codepoint. The input pointer
                   is incremented in the outer loop, thus this increments it
                   only four times out of five. */
                /** @todo handle also surrogate pairs, add a helper to the
                    Unicode lib first https://en.wikipedia.org/wiki/JSON#Character_encoding */
                const char* const unicodeBegin = in - 1;
                const char* unicodeEnd = in + 5;
                char32_t character = 0;
                if(unicodeEnd > end) {
                    /* Handled below together with values out of range */
                    unicodeEnd = end;
                    character = ~char32_t{};
                } else for(; in != unicodeEnd - 1; ++in) {
                    character <<= 4;
                    const char uc = in[1];
                    if(uc >= '0' && uc <= '9') character |= uc - '0';
                    else if(uc >= 'A' && uc <= 'F') character |= 10 + uc - 'A';
                    else if(uc >= 'a' && uc <= 'f') character |= 10 + uc - 'a';
                    else {
                        /* Handled below together with values out of range */
                        character = ~char32_t{};
                        break;
                    }
                }

                /* Convert the unicode codepoint to UTF-8. At this point, since
                   the encoding it 16 bit, it won't happen that Unicode::utf8()
                   returns 0, but it's future-proofed for UTF-16 surrogate
                   pairs when they get implemented. */
                char utf8[4];
                std::size_t utf8Size;
                if(character == ~char32_t{} || !(utf8Size = Unicode::utf8(character, utf8))) {
                    Error err;
                    err << errorPrefix << "invalid unicode escape sequence" << Containers::StringView{unicodeBegin, std::size_t(unicodeEnd - unicodeBegin)} << "at";
                    printFilePosition(err, _state->string.prefix(unicodeBegin));
                    return false;
                }

                /* Copy it to the output. The output pointer is incremented in
                   the outer loop, thus this increments it one time less. */
                for(std::size_t i = 0; i != utf8Size; ++i)
                    out[i] = utf8[i];
                out += utf8Size - 1;
                break;
            }
            default: CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        } else *out = c;
    }

    /* "Resize" the output to what got actually written. If it's a SSO, don't
       release but instead make a new (again SSO) instance. */
    /** @todo use a growable string once it exists? */
    *out = '\0';
    if(destination.isSmall())
        destination = Containers::String{destination.data(), std::size_t(out - outBegin)};
    else
        destination = Containers::String{destination.release(), std::size_t(out - outBegin), nullptr};

    /* On success save index of the parsed string and mark it as parsed.
       Make sure to keep the remaining bits (such as ones marking whether it's
       a key) the same. */
    token._dataTypeNan = (token._dataTypeNan & ~JsonToken::TypeLargeDataMask)|(_state->strings.size() - 1)|JsonToken::TypeSmallLargeIsParsed;
    return true;
}

bool Json::parseLiterals(const JsonToken token) {
    CORRADE_ASSERT(token._json == &*_state,
        "Utility::Json::parseLiterals(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Pick only tokens that are unparsed objects, arrays, nulls and bools.
           I.e., if the expression has TypeSmallLargeIsParsed set, it won't
           enter any of the branches. */
        JsonTokenData& nestedToken = _state->tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeLargeMask) == JsonToken::TypeLargeObject ||
           (nestedToken._dataTypeNan & JsonToken::TypeLargeMask) == JsonToken::TypeLargeArray) {
            parseObjectArrayInternal(nestedToken);

        } else if((nestedToken._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallNull) {
            if(!parseNullInternal("Utility::Json::parseLiterals():", nestedToken))
                return false;

        } else if((nestedToken._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallBool) {
            if(!parseBoolInternal("Utility::Json::parseLiterals():", nestedToken))
                return false;
        }
    }

    return true;
}

bool Json::parseDoubles(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseDoubles(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as doubles (which is the simpler
           check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((state.tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeDouble ||
           !nestedToken.isNumber())
            continue;

        if(!parseDoubleInternal("Utility::Json::parseDoubles():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseFloats(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseFloats(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as floats (which is the simpler
           check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallFloat ||
           !nestedToken.isNumber())
            continue;

        if(!parseFloatInternal("Utility::Json::parseFloats():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseUnsignedInts(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseUnsignedInts(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as unsigned ints (which is the
           simpler check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallUnsignedInt ||
           !nestedToken.isNumber())
            continue;

        if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedInts():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseInts(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseInts(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as ints (which is the simpler
           check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallInt ||
           !nestedToken.isNumber())
            continue;

        if(!parseIntInternal("Utility::Json::parseInts():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseUnsignedLongs(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseUnsignedLongs(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as unsigned longs (which is the
           simpler check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((state.tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeUnsignedLong ||
           !nestedToken.isNumber())
            continue;

        if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLongs():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseLongs(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseLongs(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed as longs (which is the simpler
           check, so do it first) and non-number tokens */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((state.tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeLong ||
           !nestedToken.isNumber())
            continue;

        if(!parseLongInternal("Utility::Json::parseLongs():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseSizes(const JsonToken token) {
    #ifndef CORRADE_TARGET_32BIT
    return parseUnsignedLongs(token);
    #else
    return parseUnsignedInts(token);
    #endif
}

bool Json::parseStringKeys(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseStringKeys(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-string tokens, string tokens that are not keys or string
           (key) tokens that are already parsed. I.e., if the expression has
           TypeSmallLargeIsParsed set, it'll `continue` as well. */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeLargeStringIsEscaped) != JsonToken::TypeLargeStringKey)
            continue;

        if(!parseStringInternal("Utility::Json::parseStringKeys():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseStrings(const JsonToken token) {
    State& state = *_state;
    CORRADE_ASSERT(token._json == &state,
        "Utility::Json::parseStrings(): token not owned by the instance", {});

    for(std::size_t i = token._token, max = token._token + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-string tokens or string tokens that are already parsed.
           I.e., if the expression has TypeSmallLargeIsParsed set, it'll
           `continue` as well. */
        JsonTokenData& nestedToken = state.tokenStorage[i];
        if((nestedToken._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeLargeStringIsEscaped|JsonToken::TypeLargeStringIsKey)) != JsonToken::TypeLargeString)
            continue;

        if(!parseStringInternal("Utility::Json::parseStrings():", nestedToken))
            return false;
    }

    return true;
}

JsonIterator JsonObjectView::find(const Containers::StringView key) const {
    /* If the enclosing array/object is not empty, _begin is the first child of
       it and so parent() is an O(1) operation and its never null, equivalent
       to `_begin - 1`. If the enclosing array/object is empty, _begin points
       to a token after it, which makes parent() either an O(n) operation
       (which wouldn't return a correct value) or null (in case it's the root
       token). As finding something in an empty view won't succeed anyway,
       catch that early and access the parent only if the view is
       non-empty. */
    if(_begin == _end)
        return {};
    /* Assuming non-empty view, the token right before the _begin is the
       parent */
    return JsonToken{*_json, _begin - 1}.find(key);
}

JsonIterator JsonArrayView::find(const std::size_t index) const {
    if(_begin == _end) /* See the comment above */
        return {};
    /* Assuming non-empty view, the token right before the _begin is the
       parent */
    return JsonToken{*_json, _begin - 1}.find(index);
}

JsonToken JsonObjectView::operator[](const Containers::StringView key) const {
    CORRADE_ASSERT(_begin != _end, /* See the comment above */
        "Utility::JsonObjectView::operator[](): view is empty", (JsonToken{*_json, _begin}));
    /* Assuming non-empty view, the token right before the _begin is the
       parent */
    return JsonToken{*_json, _begin - 1}[key];
}

JsonToken JsonArrayView::operator[](const std::size_t index) const {
    CORRADE_ASSERT(_begin != _end, /* See the comment above */
        "Utility::JsonArrayView::operator[](): view is empty", (JsonToken{*_json, _begin}));
    /* Assuming non-empty view, the token right before the _begin is the
       parent */
    return JsonToken{*_json, _begin - 1}[index];
}

Containers::Optional<JsonObjectView> Json::parseObject(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseObject(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeObject) {
        Error err;
        err << "Utility::Json::parseObject(): expected an object, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);

    const std::size_t childCount = token.childCount();
    for(std::size_t i = token_._token + 1, end = i + childCount; i != end; i = i + 1 + state.tokenStorage[i].childCount()) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!parseStringInternal("Utility::Json::parseObject():", nested))
            return {};
    }

    return JsonObjectView{state, token_._token + 1, childCount};
}

Containers::Optional<JsonArrayView> Json::parseArray(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    return JsonArrayView{state, token_._token + 1, token.childCount()};
}

Containers::Optional<std::nullptr_t> Json::parseNull(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseNull(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeSmallMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeSmallNull) {
        Error err;
        err << "Utility::Json::parseNull(): expected a null, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseNullInternal("Utility::Json::parseNull():", token))
        return {};
    return nullptr;
}

Containers::Optional<bool> Json::parseBool(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseBool(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeSmallMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeSmallBool) {
        Error err;
        err << "Utility::Json::parseBool(): expected a bool, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseBoolInternal("Utility::Json::parseBool():", token))
        return {};
    return token._parsedBool;
}

Containers::Optional<double> Json::parseDouble(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseDouble(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseDouble(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseDoubleInternal("Utility::Json::parseDouble():", token))
        return {};
    return token._parsedDouble;
}

Containers::Optional<float> Json::parseFloat(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseFloat(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseFloat(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseFloatInternal("Utility::Json::parseFloat():", token))
        return {};
    return token._parsedFloat;
}

Containers::Optional<std::uint32_t> Json::parseUnsignedInt(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseUnsignedInt(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseUnsignedInt(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedInt():", token))
        return {};
    return token._parsedUnsignedInt;
}

Containers::Optional<std::int32_t> Json::parseInt(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseInt(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseInt(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseIntInternal("Utility::Json::parseInt():", token))
        return {};
    return token._parsedInt;
}

Containers::Optional<std::uint64_t> Json::parseUnsignedLong(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseUnsignedLong(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseUnsignedLong(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLong():", token))
        return {};
    return token._parsedUnsignedLong;
}

Containers::Optional<std::int64_t> Json::parseLong(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseLong(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseLong(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseLongInternal("Utility::Json::parseLong():", token))
        return {};
    return token._parsedLong;
}

Containers::Optional<std::size_t> Json::parseSize(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseSize(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if(!token.isNumber()) {
        Error err;
        err << "Utility::Json::parseSize(): expected a number, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!
        #ifndef CORRADE_TARGET_32BIT
        parseUnsignedLongInternal
        #else
        parseUnsignedIntInternal
        #endif
        ("Utility::Json::parseSize():", token)
    )
        return {};

    #ifndef CORRADE_TARGET_32BIT
    return token._parsedUnsignedLong;
    #else
    return token._parsedUnsignedInt;
    #endif
}

Containers::Optional<Containers::StringView> Json::parseString(const JsonToken token_) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseString(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeLargeStringIsKey|JsonToken::TypeLargeStringIsEscaped| JsonToken::TypeSmallLargeIsParsed)) != JsonToken::TypeLargeString) {
        Error err;
        err << "Utility::Json::parseString(): expected a string, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }
    if(!parseStringInternal("Utility::Json::parseString():", token))
        return {};

    /* If the string is not escaped, reference it directly */
    if(!(token._dataTypeNan & JsonToken::TypeLargeStringIsEscaped))
        return state.string.sliceSize(
            state.tokenOffsetsSizes[token_._token]._offset + 1,
            (state.tokenOffsetsSizes[token_._token]._sizeType & ~JsonToken::TypeTokenSizeMask) - 2);

    /* Otherwise take the cached version. Compared to _json->tokens the strings
       pointer isn't stored in the base JsonData array -- it may get changed on
       every reallocation so it'd have to get updated each time parseString()
       etc gets called, but we don't need to access that in the header so we
       can also just peek directly into the derived struct. */
    return Containers::StringView{state.strings[token._dataTypeNan & JsonToken::TypeLargeDataMask]};
}

Containers::Optional<Containers::StridedBitArrayView1D> Json::parseBitArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseBitArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseBitArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if((nested._dataTypeNan & JsonToken::TypeSmallMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeSmallBool) {
            Error err;
            err << "Utility::Json::parseBitArray(): expected a bool, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseBoolInternal("Utility::Json::parseBitArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseBitArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedBool).sliceBit(0);
}

#ifdef CORRADE_BUILD_DEPRECATED
Containers::Optional<Containers::StridedArrayView1D<const bool>> Json::parseBoolArray(const JsonToken token_, const std::size_t expectedSize) {
    /* It's easier to just create the view anew than try to inflate it from the
       returned one */
    if(const Containers::Optional<Containers::StridedBitArrayView1D> parsed = parseBitArray(token_, expectedSize))
        return Containers::StridedArrayView1D<const JsonTokenData>(_state->tokenStorage.data() + token_._token + 1, parsed->size()).slice(&JsonTokenData::_parsedBool);

    return {};
}
#endif

Containers::Optional<Containers::StridedArrayView1D<const double>> Json::parseDoubleArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseDoubleArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseDoubleArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseDoubleArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseDoubleInternal("Utility::Json::parseDoubleArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseDoubleArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedDouble);
}

Containers::Optional<Containers::StridedArrayView1D<const float>> Json::parseFloatArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseFloatArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseFloatArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseFloatArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseFloatInternal("Utility::Json::parseFloatArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseFloatArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedFloat);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> Json::parseUnsignedIntArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseUnsignedIntArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseUnsignedIntArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseUnsignedIntArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedIntArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseUnsignedIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedUnsignedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int32_t>> Json::parseIntArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseIntArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseIntArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseIntArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseIntInternal("Utility::Json::parseIntArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> Json::parseUnsignedLongArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseUnsignedLongArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseUnsignedLongArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseUnsignedLongArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLongArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseUnsignedLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedUnsignedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int64_t>> Json::parseLongArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseLongArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseLongArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if(!nested.isNumber()) {
            Error err;
            err << "Utility::Json::parseLongArray(): expected a number, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseLongInternal("Utility::Json::parseLongArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StridedArrayView1D<const JsonTokenData>{&token + 1, size}.slice(&JsonTokenData::_parsedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::size_t>> Json::parseSizeArray(const JsonToken token, const std::size_t expectedSize) {
    #ifndef CORRADE_TARGET_32BIT
    const Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> out = parseUnsignedLongArray(token, expectedSize);
    #else
    const Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> out = parseUnsignedIntArray(token, expectedSize);
    #endif
    if(!out)
        return {};

    return Containers::arrayCast<const std::size_t>(*out);
}

Containers::Optional<Containers::StringIterable> Json::parseStringArray(const JsonToken token_, const std::size_t expectedSize) {
    State& state = *_state;
    CORRADE_ASSERT(token_._json == &state,
        "Utility::Json::parseStringArray(): token not owned by the instance", {});

    JsonTokenData& token = state.tokenStorage[token_._token];
    if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeArray) {
        Error err;
        err << "Utility::Json::parseStringArray(): expected an array, got" << JsonToken{state, token_._token}.type() << "at";
        printFilePosition(err, token);
        return {};
    }

    parseObjectArrayInternal(token);
    const std::size_t size = token._dataTypeNan & JsonToken::TypeLargeDataMask;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the type() check fails. */
    for(std::size_t i = token_._token + 1, end = i + size; i != end; ++i) {
        JsonTokenData& nested = state.tokenStorage[i];
        if((nested._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeLargeStringIsKey|JsonToken::TypeLargeStringIsEscaped| JsonToken::TypeSmallLargeIsParsed)) != JsonToken::TypeLargeString) {
            Error err;
            err << "Utility::Json::parseStringArray(): expected a string, got" << JsonToken{state, i}.type() << "at";
            printFilePosition(err, nested);
            return {};
        }

        if(!parseStringInternal("Utility::Json::parseStringArray():", nested))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseStringArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, token);
        return {};
    }

    return Containers::StringIterable{&token + 1, this, size, sizeof(JsonTokenData), [](const void* data, const void* context, std::ptrdiff_t, std::size_t) {
        return JsonToken{*static_cast<const Json*>(context), *static_cast<const JsonTokenData*>(data)}.asString();
    }};
}

JsonToken::JsonToken(const Json& json, const JsonTokenData& token) noexcept: _json{json._state.get()}, _token{std::size_t(&token - _json->tokens)} {
    CORRADE_ASSERT(_token < _json->tokenCount,
        "Utility::JsonToken: token not owned by given Json instance", );
}

inline Containers::StringView Json::tokenData(const Implementation::JsonData& json, const JsonTokenData& token) {
    const std::size_t index = &token - json.tokens;
    return static_cast<const State&>(json).string.sliceSize(json.tokenOffsetsSizes[index]._offset, json.tokenOffsetsSizes[index]._sizeType & ~JsonToken::TypeTokenSizeMask);
}

Containers::StringView JsonToken::data() const {
    return Json::tokenData(*_json, _json->tokens[_token]);
}

/* This is here because it's significantly less complicated than checking
   token.type() == JsonToken::Type::Number */
inline bool JsonTokenData::isNumber() const {
    /* If +NaN is set, the type is stored in the token */
    if((_dataTypeNan & JsonToken::NanMask) == JsonToken::Nan)
        /* The token is a number if it's a small type with the number bit set */
        return !(_dataTypeNan & JsonToken::TypeIsLarge) && (_dataTypeNan & JsonToken::TypeSmallIsNumber);

    /* Otherwise, if +NaN is not set, it's a parsed 64-bit number with the
       type stored in the token size. Unfortunately from here it's impossible
       to access the separate tokenOffsetsSizes array so can't do even a sanity
       debug-only check. */
    return true;
}

JsonToken::Type JsonToken::type() const {
    const JsonTokenData& data = _json->tokens[_token];

    /* If +NaN is set, the type is stored in the token */
    if((data._dataTypeNan & NanMask) == Nan) {
        /* The bits in token size should match this */
        CORRADE_INTERNAL_DEBUG_ASSERT((_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) == TypeTokenSizeOther);

        /* If it's a large type, the type is stored in the next three bits */
        if(data._dataTypeNan & TypeIsLarge) {
            switch(data._dataTypeNan & TypeLargeMask & ~TypeSmallLargeIsParsed) {
                case TypeLargeObject:
                    return Type::Object;
                case TypeLargeArray:
                    return Type::Array;
                case TypeLargeString:
                case TypeLargeStringEscaped:
                case TypeLargeStringKey:
                case TypeLargeStringKeyEscaped:
                    return Type::String;
            }

            CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        }

        /* Otherwise in the next five bits */
        switch(data._dataTypeNan & TypeSmallMask & ~TypeSmallLargeIsParsed) {
            case TypeSmallNull:
                return Type::Null;
            case TypeSmallBool:
                return Type::Bool;
            case TypeSmallNumber & ~TypeSmallLargeIsParsed:
            case TypeSmallFloat & ~TypeSmallLargeIsParsed:
            case TypeSmallUnsignedInt & ~TypeSmallLargeIsParsed:
            case TypeSmallInt & ~TypeSmallLargeIsParsed:
                return Type::Number;
        }

        CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
    }

    /* Otherwise (-NaN, 0, or anything else in the exponent bits) it can only
       be a (64-bit) number */
    CORRADE_INTERNAL_DEBUG_ASSERT((_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) != TypeTokenSizeOther);
    return Type::Number;
}

Containers::Optional<JsonToken::Type> JsonToken::commonArrayType() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask & ~TypeSmallLargeIsParsed) == TypeLargeArray,
        "Utility::JsonToken::commonArrayType(): token is a" << type() << Debug::nospace << ", expected an array", {});

    const std::size_t childCount = data._dataTypeNan & TypeLargeDataMask;
    if(!childCount)
        return {};

    const Type type = JsonToken{*_json, _token + 1}.type();
    /* Iterate from the second array token (i.e., after `_token + 1` and its
       children and compare to the first token type */
    for(std::size_t i = (_token + 1) + 1 + _json->tokens[_token + 1].childCount(), end = _token + 1 + childCount; i != end; i += 1 + _json->tokens[i].childCount())
        /* Unlike other checks there isn't really a way to avoid calling the
           complex type() here. Ultimately the commonArrayType() is meant
           mainly for diagnostic, in practice the code will likely want to just
           know "is this array of such type?" rather than "what type is this
           array of so I can branch on it?". */
        if(JsonToken{*_json, i}.type() != type)
            return {};

    return type;
}

bool JsonToken::isParsed() const {
    const JsonTokenData& data = _json->tokens[_token];

    /* If +NaN is set, the information about whether it was parsed is stored in
       the token */
    if((data._dataTypeNan & NanMask) == Nan) {
        /* The bits in token size should match this */
        CORRADE_INTERNAL_DEBUG_ASSERT((_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) == TypeTokenSizeOther);

        return data._dataTypeNan & TypeSmallLargeIsParsed;
    }

    /* Otherwise (-NaN, 0, or anything else in the exponent bits) it can only
       be an (64-bit, already parsed) number */
    CORRADE_INTERNAL_DEBUG_ASSERT((_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) != TypeTokenSizeOther);
    return true;
}

JsonToken::ParsedType JsonToken::parsedType() const {
    const JsonTokenData& data = _json->tokens[_token];

    /* If +NaN is set, the parsed state and type is stored in the token */
    if((data._dataTypeNan & NanMask) == Nan) {
        /* The bits in token size should match this */
        CORRADE_INTERNAL_DEBUG_ASSERT((_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) == TypeTokenSizeOther);

        if(data._dataTypeNan & TypeSmallLargeIsParsed) {
            /* Large types are all non-numeric */
            if(data._dataTypeNan & TypeIsLarge)
                return ParsedType::Other;

            switch(data._dataTypeNan & TypeSmallMask) {
                case TypeSmallNull|TypeSmallLargeIsParsed:
                case TypeSmallBool|TypeSmallLargeIsParsed:
                    return ParsedType::Other;
                case TypeSmallFloat:
                    return ParsedType::Float;
                case TypeSmallUnsignedInt:
                    return ParsedType::UnsignedInt;
                case TypeSmallInt:
                    return ParsedType::Int;
            }

            CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        }

        return ParsedType::None;
    }

    /* Otherwise it's in the upper bits of the token size */
    switch(_json->tokenOffsetsSizes[_token]._sizeType & TypeTokenSizeMask) {
        case TypeTokenSizeDouble:
            return ParsedType::Double;
        case TypeTokenSizeUnsignedLong:
            return ParsedType::UnsignedLong;
        case TypeTokenSizeLong:
            return ParsedType::Long;
        /* It should definitely not be JsonToken::TypeTokenSizeOther, as that
           should have entered the branch above. See the UNREACHABLE below. */
    }

    CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

Containers::Optional<JsonToken::ParsedType> JsonToken::commonParsedArrayType() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask & ~TypeSmallLargeIsParsed) == TypeLargeArray,
        "Utility::JsonToken::commonParsedArrayType(): token is a" << type() << Debug::nospace << ", expected an array", {});

    const std::size_t childCount = data._dataTypeNan & TypeLargeDataMask;
    if(!childCount)
        return {};

    /* If the first token isn't parsed, bail. It doesn't make sense to return
       ParsedType::None as the common parsed type, since that says nothing
       about the contents -- it could be a heterogeneous mixture of whatever
       and still have a None as a common parsed type. */
    const ParsedType type = JsonToken{*_json, _token + 1}.parsedType();
    if(type == ParsedType::None)
        return {};

    /* Iterate from the second array token (i.e., after `_token + 1` and its
       children and compare to the first token type */
    for(std::size_t i = (_token + 1) + 1 + _json->tokens[_token + 1].childCount(), end = _token + 1 + childCount; i != end; i += 1 + _json->tokens[i].childCount())
        /* Unlike other checks there isn't really a way to avoid calling the
           complex parsedType() here. Ultimately the commonParsedArrayType() is
           meant mainly for diagnostic, in practice the code will likely want
           to just know "is this array of such type?" rather than "what type is
           this array of so I can branch on it?". */
        if(JsonToken{*_json, i}.parsedType() != type)
            return {};

    return type;
}

std::size_t JsonTokenData::childCount() const {
    /* The only types that have children are arrays, objects and strings, which
       are all large types. Also, arrays, objects and strings are the only
       large types, so just check that the token is +NaN and that the stored
       type is large. */
    if((_dataTypeNan & JsonToken::NanMask) == JsonToken::Nan && (_dataTypeNan & JsonToken::TypeIsLarge)) {
        /* Objects and arrays store child count directly */
        const std::uint64_t type = _dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed;
        if(type == JsonToken::TypeLargeObject ||
           type == JsonToken::TypeLargeArray)
            return _dataTypeNan & JsonToken::TypeLargeDataMask;

        /* String keys have implicitly grandchild count + 1, where the
           grandchildren can be either objects and arays or value types with no
           children. Keys can't have keys as children, so this doesn't
           recurse. */
        if((type & JsonToken::TypeLargeIsString) &&
           (type & JsonToken::TypeLargeStringIsKey))
        {
            const JsonTokenData child = *(this + 1);
            const std::uint64_t childType = child._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed;
            return (childType == JsonToken::TypeLargeObject ||
                    childType == JsonToken::TypeLargeArray ?
                        (child._dataTypeNan & JsonToken::TypeLargeDataMask) : 0) + 1;
        }

        /* Otherwise it can only be a non-key string, which has no children */
        CORRADE_INTERNAL_DEBUG_ASSERT(type & JsonToken::TypeLargeIsString);
        return 0;
    }

    /* Otherwise it's a numeric or literal type, which has no children */
    return 0;
}

std::size_t JsonToken::childCount() const {
    return _json->tokens[_token].childCount();
}

JsonView JsonToken::children() const {
    return JsonView{*_json, _token + 1, childCount()};
}

JsonIterator JsonToken::parent() const {
    /* Traverse backwards until a token that spans over this one is found, or
       until we reach begin (and so the value underflows to ~std::size_t{}) */
    std::size_t prev = _token - 1;
    while(prev != ~std::size_t{} && prev + _json->tokens[prev].childCount() < _token)
        --prev;
    return prev != ~std::size_t{} ? JsonIterator{_json, prev} : JsonIterator{};
}

JsonObjectView JsonToken::asObject() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeObject|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asObject(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(),
        (JsonObjectView{*_json, 0, 0}));

    return JsonObjectView{*_json, _token + 1, std::size_t(data._dataTypeNan & TypeLargeDataMask)};
}

JsonArrayView JsonToken::asArray() const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(),
        (JsonArrayView{*_json, 0, 0}));

    return JsonArrayView{*_json, _token + 1, std::size_t(data._dataTypeNan & TypeLargeDataMask)};
}

JsonIterator JsonToken::find(const Containers::StringView key) const {
    const JsonTokenData& data = _json->tokens[_token];
    /* Returning a valid iterator from the assert to not hit a second assert
       from operator[] below when testing */
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeObject|TypeSmallLargeIsParsed),
        "Utility::JsonToken::find(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type() << Debug::nospace << ", expected a parsed object", *this);

    for(std::size_t i = _token + 1, end = i + (data._dataTypeNan & TypeLargeDataMask); i != end; i = i + 1 + _json->tokens[i].childCount()) {
        /* Returning a valid iterator from the assert to not hit a second
           assert from operator[] below when testing */
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeLargeStringIsEscaped) == (JsonToken::TypeLargeStringKey|JsonToken::TypeSmallLargeIsParsed),
            "Utility::JsonToken::find(): key string isn't parsed", (JsonIterator{_json, _token}));
        const JsonToken out{*_json, i};
        if(out.asStringInternal() == key)
            return out.firstChild();
    }

    return {};
}

JsonToken JsonToken::operator[](const Containers::StringView key) const {
    const JsonIterator found = find(key);
    /** @todo any chance to report file/line here? would need to go upwards
        until the root token to find the start of the token stream, but then it
        still wouldn't be possible to get the filename */
    CORRADE_ASSERT(found, "Utility::JsonToken::operator[](): key" << key << "not found", *this);
    return *found;
}

JsonIterator JsonToken::find(const std::size_t index) const {
    const JsonTokenData& data = _json->tokens[_token];
    /* Returning a valid iterator from the assert to not hit a second assert
       from operator[] below when testing */
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::find(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type() << Debug::nospace << ", expected a parsed array", *this);

    std::size_t counter = 0;
    for(std::size_t i = _token + 1, end = i + (data._dataTypeNan & TypeLargeDataMask); i != end; i = i + 1 + _json->tokens[i].childCount())
        if(counter++ == index)
            return JsonToken{*_json, i};

    return {};
}

JsonToken JsonToken::operator[](const std::size_t index) const {
    const JsonIterator found = find(index);
    /** @todo something better like "index N out of range for M elements",
        would need an internal helper or some such to get the counter value */
    /** @todo any chance to report file/line here? would need to go upwards
        until the root token to find the start of the token stream, but then it
        still wouldn't be possible to get the filename */
    CORRADE_ASSERT(found, "Utility::JsonToken::operator[](): index" << index << "not found", *this);
    return *found;
}

Containers::StringView JsonToken::asStringInternal() const {
    /* If the string is not escaped, reference it directly */
    const Json::State& state = *static_cast<const Json::State*>(_json);
    const JsonTokenData& data = state.tokenStorage[_token];
    if(!(data._dataTypeNan & JsonToken::TypeLargeStringIsEscaped))
        return state.string.sliceSize(
            state.tokenOffsetsSizes[_token]._offset + 1,
            (state.tokenOffsetsSizes[_token]._sizeType & ~JsonToken::TypeTokenSizeMask) - 2);

    /* Otherwise take the cached version. Compared to _json->tokens the strings
       pointer isn't stored in the base JsonData array -- it may get changed on
       every reallocation so it'd have to get updated each time parseString()
       etc gets called, but we don't need to access that in the header so we
       can also just peek directly into the derived struct. */
    return state.strings[data._dataTypeNan & TypeLargeDataMask];
}

Containers::StringView JsonToken::asString() const {
    CORRADE_ASSERT((_json->tokens[_token]._dataTypeNan & TypeLargeMask & ~(TypeLargeStringIsEscaped|TypeLargeStringIsKey)) == (TypeLargeString|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asString(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});
    return asStringInternal();
}

Containers::StridedBitArrayView1D JsonToken::asBitArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asBitArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeSmallMask) == (JsonToken::TypeSmallBool|JsonToken::TypeSmallLargeIsParsed),
            "Utility::JsonToken::asBitArray(): token" << i - _token - 1 << "is" << (JsonToken{*_json, i}.isParsed() ? "a parsed" : "an unparsed") << (JsonToken{*_json, i}.type()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asBitArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedBool).sliceBit(0);
}

#ifdef CORRADE_BUILD_DEPRECATED
Containers::StridedArrayView1D<const bool> JsonToken::asBoolArray(const std::size_t expectedSize) const {
    /* It's easier to just create the view anew than try to inflate it from the
       returned one */
    const Containers::StridedBitArrayView1D out = asBitArray(expectedSize);
    return Containers::stridedArrayView(_json->tokens + _token + 1, out.size()).slice(&JsonTokenData::_parsedBool);
}
#endif

Containers::StridedArrayView1D<const double> JsonToken::asDoubleArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asDoubleArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeDouble,
            "Utility::JsonToken::asDoubleArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asDoubleArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedDouble);
}

Containers::StridedArrayView1D<const float> JsonToken::asFloatArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asFloatArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallFloat,
            "Utility::JsonToken::asFloatArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asFloatArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedFloat);
}

Containers::StridedArrayView1D<const std::uint32_t> JsonToken::asUnsignedIntArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asUnsignedIntArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallUnsignedInt,
            "Utility::JsonToken::asUnsignedIntArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asUnsignedIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedUnsignedInt);
}

Containers::StridedArrayView1D<const std::int32_t> JsonToken::asIntArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asIntArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeSmallMask) == JsonToken::TypeSmallInt,
            "Utility::JsonToken::asIntArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedInt);
}

Containers::StridedArrayView1D<const std::uint64_t> JsonToken::asUnsignedLongArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asUnsignedLongArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeUnsignedLong,
            "Utility::JsonToken::asUnsignedLongArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asUnsignedLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedUnsignedLong);
}

Containers::StridedArrayView1D<const std::int64_t> JsonToken::asLongArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asLongArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        CORRADE_ASSERT((_json->tokenOffsetsSizes[i]._sizeType & JsonToken::TypeTokenSizeMask) == JsonToken::TypeTokenSizeLong,
            "Utility::JsonToken::asLongArray(): token" << i - _token - 1 << "is a" << (JsonToken{*_json, i}.type()) << "parsed as" << (JsonToken{*_json, i}.parsedType()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(&data + 1, size).slice(&JsonTokenData::_parsedLong);
}

Containers::StridedArrayView1D<const std::size_t> JsonToken::asSizeArray(const std::size_t expectedSize) const {
    return Containers::arrayCast<const std::size_t>(
        #ifndef CORRADE_TARGET_32BIT
        asUnsignedLongArray(expectedSize)
        #else
        asUnsignedIntArray(expectedSize)
        #endif
    );
}

Containers::StringIterable JsonToken::asStringArray(const std::size_t expectedSize) const {
    const JsonTokenData& data = _json->tokens[_token];
    CORRADE_ASSERT((data._dataTypeNan & TypeLargeMask) == (TypeLargeArray|TypeSmallLargeIsParsed),
        "Utility::JsonToken::asStringArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size = data._dataTypeNan & TypeLargeDataMask;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of skipping nested.childCount(). If a nested object or array
       would be encountered, the parsedType() check fails. */
    for(std::size_t i = _token + 1, end = i + size; i != end; ++i)
        /* String array values can't be keys so not filtering away the
           TypeLargeStringIsKey bit */
        CORRADE_ASSERT((_json->tokens[i]._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeLargeStringIsEscaped) == (JsonToken::TypeLargeString|JsonToken::TypeSmallLargeIsParsed),
            "Utility::JsonToken::asStringArray(): token" << i - _token - 1 << "is" << (JsonToken{*_json, i}.isParsed() ? "a parsed" : "an unparsed") << (JsonToken{*_json, i}.type()), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asStringArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::StringIterable{&data + 1, _json, size, sizeof(JsonTokenData), [](const void* data, const void* context, std::ptrdiff_t, std::size_t) {
        const Implementation::JsonData& json = *static_cast<const Implementation::JsonData*>(context);
        return JsonToken{json, std::size_t(static_cast<const JsonTokenData*>(data) - json.tokens)}.asString();
    }};
}

Containers::StringView JsonObjectItem::key() const {
    CORRADE_ASSERT(_token.isParsed(),
        "Utility::JsonObjectItem::key(): string isn't parsed", {});
    /** @todo asStringInternal() to avoid the nested assert? */
    return _token.asString();
}

Debug& operator<<(Debug& debug, const JsonToken::Type value) {
    debug << "Utility::JsonToken::Type" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(v) case JsonToken::Type::v: return debug << "::" #v;
        _c(Object)
        _c(Array)
        _c(Null)
        _c(Bool)
        _c(Number)
        _c(String)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << std::uint8_t(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const JsonToken::ParsedType value) {
    debug << "Utility::JsonToken::ParsedType" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(v) case JsonToken::ParsedType::v: return debug << "::" #v;
        _c(None)
        _c(Double)
        _c(Float)
        _c(UnsignedInt)
        _c(Int)
        _c(UnsignedLong)
        _c(Long)
        _c(Other)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << std::uint8_t(value) << Utility::Debug::nospace << ")";
}

Json::Json(Containers::String&& string, Containers::Array<JsonTokenData>&& tokens, Containers::Array<JsonTokenOffsetSize>&& tokenOffsetsSizes, Containers::Array<Containers::String>&& strings): _state{InPlaceInit} {
    CORRADE_ASSERT(!tokens.isEmpty(),
        "Utility::Json: expected at least one token", );
    CORRADE_ASSERT(tokens.size() == tokenOffsetsSizes.size(),
        "Utility::Json: expected token and offset/size arrays to have the same size but got" << tokens.size() << "and" << tokenOffsetsSizes.size(), );

    /* These are set before the remaining asserts so it's possible to form
       JsonToken instances for type printing and such. The actual storage is
       moved only after however, to be able to still reference the original
       input arrays. */
    _state->tokens = tokens.data();
    _state->tokenOffsetsSizes = tokenOffsetsSizes.data();
    _state->tokenCount = tokens.size();
    /* This is just for (doomed to fail) diagnostic in parse*() */
    _state->filename = Containers::String::nullTerminatedGlobalView("<in>"_s);

    #ifndef CORRADE_NO_ASSERT
    /* Check that token child counts don't overlap. Do this before everything
       else to avoid strange failures in subsequent checks that rely on child
       counts being correct. */
    Containers::Array<std::size_t> ends;
    for(std::size_t i = 0; i != tokens.size(); ++i) {
        /* The stack can be only empty for the very first token, if it becomes
           empty before the end it means there's more than one root token */
        CORRADE_ASSERT(i == 0 || !ends.isEmpty(),
            "Utility::Json: extraneous root token" << i, );

        /* Not calling childCount() as it could blow up if called on a string
           key that's errorneously last in the object. Check for that is
           below. */
        const JsonTokenData& token = tokens[i];
        if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) == JsonToken::TypeLargeArray ||
           (token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) == JsonToken::TypeLargeObject) {
            const std::size_t childCount = token._dataTypeNan & JsonToken::TypeLargeDataMask;
            CORRADE_ASSERT(i + childCount + 1 <= (ends.isEmpty() ? tokens.size() : ends.back()),
                "Utility::Json: token" << i << "has" << childCount << "children but expected at most" << (ends.isEmpty() ? tokens.size() : ends.back()) - i - 1, );
            arrayAppend(ends, i + childCount + 1);
        }

        while(!ends.isEmpty() && ends.back() == i + 1)
            arrayRemoveSuffix(ends);
    }
    /* The ends are guaranteed to be empty afterwards. If they wouldn't be it
       means that the root object child count is larger than token count, which
       should have been caught by the assert above already. */
    CORRADE_INTERNAL_ASSERT(ends.isEmpty());

    /* Check that object keys are strings and are marked as keys. Also remember
       which tokens are keys to verify non-key strings aren't marked as such in
       a subsequent loop. */
    Containers::BitArray keys{ValueInit, tokens.size()};
    for(std::size_t i = 0; i != tokens.size(); ++i) {
        const JsonTokenData& token = tokens[i];
        if((token._dataTypeNan & JsonToken::TypeLargeMask & ~JsonToken::TypeSmallLargeIsParsed) != JsonToken::TypeLargeObject)
            continue;

        /* Compared to above, here it *is* using childCount() in order to
           correctly skip over object values that are arrays or objects. But as
           every iteration verifies that the token is a string with a key bit
           is set, it should be correct. For the keys at least, there still can
           be non-key strings errorneously marked as keys, which gets checked
           in the next loop below. */
        for(std::size_t j = i + 1, end = j + token.childCount(); j != end; ) {
            CORRADE_ASSERT((tokens[j]._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeLargeStringIsEscaped|JsonToken::TypeLargeStringIsKey|JsonToken::TypeSmallLargeIsParsed)) == JsonToken::TypeLargeString,
                "Utility::Json: token" << j << "expected to be a string key but is" << (JsonToken{*this, tokens[j]}).type(), );
            CORRADE_ASSERT(tokens[j]._dataTypeNan & JsonToken::TypeLargeStringIsKey,
                "Utility::Json: string token" << j << "is not marked as a key", );
            keys.set(j);

            /* A string token marked as a key is expected to have at least one
               child. If it doesn't have it, it'll either blow up in the
               asserts above, depending on whether the next token is a string
               or not. Or, if it's at object end, the next token will be
               calculated outside of the object, which has to be caught
               explicitly as well. */
            std::size_t next = j + 1 + tokens[j].childCount();
            CORRADE_ASSERT(next <= end,
                "Utility::Json: string key token" << j << "has no value", );
            j = next;
        }
    }

    /* Verify remaining token properties that aren't reliant on child counts
       and keys */
    for(std::size_t i = 0; i != tokens.size(); ++i) {
        const JsonTokenData& token = tokens[i];

        /* Check token offset and size */
        CORRADE_ASSERT(tokenOffsetsSizes[i]._offset + (tokenOffsetsSizes[i]._sizeType & ~JsonToken::TypeTokenSizeMask) <= string.size(),
            "Utility::Json: token" << i << "offset" << tokenOffsetsSizes[i]._offset << "and size" << (tokenOffsetsSizes[i]._sizeType & ~JsonToken::TypeTokenSizeMask) << "out of range for input of size" << string.size(), );

        /* String token properties */
        if((token._dataTypeNan & JsonToken::TypeLargeMask & ~(JsonToken::TypeSmallLargeIsParsed|JsonToken::TypeLargeStringIsEscaped|JsonToken::TypeLargeStringIsKey)) == JsonToken::TypeLargeString) {
            /* If a token is a string, it should be either a key (verified
               above) or not marked as such */
            CORRADE_ASSERT(keys[i] || !(token._dataTypeNan & JsonToken::TypeLargeStringIsKey),
                "Utility::Json: string token" << i << "is not expected to be a key", );

            /* Check escaped token index */
            if(token._dataTypeNan & JsonToken::TypeLargeStringIsEscaped)
                CORRADE_ASSERT((token._dataTypeNan & JsonToken::TypeLargeDataMask) < strings.size(),
                    "Utility::Json: escaped string token" << i << "index" << (token._dataTypeNan & JsonToken::TypeLargeDataMask) << "out of range for" << strings.size() << "escaped strings", );
            /* Otherwise the token should be at least 2 bytes, containing the
               initial and final quote (which isn't checked anywhere however
               and could be even a ' character for all I care) */
            else
                CORRADE_ASSERT((tokenOffsetsSizes[i]._sizeType & ~JsonToken::TypeTokenSizeMask) >= 2,
                    "Utility::Json: string token" << i << "should be at least two bytes but got" << (tokenOffsetsSizes[i]._sizeType & ~JsonToken::TypeTokenSizeMask), );
        }
    }
    #endif

    /* All good, take ownership over everything */
    _state->string = _state->storage = Utility::move(string);
    _state->tokenStorage = Utility::move(tokens);
    _state->tokenOffsetSizeStorage = Utility::move(tokenOffsetsSizes);
    _state->strings = Utility::move(strings);
}

JsonTokenData::JsonTokenData(const JsonToken::Type type, const std::uint64_t childCountOrStringIndex, bool stringIsKey) {
    if(type == JsonToken::Type::Object || type == JsonToken::Type::Array) {
        CORRADE_ASSERT(childCountOrStringIndex <= JsonToken::TypeLargeDataMask,
            "Utility::JsonTokenData: expected child count to fit into 48 bits, got" << childCountOrStringIndex, );
        CORRADE_ASSERT(!stringIsKey,
            "Utility::JsonTokenData: object or array can't be a key", );
        _dataTypeNan = childCountOrStringIndex|(type == JsonToken::Type::Object ? JsonToken::TypeLargeObject : JsonToken::TypeLargeArray)|JsonToken::TypeSmallLargeIsParsed;
    } else if(type == JsonToken::Type::String) {
        CORRADE_ASSERT(childCountOrStringIndex == ~std::uint64_t{} || childCountOrStringIndex <= JsonToken::TypeLargeDataMask,
            "Utility::JsonTokenData: expected escaped string index to be either all 1s or fit into 48 bits but got" << childCountOrStringIndex, );
        _dataTypeNan = (childCountOrStringIndex != ~std::uint64_t{} ? childCountOrStringIndex : 0)|JsonToken::TypeLargeString|(stringIsKey ? std::uint64_t(JsonToken::TypeLargeStringIsKey) : 0)|(childCountOrStringIndex != ~std::uint64_t{} ? std::uint64_t(JsonToken::TypeLargeStringIsEscaped) : 0)|JsonToken::TypeSmallLargeIsParsed;
    } else CORRADE_ASSERT_UNREACHABLE("Utility::JsonTokenData: expected object, array or a string type but got" << type, );
}

JsonTokenData::JsonTokenData(std::nullptr_t): _dataTypeNan{JsonToken::TypeSmallNull|JsonToken::TypeSmallLargeIsParsed} {}

JsonTokenData::JsonTokenData(const bool value): _dataTypeNan{JsonToken::TypeSmallBool|JsonToken::TypeSmallLargeIsParsed} {
    _parsedBool = value;
}

JsonTokenData::JsonTokenData(const double value, JsonTokenOffsetSize& offsetSize): _parsedDouble{value} {
    CORRADE_ASSERT(!std::isnan(value) && !std::isinf(value),
        "Utility::JsonTokenData: invalid floating-point value" << value, );
    offsetSize._sizeType |= JsonToken::TypeTokenSizeDouble;
}

JsonTokenData::JsonTokenData(const float value): _dataTypeNan{JsonToken::TypeSmallFloat|JsonToken::TypeSmallLargeIsParsed} {
    CORRADE_ASSERT(!std::isnan(value) && !std::isinf(value),
        "Utility::JsonTokenData: invalid floating-point value" << value, );
    _parsedFloat = value;
}

JsonTokenData::JsonTokenData(const std::uint32_t value): _dataTypeNan{JsonToken::TypeSmallUnsignedInt|JsonToken::TypeSmallLargeIsParsed} {
    _parsedUnsignedInt = value;
}

JsonTokenData::JsonTokenData(const std::int32_t value): _dataTypeNan{JsonToken::TypeSmallInt|JsonToken::TypeSmallLargeIsParsed} {
    _parsedInt = value;
}

JsonTokenData::JsonTokenData(const unsigned long long value, JsonTokenOffsetSize& offsetSize): _parsedUnsignedLong{value} {
    CORRADE_ASSERT(value < 1ull << 52,
        "Utility::JsonTokenData: too large integer value" << value, );
    offsetSize._sizeType |= JsonToken::TypeTokenSizeUnsignedLong;
}

JsonTokenData::JsonTokenData(const long long value, JsonTokenOffsetSize& offsetSize): _parsedLong{value} {
    CORRADE_ASSERT(value >= -(1ll << 52) && value < (1ll << 52),
        "Utility::JsonTokenData: too small or large integer value" << value, );
    offsetSize._sizeType |= JsonToken::TypeTokenSizeLong;
}

}}
