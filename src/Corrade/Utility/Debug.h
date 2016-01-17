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

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/TypeTraits.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation { struct DebugOstreamFallback; }

/**
@brief Debug output handler

Provides convenient stream interface for passing data to debug output (standard
output). Data are by default separated with spaces and last value is enclosed
with newline character. Example usage:

@code
// Common usage
Debug() << "string" << 34 << 275.0f;

// Redirect debug output to string
std::ostringstream out;
Debug(&out) << "the meaning of life, universe and everything is" << 42;

// Mute debug output
Debug(nullptr) << "no one should see my ebanking password" << password;

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

## Advanced usage

Sometimes you might not want to have everything separated by spaces or having
newline at the end:
@code
// Prints "Value: 16, 24"
Debug() << "Value:" << 16 << Debug::nospace << "," << 24;

// Prints "Value\n16"
Debug() << "Value:" << Debug::newline << 16;

// Doesn't output newline at the end
Debug::noNewlineAtTheEnd() << "Hello!";
@endcode

## Scoped output redirection

Output specified in class constructor is used for all instances created during
that instance lifetime. @ref Debug, @ref Warning and @ref Error classes outputs
can be controlled separately:

@code
std::ostringstream debugOut, errorOut;

Error() << "this is printed into std::cerr";

Error redirectError(&errorOut);

{
    Debug redirectDebug(&debugOut);

    Debug() << "this is printed into debugOut";
    Error() << "this is printed into errorOut";
    Debug() << "this is also printed into debugOut";
}

Debug() << "this is printed into std::cout again";
Error() << "this is still printed into errorOut";
@endcode

@see @ref Warning, @ref Error, @ref Fatal, @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
    @ref NaClConsoleStreamBuffer
@todo Output to more ostreams at once
 */
class CORRADE_UTILITY_EXPORT Debug {
    public:
        /**
         * @brief Debug output modifier
         *
         * @see @ref nospace(), @ref newline(), @ref operator<<(Modifier)
         */
        typedef void(*Modifier)(Debug&);

        /**
         * @brief Debug output without newline at the end
         *
         * Unlike @ref Debug() doesn't put newline at the end on destruction.
         * @see @ref noNewlineAtTheEnd(std::ostream*)
         */
        static Debug noNewlineAtTheEnd();

