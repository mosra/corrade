#ifndef Corrade_Utility_AbstractHash_h
#define Corrade_Utility_AbstractHash_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Utility::AbstractHash
 */

#include <string>

#include "Corrade/Utility/Debug.h"

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
        static HashDigest<size> fromHexString(std::string digest);

        /**
         * @brief Digest from given byte array
         *
         * Assumes that the array has the right length.
         */
        static const HashDigest<size>& fromByteArray(const char* digest) {
            return *reinterpret_cast<const HashDigest<size>*>(digest);
        }

        /**
         * @brief Default constructor
         *
         * Creates zero digest.
         */
        constexpr /*implicit*/ HashDigest(): _digest() {}

        /**
         * @brief Construct digest from byte sequence
         *
         * Value count must be the same as `size`.
         */
        template<class T, class ...U> constexpr explicit HashDigest(T firstValue, U... nextValues): _digest{char(firstValue), char(nextValues)...} {
            static_assert(sizeof...(nextValues) + 1 == size, "Utility::HashDigest::HashDigest(): wrong data size");
        }

        /** @brief Equality operator */
        bool operator==(const HashDigest<size>& other) const {
            for(int i = 0; i != size; ++i)
                if(other._digest[i] != _digest[i]) return false;
            return true;
        }

        /** @brief Non-equality operator */
        bool operator!=(const HashDigest<size>& other) const {
            return !operator==(other);
        }

        /**
         * @brief Convert the digest to lowercase hexadecimal string representation
         */
        std::string hexString() const;

        /** @brief Raw digest byte array */
        constexpr const char* byteArray() const { return _digest; }

    private:
        char _digest[size];
};

/**
@brief Base template for hashing classes

@see @ref HashDigest
*/
template<std::size_t digestSize> class AbstractHash {
    public:
        /** @brief Hash digest */
        typedef HashDigest<digestSize> Digest;

        enum: std::size_t {
            /**
             * Size of the raw digest in bytes. Hexadecimal string
             * representation has double the size.
             */
            DigestSize = digestSize
        };
};

/** @debugoperator{HashDigest} */
template<std::size_t size> inline Debug& operator<<(Debug& debug, const HashDigest<size>& value) {
    return debug << value.hexString().data();
}

template<std::size_t size> HashDigest<size> HashDigest<size>::fromHexString(std::string digest) {
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

template<std::size_t size> std::string HashDigest<size>::hexString() const {
    std::string d(size*2, '0');
    for(int i = 0; i != size*2; ++i) {
        d[i] = (_digest[i/2] >> (i%2 == 0 ? 4 : 0)) & 0xF;
        d[i] += d[i] >= 0xa ? 'a'-0xa : '0';
    }
    return d;
}

}}

#endif
