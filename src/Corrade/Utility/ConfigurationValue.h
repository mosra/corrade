#ifndef Corrade_Utility_ConfigurationValue_h
#define Corrade_Utility_ConfigurationValue_h
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

/** @file
 * @brief Class @ref Corrade::Utility::ConfigurationValue, enum @ref Corrade::Utility::ConfigurationValueFlag, enum set @ref Corrade::Utility::ConfigurationValueFlags
 */

#include <cstdint>
#include <string>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Configuration value conversion flag

@see @ref ConfigurationValueFlags
*/
enum class ConfigurationValueFlag: std::uint8_t {
    Oct = 1 << 0,           /**< Numeric value as octal */
    Hex = 1 << 1,           /**< Numeric value as hexadecimal */
    Scientific = 1 << 2,    /**< Floating point values in scientific notation */
    Uppercase = 1 << 3      /**< Use uppercase characters for numeric output */
};

/**
@brief Configuration value conversion flags

@see @ref ConfigurationGroup::value(), @ref ConfigurationGroup::values(),
    @ref ConfigurationGroup::setValue(), @ref ConfigurationGroup::addValue(),
    @ref ConfigurationValue::toString(), @ref ConfigurationValue::fromString()
*/
typedef Containers::EnumSet<ConfigurationValueFlag> ConfigurationValueFlags;

CORRADE_ENUMSET_OPERATORS(ConfigurationValueFlags)

/**
@brief Configuration value parser and writer

Functions in this struct are called internally by @ref ConfigurationGroup
functions to convert values from and to templated types. Reimplement the
structure with template specialization to allow saving and getting
non-standard types into and from configuration files.

## Example: custom structure

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

namespace Implementation {
    template<class T> struct CORRADE_UTILITY_EXPORT BasicConfigurationValue {
        BasicConfigurationValue() = delete;

        static std::string toString(const T& value, ConfigurationValueFlags flags);
        static T fromString(const std::string& stringValue, ConfigurationValueFlags flags);
    };
}

/** @brief Configuration value parser and writer for `short` type */
template<> struct ConfigurationValue<short>: public Implementation::BasicConfigurationValue<short> {};
/** @brief Configuration value parser and writer for `unsigned short` type */
template<> struct ConfigurationValue<unsigned short>: public Implementation::BasicConfigurationValue<unsigned short> {};
/** @brief Configuration value parser and writer for `int` type */
template<> struct ConfigurationValue<int>: public Implementation::BasicConfigurationValue<int> {};
/** @brief Configuration value parser and writer for `unsigned int` type */
template<> struct ConfigurationValue<unsigned int>: public Implementation::BasicConfigurationValue<unsigned int> {};
/** @brief Configuration value parser and writer for `long` type */
template<> struct ConfigurationValue<long>: public Implementation::BasicConfigurationValue<long> {};
/** @brief Configuration value parser and writer for `unsigned long` type */
template<> struct ConfigurationValue<unsigned long>: public Implementation::BasicConfigurationValue<unsigned long> {};
/** @brief Configuration value parser and writer for `long long` type */
template<> struct ConfigurationValue<long long>: public Implementation::BasicConfigurationValue<long long> {};
/** @brief Configuration value parser and writer for `unsigned long long` type */
template<> struct ConfigurationValue<unsigned long long>: public Implementation::BasicConfigurationValue<unsigned long long> {};
/** @brief Configuration value parser and writer for `float` type */
template<> struct ConfigurationValue<float>: public Implementation::BasicConfigurationValue<float> {};
/** @brief Configuration value parser and writer for `double` type */
template<> struct ConfigurationValue<double>: public Implementation::BasicConfigurationValue<double> {};

#ifndef CORRADE_TARGET_EMSCRIPTEN
/**
@brief Configuration value parser and writer for `long double` type
@partialsupport Not available in @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" as
    JavaScript doesn't support doubles larger than 64 bits.
*/
template<> struct ConfigurationValue<long double>: public Implementation::BasicConfigurationValue<long double> {};
#endif

/** @brief Configuration value parser and writer for `sd::string` type */
template<> struct ConfigurationValue<std::string>: public Implementation::BasicConfigurationValue<std::string> {};

/**
@brief Configuration value parser and writer for `bool` type

Reads `1`, `yes`, `y` or `true` as `true`, any other string as `false`. Writes
`true` or `false`.
*/
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<bool> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static bool fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(bool value, ConfigurationValueFlags flags);
    #endif
};

/**
@brief Configuration value parser and writer for `char32_t` type

Reads and writes the value in hexadecimal.
*/
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<char32_t> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static char32_t fromString(const std::string& value, ConfigurationValueFlags);
    static std::string toString(char32_t value, ConfigurationValueFlags);
    #endif
};

}}

#endif
