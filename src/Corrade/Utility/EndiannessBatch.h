#ifndef Corrade_Utility_EndiannessBatch_h
#define Corrade_Utility_EndiannessBatch_h
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

/** @file
 * @brief Namespace @ref Corrade::Utility::Endianness
 * @m_since{2020,06}
 */

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Utility/Endianness.h"

namespace Corrade { namespace Utility { namespace Endianness {

namespace Implementation {
    template<class T> inline void swapInPlace(const Containers::StridedArrayView1D<T>& values) {
        /** @todo maybe we could speed this up via some SIMD? also, what about
            alignment? */
        for(T& value: values) Implementation::swapInPlace(value);
    }
}

/**
@brief Endian-swap bytes of each argument in-place
@m_since{2020,06}

Equivalent to calling @ref swap() on each value.
@see @ref littleEndianInPlace(const Containers::StridedArrayView1D<T>&),
    @ref bigEndianInPlace(const Containers::StridedArrayView1D<T>&)
*/
template<class T> void swapInPlace(const Containers::StridedArrayView1D<T>& values) {
    /* Done like this instead of calling swap() in a loop on the original type,
       as that involves a lot function calls and memcpying and stuff */
    return Implementation::swapInPlace(Containers::arrayCast<typename Implementation::TypeFor<sizeof(T)>::Type>(values));
}

/**
 * @overload
 * @m_since{2020,06}
 */
template<class T> void swapInPlace(const Containers::ArrayView<T>& values) {
    return swapInPlace(Containers::stridedArrayView(values));
}

/**
@brief Convert values from or to Little-Endian in-place
@m_since{2020,06}

On Big-Endian systems calls @ref swapInPlace(const Containers::StridedArrayView1D<T>&),
on Little-Endian systems does nothing.
@see @ref isBigEndian(), @ref CORRADE_TARGET_BIG_ENDIAN, @ref littleEndian(),
    @ref bigEndianInPlace(const Containers::StridedArrayView1D<T>&)
*/
template<class T> inline void littleEndianInPlace(const Containers::StridedArrayView1D<T>& values) {
    #ifdef CORRADE_TARGET_BIG_ENDIAN
    swapInPlace(values);
    #else
    static_cast<void>(values);
    #endif
}

/**
 * @overload
 * @m_since{2020,06}
 */
template<class T> void littleEndianInPlace(const Containers::ArrayView<T>& values) {
    return littleEndianInPlace(Containers::stridedArrayView(values));
}

/**
@brief Convert values from or to Big-Endian in-place
@m_since{2020,06}

On Little-Endian systems calls @ref swapInPlace(const Containers::StridedArrayView1D<T>&),
on Big-Endian systems does nothing.
@see @ref isBigEndian(), @ref CORRADE_TARGET_BIG_ENDIAN, @ref bigEndian(),
    @ref littleEndianInPlace(const Containers::StridedArrayView1D<T>&)
*/
template<class T> inline void bigEndianInPlace(const Containers::StridedArrayView1D<T>& values) {
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    swapInPlace(values);
    #else
    static_cast<void>(values);
    #endif
}

/**
 * @overload
 * @m_since{2020,06}
 */
template<class T> void bigEndianInPlace(const Containers::ArrayView<T>& values) {
    return bigEndianInPlace(Containers::stridedArrayView(values));
}

}}}

#endif
