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

#include "JsonWriter.h"

#include <cmath> /* std::isinf(), std::isnan() */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/StridedBitArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Utility/Format.h" /* numeric JsonWriter::write() */
#include "Corrade/Utility/Json.h"
#include "Corrade/Utility/Macros.h" /* CORRADE_FALLTHROUGH */
#include "Corrade/Utility/Path.h"

namespace Corrade { namespace Utility {

using namespace Containers::Literals;

namespace {

constexpr const char* ExpectingString[]{
    "a value",
    "an array value or array end",
    "a compact array value or array end",
    "an object key or object end",
    "an object value",
    "document end"
};

enum class Expecting {
    Value,
    ArrayValueOrArrayEnd,
    CompactArrayValueOrArrayEnd,
    ObjectKeyOrEnd,
    ObjectValue,
    DocumentEnd
};

constexpr const char* EightSpaces = "        ";
constexpr const char* ColonAndSpace = ": ";
constexpr const char* CommaAndSpace = ", ";
constexpr const char* FinalNewline = "\n";

}

struct JsonWriter::State {
    /* Contains spaces needed for a single indentation level */
    /** @todo use a StringView once growable */
    Containers::ArrayView<const char> indentation;
    /* Contains a colon or comma and a following space if needed */
    Containers::ArrayView<const char> colonAndSpace, commaAndSpace;
    /* Comma and a following space if needed in case of arrays written by
       writeArray() */
    Containers::ArrayView<const char> arrayCommaAndSpace;
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
    /* A stack of (<prefix length into the whitespace string above>, <array
       item count if an array, ~size_t{} otherwise>). If it contains just a
       single value, we're at the top level. */
    Containers::Array<Containers::Pair<std::size_t, std::size_t>> levels;

