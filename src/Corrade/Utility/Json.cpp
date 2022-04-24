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
    /* Used for line/column info in Json::parse*() error reporting */
    Containers::String filename;

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

void printFilePosition(Debug& out, const Containers::StringView filename, const Containers::StringView string) {
    std::size_t i = 0;
    std::size_t line = 1;
    std::size_t lastLineBegin = 0;
    for(; i != string.size(); ++i) {
        if(string[i] == '\n') {
            ++line;
            lastLineBegin = i + 1;
        }
    }

    /** @todo UTF-8 position instead */
    out << filename << Debug::nospace << ":" << Debug::nospace << line << Debug::nospace << ":" << Debug::nospace << (string.size() - lastLineBegin) + 1;
}

Containers::NullOptT printError(const Containers::StringView filename, Expecting expecting, const char offending, const Containers::StringView string) {
    Error err;
    err << ErrorPrefix << "expected" << ExpectingString[int(expecting)]
        << "but got" << Containers::StringView{&offending, 1} << "at";
    printFilePosition(err, filename, string);
    return Containers::NullOpt;
}

}

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const Containers::StringView string_) {
    Containers::Pointer<State> out{InPlaceInit};

    /* Make a copy of the input string if not marked as global */
    const std::uint64_t globalStringFlag = string_.flags() & Containers::StringViewFlag::Global ? std::uint64_t(JsonToken::FlagStringGlobal) : 0;
    if(globalStringFlag)
        out->string = string_;
    else
        out->string = out->storage = string_;

    /* Save also the filename for subsequent error reporting */
    out->filename = Containers::String::nullTerminatedGlobalView(filename);

    /* A sentinel token at the start, to limit Json::parent() */
    constexpr JsonToken sentinel{ValueInit};
    arrayAppend(out->tokens, sentinel);

    /* Go through the file byte by byte */
    const std::size_t size = out->string.size();
    const char* const data = out->string.data();
    /* Remember surrounding object or array token index to update its size,
       child count and check matching braces when encountering } / ] */
    std::size_t objectOrArrayTokenIndex = 0;
    /* Remember what token to expect next */
    Expecting expecting = Expecting::Value;
    /* Remember how many strings contain escape codes to allocate an immovable
       storage for them */
    std::size_t escapedStringCount = 0;
    for(std::size_t i = 0; i != size; ++i) {
        const char c = data[i];

        switch(c) {
            /* Object / array begin */
            case '{':
            case '[': {
                if(expecting != Expecting::ValueOrArrayEnd &&
                   expecting != Expecting::Value)
                    return printError(filename, expecting, c, out->string.prefix(i));

                /* Token holding the whole object / array */
                JsonToken token{NoInit};
                token._data = data + i;
                /* Size and child count gets filled in once } / ] is
                   encountered. Until then, abuse the _childCount field to
                   store the previous object / array index and remember this
                   index for when we get to } / ]. */
                #ifndef CORRADE_TARGET_32BIT
                token._sizeFlagsParsedTypeType =
                    JsonToken::ParsedTypeOther|
                    (c == '{' ? JsonToken::TypeObject : JsonToken::TypeArray);
                token._childCount = objectOrArrayTokenIndex;
                #else
                token._childCountFlagsTypeNan =
                    JsonToken::NanMask|JsonToken::FlagParsed|
                    (c == '{' ? JsonToken::TypeObject : JsonToken::TypeArray)|
                    objectOrArrayTokenIndex;
                #endif
                objectOrArrayTokenIndex = out->tokens.size();
                arrayAppend(out->tokens, token);

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
                    return printError(filename, expecting, c, out->string.prefix(i));

                /* Get the object / array token, check that the brace matches */
                JsonToken& token = out->tokens[objectOrArrayTokenIndex];
                const bool isObject = (token.
                    #ifndef CORRADE_TARGET_32BIT
                    _sizeFlagsParsedTypeType
                    #else
                    _childCountFlagsTypeNan
                    #endif
                    & JsonToken::TypeMask) == JsonToken::TypeObject;
                if((c == '}') != isObject) {
                    Error err;
                    err << ErrorPrefix << "unexpected" << out->string.slice(i, i + 1) << "at";
                    printFilePosition(err, filename, out->string.prefix(i));
                    err << "for an" << (c == ']' ? "object" : "array") << "starting at";
                    /* Printing the filename again, because it will make a
                       useful clickable link in terminal even though a bit
                       redundant */
                    printFilePosition(err, filename, out->string.prefix(token._data));
                    return {};
                }

                /* The child count field was abused to store the previous
                   object / array index. Restore it and set the actual child
                   count to the field. */
                const std::size_t tokenChildCount = out->tokens.size() - objectOrArrayTokenIndex - 1;
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
                    expecting = (out->tokens[objectOrArrayTokenIndex].
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
                    return printError(filename, expecting, c, out->string.prefix(i));

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
                            err << ErrorPrefix << "unexpected string escape sequence" << out->string.slice(i - 1, i + 1) << "at";
                            printFilePosition(err, filename, out->string.prefix(i - 1));
                            return {};
                        }
                    }
                }

                if(i == size) {
                    Error err;
                    err << ErrorPrefix << "file too short, unterminated string literal starting at";
                    printFilePosition(err, filename, out->string.prefix(start));
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
                        expecting = (out->tokens[objectOrArrayTokenIndex].
                            #ifndef CORRADE_TARGET_32BIT
                            _sizeFlagsParsedTypeType
                            #else
                            _childCountFlagsTypeNan
                            #endif
                            & JsonToken::TypeMask) == JsonToken::TypeObject ?
                            Expecting::CommaOrObjectEnd : Expecting::CommaOrArrayEnd;
                } else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */

                arrayAppend(out->tokens, token);

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
                    return printError(filename, expecting, c, out->string.prefix(i));

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

                arrayAppend(out->tokens, token);

                /* Expecting a comma or end next, depending on what the parent
                   is */
                if(!objectOrArrayTokenIndex)
                    expecting = Expecting::DocumentEnd;
                else
                    expecting = (out->tokens[objectOrArrayTokenIndex].
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
                    return printError(filename, expecting, c, out->string.prefix(i));

                /* Expecting a value next */
                expecting = Expecting::Value;
            } break;

            /* Comma after a value */
            case ',': {
                if(expecting != Expecting::CommaOrObjectEnd &&
                   expecting != Expecting::CommaOrArrayEnd)
                    return printError(filename, expecting, c, out->string.prefix(i));

                /* If we're in an object, expecting a key next, otherwise a
                   value next */
                expecting = (out->tokens[objectOrArrayTokenIndex].
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
                err << ErrorPrefix << "unexpected" << out->string.slice(i, i + 1) << "at";
                printFilePosition(err, filename, out->string.prefix(i));
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
        printFilePosition(err, filename, out->string);
        return {};
    }

    if(objectOrArrayTokenIndex != 0) {
        Error err;
        err << ErrorPrefix << "file too short, expected closing";
        const JsonToken& token = out->tokens[objectOrArrayTokenIndex];
        if(expecting == Expecting::CommaOrObjectEnd)
            err << "} for object";
        else if(expecting == Expecting::CommaOrArrayEnd)
            err << "] for array";
        else CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
        err << "starting at";
        printFilePosition(err, filename, out->string.prefix(token._data));
        return {};
    }

    /* Reserve memory for parsed string instances -- since the tokens reference
       them through a pointer, it has to be an immovable allocation */
    /** @todo use a non-reallocating allocator once it exists */
    arrayReserve(out->strings, escapedStringCount);

    /* All good */
    Json json;
    json._state = std::move(out);
    /* GCC 4.8 and Clang 3.8 need a bit of help here */
    return Containers::optional(std::move(json));
}

Containers::Optional<Json> Json::tokenize(const Containers::StringView filename, const Containers::StringView string, const Options options) {
    Containers::Optional<Json> out = tokenize(filename, string);
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

Containers::Optional<Json> Json::fromString(const Containers::StringView string) {
    return tokenize("<in>"_s, string);
}

Containers::Optional<Json> Json::fromString(const Containers::StringView string, const Options options) {
    return tokenize("<in>"_s, string, options);
}

Containers::Optional<Json> Json::fromFile(const Containers::StringView filename) {
    Containers::Optional<Containers::String> string = Path::readString(filename);
    if(!string) {
        Error{} << "Utility::Json::fromFile(): can't read" << filename;
        return {};
    }

    return tokenize(filename, *string);
}

Containers::Optional<Json> Json::fromFile(const Containers::StringView filename, const Options options) {
    Containers::Optional<Containers::String> string = Path::readString(filename);
    if(!string) {
        Error{} << "Utility::Json::fromFile(): can't read" << filename;
        return {};
    }

    return tokenize(filename, *string, options);
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

namespace {

bool parseNullInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string) {
    if(string == "null"_s) return true;

    Error{flag} << errorPrefix << "invalid null literal" << string;
    return false;
}

bool parseBoolInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, bool& out) {
    if(string == "true"_s) {
        out = true;
        return true;
    }
    if(string == "false"_s) {
        out = false;
        return true;
    }

    Error{flag} << errorPrefix << "invalid bool literal" << string;
    return false;
}

bool parseDoubleInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, double& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    out = std::strtod(buffer, &end);
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
        Error{flag} << errorPrefix << "invalid floating-point literal" << string;
        return false;
    }

    return true;
}

bool parseFloatInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, float& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    out = std::strtof(buffer, &end);
    if(std::size_t(end - buffer) != size) {
        Error{flag} << errorPrefix << "invalid floating-point literal" << string;
        return false;
    }

    return true;
}