        /**
         * @brief Debug output without newline at the end
         *
         * Unlike @ref Debug(std::ostream*) doesn't put newline at the end on
         * destruction.
         * @see @ref noNewlineAtTheEnd()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline static Debug noNewlineAtTheEnd(std::ostream* output);

        /**
         * @brief Don't put space before next value
         *
         * Debug output by default separates values with space, this disables
         * it for the immediately following value. The default behavior is
         * then restored. The following line outputs `Value: 16, 24`:
         * @code
         * Debug() << "Value:" << 16 << Debug::nospace << "," << 24;
         * @endcode
         * @see @ref newline()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline static void nospace(Debug& debug);

        /**
         * @brief Output a newline
         *
         * Puts a newline (not surrounded by spaces) to the output. The
         * following two lines are equivalent:
         * @code
         * Debug() << "Value:" << Debug::newline << 16;
         * Debug() << "Value:" << Debug::nospace << "\n" << Debug::nospace << 16;
         * @endcode
         * and their output is
         *
         *      Value:
         *      16
         *
         * @see @ref nospace()
         */
        static void newline(Debug& debug) {
            debug << nospace << "\n" << nospace;
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief Set output for instances in this scope.
         * @deprecated Use @ref Debug(std::ostream*) instead.
         */
        CORRADE_DEPRECATED("use Debug(std::ostream*) instead") static void setOutput(std::ostream* output);
        #endif

        /**
         * @brief Default constructor
         *
         * Uses output of enclosing `Debug` instance or uses `std::cout` if
         * there isn't any.
         * @see @ref noNewlineAtTheEnd(), @ref setOutput()
         */
        explicit Debug();

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      `nullptr`, no debug output will be written anywhere.
         *
         * All new instances created using the default @ref Debug() constructor
         * during lifetime of this instance will inherit the output set in
         * @p output.
         * @see @ref noNewlineAtTheEnd(std::ostream*), @ref setOutput()
         */
        explicit Debug(std::ostream* output);

        /** @brief Copying is not allowed */
        Debug(const Debug&) = delete;

        /** @brief Move constructor */
        Debug(Debug&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output back to the output of enclosing scope. If there
         * was any output, adds newline at the end.
         */
        ~Debug();

        /** @brief Copying is not allowed */
        Debug& operator=(const Debug&) = delete;

        /** @brief Move assignment is not allowed */
        Debug& operator=(Debug&&) = delete;

        /**
         * @brief Print string to debug output
         *
         * If there is already something on the output, puts space before
         * the value, unless @ref nospace() was set immediately before.
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

        /**
         * @brief Debug output modification
         *
         * See @ref nospace() and @ref newline() for more information.
         */
        Debug& operator<<(Modifier f) {
            f(*this);
            return *this;
        }

        #ifndef DOXYGEN_GENERATING_OUTPUT
        Debug& operator<<(Implementation::DebugOstreamFallback&& value);
        #endif

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        std::ostream* _output;

        enum class InternalFlag: unsigned char {
            ValueWritten = 1 << 0,
            NoSpaceBeforeNextValue = 1 << 1,
            NoNewlineAtTheEnd = 1 << 2
        };
        typedef Containers::EnumSet<InternalFlag> InternalFlags;

        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        InternalFlags _flags;

    private:
        static std::ostream* _globalOutput;

        template<class T> Debug& print(const T& value);

        std::ostream* _previousGlobalOutput;
};

CORRADE_ENUMSET_OPERATORS(Debug::InternalFlags)

inline Debug Debug::noNewlineAtTheEnd(std::ostream* const output) {
    Debug debug{output};
    debug._flags |= InternalFlag::NoNewlineAtTheEnd;
    return debug;
}

inline void Debug::nospace(Debug& debug) {
    debug._flags |= InternalFlag::NoSpaceBeforeNextValue;
}

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
it. You can also use @ref Debug::nospace() and @ref Debug::newline().
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
    debug << "{" << Debug::nospace;
    for(auto it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << Debug::nospace << ",";
        debug << *it;
    }
    debug << Debug::nospace << "}";
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
            debug << Debug::nospace << ",";
        tupleDebugOutput(debug, tuple, Sequence<sequence...>{});
    }
}

