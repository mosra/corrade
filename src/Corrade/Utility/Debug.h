#ifndef Corrade_Utility_Debug_h
#define Corrade_Utility_Debug_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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

/** @file
 * @brief Class @ref Corrade::Utility::Debug, @ref Corrade::Utility::Warning, @ref Corrade::Utility::Error
 */

#include <functional>
#include <iosfwd>
#include <utility>
#include <type_traits>

#include "Corrade/configure.h"
#include "Corrade/Utility/TypeTraits.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief %Debug output handler

Provides convenient stream interface for passing data to debug output (standard
output). Data are separated with spaces and last value is enclosed with newline
character. Example usage:

@code
// Common usage
Debug() << "string" << 34 << 275.0f;

// Redirect debug output to string
std::ostringstream output;
Debug::setOutput(&o);
Debug() << "the meaning of life, universe and everything is" << 42;

// Mute debug output
Debug::setOutput(nullptr);
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
can be added by implementing function `operator<<(Debug, const T&)` for given
type.

@see @ref Warning, @ref Error, @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
    @ref NaClConsoleStreamBuffer
@todo Output to more ostreams at once
 */
class CORRADE_UTILITY_EXPORT Debug {
    Debug& operator=(const Debug& other) = delete;
    Debug& operator=(Debug&& other) = delete;

    public:
        /** @brief Output flags */
        enum Flag {
            /* 0x01 reserved for indicating that no value was yet written */

            /** Put space after each value (enabled by default) */
            SpaceAfterEachValue = 0x02,

            /** Put newline at the end (enabled by default) */
            NewLineAtTheEnd = 0x04
        };

        /**
         * @brief Constructor
         *
         * Sets output to `std::cout`.
         * @see @ref setOutput()
         */
        explicit Debug();

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      `nullptr`, no debug output will be written anywhere.
         *
         * Constructs debug object with given output.
         * @see @ref setOutput()
         */
        explicit Debug(std::ostream* output): output(output), flags(0x01 | SpaceAfterEachValue | NewLineAtTheEnd) {}

        /**
         * @brief Copy constructor
         *
         * When copied from class which already wrote anything on the output,
         * disabling flag @ref Flag "Flag::NewLineAtTheEnd", so there aren't
         * excessive newlines in the output.
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
         * @ref Flag "Debug::NewLineAtTheEnd" the same way as in
         * @ref Debug(const Debug&).
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
         * @see @ref Flag
         */
        ~Debug();

        /** @brief Flag */
        bool flag(Flag flag) const { return flags & flag; }

        /** @brief Set flag */
        void setFlag(Flag flag, bool value);

        /**
         * @brief Print string to debug output
         *
         * If there is already something on the output, puts space before
         * the value.
         * @see @ref operator<<(Debug, const T&)
         */
        Debug operator<<(const std::string& value);
        Debug operator<<(const char* value);            /**< @overload */
        Debug operator<<(const void* value);            /**< @overload */
        Debug operator<<(bool value);                   /**< @overload */
        Debug operator<<(int value);                    /**< @overload */
        Debug operator<<(long value);                   /**< @overload */
        Debug operator<<(long long value);              /**< @overload */
        Debug operator<<(unsigned value);               /**< @overload */
        Debug operator<<(unsigned long value);          /**< @overload */
        Debug operator<<(unsigned long long value);     /**< @overload */
        Debug operator<<(float value);                  /**< @overload */
        Debug operator<<(double value);                 /**< @overload */
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        /** @overload
         * @partialsupport Not available in @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"
         *      as JavaScript doesn't support doubles larger than 64 bits.
         */
        Debug operator<<(long double value);
        #endif

        /**
         * @brief Print UTF-32 character to debug output
         *
         * Prints value as %Unicode codepoint, i.e. `U+0061`.
         */
        Debug operator<<(char32_t value);

        /**
         * @brief Print UTF-32 character literal to debug output
         *
         * Prints value as list of %Unicode codepoints, i.e.
         * `[U+0061, U+0062, U+0063}`.
         */
        Debug operator<<(const char32_t* value);        /**< @overload */

        struct Fallback {
            // Implicit construction is desired.
            template <class T>
            Fallback(const T& t)
                : apply([&] (std::ostream& s) { s << t; })
            { }

            std::function<void(std::ostream&)> apply;
        };

        Debug operator<<(Fallback value);

        /**
         * @brief Globally set output for newly created instances
         * @param output       Stream where to put debug output. If set to
         *      `nullptr`, no debug output will be written anywhere.
         *
         * All successive @ref Debug instances created with default constructor
         * will be redirected to given stream.
         */
        static void setOutput(std::ostream* output);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        std::ostream* output;

    private:
        template<class T> Debug print(const T& value);

        static std::ostream* globalOutput;
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
`std::string`) and then call @ref Debug::operator<<(const std::string&) with
it. You can also use @ref Debug::setFlag() for modifying newline and whitespace
behavior.
 */
template<class T> Debug operator<<(Debug debug, const T& value);
#endif

/** @relates Debug
@brief Operator for printing iterable types to debug

Prints the value as `{a, b, c}`.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class Iterable> Debug operator<<(Debug debug, const Iterable& value)
#else
/* libc++ from Apple's Clang "4.2" (3.2-svn) doesn't have constexpr operator
   bool for std::integral_constant, thus we need to use ::value instead */
template<class Iterable> Debug operator<<(typename std::enable_if<IsIterable<Iterable>::value && !std::is_same<Iterable, std::string>::value, Debug>::type debug, const Iterable& value)
#endif
{
    debug << "{";
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    for(auto it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << ", ";
        debug << *it;
    }
    debug << "}";
    debug.setFlag(Debug::SpaceAfterEachValue, true);
    return debug;
}

/** @relates Debug
@brief Operator for printing pair types to debug

Prints the value as `(first, second)`.
*/
template<class A, class B> Debug operator<<(Debug debug, const std::pair<A, B>& value) {
    debug << "(";
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    debug << value.first << ", " << value.second << ")";
    debug.setFlag(Debug::SpaceAfterEachValue, true);
    return debug;
}

/**
@brief %Warning output handler

Same as @ref Debug, but by default writes output to standard error output.
Thus it is possible to separate / mute @ref Debug, @ref Warning and @ref Error
outputs.
*/
class CORRADE_UTILITY_EXPORT Warning: public Debug {
    public:
        /** @copydoc Debug::setOutput() */
        static void setOutput(std::ostream* output);

        /**
         * @brief Constructor
         *
         * Sets output to `std::cerr`.
         * @see @ref setOutput()
         */
        explicit Warning();

        /** @copydoc Debug::Debug() */
        explicit Warning(std::ostream* output): Debug(output) {}

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* globalWarningOutput;
};

/**
@brief %Error output handler

@copydetails Warning
*/
class CORRADE_UTILITY_EXPORT Error: public Debug {
    public:
        /** @copydoc Debug::setOutput() */
        static void setOutput(std::ostream* output);

        /**
         * @brief Constructor
         *
         * Sets output to `std::cerr`.
         * @see @ref setOutput()
         */
        Error();

        /** @copydoc Debug::Debug() */
        Error(std::ostream* output): Debug(output) {}

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* globalErrorOutput;
};

}}

#endif
