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
 * @brief Class Corrade::Utility::Debug, Corrade::Utility::Warning, Corrade::Utility::Error, macro CORRADE_ASSERT().
 */

#include <cstdlib>
#include <ostream>

#include "TypeTraits.h"
#include "utilities.h"

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

@todo Output to more ostreams at once
@see Warning, Error
 */
class UTILITY_EXPORT Debug {
    /* Disabling assignment */
    UTILITY_LOCAL Debug& operator=(const Debug& other);

    template<class T> friend Debug operator<<(typename std::enable_if<!IsIterable<T>::value || std::is_same<T, std::string>::value, Debug>::type, const T&);

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
         *
         * @see setOutput().
         */
        inline Debug(std::ostream* _output = globalOutput): output(_output), flags(0x01 | SpaceAfterEachValue | NewLineAtTheEnd) {}

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
         *
         * @see Flag.
         */
        ~Debug();

        /** @brief Flag */
        inline bool flag(Flag flag) const { return flags & flag; }

        /** @brief Set flag */
        void setFlag(Flag flag, bool value);

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
        UTILITY_EXPORT static std::ostream* globalOutput;
        int flags;
};

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
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
template<class T> Debug operator<<(typename std::enable_if<!IsIterable<T>::value || std::is_same<T, std::string>::value, Debug>::type debug, const T& value) {
    if(!debug.output) return debug;

    /* Separate values with spaces, if enabled */
    if(debug.flags & 0x01) debug.flags &= ~0x01;
    else if(debug.flags & Debug::SpaceAfterEachValue) *debug.output << " ";

    *debug.output << value;
    return debug;
}
template<class Iterable> Debug operator<<(typename std::enable_if<IsIterable<Iterable>::value && !std::is_same<Iterable, std::string>::value, Debug>::type debug, const Iterable& value) {
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
class UTILITY_EXPORT Warning: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Warning(std::ostream* _output = globalWarningOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalWarningOutput) {
            globalWarningOutput = _output; }

    private:
        UTILITY_EXPORT static std::ostream* globalWarningOutput;
};

/**
 * @brief %Error output handler
 *
 * @copydetails Warning
 */
class UTILITY_EXPORT Error: public Debug {
    public:
        /** @copydoc Debug::Debug() */
        Error(std::ostream* _output = globalErrorOutput): Debug(_output) {}

        inline static void setOutput(std::ostream* _output = globalErrorOutput) {
            globalErrorOutput = _output; }

    private:
        UTILITY_EXPORT static std::ostream* globalErrorOutput;
};


/**
@brief Assertion macro
@param condition    Assert condition
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail
@hideinitializer

By default, if assertion fails, @p message is printed to error output and the
application exits with value `-1`. If `CORRADE_GRACEFUL_ASSERT` is defined,
the message is printed and the function returns with @p returnValue. If
`CORRADE_NO_ASSERT` is defined, this macro does nothing.
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    if(!(condition)) {                                                      \
        Corrade::Utility::Error() << message;                               \
        return returnValue;                                                 \
    }
#else
#ifdef CORRADE_NO_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue)
#else
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    if(!(condition)) {                                                      \
        Corrade::Utility::Error() << message;                               \
        exit(-1);                                                           \
        return returnValue;                                                 \
    }
#endif
#endif

}}

#endif
