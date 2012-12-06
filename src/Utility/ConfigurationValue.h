#ifndef Corrade_Utility_ConfigurationValue_h
#define Corrade_Utility_ConfigurationValue_h
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
 * @brief Class Corrade::Utility::ConfigurationValue, enum set Corrade::Utility::ConfigurationValueFlag, Corrade::Utility::ConfigurationValueFlags
 */

#include <cstdint>
#include <sstream>

#include "Containers/EnumSet.h"

#include "corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

class Configuration;

/** @relates ConfigurationGroup
@brief %Configuration value conversion flag

@see ConfigurationValueFlags
*/
enum class ConfigurationValueFlag: std::uint8_t {
    Oct = 1 << 0,           /**< Numeric value as octal */
    Hex = 1 << 1,           /**< Numeric value as hexadecimal */
    Color = 1 << 2,         /**< Numeric value as color representation */
    Scientific = 1 << 3     /**< Floating point values in scientific notation */
};

/** @relates ConfigurationGroup
@brief %Configuration value conversion flags

@see ConfigurationGroup::value(), ConfigurationGroup::values(),
    ConfigurationGroup::setValue(), ConfigurationGroup::addValue(),
    ConfigurationValue::toString(), ConfigurationValue::fromString()
*/
typedef Containers::EnumSet<ConfigurationValueFlag, std::uint8_t> ConfigurationValueFlags;

CORRADE_ENUMSET_OPERATORS(ConfigurationValueFlags)

/**
@brief Template structure for type conversion

Functions in this struct are called internally by ConfigurationGroup
functions to convert values from and to templated types. Reimplement the
structure with template specialization to allow saving and getting
non-standard types into and from configuration files.

@section ConfigurationValue_Example Example: custom structure
We have structure named `Foo` and want to store it in configuration file as a
sequence of two integers separated by a space.
@code
#include "Utility/ConfigurationGroup.h"

struct Foo {
    int a, b;
};

namespace Corrade { namespace Utility {

template<> struct ConfigurationValue<Foo> {
    static std::string toString(const Foo& value, ConfigurationValueFlags flags) {
        return
            ConfigurationValue<int>::toString(value.a, flags) + ' ' +
            ConfigurationValue<int>::toString(value.b, flags);
    }

    static Foo fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::istringstream i(stringValue);
        std::string a, b;

        Foo foo;
        (i >> a) && (area.a = ConfigurationValue<int>::fromString(a, flags));
        (i >> b) && (area.b = ConfigurationValue<int>::fromString(a, flags));
        return foo;
    }
};

}}
@endcode
When saving the structure into configuration file using e.g.
`configuration->addValue("fooValue", Foo{6, 7});`, the result will look like
this:

    fooValue=6 7
*/
template<class T> struct ConfigurationValue {
    /**
    * @brief Convert value to string
    * @param value         Value
    * @param flags         Flags
    * @return Value as string
    */
    static std::string toString(const T& value, ConfigurationValueFlags flags);

    /**
    * @brief Convert value from string
    * @param stringValue   Value as string
    * @param flags         Flags
    * @return Value
    */
    static T fromString(const std::string& stringValue, ConfigurationValueFlags flags);
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> std::string ConfigurationValue<T>::toString(const T& value, ConfigurationValueFlags flags) {
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

template<class T> T ConfigurationValue<T>::fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
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

template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<bool> {
    static bool fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(const bool& value, ConfigurationValueFlags flags);
};
#endif

}}

#endif
