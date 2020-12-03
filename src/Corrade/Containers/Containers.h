#ifndef Corrade_Containers_Containers_h
#define Corrade_Containers_Containers_h
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
template<std::size_t, class> class StaticArray;
template<class T> using Array1 = StaticArray<1, T>;
template<class T> using Array2 = StaticArray<2, T>;
template<class T> using Array3 = StaticArray<3, T>;
template<class T> using Array4 = StaticArray<4, T>;

template<unsigned, class> class StridedDimensions;
template<unsigned, class> class StridedArrayView;
template<unsigned, class> class StridedIterator;
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;
template<class T> using StridedArrayView4D = StridedArrayView<4, T>;

template<class T, typename std::underlying_type<T>::type fullValue = typename std::underlying_type<T>::type(~0)> class EnumSet;
template<class> class LinkedList;
template<class Derived, class List = LinkedList<Derived>> class LinkedListItem;

template<class T> class Optional;
template<class T> class Pointer;
template<class T> class Reference;

class ScopeGuard;

class String;
template<class> class BasicStringView;
typedef BasicStringView<const char> StringView;
typedef BasicStringView<char> MutableStringView;
#endif

}}

#endif
