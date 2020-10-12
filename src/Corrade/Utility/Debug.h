#ifndef Corrade_Utility_Debug_h
#define Corrade_Utility_Debug_h
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
 * @brief Class @ref Corrade::Utility::Debug, @ref Corrade::Utility::Warning, @ref Corrade::Utility::Error, @ref Corrade::Utility::Fatal
 * @see @ref Corrade/Utility/DebugStl.h
 */

#include <iosfwd>
#include <type_traits>
#include <utility> /** @todo consider putting this away as well (900 LOC) */

#include "Corrade/Utility/Macros.h"
#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/TypeTraits.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Source location support in debug output
@m_since{2020,06}

Defined if @ref Corrade::Utility::Debug "Utility::Debug" is able to print
source location support. Available on GCC at least since version 4.8, Clang 9+
and MSVC 2019 16.6 and newer. See @ref Utility-Debug-source-location for more
information.
*/
/* To distinguish Apple Clang (9.0 will hopefully be Xcode 12), using
   __apple_build_version__ according to https://stackoverflow.com/a/19391724 */
#if defined(DOXYGEN_GENERATING_OUTPUT) || (defined(__GNUC__) && !defined(__clang__)) || (defined(__clang__) && ((defined(__apple_build_version__) && __clang_major__ >= 12) || (!defined(__apple_build_version__) && __clang_major__ >= 9))) || (defined(_MSC_VER) && _MSC_VER >= 1926)
#define CORRADE_UTILITY_DEBUG_HAS_SOURCE_LOCATION
namespace Implementation { struct DebugSourceLocation; }
#endif

