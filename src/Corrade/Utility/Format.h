#ifndef Corrade_Utility_Format_h
#define Corrade_Utility_Format_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Function @ref Corrade::Utility::formatString(), @ref Corrade::Utility::formatInto()
 * @experimental
 */

#include <string>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Format a string

Provides type-safe formatting of arbitrary types into a template string,
similar in syntax to Python's [format()](https://docs.python.org/3.4/library/string.html#format-string-syntax).
Example usage:

@snippet Utility.cpp formatString

# Templating language

Formatting placeholders are denoted by `{}`, which can have either implicit
ordering (as shown above), or be numbered, such as `{2}`. Zero means first item
from @p args, it's allowed to repeat the numbers. An implicit placeholder
following a numbered one will get next position after. Example:

@snippet Utility.cpp formatString-numbered

Unlike in Python, it's allowed to both have more placeholders than arguments or
more arguments than placeholders. Extraneous placeholders are copied to the
output verbatim, extraneous arguments are simply ignored.

In order to write a literal curly brace to the output, simply double it:

@snippet Utility.cpp formatString-escape

# Data type support

@m_class{m-fullwidth}

| Type                                  | Behavior
| ------------------------------------- | --------
| @cpp char @ce, @cpp unsigned char @ce | Written as an integer (*not as a character*)
| @cpp short @ce, @cpp unsigned short @ce | Written as an integer
| @cpp int @ce, @cpp unsigned int @ce   | Written as an integer
| @cpp long @ce, @cpp unsigned long @ce | Written as an integer
| @cpp long long @ce, @cpp unsigned long long @ce | Written as an integer
| @cpp float @ce <b></b>    | Written as a float with 6 significant digits by default
| @cpp double @ce <b></b>   | Written as a float with 15 significant digits by default
| @cpp long double @ce <b></b> | Written as a float with 18 significant digits by default
| @cpp char* @ce <b></b> | Written as a sequence of characters until @cpp '\0' @ce (which is not written)
| @ref std::string | Written as a sequence of @ref std::string::size() characters
| @ref Containers::ArrayView "Containers::ArrayView<char>" | Written as a sequence of @ref Containers::ArrayView::size() characters

# Performance

This function always does exactly one allocation for the output string. See
@ref formatInto(std::string&, std::size_t, const char*, const Args&... args)
for an ability to write into an existing string (with at most one reallocation)
and @ref formatInto(Containers::ArrayView<char>, const char*, const Args&... args)
for a completely zero-allocation alternative. There is also
@ref formatInto(std::FILE*, const char*, const Args&... args) for writing to
files or standard output.

# Comparison to Debug

@ref Debug class desired usage is for easy printing of complex nested types,
containers, enum values or opaque types for logging and diagnostic purposes,
with focus on convenience rather than speed or advanced formatting
capabilities. The @ref formatString() family of functions is intended for cases
where it's required to have a complete control over the output, for example
when serializing text files.

@experimental
*/
template<class ...Args> std::string formatString(const char* format, const Args&... args);

/**
@brief Format a string into an existing string

Takes an existing @p string and writes the formatted content starting at
@p offset. If the string is not large enough, does at most one reallocation
(by calling @p std::string::resize()). Returns final written size (which might
be less than the string size if inserting in the middle). *Does not* write any
terminating @cpp '\0' @ce character. Example usage:

@snippet Utility.cpp formatInto-string

See @ref formatString() for more information about usage and templating
language.

@experimental
*/
template<class ...Args> std::size_t formatInto(std::string& string, std::size_t offset, const char* format, const Args&... args);

/**
@brief Format a string into an existing buffer

Writes formatted output to given @p buffer, expecting that it is large enough.
The formatting is done completely without any allocation. Returns total amount
of bytes written, *does not* write any terminating @cpp '\0' @ce character.
Example usage:

@snippet Utility.cpp formatInto-buffer

See @ref formatString() for more information about usage and templating
language.

@experimental
*/
template<class ...Args> std::size_t formatInto(Containers::ArrayView<char> buffer, const char* format, const Args&... args);

/**
@brief Format a string into a file

Writes formatted output to @p file, which can be either an arbitrary file
opened using @ref std::fopen() or @cpp stdout @ce / @cpp stderr @ce. Does not
allocate on its own (though the underlying file writing routines might), *does
not* write any terminating @cpp '\0' @ce character. Example usage:

@snippet Utility.cpp formatInto-stdout

See @ref formatString() for more information about usage and templating
language.

@experimental
*/
template<class ...Args> void formatInto(std::FILE* file, const char* format, const Args&... args);

