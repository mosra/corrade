/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "Debug.h"

#include <iostream>

namespace Corrade { namespace Utility {

std::ostream* Debug::globalOutput = &std::cout;
std::ostream* Warning::globalWarningOutput = &std::cerr;
std::ostream* Error::globalErrorOutput = &std::cerr;

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

    *output << value;
    return *this;
}

Debug Debug::operator<<(const std::string& value) { return print(value); }
Debug Debug::operator<<(const void* value) { return print(value); }
Debug Debug::operator<<(const char* value) { return print(value); }
Debug Debug::operator<<(bool value) { return print(value ? "true" : "false"); }
Debug Debug::operator<<(char value) { return print(value); }
Debug Debug::operator<<(int value) { return print(value); }
Debug Debug::operator<<(long value) { return print(value); }
Debug Debug::operator<<(long long value) { return print(value); }
Debug Debug::operator<<(unsigned value) { return print(value); }
Debug Debug::operator<<(unsigned long value) { return print(value); }
Debug Debug::operator<<(unsigned long long value) { return print(value); }
Debug Debug::operator<<(float value) { return print(value); }
Debug Debug::operator<<(double value) { return print(value); }
Debug Debug::operator<<(long double value) { return print(value); }

}}