/**
@brief Debug output handler

Provides convenient stream interface for passing data to debug output (standard
output). Data are by default separated with spaces and last value is enclosed
with newline character. Example usage:

@snippet Utility.cpp Debug-usage

Support for printing more types can be added by implementing an overload of
@ref operator<<(Debug&, const T&) for given type.

@section Utility-Debug-stl Printing STL types

To optimize compile times, the @ref Corrade/Utility/Debug.h header provides
only support for printing builtin types, generic iterable containers and
@ref std::pair. Printing of @ref std::string and @ref std::tuple is possible if
you @cpp #include @ce a separate @ref Corrade/Utility/DebugStl.h header. This
header also provides a fallback to @ref std::ostream @cpp operator<<() @ce
overloads, if there's no @cpp operator<<() @ce implemented for printing given
type using @ref Debug. Note that printing @ref std::vector or @ref std::map
containers is already possible with the generic iterable container support in
@ref Corrade/Utility/Debug.h.

@section Utility-Debug-scoped-output Scoped output redirection

Output specified in class constructor is used for all instances created during
that instance lifetime. @ref Debug, @ref Warning and @ref Error classes outputs
can be controlled separately:

@snippet Utility.cpp Debug-scoped-output

@section Utility-Debug-modifiers Output modifiers

It's possible to modify the output behavior by calling @ref setFlags() or
@ref setImmediateFlags(). The latter function applies given flag only to the
immediately following value (and then it gets reset back) instead of all
following values. For convenience, the operation of @ref setFlags() /
@ref setImmediateFlags() can be done or by passing a special function to the
output stream.

@subsection Utility-Debug-modifiers-whitespace Explicit whitespace control

Sometimes you might not want to have everything separated by spaces or having
newline at the end --- use @ref Flag::NoNewlineAtTheEnd or the @ref nospace
modifier:

@snippet Utility.cpp Debug-modifiers-whitespace

@subsection Utility-Debug-modifiers-colors Colored output

It is possible to color the output using @ref color() and @ref boldColor(). The
color is automatically reset to previous value on destruction to avoid messing
up the terminal, you can also use @ref resetColor() to reset it explicitly.

@snippet Utility.cpp Debug-modifiers-colors

@include UtilityDebug-color.ansi

On POSIX the coloring is done using ANSI color escape sequences and works both
when outputting to a terminal or any other stream. On Windows, by default due
to a platform limitation, the colored output works only when outputting
directly to a terminal without any intermediate buffer. See
@ref CORRADE_UTILITY_USE_ANSI_COLORS for possible alternative.

Note that colors make sense only when they finally appear in a terminal and not
when redirecting output to file. You can control this by setting
@ref Flag::DisableColors based on value of @ref isTty(), for example:

@snippet Utility.cpp Debug-modifiers-colors-disable

Similarly as with scoped output redirection, colors can be also scoped:

@snippet Utility.cpp Debug-modifiers-colors-scoped

With @ref Flag::Color and/or the @ref color modifier, some types can be also
printed as actual 24bit colors. The @ref operator<<(unsigned char) printer can
interpret 8bit values as greyscale colors and other libraries may provide
support for other types. For example, printing a container of 8bit values
together with the @ref packed modifier:

@snippet Utility.cpp Debug-color

This prints the below output on terminals that support it. See the operator
documentation for more information.

@include UtilityDebug-color-greyscale.ansi

@section Utility-Debug-source-location Source location

Similarly to the [dbg! macro in Rust](https://blog.rust-lang.org/2019/01/17/Rust-1.32.0.html#the-dbg-macro),
on supported compilers the utility is able to print source file location and
line where the debug output was executed, improving the "printf debugging"
experience. By default no source location info is printed, in order to do that
prefix the @ref Debug instantiation with an exclamation mark. Additionally,
an otherwise unused exclamated instantiation prints just the file + line alone
(in contrast to unexclamated instantiaton, which is a no-op):

@snippet Utility.cpp Debug-source-location

The above code then may print something like this:

@code{.shell-session}
main.cpp:10: the result is 42
main.cpp:11: the result is 5.25
main.cpp:13
and finally, 42
@endcode

At the moment, this feature is available on GCC at least since version 4.8 and
Clang 9+, elsewhere it behaves like the unexclamated version. You can check for
its availability using the @ref CORRADE_UTILITY_DEBUG_HAS_SOURCE_LOCATION
predefined macro.

@section Utility-Debug-windows ANSI color support and UTF-8 output on Windows

See the @ref main "Corrade::Main" library for more information about a
convenient way to support ANSI colors and UTF-8 output encoding on Windows.

@section Utility-Debug-multithreading Thread safety

If Corrade is compiled with @ref CORRADE_BUILD_MULTITHREADED enabled (the
default), scoped output redirection and coloring is done thread-locally. This
might cause some performance penalties --- if you are sure that you will never
need to handle these per-thread (and won't need any other functionality enabled
by this option either), build Corrade with the option disabled.

@see @ref Warning, @ref Error, @ref Fatal, @ref CORRADE_ASSERT(),
    @ref CORRADE_INTERNAL_ASSERT(), @ref CORRADE_INTERNAL_ASSERT_OUTPUT(),
    @ref AndroidLogStreamBuffer, @ref formatString()
@todo Output to more ostreams at once
 */
class CORRADE_UTILITY_EXPORT Debug {
    public:
        /**
         * @brief Debug output flag
         * @m_since{2019,10}
         *
         * @see @ref Flags, @ref Debug(Flags)
         */
        enum class Flag: unsigned char {
            /** Don't put newline at the end on destruction */
            NoNewlineAtTheEnd = 1 << 0,

            /**
             * Disable colored output in @ref color(), @ref boldColor() and
             * @ref resetColor().
             * @see @ref isTty()
             * @note Note that on @ref CORRADE_TARGET_WINDOWS "Windows" the
             *      colored output by default works only if outputting directly
             *      to the console. See also @ref CORRADE_UTILITY_USE_ANSI_COLORS.
             */
            DisableColors = 1 << 1,

            /**
             * Print without spaces between values.
             * @see @ref nospace, @ref space
             */
            NoSpace = 1 << 2,

            /**
             * Print complex values (such as containers) in a packed form.
             * @see @ref packed, @ref operator<<(Debug&, const Iterable&)
             */
            Packed = 1 << 3,

