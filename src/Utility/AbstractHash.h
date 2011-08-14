#ifndef Kompas_Utility_AbstractHash_h
#define Kompas_Utility_AbstractHash_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class Kompas::Utility::AbstractHash
 */

#include <string>
#include <cstring>

namespace Kompas { namespace Utility {

/**
@brief Base template for hashing classes

Provides Digest class for storing and converting digests.
 */
template<size_t digestSize> class AbstractHash {
    public:
        /** @brief Hash digest */
        class Digest {
            public:
                /**
                 * @brief Create digest from given string representation
                 *
                 * If the digest has invalid length or contains invalid
                 * characters (other than <tt>0-9, a-f, A-F</tt>), returns
                 * zero digest.
                 */
                static Digest fromHexString(std::string digest) {
                    Digest d;
                    if(digest.size() != digestSize*2) return d;

                    for(int i = 0; i != digestSize*2; ++i) {
                        if(digest[i] >= '0' && digest[i] <= '9')
                            digest[i] -= '0';
                        else if(digest[i] >= 'a' && digest[i] <= 'f')
                            digest[i] -= 'a'-0xa;
                        else if(digest[i] >= 'A' && digest[i] <= 'F')
                            digest[i] -= 'A'-0xa;
                        else return Digest();

                        d._digest[i/2] |= (digest[i]) << (i%2 == 0 ? 4 : 0);
                    }
                    return d;
                }

                /**
                 * @brief Create digest from given byte array
                 *
                 * Assumes that the array has the right length.
                 */
                inline static Digest fromByteArray(const char* digest) {
                    return Digest(digest);
                }

                /**
                 * @brief Default constructor
                 *
                 * Creates zero digest.
                 */
                inline Digest() {
                    memset(_digest, 0, digestSize);
                }

                /** @brief Copy constructor */
                inline Digest(const Digest& other) {
                    memcpy(_digest, other._digest, digestSize);
                }

                /** @brief Equality operator */
                bool operator==(const Digest& other) const {
                    for(int i = 0; i != digestSize; ++i)
                        if(other._digest[i] != _digest[i]) return false;
                    return true;
                }

                /** @brief Non-equality operator */
                inline bool operator!=(const Digest& other) const {
                    return !operator==(other);
                }

                /** @brief Assignment operator */
                Digest& operator=(const Digest& other) {
                    memcpy(_digest, other._digest, digestSize);
                    return *this;
                }

                /**
                 * @brief Convert the digest to lowercase hexadecimal string representation
                 */
                std::string hexString() const {
                    std::string d(digestSize*2, '0');
                    for(int i = 0; i != digestSize*2; ++i) {
                        d[i] = (_digest[i/2] >> (i%2 == 0 ? 4 : 0)) & 0xF;
                        d[i] += d[i] >= 0xa ? 'a'-0xa : '0';
                    }
                    return d;
                }

                /**
                 * @brief Raw digest byte array
                 *
                 * @attention Don't attempt to modify this array. This array
                 * will be deleted on class destruction.
                 */
                inline const char* byteArray() const { return _digest; }

            private:
                inline Digest(const char* digest) {
                    memcpy(_digest, digest, digestSize);
                }
                char _digest[digestSize];
        };

        /**
         * @brief Digest size
         *
         * Physical size of the raw digest. Hexadecimal string representation
         * has double size.
         */
        static const size_t DigestSize = digestSize;
};

}}

#endif
