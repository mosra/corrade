#ifndef Corrade_Utility_Format_h
#define Corrade_Utility_Format_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
 * @brief Function @ref Corrade::Utility::format(), @ref Corrade::Utility::formatInto(), @ref Corrade::Utility::print(), @ref Corrade::Utility::printError()
 * @experimental
 */

#include <cstdio>

#include "Corrade/Tags.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/visibility.h"

#ifdef CORRADE_BUILD_DEPRECATED
/* format() used to return an Array, now it returns a String. Add an include
   (which is fortunately not too heavy) to ease porting of existing code. */
#include "Corrade/Containers/String.h"
#endif

namespace Corrade { namespace Utility {

/**
@brief Format a string
@m_since{2019,10}

Provides type-safe formatting of arbitrary types into a template string,
similar in syntax to Python's [format()](https://docs.python.org/3.4/library/string.html#format-string-syntax).
Example usage:

@snippet Utility.cpp format

# Templating language

Formatting placeholders are denoted by `{}`, which can have either implicit
ordering (as shown above), or be numbered, such as `{2}`. Zero means first item
from @p args, it's allowed to repeat the numbers. An implicit placeholder
following a numbered one will get next position after. Example:

@snippet Utility.cpp format-numbered

Unlike in Python, it's allowed to both have more placeholders than arguments or
more arguments than placeholders. Extraneous placeholders are copied to the
output verbatim, extraneous arguments are simply ignored.

In order to write a literal curly brace to the output, simply double it:

@snippet Utility.cpp format-escape

# Data type support

@m_class{m-fullwidth}

| Type                                  | Default behavior
| ------------------------------------- | ------------------------------------
| @cpp char @ce, @cpp signed char @ce, @cpp unsigned char @ce | Written as a base-10 integer (*not as a character*)
| @cpp short @ce, @cpp unsigned short @ce | Written as a base-10 integer
| @cpp int @ce, @cpp unsigned int @ce   | Written as a base-10 integer
| @cpp long @ce, @cpp unsigned long @ce | Written as a base-10 integer
| @cpp long long @ce, @cpp unsigned long long @ce | Written as a base-10 integer
| @cpp float @ce <b></b>    | Written as a float with 6 significant digits by default
| @cpp double @ce <b></b>   | Written as a float with 15 significant digits by default
| @cpp long double @ce <b></b> | Written as a float, by default with 18 significant digits on platforms \n with 80-bit @cpp long double @ce and 15 digits on platforms @ref CORRADE_LONG_DOUBLE_SAME_AS_DOUBLE "where it is 64-bit"
| @cpp char* @ce <b></b> | Written as a sequence of characters until @cpp '\0' @ce (which is not written)
| @ref Containers::StringView, \n @ref Containers::MutableStringView "MutableStringView", @ref Containers::String "String" | Written as a sequence of @ref Containers::StringView::size() characters
| @ref Containers::ArrayView "Containers::ArrayView<const char>" @m_class{m-label m-danger} **deprecated** | Written as a sequence of @ref Containers::ArrayView::size() characters. \n Deprecated, use @ref Containers::StringView instead.
| @ref std::string | Written as a sequence of @ref std::string::size() characters \n (@cpp #include @ce @ref Corrade/Utility/FormatStl.h in addition)
| @ref std::string_view | Written as a sequence of @ref std::string_view::size() characters \n (@cpp #include @ce @ref Corrade/Utility/FormatStlStringView.h in addition)

# Advanced formatting options

Advanced formatting such as precision or presentation type is possible by
putting extra options after a semicolon, following the optional placeholder
number, such as `{:x}` to print an integer value in hexadecimal. In general,
the syntax similar to the @ref std::printf()-style formatting, with the
addition of `{}` and `:` used instead of `%` --- for example, @cpp "%.2x" @ce
can be translated to @cpp "{:.2x}" @ce.

The full placeholder syntax is the following, again a subset of the Python
[format()](https://docs.python.org/3.4/library/string.html#format-string-syntax):

    {[number][:[.precision][type]]}

The `type` is a single character specifying output conversion:

Value           | Meaning
--------------- | -------
@cpp 'c' @ce <b></b> | Character. Valid only for 8-, 16- and 32-bit integer types. At the moment, arbitrary UTF-32 codepoints don't work, only 7-bit ASCII values have a guaranteed output.
@cpp 'd' @ce <b></b> | Decimal integer (base 10). Valid only for integer types. Default for integers if nothing is specified.
@cpp 'o' @ce <b></b> | Octal integer (base 8). Valid only for integer types.
@cpp 'x' @ce <b></b> | Hexadecimal integer (base 16) with lowercase letters a--f. Valid only for integer types.
@cpp 'X' @ce <b></b> | Hexadecimal integer with uppercase letters A--F. Valid only for integer types.
@cpp 'g' @ce <b></b> | General floating-point, formatting the value either in exponent notation or fixed-point format depending on its magnitude. The exponent `e` and special values such as `nan` or `inf` are printed lowercase. Valid only for floating-point types.
@cpp 'G' @ce <b></b> | General floating-point. The exponent `E` and special values such as `NAN` or `INF` are printed uppercase. Valid only for floating-point types.
@cpp 'e' @ce <b></b> | Exponent notation. The exponent `e` and special values such as `nan` or `inf` are printed lowercase. Valid only for floating-point types.
@cpp 'E' @ce <b></b> | Exponent notation. The exponent `E` and special values such as `NAN` or `INF` are printed uppercase. Valid only for floating-point types.
@cpp 'f' @ce <b></b> | Fixed point. The exponent `e` and special values such as `nan` or `inf` are printed lowercase. Valid only for floating-point types.
@cpp 'F' @ce <b></b> | Fixed point. The exponent `E` and special values such as `NAN` or `INF` are printed uppercase. Valid only for floating-point types.
<em>none</em>   | Default based on type, equivalent to @cpp 'd' @ce for integral types and @cpp 'g' @ce for floating-point types. The only valid specifier for strings.

The `precision` field specifies a precision of the output. It's interpreted
differently based on the data type:

Type            | Meaning
--------------- | -------
Integers, except for the @cpp 'c' @ce type specifier | If the number of decimals is smaller than `precision`, the integer gets padded with the `0` character from the left. If both the number and `precision` is @cpp 0 @ce, nothing is written to the output. Default `precision` is @cpp 1 @ce.
Floating-point types with default or @cpp 'g' @ce / @cpp 'G' @ce type specifier | The number is printed with *at most* `precision` significant digits. Default `precision` depends on data type, see the type support table above.
Floating-point types with @cpp 'e' @ce / @cpp 'E' @ce type specifier | The number is always printed with *exactly* one decimal, `precision` decimal points (including trailing zeros) and the exponent. Default `precision` depends on data type, see the type support table above.
Floating-point types with @cpp 'f' @ce / @cpp 'F' @ce type specifier | The number is always printed with *exactly* `precision` decimal points including trailing zeros. Default `precision` depends on data type, see the type support table above.
Strings, characters (integers with the @cpp 'c' @ce type specifier) | If the string length is larger than `precision`, only the first `precision` *bytes* are written to the output. Default `precision` is unlimited. Note that this doesn't work with UTF-8 at the moment and specifying `precision` of @cpp 0 @ce doesn't give the expected output for characters.

Example of formating of CSS colors with correct width:

@snippet Utility.cpp format-type-precision

# Performance

This function always does exactly one allocation for the output array. See
@ref formatInto(std::string&, std::size_t, const char*, const Args&... args)
for an ability to write into an existing @ref std::string (with at most one
reallocation) and @ref formatInto(const Containers::MutableStringView&, const char*, const Args&... args)
for a completely zero-allocation alternative. There is also
@ref formatInto(std::FILE*, const char*, const Args&... args) for writing to
files or standard output.

# Comparison to Debug

@ref Debug class desired usage is for easy printing of complex nested types,
containers, enum values or opaque types for logging and diagnostic purposes,
with focus on convenience rather than speed or advanced formatting
capabilities. The @ref format() family of functions is intended for cases where
it's required to have a complete control over the output, for example when
serializing text files.

@experimental

@see @ref formatString(), @ref formatInto(), @ref print(), @ref printError()
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class ...Args> Containers::String format(const char* format, const Args&... args);
#else
/* Done this way to avoid including <Containers/String.h> for the return type */
template<class ...Args, class String = Containers::String, class MutableStringView = Containers::MutableStringView> String format(const char* format, const Args&... args);
#endif

/**
@brief Format a string into an existing buffer

Writes formatted output to given @p buffer, expecting that it is large enough.
The formatting is done completely without any allocation. Returns total amount
of bytes written, *does not* write any terminating @cpp '\0' @ce character.
Example usage:

@snippet Utility.cpp formatInto-buffer

See @ref format() for more information about usage and templating language.

@experimental
*/
template<class ...Args> std::size_t formatInto(const Containers::MutableStringView& buffer, const char* format, const Args&... args);

/** @overload */
/* This is needed as otherwise calling formatInto() with char[] as the first
   argument would use the MutableStringView constructor that relies on strlen()
   to determine the size, which isn't wanted in this case */
template<class ...Args, std::size_t size> std::size_t formatInto(char(&buffer)[size], const char* format, const Args&... args) {
    return formatInto(Containers::MutableStringView{buffer, size}, format, args...);
}

/**
@brief Format a string into a file

Writes formatted output to @p file, which can be either an arbitrary file
opened using @ref std::fopen() or @cpp stdout @ce / @cpp stderr @ce. Does not
allocate on its own (though the underlying file writing routines might), *does
not* write any terminating @cpp '\0' @ce character. Example usage:

@snippet Utility.cpp formatInto-stdout

See @ref format() for more information about usage and templating language.

@experimental
*/
template<class ...Args> void formatInto(std::FILE* file, const char* format, const Args&... args);

/**
@brief Print a string to the standard output

Equivalent to calling @ref formatInto(std::FILE*, const char*, const Args&... args)
with @cpp stdout @ce as a first parameter.

@experimental
*/
template<class ...Args> inline void print(const char* format, const Args&... args) {
    return formatInto(stdout, format, args...);
}

/**
@brief Print a string to the standard error output

Equivalent to calling @ref formatInto(std::FILE*, const char*, const Args&... args)
with @cpp stderr @ce as a first parameter.

@experimental
*/
template<class ...Args> inline void printError(const char* format, const Args&... args) {
    return formatInto(stderr, format, args...);
}

namespace Implementation {

enum class FormatType: unsigned char;

template<class T, class = void> struct Formatter;

template<> struct Formatter<int> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, int value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, int value, int precision, FormatType type);
};
template<> struct Formatter<short>: Formatter<int> {};

template<> struct Formatter<unsigned int> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, unsigned int value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, unsigned int value, int precision, FormatType type);
};
template<> struct Formatter<unsigned short>: Formatter<unsigned int> {};

