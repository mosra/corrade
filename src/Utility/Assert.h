#ifndef Corrade_Utility_Assert_h
#define Corrade_Utility_Assert_h
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

/** @file
 * @brief Macros CORRADE_ASSERT(), CORRADE_INTERNAL_ASSERT()
 */

#include <cstdlib>

#include "Debug.h"

namespace Corrade { namespace Utility {

/** @hideinitializer
@brief Assertion macro
@param condition    Assert condition
@param message      Message on assertion fail
@param returnValue  Return value on assertion fail

Usable for sanity checks on user input, as it prints explanational message on
error.

By default, if assertion fails, @p message is printed to error output and the
application exits with value `-2`. If `CORRADE_GRACEFUL_ASSERT` is defined,
the message is printed and the function returns with @p returnValue. If
`CORRADE_NO_ASSERT` is defined, this macro does nothing. Example usage:
@code
T operator[](size_t pos) const {
    CORRADE_ASSERT(pos < size(), "Index out of range", T());
    return data[pos];
}
@endcode

If the function has return type `void`, just use empty parameter (allowed in
C++11):
@code
void compile() {
    CORRADE_ASSERT(!sources.empty(), "No sources added", );

    // ...
}
@endcode

You can use stream output operators for formatting just like when printing to
Debug output:
@code
CORRADE_ASSERT(pos < size(), "Cannot access element" << pos << "in array of size" << size(), );
@endcode

@see CORRADE_INTERNAL_ASSERT()
*/
#ifdef CORRADE_GRACEFUL_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << message;                           \
            return returnValue;                                             \
        }                                                                   \
    } while(0)
#else
#ifdef CORRADE_NO_ASSERT
#define CORRADE_ASSERT(condition, message, returnValue) do {} while(0)
#else
#define CORRADE_ASSERT(condition, message, returnValue)                     \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << message;                           \
            std::exit(-2);                                                  \
            return returnValue;                                             \
        }                                                                   \
    } while(0)
#endif
#endif

/** @hideinitializer
@brief Internal assertion macro
@param condition    Assert condition

Unlike CORRADE_ASSERT() usable for sanity checks on internal state, as it
prints what failed and where.

By default, if assertion fails, failed condition, file and line is printed to
error output and the application exits with value `-1`. If `CORRADE_NO_ASSERT`
is defined, this macro does nothing. Example usage:
@code
CORRADE_INTERNAL_ASSERT(!nullptr);
@endcode
*/
#ifdef CORRADE_NO_ASSERT
#define CORRADE_INTERNAL_ASSERT(condition) do {} while(0)
#else
#define CORRADE_INTERNAL_ASSERT(condition)                                  \
    do {                                                                    \
        if(!(condition)) {                                                  \
            Corrade::Utility::Error() << "Assertion" << #condition << "failed in" << __FILE__ << "on line" << __LINE__; \
            std::exit(-1);                                                  \
        }                                                                   \
    } while(0)
#endif

}}

#endif
