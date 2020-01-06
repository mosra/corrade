/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Algorithms.h"

#include <cstring>

namespace Corrade { namespace Utility {

void copy(const Containers::ArrayView<const void>& src, const Containers::ArrayView<void>& dst) {
    const std::size_t srcSize = src.size();
    const std::size_t dstSize = dst.size();
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    std::memcpy(dst.data(), src.data(), srcSize);
}

void copy(const Containers::StridedArrayView1D<const char>& src, const Containers::StridedArrayView1D<char>& dst) {
    const std::size_t srcSize = src.size();
    const std::size_t dstSize = dst.size();
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    auto* const srcPtr = static_cast<const char*>(src.data());
    auto* const dstPtr = static_cast<char*>(dst.data());

    if(src.isContiguous() && dst.isContiguous())
        std::memcpy(dstPtr, srcPtr, srcSize);
    else {
        const std::ptrdiff_t srcStride = src.stride();
        const std::ptrdiff_t dstStride = dst.stride();
        for(std::size_t i = 0; i != srcSize; ++i)
            *(dstPtr + i*dstStride) = *(srcPtr + i*srcStride);
    }
}

void copy(const Containers::StridedArrayView2D<const char>& src, const Containers::StridedArrayView2D<char>& dst) {
    const Containers::StridedDimensions<2, std::size_t> srcSize_ = src.size();
    const Containers::StridedDimensions<2, std::size_t> dstSize_ = dst.size();
    CORRADE_ASSERT(srcSize_ == dstSize_,
        "Utility::Algorithms::copy(): sizes" << srcSize_ << "and" << dstSize_ << "don't match", );

    const std::size_t* const size = srcSize_.begin();
    auto* const srcPtr = static_cast<const char*>(src.data());
    auto* const dstPtr = static_cast<char*>(dst.data());

    if(src.isContiguous() && dst.isContiguous())
        std::memcpy(dstPtr, srcPtr, size[0]*size[1]);
    else {
        const Containers::StridedDimensions<2, std::ptrdiff_t> srcStride_ = src.stride();
        const Containers::StridedDimensions<2, std::ptrdiff_t> dstStride_ = dst.stride();
        const std::ptrdiff_t* const srcStride = srcStride_.begin();
        const std::ptrdiff_t* const dstStride = dstStride_.begin();

        if(src.isContiguous<1>() && dst.isContiguous<1>()) {
            for(std::size_t i = 0; i != size[0]; ++i)
                std::memcpy(dstPtr + i*dstStride[0],
                            srcPtr + i*srcStride[0], size[1]);

        } else {
            for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                const char* srcPtr0 = srcPtr + i0*srcStride[0];
                char* dstPtr0 = dstPtr + i0*dstStride[0];
                for(std::size_t i1 = 0; i1 != size[1]; ++i1)
                    *(dstPtr0 + i1*dstStride[1]) =
                        *(srcPtr0 + i1*srcStride[1]);
            }
        }
    }
}

void copy(const Containers::StridedArrayView3D<const char>& src, const Containers::StridedArrayView3D<char>& dst) {
    const Containers::StridedDimensions<3, std::size_t> srcSize_ = src.size();
    const Containers::StridedDimensions<3, std::size_t> dstSize_ = dst.size();
    CORRADE_ASSERT(srcSize_ == dstSize_,
        "Utility::Algorithms::copy(): sizes" << srcSize_ << "and" << dstSize_ << "don't match", );

    const std::size_t* const size = srcSize_.begin();
    auto* const srcPtr = static_cast<const char*>(src.data());
    auto* const dstPtr = static_cast<char*>(dst.data());

    if(src.isContiguous() && dst.isContiguous())
        std::memcpy(dstPtr, srcPtr, size[0]*size[1]*size[2]);
    else {
        const Containers::StridedDimensions<3, std::ptrdiff_t> srcStride_ = src.stride();
        const Containers::StridedDimensions<3, std::ptrdiff_t> dstStride_ = dst.stride();
        const std::ptrdiff_t* const srcStride = srcStride_.begin();
        const std::ptrdiff_t* const dstStride = dstStride_.begin();

        if(src.isContiguous<1>() && dst.isContiguous<1>()) {
            const std::size_t size12 = size[1]*size[2];
            for(std::size_t i = 0; i != size[0]; ++i)
                std::memcpy(dstPtr + i*dstStride[0],
                            srcPtr + i*srcStride[0], size12);
        } else {
            if(src.isContiguous<2>() && dst.isContiguous<2>()) {
                for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                    const char* srcPtr0 = srcPtr + i0*srcStride[0];
                    char* dstPtr0 = dstPtr + i0*dstStride[0];
                    for(std::size_t i1 = 0; i1 != size[1]; ++i1)
                        std::memcpy(dstPtr0 + i1*dstStride[1],
                                    srcPtr0 + i1*srcStride[1], size[2]);
                }
            } else {
                for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                    const char* srcPtr0 = srcPtr + i0*srcStride[0];
                    char* dstPtr0 = dstPtr + i0*dstStride[0];
                    for(std::size_t i1 = 0; i1 != size[1]; ++i1) {
                        const char* srcPtr1 = srcPtr0 + i1*srcStride[1];
                        char* dstPtr1 = dstPtr0 + i1*dstStride[1];
                        for(std::size_t i2 = 0; i2 != size[2]; ++i2)
                            *(dstPtr1 + i2*dstStride[2]) =
                                *(srcPtr1 + i2*srcStride[2]);
                    }
                }
            }
        }
    }
}

