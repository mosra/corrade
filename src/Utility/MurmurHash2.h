#ifndef Corrade_Utility_MurmurHash2_h
#define Corrade_Utility_MurmurHash2_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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
 * @brief Class Corrade::Utility::MurmurHash2Implementation, Corrade::Utility::MurmurHash2
 */

#include "AbstractHash.h"

namespace Corrade { namespace Utility {

/**
@brief MurmurHash implementation

Specialized implementation for 32bit and 64bit `std::size_t`.
*/
template<std::size_t> class MurmurHash2Implementation {
    #ifdef DOXYGEN_GENERATING_OUTPUT
    public:
        /**
         * @brief Constructor
         * @param seed      Seed to initialize the hash
         */
        explicit MurmurHash2Implementation(std::size_t seed);

        /** @brief Compute digest of given data */
        std::size_t operator()(const char* data, std::size_t size);
    #endif
};

/** @todo Export implementation symbols only for tests */

#ifndef DOXYGEN_GENERATING_OUTPUT
/** @todo Used only in unit test, export in only there? */
template<> class CORRADE_UTILITY_EXPORT MurmurHash2Implementation<4> {
    public:
        inline explicit MurmurHash2Implementation(unsigned int seed): seed(seed) {}
        unsigned int operator()(const unsigned char* data, unsigned int size) const;

    private:
        unsigned int seed;
};
/** @todo Used only in unit test, export in only there? */
template<> class CORRADE_UTILITY_EXPORT MurmurHash2Implementation<8> {
    public:
        inline explicit MurmurHash2Implementation(unsigned long long seed): seed(seed) {}
        unsigned long long operator()(const unsigned char* data, unsigned long long size) const;

    private:
        unsigned long long seed;
};
#endif

/**
@brief MurmurHash 2

Based on algorithm copyright Austin Appleby, http://code.google.com/p/smhasher/ .
The digest is 32bit or 64bit, depending on `sizeof(std::size_t)` and thus
usable for hasing in e.g. `std::unordered_map`.

@todo constexpr algorithm
*/
class CORRADE_UTILITY_EXPORT MurmurHash2: public AbstractHash<sizeof(std::size_t)> {
    public:
        /**
         * @brief Digest of given data
         *
         * Computes digest using default zero seed. This function is here for
         * consistency with other AbstractHash subclasses.
         */
        inline static Digest digest(const std::string& data) {
            return MurmurHash2()(data);
        }

        /**
         * @brief Constructor
         * @param seed      Seed to initialize the hash
         */
        inline explicit MurmurHash2(std::size_t seed = 0): implementation(seed) {}

        /** @brief Compute digest of given data */
        inline Digest operator()(const std::string& data) const {
            return operator()(data.c_str(), data.size());
        }

        /** @copydoc operator()(const std::string&) const */
        template<std::size_t size> inline Digest operator()(const char(&data)[size]) const {
            std::size_t d = implementation(reinterpret_cast<const unsigned char*>(data), size-1);
            return Digest::fromByteArray(reinterpret_cast<const char*>(&d));
        }

        /** @copydoc operator()(const std::string&) const */
        Digest operator()(const char* data, std::size_t size) const {
            std::size_t d = implementation(reinterpret_cast<const unsigned char*>(data), size);
            return Digest::fromByteArray(reinterpret_cast<const char*>(&d));
        }

    private:
        MurmurHash2Implementation<sizeof(std::size_t)> implementation;
};

}}

#endif
