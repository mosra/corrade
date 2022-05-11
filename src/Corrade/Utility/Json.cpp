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

#include "Json.h"

#include <climits>
#ifdef CORRADE_TARGET_32BIT
#include <cmath> /* std::isinf(), std::isnan() */
#endif
#include <cstring>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Utility/Path.h"
#include "Corrade/Utility/Unicode.h"

namespace Corrade { namespace Utility {

/*

### JSON token data layout

At the very least, the JSON token has to contain a pointer to the input string
for token begin, a token byte size and, in case of objects or arrays the number
of child tokens to make traversals possible. Token type can be determined
implicitly, as mentioned in the public docs.

On 32bit architectures all three (pointer, size and child count) are 32bit,
thus looking like on the left. On 64bit, the pointer has to be 64bit, and the
size as well, as a 4 GB limitation wouldn't be wise. Due to alignment
restrictions, this means we have up to 64 bits left for the child token count
as well, as shown on the right:

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
    be allocated externally with the token somehow storing a pointer (thus
    64-bit on 64 bit systems) to the unescaped variant.
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

As value tokens have no children, a double (or a string pointer, or a boolean
value) can be stored in place of the child count. Which is coincidentally why
the diagrams above are both 8-byte aligned. Technically, string tokens that
represent object keys have a children -- the object value -- but such
information can be again determined implicitly.

Another goal is to have numbers parsable on-demand. Thus there needs to be a
way to know whether a token has its value already parsed or not (and for
numbers additionally whether it's a double, an int, etc.), and an ability to
change the parsed state later (turning an unparsed string to a parsed one, or
a parsed float to an int).

### 64bit case

The 64bit case is simpler, as we can abuse the upper bits of a 64-bit size --
even though web is getting increasingly bloated by the day, I don't expect
JSONs with petabyte sizes to exist anytime soon.

    +-------------------+      +-------------------+      +-------------------+
    |      pointer      |      |      pointer      |      |      pointer      |
    +-------------------+      +-------------------+      +-------------------+
    |    size     | ... |  or  |    size     | ... |  or  |    size     | ... |
    +-------------------+      +-------------------+      +-------------------+
    |   bool / number   |      |  string pointer   |      |    child count    |
    +-------------------+      +-------------------+      +-------------------+

In the upper bits of size we store these 9 bits of information:

-   3 bits for token type (null, bool, number, string, object, array), to avoid
    having to suffer the data pointer indirection every time
-   3 bits for whether it's parsed and to what numeric type
-   Whether the JSON string is global (so global views can be returned for
    asString())
-   Whether the string contains any escape characters, as such information is
    already known at tokenization time and thus it would be silly to have to
    rescan the string again during a parse step
-   Whether the string is a key or a value, to easily differentiate between
    parsing string keys alone or all strings

And then the final 64bit value is either:

-   a bool value,
-   a double, float, (unsigned) int or (unsigned) long value,
-   a pointer to an external parsed string,
-   or child count for object and arrays.

### 32bit case

In the 32bit case it's not desired to limit sizes too much below 4 GB, so we
can't reuse the top bits for anything. Instead, the NaN value is abused
similarly to what JS engines do to efficiently store data. A 64-bit double
value is a (signed) NaN or (signed) infinity if the 11-bit exponent is set to
all 1s. Since JSON has no way to store NaN or infinity values, let alone NaNs
with custom bit patterns, we can abuse the remaining 53 bits to store whatever
else.

1.  +-----------+-----------+
    |  pointer  |   size    |
    +-+-----+---+-----------+
    |0| NaN | … |   data    |
    +-+-----+---+-----------+

First the initial state that happens right after tokenization --- if the
exponent is a NaN (all 1s) and the sign is 0, the remaining 52 bits store these
bits of information:

-   3 bits for token type, same as in the 64-bit case,
-   1 bit for whether it's parsed, which is always 1 for objects and arrays and
    always 0 for numbers (parsing numbers will switch them to the non-NaN
    representation),
-   3 bits for whether a string contains any escape characters, whether it's
    global or whether it's a key or a value, same as in the 64-bit case

And the lower 32 bits to store one of the following if the parsed bit is set:

-   a bool value,
-   a pointer to an external parsed string,
-   or child count for objects and arrays.

2.  +-----------+---------+-+
    |  pointer  |  size   |…|
    +-----------+---------+-+
    |?| ??? |      ...      |
    +-+-----+---------------+

Otherwise (i.e., in all other cases except a NaN and sign being 0), the token
is a parsed 32-bit or 64-bit number. In order to fit 64-bit numbers as well, we
can't abuse any bits for anything, but since a string representation of a
number is unlikely to be thousands of characters (the parsing code even caps
numeric literals at 127 chars at the moment), we can reuse the top bits of size
instead:

-   3 bits for parsed number type, same as in the 64-bit case

The actual type of data stored in the 64-bit field then depends on the parsed
type identifier, of course, but let's enumerate all cases for clarity:

2a. +-----------+---------+-+
    |  pointer  |  size   |…|
    +-----------+---------+-+
    |1| NaN |     -int      |
    +-+-----+---------------+

If the exponent is a NaN and the sign is 1, the upper bits is how a negative
53-bit number looks like when expanded to 64-bit. Meaning, it's possible to
directly store negative 64-bit integers in the bit pattern of a double.

2b. +-----------+---------+-+    +-----------+---------+-+
    |  pointer  |  size   |…|    |  pointer  |  size   |…|
    +-----------+---------+-+ or +-----------+---------+-+
    |0| 0…0 | +int / double |    |0| 0…0 |   |  number   |
    +-+-----+---------------+    +-+-----+---+-----------+

Conversely, if the exponent is all zeros and the sign is 0, the rest can be a
positive 52-bit integer. But this bit pattern can be also a double, or there
can be a 32-bit value stored in the bottom half --- the type field stored in
the upper 3 bits of size will disambiguate.

2c. +-----------+---------+-+
    |  pointer  |  size   |…|
    +-----------+---------+-+
    |     double number     |
    +-----------------------+

Finally, if the exponent is anything else than all 1s or all 0s, it's a general
double number.

*/

using namespace Containers::Literals;

struct Json::State {
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

