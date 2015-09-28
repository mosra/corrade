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

#include "Debug.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace Corrade { namespace Utility {

namespace {

template<class T> inline void toStream(std::ostream& s, const T& value) {
    s << value;
}

template<> inline void toStream<Implementation::DebugOstreamFallback>(std::ostream& s, const Implementation::DebugOstreamFallback& value) {
    value.apply(s);
}

}

std::ostream* Debug::_globalOutput = &std::cout;
std::ostream* Warning::globalWarningOutput = &std::cerr;
std::ostream* Error::globalErrorOutput = &std::cerr;

void Debug::setOutput(std::ostream* output) {
    _globalOutput = output;
}

void Warning::setOutput(std::ostream* output) {
    globalWarningOutput = output;
}

void Error::setOutput(std::ostream* output) {
    globalErrorOutput = output;
}

Debug::Debug(): _output(_globalOutput), _flags(0x01 | SpaceAfterEachValue | NewLineAtTheEnd) {}

Warning::Warning(): Debug(globalWarningOutput) {}

Error::Error(): Debug(globalErrorOutput) {}

Debug::Debug(const Debug& other): _output(other._output), _flags(other._flags) {
    if(!(other._flags & 0x01))
        setFlag(NewLineAtTheEnd, false);
}

Debug::~Debug() {
    if(_output && !(_flags & 0x01) && (_flags & NewLineAtTheEnd))
        *_output << std::endl;
}

void Debug::setFlag(Flag flag, bool value) {
    flag = static_cast<Flag>(flag & ~0x01);
    if(value) _flags |= flag;
    else _flags &= ~flag;
}

template<class T> Debug& Debug::print(const T& value) {
    if(!_output) return *this;

    /* Separate values with spaces, if enabled */
    if(_flags & 0x01) _flags &= ~0x01;
    else if(_flags & Debug::SpaceAfterEachValue) *_output << " ";

    toStream(*_output, value);
    return *this;
}

Debug& Debug::operator<<(const std::string& value) { return print(value); }
Debug& Debug::operator<<(const void* value) { return print(value); }
Debug& Debug::operator<<(const char* value) { return print(value); }
Debug& Debug::operator<<(bool value) { return print(value ? "true" : "false"); }
Debug& Debug::operator<<(int value) { return print(value); }
Debug& Debug::operator<<(long value) { return print(value); }
Debug& Debug::operator<<(long long value) { return print(value); }
Debug& Debug::operator<<(unsigned value) { return print(value); }
Debug& Debug::operator<<(unsigned long value) { return print(value); }
Debug& Debug::operator<<(unsigned long long value) { return print(value); }
Debug& Debug::operator<<(float value) { return print(value); }
Debug& Debug::operator<<(double value) { return print(value); }
#ifndef CORRADE_TARGET_EMSCRIPTEN
Debug& Debug::operator<<(long double value) { return print(value); }
#endif

Debug& Debug::operator<<(char32_t value) {
    std::ostringstream o;
    o << "U+" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << value;
    return print(o.str());
}

Debug& Debug::operator<<(const char32_t* value) {
    return *this << std::u32string(value);
}

#ifndef DOXYGEN_GENERATING_OUTPUT
Debug& Debug::operator<<(Implementation::DebugOstreamFallback&& value) {
    return print(value);
}
#endif

}}
