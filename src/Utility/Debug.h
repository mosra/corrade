#ifndef Corrade_Utility_Debug_h
#define Corrade_Utility_Debug_h
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
 * @brief Class Corrade::Utility::Debug, Corrade::Utility::Warning, Corrade::Utility::Error
 */

#include <iosfwd>
#include <utility>
#include <type_traits>

#include "TypeTraits.h"

#include "corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/**
@brief %Debug output handler

Provides convenient stream interface for passing data to debug output
(standard output). Data are separated with spaces and last value is enclosed
with newline character.
Example usage:
@code
// Common usage
Debug() << "string" << 34 << 275.0f;

// Redirect debug output to string
std::ostringstream output;
Debug::setOutput(&o);
Debug() << "the meaning of life, universe and everything is" << 42;

// Mute debug output
Debug::setOutput(0);
Debug() << "noone should see my ebanking password" << password;

// Reset debug output to default
Debug::setOutput();

// Conditional debug output (avoid inserting newline where it's not desired)
Debug d();
d << "Cannot foo";
if(bar)
    d << "because of bar.";
else
    d << "because of everything else.";
// (newline character will be written to output on object destruction)
@endcode
Support for printing other types (which are not handled by `iostream` itself)
can be added by implementing function operator<<(Debug, const T&) for given
type.

@see Warning, Error, CORRADE_ASSERT(), CORRADE_INTERNAL_ASSERT()
@todo Output to more ostreams at once
 */
class CORRADE_UTILITY_EXPORT Debug {
    /* Disabling assignment */
    CORRADE_UTILITY_LOCAL Debug& operator=(const Debug& other);

    public:
        /** @brief Output flags */
        enum Flag {
            /* 0x01 reserved for indicating that no value was yet written */
            SpaceAfterEachValue = 0x02, /**< Put space after each value (enabled by default) */
            NewLineAtTheEnd = 0x04      /**< Put newline at the end (enabled by default) */
        };

        /**
         * @brief Constructor
         * @param _output       Stream where to put debug output. If set to 0,
         *      no debug output will be written anywhere.
         *
         * Constructs debug object with given output.
         * @see setOutput()
         */
        inline explicit Debug(std::ostream* _output = globalOutput): output(_output), flags(0x01 | SpaceAfterEachValue | NewLineAtTheEnd) {}

        /**
         * @brief Copy constructor
         *
         * When copied from class which already wrote anything on the output,
         * disabling flag Debug::NewLineAtTheEnd, so there aren't excessive
         * newlines in the output.
         *
         * Called in this situation:
         * @code
         * Debug() << value;
         * @endcode
         */
        Debug(const Debug& other);

        /**
         * @brief Reference copy constructor
         *
         * Marking original class like it have already written something on
         * the output, so it adds whitespace before next value, disabling flag
         * Debug::NewLineAtTheEnd the same way as in Debug(const Debug& other).
         *
         * Called in this situation:
         * @code
         * Debug debug;
         * debug << value;
         * @endcode
         */
        Debug(Debug& other);

        /**
         * @brief Destructor
         *
         * Adds newline at the end of debug output, if it is enabled in flags
         * and the output is not empty.
         * @see Flag
         */
        ~Debug();

        /** @brief Flag */
        inline bool flag(Flag flag) const { return flags & flag; }

        /** @brief Set flag */
        void setFlag(Flag flag, bool value);

        /**
         * @brief Print string to debug output
         *
         * If there is already something on the output, puts space before
         * the value.
         * @see operator<<(Debug, const T&)
         */
        Debug operator<<(const std::string& value);
        Debug operator<<(const char* value);            /**< @overload */
        Debug operator<<(const void* value);            /**< @overload */
        Debug operator<<(bool value);                   /**< @overload */
        Debug operator<<(char value);                   /**< @overload */
        Debug operator<<(int value);                    /**< @overload */
        Debug operator<<(long value);                   /**< @overload */
        Debug operator<<(long long value);              /**< @overload */
        Debug operator<<(unsigned value);               /**< @overload */
        Debug operator<<(unsigned long value);          /**< @overload */
        Debug operator<<(unsigned long long value);     /**< @overload */
        Debug operator<<(float value);                  /**< @overload */
        Debug operator<<(double value);                 /**< @overload */
        Debug operator<<(long double value);            /**< @overload */

        /**
         * @brief Globally set output for newly created instances
         * @param _output       Stream where to put debug output. If set to 0,
         *      no debug output will be written anywhere.
         *
         * All successive Debug instances created with default constructor will
         * be redirected to given stream.
         */
        inline static void setOutput(std::ostream* _output = globalOutput) {
            globalOutput = _output;
        }

    protected:
        std::ostream* output;   /**< @brief Stream where to put the output */

    private:
        template<class T> Debug print(const T& value);

        CORRADE_UTILITY_EXPORT static std::ostream* globalOutput;
        int flags;
};

#ifdef DOXYGEN_GENERATING_OUTPUT
/** @relates Debug
@brief Operator for printing custom types to debug
@param debug     %Debug class
@param value     Value to be printed

Support for printing custom types (i.e. those not handled by `iostream`) can
be added by implementing this function for given type.

The function should convert the type to one of supported types (such as
`std::string`) and then call Debug::operator<<() with it. You can also use
Debug::setFlag() for modifying newline and whitespace behavior.
 */
template<class T> Debug operator<<(Debug debug, const T& value);
#else
template<class Iterable> Debug operator<<(typename std::enable_if<IsIterable<Iterable>::Value && !std::is_same<Iterable, std::string>::value, Debug>::type debug, const Iterable& value) {
    debug << '[';
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    for(typename Iterable::const_iterator it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << ", ";
        debug << *it;
    }
    debug << ']';
    debug.setFlag(Debug::SpaceAfterEachValue, true);
    return debug;
}
template<class A, class B> Debug operator<<(Debug debug, const std::pair<A, B>& value) {
    debug << '(';
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    debug << value.first << ", " << value.second << ')';
    debug.setFlag(Debug::SpaceAfterEachValue, true);
    return debug;
}
#endif

/**
 * @brief %Warning output handler
 *
 * Same as Debug, but by default writes output to standard error output. Thus
 * it is possible to separate / mute Debug, Warning and Error outputs.
 */
class CORRADE_UTILITY_EXPORT Warning: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Warning(std::ostream* _output = globalWarningOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalWarningOutput) {
            globalWarningOutput = _output; }

    private:
        CORRADE_UTILITY_EXPORT static std::ostream* globalWarningOutput;
};

/**
 * @brief %Error output handler
 *
 * @copydetails Warning
 */
class CORRADE_UTILITY_EXPORT Error: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Error(std::ostream* _output = globalErrorOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalErrorOutput) {
            globalErrorOutput = _output; }

    private:
        CORRADE_UTILITY_EXPORT static std::ostream* globalErrorOutput;
};

}}

#endif