    Containers::Array<JsonToken> tokens;
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

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset, const Containers::StringView string_) {
    Json json;
    json._state.emplace();

    /* Make a copy of the input string if not marked as global */
    const std::uint64_t globalStringFlag = string_.flags() & Containers::StringViewFlag::Global ? std::uint64_t(JsonToken::FlagStringGlobal) : 0;
    if(globalStringFlag)
        json._state->string = string_;
    else
        json._state->string = json._state->storage = string_;

    /* Save also the filename for subsequent error reporting */
    json._state->filename = Containers::String::nullTerminatedGlobalView(filename ? filename : "<in>"_s);
    json._state->lineOffset = lineOffset;
    json._state->columnOffset = columnOffset;

    /* A sentinel token at the start, to limit Json::parent() */
    constexpr JsonToken sentinel{ValueInit};
    arrayAppend(json._state->tokens, sentinel);

    /* Remember surrounding object or array token index to update its size,
       child count and check matching braces when encountering } / ] */
    std::size_t objectOrArrayTokenIndex = 0;

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

    /* Remember how many strings contain escape codes to allocate an immovable
       storage for them */
    std::size_t escapedStringCount = 0;

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
                JsonToken token{NoInit};
                token._data = data + i;
                /* Size and child count gets filled in once } / ] is
                   encountered. Until then, abuse the _childCount field to
                   store the previous object / array index and remember this
                   index for when we get to } / ]. */
                #ifndef CORRADE_TARGET_32BIT
                token._sizeFlagsParsedTypeType =
                    (c == '{' ? JsonToken::TypeObject : JsonToken::TypeArray);
                token._childCount = objectOrArrayTokenIndex;
                #else
                token._childCountFlagsTypeNan =
                    JsonToken::NanMask|objectOrArrayTokenIndex|
                    (c == '{' ? JsonToken::TypeObject : JsonToken::TypeArray);
                #endif
                objectOrArrayTokenIndex = json._state->tokens.size();
                arrayAppend(json._state->tokens, token);

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
                JsonToken& token = json._state->tokens[objectOrArrayTokenIndex];
                const bool isObject = (token.
                    #ifndef CORRADE_TARGET_32BIT
                    _sizeFlagsParsedTypeType
                    #else
                    _childCountFlagsTypeNan
                    #endif
                    & JsonToken::TypeMask) == JsonToken::TypeObject;
                if((c == '}') != isObject) {
                    Error err;
                    err << ErrorPrefix << "unexpected" << json._state->string.slice(i, i + 1) << "at";
                    json.printFilePosition(err, json._state->string.prefix(i));
                    err << "for an" << (c == ']' ? "object" : "array") << "starting at";
                    /* Printing the filename again, because it will make a
                       useful clickable link in terminal even though a bit
                       redundant */
                    json.printFilePosition(err, json._state->string.prefix(token._data));
                    return {};
                }

                /* The child count field was abused to store the previous
                   object / array index. Restore it and set the actual child
                   count to the field. */
                const std::size_t tokenChildCount = json._state->tokens.size() - objectOrArrayTokenIndex - 1;
                objectOrArrayTokenIndex =
                    #ifndef CORRADE_TARGET_32BIT
                    token._childCount
                    #else
                    token._childCountFlagsTypeNan & JsonToken::ChildCountMask
                    #endif
                    ;
                #ifndef CORRADE_TARGET_32BIT
                token._childCount = tokenChildCount;
                #else
                token._childCountFlagsTypeNan =
                    (token._childCountFlagsTypeNan & ~JsonToken::ChildCountMask)|
                    tokenChildCount;
                #endif

                /* Update the token size to contain everything parsed until
                   now */
                const std::size_t tokenSize = data + i - token._data + 1;
                #ifndef CORRADE_TARGET_32BIT
                token._sizeFlagsParsedTypeType |= tokenSize;
                #else
                token._sizeParsedType = tokenSize;
                #endif

                /* Next should be a comma or an end depending on what the
                   new parent is */
                if(!objectOrArrayTokenIndex)
                    expecting = Expecting::DocumentEnd;
                else
                    expecting = (json._state->tokens[objectOrArrayTokenIndex].
                        #ifndef CORRADE_TARGET_32BIT
                        _sizeFlagsParsedTypeType
                        #else
                        _childCountFlagsTypeNan
                        #endif
                        & JsonToken::TypeMask) == JsonToken::TypeObject ?
                        Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;

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

                    if(sc == '\"') break;

                    if(sc == '\\') switch(data[++i]) {
                        case '"':
                        case '\\':
                        case '/': /* JSON, why, you're weird */
                        case 'b':
                        case 'f':
                        case 'n':
                        case 'r':
                        case 't':
                        case 'u': /* Deliberately not validating Unicode here */
                            escapedFlag = JsonToken::FlagStringEscaped;
                            ++escapedStringCount;
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
                JsonToken token{NoInit};
                token._data = data + start;
                const std::size_t tokenSize = i - start + 1;
                #ifndef CORRADE_TARGET_32BIT
                token._sizeFlagsParsedTypeType = tokenSize|
                    JsonToken::TypeString|escapedFlag|globalStringFlag;
                #else
                token._sizeParsedType = tokenSize;
                token._childCountFlagsTypeNan =
                    JsonToken::NanMask|escapedFlag|globalStringFlag|
                    JsonToken::TypeString;
                #endif

                /* Remember if this is an object key. In that case we're
                   expecting the colon next. */
                if(expecting == Expecting::ObjectKey ||
                   expecting == Expecting::ObjectKeyOrEnd)
                {
                    #ifndef CORRADE_TARGET_32BIT
                    token._sizeFlagsParsedTypeType
                    #else
                    token._childCountFlagsTypeNan
                    #endif
                        |= JsonToken::FlagStringKey;
                    expecting = Expecting::ObjectKeyColon;

                /* Otherwise it's a value and we're expecting a comma or an
                   end depending on what the parent is */
                } else if(expecting == Expecting::Value ||
                          expecting == Expecting::ValueOrArrayEnd)
                {
                    if(!objectOrArrayTokenIndex)
                        expecting = Expecting::DocumentEnd;
                    else
                        expecting = (json._state->tokens[objectOrArrayTokenIndex].
                            #ifndef CORRADE_TARGET_32BIT
                            _sizeFlagsParsedTypeType
                            #else
                            _childCountFlagsTypeNan
                            #endif
                            & JsonToken::TypeMask) == JsonToken::TypeObject ?
                            Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;
                } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */

                arrayAppend(json._state->tokens, token);

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
                       lc == ',' || lc == ']' || lc == '}') break;
                }

