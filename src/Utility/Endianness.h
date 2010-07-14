#ifndef Map2X_Utility_Endianness_h
#define Map2X_Utility_Endianness_h
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
 * @brief Class Map2X::Utility::Endianness
 */

namespace Map2X { namespace Utility {

#include "Utility/EndiannessConfigure.h"

/**
 * @brief %Endianness related functions
 *
 * @todo Might not work properly when crosscompiling with CMake:
 *      http://www.cmake.org/pipermail/cmake/2006-November/012016.html
 */
class Endianness {
    public:
        /** @fn bool isBigEndian()
         * @brief Whether actual system is Big-Endian.
         */

        /** @fn int template<class T> bigEndian(T number)
         * @brief Convert number from or to Big-Endian
         * @param number    Number to convert
         * @return Number as Big-Endian. On Big-Endian systems returns unchanged
         *      value.
         */

        /** @fn int template<class T> littleEndian(T number)
         * @brief Convert number from or to Little-Endian
         * @param number    Number to convert
         * @return Number as Little-Endian. On Little-Endian systems returns
         *      unchanged value.
         */

        #ifdef ENDIANNESS_BIG_ENDIAN
        inline static bool isBigEndian() { return true; }
        template<class T> inline static T bigEndian(T number) { return number; }
        template<class T> static T littleEndian(T number) {
        #else
        inline static bool isBigEndian() { return false; }
        template<class T> inline static T littleEndian(T number) { return number; }
        template<class T> static T bigEndian(T number) {
        #endif

            /* Byte size of a number, convert to byte array */
            int size = sizeof(number);
            unsigned char* from = reinterpret_cast<unsigned char*>(&number);
            unsigned char* to = new unsigned char[size];

            /* Reverse order of bytes */
            for(int i = 0; i != size; ++i)
                to[i] = from[size-i-1];

            return *reinterpret_cast<T*>(to);
        }
};

}}

#endif
