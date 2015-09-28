#ifndef Corrade_Utility_Debug_h
#define Corrade_Utility_Debug_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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

#include <iosfwd>
#include <string>
#include <tuple>
#include <type_traits>

#include "Corrade/Utility/TypeTraits.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation { struct DebugOstreamFallback; }

/**
@brief Debug output handler

Provides convenient stream interface for passing data to debug output (standard
output). Data are separated with spaces and last value is enclosed with newline
character. Example usage:

@code
// Common usage
Debug() << "string" << 34 << 275.0f;

// Redirect debug output to string
std::ostringstream out;
Debug::setOutput(&out);
Debug() << "the meaning of life, universe and everything is" << 42;

// Mute debug output
Debug::setOutput(nullptr);
Debug() << "noone should see my ebanking password" << password;

// Reset debug output to default
Debug::setOutput();

// Conditional debug output (avoid inserting newline where it's not desired)
Debug d;
d << "Cannot foo";
if(bar)
    d << "because of bar.";
else
    d << "because of everything else.";
// (newline character will be written to output on object destruction)
@endcode

Support for printing more types can be added by implementing function
@ref operator<<(Debug&, const T&) for given type. If there is no `operator<<`
implemented for printing given type using Debug class, suitable `std::ostream`
`operator<<` overload is used as fallback, if found.

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
         */
        Debug(const Debug& other);

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
         * @see @ref operator<<(Debug&, const T&)
         */
        Debug& operator<<(const std::string& value);
        Debug& operator<<(const char* value);            /**< @overload */
        Debug& operator<<(const void* value);            /**< @overload */
        Debug& operator<<(bool value);                   /**< @overload */
        Debug& operator<<(int value);                    /**< @overload */
        Debug& operator<<(long value);                   /**< @overload */
        Debug& operator<<(long long value);              /**< @overload */
        Debug& operator<<(unsigned value);               /**< @overload */
        Debug& operator<<(unsigned long value);          /**< @overload */
        Debug& operator<<(unsigned long long value);     /**< @overload */
        Debug& operator<<(float value);                  /**< @overload */
        Debug& operator<<(double value);                 /**< @overload */
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        /** @overload
         * @partialsupport Not available in @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"
         *      as JavaScript doesn't support doubles larger than 64 bits.
         */
        Debug& operator<<(long double value);
        #endif

        /**
         * @brief Print UTF-32 character to debug output
         *
         * Prints value as Unicode codepoint, i.e. `U+0061`.
         */
        Debug& operator<<(char32_t value);

        /**
         * @brief Print UTF-32 character literal to debug output
         *
         * Prints value as list of Unicode codepoints, i.e.
         * `[U+0061, U+0062, U+0063}`.
         */
        Debug& operator<<(const char32_t* value);

        #ifndef DOXYGEN_GENERATING_OUTPUT
        Debug& operator<<(Implementation::DebugOstreamFallback&& value);
        #endif

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
        template<class T> Debug& print(const T& value);

        static std::ostream* globalOutput;
        int flags;
};

#ifndef DOXYGEN_GENERATING_OUTPUT
/* so Debug() << value works */
template<class T> inline Debug& operator<<(Debug&& debug, const T& value) {
    return debug << value;
}
#endif

#ifdef DOXYGEN_GENERATING_OUTPUT
/** @relates Debug
@brief Operator for printing custom types to debug
@param debug     Debug class
@param value     Value to be printed

Support for printing custom types (i.e. those not handled by `iostream`) can
be added by implementing this function for given type.

The function should convert the type to one of supported types (such as
`std::string`) and then call @ref Debug::operator<<(const std::string&) with
it. You can also use @ref Debug::setFlag() for modifying newline and whitespace
behavior.
 */
template<class T> Debug& operator<<(Debug& debug, const T& value);
#endif

/** @relates Debug
@brief Operator for printing iterable types to debug

Prints the value as `{a, b, c}`.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class Iterable> Debug& operator<<(Debug& debug, const Iterable& value)
#else
/* libc++ from Apple's Clang "4.2" (3.2-svn) doesn't have constexpr operator
   bool for std::integral_constant, thus we need to use ::value instead */
template<class Iterable> Debug& operator<<(typename std::enable_if<IsIterable<Iterable>::value && !std::is_same<Iterable, std::string>::value, Debug&>::type debug, const Iterable& value)
#endif
{
    const bool hadSpace = debug.flag(Debug::SpaceAfterEachValue);
    debug << "{";
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    for(auto it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << ", ";
        debug << *it;
    }
    debug << "}";
    debug.setFlag(Debug::SpaceAfterEachValue, hadSpace);
    return debug;
}

namespace Implementation {
    /** @todo C++14: use std::make_index_sequence and std::integer_sequence */
    template<std::size_t ...> struct Sequence {};

    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };
    #endif

    /* Used by operator<<(Debug&, std::tuple<>...) */
    template<class T> inline void tupleDebugOutput(Debug&, const T&, Sequence<>) {}
    template<class T, std::size_t i, std::size_t ...sequence> void tupleDebugOutput(Debug& debug, const T& tuple, Sequence<i, sequence...>) {
        debug << std::get<i>(tuple);
        if(i + 1 != std::tuple_size<T>::value)
            debug << ", ";
        tupleDebugOutput(debug, tuple, Sequence<sequence...>{});
    }
}

/** @relates Debug
@brief Operator for printing tuple types to debug

Prints the value as `(first, second, third...)`.
*/
template<class ...Args> Debug& operator<<(Debug& debug, const std::tuple<Args...>& value) {
    const bool hadSpace = debug.flag(Debug::SpaceAfterEachValue);
    debug << "(";
    debug.setFlag(Debug::SpaceAfterEachValue, false);
    Implementation::tupleDebugOutput(debug, value, typename Implementation::GenerateSequence<sizeof...(Args)>::Type{});
    debug << ")";
    debug.setFlag(Debug::SpaceAfterEachValue, hadSpace);
    return debug;
}

/** @relates Debug
 * @overload
 */
template<class T, class U> Debug& operator<<(Debug& debug, const std::pair<T, U>& value) {
    return debug << std::tuple<T, U>(value);
}

/**
@brief Warning output handler

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
@brief Error output handler

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

namespace Implementation {

/* Used by Debug::operator<<(Implementation::DebugOstreamFallback&&) */
struct DebugOstreamFallback {
    template<class T> /*implicit*/ DebugOstreamFallback(const T& t): applier(&DebugOstreamFallback::applyImpl<T>), value(&t) {}

    void apply(std::ostream& s) const {
        (this->*applier)(s);
    }

    template<class T> void applyImpl(std::ostream& s) const {
        s << *static_cast<const T*>(value);
    }

    using ApplierFunc = void(DebugOstreamFallback::*)(std::ostream&) const;
    const ApplierFunc applier;
    const void* value;
};

}

}}

#endif
