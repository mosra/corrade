#ifndef Corrade_Utility_Endianness_h
#define Corrade_Utility_Endianness_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Utility::Endianness
 */

#include <cstdint>

#include "Corrade/configure.h"
#include "Corrade/Utility/utilities.h"

namespace Corrade { namespace Utility {

/**
@brief Endianness related functions
*/
class Endianness {
    public:
        Endianness() = delete;

        /** @brief Endian-swap bytes of given value */
        template<class T> static T swap(T value) {
            return bitCast<T>(swap<sizeof(T)>(bitCast<typename TypeFor<sizeof(T)>::Type>(value)));
        }

        /** @brief Whether actual system is Big-Endian */
        constexpr static bool isBigEndian() {
            #ifdef CORRADE_BIG_ENDIAN
            return true;
            #else
            return false;
            #endif
        }

        /**
         * @brief Convert number from or to Big-Endian
         *
         * On Little-Endian systems calls @ref swap(), on Big-Endian systems
         * returns unchanged value.
         */
        template<class T> static T bigEndian(T value) {
            #ifdef CORRADE_BIG_ENDIAN
            return number;
            #else
            return swap(value);
            #endif
        }

        /**
         * @brief Convert values from or to Big-Endian in-place
         *
         * Calls @ref bigEndian() for each value and saves the result back.
         */
        #if defined(DOXYGEN_GENERATING_OUTPUT) || !defined(CORRADE_BIG_ENDIAN)
        template<class ...T> static void bigEndianInPlace(T&... values) {
            bigEndianInPlaceInternal(values...);
        }
        #else
        template<class ...T> static void bigEndianInPlace(T&...) {}
        #endif

        /**
         * @brief Convert value from or to Little-Endian
         *
         * On Big-Endian systems calls @ref swap(), on Little-Endian systems
         * returns unchanged value.
         */
        template<class T> static T littleEndian(T number) {
            #ifdef CORRADE_BIG_ENDIAN
            return swap(number);
            #else
            return number;
            #endif
        }

        /**
         * @brief Convert values from or to Little-Endian in-place
         *
         * Calls @ref littleEndian() for each value and saves the result back.
         */
        #if defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_BIG_ENDIAN)
        template<class ...T> static void littleEndianInPlace(T&... values) {
            littleEndianInPlaceInternal(values...);
        }
        #else
        template<class ...T> static void littleEndianInPlace(T&...) {}
        #endif

    private:
        template<std::size_t size> struct TypeFor {};

        template<std::size_t size> static typename TypeFor<size>::Type swap(typename TypeFor<size>::Type value);

        #ifndef CORRADE_BIG_ENDIAN
        template<class T, class ...U> static void bigEndianInPlaceInternal(T& first, U&... next) {
            first = bigEndian(first);
            bigEndianInPlaceInternal(next...);
        }
        static void bigEndianInPlaceInternal() {}
        #else
        template<class T, class ...U> static void littleEndianInPlaceInternal(T& first, U&... next) {
            first = littleEndian(first);
            littleEndianInPlaceInternal(next...);
        }
        static void littleEndianInPlaceInternal() {}
        #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct Endianness::TypeFor<1> { typedef std::uint8_t  Type; };
template<> struct Endianness::TypeFor<2> { typedef std::uint16_t Type; };
template<> struct Endianness::TypeFor<4> { typedef std::uint32_t Type; };
template<> struct Endianness::TypeFor<8> { typedef std::uint64_t Type; };

template<> inline std::uint8_t Endianness::swap<1>(std::uint8_t value) {
    return value;
}
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
