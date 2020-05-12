#ifndef Corrade_Containers_ArrayViewStlSpan_h
#define Corrade_Containers_ArrayViewStlSpan_h
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
@brief STL `std::span` compatibility for @ref Corrade::Containers::ArrayView and @ref Corrade::Containers::StaticArrayView
@m_since{2019,10}

Including this header allows you to convert
@ref Corrade::Containers::ArrayView / @ref Corrade::Containers::StaticArrayView
from and to a C++2a @ref std::span. A separate
@ref Corrade/Containers/ArrayViewStl.h header provides compatibility with
@ref std::array and @ref std::vector. See
@ref Containers-ArrayView-stl "ArrayView STL compatibility" for more
information.
*/

#include <span>

#include "Corrade/Containers/ArrayView.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

/* libc++ before version 9 had an earlier revision of <span> with ptrdiff_t
   used as a size type. That inconsistency was fortunately fixed, however we
   need to preserve compatibility so this is getting a bit ugly. */
#if defined(CORRADE_TARGET_LIBCXX) && _LIBCPP_VERSION < 9000
typedef std::ptrdiff_t StlSpanSizeType;
#else
typedef std::size_t StlSpanSizeType;
#endif

/* dynamic std::span from/to ArrayView */
template<class T> struct ArrayViewConverter<T, std::span<T>> {
    constexpr static ArrayView<T> from(std::span<T> other) {
        return {other.data(), std::size_t(other.size())};
    }
};
template<class T> struct ArrayViewConverter<T, const std::span<T>>: ArrayViewConverter<T, std::span<T>> {};
template<class T> struct ArrayViewConverter<const T, std::span<T>> {
    constexpr static ArrayView<const T> from(std::span<T> other) {
        return {other.data(), std::size_t(other.size())};
    }
};
template<class T> struct ErasedArrayViewConverter<std::span<T>>: ArrayViewConverter<T, std::span<T>> {};
template<class T> struct ErasedArrayViewConverter<const std::span<T>>: ArrayViewConverter<T, std::span<T>> {};

/* static std::span to ArrayView */
template<class T, StlSpanSizeType Extent> struct ArrayViewConverter<T, std::span<T, Extent>> {
    constexpr static ArrayView<T> from(std::span<T, Extent> other) {
        return {other.data(), std::size_t(other.size())};
    }
    /* Other way not possible as ArrayView has dynamic size */
};
template<class T, StlSpanSizeType Extent> struct ArrayViewConverter<const T, std::span<T, Extent>> {
    constexpr static ArrayView<T> from(std::span<T, Extent> other) {
        return {other.data(), std::size_t(other.size())};
    }
};
template<class T, StlSpanSizeType Extent> struct ArrayViewConverter<T, const std::span<T, Extent>>: ArrayViewConverter<T, std::span<T, Extent>> {};
template<class T, StlSpanSizeType Extent> struct ErasedArrayViewConverter<std::span<T, Extent>>: ArrayViewConverter<T, std::span<T, Extent>> {};
template<class T, StlSpanSizeType Extent> struct ErasedArrayViewConverter<const std::span<T, Extent>>: ArrayViewConverter<T, std::span<T, Extent>> {};

/* static std::span from/to StaticArrayView */
template<std::size_t size, class T> struct StaticArrayViewConverter<size, T, std::span<T, StlSpanSizeType(size)>> {
    constexpr static StaticArrayView<size, T> from(std::span<T, StlSpanSizeType(size)> other) {
        return StaticArrayView<size, T>{other.data()};
    }
    #if !defined(CORRADE_TARGET_LIBCXX) || _LIBCPP_VERSION >= 9000
    /* std::span in libc++ < 9 has an implicit all-catching constructor, which
       means we can't implement our own to() routines with compile time checks
       for proper size in case of fixed-size views */
    constexpr static std::span<T, size> to(StaticArrayView<size, T> other) {
        return std::span<T, size>{other.data(), other.size()};
    }
    #endif
};
template<std::size_t size, class T> struct StaticArrayViewConverter<size, T, const std::span<T, StlSpanSizeType(size)>>: StaticArrayViewConverter<size, T, std::span<T, StlSpanSizeType(size)>> {};
template<std::size_t size, class T> struct StaticArrayViewConverter<size, const T, std::span<T, StlSpanSizeType(size)>> {
    constexpr static StaticArrayView<size, const T> from(std::span<T, StlSpanSizeType(size)> other) {
        return StaticArrayView<size, const T>{other.data()};
    }
};
#if !defined(CORRADE_TARGET_LIBCXX) || _LIBCPP_VERSION >= 9000
/* std::span in libc++ < 9 has an implicit all-catching constructor, which
   means we can't implement our own to() routines with compile time checks
   for proper size in case of fixed-size views */
template<std::size_t size, class T> struct StaticArrayViewConverter<size, T, std::span<const T, size>> {
    constexpr static std::span<const T, size> to(StaticArrayView<size, T> other) {
        return std::span<const T, size>{other.data(), other.size()};
    }
};
#endif
template<class T, StlSpanSizeType Extent> struct ErasedStaticArrayViewConverter<std::span<T, Extent>>: StaticArrayViewConverter<std::size_t(Extent), T, std::span<T, Extent>> {
    static_assert(Extent >= 0, "can't convert dynamic std::span to StaticArrayView");
};
template<class T, StlSpanSizeType Extent> struct ErasedStaticArrayViewConverter<const std::span<T, Extent>>: StaticArrayViewConverter<std::size_t(Extent), T, std::span<T, Extent>> {
    static_assert(Extent >= 0, "can't convert dynamic std::span to StaticArrayView");
};

}}}
#endif

#endif
