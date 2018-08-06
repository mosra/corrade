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

#include "Format.h"

#include <cstring>

namespace Corrade { namespace Utility {

namespace Implementation {

std::size_t Formatter<int>::format(const Containers::ArrayView<char> buffer, const int value) {
    return std::snprintf(buffer, buffer.size(), "%i", value);
    return {};
}
void Formatter<int>::format(std::FILE* const file, const int value) {
    std::fprintf(file, "%i", value);
}
std::size_t Formatter<unsigned int>::format(const Containers::ArrayView<char> buffer, const unsigned int value) {
    return std::snprintf(buffer, buffer.size(), "%u", value);
}
void Formatter<unsigned int>::format(std::FILE* const file, const unsigned int value) {
    std::fprintf(file, "%u", value);
}
std::size_t Formatter<long long>::format(const Containers::ArrayView<char> buffer, const long long value) {
    return std::snprintf(buffer, buffer.size(), "%lli", value);
}
void Formatter<long long>::format(std::FILE* const file, const long long value) {
    std::fprintf(file, "%lli", value);
}
std::size_t Formatter<unsigned long long>::format(const Containers::ArrayView<char> buffer, const unsigned long long value) {
    return std::snprintf(buffer, buffer.size(), "%llu", value);
}
void Formatter<unsigned long long>::format(std::FILE* const file, const unsigned long long value) {
    std::fprintf(file, "%llu", value);
}

/* The default. Source: http://en.cppreference.com/w/cpp/io/ios_base/precision,
   Wikipedia says 6-digit number can be converted back and forth without loss:
   https://en.wikipedia.org/wiki/Single-precision_floating-point_format
   Kept in sync with Debug. */
std::size_t Formatter<float>::format(const Containers::ArrayView<char> buffer, const float value) {
    return std::snprintf(buffer, buffer.size(), "%.6g", double(value));
}
void Formatter<float>::format(std::FILE* const file, const float value) {
    std::fprintf(file, "%.6g", double(value));
}

/* Wikipedia says 15-digit number can be converted back and forth without loss:
   https://en.wikipedia.org/wiki/Double-precision_floating-point_format
   Kept in sync with Debug. */
std::size_t Formatter<double>::format(const Containers::ArrayView<char> buffer, const double value) {
    return std::snprintf(buffer, buffer.size(), "%.15g", value);
}
void Formatter<double>::format(std::FILE* const file, const double value) {
    std::fprintf(file, "%.15g", value);
}

/* Wikipedia says 18-digit number can be converted both ways without
   loss: https://en.wikipedia.org/wiki/Extended_precision#Working_range
   Kept in sync with Debug. */
std::size_t Formatter<long double>::format(const Containers::ArrayView<char> buffer, const long double value) {
    return std::snprintf(buffer, buffer.size(), "%.18Lg", value);
}
void Formatter<long double>::format(std::FILE* const file, const long double value) {
    std::fprintf(file, "%.18Lg", value);
}

std::size_t Formatter<Containers::ArrayView<const char>>::format(const Containers::ArrayView<char> buffer, const Containers::ArrayView<const char> value) {
    /* strncpy() would stop on \0 characters */
    if(buffer) std::memcpy(buffer, value, value.size());
    return value.size();
}
void Formatter<Containers::ArrayView<const char>>::format(std::FILE* const file, const Containers::ArrayView<const char> value) {
    std::fwrite(value.data(), value.size(), 1, file);
}
std::size_t Formatter<const char*>::format(const Containers::ArrayView<char> buffer, const char* value) {
    return Formatter<Containers::ArrayView<const char>>::format(buffer, {value, std::strlen(value)});
}
void Formatter<const char*>::format(std::FILE* const file, const char* value) {
    Formatter<Containers::ArrayView<const char>>::format(file, {value, std::strlen(value)});
}
std::size_t Formatter<std::string>::format(const Containers::ArrayView<char> buffer, const std::string& value) {
    return Formatter<Containers::ArrayView<const char>>::format(buffer, {value.data(), value.size()});
}
void Formatter<std::string>::format(std::FILE* const file, const std::string& value) {
    return Formatter<Containers::ArrayView<const char>>::format(file, {value.data(), value.size()});
}

namespace {

template<class Writer, class FormattedWriter, class Formatter> void formatWith(const Writer writer, const FormattedWriter formattedWriter, const Containers::ArrayView<const char> format, const Containers::ArrayView<Formatter> formatters) {
    bool inPlaceholder = false;
    std::size_t placeholderOffset = 0;
    std::size_t formatterToGo = 0;
    int placeholderIndex = -1;
    for(std::size_t formatOffset = 0; formatOffset != format.size(); ) {
        /* Placeholder begin (or escaped {) */
        if(format[formatOffset] == '{') {
            if(formatOffset + 1 < format.size() && format[formatOffset+1] == '{') {
                writer(format.slice<1>(formatOffset));
                formatOffset += 2;
                continue;
            }

            CORRADE_INTERNAL_ASSERT(!inPlaceholder);
            inPlaceholder = true;
            placeholderOffset = formatOffset;
            placeholderIndex = -1;

            ++formatOffset;
            continue;
        }

        /* Placeholder end (or escaped }) */
        if(format[formatOffset] == '}') {
            if(!inPlaceholder && formatOffset + 1 < format.size() && format[formatOffset+1] == '}') {
                writer(format.slice<1>(formatOffset));
                formatOffset += 2;
                continue;
            }

            CORRADE_ASSERT(inPlaceholder, "Utility::format(): mismatched }", );
            inPlaceholder = false;

            /* If the placeholder was numbered, use that number, otherwise
               just use the formatter that's next */
            if(placeholderIndex != -1) formatterToGo = placeholderIndex;

            /* Formatter index is in bounds, write */
            if(formatterToGo < formatters.size())
                formattedWriter(formatters[formatterToGo]);

            /* Otherwise just verbatim copy the placeholder (including }) */
            else writer(format.slice(placeholderOffset, formatOffset + 1));

            /* Next time we see an unnumbered placeholder, take the next
               formatter */
            ++formatterToGo;

            ++formatOffset;
            continue;
        }

        /* Placeholder contents */
        if(inPlaceholder) {
            /* Placeholder index */
            while(formatOffset < format.size() && format[formatOffset] >= '0' && format[formatOffset] <= '9') {
                if(placeholderIndex == -1) placeholderIndex = 0;
                else placeholderIndex *= 10;
                placeholderIndex += (format[formatOffset] - '0');
                ++formatOffset;
            }

            /* Unexpected end, break -- the assert at the end of function
               takes care of this */
            if(formatOffset == format.size()) break;

            /* Next should be the placeholder end */
            CORRADE_ASSERT(format[formatOffset] == '}',
                "Utility::format(): unknown placeholder content:" << std::string{format[formatOffset]}, );
            continue;
        }

        /* Other things, just copy. Grab as much as I can to avoid calling
           functions on single bytes. */
        std::size_t next = formatOffset;
        while(next < format.size() && format[next] != '{' && format[next] != '}')
            ++next;
        writer(format.slice(formatOffset, next));
        formatOffset = next;
    }

    CORRADE_ASSERT(!inPlaceholder, "Utility::format(): unexpected end of format string", );
}

}

std::size_t formatInto(const Containers::ArrayView<char> buffer, const char* const format, Containers::ArrayView<BufferFormatter> const formatters) {
    std::size_t bufferOffset = 0;
    formatWith([&buffer, &bufferOffset](Containers::ArrayView<const char> data) {
        if(buffer) {
            CORRADE_ASSERT(data.size() <= buffer.size(),
                "Utility::formatInto(): buffer too small, expected at least" << bufferOffset + data.size() << "but got" << bufferOffset + buffer.size(), );
            /* strncpy() would stop on \0 characters */
            std::memcpy(buffer + bufferOffset, data, data.size());
        }
        bufferOffset += data.size();
    }, [&buffer, &bufferOffset](BufferFormatter& formatter) {
        if(buffer) {
            formatter.size = formatter(buffer.suffix(bufferOffset));
            CORRADE_ASSERT(bufferOffset + formatter.size <= buffer.size(),
                "Utility::formatInto(): buffer too small, expected at least" << bufferOffset + formatter.size << "but got" << buffer.size(), );
        } else if(formatter.size == ~std::size_t{})
            formatter.size = formatter(nullptr);
        bufferOffset += formatter.size;
    }, {format, std::strlen(format)}, formatters);
    return bufferOffset;
}

std::size_t formatInto(std::string& buffer, const std::size_t offset, const char* const format, const Containers::ArrayView<BufferFormatter> formatters) {
    const std::size_t size = formatInto(nullptr, format, formatters);
    if(buffer.size() < offset + size) buffer.resize(offset + size);
    /* Under C++11, the character storage always includes the null terminator
       and printf() always wants to print the null terminator, so allow it */
    return offset + formatInto({&buffer[offset], buffer.size() + 1}, format, formatters);
}

void formatInto(std::FILE* const file,  const char* format, const Containers::ArrayView<FileFormatter> formatters) {
    formatWith([&file](Containers::ArrayView<const char> data) {
        fwrite(data.data(), data.size(), 1, file);
    }, [&file](const FileFormatter& formatter) {
        formatter(file);
    }, {format, std::strlen(format)}, formatters);
}

}

}}
