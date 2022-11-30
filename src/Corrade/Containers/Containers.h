#ifndef Corrade_Containers_Containers_h
#define Corrade_Containers_Containers_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

/** @file
 * @brief Forward declarations for the @ref Corrade::Containers namespace
 */

#include <type_traits>
#include <cstddef>

#include "Corrade/configure.h"

namespace Corrade { namespace Containers {

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T, class = void(*)(T*, std::size_t)> class Array;
template<class> class ArrayView;
class ArrayTuple;
template<std::size_t, class> class StaticArrayView;
template<class T> using ArrayView1 = StaticArrayView<1, T>;
template<class T> using ArrayView2 = StaticArrayView<2, T>;
template<class T> using ArrayView3 = StaticArrayView<3, T>;
template<class T> using ArrayView4 = StaticArrayView<4, T>;
template<std::size_t, class> class StaticArray;
template<class T> using Array1 = StaticArray<1, T>;
template<class T> using Array2 = StaticArray<2, T>;
template<class T> using Array3 = StaticArray<3, T>;
template<class T> using Array4 = StaticArray<4, T>;

template<class T, std::size_t size = 1 << (sizeof(T)*8 - 6)> class BigEnumSet;

class BitArray;
template<class> class BasicBitArrayView;
typedef BasicBitArrayView<const char> BitArrayView;
typedef BasicBitArrayView<char> MutableBitArrayView;

template<unsigned, class> class StridedDimensions;
template<unsigned dimensions> using Size = StridedDimensions<dimensions, std::size_t>;
typedef Size<1> Size1D;
typedef Size<2> Size2D;
typedef Size<3> Size3D;
typedef Size<4> Size4D;
template<unsigned dimensions> using Stride = StridedDimensions<dimensions, std::ptrdiff_t>;
typedef Stride<1> Stride1D;
typedef Stride<2> Stride2D;
typedef Stride<3> Stride3D;
typedef Stride<4> Stride4D;

template<unsigned, class> class StridedArrayView;
template<unsigned, class> class StridedIterator;
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;
template<class T> using StridedArrayView4D = StridedArrayView<4, T>;

template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)> class EnumSet;

template<class> class Iterable;
template<class> class IterableIterator;

template<class> class LinkedList;
template<class Derived, class List = LinkedList<Derived>> class LinkedListItem;

template<class> class Optional;
template<class, class> class Pair;
template<class> class Pointer;
template<class> class Reference;
template<class> class MoveReference;
template<class> class AnyReference;

class ScopeGuard;

enum class StringViewFlag: std::size_t;
/* This is defined again in StringView.h using actual enum values to ensure
   consistency */
typedef EnumSet<StringViewFlag, (std::size_t{3} << (sizeof(std::size_t)*8 - 2))> StringViewFlags;
class String;
template<class> class BasicStringView;
typedef BasicStringView<const char> StringView;
typedef BasicStringView<char> MutableStringView;

class StringIterable;
class StringIterableIterator;

template<class, class, class> class Triple;
#endif

}}

#endif
