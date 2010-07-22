#ifndef Map2X_Utility_utilities_h
#define Map2X_Utility_utilities_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Basic utilities
 */

#include <string>

namespace Map2X { namespace Utility {

/**
 * @brief Trim leading and trailing whitespaces from string
 * @param str           String to be trimmed
 * @param characters    Characters which will be trimmed
 * @return Trimmed string
 */
std::string trim(std::string str, const std::string& characters = " \t\f\v\r\n");

}}

#endif