                /* Decrement i as it's incremented again by the outer loop */
                --i;

                const std::size_t tokenSize = i - start + 1;
                std::uint64_t tokenType;
                if(c == 'n')
                    tokenType = JsonToken::TypeNull;
                else if(c == 't' || c == 'f')
                    tokenType = JsonToken::TypeBool;
                else
                    tokenType = JsonToken::TypeNumber;

                JsonToken token{NoInit};
                token._data = data + start;
                #ifndef CORRADE_TARGET_32BIT
                token._sizeFlagsParsedTypeType = tokenSize|tokenType;
                #else
                token._sizeParsedType = tokenSize;
                token._childCountFlagsTypeNan = JsonToken::NanMask|tokenType;
                #endif

                arrayAppend(json._state->tokens, token);

                /* Expecting a comma or end next, depending on what the parent
                   is */
                if(!objectOrArrayTokenIndex)
                    expecting = Expecting::DocumentEnd;
                else
                    expecting = (json._state->tokens[objectOrArrayTokenIndex].
                        #ifndef CORRADE_TARGET_32BIT
                        _sizeFlagsParsedTypeType
                        #else
                        _childCountFlagsTypeNan
                        #endif
                        & JsonToken::TypeMask) == JsonToken::TypeObject ?
                            Expecting::CommaOrObjectEnd :
                            Expecting::CommaOrArrayEnd;

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
                expecting = (json._state->tokens[objectOrArrayTokenIndex].
                    #ifndef CORRADE_TARGET_32BIT
                    _sizeFlagsParsedTypeType
                    #else
                    _childCountFlagsTypeNan
                    #endif
                    & JsonToken::TypeMask) == JsonToken::TypeObject ?
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

    if(objectOrArrayTokenIndex != 0) {
        Error err;
        err << ErrorPrefix << "file too short, expected closing";
        const JsonToken& token = json._state->tokens[objectOrArrayTokenIndex];
        if(expecting == Expecting::CommaOrObjectEnd)
            err << "} for object";
        else if(expecting == Expecting::CommaOrArrayEnd)
            err << "] for array";
        else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        err << "starting at";
        json.printFilePosition(err, json._state->string.prefix(token._data));
        return {};
    }

    /* Reserve memory for parsed string instances -- since the tokens reference
       them through a pointer, it has to be an immovable allocation */
    /** @todo use a non-reallocating allocator once it exists */
    arrayReserve(json._state->strings, escapedStringCount);

