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

#include "utilities.h"

#ifndef _WIN32
#include "unistd.h"
#else
#include <windows.h>
#endif

namespace Corrade { namespace Utility {

unsigned int log2(unsigned int number) {
    int log = 0;
    while(number >>= 1)
        ++log;
    return log;
}

void sleep(std::size_t ms) {
    #ifndef _WIN32
    usleep(ms*1000);
    #else
    Sleep(ms);
    #endif
}

}}
