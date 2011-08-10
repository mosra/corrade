#ifndef Kompas_Utility_Sha1_h
#define Kompas_Utility_Sha1_h
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

#include "AbstractHash.h"

namespace Kompas { namespace Utility {

/** @brief SHA-1 */
class Sha1: public AbstractHash<20> {
    public:
        inline static Digest digest(const std::string& data) {
            return (Sha1() << data).digest();
        }

        inline Sha1(): _dataSize(0) {
            memcpy(_digest, initialDigest, DigestSize);
        }

        Sha1& operator<<(const std::string& data);
        Digest digest();

    private:
        void processChunk(const char* data);

        inline unsigned int leftrotate(unsigned int data, unsigned int shift) {
            return data << shift | data >> (32-shift);
        }

        static const unsigned int initialDigest[5];
        static const unsigned int constants[4];

        std::string _buffer;
        unsigned long long _dataSize;
        unsigned int _digest[5];
};

}}

#endif
