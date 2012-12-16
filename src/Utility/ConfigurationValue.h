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
#include <string>

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
@brief %Configuration value parser and writer

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
    ConfigurationValue() = delete;

    #ifdef DOXYGEN_GENERATING_OUTPUT
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
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    template<class T> struct CORRADE_UTILITY_EXPORT BasicConfigurationValue {
        BasicConfigurationValue() = delete;

        static std::string toString(const T& value, ConfigurationValueFlags flags);
        static T fromString(const std::string& stringValue, ConfigurationValueFlags flags);
    };
}
#endif

/** @brief %Configuration value parser and writer for `short` type */
template<> struct ConfigurationValue<short>: public Implementation::BasicConfigurationValue<short> {};
/** @brief %Configuration value parser and writer for `unsigned short` type */
template<> struct ConfigurationValue<unsigned short>: public Implementation::BasicConfigurationValue<unsigned short> {};
/** @brief %Configuration value parser and writer for `int` type */
template<> struct ConfigurationValue<int>: public Implementation::BasicConfigurationValue<int> {};
/** @brief %Configuration value parser and writer for `unsigned int` type */
template<> struct ConfigurationValue<unsigned int>: public Implementation::BasicConfigurationValue<unsigned int> {};
/** @brief %Configuration value parser and writer for `long` type */
template<> struct ConfigurationValue<long>: public Implementation::BasicConfigurationValue<long> {};
/** @brief %Configuration value parser and writer for `unsigned long` type */
template<> struct ConfigurationValue<unsigned long>: public Implementation::BasicConfigurationValue<unsigned long> {};
/** @brief %Configuration value parser and writer for `long long` type */
template<> struct ConfigurationValue<long long>: public Implementation::BasicConfigurationValue<long long> {};
/** @brief %Configuration value parser and writer for `unsigned long long` type */
template<> struct ConfigurationValue<unsigned long long>: public Implementation::BasicConfigurationValue<unsigned long long> {};
/** @brief %Configuration value parser and writer for `float` type */
template<> struct ConfigurationValue<float>: public Implementation::BasicConfigurationValue<float> {};
/** @brief %Configuration value parser and writer for `double` type */
template<> struct ConfigurationValue<double>: public Implementation::BasicConfigurationValue<double> {};
/** @brief %Configuration value parser and writer for `long double` type */
template<> struct ConfigurationValue<long double>: public Implementation::BasicConfigurationValue<long double> {};
/** @brief %Configuration value parser and writer for `sd::string` type */
template<> struct ConfigurationValue<std::string>: public Implementation::BasicConfigurationValue<std::string> {};

/** @brief %Configuration value parser and writer for `bool` type */
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<bool> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static bool fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(const bool& value, ConfigurationValueFlags flags);
    #endif
};

}}

#endif