bool parseUnsignedIntInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, std::uint32_t& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
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
        Error{flag} << errorPrefix << "invalid unsigned integer literal" << string;
        return false;
    }
    if(outLong > ~std::uint32_t{}) {
        Error{flag} << errorPrefix << "too large integer literal" << string;
        return false;
    }

    out = outLong;
    return true;
}

bool parseIntInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, std::int32_t& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
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
        Error{flag} << errorPrefix << "invalid integer literal" << string;
        return false;
    }
    if(outLong < INT_MIN || outLong > INT_MAX) {
        Error{flag} << errorPrefix << "too small or large integer literal" << string;
        return false;
    }

    out = outLong;
    return true;
}

bool parseUnsignedLongInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, std::uint64_t& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    out = std::strtoull(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error{flag} << errorPrefix << "invalid unsigned integer literal" << string;
        return false;
    }
    if(out >= 1ull << 52) {
        Error{flag} << errorPrefix << "too large integer literal" << string;
        return false;
    }

    return true;
}

bool parseLongInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, std::int64_t& out) {
    /** @todo replace with something that can parse non-null-terminated stuff,
        then drop this "too long" error */
    char buffer[128];
    const std::size_t size = string.size();
    if(size > Containers::arraySize(buffer) - 1) {
        Error{flag} << errorPrefix << "too long numeric literal" << string;
        return false;
    }

    std::memcpy(buffer, string.data(), size);
    buffer[size] = '\0';
    char* end;
    out = std::strtoll(buffer, &end, 10);
    if(std::size_t(end - buffer) != size) {
        Error{flag} << errorPrefix << "invalid integer literal" << string;
        return false;
    }
    if(out < -(1ll << 52) || out >= (1ll << 52)) {
        Error{flag} << errorPrefix << "too small or large integer literal" << string;
        return false;
    }

    return true;
}

