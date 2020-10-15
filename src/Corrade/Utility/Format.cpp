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

#include "Format.h"
#include "FormatStl.h"

#include <cstring>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/TypeTraits.h"

namespace Corrade { namespace Utility { namespace Implementation {

enum class FormatType: unsigned char {
    Unspecified,
    Octal,
    Decimal,
    Hexadecimal,
    HexadecimalUppercase,
    Float,
    FloatUppercase,
    FloatExponent,
    FloatExponentUppercase,
    FloatFixed,
    FloatFixedUppercase
};

template<class> char formatTypeChar(FormatType type);

template<> char formatTypeChar<int>(FormatType type) {
    switch(type) {
        case FormatType::Unspecified:
        case FormatType::Decimal: return 'i';
        case FormatType::Octal: return 'o';
        case FormatType::Hexadecimal: return 'x';
        case FormatType::HexadecimalUppercase: return 'X';

        case FormatType::Float:
        case FormatType::FloatUppercase:
        case FormatType::FloatExponent:
        case FormatType::FloatExponentUppercase:
        case FormatType::FloatFixed:
        case FormatType::FloatFixedUppercase:
            /* Return some reasonable default so we can test for the assert */
            CORRADE_ASSERT_UNREACHABLE("Utility::format(): floating-point type used for an integral value", 'i');
    }

    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

template<> char formatTypeChar<unsigned int>(FormatType type) {
    switch(type) {
        case FormatType::Unspecified:
        case FormatType::Decimal: return 'u';
        case FormatType::Octal: return 'o';
        case FormatType::Hexadecimal: return 'x';
        case FormatType::HexadecimalUppercase: return 'X';

        case FormatType::Float:
        case FormatType::FloatUppercase:
        case FormatType::FloatExponent:
        case FormatType::FloatExponentUppercase:
        case FormatType::FloatFixed:
        case FormatType::FloatFixedUppercase:
            /* Return some reasonable default so we can test for the assert */
            CORRADE_ASSERT_UNREACHABLE("Utility::format(): floating-point type used for an integral value", 'u');
    }

    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

template<> char formatTypeChar<float>(FormatType type) {
    switch(type) {
        case FormatType::Unspecified:
        case FormatType::Float: return 'g';
        case FormatType::FloatUppercase: return 'G';
        case FormatType::FloatExponent: return 'e';
        case FormatType::FloatExponentUppercase: return 'E';
        case FormatType::FloatFixed: return 'f';
        case FormatType::FloatFixedUppercase: return 'F';

        case FormatType::Decimal:
        case FormatType::Octal:
        case FormatType::Hexadecimal:
        case FormatType::HexadecimalUppercase:
            /* Return some reasonable default so we can test for the assert */
            CORRADE_ASSERT_UNREACHABLE("Utility::format(): integral type used for a floating-point value", 'g');
    }

    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

std::size_t Formatter<int>::format(const Containers::ArrayView<char>& buffer, const int value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', formatTypeChar<int>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
    return {};
}
void Formatter<int>::format(std::FILE* const file, const int value, int precision, FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', formatTypeChar<int>(type), 0 };
    std::fprintf(file, format, precision, value);
}
std::size_t Formatter<unsigned int>::format(const Containers::ArrayView<char>& buffer, const unsigned int value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', formatTypeChar<unsigned int>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
}
void Formatter<unsigned int>::format(std::FILE* const file, const unsigned int value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', formatTypeChar<unsigned int>(type), 0 };
    std::fprintf(file, format, precision, value);
}
std::size_t Formatter<long long>::format(const Containers::ArrayView<char>& buffer, const long long value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', 'l', 'l', formatTypeChar<int>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
}
void Formatter<long long>::format(std::FILE* const file, const long long value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', 'l', 'l', formatTypeChar<int>(type), 0 };
    std::fprintf(file, format, precision, value);
}
std::size_t Formatter<unsigned long long>::format(const Containers::ArrayView<char>& buffer, const unsigned long long value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', 'l', 'l', formatTypeChar<unsigned int>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
}
void Formatter<unsigned long long>::format(std::FILE* const file, const unsigned long long value, int precision, const FormatType type) {
    if(precision == -1) precision = 1;
    const char format[]{ '%', '.', '*', 'l', 'l', formatTypeChar<unsigned int>(type), 0 };
    std::fprintf(file, format, precision, value);
}

std::size_t Formatter<float>::format(const Containers::ArrayView<char>& buffer, const float value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<float>::Digits;
    const char format[]{ '%', '.', '*', formatTypeChar<float>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, double(value));
}
void Formatter<float>::format(std::FILE* const file, const float value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<float>::Digits;
    const char format[]{ '%', '.', '*', formatTypeChar<float>(type), 0 };
    std::fprintf(file, format, precision, double(value));
}

std::size_t Formatter<double>::format(const Containers::ArrayView<char>& buffer, const double value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<double>::Digits;
    const char format[]{ '%', '.', '*', formatTypeChar<float>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
}
void Formatter<double>::format(std::FILE* const file, const double value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<double>::Digits;
    const char format[]{ '%', '.', '*', formatTypeChar<float>(type), 0 };
    std::fprintf(file, format, precision, value);
}

std::size_t Formatter<long double>::format(const Containers::ArrayView<char>& buffer, const long double value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<long double>::Digits;
    const char format[]{ '%', '.', '*', 'L', formatTypeChar<float>(type), 0 };
    return std::snprintf(buffer, buffer.size(), format, precision, value);
}
void Formatter<long double>::format(std::FILE* const file, const long double value, int precision, const FormatType type) {
    if(precision == -1) precision = Implementation::FloatPrecision<long double>::Digits;
    const char format[]{ '%', '.', '*', 'L', formatTypeChar<float>(type), 0 };
    std::fprintf(file, format, precision, value);
}

std::size_t Formatter<Containers::StringView>::format(const Containers::ArrayView<char>& buffer, const Containers::StringView value, const int precision, const FormatType type) {
    std::size_t size = value.size();
    if(std::size_t(precision) < size) size = precision;
    CORRADE_ASSERT(type == FormatType::Unspecified,
        "Utility::format(): type specifier can't be used for a string value", {});
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(type);
    #endif
    /* strncpy() would stop on \0 characters */
    if(buffer) std::memcpy(buffer, value.data(), size);
    return size;
}
void Formatter<Containers::StringView>::format(std::FILE* const file, const Containers::StringView value, const int precision, const FormatType type) {
    std::size_t size = value.size();
    if(std::size_t(precision) < size) size = precision;
    CORRADE_ASSERT(type == FormatType::Unspecified,
        "Utility::format(): type specifier can't be used for a string value", );
    #ifdef CORRADE_NO_ASSERT
    static_cast<void>(type);
    #endif
    std::fwrite(value.data(), size, 1, file);
}
std::size_t Formatter<const char*>::format(const Containers::ArrayView<char>& buffer, const char* value, const int precision, const FormatType type) {
    return Formatter<Containers::StringView>::format(buffer, value, precision, type);
}
void Formatter<const char*>::format(std::FILE* const file, const char* value, const int precision, const FormatType type) {
    Formatter<Containers::StringView>::format(file, value, precision, type);
}
#ifdef CORRADE_BUILD_DEPRECATED
std::size_t Formatter<Containers::ArrayView<const char>>::format(const Containers::ArrayView<char>& buffer, const Containers::ArrayView<const char> value, const int precision, const FormatType type) {
    return Formatter<Containers::StringView>::format(buffer, value, precision, type);
}
void Formatter<Containers::ArrayView<const char>>::format(std::FILE* const file, const Containers::ArrayView<const char> value, const int precision, const FormatType type) {
    Formatter<Containers::StringView>::format(file, value, precision, type);
}
#endif
std::size_t Formatter<std::string>::format(const Containers::ArrayView<char>& buffer, const std::string& value, const int precision, const FormatType type) {
    return Formatter<Containers::StringView>::format(buffer, value, precision, type);
}
void Formatter<std::string>::format(std::FILE* const file, const std::string& value, const int precision, const FormatType type) {
    return Formatter<Containers::StringView>::format(file, value, precision, type);
}

namespace {

int parseNumber(Containers::StringView format, std::size_t& formatOffset) {
    int number = -1;
    while(formatOffset < format.size() && format[formatOffset] >= '0' && format[formatOffset] <= '9') {
        if(number == -1) number = 0;
        else number *= 10;
        number += (format[formatOffset] - '0');
        ++formatOffset;
    }
    return number;
}

template<class Writer, class FormattedWriter, class Formatter> void formatWith(const Writer writer, const FormattedWriter formattedWriter, const Containers::StringView format, const Containers::ArrayView<Formatter> formatters) {
    bool inPlaceholder = false;
    std::size_t placeholderOffset = 0;
    std::size_t formatterToGo = 0;
    int placeholderIndex = -1;
    int precision = -1;
    FormatType type = FormatType::Unspecified;
    for(std::size_t formatOffset = 0; formatOffset != format.size(); ) {
        /* Placeholder begin (or escaped {) */
        if(format[formatOffset] == '{') {
            if(formatOffset + 1 < format.size() && format[formatOffset+1] == '{') {
                writer(format.slice(formatOffset, formatOffset + 1));
                formatOffset += 2;
                continue;
            }

            CORRADE_INTERNAL_ASSERT(!inPlaceholder);
            inPlaceholder = true;
            placeholderOffset = formatOffset;
            placeholderIndex = -1;
            precision = -1;
            type = FormatType::Unspecified;

            ++formatOffset;
            continue;
        }

        /* Placeholder end (or escaped }) */
        if(format[formatOffset] == '}') {
            if(!inPlaceholder && formatOffset + 1 < format.size() && format[formatOffset+1] == '}') {
                writer(format.slice(formatOffset, formatOffset + 1));
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
                formattedWriter(formatters[formatterToGo], precision, type);

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
            placeholderIndex = parseNumber(format, formatOffset);

            /* Formatting options */
            if(formatOffset < format.size() && format[formatOffset] == ':') {
                ++formatOffset;

                /* Precision */
                if(formatOffset + 1 < format.size() && format[formatOffset] == '.') {
                    ++formatOffset;
                    precision = parseNumber(format, formatOffset);
                    CORRADE_ASSERT(precision != -1,
                        "Utility::format(): invalid character in precision specifier:" << format.slice(formatOffset, formatOffset + 1), );
                }

                /* Type */
                if(formatOffset < format.size() && format[formatOffset] != '}') {
                    switch(format[formatOffset]) {
                        /** @todo binary */
                        case 'o':
                            type = FormatType::Octal;
                            break;
                        case 'd':
                            type = FormatType::Decimal;
                            break;
                        case 'x':
                            type = FormatType::Hexadecimal;
                            break;
                        case 'X':
                            type = FormatType::HexadecimalUppercase;
                            break;
                        case 'g':
                            type = FormatType::Float;
                            break;
                        case 'G':
                            type = FormatType::FloatUppercase;
                            break;
                        case 'e':
                            type = FormatType::FloatExponent;
                            break;
                        case 'E':
                            type = FormatType::FloatExponentUppercase;
                            break;
                        case 'f':
                            type = FormatType::FloatFixed;
                            break;
                        case 'F':
                            type = FormatType::FloatFixedUppercase;
                            break;
                        default:
                            CORRADE_ASSERT(false,
                                "Utility::format(): invalid type specifier:" << format.slice(formatOffset, formatOffset + 1), );
                    }
                    ++formatOffset;
                }
            }

            /* Unexpected end, break -- the assert at the end of function
               takes care of this */
            if(formatOffset == format.size()) break;

            /* Next should be the placeholder end */
            CORRADE_ASSERT(format[formatOffset] == '}',
                "Utility::format(): unknown placeholder content:" << format.slice(formatOffset, formatOffset + 1), );
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

std::size_t formatInto(const Containers::ArrayView<char>& buffer, const char* const format, BufferFormatter* const formatters, std::size_t formatterCount) {
    std::size_t bufferOffset = 0;
    formatWith([&buffer, &bufferOffset](Containers::ArrayView<const char> data) {
        if(buffer) {
            CORRADE_ASSERT(data.size() <= buffer.size(),
                "Utility::formatInto(): buffer too small, expected at least" << bufferOffset + data.size() << "but got" << bufferOffset + buffer.size(), );
            /* strncpy() would stop on \0 characters */
            std::memcpy(buffer + bufferOffset, data, data.size());
        }
        bufferOffset += data.size();
    }, [&buffer, &bufferOffset](BufferFormatter& formatter, int precision, FormatType type) {
        if(buffer) {
            formatter.size = formatter(buffer.suffix(bufferOffset), precision, type);
            CORRADE_ASSERT(bufferOffset + formatter.size <= buffer.size(),
                "Utility::formatInto(): buffer too small, expected at least" << bufferOffset + formatter.size << "but got" << buffer.size(), );
        } else if(formatter.size == ~std::size_t{})
            formatter.size = formatter(nullptr, precision, type);
        bufferOffset += formatter.size;
    }, format, Containers::arrayView(formatters, formatterCount));
    return bufferOffset;
}

std::size_t formatInto(std::string& buffer, const std::size_t offset, const char* const format, BufferFormatter* const formatters, std::size_t formatterCount) {
    const std::size_t size = formatInto(nullptr, format, formatters, formatterCount);
    if(buffer.size() < offset + size) buffer.resize(offset + size);
    /* Under C++11, the character storage always includes the null terminator
       and printf() always wants to print the null terminator, so allow it */
    return offset + formatInto({&buffer[offset], buffer.size() + 1}, format, formatters, formatterCount);
}

void formatInto(std::FILE* const file, const char* format, FileFormatter* const formatters, std::size_t formatterCount) {
    formatWith([&file](Containers::ArrayView<const char> data) {
        fwrite(data.data(), data.size(), 1, file);
    }, [&file](const FileFormatter& formatter, int precision, FormatType type) {
        formatter(file, precision, type);
    }, format, Containers::arrayView(formatters, formatterCount));
}

}

}}