    Expecting expecting = Expecting::Value;
    /* Indicates that a first value at given level is being written, thus no
       comma before. Gets reset in beginObject(), beginArray() and
       writeObjectKey(), gets set right after a value gets written. */
    bool needsCommaBefore = false;
    /* After how many items to wrap inside a compact array. Used only if
       expecting is CompactArrayValueOrArrayEnd. */
    std::uint32_t compactArrayWrapAfter = 0;
};

JsonWriter::JsonWriter(const Options options, const std::uint32_t indentation, const std::uint32_t initialIndentation):
    /* Since a Writer instance is write-only, it isn't expected to be used as a
       placeholder the same way as, say, an empty String. So we shouldn't need
       to optimize for an overhead-free or noexcept default construction. */
    _state{InPlaceInit}
{
    CORRADE_ASSERT(indentation <= 8,
        "Utility::JsonWriter: indentation can be at most 8 characters, got" << indentation, );

    /* Initialize the indentation and after-colon spacing strings */
    _state->indentation = {EightSpaces, options & Option::Wrap ? indentation : 0};
    _state->colonAndSpace = {ColonAndSpace, options & Option::TypographicalSpace ? 2u : 1u};

    /* If we're wrapping, initialize the whitespace string with a single
       newline and the initial indentation, and a newline + \0 to put at
       document end. Spaces will get added to it as we dive deeper. If we're
       not, then there's a space after every comma instead, and no final
       newline, only the \0. */
    if(options & Option::Wrap) {
        arrayAppend(_state->whitespace, '\n');
        /** @todo arrayAppend(DirectInit, size, Args&&...) might be useful
            here, calling memset internally if possible */
        for(char& i: arrayAppend(_state->whitespace, NoInit, initialIndentation))
            i = ' ';

        _state->commaAndSpace = {CommaAndSpace, 1};
        _state->arrayCommaAndSpace = {CommaAndSpace, options & Option::TypographicalSpace ? 2u : 1u};

        /* If there's initial indentation, assume the output will be put into
           other JSON writers and thus a newline at the end isn't desired */
        if(!initialIndentation)
            _state->finalNewlineNull = {FinalNewline, 2};
        else
            _state->finalNewlineNull = {FinalNewline + 1, 1};
    } else {
        _state->commaAndSpace = {CommaAndSpace, options & Option::TypographicalSpace ? 2u : 1u};
        _state->arrayCommaAndSpace = {CommaAndSpace, options & Option::TypographicalSpace ? 2u : 1u};
        _state->finalNewlineNull = {FinalNewline + 1, 1};
    }

    /* Initialize the whitespace prefix stack with a root value. Once the size
       of the levels array becomes 1 again, we're at the document end. */
    arrayAppend(_state->levels, InPlaceInit, _state->whitespace.size(), ~std::size_t{});
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

    /* If we're inside a compact array and it's not time to wrap, add just a
       comma (and potential space after), nothing else to do. */
    if(state.expecting == Expecting::CompactArrayValueOrArrayEnd) {
        const std::size_t currentArraySize = _state->levels.back().second();
        CORRADE_INTERNAL_ASSERT(currentArraySize != ~std::size_t{});
        if(!state.compactArrayWrapAfter || currentArraySize % state.compactArrayWrapAfter != 0) {
            if(state.needsCommaBefore)
                arrayAppend(state.out, state.arrayCommaAndSpace);
            return;
        }
    }

    /* Comma after previous value */
    if(state.needsCommaBefore) arrayAppend(state.out, state.commaAndSpace);

    /* Newline and indent */
    arrayAppend(state.out, state.whitespace.prefix(state.levels.back().first()));
}

void JsonWriter::finalizeValue() {
    State& state = *_state;

    /* If we're at the root or got back to it after ending an object or array,
       finalize the document */
    if(state.levels.size() == 1) {
        /* Add a \n if we're wrapping and a \0. We can't arrayRemoveSuffix() it
           to make it sentinel because that would trigger ASAN complaint when
           it gets accessed, so instead the size has to be patched in
           toString() and toFile(). */
        /** @todo drop workarounds once growable String exists */
        arrayAppend(state.out, state.finalNewlineNull);

        /* Not expecting any more JSON after this point */
        state.expecting = Expecting::DocumentEnd;

    /* Otherwise expect either an array value or an object key depending on
       where we ended up. If it's an array value, increase the array size
       counter for the value we just wrote. */
    } else if(state.levels.back().second() != ~std::size_t{}) {
        ++state.levels.back().second();
        if(state.expecting != Expecting::CompactArrayValueOrArrayEnd)
            state.expecting = Expecting::ArrayValueOrArrayEnd;
        state.needsCommaBefore = true;
    } else {
        state.expecting = Expecting::ObjectKeyOrEnd;
        state.needsCommaBefore = true;
    }
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

    /* Indent next level further; mark this as an object */
    if(arrayAppend(state.levels, InPlaceInit, state.levels.back().first() + state.indentation.size(), ~std::size_t{}).first() > state.whitespace.size())
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
        arrayAppend(state.out, state.whitespace.prefix(state.levels.back().first()));

    /* Object closing brace */
    arrayAppend(state.out, '}');

    /* Decide what to expect next or finalize the document if the top level
       value got written */
    finalizeValue();

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

    /* Indent next level further; mark this as an array with 0 items so far */
    if(arrayAppend(state.levels, InPlaceInit, state.levels.back().first() + state.indentation.size(), 0u).first() > state.whitespace.size())
        arrayAppend(state.whitespace, state.indentation);

    /* Next expectig a value or end */
    state.expecting = Expecting::ArrayValueOrArrayEnd;
    state.needsCommaBefore = false;

    return *this;
}

JsonWriter& JsonWriter::beginCompactArray(const std::uint32_t wrapAfter) {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::beginCompactArray(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent, array opening brace */
    writeCommaNewlineIndentInternal();
    arrayAppend(state.out, '[');

    /* Indent next level further; mark this as an array with 0 items so far.
       If wrapAfter is 0, the next level indentation will never get used
       by writeCommaNewlineIndentInternal() however. */
    if(arrayAppend(state.levels, InPlaceInit, state.levels.back().first() + state.indentation.size(), 0u).first() > state.whitespace.size())
        arrayAppend(state.whitespace, state.indentation);

    /* Next expectig a compact value or end, remember how many values to wrap
       after */
    state.expecting = Expecting::CompactArrayValueOrArrayEnd;
    state.needsCommaBefore = false;
    state.compactArrayWrapAfter = wrapAfter;

    return *this;
}

JsonWriter& JsonWriter::endArray() {
    State& state = *_state;
    CORRADE_ASSERT(
        state.expecting == Expecting::ArrayValueOrArrayEnd ||
        state.expecting == Expecting::CompactArrayValueOrArrayEnd,
        "Utility::JsonWriter::endArray(): expected" << ExpectingString[int(state.expecting)], *this);

    /* One nesting level back. There's at least one level, guarded by
       state.expecting above. */
    arrayRemoveSuffix(state.levels, 1);

    /* Unless we're in a compact array without wrapping, if a comma is expected
       it means a value was written. Add a newline and an indent in that case,
       otherwise nothing so this becomes a []. */
    if((state.expecting != Expecting::CompactArrayValueOrArrayEnd || state.compactArrayWrapAfter) && state.needsCommaBefore)
        arrayAppend(state.out, state.whitespace.prefix(state.levels.back().first()));

    /* Array closing brace */
    arrayAppend(state.out, ']');

    /* Decide what to expect next or finalize the document if the top level
       value got written */
    finalizeValue();

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

Containers::ScopeGuard JsonWriter::beginCompactArrayScope(const std::uint32_t wrapAfter) {
    beginCompactArray(wrapAfter);
    return Containers::ScopeGuard{this, [](JsonWriter* self) {
        self->endArray();
    }};
}

std::size_t JsonWriter::currentArraySize() const {
    const std::size_t size = _state->levels.back().second();
    CORRADE_ASSERT(size != ~std::size_t{},
        "Utility::JsonWriter::currentArraySize(): not in an array", {});
    return size;
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
        /* Escaping / is possible but not required. The reason for this feature
           is to allow putting closing HTML tags (such as </marquee>) inside
           JSON which is then inside a <script>, and </ isn't allowed inside
           strings. https://stackoverflow.com/a/1580682 */
        /** @todo introduce an option to escape /, if ever needed in practice */
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
        state.expecting == Expecting::ArrayValueOrArrayEnd ||
        state.expecting == Expecting::CompactArrayValueOrArrayEnd,
        "Utility::JsonWriter::write(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Literal value */
    arrayAppend(state.out, literal);

    /* Decide what to expect next or finalize the document if the top level
       value got written */
    finalizeValue();

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
    #ifndef CORRADE_NO_ASSERT
    State& state = *_state;
    #endif
    /* Object key is *not* expected to prevent accidents where a missing key
       would mean the next (string) value is wrongly interpreted as a key
       instead of failing directly when writing a value without a key before */
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd ||
        state.expecting == Expecting::CompactArrayValueOrArrayEnd,
        "Utility::JsonWriter::write(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    writeStringLiteralInternal(value);

    /* Decide what to expect next or finalize the document if the top level
       value got written */
    finalizeValue();

    return *this;
}

template<class T> JsonWriter& JsonWriter::writeArrayInternal(const T& values, const std::uint32_t wrapAfter) {
    #ifndef CORRADE_NO_ASSERT
    State& state = *_state;
    #endif
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::writeArray(): expected" << ExpectingString[int(state.expecting)], *this);

    beginCompactArray(wrapAfter);

    for(std::size_t i = 0; i != values.size(); ++i)
        write(values[i]);

    endArray();

    return *this;
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedBitArrayView1D& values, const std::uint32_t wrapAfter) {
    #ifndef CORRADE_NO_ASSERT
    State& state = *_state;
    #endif
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::writeArray(): expected" << ExpectingString[int(state.expecting)], *this);

    beginCompactArray(wrapAfter);

    for(std::size_t i = 0; i != values.size(); ++i)
        write(values[i]);

    endArray();

    return *this;
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<bool> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::stridedArrayView(values).sliceBit(0), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const float>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<float> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const double>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<double> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const std::uint32_t>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<std::uint32_t> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const std::int32_t>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<std::int32_t> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const unsigned long long>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<unsigned long long> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const unsigned long>& values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayCast<const typename std::conditional<sizeof(unsigned long) == 8, unsigned long long, unsigned int>::type>(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<unsigned long> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const long long>& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<long long> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StridedArrayView1D<const long>& values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayCast<const typename std::conditional<sizeof(long) == 8, long long, int>::type>(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const std::initializer_list<long> values, const std::uint32_t wrapAfter) {
    return writeArray(Containers::arrayView(values), wrapAfter);
}

JsonWriter& JsonWriter::writeArray(const Containers::StringIterable& values, const std::uint32_t wrapAfter) {
    return writeArrayInternal(values, wrapAfter);
}

JsonWriter& JsonWriter::writeJson(const Containers::StringView json) {
    State& state = *_state;
    /* Object key is *not* expected for consistency with write() / writeKey(),
       writeJsonKey() is meant for keys instead */
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::writeJson(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Literal value */
    arrayAppend(state.out, json);

    /* Decide what to expect next or finalize the document if the top level
       value got written */
    finalizeValue();

    return *this;
}

JsonWriter& JsonWriter::writeJsonKey(const Containers::StringView json) {
    State& state = *_state;
    CORRADE_ASSERT(state.expecting == Expecting::ObjectKeyOrEnd,
        "Utility::JsonWriter::writeJsonKey(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Comma, newline and indent */
    writeCommaNewlineIndentInternal();

    /* Literal value */
    arrayAppend(state.out, json);

    /* Colon */
    arrayAppend(state.out, state.colonAndSpace);

    /* Next expecting an object value (i.e., not indented, no comma) */
    state.expecting = Expecting::ObjectValue;

    return *this;
}

JsonWriter& JsonWriter::writeJson(const JsonToken json) {
    State& state = *_state;
    /* Object key is *not* expected for consistency with the StringView
       overload. There's also no writeJsonKey() for JsonToken, because it's
       so far unclear whether such a token should be processed including its
       children (and thus writing its value as well) or as just a key. Might
       loosen up the requirements once a practical use case emerges. */
    CORRADE_ASSERT(
        state.expecting == Expecting::Value ||
        state.expecting == Expecting::ObjectValue ||
        state.expecting == Expecting::ArrayValueOrArrayEnd,
        "Utility::JsonWriter::writeJson(): expected" << ExpectingString[int(state.expecting)], *this);

    /* Complementary to the above, if the token is a string, it should be a
       string value, not a key (with children) */
    CORRADE_ASSERT(json.type() != JsonToken::Type::String || json.children().isEmpty(),
        "Utility::JsonWriter::writeJson(): expected a value token but got an object key", *this);

    /* Iterate arrays and recurse */
    const JsonToken::Type type = json.type();
    if(type == JsonToken::Type::Array) {
        beginArray();
        /* Not using asArray() as that only works if the array is parsed. If
           the array is empty, firstChild() is an invalid iterator, test that
           as well. */
        for(JsonIterator i = json.firstChild(), end = json.next(); i && i != end; i = i->next())
            writeJson(*i);
        endArray();

    /* Iterate objects and recurse */
    } else if(type == JsonToken::Type::Object) {
        beginObject();
        /* Not using asArray() as that only works if the object is parsed. If
           the object is empty, firstChild() is an invalid iterator, test that
           as well. */
        for(JsonIterator i = json.firstChild(), end = json.next(); i && i != end; i = i->next()) {
            if(i->isParsed())
                writeKey(i->asString());
            else
                writeJsonKey(i->data());
            writeJson(*i->firstChild());
        }
        endObject();

    /* Write values */
    } else switch(json.parsedType()) {
        case JsonToken::ParsedType::None:
            writeJson(json.data());
            break;
        case JsonToken::ParsedType::Double:
            write(json.asDouble());
            break;
        case JsonToken::ParsedType::Float:
            write(json.asFloat());
            break;
        case JsonToken::ParsedType::UnsignedInt:
            write(json.asUnsignedInt());
            break;
        case JsonToken::ParsedType::Int:
            write(json.asInt());
            break;
        case JsonToken::ParsedType::UnsignedLong:
            write(json.asUnsignedLong());
            break;
        case JsonToken::ParsedType::Long:
            write(json.asLong());
            break;
        case JsonToken::ParsedType::Other:
            switch(type) {
                case JsonToken::Type::Null:
                    write(json.asNull());
                    break;
                case JsonToken::Type::Bool:
                    write(json.asBool());
                    break;
                case JsonToken::Type::String:
                    write(json.asString());
                    break;
                /* LCOV_EXCL_START */
                /* Numbers are never ParsedType::Other */
                case JsonToken::Type::Number:
                /* These are already handled above */
                case JsonToken::Type::Array:
                case JsonToken::Type::Object:
                    CORRADE_INTERNAL_ASSERT_UNREACHABLE();
                /* LCOV_EXCL_STOP */
            }
            break;
    }

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