bool parseStringInto(const char* const errorPrefix, const Debug::Flag flag, const Containers::StringView string, Containers::String& destination) {
    destination = Containers::String{NoInit, string.size()};

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
                Error{flag} << errorPrefix << "sorry, unicode escape sequences are not implemented yet";
                return false;
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
    return true;
}

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

        if(nestedToken.type() == JsonToken::Type::Null) {
            if(!parseNullInto("Utility::Json::parseLiterals():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data())) {
                Error err;
                err << " at";
                printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
                return false;
            }

        } else if(nestedToken.type() == JsonToken::Type::Bool) {
            if(!parseBoolInto("Utility::Json::parseLiterals():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), nestedToken._parsedBool)) {
                Error err;
                err << " at";
                printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
                return false;
            }
        } else continue;

        /* Mark the token as parsed */
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeOther;
        #else
        nestedToken._childCountFlagsTypeNan |= JsonToken::FlagParsed;
        #endif
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

        /* Not saving to token._parsedDouble directly to avoid a failure
           corrupting the high bits storing token type and flags on 32bit */
        double parsed;
        if(!parseDoubleInto("Utility::Json::parseDoubles():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), parsed)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* On success save the parsed value and its type. On 32bit the parsed
           type is stored in the size, the lack of a NaN implying that it's
           parsed. */
        nestedToken._parsedDouble = parsed;
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeDouble;
        #else
        CORRADE_INTERNAL_ASSERT((nestedToken._childCountFlagsTypeNan & JsonToken::NanMask) != JsonToken::NanMask);
        nestedToken._sizeParsedType = JsonToken::ParsedTypeDouble|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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

        if(!parseFloatInto("Utility::Json::parseFloats():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), nestedToken._parsedFloat)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* Save the parsed token type. On 32bit it's contained in the size,
           clear the NaN bits to imply that it's parsed. */
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeFloat;
        #else
        nestedToken._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
        nestedToken._sizeParsedType = JsonToken::ParsedTypeFloat|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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

        if(!parseUnsignedIntInto("Utility::Json::parseUnsignedInts():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), nestedToken._parsedUnsignedInt)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* Save the parsed token type. On 32bit it's contained in the size,
           clear the NaN bits to imply that it's parsed. */
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeUnsignedInt;
        #else
        nestedToken._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
        nestedToken._sizeParsedType = JsonToken::ParsedTypeUnsignedInt|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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

        if(!parseIntInto("Utility::Json::parseInts():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), nestedToken._parsedInt)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* Save the parsed token type. On 32bit it's contained in the size,
           clear the NaN bits to imply that it's parsed. */
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeInt;
        #else
        nestedToken._childCountFlagsTypeNan &= ~(JsonToken::NanMask|JsonToken::SignMask);
        nestedToken._sizeParsedType = JsonToken::ParsedTypeInt|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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

        /* Not saving to token._parsedUnsignedLong directly to avoid a failure
           corrupting the high bits storing token type and flags on 32bit */
        std::uint64_t parsed;
        if(!parseUnsignedLongInto("Utility::Json::parseUnsignedLongs():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), parsed)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* On success save the parsed value and its type. On 32bit the parsed
           type is stored in the size, the NaN and sign bits should be already
           all 0 for a 52-bit number. */
        nestedToken._parsedUnsignedLong = parsed;
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeUnsignedLong;
        #else
        CORRADE_INTERNAL_ASSERT((nestedToken._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == 0);
        nestedToken._sizeParsedType = JsonToken::ParsedTypeUnsignedLong|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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
           nestedToken.parsedType() == JsonToken::ParsedType::Int)
            continue;

        /* Not saving to token._parsedUnsignedLong directly to avoid a failure
           corrupting the high bits storing token type and flags on 32bit */
        std::int64_t parsed;
        if(!parseLongInto("Utility::Json::parseLongs():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), parsed)) {
            Error err;
            err << " at";
            printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
            return false;
        }

        /* On success save the parsed value and its type. On 32bit the parsed
           type is stored in the size, the NaN and sign bits should be already
           either all 0 for a positive 52-bit number or all 1 for a negative
           53-bit number. */
        nestedToken._parsedLong = parsed;
        #ifndef CORRADE_TARGET_32BIT
        nestedToken._sizeFlagsParsedTypeType =
            (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
            JsonToken::ParsedTypeLong;
        #else
        CORRADE_INTERNAL_ASSERT(
            (nestedToken._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == 0 ||
            (nestedToken._childCountFlagsTypeNan & (JsonToken::NanMask|JsonToken::SignMask)) == (JsonToken::NanMask|JsonToken::SignMask));
        nestedToken._sizeParsedType = JsonToken::ParsedTypeLong|
            (nestedToken._sizeParsedType & ~JsonToken::ParsedTypeMask);
        #endif
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

        /* If a token has no escapes, mark it as parsed. This is not done
           implicitly in order to force users to always explicitly call
           parseString*() before using the string values. */
        if(
            #ifndef CORRADE_TARGET_32BIT
            !(nestedToken._sizeFlagsParsedTypeType & JsonToken::FlagStringEscaped)
            #else
            !(nestedToken._childCountFlagsTypeNan & JsonToken::FlagStringEscaped)
            #endif
        ) {
            #ifndef CORRADE_TARGET_32BIT
            nestedToken._sizeFlagsParsedTypeType |= JsonToken::ParsedTypeOther;
            #else
            nestedToken._childCountFlagsTypeNan |= JsonToken::FlagParsed;
            #endif

        /* Otherwise parse it into a new entry in the cached string array. The
           array isn't meant to be reallocated as that would cause existing
           pointers to it to be invalidated. */
        } else {
            /** @todo use a non-reallocating allocator for more robustness once
                it exists */
            CORRADE_INTERNAL_ASSERT(_state->strings.size() < arrayCapacity(_state->strings));
            Containers::String& out = arrayAppend(_state->strings, InPlaceInit);

            if(!parseStringInto("Utility::Json::parseStringKeys():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), out)) {
                Error err;
                err << " at";
                printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
                return false;
            }

            nestedToken._parsedString = &out;
            #ifndef CORRADE_TARGET_32BIT
            nestedToken._sizeFlagsParsedTypeType =
                (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
                JsonToken::ParsedTypeOther;
            #else
            nestedToken._childCountFlagsTypeNan |= JsonToken::FlagParsed;
            #endif
        }
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

        /* If a token has no escapes, mark it as parsed. This is not done
           implicitly in order to force users to always explicitly call
           parseString*() before using the string values. */
        if(
            #ifndef CORRADE_TARGET_32BIT
            !(nestedToken._sizeFlagsParsedTypeType & JsonToken::FlagStringEscaped)
            #else
            !(nestedToken._childCountFlagsTypeNan & JsonToken::FlagStringEscaped)
            #endif
        ) {
            #ifndef CORRADE_TARGET_32BIT
            nestedToken._sizeFlagsParsedTypeType |= JsonToken::ParsedTypeOther;
            #else
            nestedToken._childCountFlagsTypeNan |= JsonToken::FlagParsed;
            #endif

        /* Otherwise parse it into a new entry in the cached string array. The
           array isn't meant to be reallocated as that would cause existing
           pointers to it to be invalidated. */
        } else {
            /** @todo use a non-reallocating allocator for more robustness once
                it exists */
            CORRADE_INTERNAL_ASSERT(_state->strings.size() < arrayCapacity(_state->strings));
            Containers::String& out = arrayAppend(_state->strings, InPlaceInit);

            if(!parseStringInto("Utility::Json::parseStrings():", Error::Flag::NoNewlineAtTheEnd, nestedToken.data(), out)) {
                Error err;
                err << " at";
                printFilePosition(err, _state->filename, _state->string.prefix(nestedToken._data));
                return false;
            }

            nestedToken._parsedString = &out;
            #ifndef CORRADE_TARGET_32BIT
            nestedToken._sizeFlagsParsedTypeType =
                (nestedToken._sizeFlagsParsedTypeType & ~JsonToken::ParsedTypeMask)|
                JsonToken::ParsedTypeOther;
            #else
            nestedToken._childCountFlagsTypeNan |= JsonToken::FlagParsed;
            #endif
        }
    }

    return true;
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
            return ((_childCountFlagsTypeNan & (NanMask|SignMask)) == NanMask &&
                    child._childCountFlagsTypeNan & (TypeObject|TypeArray) ?
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
    CORRADE_ASSERT(type() == Type::Object,
        "Utility::JsonToken::asObject(): token is a" << type(),
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
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asArray(): token is a" << type(),
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
    CORRADE_ASSERT(type() == Type::Object,
        "Utility::JsonToken::find(): token is a" << type() << Debug::nospace << ", not an object", this);

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
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::find(): token is a" << type() << Debug::nospace << ", not an array", this);

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

Containers::Optional<std::nullptr_t> JsonToken::parseNull() const {
    if(type() != Type::Null) return {};
    if(isParsed()) return nullptr;
    if(parseNullInto("Utility::JsonToken::parseNull():", {}, data()))
        return nullptr;
    return {};
}

Containers::Optional<bool> JsonToken::parseBool() const {
    if(type() != Type::Bool) return {};
    if(isParsed()) return _parsedBool;

    bool out;
    if(parseBoolInto("Utility::JsonToken::parseBool():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<double> JsonToken::parseDouble() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::Double) return _parsedDouble;

    double out;
    if(parseDoubleInto("Utility::JsonToken::parseDouble():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<float> JsonToken::parseFloat() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::Float) return _parsedFloat;

    float out;
    if(parseFloatInto("Utility::JsonToken::parseFloat():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<std::uint32_t> JsonToken::parseUnsignedInt() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::UnsignedInt) return _parsedUnsignedInt;

    std::uint32_t out;
    if(parseUnsignedIntInto("Utility::JsonToken::parseUnsignedInt():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<std::int32_t> JsonToken::parseInt() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::Int) return _parsedInt;

    std::int32_t out;
    if(parseIntInto("Utility::JsonToken::parseInt():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<std::uint64_t> JsonToken::parseUnsignedLong() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::UnsignedLong) return _parsedUnsignedLong;

    std::uint64_t out;
    if(parseUnsignedLongInto("Utility::JsonToken::parseUnsignedLong():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<std::int64_t> JsonToken::parseLong() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::Long) return _parsedLong;

    std::int64_t out;
    if(parseLongInto("Utility::JsonToken::parseLong():", {}, data(), out))
        return out;
    return {};
}

Containers::Optional<std::size_t> JsonToken::parseSize() const {
    if(type() != Type::Number) return {};
    if(parsedType() == ParsedType::Size)
        #ifndef CORRADE_TARGET_32BIT
        return _parsedUnsignedLong;
        #else
        return _parsedUnsignedInt;
        #endif

    std::size_t out;
    if(
        #ifndef CORRADE_TARGET_32BIT
        parseUnsignedLongInto
        #else
        parseUnsignedIntInto
        #endif
        ("Utility::JsonToken::parseSize():", {}, data(),
            /* Sometimes uint64_t is long long but size_t is long etc., so the
               cast is essential */
            #ifndef CORRADE_TARGET_32BIT
            reinterpret_cast<std::uint64_t&>(out)
            #else
            reinterpret_cast<std::uint32_t&>(out)
            #endif
        )
    )
        return out;
    return {};
}

Containers::Optional<Containers::String> JsonToken::parseString() const {
    if(type() != Type::String) return {};

    /* If the string is not escaped, we can copy it directly */
    if(
        #ifndef CORRADE_TARGET_32BIT
        !(_sizeFlagsParsedTypeType & JsonToken::FlagStringEscaped)
        #else
        !(_childCountFlagsTypeNan & JsonToken::FlagStringEscaped)
        #endif
    )
        return Containers::String{_data + 1,
            #ifndef CORRADE_TARGET_32BIT
            (_sizeFlagsParsedTypeType & SizeMask)
            #else
            _sizeParsedType
            #endif
            - 2};

    /* Otherwise, if it's already parsed, we can take the cached version */
    if(isParsed())
        return *_parsedString;

    /* Otherwise, parse from scratch */
    Containers::String out;
    if(parseStringInto("Utility::JsonToken::parseString():", {}, data(), out))
        return out;
    return {};
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

Containers::Optional<Containers::StridedArrayView1D<const bool>> JsonToken::asBoolArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asBoolArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the type() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->type() != Type::Bool || !i->isParsed()) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedBool);
}

Containers::Optional<Containers::StridedArrayView1D<const double>> JsonToken::asDoubleArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asDoubleArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::Double) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedDouble);
}

Containers::Optional<Containers::StridedArrayView1D<const float>> JsonToken::asFloatArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asFloatArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::Float) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedFloat);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> JsonToken::asUnsignedIntArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asUnsignedIntArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::UnsignedInt) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedUnsignedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int32_t>> JsonToken::asIntArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asIntArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::Int) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedInt);
}

Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> JsonToken::asUnsignedLongArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asUnsignedLongArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::UnsignedLong) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedUnsignedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::int64_t>> JsonToken::asLongArray() const {
    CORRADE_ASSERT(type() == Type::Array,
        "Utility::JsonToken::asLongArray(): token is a" << type(), {});

    const std::size_t size =
        #ifndef CORRADE_TARGET_32BIT
        _childCount
        #else
        _childCountFlagsTypeNan & ChildCountMask
        #endif
        ;
    /* As this is expected to be a value array, we go by simple incrementing
       instead of with i->next(). If a nested object or array would be
       encountered, the parsedType() check fails. */
    for(const JsonToken *i = this + 1, *end = this + 1 + size; i != end; ++i)
        if(i->parsedType() != ParsedType::Long) return {};

    return Containers::stridedArrayView(this + 1, size).slice(&JsonToken::_parsedLong);
}

Containers::Optional<Containers::StridedArrayView1D<const std::size_t>> JsonToken::asSizeArray() const {
    #ifndef CORRADE_TARGET_32BIT
    Containers::Optional<Containers::StridedArrayView1D<const std::uint64_t>> out = asUnsignedLongArray();
    #else
    Containers::Optional<Containers::StridedArrayView1D<const std::uint32_t>> out = asUnsignedIntArray();
    #endif
    if(!out) return {};
    return Containers::arrayCast<const std::size_t>(*out);
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