/* The char is signed or unsigned depending on platform (ARM has it unsigned by
   default for example, because it maps better to the instruction set), and
   thus std::int8_t is usually a typedef to `signed char`. Be sure to support
   that as well, `signed short` and such OTOH isn't so important. Also, while I
   could delegate `char` to `std::conditional<std::is_signed<char>::value, int,
   unsigned int>::type`, it doesn't matter because the two variants currently
   don't behave any different and `int` will cover the whole range easily. */
template<> struct Formatter<char>: Formatter<int> {};
template<> struct Formatter<signed char>: Formatter<int> {};
template<> struct Formatter<unsigned char>: Formatter<unsigned int> {};

template<> struct Formatter<long long> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, long long value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, long long value, int precision, FormatType type);
};
template<> struct Formatter<long>: Formatter<long long> {};

template<> struct Formatter<unsigned long long> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, unsigned long long value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, unsigned long long value, int precision, FormatType type);
};
template<> struct Formatter<unsigned long>: Formatter<unsigned long long> {};

template<> struct Formatter<float> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, float value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, float value, int precision, FormatType type);
};
template<> struct Formatter<double> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, double value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, double value, int precision, FormatType type);
};
template<> struct Formatter<long double> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, long double value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, long double value, int precision, FormatType type);
};
template<> struct Formatter<const char*> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, const char* value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, const char* value, int precision, FormatType type);
};
template<> struct Formatter<char*>: Formatter<const char*> {};
template<> struct Formatter<Containers::StringView> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, Containers::StringView value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, Containers::StringView value, int precision, FormatType type);
};
template<> struct Formatter<Containers::MutableStringView>: Formatter<Containers::StringView> {};
template<> struct Formatter<Containers::String>: Formatter<Containers::StringView> {};
#ifdef CORRADE_BUILD_DEPRECATED
/* Sigh, can't mark this with CORRADE_DEPRECATED() because it's then not
   possible to suppress the warning via CORRADE_IGNORE_DEPRECATED for tests.
   When removing, remove this type from the table in the docs as well. */
