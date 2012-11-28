#ifndef Corrade_Utility_String_h
#define Corrade_Utility_String_h
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
 * @brief Class Corrade::Utility::String
 */

#include <string>
#include <vector>

#include "corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/** @brief %String utilities */
class UTILITY_EXPORT String {
    public:
        String() = delete;

        /**
         * @brief Trim leading and trailing whitespaces from string
         * @param str           %String to be trimmed
         * @param characters    Characters which will be trimmed
         * @return Trimmed string
         */
        /* Parameter intentionally passed by copy to avoid another copy internally */
        static std::string trim(std::string str, const std::string& characters = " \t\f\v\r\n");

        /**
         * @brief Split string on given character
         * @param str               %String to be splitted
         * @param delim             Delimiter
         * @param keepEmptyParts    Whether to keep empty parts
         * @return Vector of splitted strings
         */
        static std::vector<std::string> split(const std::string& str, char delim, bool keepEmptyParts = true);

        /**
         * @brief Convert string to lowercase
         * @param str               %String to be converted
         * @return Lowercase version of the string
         *
         * @attention Doesn't work with UTF-8.
         */
        /* Parameter intentionally passed by copy to avoid another copy internally */
        static std::string lowercase(std::string str);
};

}}

#endif