void copy(const Containers::StridedArrayView4D<const char>& src, const Containers::StridedArrayView4D<char>& dst) {
    const Containers::StridedDimensions<4, std::size_t> srcSize_ = src.size();
    const Containers::StridedDimensions<4, std::size_t> dstSize_ = dst.size();
    CORRADE_ASSERT(srcSize_ == dstSize_,
        "Utility::Algorithms::copy(): sizes" << srcSize_ << "and" << dstSize_ << "don't match", );

    const std::size_t* const size = srcSize_.begin();
    auto* const srcPtr = static_cast<const char*>(src.data());
    auto* const dstPtr = static_cast<char*>(dst.data());

    if(src.isContiguous() && dst.isContiguous())
        std::memcpy(dstPtr, srcPtr, size[0]*size[1]*size[2]*size[3]);
    else {
        const Containers::StridedDimensions<4, std::ptrdiff_t> srcStride_ = src.stride();
        const Containers::StridedDimensions<4, std::ptrdiff_t> dstStride_ = dst.stride();
        const std::ptrdiff_t* const srcStride = srcStride_.begin();
        const std::ptrdiff_t* const dstStride = dstStride_.begin();

        if(src.isContiguous<1>() && dst.isContiguous<1>()) {
            const std::size_t size123 = size[1]*size[2]*size[3];
            for(std::size_t i = 0; i != size[0]; ++i)
                std::memcpy(dstPtr + i*dstStride[0],
                            srcPtr + i*srcStride[0], size123);
        } else {
            if(src.isContiguous<2>() && dst.isContiguous<2>()) {
                const std::size_t size23 = size[2]*size[3];
                for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                    const char* srcPtr0 = srcPtr + i0*srcStride[0];
                    char* dstPtr0 = dstPtr + i0*dstStride[0];
                    for(std::size_t i1 = 0; i1 != size[1]; ++i1)
                        std::memcpy(dstPtr0 + i1*dstStride[1],
                                    srcPtr0 + i1*srcStride[1], size23);
                }
            } else {
                if(src.isContiguous<3>() && dst.isContiguous<3>()) {
                    /* Here I tried to have a special branch where it would
                       copy the elements directly instead of using memcpy if
                       size[3] < 32, but that caused both debug and release
                       performance to worsen. Additionally this would need to
                       check for alignment etc and that's probably not worth
                       it. */
                    for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                        const char* srcPtr0 = srcPtr + i0*srcStride[0];
                        char* dstPtr0 = dstPtr + i0*dstStride[0];
                        for(std::size_t i1 = 0; i1 != size[1]; ++i1) {
                            const char* srcPtr1 = srcPtr0 + i1*srcStride[1];
                            char* dstPtr1 = dstPtr0 + i1*dstStride[1];
                            for(std::size_t i2 = 0; i2 != size[2]; ++i2)
                                std::memcpy(dstPtr1 + i2*dstStride[2],
                                            srcPtr1 + i2*srcStride[2], size[3]);
                        }
                    }
                } else {
                    for(std::size_t i0 = 0; i0 != size[0]; ++i0) {
                        const char* srcPtr0 = srcPtr + i0*srcStride[0];
                        char* dstPtr0 = dstPtr + i0*dstStride[0];
                        for(std::size_t i1 = 0; i1 != size[1]; ++i1) {
                            const char* srcPtr1 = srcPtr0 + i1*srcStride[1];
                            char* dstPtr1 = dstPtr0 + i1*dstStride[1];
                            for(std::size_t i2 = 0; i2 != size[2]; ++i2) {
                                const char* srcPtr2 = srcPtr1 + i2*srcStride[2];
                                char* dstPtr2 = dstPtr1 + i2*dstStride[2];
                                for(std::size_t i3 = 0; i3 != size[3]; ++i3)
                                    *(dstPtr2 + i3*dstStride[3]) =
                                        *(srcPtr2 + i3*srcStride[3]);
                            }
                        }
                    }
                }
            }
        }
    }
}

}}
