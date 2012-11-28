#ifndef Corrade_Utility_utilities_h
#define Corrade_Utility_utilities_h
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
CORRADE_UTILITY_EXPORT void sleep(size_t ms);

/*@}*/

/** @{ @name Macros */

/**
 * @brief Disable copying of given class
 * @param class             Class name
 *
 * Makes copy constructor and assignment operator private, so the class cannot
 * be copied. Should be placed at the beginning of class definition
 */
#define DISABLE_COPY(class) \
    class(const class&); \
    class& operator=(const class&);

/**
 * @brief Declare automatic initializer
 * @param function Initializer function name of type int(*)().
 *
 * Function passed as argument will be called even before entering main()
 * function. This is usable when e.g. automatically registering plugins or data
 * resources without forcing the user to write additional code in main().
 * @attention This macro does nothing in static libraries.
 */
#define AUTOMATIC_INITIALIZER(function)                                       \
    static const int __##function = function();

/**
 * @brief Declare automatic initializer
 * @param function Finalizer function name of type int(*)().
 *
 * Function passed as argument will be called even before entering main()
 * function. This is usable in conjuction with ::AUTOMATIC_INITIALIZER() when
 * there is need to properly discard initialized data.
 * @attention This macro does nothing in static libraries.
 */
#define AUTOMATIC_FINALIZER(function)                                         \
    class __##function {                                                      \
        public:                                                               \
            inline __##function() {}                                          \
            inline ~__##function() { function(); }                            \
    } __##function;

/*@}*/

}}

#endif
