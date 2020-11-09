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

#include "Algorithms.h"

#include <cstring>
/* CORRADE_FALLTHROUGH, needed on Clang when CORRADE_NO_ASSERT is defined */
#include <Corrade/Utility/Macros.h>

namespace Corrade { namespace Utility {

/* I might be going a bit overboard with the avoidance of inline function calls
   in Debug -- should revisit with a clearer mind later.

   To avoid maintenance hell, the 1D/2D/3D variants populate a 4D view with the
   first dimensions having a size of 1, thus "drilling down" directly to the
   leaf branches of the 4D variant. The overhead isn't too big.

   OTOH the ArrayView overhead can delegate directly to a memcpy, so for code
   size savings it shouldn't be handled in the complex stride-aware code. */

void copy(const Containers::ArrayView<const void>& src, const Containers::ArrayView<void>& dst) {
    const std::size_t srcSize = src.size();
    #ifndef CORRADE_NO_ASSERT
    const std::size_t dstSize = dst.size();
    #endif
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    std::memcpy(dst.data(), src.data(), srcSize);
}

void copy(const Containers::StridedArrayView1D<const char>& src, const Containers::StridedArrayView1D<char>& dst) {
    const std::size_t srcSize = src.size();
    #ifndef CORRADE_NO_ASSERT
    const std::size_t dstSize = dst.size();
    #endif
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    const std::ptrdiff_t srcStride = src.stride();
    const std::ptrdiff_t dstStride = dst.stride();
    /* Using ~std::size_t{} for arrayview size as a shortcut -- there it's just
       for the size assert anyway */
    return copy(Containers::StridedArrayView4D<const char>{
            {static_cast<const char*>(src.data()), ~std::size_t{}},
            {1, 1, 1, srcSize},
            {srcStride, srcStride, srcStride, srcStride}},
        Containers::StridedArrayView4D<char>{
            {static_cast<char*>(dst.data()), ~std::size_t{}},
            {1, 1, 1, srcSize},
            {dstStride, dstStride, dstStride, dstStride}});
}

void copy(const Containers::StridedArrayView2D<const char>& src, const Containers::StridedArrayView2D<char>& dst) {
    const Containers::StridedDimensions<2, std::size_t> srcSize = src.size();
    #ifndef CORRADE_NO_ASSERT
    const Containers::StridedDimensions<2, std::size_t> dstSize = dst.size();
    #endif
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    const std::size_t* const size = srcSize.begin();
    const std::ptrdiff_t* const srcStride = src.stride().begin();
    const std::ptrdiff_t* const dstStride = dst.stride().begin();
    /* Using ~std::size_t{} for arrayview size as a shortcut -- there it's just
       for the size assert anyway */
    return copy(Containers::StridedArrayView4D<const char>{
            {static_cast<const char*>(src.data()), ~std::size_t{}},
            {1, 1, size[0], size[1]},
            {srcStride[0], srcStride[0], srcStride[0], srcStride[1]}},
        Containers::StridedArrayView4D<char>{
            {static_cast<char*>(dst.data()), ~std::size_t{}},
            {1, 1, size[0], size[1]},
            {dstStride[0], dstStride[0], dstStride[0], dstStride[1]}});
}

void copy(const Containers::StridedArrayView3D<const char>& src, const Containers::StridedArrayView3D<char>& dst) {
    const Containers::StridedDimensions<3, std::size_t> srcSize = src.size();
    #ifndef CORRADE_NO_ASSERT
    const Containers::StridedDimensions<3, std::size_t> dstSize = dst.size();
    #endif
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    const std::size_t* const size = srcSize.begin();
    const std::ptrdiff_t* const srcStride = src.stride().begin();
    const std::ptrdiff_t* const dstStride = dst.stride().begin();
    /* Using ~std::size_t{} for arrayview size as a shortcut -- there it's just
       for the size assert anyway */
    return copy(Containers::StridedArrayView4D<const char>{
            {static_cast<const char*>(src.data()), ~std::size_t{}},
            {1, size[0], size[1], size[2]},
            {srcStride[0], srcStride[0], srcStride[1], srcStride[2]}},
        Containers::StridedArrayView4D<char>{
            {static_cast<char*>(dst.data()), ~std::size_t{}},
            {1, size[0], size[1], size[2]},
            {dstStride[0], dstStride[0], dstStride[1], dstStride[2]}});
}

void copy(const Containers::StridedArrayView4D<const char>& src, const Containers::StridedArrayView4D<char>& dst) {
    const Containers::StridedDimensions<4, std::size_t> srcSize = src.size();
    #ifndef CORRADE_NO_ASSERT
    const Containers::StridedDimensions<4, std::size_t> dstSize = dst.size();
    #endif
    CORRADE_ASSERT(srcSize == dstSize,
        "Utility::Algorithms::copy(): sizes" << srcSize << "and" << dstSize << "don't match", );

    const std::size_t* const size = srcSize.begin();
    auto* const srcPtr = static_cast<const char*>(src.data());
    auto* const dstPtr = static_cast<char*>(dst.data());

    /* If the size is zero in any direction, there's nothing to do, so no need
       to go through all the loops below. Additionally Duff's device doesn't
       expect a zero size, so this prevents it from accessing bad memory.
       Ideally this check would be directly in the 1D/2D/3D "delegators" above
       as well, but I don't think it'd be a perf issue so checking just here,
       at the bottom of it all. */
    for(std::size_t i = 0; i != 4; ++i) if(!size[i]) return;

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
                /* On Clang, for smaller sizes in the last dimension we prefer
                   Duff's device. The size is chosen based on the benchmark in
                   the test, might need to adjust for different platforms. For
                   Clang on Mac, copyBenchmark3DNonContiguous() numbers:

                    bytes   memcpy  loop    duff
                    ------- ------- ------- -----
                    1B      55.3    30.37   24.43
                    4B      19.3    13.91   12.34
                    8B       6.6    11.32    7.90 (for >= 8B it gets worse)

                   It becomes slightly worse in Debug (but not slower than a
                   hand-written loop using operator[], so I think that's still
                   acceptable). OTOH, GCC is slower with Duff in both Debug and Release, so there we use the loop instead. */
                if(src.isContiguous<3>() && dst.isContiguous<3>() && size[3] >= 8) {
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

                                #if !defined(CORRADE_TARGET_CLANG)
                                for(std::size_t i3 = 0; i3 != size[3]; ++i3)
                                    *(dstPtr2 + i3*dstStride[3]) =
                                        *(srcPtr2 + i3*srcStride[3]);
                                #else
                                std::size_t n = (size[3] + 7)/8;
                                switch(size[3]%8) {
                                    case 0: do { *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 7:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 6:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 5:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 4:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 3:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 2:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                                 CORRADE_FALLTHROUGH
                                    case 1:      *dstPtr2 = *srcPtr2;
                                                 dstPtr2 += dstStride[3];
                                                 srcPtr2 += srcStride[3];
                                            } while(--n > 0);
                                }
                                #endif
                            }
                        }
                    }
                }
            }
        }
    }
}

}}
