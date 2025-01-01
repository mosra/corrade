/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

#include "BitAlgorithms.h"

#include <cstring>

namespace Corrade { namespace Utility {

void copyMasked(const Containers::StridedArrayView2D<const char>& src, const Containers::BitArrayView srcMask, const Containers::StridedArrayView2D<char>& dst) {
    const std::size_t srcSize = src.size()[0];
    CORRADE_ASSERT(srcSize == srcMask.size(),
        "Utility::copyMasked(): expected source mask size to be" << srcSize << "but got" << srcMask.size(), );
    #ifndef CORRADE_NO_ASSERT
    const std::size_t srcMaskCount = srcMask.count();
    #endif
    CORRADE_ASSERT(srcMaskCount == dst.size()[0],
        "Utility::copyMasked(): expected" << srcMaskCount << "destination items but got" << dst.size()[0], );
    const std::size_t srcTypeSize = src.size()[1];
    #ifndef CORRADE_NO_ASSERT
    const std::size_t dstTypeSize = dst.size()[1];
    #endif
    CORRADE_ASSERT(srcTypeSize == dstTypeSize,
        "Utility::copyMasked(): expected second destination dimension size to be" << srcTypeSize << "but got" << dstTypeSize, );
    CORRADE_ASSERT(src.isContiguous<1>(),
        "Utility::copyMasked(): second source view dimension is not contiguous", );
    CORRADE_ASSERT(dst.isContiguous<1>(),
        "Utility::copyMasked(): second destination view dimension is not contiguous", );

    const std::ptrdiff_t srcStride = src.stride()[0];
    const std::ptrdiff_t dstStride = dst.stride()[0];
    const char* srcPtr = static_cast<const char*>(src.data());
    char* dstPtr = static_cast<char*>(dst.data());
    /** @todo instead of iterating over all src items it could iterate over all
        dst, and then at the end assert that there's no bits left (or that
        there's still bit positions), which could avoid the otherwise
        unnecessary popcount */
    for(std::size_t i = 0; i != srcSize; ++i) {
        /** @todo some better way to iterate set bits */
        if(srcMask[i]) {
            std::memcpy(dstPtr, srcPtr, srcTypeSize);
            dstPtr += dstStride;
        }

        srcPtr += srcStride;
    }
}

}}
