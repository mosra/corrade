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

    template struct BasicConfigurationValue<short>;
    template struct BasicConfigurationValue<unsigned short>;
    template struct BasicConfigurationValue<int>;
    template struct BasicConfigurationValue<unsigned int>;
    template struct BasicConfigurationValue<long>;
    template struct BasicConfigurationValue<unsigned long>;
    template struct BasicConfigurationValue<long long>;
    template struct BasicConfigurationValue<unsigned long long>;
    template struct BasicConfigurationValue<float>;
    template struct BasicConfigurationValue<double>;
    template struct BasicConfigurationValue<long double>;
    template struct BasicConfigurationValue<std::string>;
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