template<> struct Formatter<Containers::ArrayView<const char>> {
    static CORRADE_UTILITY_EXPORT std::size_t format(const Containers::MutableStringView& buffer, Containers::ArrayView<const char> value, int precision, FormatType type);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, Containers::ArrayView<const char> value, int precision, FormatType type);
};
#endif

/* If the type is an enum, use its underlying type, assuming the enum is
   convertible to it */
template<class T> struct Formatter<T, typename std::enable_if<std::is_enum<T>::value>::type>: Formatter<typename std::underlying_type<T>::type> {};

struct BufferFormatter {
    /* Needed for a sentinel value (C arrays can't have zero size) */
    /*implicit*/ constexpr BufferFormatter(): _fn{}, _value{} {}

    template<class T> explicit BufferFormatter(const T& value): _value{&value} {
        _fn = [](const Containers::MutableStringView& buffer, const void* val, int precision, FormatType type) {
            return Formatter<typename std::decay<T>::type>::format(buffer, *static_cast<const T*>(val), precision, type);
        };
    }

    std::size_t operator()(const Containers::MutableStringView& buffer, int precision, FormatType type) const {
        return _fn(buffer, _value, precision, type);
    }

    /* Cached size of the formatted string to avoid recalculations */
    std::size_t size{~std::size_t{}};

