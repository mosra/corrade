#ifndef Corrade_Utility_Unicode_h
#define Corrade_Utility_Unicode_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
 * @brief Class @ref Corrade::Utility::Unicode
 */

#include <cstddef>
#include <string>
#include <utility>

#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/** @brief Unicode utilities */
class CORRADE_UTILITY_EXPORT Unicode {
    public:
        /**
         * @brief Next UTF-8 character
         *
         * Returns Unicode codepoint of character on the cursor and position
         * of the following character. If an error occurs, returns position of
         * next byte and `0xffffffffu` as codepoint.
         */
        static std::pair<char32_t, std::size_t> nextChar(const std::string& text, const std::size_t cursor);

        /** @brief Convert UTF-8 to UTF-32 */
        static std::u32string utf32(const std::string& text);
};

}}

#endif
