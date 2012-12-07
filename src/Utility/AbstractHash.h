#ifndef Corrade_Utility_AbstractHash_h
#define Corrade_Utility_AbstractHash_h
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
 * @brief Class Corrade::Utility::AbstractHash
 */

#include <string>

#include "Debug.h"

namespace Corrade { namespace Utility {

/** @brief Hash digest */
template<std::size_t size> class HashDigest {
    public:
        /**
         * @brief Create digest from given string representation
         *
         * If the digest has invalid length or contains invalid
         * characters (other than `0-9, a-f, A-F`), returns zero
         * digest.
         */
        static HashDigest<size> fromHexString(std::string digest) {
            HashDigest<size> d;
            if(digest.size() != size*2) return d;

            for(int i = 0; i != size*2; ++i) {
                if(digest[i] >= '0' && digest[i] <= '9')
                    digest[i] -= '0';
                else if(digest[i] >= 'a' && digest[i] <= 'f')
                    digest[i] -= 'a'-0xa;
                else if(digest[i] >= 'A' && digest[i] <= 'F')
                    digest[i] -= 'A'-0xa;
                else return HashDigest<size>();

                d._digest[i/2] |= (digest[i]) << (i%2 == 0 ? 4 : 0);
            }
            return d;
        }

        /**
         * @brief Digest from given byte array
         *
         * Assumes that the array has the right length.
         */
        inline static const HashDigest<size>& fromByteArray(const char* digest) {
            return *reinterpret_cast<const HashDigest<size>*>(digest);
        }

        /**
         * @brief Default constructor
         *
         * Creates zero digest.
         */
        inline constexpr HashDigest(): _digest() {}

        /** @brief Equality operator */
        bool operator==(const HashDigest<size>& other) const {
            for(int i = 0; i != size; ++i)
                if(other._digest[i] != _digest[i]) return false;
            return true;
        }

        /** @brief Non-equality operator */
        inline bool operator!=(const HashDigest<size>& other) const {
            return !operator==(other);
        }

        /**
         * @brief Convert the digest to lowercase hexadecimal string representation
         */
        std::string hexString() const {
            std::string d(size*2, '0');
            for(int i = 0; i != size*2; ++i) {
                d[i] = (_digest[i/2] >> (i%2 == 0 ? 4 : 0)) & 0xF;
                d[i] += d[i] >= 0xa ? 'a'-0xa : '0';
            }
            return d;
        }

        /** @brief Raw digest byte array */
        inline constexpr const char* byteArray() const { return _digest; }

    private:
        char _digest[size];
};

/**
@brief Base template for hashing classes

@see HashDigest
*/
template<std::size_t digestSize> class AbstractHash {
    public:
        /** @brief Hash digest */
        typedef HashDigest<digestSize> Digest;

        /**
         * @brief %Digest size
         *
         * Size of the raw digest in bytes. Hexadecimal string representation
         * has double size.
         */
        static const std::size_t DigestSize = digestSize;
};

/** @debugoperator{Corrade::Utility::HashDigest} */
template<std::size_t size> Debug operator<<(Debug debug, const HashDigest<size>& value) {
    return debug << value.hexString();
}

}}

#endif
