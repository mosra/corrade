#ifndef Corrade_Utility_Endianness_h
#define Corrade_Utility_Endianness_h
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
 * @brief Class Corrade::Utility::Endianness
 */

#include <cstdint>

#include "utilities.h"

#include "corradeCompatibility.h"
#include "corradeConfigure.h"

namespace Corrade { namespace Utility {

/**
 * @brief %Endianness related functions
 */
class Endianness {
    public:
        /** @brief Whether actual system is Big-Endian */
        inline constexpr static bool isBigEndian() {
            #ifdef CORRADE_BIG_ENDIAN
            return true;
            #else
            return false;
            #endif
        }

        /**
         * @brief Convert number from or to Big-Endian
         * @param number    Number to convert
         * @return Number as Big-Endian. On Big-Endian systems returns unchanged
         *      value.
         */
        template<class T> inline static T bigEndian(T number) {
            #ifdef CORRADE_BIG_ENDIAN
            return number;
            #else
            return swap<sizeof(T)>(bitCast<typename TypeFor<sizeof(T)>::Type>(number));
            #endif
        }

        /**
         * @brief Convert number from or to Little-Endian
         * @param number    Number to convert
         * @return Number as Little-Endian. On Little-Endian systems returns
         *      unchanged value.
         */
        template<class T> inline static T littleEndian(T number) {
            #ifdef CORRADE_BIG_ENDIAN
            return swap<sizeof(T)>(bitCast<typename TypeFor<sizeof(T)>::Type>(number));
            #else
            return number;
            #endif
        }

    private:
        template<std::size_t size> struct TypeFor {};

        template<std::size_t size> static typename TypeFor<size>::Type swap(typename TypeFor<size>::Type value);
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct Endianness::TypeFor<2> { typedef std::uint16_t Type; };
template<> struct Endianness::TypeFor<4> { typedef std::uint32_t Type; };
template<> struct Endianness::TypeFor<8> { typedef std::uint64_t Type; };

template<> inline std::uint16_t Endianness::swap<2>(std::uint16_t value) {
    return (value >> 8) |
           (value << 8);
}
template<> inline std::uint32_t Endianness::swap<4>(std::uint32_t value) {
    return (value >> 24) |
          ((value << 8) & 0x00ff0000u) |
          ((value >> 8) & 0x0000ff00u) |
           (value << 24);
}
template<> inline std::uint64_t Endianness::swap<8>(std::uint64_t value) {
    return (value >> 56) |
          ((value << 40) & 0x00ff000000000000ull) |
          ((value << 24) & 0x0000ff0000000000ull) |
          ((value <<  8) & 0x000000ff00000000ull) |
          ((value >>  8) & 0x00000000ff000000ull) |
          ((value >> 24) & 0x0000000000ff0000ull) |
          ((value >> 40) & 0x000000000000ff00ull) |
           (value << 56);
}
#endif

}}

#endif
