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

#include "JsonWriter.h"

#include <cmath> /* std::isinf(), std::isnan() */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Utility/Format.h" /* numeric JsonWriter::writeValue() */
#include "Corrade/Utility/Macros.h" /* CORRADE_FALLTHROUGH */
#include "Corrade/Utility/Path.h"

namespace Corrade { namespace Utility {

using namespace Containers::Literals;

namespace {

constexpr const char* ExpectingString[]{
    "a value",
    "an array value or array end",
    "an object key or object end",
    "an object value",
    "document end"
};

enum class Expecting {
    Value,
    ArrayValueOrArrayEnd,
    ObjectKeyOrEnd,
    ObjectValue,
    DocumentEnd
};

/* See State::levels for details */
enum: std::uint32_t {
    LevelPrefixMask = 0x7fffffffu,
    LevelIsArray = 0x80000000u
};

constexpr const char* EightSpaces = "        ";
constexpr const char* ColonAndSpace = ": ";
constexpr const char* CommaAndSpace = ", ";
constexpr const char* FinalNewline = "\n";

}

struct JsonWriter::State {
    Expecting expecting;

    /* Contains spaces needed for a single indetation level */
    /** @todo use a StringView once growable */
    Containers::ArrayView<const char> indentation;
    /* Contains a colon or comma and a following space if needed */
    Containers::ArrayView<const char> colonAndSpace, commaAndSpace;
    /* Contains the final newline at document end if needed */
    Containers::ArrayView<const char> finalNewlineNull;

