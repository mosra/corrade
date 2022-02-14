#ifndef Corrade_Utility_Algorithms_h
#define Corrade_Utility_Algorithms_h
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
 * @brief Function @ref Corrade::Utility::copy(), @ref Corrade::Utility::flipInPlace()
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
template<unsigned dimensions, class T> void copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>& dst);

namespace Implementation {

/* Vaguely inspired by the Utility::IsIterable type trait */
template<class T, class View = decltype(Containers::Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>()))> static Containers::ArrayView<typename View::Type> arrayViewTypeFor(T&&);
template<class T> static Containers::ArrayView<T> arrayViewTypeFor(const Containers::ArrayView<T>&);
template<std::size_t size, class T
    /* If T is an array of a non-builtin type, Clang 3.8 thinks it's a
       non-trivially-copyable type, failing the assert in copy<T>() above. If
       this overload gets disabled, the copy instead picks the ArrayView<void>
       overload, where ArrayView<void> constructor doesn't check that T is
       trivially copyable (even though it should!). Existing code relies on
       this behavior already, but eventually the ArrayView<void> constructors
       should get the assert as well. Tested in
       AlgorithmsTest::copyMultiDimensionalArray(). */
    #if defined(CORRADE_TARGET_CLANG) && __clang_major__*100 + __clang_minor__ <= 309
    , class = typename std::enable_if<!std::is_array<T>::value>::type
    #endif
> static Containers::ArrayView<T> arrayViewTypeFor(T(&)[size]);
template<unsigned dimensions, class T> static Containers::StridedArrayView<dimensions, T> arrayViewTypeFor(const Containers::StridedArrayView<dimensions, T>&);

}

/**
@brief Copy a view to another
@m_since{2020,06}

Converts @p src and @p dst to a common array view type that's either
@ref Containers::ArrayView or @ref Containers::StridedArrayView and then calls
either @ref copy(const Containers::ArrayView<const T>&, const Containers::ArrayView<T>&)
or @ref copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>&).
Works with any type that's convertible to @ref Containers::StridedArrayView,
expects that both views have the same underlying type, the same dimension count
and size and the @p dst is not @cpp const @ce.
*/
template<class From, class To, class FromView = decltype(Implementation::arrayViewTypeFor(std::declval<From&&>())), class ToView = decltype(Implementation::arrayViewTypeFor(std::declval<To&&>()))> void copy(From&& src, To&& dst);

/**
@brief Copy an initializer list to a view
@m_since_latest

Based on whether the @p dst is convertible to either @ref Containers::ArrayView
or @ref Containers::StridedArrayView, calls either
@ref copy(const Containers::ArrayView<const T>&, const Containers::ArrayView<T>&)
or @ref copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>&).
Works with any @p dst that's convertible to an one-dimensional
@ref Containers::StridedArrayView, expects that @p src has the same underlying
type as @p dst, that they have both the same size and the @p dst is not
@cpp const @ce.

This overload can also be used for convenient filling of C arrays, which would
otherwise have to be done element-by-element or using @ref std::memcpy() as
neither C nor C++ allows array assignment:

@snippet Utility.cpp Algorithms-copy-C-array
*/
template<class To, class ToView = decltype(Implementation::arrayViewTypeFor(std::declval<To&&>()))
    /* On MSVC's STL, this overload is somehow getting picked for a
       Containers::ArrayView<const void> argument, attempting to materialize
       std::initializer_list<void> and dying with "forming a reference to
       void". Happens with clang-cl as well, so I suspect the STL
       implementation is to blame... but why? There's no catch-all constructor,
       just a two-pointer one. */
    #ifdef CORRADE_TARGET_DINKUMWARE
    , class = typename std::enable_if<!std::is_same<typename std::remove_const<typename ToView::Type>::type, void>::value>::type
    #endif
> void copy(std::initializer_list<typename ToView::Type> src, To&& dst);

/**
@brief Flip given dimension of a view in-place
@m_since_latest

Swaps all items in @p dimension such that @cpp viewDimension[i] @ce and
@cpp viewDimension[dimensionSize - i - 1] @ce exchange their contents for all
@cpp i < dimensionSize/2 @ce. Expects that @p T is a trivially copyable type
and the view is contiguous after @p dimension.

View flip not in place can be performed using a @ref copy() to a view that has
the desired dimension @ref Containers::StridedArrayView::flipped(). In the
following snippet, the first expression does the flip during a copy, while the
second performs it in-place:

@snippet Utility.cpp Algorithms-flipInPlace

@see @ref Containers::StridedArrayView::isContiguous()
*/
template<unsigned dimension, unsigned dimensions, class T> void flipInPlace(const Containers::StridedArrayView<dimensions, T>& view);

