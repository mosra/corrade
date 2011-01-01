#ifndef Kompas_Utility_Debug_h
#define Kompas_Utility_Debug_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Kompas::Utility::Debug
 */

#include <ostream>

#include "utilities.h"

namespace Kompas { namespace Utility {

/**
 * @brief %Debug output handler
 *
 * Provides convenient stream interface for passing data to debug output
 * (standard output). Data are separated with spaces and last value is enclosed
 * with newline character.
 * Example usage:
 * @code
 * // Common usage
 * Debug() << "string" << 34 << 275.0f;
 *
 * // Redirect debug output to string
 * std::ostringstream output;
 * Debug::setOutput(&o);
 * Debug() << "the meaning of life, universe and everything is" << 42;
 *
 * // Mute debug output
 * Debug::setOutput(0);
 * Debug() << "noone should see my ebanking password" << password;
 *
 * // Reset debug output to default
 * Debug::setOutput();
 *
 * // Conditional debug output (avoid inserting newline where it's not desired)
 * Debug d();
 * d << "Cannot foo";
 * if(bar)
 *     d << "because of bar.";
 * else
 *     d << "because of everything else.";
 * // (newline character will be written to output on object destruction)
 * @endcode
 * @todo Output to more ostreams at once
 * @see Warning, Error
 */
class UTILITY_EXPORT Debug {
    public:
        /**
         * @brief Constructor
         * @param _output       Stream where to put debug output. If set to 0,
         *      no debug output will be written anywhere.
         *
         * Constructs debug object with given output. See also setOutput().
         */
        Debug(std::ostream* _output = globalOutput): output(_output), firstValue(true) {}

        /**
         * @brief Destructor
         *
         * Adds newline at the end of debug output, if it is not empty.
         */
        ~Debug() { if(output && !firstValue) *output << std::endl; }

        /**
         * @brief Globally set output for newly created instances
         * @param _output       Stream where to put debuug output. If set to 0,
         *      no debug output will be written anywhere.
         *
         * All successive Debug instances created with default constructor will
         * be outputted to given stream.
         */
        inline static void setOutput(std::ostream* _output = globalOutput) { globalOutput = _output; }

        /**
         * @brief Write value to debug output
         * @param value         Value
         *
         * The implementation supports all types supported in STL streams.
         * library. Support for debugging another non-trivial types can be
         * added with implementing @c operator<<(Debug, const T&) for given
         * type.
         */
        template<class T> Debug& operator<<(const T& value) {
            if(!output) return *this;

            /* Separate values with spaces */
            if(!firstValue) *output << " ";
            else firstValue = false;

            *output << value;
            return *this;
        }

    protected:
        std::ostream* output;   /**< @brief Stream where to put the output */

    private:
        static std::ostream* globalOutput;
        bool firstValue;
};

/**
 * @brief Warning output handler
 *
 * Same as Debug, but by default writes output to standard error output. Thus
 * it is possible to separate / mute Debug, Warning and Error outputs.
 */
class UTILITY_EXPORT Warning: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Warning(std::ostream* _output = globalOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalOutput) { globalOutput = _output; }

    private:
        static std::ostream* globalOutput;
};

/**
 * @brief Error output handler
 *
 * @copydetails Warning
 */
class UTILITY_EXPORT Error: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Error(std::ostream* _output = globalOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalOutput) { globalOutput = _output; }

    private:
        static std::ostream* globalOutput;
};

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
 * @brief Debug output for custom types
 * @param debug     Debug class
 * @param value     Value to be outputted
 *
 * The operator should convert the type to one of supported types, such as
 * std::string and then call Debug::operator<<() with it.
 */
template<class T> Debug& operator<<(Debug debug, const T& value);
#endif

}}

#endif
