#ifndef Corrade_Containers_StridedArrayViewStl_h
#define Corrade_Containers_StridedArrayViewStl_h
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
@brief STL compatibility for @ref Corrade::Containers::StridedArrayView
@m_since_latest

Including this header allows you to use strided views in STL algorithms such as
@ref std::lower_bound(). See @ref Containers-StridedArrayView-stl for more
information.
*/

#include <iterator>

#include "Corrade/Containers/StridedArrayView.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace std {

/* STL, I wonder why this can't just be inferred. All the info is there,
   directly in the class.

   Currently only for one dimension because nested views returned by iterators
   are probably not useful for anything in (1D) STL algorithms. And the
   returned views are temporary and thus it's unclear what a "pointer" or
   "reference" should be. */
template<class T> struct iterator_traits<Corrade::Containers::StridedIterator<1, T>> {
    typedef std::ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::random_access_iterator_tag iterator_category;
};

}
#endif

#endif