namespace Implementation {

template<class> struct ArrayViewType;
template<class T> struct ArrayViewType<Containers::ArrayView<T>> {
    enum: unsigned { Dimensions = 1 };
    typedef Containers::ArrayView<typename std::remove_const<T>::type> Type;
    typedef Containers::ArrayView<const T> ConstType;
};
template<unsigned dimensions, class T> struct ArrayViewType<Containers::StridedArrayView<dimensions, T>> {
    enum: unsigned { Dimensions = dimensions };
    typedef Containers::StridedArrayView<dimensions, typename std::remove_const<T>::type> Type;
    typedef Containers::StridedArrayView<dimensions, const T> ConstType;
};

}

template<class From, class To, class FromView, class ToView> void copy(From&& src, To&& dst) {
    static_assert(std::is_same<typename std::remove_const<typename FromView::Type>::type, typename std::remove_const<typename ToView::Type>::type>::value, "can't copy between views of different types");
    static_assert(!std::is_const<typename ToView::Type>::value, "can't copy to a const view");
    static_assert(unsigned(Implementation::ArrayViewType<FromView>::Dimensions) ==
        unsigned(Implementation::ArrayViewType<ToView>::Dimensions),
        "can't copy between views of different dimensions");
    /* We need to pass const& to the copy(), passing temporary instances
       directly would lead to infinite recursion */
    const typename std::common_type<
        typename Implementation::ArrayViewType<FromView>::ConstType,
        typename Implementation::ArrayViewType<ToView>::ConstType>::type srcV{src};
    const typename std::common_type<
        typename Implementation::ArrayViewType<FromView>::Type,
        typename Implementation::ArrayViewType<ToView>::Type>::type dstV{dst};
    copy(srcV, dstV);
}

template<class To, class ToView
    #ifdef CORRADE_TARGET_DINKUMWARE
    , class
    #endif
> void copy(std::initializer_list<typename ToView::Type> src, To&& dst) {
    static_assert(!std::is_const<typename ToView::Type>::value,
        "can't copy to a const view");
    static_assert(Implementation::ArrayViewType<ToView>::Dimensions == 1,
        "can copy an initializer list only to a 1D view");
    /* We need to pass const& to the copy(), passing temporary instances
       directly would lead to infinite recursion */
    const typename Implementation::ArrayViewType<ToView>::ConstType srcV = Containers::arrayView(src);
    const typename Implementation::ArrayViewType<ToView>::Type dstV{dst};
    copy(srcV, dstV);
}

template<unsigned dimensions> void copy(const Containers::StridedArrayView<dimensions, const char>& src, const Containers::StridedArrayView<dimensions, char>& dst) {
    CORRADE_ASSERT(src.size() == dst.size(),
        "Utility::Algorithms::copy(): sizes" << src.size() << "and" << dst.size() << "don't match", );

    for(std::size_t i = 0, max = src.size()[0]; i != max; ++i)
        copy(src[i], dst[i]);
}

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

CORRADE_UTILITY_EXPORT void flipSecondToLastDimensionInPlace(const Containers::StridedArrayView2D<char>& view);
CORRADE_UTILITY_EXPORT void flipSecondToLastDimensionInPlace(const Containers::StridedArrayView3D<char>& view);
CORRADE_UTILITY_EXPORT void flipSecondToLastDimensionInPlace(const Containers::StridedArrayView4D<char>& view);

template<unsigned dimensions> void flipSecondToLastDimensionInPlace(const Containers::StridedArrayView<dimensions, char>& view) {
    for(const Containers::StridedArrayView<dimensions - 1, char> i: view)
        flipSecondToLastDimensionInPlace(i);
}

}

template<unsigned dimension, unsigned dimensions, class T> void flipInPlace(const Containers::StridedArrayView<dimensions, T>& view) {
    static_assert(dimension < dimensions, "dimension out of range");
    static_assert(
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<T>::value
        #else
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #endif
        , "types must be trivially copyable");

    const Containers::StridedArrayView<dimensions + 1, char> expanded =
        Containers::arrayCast<dimensions + 1, char>(view);
    CORRADE_ASSERT(expanded.template isContiguous<dimension + 1>(),
        "Utility::flipInPlace(): the view is not contiguous after dimension" << dimension, );
    Implementation::flipSecondToLastDimensionInPlace(expanded.template asContiguous<dimension + 1>());
}

}}

#endif
