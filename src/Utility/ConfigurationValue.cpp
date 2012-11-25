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

#include "ConfigurationValue.h"

namespace Corrade { namespace Utility {

#ifndef DOXYGEN_GENERATING_OUTPUT
bool ConfigurationValue<bool>::fromString(const std::string& value, ConfigurationValueFlags) {
    if(value == "1" || value == "yes" || value == "y" || value == "true") return true;
    return false;
}

std::string ConfigurationValue<bool>::toString(const bool& value, ConfigurationValueFlags) {
    if(value) return "true";
    return "false";
}
#endif

}}