    /* All good */
    /* GCC 4.8 and Clang 3.8 need a bit of help here */
    return Containers::optional(std::move(json));
}

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const std::size_t lineOffset, const std::size_t columnOffset, const Containers::StringView string, const Options options) {
    Containers::Optional<Json> out = tokenize(filename, lineOffset, columnOffset, string);
    if(!out) return {};

    if((options & Option::ParseLiterals) && !out->parseLiterals(out->root()))
        return {};

    /* If both ParseDoubles and ParseFloats is specified, doubles get a
       priority */
    if(options & Option::ParseDoubles) {
        if(!out->parseDoubles(out->root())) return {};
    } else if(options & Option::ParseFloats) {
        if(!out->parseFloats(out->root())) return {};
    }

    /* ParseStrings is a superset of ParseStringKeys, so don't call both */
    if(options >= Option::ParseStrings) {
        if(!out->parseStrings(out->root())) return {};
    } else if(options >= Option::ParseStringKeys) {
        if(!out->parseStringKeys(out->root())) return {};
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

Containers::ArrayView<const JsonToken> Json::tokens() const {
    return _state->tokens.exceptPrefix(1);
}

const JsonToken& Json::root() const {
    /* An empty file is not a valid JSON, so there should always be at least
       one token plus sentinel at the start */
    CORRADE_INTERNAL_ASSERT(_state->tokens.size() >= 2);
    return _state->tokens[1];
}

void Json::parseObjectArrayInternal(JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if(
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask
        #else
        token._childCountFlagsTypeNan & JsonToken::FlagParsed
        #endif
    ) return;

    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeOther;
    #else
    token._childCountFlagsTypeNan |= JsonToken::FlagParsed;
    #endif
}

bool Json::parseNullInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if(
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask
        #else
        token._childCountFlagsTypeNan & JsonToken::FlagParsed
        #endif
    ) return true;

    const Containers::StringView string = token.data();
    if(string != "null"_s) {
        Error err;
        err << errorPrefix << "invalid null literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success mark the token as parsed. The value is implicit, so nothing
       to save. */
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeOther;
    #else
    token._childCountFlagsTypeNan |= JsonToken::FlagParsed;
    #endif
    return true;
}

bool Json::parseBoolInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if(
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask
        #else
        token._childCountFlagsTypeNan & JsonToken::FlagParsed
        #endif
    ) return true;

    const Containers::StringView string = token.data();
    if(string == "true"_s)
        token._parsedBool = true;
    else if(string == "false"_s)
        token._parsedBool = false;
    else {
        Error err;
        err << errorPrefix << "invalid bool literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success mark the token as parsed */
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeOther;
    #else
    token._childCountFlagsTypeNan |= JsonToken::FlagParsed;
    #endif
    return true;
}

bool Json::parseDoubleInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeDouble
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedDouble directly to avoid a failure corrupting
       the high bits storing token type and flags on 32bit */
    const double out = std::strtod(buffer, &end);
    if(std::size_t(end - buffer) != size
        #ifdef CORRADE_TARGET_32BIT
        /* Explicitly disallowing NaNs to not clash with the NaN bit pattern
           stuffing on 32b. Not on 64b and not , even though NAN and INF
           literals in a JSON is a non-conforming behavior (see all XFAILs in
           tests) */
        /** @todo drop this once we have a conversion routine with proper
            control */
        || std::isinf(out) || std::isnan(out)
        #endif
    ) {
        Error err;
        err << errorPrefix << "invalid floating-point literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success save the parsed value and its type. On 32bit the parsed type
       is stored in the size, the lack of a NaN implying that it's parsed. */
    token._parsedDouble = out;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeDouble;
    #else
    CORRADE_INTERNAL_ASSERT((token._childCountFlagsTypeNan & JsonToken::NanMask) != JsonToken::NanMask);
    token._sizeParsedType = JsonToken::ParsedTypeDouble|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseFloatInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeFloat
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    token._parsedFloat = std::strtof(buffer, &end);
    if(std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid floating-point literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success save the parsed token type. On 32bit it's contained in the
       size, clear the NaN bits to imply that it's parsed. */
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeFloat;
    #else
    token._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
    token._sizeParsedType = JsonToken::ParsedTypeFloat|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseUnsignedIntInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeUnsignedInt
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not using strtoul() here as on Windows it's 32-bit and we wouldn't be
       able to detect overflows */
    /** @todo replace with something that can report errors in a non-insane
        way */
    const std::uint64_t outLong = std::strtoull(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid unsigned integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }
    if(outLong > ~std::uint32_t{}) {
        Error err;
        err << errorPrefix << "too large integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success convert the value to 32 bits and save the parsed token type.
       On 32bit it's contained in the size, clear the NaN bits to imply that
       it's parsed. */
    token._parsedUnsignedInt = outLong;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeUnsignedInt;
    #else
    token._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
    token._sizeParsedType = JsonToken::ParsedTypeUnsignedInt|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseIntInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeInt
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not using strtol() here as on Windows it's 32-bit and we wouldn't be
       able to detect overflows */
    /** @todo replace with something that can report errors in a non-insane
        way */
    const std::int64_t outLong = std::strtoll(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }
    if(outLong < INT_MIN || outLong > INT_MAX) {
        Error err;
        err << errorPrefix << "too small or large integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success convert the value to 32 bits and save the parsed token type.
       On 32bit it's contained in the size, clear the NaN bits to imply that
       it's parsed. */
    token._parsedInt = outLong;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeInt;
    #else
    token._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
    token._sizeParsedType = JsonToken::ParsedTypeInt|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseUnsignedLongInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeUnsignedLong
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedUnsignedLong directly to avoid a failure
       corrupting the high bits storing token type and flags on 32bit */
    const std::uint64_t out = std::strtoull(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid unsigned integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }
    if(out >= 1ull << 52) {
        Error err;
        err << errorPrefix << "too large integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success save the parsed value and its type. On 32bit the parsed type
       is stored in the size, the NaN and sign bits should be already all 0 for
       a 52-bit number. */
    token._parsedUnsignedLong = out;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeUnsignedLong;
    #else
    CORRADE_INTERNAL_ASSERT((token._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == 0);
    token._sizeParsedType = JsonToken::ParsedTypeUnsignedLong|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseLongInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if((
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType
        #else
        token._sizeParsedType
        #endif
        & JsonToken::ParsedTypeMask) == JsonToken::ParsedTypeLong
    ) return true;

    const Containers::StringView string = token.data();
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error err;
        err << errorPrefix << "too long numeric literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    /* Not saving to token._parsedLong directly to avoid a failure corrupting
       the high bits storing token type and flags on 32bit */
    const std::int64_t out = std::strtoll(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error err;
        err << errorPrefix << "invalid integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }
    if(out < -(1ll << 52) || out >= (1ll << 52)) {
        Error err;
        err << errorPrefix << "too small or large integer literal" << string << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return false;
    }

    /* On success save the parsed value and its type. On 32bit the parsed type
       is stored in the size, the NaN and sign bits should be already either
       all 0 for a positive 52-bit number or all 1 for a negative 53-bit
       number. */
    token._parsedLong = out;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeLong;
    #else
    CORRADE_INTERNAL_ASSERT(
        (token._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == 0 ||
        (token._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == (JsonToken::NanMask|JsonToken::SignMask));
    token._sizeParsedType = JsonToken::ParsedTypeLong|
        (token._sizeParsedType & ~JsonToken::ParsedTypeMask);
    #endif
    return true;
}

bool Json::parseStringInternal(const char* const errorPrefix, JsonToken& token) {
    /* If the token is already parsed, nothing to do. Assumes the caller
       checked for correct token type, otherwise this would be testing totally
       random bits. */
    if(
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask
        #else
        token._childCountFlagsTypeNan & JsonToken::FlagParsed
        #endif
    ) return true;

    /* If a token has no escapes, mark it as parsed and return. This is not
       done implicitly in order to force users to always explicitly call
       parseString*() before using the string values. */
    if(
        #ifndef CORRADE_TARGET_32BIT
        !(token._sizeFlagsParsedTypeType & JsonToken::FlagStringEscaped)
        #else
        !(token._childCountFlagsTypeNan & JsonToken::FlagStringEscaped)
        #endif
    ) {
        #ifndef CORRADE_TARGET_32BIT
        token._sizeFlagsParsedTypeType |= JsonToken::ParsedTypeOther;
        #else
        token._childCountFlagsTypeNan |= JsonToken::FlagParsed;
        #endif
        return true;
    }

    /* Otherwise parse the escapes */
    const Containers::StringView string = token.data();
    /** @todo use a non-reallocating allocator for more robustness once it
        exists */
    /* This assert would fire if we miscalculated during an initial parse
       (unlikely), or if the referenced token is not owned by the Json
       instance. That should have been checked by the caller. */
    CORRADE_INTERNAL_ASSERT(_state->strings.size() < arrayCapacity(_state->strings));
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

    /* On success save pointer to the parsed string and mark it as parsed */
    token._parsedString = &destination;
    #ifndef CORRADE_TARGET_32BIT
    token._sizeFlagsParsedTypeType =
        (token._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
        JsonToken::ParsedTypeOther;
    #else
    token._childCountFlagsTypeNan |= JsonToken::FlagParsed;
    #endif
    return true;
}

bool Json::parseLiterals(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseLiterals(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip tokens that are already parsed */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.isParsed()) continue;

        if(nestedToken.type() == JsonToken::Type::Object ||
           nestedToken.type() == JsonToken::Type::Array) {
            parseObjectArrayInternal(nestedToken);

        } else if(nestedToken.type() == JsonToken::Type::Null) {
            if(!parseNullInternal("Utility::Json::parseLiterals():", nestedToken))
                return false;

        } else if(nestedToken.type() == JsonToken::Type::Bool) {
            if(!parseBoolInternal("Utility::Json::parseLiterals():", nestedToken))
                return false;
        }
    }

    return true;
}

bool Json::parseDoubles(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseDoubles(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           doubles */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::Double)
            continue;

        if(!parseDoubleInternal("Utility::Json::parseDoubles():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseFloats(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseFloats(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           floats */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::Float)
            continue;

        if(!parseFloatInternal("Utility::Json::parseFloats():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseUnsignedInts(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseUnsignedInts(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           unsigned ints */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::UnsignedInt)
            continue;

        if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedInts():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseInts(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseInts(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           ints */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::Int)
            continue;

        if(!parseIntInternal("Utility::Json::parseInts():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseUnsignedLongs(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseUnsignedLongs(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           unsigned longs */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::UnsignedLong)
            continue;

        if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLongs():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseLongs(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseLongs(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-number tokens or tokens that are already parsed as
           longs */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::Number ||
           nestedToken.parsedType() == JsonToken::ParsedType::Long)
            continue;

        if(!parseLongInternal("Utility::Json::parseLongs():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseSizes(const JsonToken& token) {
    #ifndef CORRADE_TARGET_32BIT
    return parseUnsignedLongs(token);
    #else
    return parseUnsignedInts(token);
    #endif
}

bool Json::parseStringKeys(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseStringKeys(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-string tokens, string tokens that are not keys or string
           tokens that are already parsed */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::String ||
            #ifndef CORRADE_TARGET_32BIT
            !(nestedToken._sizeFlagsParsedTypeType & JsonToken::FlagStringKey) ||
             (nestedToken._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask)
            #else
            !(nestedToken._childCountFlagsTypeNan & JsonToken::FlagStringKey) ||
             (nestedToken._childCountFlagsTypeNan & JsonToken::FlagParsed)
            #endif
        )
            continue;

        if(!parseStringInternal("Utility::Json::parseStringKeys():", nestedToken))
            return false;
    }

    return true;
}

bool Json::parseStrings(const JsonToken& token) {
    /* Get an index into the array */
    const std::size_t tokenIndex = &token - _state->tokens;
    CORRADE_ASSERT(tokenIndex < _state->tokens.size(),
        "Utility::Json::parseStrings(): token not owned by the instance", {});

    for(std::size_t i = tokenIndex, max = tokenIndex + 1 + token.childCount(); i != max; ++i) {
        /* Skip non-string tokens or string tokens that are already parsed */
        JsonToken& nestedToken = _state->tokens[i];
        if(nestedToken.type() != JsonToken::Type::String ||
            #ifndef CORRADE_TARGET_32BIT
            (nestedToken._sizeFlagsParsedTypeType & JsonToken::ParsedTypeMask)
            #else
            (nestedToken._childCountFlagsTypeNan & JsonToken::FlagParsed)
            #endif
        )
            continue;

        if(!parseStringInternal("Utility::Json::parseStrings():", nestedToken))
            return false;
    }

    return true;
}

Containers::Optional<JsonView<JsonObjectItem>> Json::parseObject(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseObject(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Object) {
        Error err;
        err << "Utility::Json::parseObject(): expected an object, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));

    const std::size_t childCount = token.childCount();
    for(JsonToken *i = const_cast<JsonToken*>(&token) + 1, *iMax = i + childCount; i != iMax; i = const_cast<JsonToken*>(i->next())) {
        if(!parseStringInternal("Utility::Json::parseObject():", *i)) return {};
    }

    return JsonView<JsonObjectItem>{&token + 1, &token + 1 + childCount};
}

Containers::Optional<JsonView<JsonArrayItem>> Json::parseArray(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    return JsonView<JsonArrayItem>{&token + 1, &token + 1 + token.childCount()};
}

Containers::Optional<std::nullptr_t> Json::parseNull(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseNull(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Null) {
        Error err;
        err << "Utility::Json::parseNull(): expected a null, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseNullInternal("Utility::Json::parseNull():", const_cast<JsonToken&>(token)))
        return {};
    return nullptr;
}

Containers::Optional<bool> Json::parseBool(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseBool(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Bool) {
        Error err;
        err << "Utility::Json::parseBool(): expected a bool, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseBoolInternal("Utility::Json::parseBool():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedBool;
}

Containers::Optional<double> Json::parseDouble(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseDouble(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseDouble(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseDoubleInternal("Utility::Json::parseDouble():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedDouble;
}

Containers::Optional<float> Json::parseFloat(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseFloat(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseFloat(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseFloatInternal("Utility::Json::parseFloat():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedFloat;
}

Containers::Optional<std::uint32_t> Json::parseUnsignedInt(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseUnsignedInt(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseUnsignedInt(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedInt():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedUnsignedInt;
}

Containers::Optional<std::int32_t> Json::parseInt(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseInt(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseInt(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseIntInternal("Utility::Json::parseInt():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedInt;
}

Containers::Optional<std::uint64_t> Json::parseUnsignedLong(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseUnsignedLong(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseUnsignedLong(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLong():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedUnsignedLong;
}

Containers::Optional<std::int64_t> Json::parseLong(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseLong(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseLong(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseLongInternal("Utility::Json::parseLong():", const_cast<JsonToken&>(token)))
        return {};
    return token._parsedLong;
}

Containers::Optional<std::size_t> Json::parseSize(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseSize(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Number) {
        Error err;
        err << "Utility::Json::parseSize(): expected a number, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!
        #ifndef CORRADE_TARGET_32BIT
        parseUnsignedLongInternal
        #else
        parseUnsignedIntInternal
        #endif
        ("Utility::Json::parseSize():", const_cast<JsonToken&>(token))
    )
        return {};

    #ifndef CORRADE_TARGET_32BIT
    return token._parsedUnsignedLong;
    #else
    return token._parsedUnsignedInt;
    #endif
}

Containers::Optional<Containers::StringView> Json::parseString(const JsonToken& token) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseString(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::String) {
        Error err;
        err << "Utility::Json::parseString(): expected a string, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }
    if(!parseStringInternal("Utility::Json::parseString():", const_cast<JsonToken&>(token)))
        return {};

    /* If the string is not escaped, reference it directly */
    if(
        #ifndef CORRADE_TARGET_32BIT
        !(token._sizeFlagsParsedTypeType & JsonToken::FlagStringEscaped)
        #else
        !(token._childCountFlagsTypeNan & JsonToken::FlagStringEscaped)
        #endif
    )
        return Containers::StringView{token._data + 1,
            #ifndef CORRADE_TARGET_32BIT
            (token._sizeFlagsParsedTypeType & JsonToken::SizeMask)
            #else
            token._sizeParsedType
            #endif
                - 2,
            #ifndef CORRADE_TARGET_32BIT
            token._sizeFlagsParsedTypeType & JsonToken::FlagStringGlobal ?
            #else
            token._childCountFlagsTypeNan & JsonToken::FlagStringGlobal ?
            #endif
                Containers::StringViewFlag::Global : Containers::StringViewFlags{}
        };

    /* Otherwise take the cached version */
    return Containers::StringView{*token._parsedString};
}

Containers::Optional<Containers::StridedArrayView1D<const bool>> Json::parseBoolArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseBoolArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseBoolArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Bool) {
            Error err;
            err << "Utility::Json::parseBoolArray(): expected a bool, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseBoolInternal("Utility::Json::parseBoolArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseBoolArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedBool);
}

Containers::Optional<Containers::StridedArrayView1D<const double>> Json::parseDoubleArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseDoubleArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseDoubleArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseDoubleArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseDoubleInternal("Utility::Json::parseDoubleArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseDoubleArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedDouble);
}

Containers::Optional<Containers::StridedArrayView1D<const float>> Json::parseFloatArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseFloatArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseFloatArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseFloatArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseFloatInternal("Utility::Json::parseFloatArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseFloatArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedFloat);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> Json::parseUnsignedIntArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseUnsignedIntArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseUnsignedIntArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseUnsignedIntArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseUnsignedIntInternal("Utility::Json::parseUnsignedIntArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseUnsignedIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedUnsignedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int32_t>> Json::parseIntArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseIntArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseIntArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseIntArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseIntInternal("Utility::Json::parseIntArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> Json::parseUnsignedLongArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseUnsignedLongArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseUnsignedLongArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseUnsignedLongArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseUnsignedLongInternal("Utility::Json::parseUnsignedLongArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseUnsignedLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedUnsignedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int64_t>> Json::parseLongArray(const JsonToken& token, const std::size_t expectedSize) {
    CORRADE_ASSERT(std::size_t(&token - _state->tokens) < _state->tokens.size(),
        "Utility::Json::parseLongArray(): token not owned by the instance", {});

    if(token.type() != JsonToken::Type::Array) {
        Error err;
        err << "Utility::Json::parseLongArray(): expected an array, got" << token.type() << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    parseObjectArrayInternal(const_cast<JsonToken&>(token));
    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        token._childCount
        #else
        token._childCountFlagsTypeNan & JsonToken::ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = &token + 1, *end = &token + 1 + size; i != end; ++i) {
        if(i->type() != JsonToken::Type::Number) {
            Error err;
            err << "Utility::Json::parseLongArray(): expected a number, got" << i->type() << "at";
            printFilePosition(err, _state->string.prefix(i->_data));
            return {};
        }

        if(!parseLongInternal("Utility::Json::parseLongArray():", const_cast<JsonToken&>(*i)))
            return {};
    }

    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    if(expectedSize && size != expectedSize) {
        Error err;
        err << "Utility::Json::parseLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size << "at";
        printFilePosition(err, _state->string.prefix(token._data));
        return {};
    }

    return Containers::stridedArrayView(&token + 1, size).slice(&JsonToken::_parsedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::size_t>> Json::parseSizeArray(const JsonToken& token, const std::size_t expectedSize) {
    #ifndef CORRADE_TARGET_32BIT
    const Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> out = parseUnsignedLongArray(token, expectedSize);
    #else
    const Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> out = parseUnsignedIntArray(token, expectedSize);
    #endif
    if(!out) return {};

    return Containers::arrayCast<const std::size_t>(*out);
}

Containers::StringView JsonToken::data() const {
    /* This could technically be made to preserve the Global flag, but on 32bit
       it would mean it'd have to be stored in two places -- either in the NaN
       bit pattern for object/array/string/literal and unparsed numeric tokens
       or in the upper bits of size for parsed numeric tokens (as the whole 64
       bits may be used by the stored number). But not just that, the flag
       would also have to be transferred from the NaN pattern to the size when
       parsing the numeric value for the first time, and *not* transferred from
       there if the numeric value is already parsed (since the bit there would
       likely mean for something else). And that's just too much logic and
       testing effort for something with a doubtful usefulness (compared to
       preserving the flag for asString()), so it's not done. */
    #ifndef CORRADE_TARGET_32BIT
    return {_data, _sizeFlagsParsedTypeType & SizeMask};
    #else
    /* If NaN is set and sign is 0, the full size is used */
    if((_childCountFlagsTypeNan & (NanMask|SignMask)) == NanMask)
        return {_data, _sizeParsedType};
    /* Otherwise it's likely small and the top is repurposed */
    return {_data, _sizeParsedType & SizeMask};
    #endif
}

std::size_t JsonToken::childCount() const {
    #ifndef CORRADE_TARGET_32BIT
    /* Objects and arrays store child count directly */
    if((_sizeFlagsParsedTypeType & TypeMask) == TypeObject ||
       (_sizeFlagsParsedTypeType & TypeMask) == TypeArray) {
        return _childCount;
    /* String keys have implicitly grandchild count + 1, where the
       grandchildren can be either objects and arays or value types with no
       children. Keys can't have keys as children, so this doesn't recurse. */
    } else if(_sizeFlagsParsedTypeType & FlagStringKey) {
        const JsonToken& child = *(this + 1);
        return ((child._sizeFlagsParsedTypeType & TypeMask) == TypeObject ||
                (child._sizeFlagsParsedTypeType & TypeMask) == TypeArray ?
            child._childCount : 0) + 1;
    /* Otherwise value types have no children */
    } else return 0;
    #else
    /* If NaN is set and sign is 0, the child count is stored for objects and
       arrays, implicit as grandchild count + 1 for string keys (where we have
       to again branch on a NaN), and 0 otherwise */
    if((_childCountFlagsTypeNan & (NanMask|SignMask)) == NanMask) {
        if((_childCountFlagsTypeNan & TypeMask) == TypeObject ||
           (_childCountFlagsTypeNan & TypeMask) == TypeArray) {
            return _childCountFlagsTypeNan & ChildCountMask;
        } else if(_childCountFlagsTypeNan & FlagStringKey) {
            const JsonToken& child = *(this + 1);
            return ((child._childCountFlagsTypeNan & (NanMask|SignMask)) == NanMask &&
                    ((child._childCountFlagsTypeNan & TypeMask) == TypeObject ||
                     (child._childCountFlagsTypeNan & TypeMask) == TypeArray) ?
                child._childCountFlagsTypeNan & ChildCountMask : 0) + 1;
        } else return 0;
    /* Otherwise it's a numeric value and that has no children */
    } else return 0;
    #endif
}

Containers::ArrayView<const JsonToken> JsonToken::children() const {
    return {this + 1, childCount()};
};

const JsonToken* JsonToken::parent() const {
    /* Traverse backwards until a token that spans over this one is found, or
       until we reach the sentinel */
    const JsonToken* prev = this - 1;
    while(prev->_data && prev + prev->childCount() < this)
        --prev;
    return prev->_data ? prev : nullptr;
}

JsonView<JsonObjectItem> JsonToken::asObject() const {
    CORRADE_ASSERT(type() == Type::Object && isParsed(),
        "Utility::JsonToken::asObject(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(),
        (JsonView<JsonObjectItem>{this + 1, this + 1}));

    return JsonView<JsonObjectItem>{this + 1, this + 1 +
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        (_childCountFlagsTypeNan & ChildCountMask)
        #endif
        };
}

JsonView<JsonArrayItem> JsonToken::asArray() const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(),
        (JsonView<JsonArrayItem>{this + 1, this + 1}));

    return JsonView<JsonArrayItem>{this + 1, this + 1 +
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        (_childCountFlagsTypeNan & ChildCountMask)
        #endif
        };
}

const JsonToken* JsonToken::find(const Containers::StringView key) const {
    CORRADE_ASSERT(type() == Type::Object && isParsed(),
        "Utility::JsonToken::find(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type() << Debug::nospace << ", expected a parsed object", this);

    for(const JsonToken *i = this + 1, *end = this + 1 +
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        (_childCountFlagsTypeNan & ChildCountMask)
        #endif
    ; i != end; i = i->next()) {
        CORRADE_ASSERT(i->isParsed(), "Utility::JsonToken::find(): key string isn't parsed", this);
        /** @todo asStringInternal() to avoid the nested assert? */
        if(i->asString() == key) return i->firstChild();
    }

    return nullptr;
}

const JsonToken& JsonToken::operator[](const Containers::StringView key) const {
    const JsonToken* found = find(key);
    CORRADE_ASSERT(found, "Utility::JsonToken::operator[](): key" << key << "not found", *this);
    return *found;
}

const JsonToken* JsonToken::find(const std::size_t index) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::find(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type() << Debug::nospace << ", expected a parsed array", this);

    std::size_t counter = 0;
    for(const JsonToken *i = this + 1, *end = this + 1 +
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        (_childCountFlagsTypeNan & ChildCountMask)
        #endif
    ; i != end; i = i->next())
        if(counter++ == index) return i;

    return nullptr;
}

const JsonToken& JsonToken::operator[](const std::size_t index) const {
    const JsonToken* found = find(index);
    /** @todo something better like "index N out of bounds for M elements",
        would need an internal helper or some such to get the counter value */
    CORRADE_ASSERT(found, "Utility::JsonToken::operator[](): index" << index << "not found", *this);
    return *found;
}

Containers::StringView JsonToken::asString() const {
    CORRADE_ASSERT(type() == Type::String && isParsed(),
        "Utility::JsonToken::asString(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    /* If the string is not escaped, reference it directly */
    if(
        #ifndef CORRADE_TARGET_32BIT
        !(_sizeFlagsParsedTypeType & FlagStringEscaped)
        #else
        !(_childCountFlagsTypeNan & FlagStringEscaped)
        #endif
    )
        return {_data + 1,
            #ifndef CORRADE_TARGET_32BIT
            (_sizeFlagsParsedTypeType & SizeMask)
            #else
            _sizeParsedType
            #endif
                - 2,
            #ifndef CORRADE_TARGET_32BIT
            _sizeFlagsParsedTypeType & FlagStringGlobal ?
            #else
            _childCountFlagsTypeNan & FlagStringGlobal ?
            #endif
                Containers::StringViewFlag::Global : Containers::StringViewFlags{}
        };

    /* Otherwise take the cached version */
    return *_parsedString;
}

Containers::StridedArrayView1D<const bool> JsonToken::asBoolArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asBoolArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->type() == Type::Bool && i->isParsed(),
            "Utility::JsonToken::asBoolArray(): token" << i - this - 1 << "is" << (i->isParsed() ? "a parsed" : "an unparsed") << i->type(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asBoolArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedBool);
}

Containers::StridedArrayView1D<const double> JsonToken::asDoubleArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asDoubleArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::Double,
            "Utility::JsonToken::asDoubleArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asDoubleArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedDouble);
}

Containers::StridedArrayView1D<const float> JsonToken::asFloatArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asFloatArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::Float,
            "Utility::JsonToken::asFloatArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asFloatArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedFloat);
}

Containers::StridedArrayView1D<const std::uint32_t> JsonToken::asUnsignedIntArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asUnsignedIntArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::UnsignedInt,
            "Utility::JsonToken::asUnsignedIntArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asUnsignedIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedUnsignedInt);
}

Containers::StridedArrayView1D<const std::int32_t> JsonToken::asIntArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asIntArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::Int,
            "Utility::JsonToken::asIntArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asIntArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedInt);
}

Containers::StridedArrayView1D<const std::uint64_t> JsonToken::asUnsignedLongArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asUnsignedLongArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::UnsignedLong,
            "Utility::JsonToken::asUnsignedLongArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asUnsignedLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedUnsignedLong);
}

Containers::StridedArrayView1D<const std::int64_t> JsonToken::asLongArray(const std::size_t expectedSize) const {
    CORRADE_ASSERT(type() == Type::Array && isParsed(),
        "Utility::JsonToken::asLongArray(): token is" << (isParsed() ? "a parsed" : "an unparsed") << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    #ifndef CORRADE_NO_ASSERT
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        CORRADE_ASSERT(i->parsedType() == ParsedType::Long,
            "Utility::JsonToken::asLongArray(): token" << i - this - 1 << "is a" << i->type() << "parsed as" << i->parsedType(), {});
    /* Needs to be after the type-checking loop, otherwise the child count may
       include also nested tokens and the message would be confusing */
    CORRADE_ASSERT(!expectedSize || size == expectedSize,
        "Utility::JsonToken::asLongArray(): expected a" << expectedSize << Debug::nospace << "-element array, got" << size, {});
    #else
    static_cast<void>(expectedSize);
    #endif

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedLong);
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

Containers::StringView JsonObjectItem::key() const {
    CORRADE_ASSERT(_token->isParsed(),
        "Utility::JsonObjectItem::key(): string isn't parsed", {});
    /** @todo asStringInternal() to avoid the nested assert? */
    return _token->asString();
}

Utility::Debug& operator<<(Utility::Debug& debug, const JsonToken::Type value) {
    debug << "Utility::JsonToken::Type" << Utility::Debug::nospace;

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

    return debug << "(" << Utility::Debug::nospace << reinterpret_cast<void*>(std::uint64_t(value)) << Utility::Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, const JsonToken::ParsedType value) {
    debug << "Utility::JsonToken::ParsedType" << Utility::Debug::nospace;

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

    return debug << "(" << Utility::Debug::nospace << reinterpret_cast<void*>(std::uint64_t(value)) << Utility::Debug::nospace << ")";
}

}}