            /**
             * Print colored values as colored squares in the terminal.
             * @see @ref color, @ref operator<<(unsigned char)
             */
            Color = 1 << 4

            /* When adding values, don't forget to adapt InternalFlag as well
               and update PublicFlagMask in Debug.cpp */
        };

        /**
         * @brief Debug output flags
         * @m_since{2019,10}
         *
         * @see @ref Debug(Flags)
         */
        typedef Containers::EnumSet<Flag> Flags;

        /** @{ @name Output modifiers
         * See @ref Utility-Debug-modifiers for more information.
         */

        /**
         * @brief Debug output modifier
         *
         * @see @ref nospace, @ref newline, @ref space,
         *      @ref operator<<(Modifier)
         */
        typedef void(*Modifier)(Debug&);

        /**
         * @brief Output color
         *
         * @see @ref color(), @ref boldColor()
         */
        enum class Color: char {
            /** Black */
            Black = 0,

            /** Red */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Red = 1,
            #else
            Red = 4,
            #endif

            /** Green */
            Green = 2,

            /** Yellow */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Yellow = 3,
            #else
            Yellow = 6,
            #endif

            /** Blue */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Blue = 4,
            #else
            Blue = 1,
            #endif

            /** Magenta */
            Magenta = 5,

            /** Cyan */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Cyan = 6,
            #else
            Cyan = 3,
            #endif

            /** White */
            White = 7,

            /** Default (implementation/style-defined) */
            #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
            Default = 9
            #else
            Default = 7
            #endif
        };

        /**
         * @brief Don't put space before next value
         *
         * Debug output by default separates values with space, this disables
         * it for the immediately following value. The default behavior is
         * then restored. The following line outputs
         * @cb{.shell-session} Value: 16, 24 @ce:
         *
         * @snippet Utility.cpp Debug-nospace
         *
         * @see @ref Flag::NoSpace, @ref space, @ref newline
         */
        static void nospace(Debug& debug) {
            debug._immediateFlags |= InternalFlag::NoSpace;
        }

        /**
         * @brief Output a newline
         *
         * Puts a newline (not surrounded by spaces) to the output. The
         * following two lines are equivalent:
         *
         * @snippet Utility.cpp Debug-newline
         *
         * and their output is
         *
         * @code{.shell-session}
         * Value:
         * 16
         * @endcode
         *
         * @see @ref nospace
         */
        static void newline(Debug& debug) {
            debug << nospace << "\n" << nospace;
        }

        /**
         * @brief Output a space
         * @m_since{2020,06}
         *
         * Puts a space (not surrounded by additional spaces) to the output.
         * Useful for adding an explicit leading space or for delimiting values
         * with spaces when @ref Flag::NoSpace is set. The last two lines are
         * equivalent:
         *
         * @snippet Utility.cpp Debug-space
         *
         * and the output is
         *
         * @code{.shell-session}
         * Value:
         *  16
         * @endcode
         *
         * @see @ref nospace, @ref newline
         */
        static void space(Debug& debug) {
            debug << nospace << " " << nospace;
        }

        /**
         * @brief Set output color
         *
         * Resets previous @ref color() or @ref boldColor() setting. The color
         * is also automatically reset on object destruction to a value that
         * was active in outer scope. If @ref Flag::DisableColors was set, this
         * function does nothing.
         */
        static Modifier color(Color color);

        /**
         * @brief Set bold output color
         *
         * Resets previous @ref color() or @ref boldColor() setting. The color
         * is also automatically reset on object destruction to a value that
         * was active in outer scope. If @ref Flag::DisableColors was set, this
         * function does nothing.
         */
        static Modifier boldColor(Color color);

        /**
         * @brief Reset output color
         *
         * Resets any previous @ref color() or @ref boldColor() setting to a
         * value that was active in outer scope. The same is also automatically
         * done on object destruction. If the color was not changed by this
         * instance or @ref Flag::DisableColors was set, this function does
         * nothing.
         */
        static void resetColor(Debug& debug);

