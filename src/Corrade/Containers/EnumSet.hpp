#ifndef Corrade_Containers_EnumSet_hpp
#define Corrade_Containers_EnumSet_hpp
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

/** @file
 * @brief Function @ref Corrade::Containers::enumSetDebugOutput()
 */

#include <initializer_list>

#include "Corrade/Containers/EnumSet.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

/** @relatedalso EnumSet
@brief Print enum set to debug output
@param debug    Debug output
@param value    Value to be printed
@param empty    What to print in case of an empty enum set
@param enums    Recognized enum values

Assuming underlying enum type has already implemented `operator<<` for
@ref Utility::Debug, this function is able to print value of given enum set.
Example definition:

@snippet Containers.cpp enumSetDebugOutput

The usage would be then straightforward:

@snippet Containers.cpp enumSetDebugOutput-usage

@attention This function assumes that the recognized values have unique bits
    set. The output is undefined if more than one value share the same bit.
*/
template<class T, typename std::underlying_type<T>::type fullValue> Utility::Debug& enumSetDebugOutput(Utility::Debug& debug, EnumSet<T, fullValue> value, const char* empty, std::initializer_list<T> enums) {
    /* Print the empty value in case there is nothing */
    if(!value) return debug << empty;

    /* Print known values, if set, and strip them out of the value */
    bool separate = false;
    for(const T e: enums) if(value >= e) {
        if(separate) debug << Utility::Debug::nospace << "|" << Utility::Debug::nospace;
        else separate = true;
        debug << e;

        /* Avoid stripping out the unknown bits by the EnumSet operator~ */
        value &= T(~typename std::underlying_type<T>::type(e));
    }

    /* If there are leftover, pass them to the original debug operator and
       expect it will print them as raw value */
    if(value) {
        if(separate) debug << Utility::Debug::nospace << "|" << Utility::Debug::nospace;
        debug << T(typename std::underlying_type<T>::type(value));
    }

    return debug;
}

}}

#endif
