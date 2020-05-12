#ifndef Corrade_Utility_MurmurHash2_h
#define Corrade_Utility_MurmurHash2_h
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
 * @brief Class @ref Corrade::Utility::MurmurHash2
 */

#include <cstddef>
#include <string>

#include "Corrade/Utility/AbstractHash.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

namespace Implementation {
    template<std::size_t> struct MurmurHash2;
    template<> struct CORRADE_UTILITY_EXPORT MurmurHash2<4> {
        unsigned int operator()(unsigned int seed, const char* data, unsigned int size) const;
    };
    template<> struct CORRADE_UTILITY_EXPORT MurmurHash2<8> {
        unsigned long long operator()(unsigned long long seed, const char* data, unsigned long long size) const;
    };
}

/**
@brief MurmurHash 2

Based on algorithm copyright Austin Appleby, http://code.google.com/p/smhasher/ .
The digest is 32bit or 64bit, depending on @cpp sizeof(std::size_t) @ce and
thus usable for hashing in e.g. @ref std::unordered_map.

@todo constexpr algorithm
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
            return MurmurHash2()(data);
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

        /** @copydoc operator()(const std::string&) const */
        template<std::size_t size> Digest operator()(const char(&data)[size]) const {
            std::size_t d = Implementation::MurmurHash2<sizeof(std::size_t)>{}(_seed, data, size-1);
            return Digest::fromByteArray(reinterpret_cast<const char*>(&d));
        }

        /** @copydoc operator()(const std::string&) const */
        Digest operator()(const char* data, std::size_t size) const {
            std::size_t d = Implementation::MurmurHash2<sizeof(std::size_t)>{}(_seed, data, size);
            return Digest::fromByteArray(reinterpret_cast<const char*>(&d));
        }

    private:
        std::size_t _seed;
};

}}

#endif
