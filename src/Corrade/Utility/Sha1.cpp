/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2019 Jonathan Hale <squareys@googlemail.com>

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
#include <string>

#include "Corrade/Containers/ArrayView.h"
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

Sha1::Sha1(): _digest{InitialDigest[0], InitialDigest[1], InitialDigest[2], InitialDigest[3], InitialDigest[4]} {}

Sha1& Sha1::operator<<(Containers::ArrayView<const char> data) {
    const std::size_t dataOffset = _bufferSize ? 64 - _bufferSize : 0;

    /* Process leftovers */
    if(_bufferSize != 0) {
        /* Not large enough, try it next time */
        if(data.size() + _bufferSize < 64) {
            std::memcpy(_buffer + _bufferSize, data.data(), data.size());
            _bufferSize += data.size();
            _dataSize += data.size();
            return *this;
        }

        /* Append few last bytes to have the buffer at 64 bytes */
        std::memcpy(_buffer + _bufferSize, data.data(), dataOffset);
        _bufferSize += dataOffset;
        processChunk(_buffer);
    }

    for(std::size_t i = dataOffset; i + 64 <= data.size(); i += 64)
        processChunk(data.data() + i);

    /* Save last unfinished 512-bit chunk of data */
    auto leftOver = data.suffix(dataOffset + ((data.size() - dataOffset)/64)*64);
    std::memcpy(_buffer, leftOver.data(), leftOver.size());
    _bufferSize = leftOver.size();
    _dataSize += data.size();
    return *this;
}

Sha1& Sha1::operator<<(const std::string& data) {
    return *this << Containers::arrayView(data.data(), data.size());
}

/* GCC 6 (and possibly 7) on Raspberry Pi 3 Model B+ (aarch64) misoptimizes the
   Sha1 calculation (e.g. giving 7073c2761c38837eb837837ef037837eafd80709 for
   an empty input instead of correct da39a3ee5e6b4b0d3255bfef95601890afd80709)
   when -O3 is used. Does not happen on GCC 8, does not happen with Clang. Due
   to not having a better access to the device, I'm forcing O2 on all related
   code. However note that this also forces it in case O0 was used. The
   actual line affected seems to be  Digest d = Digest::fromByteArray(...). A
   report with further details: https://github.com/mosra/corrade/issues/45 */
#if defined(__GNUC__) && !defined(__clang__) && defined(CORRADE_TARGET_ARM) && __GNUC__ < 8
#pragma GCC push_options
#pragma GCC optimize ("O2")
#endif
Sha1::Digest Sha1::digest() {
    /* Add '1' bit to the leftovers */
    _buffer[_bufferSize++] = '\x80';

    /* Pad to (n*64)+56 bytes */
    const size_t padding = (_bufferSize > 56 ? 120 : 56) - _bufferSize;
    CORRADE_INTERNAL_ASSERT(_bufferSize + padding + 8 <= sizeof(_buffer));
    std::memset(_buffer + _bufferSize, 0, padding);
    _bufferSize += padding;

    /* Add size of data in bits in big endian */
    unsigned long long dataSizeBigEndian = Endianness::bigEndian<unsigned long long>(_dataSize*8);
    std::memcpy(_buffer + _bufferSize, reinterpret_cast<const char*>(&dataSizeBigEndian), 8);
    _bufferSize += 8;

    /* Process remaining chunks */
    for(std::size_t i = 0; i != _bufferSize/64; ++i) {
        processChunk(_buffer+i*64);
    }

    /* Convert digest from big endian */
    unsigned int digest[5];
    for(int i = 0; i != 5; ++i)
        digest[i] = Endianness::bigEndian<unsigned int>(_digest[i]);
    const Digest d = Digest::fromByteArray(reinterpret_cast<const char*>(digest));

    /* Clear data and return */
    std::copy(InitialDigest, InitialDigest+5, _digest);
    _dataSize = 0;
    _bufferSize = 0;
    return d;
}
#if defined(__GNUC__) && !defined(__clang__) && defined(CORRADE_TARGET_ARM) && __GNUC__ < 8
#pragma GCC pop_options
#endif

void Sha1::processChunk(const char* data) {
    /* Extend the data to 80 bytes, make it big endian */
    unsigned int extended[80];
    /* Some memory juggling to avoid unaligned reads on platforms that don't
       like it (Emscripten). The data don't have any endianness, so take the
       first byte first, as usual. */
    for(int i = 0; i != 16; ++i)
        extended[i] =
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 0])) << 24) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 1])) << 16) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 2])) <<  8) |
            (static_cast<unsigned int>(static_cast<unsigned char>(data[i*4 + 3])) <<  0);
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
