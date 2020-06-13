#ifndef Corrade_Utility_ConfigurationValue_h
#define Corrade_Utility_ConfigurationValue_h
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

/** @file
 * @brief Class @ref Corrade::Utility::ConfigurationValue, enum @ref Corrade::Utility::ConfigurationValueFlag, enum set @ref Corrade::Utility::ConfigurationValueFlags
 */

#include <cstdint>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/StlForwardString.h"
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
and @ref Arguments to convert values from and to templated types. Implement a
template specialization to allow saving and getting custom types into and from
configuration files or parsing them from command line.

@section Utility-ConfigurationValue-example Example: custom structure

We have structure named `Foo` and want to store it in configuration file as a
sequence of two integers separated by a space:

@snippet Utility.cpp ConfigurationValue

When saving the structure into configuration file using e.g.
@cpp configuration->addValue("fooValue", Foo{6, 7}); @ce, the result will look
like this:

@code{.ini}
fooValue=6 7
@endcode
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

/**
@brief Configuration value parser and writer for @ref Containers::StringView
@m_since_latest

The returned view might become dangling, parse as @ref Containers::String to
prevent that.
*/
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<Containers::StringView> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static Containers::StringView fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(Containers::StringView value, ConfigurationValueFlags flags);
    #endif
};

/**
@brief Configuration value parser and writer for @ref Containers::String
@m_since_latest

The returned string is a copy, parse as @ref Containers::StringView to avoid a
copy.
*/
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<Containers::String> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static Containers::String fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(const Containers::String& value, ConfigurationValueFlags flags);
    #endif
};

namespace Implementation {
    template<class T> struct CORRADE_UTILITY_EXPORT IntegerConfigurationValue {
        IntegerConfigurationValue() = delete;

        static std::string toString(const T& value, ConfigurationValueFlags flags);
        static T fromString(const std::string& stringValue, ConfigurationValueFlags flags);
    };
    template<class T> struct CORRADE_UTILITY_EXPORT FloatConfigurationValue {
        FloatConfigurationValue() = delete;

        static std::string toString(const T& value, ConfigurationValueFlags flags);
        static T fromString(const std::string& stringValue, ConfigurationValueFlags flags);
    };
}

/**
@brief Configuration value parser and writer for the `short` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<short>: public Implementation::IntegerConfigurationValue<short> {};

/**
@brief Configuration value parser and writer for the `unsigned short` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<unsigned short>: public Implementation::IntegerConfigurationValue<unsigned short> {};

/**
@brief Configuration value parser and writer for the `int` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<int>: public Implementation::IntegerConfigurationValue<int> {};

/**
@brief Configuration value parser and writer for the `unsigned int` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<unsigned int>: public Implementation::IntegerConfigurationValue<unsigned int> {};

/**
@brief Configuration value parser and writer for the `long` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<long>: public Implementation::IntegerConfigurationValue<long> {};

/**
@brief Configuration value parser and writer for the `unsigned long` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<unsigned long>: public Implementation::IntegerConfigurationValue<unsigned long> {};

/**
@brief Configuration value parser and writer for the `long long` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<long long>: public Implementation::IntegerConfigurationValue<long long> {};

/**
@brief Configuration value parser and writer for the `unsigned long long` type

Empty value is parsed as @cpp 0 @ce.
*/
template<> struct ConfigurationValue<unsigned long long>: public Implementation::IntegerConfigurationValue<unsigned long long> {};

/**
@brief Configuration value parser and writer for the `float` type

Empty value is parsed as @cpp 0.0f @ce. Values are saved with 6 significant
digits, same as how @ref Debug or @ref format() prints them.
*/
template<> struct ConfigurationValue<float>: public Implementation::FloatConfigurationValue<float> {};

/**
@brief Configuration value parser and writer for the `double` type

Empty value is parsed as @cpp 0.0 @ce. Values are saved with 15 significant
digits, same as how @ref Debug or @ref format() prints them.
*/
template<> struct ConfigurationValue<double>: public Implementation::FloatConfigurationValue<double> {};

/**
@brief Configuration value parser and writer for `long double` type

Empty value is parsed as @cpp 0.0l @ce. Values are saved with 18 significant
digits on platforms with 80-bit @cpp long double @ce and 15 digits on platforms
@ref CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE "where it is 64-bit", same as how
@ref Debug or @ref format() prints them.
*/
template<> struct ConfigurationValue<long double>: public Implementation::FloatConfigurationValue<long double> {};

/** @brief Configuration value parser and writer for @ref std::string type */
template<> struct CORRADE_UTILITY_EXPORT ConfigurationValue<std::string> {
    ConfigurationValue() = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    static std::string fromString(const std::string& value, ConfigurationValueFlags flags);
    static std::string toString(const std::string& value, ConfigurationValueFlags flags);
    #endif
};

/**
@brief Configuration value parser and writer for `bool` type

Reads `1`, `yes`, `y` or `true` as @cpp true @ce, any other string as
@cpp false @ce. Writes `true` or `false`.
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

Reads and writes the value in hexadecimal. Empty value is parsed as @cpp 0 @ce.
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
