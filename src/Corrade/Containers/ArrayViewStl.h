#ifndef Corrade_Containers_ArrayViewStl_h
#define Corrade_Containers_ArrayViewStl_h
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
@brief STL compatibility for @ref Corrade::Containers::ArrayView and @ref Corrade::Containers::StaticArrayView
@m_since{2019,10}

Including this header allows you to convert an
@ref Corrade::Containers::ArrayView / @ref Corrade::Containers::StaticArrayView
from and to @ref std::array or @ref std::vector. A separate
@ref Corrade/Containers/ArrayViewStlSpan.h header provides compatibility with
@ref std::span from C++2a. See @ref Containers-ArrayView-stl "ArrayView STL compatibility"
for more information.
*/

#include <array>
#include <vector>

#include "Corrade/Containers/ArrayView.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

/* std::array to ArrayView */
template<std::size_t size, class T> struct ArrayViewConverter<T, std::array<T, size>> {
    constexpr static ArrayView<T> from(std::array<T, size>& other) {
        return {other.data(), other.size()};
    }
    constexpr static ArrayView<T> from(std::array<T, size>&& other) {
        return {other.data(), other.size()};
    }
};
template<std::size_t size, class T> struct ArrayViewConverter<const T, std::array<T, size>> {
    constexpr static ArrayView<const T> from(const std::array<T, size>& other) {
        return {other.data(), other.size()};
    }
};
template<std::size_t size, class T> struct ErasedArrayViewConverter<std::array<T, size>>: ArrayViewConverter<T, std::array<T, size>> {};
template<std::size_t size, class T> struct ErasedArrayViewConverter<const std::array<T, size>>: ArrayViewConverter<const T, std::array<T, size>> {};

/* std::vector to ArrayView */
template<class T, class Allocator> struct ArrayViewConverter<T, std::vector<T, Allocator>> {
    static ArrayView<T> from(std::vector<T, Allocator>& other) {
        return {other.data(), other.size()};
    }
    static ArrayView<T> from(std::vector<T, Allocator>&& other) {
        return {other.data(), other.size()};
    }
};
template<class T, class Allocator> struct ArrayViewConverter<const T, std::vector<T, Allocator>> {
    static ArrayView<const T> from(const std::vector<T, Allocator>& other) {
        return {other.data(), other.size()};
    }
};
template<class T, class Allocator> struct ErasedArrayViewConverter<std::vector<T, Allocator>>: ArrayViewConverter<T, std::vector<T, Allocator>> {};
template<class T, class Allocator> struct ErasedArrayViewConverter<const std::vector<T, Allocator>>: ArrayViewConverter<const T, std::vector<T, Allocator>> {};

/* std::array to StaticArrayView */
template<std::size_t size, class T> struct StaticArrayViewConverter<size, T, std::array<T, size>> {
    constexpr static StaticArrayView<size, T> from(std::array<T, size>& other) {
        return StaticArrayView<size, T>{other.data()};
    }
};
template<std::size_t size, class T> struct StaticArrayViewConverter<size, const T, std::array<T, size>> {
    constexpr static StaticArrayView<size, const T> from(const std::array<T, size>& other) {
        return StaticArrayView<size, const T>(other.data());
    }
};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<std::array<T, size>>: StaticArrayViewConverter<size, T, std::array<T, size>> {};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<const std::array<T, size>>: StaticArrayViewConverter<size, const T, std::array<T, size>> {};

}}}
#endif

#endif
