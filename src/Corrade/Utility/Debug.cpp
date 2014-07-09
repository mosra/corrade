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

#include "Debug.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace Corrade { namespace Utility {

namespace {

template<class T>
inline void toStream(std::ostream& s, const T& value) {
    s << value;
}

template<>
inline void toStream<Debug::Fallback>(std::ostream& s, const Debug::Fallback& value) {
    value.apply(s);
}

}

std::ostream* Debug::globalOutput = &std::cout;
std::ostream* Warning::globalWarningOutput = &std::cerr;
std::ostream* Error::globalErrorOutput = &std::cerr;

void Debug::setOutput(std::ostream* output) {
    globalOutput = output;
}

void Warning::setOutput(std::ostream* output) {
    globalWarningOutput = output;
}

void Error::setOutput(std::ostream* output) {
    globalErrorOutput = output;
}

Debug::Debug(): output(globalOutput), flags(0x01 | SpaceAfterEachValue | NewLineAtTheEnd) {}

Warning::Warning(): Debug(globalWarningOutput) {}

Error::Error(): Debug(globalErrorOutput) {}

Debug::Debug(const Debug& other): output(other.output), flags(other.flags) {
    if(!(other.flags & 0x01))
        setFlag(NewLineAtTheEnd, false);
}

Debug::Debug(Debug& other): output(other.output), flags(other.flags) {
    other.flags &= ~0x01;

    setFlag(NewLineAtTheEnd, false);
}

Debug::~Debug() {
    if(output && !(flags & 0x01) && (flags & NewLineAtTheEnd))
        *output << std::endl;
}

void Debug::setFlag(Flag flag, bool value) {
    flag = static_cast<Flag>(flag & ~0x01);
    if(value) flags |= flag;
    else flags &= ~flag;
}

template<class T> Debug Debug::print(const T& value) {
    if(!output) return *this;

    /* Separate values with spaces, if enabled */
    if(flags & 0x01) flags &= ~0x01;
    else if(flags & Debug::SpaceAfterEachValue) *output << " ";

    toStream(*output, value);
    return *this;
}

Debug Debug::operator<<(const std::string& value) { return print(value); }
Debug Debug::operator<<(const void* value) { return print(value); }
Debug Debug::operator<<(const char* value) { return print(value); }
Debug Debug::operator<<(bool value) { return print(value ? "true" : "false"); }
Debug Debug::operator<<(int value) { return print(value); }
Debug Debug::operator<<(long value) { return print(value); }
Debug Debug::operator<<(long long value) { return print(value); }
Debug Debug::operator<<(unsigned value) { return print(value); }
Debug Debug::operator<<(unsigned long value) { return print(value); }
Debug Debug::operator<<(unsigned long long value) { return print(value); }
Debug Debug::operator<<(float value) { return print(value); }
Debug Debug::operator<<(double value) { return print(value); }
#ifndef CORRADE_TARGET_EMSCRIPTEN
Debug Debug::operator<<(long double value) { return print(value); }
#endif

Debug Debug::operator<<(char32_t value) {
    std::ostringstream o;
    o << "U+" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << value;
    return print(o.str());
}

Debug Debug::operator<<(const char32_t* value) {
    return *this << std::u32string(value);
}

Debug Debug::operator<<(Fallback&& value) { return print(value); }

}}