namespace Implementation {

template<class T> struct Formatter;

template<> struct Formatter<int> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, int value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, int value);
};
template<> struct Formatter<char>: Formatter<int> {};
template<> struct Formatter<short>: Formatter<int> {};

template<> struct Formatter<unsigned int> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, unsigned int value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, unsigned int value);
};
template<> struct Formatter<unsigned char>: Formatter<unsigned int> {};
template<> struct Formatter<unsigned short>: Formatter<unsigned int> {};

template<> struct Formatter<long long> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, long long value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, long long value);
};
template<> struct Formatter<long>: Formatter<long long> {};

template<> struct Formatter<unsigned long long> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, unsigned long long value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, unsigned long long value);
};
template<> struct Formatter<unsigned long>: Formatter<unsigned long long> {};

template<> struct Formatter<float> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, float value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, float value);
};
template<> struct Formatter<double> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, double value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, double value);
};
template<> struct Formatter<long double> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, long double value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, long double value);
};
template<> struct Formatter<const char*> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, const char* value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, const char* value);
};
template<> struct Formatter<char*>: Formatter<const char*> {};
template<> struct Formatter<Containers::ArrayView<const char>> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, Containers::ArrayView<const char> value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, Containers::ArrayView<const char> value);
};
template<> struct Formatter<std::string> {
    static CORRADE_UTILITY_EXPORT std::size_t format(Containers::ArrayView<char> buffer, const std::string& value);
    static CORRADE_UTILITY_EXPORT void format(std::FILE* file, const std::string& value);
};

struct BufferFormatter {
    /* Needed for a sentinel value (C arrays can't have zero size) */
    /*implicit*/ constexpr BufferFormatter(): _fn{}, _value{} {}

    template<class T> explicit BufferFormatter(const T& value): _value{&value} {
        _fn = [](Containers::ArrayView<char> buffer, const void* value) {
            return Formatter<typename std::decay<T>::type>::format(buffer, *static_cast<const T*>(value));
        };
    }

    std::size_t operator()(Containers::ArrayView<char> buffer) const {
        return _fn(buffer, _value);
    }

    /* Cached size of the formatted string to avoid recalculations */
    std::size_t size{~std::size_t{}};

    private:
        std::size_t(*_fn)(Containers::ArrayView<char>, const void*);
        const void* _value;
};

struct FileFormatter {
    /* Needed for a sentinel value (C arrays can't have zero size) */
    /*implicit*/ constexpr FileFormatter(): _fn{}, _value{} {}

    template<class T> explicit FileFormatter(const T& value): _value{&value} {
        _fn = [](std::FILE* file, const void* value) {
            Formatter<typename std::decay<T>::type>::format(file, *static_cast<const T*>(value));
        };
    }

    void operator()(std::FILE* file) const { _fn(file, _value); }

    private:
        void(*_fn)(std::FILE*, const void*);
        const void* _value;
};

CORRADE_UTILITY_EXPORT std::size_t formatInto(Containers::ArrayView<char> buffer, const char* format, Containers::ArrayView<BufferFormatter> formatters);
CORRADE_UTILITY_EXPORT std::size_t formatInto(std::string& buffer, std::size_t offset, const char* format, Containers::ArrayView<BufferFormatter> formatters);
CORRADE_UTILITY_EXPORT void formatInto(std::FILE* file, const char* format, Containers::ArrayView<FileFormatter> formatters);

}

template<class ...Args> std::string formatString(const char* format, const Args&... args) {
    std::string buffer;
    formatInto(buffer, 0, format, args...);
    return buffer;
}

template<class ...Args> std::size_t formatInto(Containers::ArrayView<char> buffer, const char* format, const Args&... args) {
    Implementation::BufferFormatter formatters[sizeof...(args) + 1] { Implementation::BufferFormatter{args}..., {} };
    return Implementation::formatInto(buffer, format, {formatters, sizeof...(args)});
}

template<class ...Args> std::size_t formatInto(std::string& buffer, std::size_t offset, const char* format, const Args&... args) {
    Implementation::BufferFormatter formatters[sizeof...(args) + 1] { Implementation::BufferFormatter{args}..., {} };
    return Implementation::formatInto(buffer, offset, format, {formatters, sizeof...(args)});
}

template<class ...Args> void formatInto(std::FILE* file, const char* format, const Args&... args) {
    Implementation::FileFormatter formatters[sizeof...(args) + 1] { Implementation::FileFormatter{args}..., {} };
    Implementation::formatInto(file, format, {formatters, sizeof...(args)});
}

}}

#endif