    /* Output string */
    /** @todo use a String once it's growable */
    Containers::Array<char> out;
    /* Contains all whitespace ever needed to indent anything. If Option::Wrap
       is set, the first byte is a \n, follows a number of spaces based on
       indentation. If the option is not set, this string is empty. */
    /** @todo use a String once it's growable */
    Containers::Array<char> whitespace;
    /* A stack of prefix lengths into the whitespace string above. If it
       contains just a single value, we're at the top level. Prefix sizes
       masked with LevelPrefixMask, leftmost bit of second and following values
       indicates if given level is an object or array (LevelIsArray). */
    Containers::Array<std::uint32_t> levels;
    /* Indicates that a first value at given level is being written, thus no
       comma before. Gets reset in beginObject(), beginArray() and
       writeObjectKey(), gets set right after a value gets written. */
    bool needsCommaBefore = false;
};

JsonWriter::JsonWriter(const Options options, const std::uint32_t indentation):
    /* Since a Writer instance is write-only, it isn't expected to be used as a
       placeholder the same way as, say, an empty String. So we shouldn't need
       to optimize for an overhead-free or noexcept default construction. */
    _state{InPlaceInit}
{
    CORRADE_ASSERT(indentation <= 8,
        "Utility::JsonWriter: indentation can be at most 8 characters, got" << indentation, );

    _state->expecting = Expecting::Value;

    /* Initialize the indentation and after-colon spacing strings */
    _state->indentation = {EightSpaces, options & Option::Wrap ? indentation : 0};
    _state->colonAndSpace = {ColonAndSpace, options & Option::TypographicalSpace ? 2u : 1u};

    /* If we're wrapping, initialize the whitespace string with a single
       newline, and a newline + \0 to put at document end. Spaces will get
       added to it as we dive deeper. If we're not, then there's a space after
       every comma instead, and no final newline, only the \0. */
    if(options & Option::Wrap) {
        arrayAppend(_state->whitespace, '\n');
        _state->commaAndSpace = {CommaAndSpace, 1};
        _state->finalNewlineNull = {FinalNewline, 2};
    } else {
        _state->commaAndSpace = {CommaAndSpace, options & Option::TypographicalSpace ? 2u : 1u};
        _state->finalNewlineNull = {FinalNewline + 1, 1};
    }

    /* Initialize the whitespace prefix stack with a root value. Once the size
       of the levels array becomes 1 again, we're at the document end. */
    arrayAppend(_state->levels, _state->whitespace.size());
}

JsonWriter::JsonWriter(JsonWriter&&) noexcept = default;

JsonWriter::~JsonWriter() = default;

JsonWriter& JsonWriter::operator=(JsonWriter&&) noexcept = default;

void JsonWriter::writeCommaNewlineIndentInternal() {
    State& state = *_state;

    /* If this is the root JSON value being written, nothing to do here. Same
       in case an object value is expected. */
    if(state.levels.size() == 1 || state.expecting == Expecting::ObjectValue)
        return;

    /* Comma after previous value */
    if(state.needsCommaBefore) arrayAppend(state.out, state.commaAndSpace);

    /* Newline and indent */
    /** @todo F.F.S. what's up with the crazy casts */
    arrayAppend(state.out, Containers::ArrayView<const char>{state.whitespace.prefix(state.levels.back() & LevelPrefixMask)});
}

void JsonWriter::finalizeDocument() {
    State& state = *_state;

    /* Add a \n if we're wrapping and a \0. We can't arrayRemoveSuffix() it
       to make it sentinel because that would trigger ASAN complaint when
       it gets accessed, so instead the size has to be patched in toString()
       and toFile(). */
    /** @todo drop workarounds once growable String exists */
    arrayAppend(state.out, state.finalNewlineNull);

    /* Not expecting any more JSON crap after this point */
    state.expecting = Expecting::DocumentEnd;
}

bool JsonWriter::isEmpty() const {
    /* No need for any explicit handling for terminating \0 like in size() here
       -- the array is empty initially (and toString() would fail because the
       value isn't complete, thus we don't need a sentinel \0 in that case) */
    /** @todo drop this comment once growable String is used */
    return _state->out.isEmpty();
}

std::size_t JsonWriter::size() const {
    const State& state = *_state;

    /* Due to the lack of growable Strings, once finalizeDocument() is called
       the array contains also the terminating null character, so strip it
       away. Otherwise not. */
    /** @todo drop this workaround once growable String is used */
    return state.out.size() - (state.expecting == Expecting::DocumentEnd ? 1 : 0);
}

JsonWriter& JsonWriter::beginObject() {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::beginObject(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent, object opening brace */
    writeCommaNewlineIndentInternal();
    arrayAppend(state.out, '{');

    /* Indent next level further */
    if(arrayAppend(state.levels, (state.levels.back() & LevelPrefixMask) + state.indentation.size()) > state.whitespace.size())
        arrayAppend(state.whitespace, state.indentation);

    /* Next expectig an object key or end */
    state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = false;

    return *this;
}

JsonWriter& JsonWriter::endObject() {
    State& state = *_state;
    CORRADE_ASSERT(state.expecting == Expecting::ObjectKeyOrEnd,
        "Utility::JsonWriter::endObject(): expected" << ExpectingString[int(state.expecting)], *this);

    /* One nesting level back. There's at least one level, guarded by
       state.expecting above. */
    arrayRemoveSuffix(state.levels, 1);

    /* If a comma is expected it means a value was written. Add a newline and
       an indent in that case, otherwise nothing. */
    if(state.needsCommaBefore)
        /** @todo F.F.S. what's up with the crazy casts */
        arrayAppend(state.out, Containers::ArrayView<const char>{state.whitespace.prefix(state.levels.back() & LevelPrefixMask)});

    /* Object closing brace */
    arrayAppend(state.out, '}');

    /* If we're back at the root, finalize the document. Otherwise expect
       either an array value or an object key depending on where we ended
       up. */
    if(state.levels.size() == 1)
        finalizeDocument();
    else if(state.levels.back() & LevelIsArray)
        state.expecting = Expecting::ArrayValueOrArrayEnd;
    else
        state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = true;

    return *this;
}

JsonWriter& JsonWriter::beginArray() {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::beginArray(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent, array opening brace */
    writeCommaNewlineIndentInternal();
    arrayAppend(state.out, '[');

    /* Indent next level further */
    if(arrayAppend(state.levels, LevelIsArray|((state.levels.back() & LevelPrefixMask) + state.indentation.size())) > state.whitespace.size())
        arrayAppend(state.whitespace, state.indentation);

    /* Next expectig a value or end */
    state.expecting = Expecting::ArrayValueOrArrayEnd;
    state.needsCommaBefore = false;

    return *this;
}

JsonWriter& JsonWriter::endArray() {
    State& state = *_state;
    CORRADE_ASSERT(state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::endArray(): expected" << ExpectingString[int(state.expecting)], *this);

    /* One nesting level back. There's at least one level, guarded by
       state.expecting above. */
    arrayRemoveSuffix(state.levels, 1);

    /* If a comma is expected it means a value was written. Add a newline and
       an indent in that case, otherwise nothing. */
    if(state.needsCommaBefore)
        /** @todo F.F.S. what's up with the crazy casts */
        arrayAppend(state.out, Containers::ArrayView<const char>{state.whitespace.prefix(state.levels.back() & LevelPrefixMask)});

    /* Array closing brace */
    arrayAppend(state.out, ']');

    /* If we're back at the root, finalize the document. Otherwise expect
       either an array value or an object key depending on where we ended
       up. */
    if(state.levels.size() == 1)
        finalizeDocument();
    else if(state.levels.back() & LevelIsArray)
        state.expecting = Expecting::ArrayValueOrArrayEnd;
    else
        state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = true;

    return *this;
}

Containers::ScopeGuard JsonWriter::beginObjectScope() {
    beginObject();
    return Containers::ScopeGuard{this, [](JsonWriter* self) {
        self->endObject();
    }};
}

Containers::ScopeGuard JsonWriter::beginArrayScope() {
    beginArray();
    return Containers::ScopeGuard{this, [](JsonWriter* self) {
        self->endArray();
    }};
}

void JsonWriter::writeStringLiteralInternal(const Containers::StringView string) {
    State& state = *_state;
    /* Not checking state.expecting here, done by the caller */

    /* String opening quote */
    arrayAppend(state.out, '"');

    for(char c: string) switch(c) {
        case '\b':
            arrayAppend(state.out, Containers::arrayView({'\\', 'b'}));
            break;
        case '\f':
            arrayAppend(state.out, Containers::arrayView({'\\', 'f'}));
            break;
        case '\n':
            arrayAppend(state.out, Containers::arrayView({'\\', 'n'}));
            break;
        case '\t':
            arrayAppend(state.out, Containers::arrayView({'\\', 't'}));
            break;
        case '\r':
            arrayAppend(state.out, Containers::arrayView({'\\', 'r'}));
            break;
        case '"':
        case '\\':
        case '/':  /* JSON, why, you're weird */
            arrayAppend(state.out, '\\');
            CORRADE_FALLTHROUGH
        default:
            arrayAppend(state.out, c);
    }

    arrayAppend(state.out, '"');

    /* Not updating state.expecting here, done by the caller */
}

JsonWriter& JsonWriter::writeKey(const Containers::StringView key) {
    State& state = *_state;
    CORRADE_ASSERT(state.expecting == Expecting::ObjectKeyOrEnd,
        "Utility::JsonWriter::writeKey(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Key string literal */
    writeStringLiteralInternal(key);

    /* Colon */
    arrayAppend(state.out, state.colonAndSpace);

    /* Next expecting an object value (i.e., not indented, no comma) */
    state.expecting = Expecting::ObjectValue;

    return *this;
}

JsonWriter& JsonWriter::writeInternal(const Containers::StringView literal) {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::write(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Literal value */
    /** @todo F.F.S. what's up with the crazy casts */
    arrayAppend(state.out, Containers::ArrayView<const char>{literal});

    /* If we're at the root, finalize the document. Otherwise expect either an
       array value or an object key depending on where we are. */
    if(state.levels.size() == 1)
        finalizeDocument();
    else if(state.levels.back() & LevelIsArray)
        state.expecting = Expecting::ArrayValueOrArrayEnd;
    else
        state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = true;

    return *this;
}

JsonWriter& JsonWriter::write(std::nullptr_t) {
    return writeInternal("null"_s);
}

JsonWriter& JsonWriter::writeBoolInternal(const bool value) {
    return writeInternal(value ? "true"_s : "false"_s);
}

JsonWriter& JsonWriter::write(const float value) {
    CORRADE_ASSERT(!std::isnan(value) && !std::isinf(value),
        "Utility::JsonWriter::write(): invalid floating-point value" << value, *this);

    /* Should be sufficiently large for printing with 6 significant digits */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const double value) {
    CORRADE_ASSERT(!std::isnan(value) && !std::isinf(value),
        "Utility::JsonWriter::write(): invalid floating-point value" << value, *this);

    /* Should be sufficiently large for printing with 15 significant digits */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const std::uint32_t value) {
    /* Should be sufficiently large */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const std::int32_t value) {
    /* Should be sufficiently large */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const unsigned long long value) {
    CORRADE_ASSERT(value < 1ull << 52,
        "Utility::JsonWriter::write(): too large integer value" << value, *this);

    /* Should be sufficiently large */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const long long value) {
    CORRADE_ASSERT(value >= -(1ll << 52) && value < (1ll << 52),
        "Utility::JsonWriter::write(): too small or large integer value" << value, *this);

    /* Should be sufficiently large */
    /** @todo use the direct string conversion APIs once they exist */
    char buffer[127];
    return writeInternal({buffer, formatInto(buffer, "{}", value)});
}

JsonWriter& JsonWriter::write(const Containers::StringView value) {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::write(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    writeStringLiteralInternal(value);

    /* If we're at the root, finalize the document. Otherwise expect either an
       array value or an object key depending on where we are. */
    if(state.levels.size() == 1)
        finalizeDocument();
    else if(state.levels.back() & LevelIsArray)
        state.expecting = Expecting::ArrayValueOrArrayEnd;
    else
        state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = true;

    return *this;
}

JsonWriter& JsonWriter::writeJson(const Containers::StringView json) {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::writeJson(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Literal value */
    /** @todo F.F.S. what's up with the crazy casts */
    arrayAppend(state.out, Containers::ArrayView<const char>{json});

    /* If we're at the root, finalize the document. Otherwise expect either an
       array value or an object key depending on where we are. */
    if(state.levels.size() == 1)
        finalizeDocument();
    else if(state.levels.back() & LevelIsArray)
        state.expecting = Expecting::ArrayValueOrArrayEnd;
    else
        state.expecting = Expecting::ObjectKeyOrEnd;
    state.needsCommaBefore = true;

    return *this;
}

Containers::StringView JsonWriter::toString() const {
    const State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::DocumentEnd,
        "Utility::JsonWriter::toString(): incomplete JSON, expected" << ExpectingString[int(state.expecting)], {});

    /* The array contains a non-sentinel \0, strip it. See finalizeDocument()
       for more information. */
    /** @todo drop workarounds once growable String exists */
    return Containers::StringView{state.out, state.out.size() - 1, Containers::StringViewFlag::NullTerminated};
}

bool JsonWriter::toFile(const Containers::StringView filename) const {
    const State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::DocumentEnd,
        "Utility::JsonWriter::toFile(): incomplete JSON, expected" << ExpectingString[int(state.expecting)], {});

    /* The array contains a non-sentinel \0, strip it. See finalizeDocument()
       for more information. */
    /** @todo drop workarounds once growable String exists */
    if(!Utility::Path::write(filename, state.out.exceptSuffix(1))) {
        Error{} << "Utility::JsonWriter::toFile(): can't write to" << filename;
        return false;
    }

    return true;
}

}}
