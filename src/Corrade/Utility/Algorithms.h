#ifndef Corrade_Utility_Algorithms_h
#define Corrade_Utility_Algorithms_h
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
 * @brief Function @ref Corrade::Utility::copy()
 * @m_since{2020,06}
 */

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Copy an array view to another
@m_since{2020,06}

Calls @ref std::memcpy() on the contents. Expects that both arrays have the
same size.
*/
CORRADE_UTILITY_EXPORT void copy(const Containers::ArrayView<const void>& src, const Containers::ArrayView<void>& dst);

/**
@brief Copy an array view to another
@m_since{2020,06}

Casts views into a @cpp void @ce type and delegates into
@ref copy(const Containers::ArrayView<const void>&, const Containers::ArrayView<void>&).
Expects that both arrays have the same size and @p T is a trivially copyable
type.
*/
template<class T> inline void copy(const Containers::ArrayView<const T>& src, const Containers::ArrayView<T>& dst) {
    static_assert(
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<T>::value
        #else
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #endif
        , "types must be trivially copyable");

    return copy(Containers::ArrayView<const void>(src), Containers::ArrayView<void>(dst));
}

/**
@brief Copy a strided array view to another
@m_since{2020,06}

Optimized to call @ref std::memcpy() on largest contiguous sub-dimensions,
looping over the non-contiguous dimensions (except when memcpy would be called
for very small pieces of memory, in which case either a loop or a variant of
<a href="https://en.wikipedia.org/wiki/Duff's_device">Duff's device</a> is
used, depending on which is faster on given compiler). The function has
specializations for 1D, 2D, 3D and 4D, higher dimensions recurse into these.
Expects that both arrays have the same size.
@see @ref Containers::StridedArrayView::isContiguous()
*/
template<unsigned dimensions> void copy(const Containers::StridedArrayView<dimensions, const char>& src, const Containers::StridedArrayView<dimensions, char>& dst);

/**
 * @overload
 * @m_since{2020,06}
 */
CORRADE_UTILITY_EXPORT void copy(const Containers::StridedArrayView1D<const char>& src, const Containers::StridedArrayView1D<char>& dst);

/**
 * @overload
 * @m_since{2020,06}
 */
CORRADE_UTILITY_EXPORT void copy(const Containers::StridedArrayView2D<const char>& src, const Containers::StridedArrayView2D<char>& dst);

/**
 * @overload
 * @m_since{2020,06}
 */
CORRADE_UTILITY_EXPORT void copy(const Containers::StridedArrayView3D<const char>& src, const Containers::StridedArrayView3D<char>& dst);

/**
 * @overload
 * @m_since{2020,06}
 */
CORRADE_UTILITY_EXPORT void copy(const Containers::StridedArrayView4D<const char>& src, const Containers::StridedArrayView4D<char>& dst);

/**
@brief Copy a strided array view to another
@m_since{2020,06}

Casts views into a @cpp char @ce type of one dimension more (where the last
dimension has a size of @cpp sizeof(T) @ce and delegates into
@ref copy(const Containers::StridedArrayView<dimensions, const char>&, const Containers::StridedArrayView<dimensions, char>&).
Expects that both arrays have the same size and @p T is a trivially copyable
type.
*/
template<unsigned dimensions, class T> void copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>& dst) {
    static_assert(
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<T>::value
        #else
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #endif
        , "types must be trivially copyable");

    return copy(Containers::arrayCast<dimensions + 1, const char>(src),
                Containers::arrayCast<dimensions + 1, char>(dst));
}

namespace Implementation {

/* Vaguely inspired by the Utility::IsIterable type trait */
template<class T, class View = decltype(Containers::Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>()))> static Containers::StridedArrayView1D<typename View::Type> stridedArrayViewTypeFor(T&&);
template<class T> static Containers::StridedArrayView1D<T> stridedArrayViewTypeFor(const Containers::ArrayView<T>&);
template<unsigned dimensions, class T> static Containers::StridedArrayView<dimensions, T> stridedArrayViewTypeFor(const Containers::StridedArrayView<dimensions, T>&);

template<class> struct StridedArrayViewType;
template<class T> struct StridedArrayViewType<Containers::ArrayView<T>> {
    enum: unsigned { Dimensions = 1 };
    typedef Containers::ArrayView<typename std::remove_const<T>::type> Type;
    typedef Containers::ArrayView<const T> ConstType;
};
template<unsigned dimensions, class T> struct StridedArrayViewType<Containers::StridedArrayView<dimensions, T>> {
    enum: unsigned { Dimensions = dimensions };
    typedef Containers::StridedArrayView<dimensions, typename std::remove_const<T>::type> Type;
    typedef Containers::StridedArrayView<dimensions, const T> ConstType;
};

}

/**
@brief Copy a view to another
@m_since{2020,06}

Converts @p src and @p dst to a common array view type that's either
@ref Containers::ArrayView or @ref Containers::StridedArrayView and then calls
either @ref copy(const Containers::ArrayView<const T>&, const Containers::ArrayView<T>&)
or @ref copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>&).
Works with any type that's convertible to @ref Containers::StridedArrayView,
expects that both views have the same underlying type and the same dimension
count and the @p dst is not @cpp const @ce.
*/
template<class From, class To, class FromView = decltype(Implementation::stridedArrayViewTypeFor(std::declval<From&&>())), class ToView = decltype(Implementation::stridedArrayViewTypeFor(std::declval<To&&>()))> void copy(From&& src, To&& dst) {
    static_assert(std::is_same<typename std::remove_const<typename FromView::Type>::type, typename std::remove_const<typename ToView::Type>::type>::value, "can't copy between views of different types");
    static_assert(!std::is_const<typename ToView::Type>::value, "can't copy to a const view");
    static_assert(unsigned(Implementation::StridedArrayViewType<FromView>::Dimensions) ==
        unsigned(Implementation::StridedArrayViewType<ToView>::Dimensions),
        "can't copy between views of different dimensions");
    /* We need to pass const& to the copy(), passing temporary instances
       directly would lead to infinite recursion */
    const typename std::common_type<
        typename Implementation::StridedArrayViewType<FromView>::ConstType,
        typename Implementation::StridedArrayViewType<ToView>::ConstType>::type srcV{src};
    const typename std::common_type<
        typename Implementation::StridedArrayViewType<FromView>::Type,
        typename Implementation::StridedArrayViewType<ToView>::Type>::type dstV{dst};
    copy(srcV, dstV);
}

template<unsigned dimensions> void copy(const Containers::StridedArrayView<dimensions, const char>& src, const Containers::StridedArrayView<dimensions, char>& dst) {
    CORRADE_ASSERT(src.size() == dst.size(),
        "Utility::Algorithms::copy(): sizes" << src.size() << "and" << dst.size() << "don't match", );

    for(std::size_t i = 0, max = src.size()[0]; i != max; ++i)
        copy(src[i], dst[i]);
}

}}

#endif
