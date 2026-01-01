#ifndef Corrade_Containers_StructuredBindings_h
#define Corrade_Containers_StructuredBindings_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2022 Stanislaw Halik <sthalik@misaki.pl>

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
@brief @m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size) and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element) specializations for container types
@m_since_latest

Contains specializations of @m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
for @relativeref{Corrade,Containers::Pair},
@relativeref{Corrade,Containers::Triple},
@relativeref{Corrade,Containers::StaticArray},
@relativeref{Corrade,Containers::StaticArrayView} and
@relativeref{Corrade,Containers::StridedDimensions}, making them usable with
C++17 structured bindings. See documentation of each class and the
@ref Corrade/Utility/StlForwardTupleSizeElement.h header for more information.
*/

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/StlForwardTupleSizeElement.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
/* Can't have `template<> struct std::tuple_size<T>`, on GCC 4.8 it leads to
   `error: specialization of '...' in different namespace`. */
namespace std {

/* The actual get() implementations are friend functions defined in each class,
   but the std::tuple_size / std::tuple_element specializations may require
   including a non-trivial STL header so they're separate. See the
   StlForwardTupleSizeElement.h file for implementation-specific details.

   Each of these is additionally guarded by an extra ifdef so single-header
   libraries can pick up individual bits without pulling in all other types as
   well.

   To avoid a need for `#include <utility>`, which provides convenience
   tuple_size and tuple_element overloads for `const T`, those are explicitly
   added for each type. */

/* Pair */
#ifndef Corrade_Containers_StructuredBindings_Pair_h
#define Corrade_Containers_StructuredBindings_Pair_h
template<class F, class S> struct tuple_size<Corrade::Containers::Pair<F, S>>: integral_constant<size_t, 2> {};
template<class F, class S> struct tuple_size<const Corrade::Containers::Pair<F, S>>: integral_constant<size_t, 2> {};
template<class F, class S> struct tuple_element<0, Corrade::Containers::Pair<F, S>> { typedef F type; };
template<class F, class S> struct tuple_element<1, Corrade::Containers::Pair<F, S>> { typedef S type; };
template<size_t i, class F, class S> struct tuple_element<i, const Corrade::Containers::Pair<F, S>> {
    typedef const typename tuple_element<i, Corrade::Containers::Pair<F, S>>::type type;
};
#endif

/* Triple */
#ifndef Corrade_Containers_StructuredBindings_Triple_h
#define Corrade_Containers_StructuredBindings_Triple_h
template<class F, class S, class T> struct tuple_size<Corrade::Containers::Triple<F, S, T>>: integral_constant<size_t, 3> {};
template<class F, class S, class T> struct tuple_size<const Corrade::Containers::Triple<F, S, T>>: integral_constant<size_t, 3> {};
template<class F, class S, class T> struct tuple_element<0, Corrade::Containers::Triple<F, S, T>> { typedef F type; };
template<class F, class S, class T> struct tuple_element<1, Corrade::Containers::Triple<F, S, T>> { typedef S type; };
template<class F, class S, class T> struct tuple_element<2, Corrade::Containers::Triple<F, S, T>> { typedef T type; };
template<size_t i, class F, class S, class T> struct tuple_element<i, const Corrade::Containers::Triple<F, S, T>> {
    typedef const typename tuple_element<i, Corrade::Containers::Triple<F, S, T>>::type type;
};
#endif

/* StaticArray. Note that `size` can't be used as it may conflict with
   std::size() in C++17. */
#ifndef Corrade_Containers_StructuredBindings_StaticArray_h
#define Corrade_Containers_StructuredBindings_StaticArray_h
template<size_t size_, class T> struct tuple_size<Corrade::Containers::StaticArray<size_, T>>: integral_constant<size_t, size_> {};
template<size_t size_, class T> struct tuple_size<const Corrade::Containers::StaticArray<size_, T>>: integral_constant<size_t, size_> {};
template<size_t index, size_t size_, class T> struct tuple_element<index, Corrade::Containers::StaticArray<size_, T>> { typedef T type; };
template<size_t index, size_t size_, class T> struct tuple_element<index, const Corrade::Containers::StaticArray<size_, T>> { typedef const T type; };
#endif

/* StaticArrayView. Note that `size` can't be used as it may conflict with
   std::size() in C++17. */
#ifndef Corrade_Containers_StructuredBindings_StaticArrayView_h
#define Corrade_Containers_StructuredBindings_StaticArrayView_h
template<size_t size_, class T> struct tuple_size<Corrade::Containers::StaticArrayView<size_, T>>: integral_constant<size_t, size_> {};
template<size_t size_, class T> struct tuple_size<const Corrade::Containers::StaticArrayView<size_, T>>: integral_constant<size_t, size_> {};
template<size_t index, size_t size_, class T> struct tuple_element<index, Corrade::Containers::StaticArrayView<size_, T>> { typedef T type; };
template<size_t index, size_t size_, class T> struct tuple_element<index, const Corrade::Containers::StaticArrayView<size_, T>> { typedef const T type; };
#endif

/* StridedDimensions */
#ifndef Corrade_Containers_StructuredBindings_StridedDimensions_h
#define Corrade_Containers_StructuredBindings_StridedDimensions_h
template<unsigned dimensions, class T> struct tuple_size<Corrade::Containers::StridedDimensions<dimensions, T>>: integral_constant<size_t, dimensions> {};
template<unsigned dimensions, class T> struct tuple_size<const Corrade::Containers::StridedDimensions<dimensions, T>>: integral_constant<size_t, dimensions> {};
template<size_t index, unsigned dimensions, class T> struct tuple_element<index, Corrade::Containers::StridedDimensions<dimensions, T>> { typedef T type; };
template<size_t index, unsigned dimensions, class T> struct tuple_element<index, const Corrade::Containers::StridedDimensions<dimensions, T>> { typedef const T type; };
#endif

}
#endif

#endif
