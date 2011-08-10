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

#include "Sha1.h"

#include "Utility/Endianness.h"

using namespace std;

namespace Kompas { namespace Utility {

const unsigned int Sha1::initialDigest[5] = { 0x67452301,
                                              0xEFCDAB89,
                                              0x98BADCFE,
                                              0x10325476,
                                              0xC3D2E1F0 };

const unsigned int Sha1::constants[4] = { 0x5A827999,
                                          0x6ED9EBA1,
                                          0x8F1BBCDC,
                                          0xCA62C1D6 };

Sha1& Sha1::operator<<(const string& data) {
    /* Process leftovers */
    if(_buffer.size() != 0) {
        /* Not enough large, try it next time */
        if(data.size()+ _buffer.size() < 64) {
            _buffer.append(data);
            return *this;
        }

        _buffer.append(data.substr(0, 64- _buffer.size()));
        processChunk(_buffer.c_str());
    }

    for(size_t i = _buffer.size(); i != data.size()/64; ++i)
        processChunk(data.c_str()+i*64);

    /* Save last unfinished 512-bit chunk of data */
    if(data.size()%64 != 0) _buffer = data.substr((data.size()/64)*64);
    else _buffer = string();

    _dataSize += data.size();
    return *this;
}

Sha1::Digest Sha1::digest() {
    /* Add '1' bit to the leftovers, pad to (n*64)+56 bytes */
    _buffer.append(1, 0x80);
    _buffer.append((_buffer.size() > 56 ? 120 : 56)- _buffer.size(), 0);

    /* Add size of data in bits in big endian */
    unsigned long long dataSizeBigEndian = Endianness::bigEndian<unsigned long long>(_dataSize*8);
    _buffer.append(reinterpret_cast<const char*>(&dataSizeBigEndian), 8);

    /* Process remaining chunks */
    for(size_t i = 0; i != _buffer.size()/64; ++i)
        processChunk(_buffer.c_str()+i*64);

    /* Convert digest from big endian */
    unsigned int digest[5];
    for(int i = 0; i != 5; ++i)
        digest[i] = Endianness::bigEndian<unsigned int>(_digest[i]);
    Digest d = Digest::fromByteArray(reinterpret_cast<const char*>(digest));

    /* Clear data and return */
    memcpy(_digest, initialDigest, DigestSize);
    _buffer.clear();
    _dataSize = 0;
    return d;
}

void Sha1::processChunk(const char* data) {
    /* Extend the data to 80 bytes, make it big endian */
    unsigned int extended[80];
    for(int i = 0; i != 16; ++i)
        extended[i] = Endianness::bigEndian<unsigned int>(*reinterpret_cast<const unsigned int*>(data+i*4));
    for(int i = 16; i != 80; ++i)
        extended[i] = leftrotate((extended[i-3] ^ extended[i-8] ^ extended[i-14] ^ extended[i-16]), 1);

    /* Initialize value for this chunk */
    unsigned int d[5];
    unsigned int f, constant, temp;
    memcpy(d, _digest, DigestSize);

    /* Main loop */
    for(int i = 0; i != 80; ++i) {
        if(i < 20) {
            f = d[3] ^ (d[1] & (d[2] ^ d[3]));
            constant = constants[0];
        } else if(i < 40) {
            f = d[1] ^ d[2] ^ d[3];
            constant = constants[1];
        } else if(i < 60) {
            f = (d[1] & d[2]) | (d[3] & (d[1] | d[2]));
            constant = constants[2];
        } else {
            f = d[1] ^ d[2] ^ d[3];
            constant = constants[3];
        }

        temp =
            leftrotate(d[0], 5) + f + d[4] + constant + extended[i];
        d[4] = d[3];
        d[3] = d[2];
        d[2] = leftrotate(d[1], 30);
        d[1] = d[0];
        d[0] = temp;
    }

    /* Add the values to digest */
    for(int i = 0; i != 5; ++i)
        _digest[i] += d[i];
}

}}
