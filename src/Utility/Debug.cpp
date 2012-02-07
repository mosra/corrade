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

using namespace std;

namespace Corrade { namespace Utility {

ostream* Debug::globalOutput = &cout;
ostream* Warning::globalWarningOutput = &cerr;
ostream* Error::globalErrorOutput = &cerr;

Debug::Debug(const Debug& other): output(other.output), flags(other.flags) {
    if(!(other.flags & 0x01))
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

}}
