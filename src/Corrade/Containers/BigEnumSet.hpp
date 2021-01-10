#ifndef Corrade_Containers_BigEnumSet_hpp
#define Corrade_Containers_BigEnumSet_hpp
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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
 * @brief Function @ref Corrade::Containers::bigEnumSetDebugOutput()
 * @m_since_latest
 */

#include <initializer_list>

#include "Corrade/Containers/BigEnumSet.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

/** @relatedalso BigEnumSet
@brief Print a big enum set to debug output
@param debug    Debug output
@param value    Value to be printed
@param empty    What to print in case of an empty enum set
@m_since_latest

Compared to @ref enumSetDebugOutput(), this function doesn't need an explicit
list of known values but will instead go through all set bits and print them
one by one. This also means unknown bits, if any, will be interleaved with the
known ones. Example definition:

@snippet Containers.cpp bigEnumSetDebugOutput

The output is then as follows:

@snippet Containers.cpp bigEnumSetDebugOutput-usage
*/
/* The set has to be taken by value because it gets modified in the process */
template<class T, std::size_t size> Utility::Debug& bigEnumSetDebugOutput(Utility::Debug& debug, BigEnumSet<T, size> value, const char* empty) {
    /* Print the empty value in case there is nothing */
    if(!value) return debug << empty;

    /* Go through all bits in the range and print each of them, if set. This
       will mean known and unknown values will be interleaved, but better than
       forcing users to supply a list of 100+ values like with EnumSet. */
    bool separate = false;
    for(std::size_t i = 0; value && i != size*64; ++i) {
        if(!(value & T(i))) continue;

        if(separate) debug << Utility::Debug::nospace << "|" << Utility::Debug::nospace;
        else separate = true;
        debug << T(i);

        /* Clear the value from the enum so we can do an early exit if there's
           no more bits set */
        value &= ~T(i);
    }

    return debug;
}

}}

#endif
