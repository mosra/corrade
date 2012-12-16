#ifndef Corrade_Utility_Sha1_h
#define Corrade_Utility_Sha1_h
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
 * @brief Class Corrade::Utility::Sha1
 */

#include "AbstractHash.h"
#include "utilities.h"

namespace Corrade { namespace Utility {

/** @brief SHA-1 */
class CORRADE_UTILITY_EXPORT Sha1: public AbstractHash<20> {
    public:
        /**
         * @brief Digest of given data
         *
         * Convenience function for
         * @code
         * (Sha1() << data).digest()
         * @endcode
         */
        inline static Digest digest(const std::string& data) {
            return (Sha1() << data).digest();
        }

        #ifndef DOXYGEN_GENERATING_OUTPUT
        inline explicit Sha1(): _dataSize(0), _digest{initialDigest[0], initialDigest[1], initialDigest[2], initialDigest[3], initialDigest[4]} {}
        #endif

        /** @brief Add data for digesting */
        Sha1& operator<<(const std::string& data);

        /** @brief Digest of all added data */
        Digest digest();

    private:
        CORRADE_UTILITY_LOCAL void processChunk(const char* data);

        inline unsigned int leftrotate(unsigned int data, unsigned int shift) {
            return data << shift | data >> (32-shift);
        }

        CORRADE_UTILITY_EXPORT static const unsigned int initialDigest[5];
        CORRADE_UTILITY_LOCAL static const unsigned int constants[4];

        std::string _buffer;
        unsigned long long _dataSize;
        unsigned int _digest[5];
};

}}

#endif
