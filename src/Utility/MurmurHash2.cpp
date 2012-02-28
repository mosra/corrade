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

#include "MurmurHash2.h"

namespace Corrade { namespace Utility {

/* The algorithm is copyright Austin Appleby */

unsigned int MurmurHash2Implementation<4>::operator()(const unsigned char* data, unsigned int size) const {
    /* m and r are mixing constants generated offline. They're not really
       magic, they just happen to work well. */
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a random value */
    unsigned int h = seed^size;

    /* Mix 4 bytes at a time into the hash */
    for(size_t i = 0; i+4 <= size; i += 4) {
        unsigned int k = *reinterpret_cast<const unsigned int*>(data+i);

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

unsigned long long MurmurHash2Implementation<8>::operator()(const unsigned char* data, unsigned long long size) const {
    /* m and r are mixing constants generated offline. They're not really
       magic, they just happen to work well. */
    const unsigned long long m = 0xc6a4a7935bd1e995ull;
    const int r = 47;

    /* Initialize the hash to a random value */
    unsigned long long h = seed^(size*m);

    /* Mix 8 bytes at a time into the hash */
    for(size_t i = 0; i+8 <= size; i += 8) {
        unsigned long long k = *reinterpret_cast<const unsigned long long*>(data+i);

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

}}