/** @relates Debug
@brief Operator for printing tuple types to debug

Prints the value as `(first, second, third...)`.
*/
template<class ...Args> Debug& operator<<(Debug& debug, const std::tuple<Args...>& value) {
    debug << "(" << Debug::nospace;
    Implementation::tupleDebugOutput(debug, value, typename Implementation::GenerateSequence<sizeof...(Args)>::Type{});
    debug << Debug::nospace << ")";
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
        /**
         * @brief Warning output without newline at the end
         *
         * Unlike @ref Warning() doesn't put newline at the end on destruction.
         * @see @ref noNewlineAtTheEnd(std::ostream*)
         */
        static Warning noNewlineAtTheEnd();

        /**
         * @brief Warning output without newline at the end
         *
         * Unlike @ref Warning(std::ostream*) doesn't put newline at the end on
         * destruction.
         * @see @ref noNewlineAtTheEnd()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline static Warning noNewlineAtTheEnd(std::ostream* output);

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief Set output for instances in this scope.
         * @deprecated Use @ref Warrning(std::ostream*) instead.
         */
        CORRADE_DEPRECATED("use Debug(std::ostream*) instead") static void setOutput(std::ostream* output);
        #endif

        /**
         * @brief Default constructor
         *
         * Inherits output of enclosing `Warning` instance or uses `std::cerr`
         * if there isn't any.
         * @see @ref noNewlineAtTheEnd()
         */
        explicit Warning();

        /**
         * @brief Constructor
         * @param output        Stream where to put warning output. If set to
         *      `nullptr`, no warning output will be written anywhere.
         *
         * All new instances created using the default @ref Warning()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         * @see @ref noNewlineAtTheEnd(std::ostream*)
         */
        explicit Warning(std::ostream* output);

        /** @brief Copying is not allowed */
        Warning(const Warning&) = delete;

        /** @brief Move constructor */
        Warning(Warning&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output back to the output of enclosing scope. If there
         * was any output, adds newline at the end.
         */
        ~Warning();

        /** @brief Copying is not allowed */
        Warning& operator=(const Warning&) = delete;

        /** @brief Move assignment is not allowed */
        Warning& operator=(Warning&&) = delete;

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* _globalWarningOutput;
        std::ostream* _previousGlobalWarningOutput;
};

inline Warning Warning::noNewlineAtTheEnd(std::ostream* const output) {
    Warning warning{output};
    warning._flags |= InternalFlag::NoNewlineAtTheEnd;
    return std::move(warning);
}

/**
@brief Error output handler

@copydetails Warning
@see @ref Fatal
*/
class CORRADE_UTILITY_EXPORT Error: public Debug {
    friend Fatal;

    public:
        /**
         * @brief Error output without newline at the end
         *
         * Unlike @ref Error() doesn't put newline at the end on destruction.
         * @see @ref noNewlineAtTheEnd(std::ostream*)
         */
        static Error noNewlineAtTheEnd();

        /**
         * @brief Error output without newline at the end
         *
         * Unlike @ref Error(std::ostream*) doesn't put newline at the end on
         * destruction.
         * @see @ref noNewlineAtTheEnd()
         */
        /* MinGW complains loudly if the declaration doesn't also have inline */
        inline static Error noNewlineAtTheEnd(std::ostream* output);

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @brief Set output for instances in this scope.
         * @deprecated Use @ref Error(std::ostream*) instead.
         */
        CORRADE_DEPRECATED("use Error(std::ostream*) instead") static void setOutput(std::ostream* output);
        #endif

        /**
         * @brief Default constructor
         *
         * Inherits output of enclosing `Error` instance or uses `std::cerr` if
         * there isn't any.
         * @see @ref noNewlineAtTheEnd()
         */
        explicit Error();

        /**
         * @brief Constructor
         * @param output        Stream where to put error output. If set to
         *      `nullptr`, no error output will be written anywhere.
         *
         * All new instances created using the default @ref Error()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         * @see @ref noNewlineAtTheEnd(std::ostream*)
         */
        explicit Error(std::ostream* output);

        /** @brief Copying is not allowed */
        Error(const Error&) = delete;

        /** @brief Move constructor */
        Error(Error&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output back to the output of enclosing scope. If there
         * was any output, adds newline at the end.
         */
        ~Error();

        /** @brief Copying is not allowed */
        Error& operator=(const Error&) = delete;

        /** @brief Move assignment is not allowed */
        Error& operator=(Error&&) = delete;

    private:
        static CORRADE_UTILITY_LOCAL std::ostream* _globalErrorOutput;
        std::ostream* _previousGlobalErrorOutput;
};

inline Error Error::noNewlineAtTheEnd(std::ostream* const output) {
    Error error{output};
    error._flags |= InternalFlag::NoNewlineAtTheEnd;
    return std::move(error);
}

/**
@brief Warning output handler

Equivalent to @ref Error, but exits with defined exit code on destruction. So
instead of this:
@code
if(stuff.broken()) {
    Error() << "Everything's broken, exiting.";
    std::exit(42);
}
@endcode
You can write just this:
@code
if(stuff.broken())
    Fatal(42) << "Everything's broken, exiting.";
@endcode

As the message produced by this class is the last that the program writes,
there is no need for ability to disable the newline at the end (it also made
the implementation much simpler).
*/
class CORRADE_UTILITY_EXPORT Fatal: public Error {
    public:
        /**
         * @brief Constructor
         *
         * Sets output to `std::cerr`. The @p exitcode is passed to `std::exit()`
         * on destruction.
         * @see @ref noNewlineAtTheEnd()
         */
        Fatal(int exitCode = 1): _exitCode{exitCode} {}

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      `nullptr`, no debug output will be written anywhere.
         * @param exitCode      Application exit code to be used on destruction
         */
        Fatal(std::ostream* output, int exitCode = 1): Error{output}, _exitCode{exitCode} {}

        /**
         * @brief Destructor
         *
         * Exits the application with exit code specified in constructor.
         */
        ~Fatal();

    private:
        using Error::noNewlineAtTheEnd;

        int _exitCode;
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
