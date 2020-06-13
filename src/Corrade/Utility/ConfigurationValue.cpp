/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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

#include <iomanip>
#include <sstream>

#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/TypeTraits.h"

namespace Corrade { namespace Utility {

Containers::StringView ConfigurationValue<Containers::StringView>::fromString(const std::string& value, ConfigurationValueFlags) {
    return value;
}
std::string ConfigurationValue<Containers::StringView>::toString(const Containers::StringView value, ConfigurationValueFlags) {
    return value;
}

Containers::String ConfigurationValue<Containers::String>::fromString(const std::string& value, ConfigurationValueFlags) {
    return value;
}
std::string ConfigurationValue<Containers::String>::toString(const Containers::String& value, ConfigurationValueFlags) {
    return value;
}

namespace Implementation {
    template<class T> std::string IntegerConfigurationValue<T>::toString(const T& value, ConfigurationValueFlags flags) {
        std::ostringstream stream;

        /* Hexadecimal / octal values */
        if(flags & ConfigurationValueFlag::Hex)
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

        stream << value;
        return stream.str();
    }

    template<class T> T IntegerConfigurationValue<T>::fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        if(stringValue.empty()) return T{};

        std::istringstream stream{stringValue};

        /* Hexadecimal / octal values */
        if(flags & ConfigurationValueFlag::Hex)
            stream.setf(std::istringstream::hex, std::istringstream::basefield);
        else if(flags & ConfigurationValueFlag::Oct)
            stream.setf(std::istringstream::oct, std::istringstream::basefield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

        T value;
        stream >> value;
        return value;
    }

    template struct IntegerConfigurationValue<short>;
    template struct IntegerConfigurationValue<unsigned short>;
    template struct IntegerConfigurationValue<int>;
    template struct IntegerConfigurationValue<unsigned int>;
    template struct IntegerConfigurationValue<long>;
    template struct IntegerConfigurationValue<unsigned long>;
    template struct IntegerConfigurationValue<long long>;
    template struct IntegerConfigurationValue<unsigned long long>;

    template<class T> std::string FloatConfigurationValue<T>::toString(const T& value, ConfigurationValueFlags flags) {
        std::ostringstream stream;

        /* Scientific notation */
        if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

        stream << std::setprecision(FloatPrecision<T>::Digits) << value;
        return stream.str();
    }

    template<class T> T FloatConfigurationValue<T>::fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        if(stringValue.empty()) return T{};

        std::istringstream stream{stringValue};

        /* Scientific notation */
        if(flags & ConfigurationValueFlag::Scientific)
            stream.setf(std::istringstream::scientific, std::istringstream::floatfield);

        if(flags & ConfigurationValueFlag::Uppercase)
            stream.setf(std::istringstream::uppercase);

        T value;
        stream >> value;
        return value;
    }

    template struct FloatConfigurationValue<float>;
    template struct FloatConfigurationValue<double>;
    template struct FloatConfigurationValue<long double>;
}

#ifndef DOXYGEN_GENERATING_OUTPUT
std::string ConfigurationValue<std::string>::fromString(const std::string& value, ConfigurationValueFlags) {
    return value;
}
std::string ConfigurationValue<std::string>::toString(const std::string& value, ConfigurationValueFlags) {
    return value;
}

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
