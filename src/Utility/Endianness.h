#ifndef Kompas_Utility_Endianness_h
#define Kompas_Utility_Endianness_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Kompas::Utility::Endianness
 */

#include "utilities.h"

namespace Kompas { namespace Utility {

/**
 * @brief %Endianness related functions
 */
class UTILITY_EXPORT Endianness {
    public:
        /** @brief Whether actual system is Big-Endian */
        static bool isBigEndian();

        /**
         * @brief Convert number from or to Big-Endian
         * @param number    Number to convert
         * @return Number as Big-Endian. On Big-Endian systems returns unchanged
         *      value.
         */
        template<class T> inline static T bigEndian(T number) {
            T output = number;
            _bigEndian(reinterpret_cast<unsigned char*>(&number),
                       reinterpret_cast<unsigned char*>(&output),
                       sizeof(number));
            return output;
        }

        /**
         * @brief Convert number from or to Little-Endian
         * @param number    Number to convert
         * @return Number as Little-Endian. On Little-Endian systems returns
         *      unchanged value.
         */
        template<class T> inline static T littleEndian(T number) {
            T output = number;
            _littleEndian(reinterpret_cast<unsigned char*>(&number),
                          reinterpret_cast<unsigned char*>(&output),
                          sizeof(number));
            return output;
        }

    private:
        static void _bigEndian(unsigned char* number, unsigned char* output, int size);
        static void _littleEndian(unsigned char* number, unsigned char* output, int size);
};

}}

#endif
