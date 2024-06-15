/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include "Unicode.h"

#include <cstdint>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/StringView.h"

#ifdef CORRADE_TARGET_WINDOWS
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>

#include "Corrade/Containers/String.h"
#endif

namespace Corrade { namespace Utility { namespace Unicode {

Containers::Pair<char32_t, std::size_t> nextChar(const Containers::StringView text, const std::size_t cursor) {
    CORRADE_DEBUG_ASSERT(cursor < text.size(),
        "Utility::Unicode::nextChar(): expected cursor to be less than" << text.size() << "but got" << cursor, {});

    std::uint32_t character = text[cursor];
    std::size_t end = cursor;
    std::uint32_t mask;

    /** @todo assert for overlong sequences */

    /* Sequence size */
    if(character < 0x80) {
        end += 1;
        mask = 0x7f;
    } else if((character & 0xe0) == 0xc0) {
        end += 2;
        mask = 0x1f;
    } else if((character & 0xf0) == 0xe0) {
        end += 3;
        mask = 0x0f;
    } else if((character & 0xf8) == 0xf0) {
        end += 4;
        mask = 0x07;

    /* Wrong sequence start */
    } else return {U'\xffffffff', cursor+1};

    /* Unexpected end */
    if(text.size() < end) return {U'\xffffffff', cursor+1};

    /* Compute the codepoint */
    char32_t result = character & mask;
    for(std::size_t i = cursor + 1; i != end; ++i) {
        /* Garbage in the sequence */
        if((text[i] & 0xc0) != 0x80)
            return {U'\xffffffff', cursor+1};

        result <<= 6;
        result |= (text[i] & 0x3f);
    }

    return {result, end};
}

Containers::Pair<char32_t, std::size_t> prevChar(const Containers::StringView text, const std::size_t cursor) {
    CORRADE_DEBUG_ASSERT(cursor > 0 && cursor <= text.size(),
        "Utility::Unicode::prevChar(): expected cursor to be greater than 0 and less than or equal to" << text.size() << "but got" << cursor, {});

    /* If the previous byte is a continuation byte, go back until it isn't, but
       only up to three bytes -- any longer sequence of continuation bytes
       would be invalid anyway */
    const std::size_t iMax = Utility::min(std::size_t{4}, cursor);
    std::size_t i = 1;
    while(i != iMax && (text[cursor - i] & 0xc0) == 0x80)
        ++i;

    /* Delegate to nextChar() for the actual codepoint calculation and
       validation. It's also invalid if the next UTF-8 character isn't
       *exactly* this cursor position. */
    const Containers::Pair<char32_t, std::size_t> prev = nextChar(text, cursor - i);
    if(prev.first() == U'\xffffffff' || prev.second() != cursor)
        return {U'\xffffffff', cursor - 1};

    return {prev.first(), cursor - i};
}

std::size_t utf8(const char32_t character, const Containers::ArrayView4<char> result) {
    if(character < U'\x00000080') {
        result[0] = 0x00 | ((character >>  0) & 0x7f);
        return 1;
    }

    if(character < U'\x00000800') {
        result[0] = 0xc0 | ((character >>  6) & 0x1f);
        result[1] = 0x80 | ((character >>  0) & 0x3f);
        return 2;
    }

    if(character < U'\x00010000') {
        result[0] = 0xe0 | ((character >> 12) & 0x0f);
        result[1] = 0x80 | ((character >>  6) & 0x3f);
        result[2] = 0x80 | ((character >>  0) & 0x3f);
        return 3;
    }

    if(character < U'\x00110000') {
        result[0] = 0xf0 | ((character >> 18) & 0x07);
        result[1] = 0x80 | ((character >> 12) & 0x3f);
        result[2] = 0x80 | ((character >>  6) & 0x3f);
        result[3] = 0x80 | ((character >>  0) & 0x3f);
        return 4;
    }

    /* Value outside of UTF-32 range */
    return 0;
}

Containers::Optional<Containers::Array<char32_t>> utf32(const Containers::StringView text) {
    Containers::Array<char32_t> result;
    arrayReserve(result, text.size());

    for(std::size_t i = 0; i != text.size(); ) {
        const Containers::Pair<char32_t, std::size_t> next = nextChar(text, i);
        if(next.first() == U'\xffffffff')
            return {};
        arrayAppend(result, next.first());
        i = next.second();
    }

    /* GCC 4.8 needs extra help here */
    return Containers::optional(Utility::move(result));
}

#ifdef CORRADE_TARGET_WINDOWS
namespace Implementation {

/* Called directly from the header to work around some C++ crap, widen()
   doesn't need any similar treatment so it's implemented inline */
Containers::String narrow(const wchar_t* const text, const int size) {
    /* Compared to the above case with an Array, if size is zero, we can just
       do an early return -- the String constructor takes care of the
       guarantee itself */
    if(!size) return {};
    /* WCtoMB counts the trailing \0 into the size, which we have to cut.
       Containers::String takes care of allocating extra for the null
       terminator so we don't need to do that explicitly. */
    Containers::String result{NoInit, std::size_t(WideCharToMultiByte(CP_UTF8, 0, text, size, nullptr, 0, nullptr, nullptr) - (size == -1 ? 1 : 0))};
    WideCharToMultiByte(CP_UTF8, 0, text, size, result.data(), result.size(), nullptr, nullptr);
    return result;
}

}

Containers::Array<wchar_t> widen(const Containers::StringView text) {
    const std::size_t size = text.size();
    /* MBtoWC can't be called with a zero size for some stupid reason, in that
       case just set the result size to zero. We can't just `return {}` because
       the output array is guaranteed to be a pointer to a null-terminated
       string. */
    const std::size_t resultSize = size == 0 ? 0 : MultiByteToWideChar(CP_UTF8, 0, text.data(), size, nullptr, 0);
    /* Create the array with a sentinel null terminator. If size is zero, this
       is just a single null terminator. */
    Containers::Array<wchar_t> result{NoInit, resultSize + 1};
    result[resultSize] = L'\0';
    /* Again, this function doesn't like to be called if size is zero */
    if(size) MultiByteToWideChar(CP_UTF8, 0, text.data(), size, result.data(), resultSize);
    /* Return the size without the null terminator */
    return Containers::Array<wchar_t>{result.release(), resultSize};
}

Containers::String narrow(const Containers::ArrayView<const wchar_t> text) {
    return Implementation::narrow(text.data(), text.size());
}
#endif

}}}
