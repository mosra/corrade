#ifndef Corrade_Utility_Algorithms_h
#define Corrade_Utility_Algorithms_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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

/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Utility {

/**
@brief Copy an array view to another
@m_since{2020,06}

Calls @ref std::memcpy() on the contents. Expects that both arrays have the
same size.
*/
CORRADE_UTILITY_EXPORT void copy(Containers::ArrayView<const void> src, Containers::ArrayView<void> dst);

/**
@brief Copy an array view to another
@m_since{2020,06}

Casts views into a @cpp void @ce type and delegates into
@ref copy(Containers::ArrayView<const void>, Containers::ArrayView<void>).
Expects that @p T is a trivially copyable type.
*/
template<class T> inline void copy(Containers::ArrayView<const T> src, Containers::ArrayView<T> dst) {
    static_assert(
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #else
        std::is_trivially_copyable<T>::value
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

namespace Implementation {
    /* The dimensions argument is only for assertions, but not wrapping that in
       #ifndef CORRADE_NO_ASSERT to avoid ABI mismatches if a project is
       compiled with assertions disabled but Corrade not, and vice versa */
    CORRADE_UTILITY_EXPORT void copy(const Containers::StridedArrayView4D<const char>& src, const Containers::StridedArrayView4D<char>& dst, unsigned dimensions);
}

/**
 * @overload
 * @m_since{2020,06}
 */
inline void copy(const Containers::StridedArrayView1D<const char>& src, const Containers::StridedArrayView1D<char>& dst) {
    return Implementation::copy(
        Containers::StridedArrayView4D<const char>{src},
        Containers::StridedArrayView4D<char>{dst}, 1);
}

/**
 * @overload
 * @m_since{2020,06}
 */
inline void copy(const Containers::StridedArrayView2D<const char>& src, const Containers::StridedArrayView2D<char>& dst) {
    return Implementation::copy(
        Containers::StridedArrayView4D<const char>{src},
        Containers::StridedArrayView4D<char>{dst}, 2);
}

/**
 * @overload
 * @m_since{2020,06}
 */
inline void copy(const Containers::StridedArrayView3D<const char>& src, const Containers::StridedArrayView3D<char>& dst) {
    return Implementation::copy(
        Containers::StridedArrayView4D<const char>{src},
        Containers::StridedArrayView4D<char>{dst}, 3);
}

/**
 * @overload
 * @m_since{2020,06}
 */
inline void copy(const Containers::StridedArrayView4D<const char>& src, const Containers::StridedArrayView4D<char>& dst) {
    return Implementation::copy(
        Containers::StridedArrayView4D<const char>{src},
        Containers::StridedArrayView4D<char>{dst}, 4);
}

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
template<std::size_t size, class T> static Containers::ArrayView<T> arrayViewTypeFor(T(&)[size]);
template<unsigned dimensions, class T> static Containers::StridedArrayView<dimensions, T> arrayViewTypeFor(const Containers::StridedArrayView<dimensions, T>&);

}

/**
@brief Copy an initializer list to a view
@m_since_latest

Based on whether the @p dst is convertible to either @ref Containers::ArrayView
or @ref Containers::StridedArrayView, calls either
@ref copy(Containers::ArrayView<const T>, Containers::ArrayView<T>) or
@ref copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>&).
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
    #ifndef DOXYGEN_GENERATING_OUTPUT
    /* On Clang, MSVC and GCC 5 to 10, this overload is somehow getting picked
       for a Containers::ArrayView<const void> argument, attempting to
       materialize std::initializer_list<void> and dying with "forming a
       reference to void". Doesn't happen on GCC 4.8 and 11+, however GCC 11+
       fails the same way if the source is Containers::ArrayView<void> instead.
       Why? There's no catch-all constructor, just a two-pointer one in case of
       MSVC, and a pointer + size in libc++ / libstdc++.

       Important: keep this expression in sync with the actual implementation
       below, otherwise you'll get linker errors. */
    #if defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 5)
    , class = typename std::enable_if<!std::is_same<typename std::remove_const<typename ToView::Type>::type, void>::value>::type
    #endif
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

/* Hidden from documentation output because it only adds confusion on top of
   the ArrayView and StridedArrayView variants. All it does is magic that
   prefers to delegate to the ArrayView variant if both arguments are
   convertible to it to save on complicated assertions. */
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class From, class To, class FromView = decltype(Implementation::arrayViewTypeFor(std::declval<From&&>())), class ToView = decltype(Implementation::arrayViewTypeFor(std::declval<To&&>()))> void copy(From&& src, To&& dst) {
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
#endif

template<class To, class ToView
    /* Make sure to keep this branch in sync with the declaration above */
    #if defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC) || (defined(CORRADE_TARGET_GCC) && __GNUC__ >= 5)
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
    /* Compared to the contiguous ArrayView copy() this has a full assertion,
       as the expectation is that it's called on large chunks of data where the
       assert overhead doesn't matter that much compared to the safety
       gains. */
    CORRADE_ASSERT(src.size() == dst.size(),
        "Utility::copy(): sizes" << src.size() << "and" << dst.size() << "don't match", );

    for(std::size_t i = 0, max = src.size()[0]; i != max; ++i)
        /* Explicitly pick the final overload to avoid having to go through the
           copy(From&&, To&&) proxy again */
        static_cast<void(*)(const Containers::StridedArrayView<dimensions - 1, const char>&, const Containers::StridedArrayView<dimensions - 1, char>&)>(copy)(src[i], dst[i]);
}

template<unsigned dimensions, class T> void copy(const Containers::StridedArrayView<dimensions, const T>& src, const Containers::StridedArrayView<dimensions, T>& dst) {
    static_assert(
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #else
        std::is_trivially_copyable<T>::value
        #endif
        , "types must be trivially copyable");

    /* Cast the views to char and pass them as const& to avoid having to go
       through the copy(From&&, To&&) proxy again. Can't do a static_cast like
       elsewhere as there's an ambiguity between copy<dimensions>() and copy()
       that's explicitly stamped out for 1D, 2D, 3D and 4D.

       The arrayCast() has full assertions as well -- the expectation here is
       that the StridedArrayView variants are called on large chunks of data
       where the assert overhead doesn't matter that much compared to the
       safety gains. */
    const Containers::StridedArrayView<dimensions + 1, const char> srcChar = Containers::arrayCast<dimensions + 1, const char>(src);
    const Containers::StridedArrayView<dimensions + 1, char> dstChar = Containers::arrayCast<dimensions + 1, char>(dst);
    return copy(srcChar, dstChar);
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
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #else
        std::is_trivially_copyable<T>::value
        #endif
        , "types must be trivially copyable");

    const Containers::StridedArrayView<dimensions + 1, char> expanded =
        Containers::arrayCast<dimensions + 1, char>(view);
    /* Compared to the contiguous ArrayView APIs this has a full assertion, as
       the expectation is that it's called on large chunks of data where the
       assert overhead doesn't matter that much compared to the safety
       gains. */
    CORRADE_ASSERT(expanded.template isContiguous<dimension + 1>(),
        "Utility::flipInPlace(): the view is not contiguous after dimension" << dimension, );
    Implementation::flipSecondToLastDimensionInPlace(expanded.template asContiguous<dimension + 1>());
}

}}

#endif
