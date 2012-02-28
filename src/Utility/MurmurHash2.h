#ifndef Corrade_Utility_MurmurHash2_h
#define Corrade_Utility_MurmurHash2_h
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

#include "AbstractHash.h"
#include "utilities.h"

namespace Corrade { namespace Utility {

/**
@brief MurmurHash implementation

Specialized implementation for 32bit and 64bit size_t.
*/
template<size_t> class MurmurHash2Implementation {
    #ifdef DOXYGEN_GENERATING_OUTPUT
    public:
        /**
         * @brief Constructor
         * @param seed      Seed to initialize the hash
         */
        MurmurHash2Implementation(size_t seed);

        /** @brief Compute digest of given data */
        size_t operator()(const char* data, size_t size);
    #endif
};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> class MurmurHash2Implementation<4> {
    public:
        inline MurmurHash2Implementation(unsigned int seed): seed(seed) {}
        unsigned int operator()(const unsigned char* data, unsigned int size) const;

    private:
        unsigned int seed;
};
template<> class MurmurHash2Implementation<8> {
    public:
        inline MurmurHash2Implementation(unsigned long long seed): seed(seed) {}
        unsigned long long operator()(const unsigned char* data, unsigned long long size) const;

    private:
        unsigned long long seed;
};
#endif

/**
@brief MurmurHash 2

Based on algorithm copyright Austin Appleby, http://code.google.com/p/smhasher/ .
The digest is 32bit or 64bit, depending on <tt>sizeof(size_t)</tt> and thus
usable for hasing in e.g. <tt>std::unordered_map</tt>.
*/
class UTILITY_EXPORT MurmurHash2: public AbstractHash<sizeof(size_t)> {
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
        inline MurmurHash2(size_t seed = 0): implementation(seed) {}

        /** @brief Compute digest of given data */
        Digest operator()(const std::string& data) const {
            return operator()(data.c_str(), data.size());
        }

        /** @copydoc operator()(const std::string&) const */
        Digest operator()(const char* data, size_t size) const {
            size_t d = implementation(reinterpret_cast<const unsigned char*>(data), size);
            return Digest::fromByteArray(reinterpret_cast<const char*>(&d));
        }

    private:
        MurmurHash2Implementation<sizeof(size_t)> implementation;
};

}}

#endif
