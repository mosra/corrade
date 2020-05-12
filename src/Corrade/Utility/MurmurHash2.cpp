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

#include "MurmurHash2.h"

namespace Corrade { namespace Utility { namespace Implementation {

unsigned int MurmurHash2<4>::operator()(const unsigned int seed, const char* const signedData, unsigned int size) const {
    const unsigned char* const data = reinterpret_cast<const unsigned char*>(signedData);

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
            data[i + 3] << 24 |
            data[i + 2] << 16 |
            data[i + 1] <<  8 |
            data[i + 0];

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;
    }

    /* Handle the last few bytes of the input array */
    if(size & 0x03) {
        while(size-- & 0x03)
            h ^= data[size] << (8 * (size & 0x03));
        h *= m;
    }

    /* Do a few final mixes of the hash to ensure the last few bytes are
       well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

unsigned long long MurmurHash2<8>::operator()(const unsigned long long seed, const char* const signedData, unsigned long long size) const {
    const unsigned char* const data = reinterpret_cast<const unsigned char*>(signedData);

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

}}}
