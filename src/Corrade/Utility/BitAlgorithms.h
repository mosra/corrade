#ifndef Corrade_Utility_BitAlgorithms_h
#define Corrade_Utility_BitAlgorithms_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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
 * @brief Function @ref Corrade::Utility::copyMasked()
 * @m_since_latest
 */

#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Utility/TypeTraits.h" /* CORRADE_NO_STD_IS_TRIVIALLY_TRAITS */
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Copy a masked array view to another
@m_since_latest

For all bits that are set in @p srcMask takes an element from @p src and
copies it to @p dst. Expects that @p src and @p srcMask have the same size,
that count of bits set in @p srcMask is the same as @p dst size and that the
second dimension of both @p src and @p dst has the same size and is contiguous.
@experimental
*/
CORRADE_UTILITY_EXPORT void copyMasked(const Containers::StridedArrayView2D<const char>& src, Containers::BitArrayView srcMask, const Containers::StridedArrayView2D<char>& dst);

/**
@brief Copy a masked array view to another
@m_since_latest

Casts views into a @cpp char @ce type of one dimension more (where the last
dimension has a size of @cpp sizeof(T) @ce and delegates into
@ref copyMasked(const Containers::StridedArrayView2D<const char>&, Containers::BitArrayView, const Containers::StridedArrayView2D<char>&).
Expects that @p T is a trivially copyable type.
@experimental
*/
template<class T> void copyMasked(const Containers::StridedArrayView1D<const T>& src, Containers::BitArrayView srcMask, const Containers::StridedArrayView1D<T>& dst) {
    static_assert(
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #else
        std::is_trivially_copyable<T>::value
        #endif
        , "types must be trivially copyable");

    return copyMasked(Containers::arrayCast<2, const char>(src), srcMask,
                      Containers::arrayCast<2, char>(dst));
}

namespace Implementation {

/* Adapted from Algorithms.h and restricted to 1D strided array views */
template<class T, class View = decltype(Containers::Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>()))> static Containers::StridedArrayView1D<typename View::Type> stridedArrayView1DTypeFor(T&&);
template<class T> static Containers::StridedArrayView1D<T> stridedArrayView1DTypeFor(const Containers::ArrayView<T>&);
template<std::size_t size, class T> static Containers::StridedArrayView1D<T> stridedArrayView1DTypeFor(T(&)[size]);
template<class T> static Containers::StridedArrayView1D<T> stridedArrayView1DTypeFor(const Containers::StridedArrayView1D<T>&);

}

/* Hidden from documentation output because it only adds confusion on top of
   the base StridedArrayView variants above. All it does is automagic that
   makes the arguments convert to an appropriate StridedArrayView without
   explicit casting on user side. */
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class From, class To, class FromView = decltype(Implementation::stridedArrayView1DTypeFor(std::declval<From&&>())), class ToView = decltype(Implementation::stridedArrayView1DTypeFor(std::declval<To&&>()))> void copyMasked(From&& src, const Containers::BitArrayView srcMask, To&& dst) {
    static_assert(std::is_same<typename std::remove_const<typename FromView::Type>::type, typename std::remove_const<typename ToView::Type>::type>::value, "can't copy between views of different types");
    static_assert(!std::is_const<typename ToView::Type>::value, "can't copy to a const view");
    /* We need to pass const& to the copyMasked(), passing temporary instances
       directly would lead to infinite recursion */
    const Containers::StridedArrayView1D<const typename FromView::Type> srcV{src};
    const Containers::StridedArrayView1D<typename ToView::Type> dstV{dst};
    copyMasked(srcV, srcMask, dstV);
}
#endif

}}

#endif
