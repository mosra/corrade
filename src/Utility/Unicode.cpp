/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Unicode.h"

#include <tuple>

#include "Utility/Assert.h"

namespace Corrade { namespace Utility {

std::tuple<std::uint32_t, std::size_t> Unicode::nextChar(const std::string& text, std::size_t cursor) {
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
    std::uint32_t result = character & mask;
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
