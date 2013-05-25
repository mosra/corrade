#ifndef Corrade_Utility_utilities_h
#define Corrade_Utility_utilities_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
 * @brief Basic utilities
 */

#include <cstring>

#include "corradeUtilityVisibility.h"

namespace Corrade { namespace Utility {

/** @{ @name Type utilities */

/**
 * @brief Cast type to another of the same size
 *
 * Unlike reinterpret_cast this doesn't break strict-aliasing rules.
 */
template<class To, class From> inline To bitCast(const From& from) {
    /* Based on https://github.com/chromium/chromium/blob/trunk/base/basictypes.h#L306 */
    static_assert(sizeof(From) == sizeof(To), "Utility::bitCast(): resulting type must have the same size");

    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
}

/*@}*/

/** @{ @name Numeric utilities */

/** @brief Power of 2
 * @return 2^number
 */
inline unsigned int pow2(unsigned int number) { return 1 << number; }

/**
 * @brief Base-2 logarithm
 * @return log2(number)
 */
CORRADE_UTILITY_EXPORT unsigned int log2(unsigned int number);

/*@}*/

/** @{ @name System utilities */

/**
 * @brief Sleep for given time
 * @param ms                Miliseconds
 */
CORRADE_UTILITY_EXPORT void sleep(std::size_t ms);

/*@}*/

/** @{ @name Macros */

/**
 * @brief Declare automatic initializer
 * @param function Initializer function name of type int(*)().
 *
 * Function passed as argument will be called even before entering main()
 * function. This is usable when e.g. automatically registering plugins or data
 * resources without forcing the user to write additional code in main().
 * @attention This macro does nothing in static libraries.
 */
#define AUTOMATIC_INITIALIZER(function)                                     \
    static const int initializer_##function = function();

/**
 * @brief Declare automatic initializer
 * @param function Finalizer function name of type int(*)().
 *
 * Function passed as argument will be called even before entering main()
 * function. This is usable in conjuction with ::AUTOMATIC_INITIALIZER() when
 * there is need to properly discard initialized data.
 * @attention This macro does nothing in static libraries.
 */
#define AUTOMATIC_FINALIZER(function)                                       \
    class Finalizer_##function {                                            \
        public:                                                             \
            Finalizer_##function() {}                                       \
            ~Finalizer_##function() { function(); }                         \
    } Finalizer_##function;

/*@}*/

}}

#endif
