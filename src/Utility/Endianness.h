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

namespace Kompas { namespace Utility {

/**
 * @brief %Endianness related functions
 *
 * @todo Might not work properly when crosscompiling with CMake:
 *      http://www.cmake.org/pipermail/cmake/2006-November/012016.html
 * @todo Ugly and dangerous code. Templated functions must be in header to avoid
 *      including .cpp after every templated calll. But! - the actual swapping
 *      code needs generated header which says whether the system is Big or
 *      Little endian. So, to avoid generating that header for every call of
 *      these functions, that code must be in .cpp. Also GCC 4.5@@64bit doesn't
 *      like new unsigned char[] here, so the resulting number must be allocated
 *      in advance and the swapping function writes to that via pointer.
 */
class Endianness {
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
