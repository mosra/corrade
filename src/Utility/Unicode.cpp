/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include <tuple>

#include "Utility/Assert.h"

namespace Corrade { namespace Utility {

std::tuple<char32_t, std::size_t> Unicode::nextChar(const std::string& text, std::size_t cursor) {
    CORRADE_INTERNAL_ASSERT(cursor < text.length());

    std::uint32_t character = text[cursor];
    std::size_t end = cursor;
    std::uint32_t mask;

    /** @todo assert for overlong sequences */

    /* Sequence size */
    if(character < 128) {
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
    }

    /* Wrong sequence start */
    if(cursor == end || text.size() < end)
        return std::make_tuple(0xffffffffu, cursor+1);

    /* Compute the codepoint */
    char32_t result = character & mask;
    for(std::size_t i = cursor+1; i != end; ++i) {
        /* Garbage in the sequence */
        if((text[i] & 0xc0) != 0x80)
            return std::make_tuple(0xffffffffu, cursor+1);

        result <<= 6;
        result |= (text[i] & 0x3f);
    }

    return std::make_tuple(result, end);
}

}}
