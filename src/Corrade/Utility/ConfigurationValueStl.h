#ifndef Corrade_Utility_ConfigurationValueStl_h
#define Corrade_Utility_ConfigurationValueStl_h
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

/** @file
@brief STL @ref std::string compatibility for @ref Corrade::Utility::ConfigurationValue
 */

#include <string>
#include "Corrade/Utility/ConfigurationValue.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/** @brief Configuration value parser and writer for @ref std::string type */
template <> struct CORRADE_UTILITY_EXPORT ConfigurationValue<std::string> {
    ConfigurationValue() = delete;

#ifndef DOXYGEN_GENERATING_OUTPUT
    static std::string fromString(Containers::StringView value, ConfigurationValueFlags) {
        return value;
    }
    static Containers::String toString(const std::string& value, ConfigurationValueFlags) {
        return value;
    }
#endif
};

}}

#endif
