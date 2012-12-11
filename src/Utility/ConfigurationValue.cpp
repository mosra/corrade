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

#include <sstream>

namespace Corrade { namespace Utility {

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    template<class T> std::string BasicConfigurationValue<T>::toString(const T& value, ConfigurationValueFlags flags) {
        std::ostringstream stream;

        /* Hexadecimal / octal values */
        if(flags & (ConfigurationValueFlag::Color|ConfigurationValueFlag::Hex))
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);
        if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        stream << value;

        std::string stringValue = stream.str();

        /* Strip initial # character, if user wants a color */
        if(flags & ConfigurationValueFlag::Color)
            stringValue = '#' + stringValue;

        return stringValue;
    }

    template<class T> T BasicConfigurationValue<T>::fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::string _stringValue = stringValue;

        /* Strip initial # character, if user wants a color */
        if(flags & ConfigurationValueFlag::Color && !stringValue.empty() && stringValue[0] == '#')
            _stringValue = stringValue.substr(1);

        std::istringstream stream(_stringValue);

        /* Hexadecimal / octal values, scientific notation */
        if(flags & (ConfigurationValueFlag::Color|ConfigurationValueFlag::Hex))
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);
        if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        T value;
        stream >> value;

        return value;
    }

    template class BasicConfigurationValue<short>;
    template class BasicConfigurationValue<unsigned short>;
    template class BasicConfigurationValue<int>;
    template class BasicConfigurationValue<unsigned int>;
    template class BasicConfigurationValue<long>;
    template class BasicConfigurationValue<unsigned long>;
    template class BasicConfigurationValue<long long>;
    template class BasicConfigurationValue<unsigned long long>;
    template class BasicConfigurationValue<float>;
    template class BasicConfigurationValue<double>;
    template class BasicConfigurationValue<long double>;
    template class BasicConfigurationValue<std::string>;
}

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
