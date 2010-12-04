#ifndef Map2X_Utility_utilities_h
#define Map2X_Utility_utilities_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Basic utilities
 */

#include <string>
#include <vector>

namespace Map2X { namespace Utility {

/** @{ @name Number utilities */

/** @brief Power of 2
 * @return 2^number
 */
inline unsigned int pow2(unsigned int number) { return 1 << number; }

/*@}*/

/** @{ @name String utilities */

/**
 * @brief Trim leading and trailing whitespaces from string
 * @param str           String to be trimmed
 * @param characters    Characters which will be trimmed
 * @return Trimmed string
 */
std::string trim(std::string str, const std::string& characters = " \t\f\v\r\n");

/**
 * @brief Split string on given character
 * @param str               String to be splitted
 * @param delim             Delimiter
 * @param keepEmptyParts    Whether to keep empty parts
 * @return Vector of splitted strings
 */
std::vector<std::string> split(const std::string& str, char delim, bool keepEmptyParts = true);

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
 * @todo Test this
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
 * @todo Test this
 *
 * Function passed as argument will be called even before entering main()
 * function. This is usable in conjuction with ::AUTOMATIC_INITIALIZER() when
 * there is need to properly discard initialized data.
 * @attention This macro does nothing in static libraries.
 */
#define AUTOMATIC_FINALIZER(function)                                         \
    class __##function {                                                      \
        public:                                                               \
            inline __##function() {};                                         \
            inline ~__##function() { function(); };                           \
    } __##function;

/*@}*/

}}

#endif