    private:
        std::size_t(*_fn)(const Containers::MutableStringView&, const void*, int precision, FormatType type);
        const void* _value;
};

struct FileFormatter {
    /* Needed for a sentinel value (C arrays can't have zero size) */
    /*implicit*/ constexpr FileFormatter(): _fn{}, _value{} {}

    template<class T> explicit FileFormatter(const T& value): _value{&value} {
        _fn = [](std::FILE* file, const void* val, int precision, FormatType type) {
            Formatter<typename std::decay<T>::type>::format(file, *static_cast<const T*>(val), precision, type);
        };
    }

    void operator()(std::FILE* file, int precision, FormatType type) const { _fn(file, _value, precision, type); }

    private:
        void(*_fn)(std::FILE*, const void*, int precision, FormatType type);
        const void* _value;
};

CORRADE_UTILITY_EXPORT std::size_t formatFormatters(const Containers::MutableStringView& buffer, const char* format, BufferFormatter* formatters, std::size_t formattersCount);
CORRADE_UTILITY_EXPORT void formatFormatters(std::FILE* file, const char* format, FileFormatter* formatters, std::size_t formattersCount);

}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class ...Args, class String, class MutableStringView> String format(const char* format, const Args&... args) {
    /* Get just the size first. Can't pass just nullptr, because that would
       match the formatInto(std::FILE*) overload, can't pass a String because
       it's guaranteed to always point to a null-terminated char array, even if
       it's empty. */
    const std::size_t size = formatInto(MutableStringView{}, format, args...);
    String string{NoInit, size};
    /* The String is created with an extra byte for the null terminator, but
       since printf() always wants to print the null terminator, we need to
       pass a view *including* the null terminator to it -- which is why we
       have to create the view manually. Once we switch away from printf() this
       workaround can be removed. */
    formatInto(MutableStringView{string.data(), size + 1}, format, args...);
    return string;
}
#endif

template<class ...Args> std::size_t formatInto(const Containers::MutableStringView& buffer, const char* format, const Args&... args) {
    Implementation::BufferFormatter formatters[sizeof...(args) + 1] { Implementation::BufferFormatter{args}..., {} };
    return Implementation::formatFormatters(buffer, format, formatters, sizeof...(args));
}

template<class ...Args> void formatInto(std::FILE* file, const char* format, const Args&... args) {
    Implementation::FileFormatter formatters[sizeof...(args) + 1] { Implementation::FileFormatter{args}..., {} };
    Implementation::formatFormatters(file, format, formatters, sizeof...(args));
}

}}

#endif