        /**
         * @brief Print the next value in a packed form
         * @m_since{2019,10}
         *
         * Enables a more compact output for types that support it (such as
         * iterable containers).
         * @see @ref Flag::Packed, @ref operator<<(Debug&, const Iterable&)
         */
        static void packed(Debug& debug) {
            debug._immediateFlags |= InternalFlag::Packed;
        }

        /**
         * @brief Print the next value as a color
         * @m_since{2019,10}
         *
         * Prints color-like values as actual 24bit ANSI color sequences.
         * @see @ref Flag::Color, @ref operator<<(unsigned char)
         */
        static void color(Debug& debug) {
            debug._immediateFlags |= InternalFlag::Color;
        }

        /**
         * @brief Debug output modification
         *
         * See @ref Utility-Debug-modifiers for more information.
         */
        Debug& operator<<(Modifier f) {
            f(*this);
            return *this;
        }

        /* Since 1.8.17, the original short-hand group closing doesn't work
           anymore. FFS. */
        /**
         * @}
         */

        /**
         * @brief Default debug output stream
         * @m_since{2019,10}
         *
         * Debug output when no output redirection happens. A pointer to
         * @ref std::cout.
         * @see @ref output()
         */
        static std::ostream* defaultOutput();

        /**
         * @brief Current debug output stream
         *
         * Debug output constructed with the @ref Debug(Flags) constructor will
         * be using this output stream.
         * @see @ref defaultOutput()
         */
        static std::ostream* output();

        /**
         * @brief Whether given output stream is a TTY
         *
         * Useful for deciding whether to use ANSI colored output using
         * @ref Flag::DisableColors. Returns @cpp true @ce if @p output is a
         * pointer to @ref std::cout / @ref std::cerr and the stream is not
         * redirected to a file, @cpp false @ce otherwise. Calls @cpp isatty() @ce
         * on Unix-like systems and Windows with @ref CORRADE_UTILITY_USE_ANSI_COLORS
         * enabled, calls Windows APIs if @ref CORRADE_UTILITY_USE_ANSI_COLORS
         * is disabled. On platforms without @cpp isatty() @ce equivalent
         * returns always @cpp false @ce.
         *
         * @note Returns @cpp false @ce when running inside Xcode even though
         *      @cpp isatty() @ce reports a positive value, because Xcode is
         *      not able to handle ANSI colors inside the output view.
         * @note Uses Node.js @cb{.js} process.stdout.isTTY @ce /
         *      @cb{.js} process.stderr.isTTY @ce instead of @cpp isatty() @ce
         *      on @ref CORRADE_TARGET_EMSCRIPTEN "Emscripten" because
         *      @cpp isatty() @ce is not able to detect file redirection.
         */
        static bool isTty(std::ostream* output);

