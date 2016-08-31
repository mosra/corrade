/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>

/* For colored output */
#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN
#include <windows.h>
#include <wincon.h>
#endif

namespace Corrade { namespace Utility {

namespace {

template<class T> inline void toStream(std::ostream& s, const T& value) {
    s << value;
}

template<> inline void toStream<Implementation::DebugOstreamFallback>(std::ostream& s, const Implementation::DebugOstreamFallback& value) {
    value.apply(s);
}

#if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
HANDLE streamOutputHandle(const std::ostream* s) {
    return s == &std::cout ? GetStdHandle(STD_OUTPUT_HANDLE) :
           s == &std::cerr ? GetStdHandle(STD_ERROR_HANDLE) :
           INVALID_HANDLE_VALUE;
}
#endif

}

std::ostream* Debug::_globalOutput = &std::cout;
std::ostream* Warning::_globalWarningOutput = &std::cerr;
std::ostream* Error::_globalErrorOutput = &std::cerr;

#if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
Debug::Color Debug::_globalColor = Debug::Color::Default;
bool Debug::_globalColorBold = false;
#endif

template<Debug::Color c, bool bold> Debug::Modifier Debug::colorInternal() {
    return [](Debug& debug) {
        if(!debug._output || (debug._flags & InternalFlag::DisableColors)) return;

        debug._flags |= InternalFlag::ColorWritten|InternalFlag::ValueWritten;
        #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        HANDLE h = streamOutputHandle(debug._output);
        if(h != INVALID_HANDLE_VALUE) SetConsoleTextAttribute(h,
            (debug._previousColorAttributes & ~(FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY)) |
            char(c) |
            (bold ? FOREGROUND_INTENSITY : 0));
        #else
        _globalColor = c;
        _globalColorBold = bold;
        constexpr const char code[] = { '\033', '[', bold ? '1' : '0', ';', '3', '0' + char(c), 'm', '\0' };
        *debug._output << code;
        #endif
    };
}

inline void Debug::resetColorInternal() {
    if(!_output || !(_flags & InternalFlag::ColorWritten)) return;

    _flags &= ~InternalFlag::ColorWritten;
    _flags |= InternalFlag::ValueWritten;
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    HANDLE h = streamOutputHandle(_output);
    if(h != INVALID_HANDLE_VALUE)
        SetConsoleTextAttribute(h, _previousColorAttributes);
    #else
    if(_previousColor != Color::Default || _previousColorBold) {
        const char code[] = { '\033', '[', _previousColorBold ? '1' : '0', ';', '3', char('0' + char(_previousColor)), 'm', '\0' };
        *_output << code;
    } else *_output << "\033[0m";

    _globalColor = _previousColor;
    _globalColorBold = _previousColorBold;
    #endif
}

auto Debug::color(Color color) -> Modifier {
    /* Crazy but working solution to work around the need for capturing lambda
       which disallows converting it to function pointer */
    switch(color) {
        #define _c(color) case Color::color: return colorInternal<Color::color, false>();
        _c(Black)
        _c(Red)
        _c(Green)
        _c(Yellow)
        _c(Blue)
        _c(Magenta)
        _c(Cyan)
        _c(White)
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        _c(Default)
        #endif
        #undef _c
    }

    return [](Debug&) {};
}

auto Debug::boldColor(Color color) -> Modifier {
    /* Crazy but working solution to work around the need for capturing lambda
       which disallows converting it to function pointer */
    switch(color) {
        #define _c(color) case Color::color: return colorInternal<Color::color, true>();
        _c(Black)
        _c(Red)
        _c(Green)
        _c(Yellow)
        _c(Blue)
        _c(Magenta)
        _c(Cyan)
        _c(White)
        #if !defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_UTILITY_USE_ANSI_COLORS)
        _c(Default)
        #endif
        #undef _c
    }

    return [](Debug&) {};
}

void Debug::resetColor(Debug& debug) {
    debug.resetColorInternal();
}

#ifdef CORRADE_BUILD_DEPRECATED
void Debug::setOutput(std::ostream* output) {
    _globalOutput = output;
}

void Warning::setOutput(std::ostream* output) {
    _globalWarningOutput = output;
}

void Error::setOutput(std::ostream* output) {
    _globalErrorOutput = output;
}
#endif

Debug::Debug(std::ostream* const output, const Flags flags): _flags{InternalFlag(static_cast<unsigned char>(flags))|InternalFlag::NoSpaceBeforeNextValue} {
    /* Save previous global output and replace it with current one */
    _previousGlobalOutput = _globalOutput;
    _globalOutput = _output = output;

    /* Save previous global color */
    #if defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_UTILITY_USE_ANSI_COLORS)
    HANDLE h = streamOutputHandle(_output);
    if(h != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(h, &csbi);
        _previousColorAttributes = csbi.wAttributes;
    }
    #else
    _previousColor = _globalColor;
    _previousColorBold = _globalColorBold;
    #endif
}

Warning::Warning(std::ostream* const output, const Flags flags): Debug{flags} {
    /* Save previous global output and replace it with current one */
    _previousGlobalWarningOutput = _globalWarningOutput;
    _globalWarningOutput = _output = output;
}

Error::Error(std::ostream* const output, const Flags flags): Debug{flags} {
    /* Save previous global output and replace it with current one */
    _previousGlobalErrorOutput = _globalErrorOutput;
    _globalErrorOutput = _output = output;
}

Debug::Debug(const Flags flags): Debug{_globalOutput, flags} {}
Warning::Warning(const Flags flags): Warning{_globalWarningOutput, flags} {}
Error::Error(const Flags flags): Error{_globalErrorOutput, flags} {}

Debug::~Debug() {
    /* Reset output color */
    resetColorInternal();

    /* Newline at the end */
    if(_output && (_flags & InternalFlag::ValueWritten) && !(_flags & InternalFlag::NoNewlineAtTheEnd))
        *_output << std::endl;

    /* Reset previous global output */
    _globalOutput = _previousGlobalOutput;
}

Warning::~Warning() {
    _globalWarningOutput = _previousGlobalWarningOutput;
}

Error::~Error() {
    _globalErrorOutput = _previousGlobalErrorOutput;
}

Fatal::~Fatal() {
    std::exit(_exitCode);
}

template<class T> Debug& Debug::print(const T& value) {
    if(!_output) return *this;

    /* Separate values with spaces, if enabled */
    if(_flags & InternalFlag::NoSpaceBeforeNextValue)
        _flags &= ~InternalFlag::NoSpaceBeforeNextValue;
    else *_output << ' ';

    toStream(*_output, value);

    _flags |= InternalFlag::ValueWritten;
    return *this;
}

Debug& Debug::operator<<(const std::string& value) { return print(value); }

Debug& Debug::operator<<(const void* const value) {
    std::ostringstream o;
    o << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(value);
    return print(o.str());
}

Debug& Debug::operator<<(const char* value) { return print(value); }
Debug& Debug::operator<<(bool value) { return print(value ? "true" : "false"); }
Debug& Debug::operator<<(int value) { return print(value); }
Debug& Debug::operator<<(long value) { return print(value); }
Debug& Debug::operator<<(long long value) { return print(value); }
Debug& Debug::operator<<(unsigned value) { return print(value); }
Debug& Debug::operator<<(unsigned long value) { return print(value); }
Debug& Debug::operator<<(unsigned long long value) { return print(value); }
Debug& Debug::operator<<(float value) {
    if(!_output) return *this;
    /* The default. Source: http://en.cppreference.com/w/cpp/io/ios_base/precision */
    *_output << std::setprecision(6);
    return print(value);
}
Debug& Debug::operator<<(double value) {
    if(!_output) return *this;
    *_output << std::setprecision(15);
    return print(value);
}
#ifndef CORRADE_TARGET_EMSCRIPTEN
Debug& Debug::operator<<(long double value) {
    if(!_output) return *this;
    *_output << std::setprecision(18);
    return print(value);
}
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
