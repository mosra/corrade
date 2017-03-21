/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Sha1.h"

#include <cstddef>

#include "Corrade/Utility/Endianness.h"

namespace Corrade { namespace Utility {

namespace {

constexpr const unsigned int InitialDigest[5] = { 0x67452301,
                                                  0xEFCDAB89,
                                                  0x98BADCFE,
                                                  0x10325476,
                                                  0xC3D2E1F0 };

constexpr const unsigned int Constants[4] = { 0x5A827999,
                                              0x6ED9EBA1,
                                              0x8F1BBCDC,
                                              0xCA62C1D6 };

unsigned int leftrotate(unsigned int data, unsigned int shift) {
    return data << shift | data >> (32 - shift);
}

}

Sha1::Sha1(): _dataSize(0), _digest{InitialDigest[0], InitialDigest[1], InitialDigest[2], InitialDigest[3], InitialDigest[4]} {}

Sha1& Sha1::operator<<(const std::string& data) {
    const std::size_t dataOffset = _buffer.empty() ? 0 : 64 - _buffer.size();

    /* Process leftovers */
    if(!_buffer.empty()) {
        /* Not large enough, try it next time */
        if(data.size() + _buffer.size() < 64) {
            _buffer.append(data);
            _dataSize += data.size();
            return *this;
        }

        /* Append few last bytes to have the buffer at 64 bytes */
        _buffer.append(data.substr(0, dataOffset));
        processChunk(_buffer.data());
    }

    for(std::size_t i = dataOffset; i + 64 <= data.size(); i += 64)
        processChunk(data.data() + i);

    /* Save last unfinished 512-bit chunk of data */
    _buffer = data.substr(dataOffset + ((data.size() - dataOffset)/64)*64);

    _dataSize += data.size();
    return *this;
}

Sha1::Digest Sha1::digest() {
    /* Add '1' bit to the leftovers, pad to (n*64)+56 bytes */
    _buffer.append(1, '\x80');
    _buffer.append((_buffer.size() > 56 ? 120 : 56) - _buffer.size(), 0);

    /* Add size of data in bits in big endian */
    unsigned long long dataSizeBigEndian = Endianness::bigEndian<unsigned long long>(_dataSize*8);
    _buffer.append(reinterpret_cast<const char*>(&dataSizeBigEndian), 8);

    /* Process remaining chunks */
    for(std::size_t i = 0; i != _buffer.size()/64; ++i)
        processChunk(_buffer.data()+i*64);

    /* Convert digest from big endian */
    unsigned int digest[5];
    for(int i = 0; i != 5; ++i)
        digest[i] = Endianness::bigEndian<unsigned int>(_digest[i]);
    Digest d = Digest::fromByteArray(reinterpret_cast<const char*>(digest));

    /* Clear data and return */
    std::copy(InitialDigest, InitialDigest+5, _digest);
    _buffer.clear();
    _dataSize = 0;
    return d;
}

void Sha1::processChunk(const char* data) {
    /* Extend the data to 80 bytes, make it big endian */
    unsigned int extended[80];
    /* Some memory juggling to avoid unaligned reads on platforms that don't
       like it (Emscripten) */
    for(int i = 0; i != 16; ++i)
        extended[i] = Endianness::bigEndian<unsigned int>(
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 3])) << 24) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 2])) << 16) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 1])) <<  8) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 0])) <<  0));
    for(int i = 16; i != 80; ++i)
        extended[i] = leftrotate((extended[i-3] ^ extended[i-8] ^ extended[i-14] ^ extended[i-16]), 1);

    /* Initialize value for this chunk */
    unsigned int d[5];
    unsigned int f, constant, temp;
    std::copy(_digest, _digest+5, d);

    /* Main loop */
    for(int i = 0; i != 80; ++i) {
        if(i < 20) {
            f = d[3] ^ (d[1] & (d[2] ^ d[3]));
            constant = Constants[0];
        } else if(i < 40) {
            f = d[1] ^ d[2] ^ d[3];
            constant = Constants[1];
        } else if(i < 60) {
            f = (d[1] & d[2]) | (d[3] & (d[1] | d[2]));
            constant = Constants[2];
        } else {
            f = d[1] ^ d[2] ^ d[3];
            constant = Constants[3];
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
