#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
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

/** @file
 * @brief Class Corrade::Utility::Unicode
 */

#include <string>

#include "Utility/corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/** @brief %Unicode utilities */
class CORRADE_UTILITY_EXPORT Unicode {
    public:
        /**
        * @brief Next UTF-8 character
        *
        * Returns %Unicode codepoint of character on the cursor and position of
        * the following character. If an error occurs, returns position of next
        * byte and `0xffffffffu` as codepoint.
        */
        static std::tuple<std::uint32_t, std::size_t> nextChar(const std::string& text, const std::size_t cursor);
};

}}

#endif