        /**
         * @brief Whether current debug output is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing @ref Debug
         * instance or with @ref std::cerr if there isn't any.
         * @see @ref Warning::isTty(), @ref Error::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Uses output of enclosing @ref Debug instance or uses @ref std::cout
         * if there isn't any.
         */
        explicit Debug(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      @cpp nullptr @ce, no debug output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Debug() constructor
         * during lifetime of this instance will inherit the output set in
         * @p output.
         */
        explicit Debug(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Debug(const Debug&) = delete;

        /** @brief Move constructor */
        Debug(Debug&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Debug();

        /** @brief Copying is not allowed */
        Debug& operator=(const Debug&) = delete;

        /** @brief Move assignment is not allowed */
        Debug& operator=(Debug&&) = delete;

        /**
         * @brief Flags applied for all following values
         * @m_since{2019,10}
         *
         * @see @ref Utility-Debug-modifiers, @ref immediateFlags()
         */
        Flags flags() const;

        /**
         * @brief Set flags applied for all following values
         * @m_since{2019,10}
         *
         * @see @ref Utility-Debug-modifiers, @ref setImmediateFlags()
         */
        void setFlags(Flags flags);

        /**
         * @brief Flags applied for the immediately following value
         * @m_since{2019,10}
         *
         * Returned value is a combination of @ref flags() and immediate flags.
         * The immediate part gets reset after a value is printed.
         * @see @ref Utility-Debug-modifiers
         */
        Flags immediateFlags() const;

        /**
         * @brief Set flags to be applied for the immediately following value
         * @m_since{2019,10}
         *
         * Unlike flags set with @ref setFlags(), these get applied only to the
         * immediately following value and reset after.
         * @see @ref Utility-Debug-modifiers, @ref nospace
         */
        void setImmediateFlags(Flags flags);

        /**
         * @brief Print string to debug output
         *
         * If there is already something in the output, puts a space before
         * the value, unless @ref nospace was set immediately before.
         * @see @ref operator<<(Debug&, const std::string&),
         *      @ref operator<<(Debug&, const T&)
         */
        Debug& operator<<(const char* value);

        /**
         * @overload
         * @m_since_latest
         */
        Debug& operator<<(Containers::StringView value);

        /* Unfortunately we can't have just a StringView overload because
           when StringStl.h is included, printing a String also matches
           operator<<(DebugOstreamFallback&&), causing ambiguity. And when we
           have a String overload, we need a MutableStringView one as well,
           because otherwise there's an ambiguity between StringView and
           String. Sigh. */

        /**
         * @overload
         * @m_since_latest
         */
        Debug& operator<<(Containers::MutableStringView value);

        /**
         * @overload
         * @m_since_latest
         */
        Debug& operator<<(const Containers::String& value);

        /**
         * @brief Print a pointer value to debug output
         *
         * The value is printed in lowercase hexadecimal, for example
         * @cb{.shell-session} 0xdeadbeef @ce.
         */
        Debug& operator<<(const void* value);            /**< @overload */

        /**
         * @brief Print a boolean value to debug output
         *
         * The value is printed as literal @cb{.shell-session} true @ce or
         * @cb{.shell-session} false @ce.
         */
        Debug& operator<<(bool value);

        /**
         * @brief Print char to debug output
         *
         * Printed as a numeric value.
         */
        Debug& operator<<(char value);

        /**
         * @brief Print unsigned char to debug output
         *
         * If @ref Flag::Color is enabled or @ref color was set immediately
         * before, prints the value as a greyscale ANSI 24bit color escape
         * sequence using two successive Unicode block characters (to have it
         * roughly square). To preserve at least some information when text is
         * copied, the square consists of one of the five
         * @cb{.shell-session} ░▒▓█ @ce shades, however the color is set for
         * both foreground and background so the actual block character is
         * indistinguishable when seen on a terminal. See the
         * @ref Utility-Debug-modifiers-colors "class documentation" for more
         * information.
         *
         * If @ref Flag::Color is enabled and @ref Flag::DisableColors is set,
         * only the shaded character is used, without any ANSI color escape
         * sequence.
         *
         * If @ref Flag::Color is not enabled, the value is printed as a
         * number.
         */
        Debug& operator<<(unsigned char value);

        Debug& operator<<(int value);                    /**< @overload */
        Debug& operator<<(long value);                   /**< @overload */
        Debug& operator<<(long long value);              /**< @overload */
        Debug& operator<<(unsigned value);               /**< @overload */
        Debug& operator<<(unsigned long value);          /**< @overload */
        Debug& operator<<(unsigned long long value);     /**< @overload */

        /**
         * @brief Print `float` value to debug output
         *
         * Prints the value with 6 significant digits.
         */
        Debug& operator<<(float value);

        /**
         * @brief Print `double` value to debug output
         *
         * Prints the value with 15 significant digits.
         */
        Debug& operator<<(double value);

        /**
         * @brief Print `long double` value to debug output
         *
         * Prints the value with 18 significant digits on platforms with 80-bit
         * @cpp long double @ce and 15 digits on platforms
         * @ref CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE "where it is 64-bit".
         */
        Debug& operator<<(long double value);

        /**
         * @brief Print UTF-32 character to debug output
         *
         * Prints the value as Unicode codepoint, i.e. @cb{.shell-session} U+0061 @ce.
         */
        Debug& operator<<(char32_t value);

        /**
         * @brief Print UTF-32 character literal to debug output
         *
         * Prints the value as list of Unicode codepoints, i.e.
         * @cb{.shell-session} {U+0061, U+0062, U+0063} @ce.
         */
        Debug& operator<<(const char32_t* value);

        /**
         * @brief Print a nullptr to debug output
         *
         * Prints the value as @cb{.shell-session} nullptr @ce.
         */
        Debug& operator<<(std::nullptr_t);

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #endif
        /* Used by the out-of-class operator<<(Debug&, const std::string&) and
           operator<<(Debug&, Implementation::DebugOstreamFallback&&) which is
           defined in DebugStl.h. */
        template<class T> CORRADE_UTILITY_LOCAL Debug& print(const T& value);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        std::ostream* _output;

        enum class InternalFlag: unsigned char {
            /* Values compatible with Flag enum */
            NoNewlineAtTheEnd = 1 << 0,
            DisableColors = 1 << 1,
            NoSpace = 1 << 2,
            Packed = 1 << 3,
            Color = 1 << 4,
            ValueWritten = 1 << 5,
            ColorWritten = 1 << 6
        };
        typedef Containers::EnumSet<InternalFlag> InternalFlags;

        CORRADE_ENUMSET_FRIEND_OPERATORS(InternalFlags)

        CORRADE_UTILITY_LOCAL void cleanupOnDestruction(); /* Needed for Fatal */

        InternalFlags _flags;
        InternalFlags _immediateFlags;

    private:
        #ifdef CORRADE_UTILITY_DEBUG_HAS_SOURCE_LOCATION
        friend Implementation::DebugSourceLocation;
        #endif

        template<Color c, bool bold> CORRADE_UTILITY_LOCAL static Modifier colorInternal();

        CORRADE_UTILITY_LOCAL void resetColorInternal();

        std::ostream* _previousGlobalOutput;
        #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        unsigned short _previousColorAttributes = 0xffff;
        #else
        Color _previousColor;
        bool _previousColorBold;
        #endif
        #ifdef CORRADE_UTILITY_DEBUG_HAS_SOURCE_LOCATION
        const char* _sourceLocationFile{};
        int _sourceLocationLine{};
        #endif
};

/** @debugoperatorclassenum{Debug,Debug::Color} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Debug::Color value);

/** @debugoperatorclassenum{Debug,Debug::Flag} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Debug::Flag value);

/** @debugoperatorclassenum{Debug,Debug::Flags} */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Debug::Flags value);

CORRADE_ENUMSET_OPERATORS(Debug::Flags)

#ifdef CORRADE_UTILITY_DEBUG_HAS_SOURCE_LOCATION
namespace Implementation {
    struct CORRADE_UTILITY_EXPORT DebugSourceLocation {
        #if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
        /* Not using std::experimental::source_location because it's not in
           libc++ 9 yet and GCC version has a C++14 usage of constexpr */
        /*implicit*/ DebugSourceLocation(Debug&& debug, const char* file = __builtin_FILE(), int line = __builtin_LINE());
        #else
        #error this needs to be implemented for new compilers
        #endif
        Debug* debug;
    };
}

/** @relatesalso Debug
@brief Prefix the output with source location
@m_since{2020,06}

Only on supported compilers, does nothing otherwise. See
@ref Utility-Debug-source-location for more information.
*/
/* Unfortunately it's not possible to add additional (default) arguments to
   operator! so we need to use a implicitly convertible type and capture the
   source location in its constructor */
inline Debug& operator!(Implementation::DebugSourceLocation debug) {
    return *debug.debug;
}
#else
inline Debug& operator!(Debug&& debug) { return debug; }
#endif

#ifndef DOXYGEN_GENERATING_OUTPUT
/* so Debug() << value works */
template<class T> inline Debug& operator<<(Debug&& debug, const T& value) {
    return debug << value;
}
#endif

#ifdef DOXYGEN_GENERATING_OUTPUT
/** @relatesalso Debug
@brief Operator for printing custom types to debug output
@param debug     Debug class
@param value     Value to be printed

Support for printing custom types (i.e. those not handled by @ref std::iostream)
can be added by implementing this function for given type.

The function should convert the type to one of supported types (such as the
builtin types or @ref std::string) and then call @ref Debug::operator<<() with
it. You can also use @ref Debug::nospace and @ref Debug::newline.
 */
template<class T> Debug& operator<<(Debug& debug, const T& value);
#endif

/** @relatesalso Debug
@brief Operator for printing iterable types to debug output

Prints the value as @cb{.shell-session} {a, b, c} @ce. If the type contains
a nested iterable type, the values are separated by newlines. Specifying
@ref Debug::Flag::Packed or using @ref Debug::packed will print the values
tightly-packed without commas and spaces in between.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class Iterable> Debug& operator<<(Debug& debug, const Iterable& value)
#else
/* libc++ from Apple's Clang "4.2" (3.2-svn) doesn't have constexpr operator
   bool for std::integral_constant, thus we need to use ::value instead */
template<class Iterable> Debug& operator<<(typename std::enable_if<IsIterable<Iterable>::value && !IsStringLike<Iterable>::value, Debug&>::type debug, const Iterable& value)
#endif
{
    /* Nested containers should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Debug::Flag::NoSpace));

    const char *beg, *sep, *end;
    if(debug.immediateFlags() & Debug::Flag::Packed) {
        beg = end = "";
        sep = IsIterable<decltype(*value.begin())>::value && !IsStringLike<decltype(*value.begin())>::value ? "\n" : "";
    } else {
        beg = "{";
        end = "}";
        sep = IsIterable<decltype(*value.begin())>::value && !IsStringLike<decltype(*value.begin())>::value ? ",\n " : ", ";
    }

    debug << beg << Debug::nospace;
    for(auto it = value.begin(); it != value.end(); ++it) {
        if(it != value.begin())
            debug << Debug::nospace << sep << Debug::nospace;
        debug << *it;
    }
    debug << Debug::nospace << end;

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

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
}

/** @relatesalso Debug
@brief Print a @ref std::pair to debug output

Prints the value as @cb{.shell-session} (first, second) @ce. Unlike
@ref operator<<(Debug& debug, const Iterable& value), the output is not
affected by @ref Debug::Flag::Packed / @ref Debug::packed.
*/
template<class T, class U> Debug& operator<<(Debug& debug, const std::pair<T, U>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Debug::Flag::NoSpace));

    debug << "(" << Debug::nospace << value.first << Debug::nospace << "," << value.second << Debug::nospace << ")";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
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
         * @brief Default warning output stream
         * @m_since{2019,10}
         *
         * Warning output when no output redirection happens. A pointer to
         * @ref std::cerr.
         * @see @ref output()
         */
        static std::ostream* defaultOutput();

        /**
         * @brief Current warning output stream
         *
         * Warning output constructed with the @ref Warning(Flags) constructor
         * will be using this output stream.
         * @see @ref defaultOutput()
         */
        static std::ostream* output();

        /**
         * @brief Whether current warning output is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing
         * @ref Warning instance or with @ref std::cerr if there isn't any.
         * @see @ref Debug::isTty(), @ref Error::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Inherits output of enclosing @ref Warning instance or uses
         * @ref std::cerr if there isn't any.
         */
        explicit Warning(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put warning output. If set to
         *      @cpp nullptr @ce, no warning output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Warning()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         */
        explicit Warning(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Warning(const Warning&) = delete;

        /** @brief Move constructor */
        Warning(Warning&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Warning();

        /** @brief Copying is not allowed */
        Warning& operator=(const Warning&) = delete;

        /** @brief Move assignment is not allowed */
        Warning& operator=(Warning&&) = delete;

    private:
        std::ostream* _previousGlobalWarningOutput;
};

/**
@brief Error output handler

@copydetails Warning
@see @ref Fatal
*/
class CORRADE_UTILITY_EXPORT Error: public Debug {
    friend Fatal;

    public:
        /**
         * @brief Default error output stream
         * @m_since{2019,10}
         *
         * Error output when no output redirection happens. A pointer to
         * @ref std::cerr.
         * @see @ref output()
         */
        static std::ostream* defaultOutput();

        /**
         * @brief Current error output stream
         *
         * Error output constructed with the @ref Error(Flags) constructor
         * will be using this output stream.
         * @see @ref defaultOutput()
         */
        static std::ostream* output();

        /**
         * @brief Whether current error output stream is a TTY
         *
         * Calls @ref isTty(std::ostream*) with output of enclosing @ref Error
         * instance or with @ref std::cerr if there isn't any.
         * @see @ref Debug::isTty(), @ref Warning::isTty()
         */
        static bool isTty();

        /**
         * @brief Default constructor
         * @param flags         Output flags
         *
         * Inherits output of enclosing @ref Error instance or uses
         * @ref std::cerr if there isn't any.
         */
        explicit Error(Flags flags = {});

        /**
         * @brief Constructor
         * @param output        Stream where to put error output. If set to
         *      @cpp nullptr @ce, no error output will be written anywhere.
         * @param flags         Output flags
         *
         * All new instances created using the default @ref Error()
         * constructor during lifetime of this instance will inherit the output
         * set in @p output.
         */
        explicit Error(std::ostream* output, Flags flags = {});

        /** @brief Copying is not allowed */
        Error(const Error&) = delete;

        /** @brief Move constructor */
        Error(Error&&) = default;

        /**
         * @brief Destructor
         *
         * Resets the output redirection back to the output of enclosing scope.
         * If there was any output, adds newline at the end. Also resets output
         * color modifier, if there was any.
         * @see @ref resetColor()
         */
        ~Error();

        /** @brief Copying is not allowed */
        Error& operator=(const Error&) = delete;

        /** @brief Move assignment is not allowed */
        Error& operator=(Error&&) = delete;

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        CORRADE_UTILITY_LOCAL void cleanupOnDestruction(); /* Needed for Fatal */

    private:
        std::ostream* _previousGlobalErrorOutput;
};

/**
@brief Fatal output handler

Equivalent to @ref Error, but exits with defined exit code on destruction. So
instead of this:

@snippet Utility.cpp Fatal-Error

You can write just this:

@snippet Utility.cpp Fatal-Fatal
*/
class CORRADE_UTILITY_EXPORT Fatal: public Error {
    public:
        /**
         * @brief Constructor
         * @param exitCode      Application exit code to be used on destruction
         * @param flags         Output flags
         *
         * Sets output to @ref std::cerr. The @p exitCode is passed to
         * @ref std::exit() on destruction.
         */
        Fatal(int exitCode = 1, Flags flags = {}): Error{flags}, _exitCode{exitCode} {}

        /** @overload */
        Fatal(Flags flags): Fatal{1, flags} {}

        /**
         * @brief Constructor
         * @param output        Stream where to put debug output. If set to
         *      @cpp nullptr @ce, no debug output will be written anywhere.
         * @param exitCode      Application exit code to be used on destruction
         * @param flags         Output flags
         */
        Fatal(std::ostream* output, int exitCode = 1, Flags flags = {}): Error{output, flags}, _exitCode{exitCode} {}

        /** @overload */
        Fatal(std::ostream* output, Flags flags = {}): Fatal{output, 1, flags} {}

        /**
         * @brief Destructor
         *
         * Exits the application with exit code specified in constructor.
         */
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* http://stackoverflow.com/questions/38378693/did-visual-studio-2015-update-3-break-constructor-attributes */
        CORRADE_NORETURN
        #endif
        ~Fatal();

    private:
        int _exitCode;
};

}}

#endif
