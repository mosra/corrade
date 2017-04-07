/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Implementation {
    template<class T> std::string BasicConfigurationValue<T>::toString(const T& value, ConfigurationValueFlags flags) {
        std::ostringstream stream;

        /* Hexadecimal / octal values, scientific notation */
        if(flags & ConfigurationValueFlag::Hex)
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

        stream << value;
        return stream.str();
    }

    template<class T> T BasicConfigurationValue<T>::fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::istringstream stream{stringValue};

        /* Hexadecimal / octal values, scientific notation */
        if(flags & ConfigurationValueFlag::Hex)
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

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
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    template struct BasicConfigurationValue<long double>;
    #endif
    template struct BasicConfigurationValue<std::string>;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
bool ConfigurationValue<bool>::fromString(const std::string& value, ConfigurationValueFlags) {
    return value == "1" || value == "yes" || value == "y" || value == "true";
}
std::string ConfigurationValue<bool>::toString(const bool value, ConfigurationValueFlags) {
    return value ? "true" : "false";
}

char32_t ConfigurationValue<char32_t>::fromString(const std::string& value, ConfigurationValueFlags) {
    return char32_t(ConfigurationValue<unsigned long long>::fromString(value, ConfigurationValueFlag::Hex|ConfigurationValueFlag::Uppercase));
}
std::string ConfigurationValue<char32_t>::toString(const char32_t value, ConfigurationValueFlags) {
    return ConfigurationValue<unsigned long long>::toString(value, ConfigurationValueFlag::Hex|ConfigurationValueFlag::Uppercase);
}
#endif

}}
