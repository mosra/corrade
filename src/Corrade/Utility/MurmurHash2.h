#ifndef Corrade_Utility_MurmurHash2_h
#define Corrade_Utility_MurmurHash2_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
              Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Utility::MurmurHash2
 */

#include "Corrade/Utility/AbstractHash.h"

namespace Corrade { namespace Utility {

namespace Implementation {
    template<std::size_t> struct MurmurHash2;
    template<> struct CORRADE_UTILITY_EXPORT MurmurHash2<4> {
        constexpr unsigned int operator()(unsigned int seed, const char* data, unsigned int size) const;
    };
    template<> struct CORRADE_UTILITY_EXPORT MurmurHash2<8> {
        constexpr unsigned long long operator()(unsigned long long seed, const char* data, unsigned long long size) const;
    };
}

/**
@brief MurmurHash 2

Based on algorithm copyright Austin Appleby, http://code.google.com/p/smhasher/ .
The digest is 32bit or 64bit, depending on `sizeof(std::size_t)` and thus
usable for hasing in e.g. `std::unordered_map`.
*/
class CORRADE_UTILITY_EXPORT MurmurHash2: public AbstractHash<sizeof(std::size_t)> {
    public:
        /**
         * @brief Digest of given data
         *
         * Computes digest using default zero seed. This function is here for
         * consistency with other @ref AbstractHash subclasses.
         */
        static Digest digest(const std::string& data) {
            return Digest{Implementation::MurmurHash2<sizeof(std::size_t)>{}(0, data.data(), data.size())};
        }

        /** @overload */
        template<std::size_t size> constexpr static Digest digest(const char(&data)[size]) {
            return Digest{Implementation::MurmurHash2<sizeof(std::size_t)>{}(0, data, size - 1)};
        }

        /**
         * @brief Constructor
         * @param seed      Seed to initialize the hash
         */
        constexpr explicit MurmurHash2(std::size_t seed = 0): _seed(seed) {}

        /** @brief Compute digest of given data */
        Digest operator()(const std::string& data) const {
            return operator()(data.data(), data.size());
        }

        /** @overload */
        template<std::size_t size> constexpr Digest operator()(const char(&data)[size]) const {
            return Digest{Implementation::MurmurHash2<sizeof(std::size_t)>{}(_seed, data, size - 1)};
        }

        /** @overload */
        Digest operator()(const char* data, std::size_t size) const {
            return Digest{Implementation::MurmurHash2<sizeof(std::size_t)>{}(_seed, data, size)};
        }

    private:
        std::size_t _seed;
};

namespace Implementation {

constexpr unsigned int MurmurHash2<4>::operator()(const unsigned int seed, const char* const data, unsigned int size) const {
    /* m and r are mixing constants generated offline. They're not really
       magic, they just happen to work well. */
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a random value */
    unsigned int h = seed^size;

    /* Mix 4 bytes at a time into the hash */
    for(std::size_t i = 0; i+4 <= size; i += 4) {
        /* Can't use *reinterpret_cast<const unsigned int*>(data+i), as it is
           unaligned read (not supported on ARM or in Emscripten) */
        unsigned int k =
            static_cast<unsigned int>(data[i + 3]) << 24 |
            static_cast<unsigned int>(data[i + 2]) << 16 |
            static_cast<unsigned int>(data[i + 1]) <<  8 |
            static_cast<unsigned int>(data[i + 0]);

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;
    }

    /* Handle the last few bytes of the input array */
    if(size & 0x03) {
        while(size-- & 0x03)
            h ^= static_cast<unsigned int>(data[size]) << (8 * (size & 0x03));
        h *= m;
    }

    /* Do a few final mixes of the hash to ensure the last few bytes are
       well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

constexpr unsigned long long MurmurHash2<8>::operator()(const unsigned long long seed, const char* const data, unsigned long long size) const {
    /* m and r are mixing constants generated offline. They're not really
       magic, they just happen to work well. */
    const unsigned long long m = 0xc6a4a7935bd1e995ull;
    const int r = 47;

    /* Initialize the hash to a random value */
    unsigned long long h = seed^(size*m);

    /* Mix 8 bytes at a time into the hash */
    for(std::size_t i = 0; i+8 <= size; i += 8) {
        /* Can't use *reinterpret_cast<const unsigned int*>(data+i), as it is
           unaligned read (not supported on ARM or in Emscripten) */
        unsigned long long k =
            static_cast<unsigned long long>(data[i + 7]) << 56 |
            static_cast<unsigned long long>(data[i + 6]) << 48 |
            static_cast<unsigned long long>(data[i + 5]) << 40 |
            static_cast<unsigned long long>(data[i + 4]) << 32 |
            static_cast<unsigned long long>(data[i + 3]) << 24 |
            static_cast<unsigned long long>(data[i + 2]) << 16 |
            static_cast<unsigned long long>(data[i + 1]) <<  8 |
            static_cast<unsigned long long>(data[i + 0]);

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    /* Handle the last few bytes of the input array */
    if(size & 0x07) {
        while(size-- & 0x07)
            h ^= static_cast<unsigned long long>(data[size]) << (8 * (size & 0x07));
        h *= m;
    }

    /* Do a few final mixes of the hash to ensure the last few bytes are
       well-incorporated. */
    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

}

}}

#endif
