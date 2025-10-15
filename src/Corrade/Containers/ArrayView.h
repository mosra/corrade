#ifndef Corrade_Containers_ArrayView_h
#define Corrade_Containers_ArrayView_h
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
 * @brief Class @ref Corrade::Containers::ArrayView, @ref Corrade::Containers::StaticArrayView, alias @ref Corrade::Containers::ArrayView2, @ref Corrade::Containers::ArrayView3, @ref Corrade::Containers::ArrayView4
 */

#include <initializer_list>
/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Move.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#endif

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class> struct ErasedArrayViewConverter;
    #ifndef CORRADE_SINGLES_NO_ARRAYTUPLE_COMPATIBILITY
    /* so ArrayTuple can update the data pointer */
    template<class T>
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* warns that "the inline specifier cannot be used when a friend
           declaration refers to a specialization of a function template" due
           to friend dataRef<>() being used below. AMAZING */
        inline
        #endif
    T*& dataRef(ArrayView<T>& view) {
        return view._data;
    }
    #endif
    #ifndef CORRADE_SINGLES_NO_PYTHON_COMPATIBILITY
    /* so Python buffer protocol can point to the size member */
    template<class T>
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* warns that "the inline specifier cannot be used when a friend
           declaration refers to a specialization of a function template" due
           to friend dataRef<>() being used below. AMAZING */
        inline
        #endif
    std::size_t& sizeRef(ArrayView<T>& view) {
        return view._size;
    }
    #endif
}

/**
@brief Array view

A non-owning wrapper around a sized continuous range of data, similar to a
dynamic @ref std::span from C++20. For a variant with compile-time size
information see @ref StaticArrayView, for sparse and multi-dimensional views
see @ref StridedArrayView, for efficient bit manipulation see
@ref BasicBitArrayView "BitArrayView". An owning version of this container is
an @ref Array.

@section Containers-ArrayView-usage Usage

The class is implicitly convertible from compile-time-sized C arrays and
@ref Array / @ref StaticArray instances of the same underlying type; it's also
possible to implicitly create a @cpp const @ce view on a mutable array. Besides
that, a view can be created manually from a pointer and size:

@snippet Containers.cpp ArrayView-usage

@attention Note that when using @cpp Containers::ArrayView<const char> @ce, C
    string literals (such as @cpp "hello" @ce) are implicitly convertible to it
    and the size includes also the zero-terminator (thus in case of
    @cpp "hello" @ce the size would be 6, not 5, as one might expect). For
    string views you're encouraged to use the @ref BasicStringView "StringView"
    class instead.

In addition there are @ref ArrayView<void> / @ref ArrayView<const void>
specializations, which are meant to be used in cases where APIs accept opaque
blobs of data, such as writing data to a file. These provide no element access,
only data pointer and size in bytes, and an @ref ArrayView of any type is
convertible to them:

@snippet Containers.cpp ArrayView-usage-void

@subsection Containers-ArrayView-usage-access Data access

The class provides the usual C++ container interface --- @ref data(),
@ref size() and @ref isEmpty(); subscript access via @ref operator T*(), range
access via @ref begin() / @ref end(), and their overloads and acess to the
@ref front() and @ref back() element, if the view is non-empty. The view itself
is immutable and thus all member functions are @cpp const @ce, but if the
underlying type is mutable the references and pointers returned from these are
mutable as well.

@snippet Containers.cpp ArrayView-usage-access

@subsection Containers-ArrayView-usage-slicing View slicing

Except for the usual element access via @ref begin(), @ref end() and
@ref operator T*() that provides also access via @cpp [] @ce, there's a
collection of slicing functions --- @ref slice(), @ref sliceSize(),
@ref prefix(), @ref suffix(), @ref exceptPrefix() and @ref exceptSuffix():

@snippet Containers.cpp ArrayView-usage-slicing

The @ref slice(), @ref sliceSize(), @ref prefix(), @ref suffix() APIs accept
also a pointer as the begin/end arguments. As a special case, if @ref prefix()
and @ref suffix() are called with @cpp nullptr @ce, they return a zero-sized
@cpp nullptr @ce view. Both are useful when doing various kinds of searches and
propagating failure states, for example as shown below:

@snippet Containers.cpp ArrayView-usage-slicing2

Finally, the slicing operations provide a conversion to
@ref StaticArrayView (or its convenience typedefs @ref ArrayView2,
@ref ArrayView3 etc.). Compile-time-sized arrays are useful for APIs that want
to enforce a particular number of elements at compile time:

@snippet Containers.cpp ArrayView-usage-slicing3

All slice operations fire an assert if the arguments are out of range, for
@ref StaticArrayView conversions the checks are done at compile time when
possible.

@subsection Containers-ArrayView-usage-convenience Convenience functions and type conversion

To avoid having to specify the full type when constructing a view or when you
need to disambiguate in a function call, the @ref arrayView() (or
@ref staticArrayView()) function can be used. These also provide a safe
conversion from @ref std::initializer_list that avoids the
@ref Containers-ArrayView-initializer-list "pitfalls described below"  As a
safer alternative to a @cpp reinterpret_cast @ce, @ref arrayCast() performs a
cast, recalculates the view size accordingly and additionally checks that the
type change makes sense for given type combination and array size:

@snippet Containers.cpp arrayCast

Finally, @ref arraySize() can be used to query size of compile-time C arrays in
a failproof way:

@snippet Containers.cpp arraySize

@section Containers-ArrayView-stl STL compatibility

Instances of @ref ArrayView and @ref StaticArrayView are convertible from
@ref std::vector and @ref std::array if you include
@ref Corrade/Containers/ArrayViewStl.h. The conversion is provided in a
separate header to avoid unconditional @cpp #include <vector> @ce or
@cpp #include <array> @ce, which significantly affect compile times.
Additionally, the @ref arrayView(T&&) overload also allows for such a
conversion. The following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref ArrayView "ArrayView<T>"   | ← | @ref std::array "std::array<T, size>"
@ref ArrayView<void>            | ← | @ref std::array "std::array<T, size>"
@ref ArrayView "ArrayView<const T>" | ← | @ref std::array "const std::array<T, size>"
@ref ArrayView<const void>      | ← | @ref std::array "const std::array<T, size>"
@ref ArrayView "ArrayView<T>"   | ← | @ref std::vector "std::vector<T, Allocator>"
@ref ArrayView<void>            | ← | @ref std::vector "std::vector<T, Allocator>"
@ref ArrayView "ArrayView<const T>" | ← | @ref std::vector "const std::vector<T, Allocator>"
@ref ArrayView<const void>      | ← | @ref std::vector "const std::vector<T, Allocator>"
@ref StaticArrayView "StaticArrayView<size, T>" | ← | @ref std::array "std::array<T, size>"
@ref StaticArrayView "StaticArrayView<size, const T>" | ← | @ref std::array "const std::array<T, size>"

Example:

@snippet Containers-stl.cpp ArrayView

On compilers that support C++2a and @ref std::span, implicit conversion from
and also to it is provided in @ref Corrade/Containers/ArrayViewStlSpan.h. For
similar reasons, it's a dedicated header to avoid unconditional
@cpp #include <span> @ce, but this one is even significantly heavier than the
@ref vector "<vector>" etc. includes, so it's separate from the others as well.
The following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref ArrayView "ArrayView<T>"   | ⇆ | @ref std::span "std::span<T>"
@ref ArrayView "ArrayView<T>"   | ← | @ref std::span "std::span<T, size>"
@ref ArrayView<void>            | ← | @ref std::span "std::span<T>"
@ref ArrayView<void>            | ← | @ref std::span "std::span<T, size>"
@ref ArrayView<const void>      | ← | @ref std::span "std::span<T>"
@ref ArrayView<const void>      | ← | @ref std::span "std::span<T, size>"
@ref StaticArrayView "StaticArrayView<size, T>" | ⇆ | @ref std::span "std::span<T, size>"
@ref StaticArrayView "StaticArrayView<size, T>" | → | @ref std::span "std::span<T>"

Example:

@snippet Containers-stl2a.cpp ArrayView

In all cases above, it's also possible to create an @ref ArrayView "ArrayView<T>"
instance from a @ref std::array, @ref std::vector or @ref std::span of a type
derived from `T` if the types have the same size. This basically just expands
the @ref ArrayView(ArrayView<U>) constructor functionality to external types as
well.

@anchor Containers-ArrayView-initializer-list

<b></b>

@m_class{m-block m-warning}

@par Conversion from std::initializer_list
    The class deliberately *doesn't* provide a @ref std::initializer_list
    constructor, as it would lead to dangerous behavior even in very simple and
    seemingly innocent cases --- see the snippet below. Instead, where it makes
    sense, functions accepting @ref ArrayView provide also an overload taking
    @ref std::initializer_list. Alternatively, you can use
    @ref arrayView(std::initializer_list<T>), which is more explicit and thus
    should prevent accidental use.
@par
    @code{.cpp}
    std::initializer_list<int> a{1, 2, 3, 4};
    foo(a[2]);  // okay

    Containers::ArrayView<const int> b{1, 2, 3, 4}; // hypothetical, doesn't compile
    foo(b[2]);  // crash, initializer_list already destructed here
    @endcode

Other array classes provide a subset of this STL compatibility as well, see the
documentation of @ref Containers-StaticArrayView-stl "StaticArrayView",
@ref Containers-Array-stl "Array", @ref Containers-StaticArray-stl "StaticArray"
and @ref Containers-StridedArrayView-stl "StridedArrayView" for more
information.

@anchor Containers-ArrayView-single-header

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with the @ref ArrayView<void> /
    @ref ArrayView<const void> specializations and @ref StaticArrayView, is
    also available as a single-header, dependency-less [CorradeArrayView.h](https://github.com/mosra/magnum-singles/tree/master/CorradeArrayView.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.
@par
    Structured bindings for @ref StaticArrayView on C++17 are opt-in due to
    reliance on a potentially heavy STL header ---
    @cpp #define CORRADE_STRUCTURED_BINDINGS @ce before including the file. The
    above-mentioned STL compatibility bits are included as well. Enable them
    for @ref std::vector and @ref std::array by specifying
    @cpp #define CORRADE_ARRAYVIEW_STL_COMPATIBILITY @ce and for @ref std::span
    by compiling as C++2a and specifying
    @cpp #define CORRADE_ARRAYVIEW_STL_SPAN_COMPATIBILITY @ce before including
    the file. Including it multiple times with different macros defined works
    as well.
*/
/* All member functions are const because the view doesn't own the data */
template<class T> class ArrayView {
    public:
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Default constructor
         *
         * Creates an empty @cpp nullptr @ce view. Copy a non-empty @ref Array
         * or @ref ArrayView onto the instance to make it useful.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ ArrayView(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either an ArrayView or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> constexpr /*implicit*/ ArrayView(U) noexcept: _data{}, _size{} {}

        constexpr /*implicit*/ ArrayView() noexcept: _data{}, _size{} {}
        #endif

        /**
         * @brief Construct a view on an array with explicit size
         * @param data      Data pointer
         * @param size      Data size
         *
         * @see @ref arrayView(T*, std::size_t)
         */
        constexpr /*implicit*/ ArrayView(T* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Construct a view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref arrayView(T(&)[size])
         */
        template<class U, std::size_t size
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(U(&data)[size]) noexcept: _data{data}, _size{size} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on an @ref ArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(ArrayView<U> view) noexcept: _data{view}, _size{view.size()} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on a @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref arrayView(StaticArrayView<size, T>)
         */
        template<std::size_t size, class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(StaticArrayView<size, U> view) noexcept: _data{view}, _size{size} {
            static_assert(sizeof(U) == sizeof(T), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. Besides that, to simplify the implementation,
           there's no const-adding conversion. Instead, the implementer is
           supposed to add an ArrayViewConverter variant for that. */
        template<class U, class = decltype(Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ ArrayView(U&& other) noexcept: ArrayView{Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(Utility::forward<U>(other))} {}

        /**
         * @brief Convert the view to external representation
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* To simplify the implementation, there's no const-adding conversion.
           Instead, the implementer is supposed to add an ArrayViewConverter
           variant for that. */
        template<class U, class = decltype(Implementation::ArrayViewConverter<T, U>::to(std::declval<ArrayView<T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::ArrayViewConverter<T, U>::to(*this);
        }

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the view is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to the underlying type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief View data */
        constexpr T* data() const { return _data; }

        /**
         * @brief View size
         *
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const { return _size; }

        /**
         * @brief Whether the view is empty
         * @m_since_latest
         *
         * @see @ref size()
         */
        constexpr bool isEmpty() const { return !_size; }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief isEmpty()
         * @m_deprecated_since_latest Use @ref isEmpty() instead.
         */
        CORRADE_DEPRECATED("use isEmpty() instead") constexpr bool empty() const { return !_size; }
        #endif

        /**
         * @brief Pointer to the first element
         *
         * @see @ref front(), @ref operator[]()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) the last element
         *
         * @see @ref back(), @ref operator[]()
         */
        constexpr T* end() const { return _data+_size; }
        constexpr T* cend() const { return _data+_size; } /**< @overload */

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        constexpr T& front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        constexpr T& back() const;

        /**
         * @brief Element access
         * @m_since_latest
         *
         * Expects that @p i is less than @ref size().
         * @see @ref front(), @ref back()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr T& operator[](std::size_t i) const;
        #else
        /* Has to be done this way because otherwise it causes ambiguity with a
           builtin operator[] for pointers if an int or ssize_t is used due to
           the implicit pointer conversion. Sigh. */
        /** @todo clean up once implicit pointer conversion is removed */
        template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr T& operator[](U i) const;
        #endif

        /**
         * @brief View slice
         *
         * Both arguments are expected to be in range.
         * @see @ref sliceSize(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix(),
         *      @ref slice(std::size_t, std::size_t) const
         */
        /* Unlike sliceSize(T*, size_t), slice<size>(T*), prefix(T*) and
           suffix(T*) this doesn't have ambiguity prevention for slice(0, 0)
           as such use case is rather rare I think. */
        constexpr ArrayView<T> slice(T* begin, T* end) const;

        /** @overload */
        constexpr ArrayView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief View slice of given size
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice(begin, begin + size) @ce.
         * @see @ref slice(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix(),
         *      @ref sliceSize(std::size_t, std::size_t) const
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<T> sliceSize(T* begin, std::size_t size) const;
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<T> sliceSize(U begin, std::size_t size) const {
            return slice(begin, begin + size);
        }
        #endif

        /**
         * @overload
         * @m_since_latest
         */
        constexpr ArrayView<T> sliceSize(std::size_t begin, std::size_t size) const {
            return slice(begin, begin + size);
        }

        /**
         * @brief Fixed-size view slice
         *
         * Both @p begin and @cpp begin + size_ @ce are expected to be in
         * range.
         * @see @ref prefix() const
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size_> constexpr StaticArrayView<size_, T> slice(T* begin) const;
        #else
        /* To avoid ambiguity when calling slice<size>(0). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<std::size_t size_, class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr StaticArrayView<size_, T> slice(U begin) const;
        #endif

        /** @overload */
        template<std::size_t size_> constexpr StaticArrayView<size_, T> slice(std::size_t begin) const;

        /**
         * @brief Fixed-size view slice
         * @m_since{2019,10}
         *
         * At compile time expects that @cpp begin < end_ @ce, at runtime that
         * @p end_ is not larger than @ref size().
         * @see @ref sliceSize(), @ref slice(T*) const, @ref prefix() const
         */
        template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> slice() const;

        /**
         * @brief Fixed-size view slice of given size
         * @m_since_latest
         *
         * Expects that `begin_ + size_` is not larger than @ref size().
         * @see @ref slice() const
         */
        template<std::size_t begin_, std::size_t size_> constexpr StaticArrayView<size_, T> sliceSize() const {
            return slice<begin_, begin_ + size_>();
        }

        /**
         * @brief View prefix until a pointer
         *
         * Equivalent to @cpp data.slice(data.begin(), end) @ce. If @p end is
         * @cpp nullptr @ce, returns zero-sized @cpp nullptr @ce array.
         * @see @ref slice(T*, T*) const, @ref sliceSize(T*, std::size_t) const,
         *      @ref suffix(T*) const, @ref prefix(std::size_t) const
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<T> prefix(T* end) const;
        #else
        /* To avoid ambiguity when calling prefix(0). FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<T> prefix(U end) const {
            return static_cast<T*>(end) ? slice(_data, end) : nullptr;
        }
        #endif

        /**
         * @brief View suffix after a pointer
         *
         * Equivalent to @cpp data.slice(begin, data.end()) @ce. If @p begin is
         * @cpp nullptr @ce and the original array isn't, returns zero-sized
         * @cpp nullptr @ce array.
         * @see @ref slice(T*, T*) const, @ref sliceSize(T*, std::size_t) const,
         *      @ref prefix(T*) const
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        constexpr ArrayView<T> suffix(T* begin) const {
            return _data && !begin ? nullptr : slice(begin, _data + _size);
        }

        /**
         * @brief View on the first @p size items
         *
         * Equivalent to @cpp data.slice(0, size) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref exceptPrefix(), @ref prefix(T*) const
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         */
        constexpr ArrayView<T> prefix(std::size_t size) const {
            return slice(0, size);
        }

        /* Here will be suffix(std::size_t size), view on the last size
           items, once the deprecated suffix(std::size_t begin) is gone and
           enough time passes to not cause silent breakages in existing code.
           The fixed-size suffix<size_>() below could be added already as it
           doesn't clash with anything. */

        /**
         * @brief Fixed-size view on the first @p size_ items
         *
         * Equivalent to @cpp data.slice<0, size_>() @ce.
         * @see @ref slice() const, @ref sliceSize() const, @ref suffix() const
         */
        template<std::size_t size_> constexpr StaticArrayView<size_, T> prefix() const {
            return slice<0, size_>();
        }

        /**
         * @brief Fixed-size view on the last @p size_ items
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice<size_>(data.size() - size_) @ce.
         * @see @ref slice() const, @ref sliceSize() const, @ref prefix() const
         */
        template<std::size_t size_> constexpr StaticArrayView<size_, T> suffix() const {
            return slice<size_>(_size - size_);
        }

        /**
         * @brief View except the first @p size items
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice(size, data.size()) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref prefix(std::size_t) const, @ref exceptSuffix()
         */
        constexpr ArrayView<T> exceptPrefix(std::size_t size) const {
            return slice(size, _size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") constexpr ArrayView<T> suffix(std::size_t begin) const {
            return slice(begin, _size);
        }
        #endif

        /**
         * @brief View except the last @p size items
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice(0, data.size() - size) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref exceptPrefix()
         * @todoc link to suffix(std::size_t) once it takes size and not begin
         */
        constexpr ArrayView<T> exceptSuffix(std::size_t size) const {
            return slice(0, _size - size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") constexpr ArrayView<T> except(std::size_t count) const {
            return slice(0, _size - count);
        }
        #endif

    private:
        #ifndef CORRADE_SINGLES_NO_ARRAYTUPLE_COMPATIBILITY
        /* So ArrayTuple can update the data pointer */
        friend T*& Implementation::dataRef<>(ArrayView<T>&);
        #endif
        #ifndef CORRADE_SINGLES_NO_PYTHON_COMPATIBILITY
        /* So Python buffer protocol can point to the size member */
        friend std::size_t& Implementation::sizeRef<>(ArrayView<T>&);
        #endif
        T* _data;
        std::size_t _size;
};

/**
@brief Void array view
@m_since{2019,10}

Specialization of @ref ArrayView which is convertible from a compile-time
array, @ref Array, @ref ArrayView or @ref StaticArrayView of any non-constant
type and also any non-constant type convertible to them. Size for a particular
type is recalculated to a size in bytes. This specialization doesn't provide
any accessors besides @ref data(), because it has no use for the @cpp void @ce
type. Instead, use @ref arrayCast(ArrayView<void>) to first cast the array to a
concrete type and then access the particular elements. See
@ref Containers-ArrayView-usage "ArrayView class docs" for usage examples.

@m_class{m-block m-success}

@par Single-header version
    This specialization, together with @ref ArrayView<const void> and the
    generic @ref ArrayView and @ref StaticArrayView, is also available as a
    single-header, dependency-less library in the Magnum Singles repository for
    easier integration into your projects. See the
    @ref Containers-ArrayView-single-header "ArrayView documentation" for more
    information.
*/
template<> class ArrayView<void> {
    public:
        typedef void Type;      /**< @brief Element type */

        /**
         * @brief Default constructor
         *
         * Creates an empty @cpp nullptr @ce view. Copy a non-empty @ref Array
         * or @ref ArrayView onto the instance to make it useful.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ ArrayView(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either an ArrayView or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> constexpr /*implicit*/ ArrayView(U) noexcept: _data{}, _size{} {}
        constexpr /*implicit*/ ArrayView() noexcept: _data{}, _size{} {}
        #endif

        /**
         * @brief Construct a view on a type-erased array with explicit length
         * @param data      Data pointer
         * @param size      Data size in bytes
         */
        constexpr /*implicit*/ ArrayView(void* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Construct a view on a typed array with explicit length
         * @param data      Data pointer
         * @param size      Data size
         *
         * Size is recalculated to size in bytes.
         */
        template<class T> constexpr /*implicit*/ ArrayView(T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct a view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        template<class T, std::size_t size
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<!std::is_const<T>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}

        /** @brief Construct a void view on any @ref ArrayView */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<!std::is_const<T>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(ArrayView<T> array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct a void view on any @ref StaticArrayView */
        template<std::size_t size, class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<!std::is_const<T>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(const StaticArrayView<size, T>& array) noexcept: _data{array}, _size{size*sizeof(T)} {}

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. */
        template<class T, class U = decltype(Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(std::declval<T&&>()))
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<!std::is_const<typename U::Type>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ ArrayView(T&& other) noexcept: ArrayView{Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(other)} {}

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the view is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to the underlying type */
        constexpr /*implicit*/ operator void*() const { return _data; }

        /** @brief View data */
        constexpr void* data() const { return _data; }

        /**
         * @brief View size
         *
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const { return _size; }

        /**
         * @brief Whether the view is empty
         * @m_since_latest
         *
         * @see @ref size()
         */
        constexpr bool isEmpty() const { return !_size; }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief isEmpty()
         * @m_deprecated_since_latest Use @ref isEmpty() instead.
         */
        CORRADE_DEPRECATED("use isEmpty() instead") constexpr bool empty() const { return !_size; }
        #endif

    private:
        void* _data;
        std::size_t _size;
};

/**
@brief Constant void array view

Specialization of @ref ArrayView which is convertible from a compile-time
array, @ref Array, @ref ArrayView or @ref StaticArrayView of any type and also
any type convertible to them. Size for a particular type is recalculated to
a size in bytes. This specialization doesn't provide any accessors besides
@ref data(), because it has no use for the @cpp void @ce type. Instead, use
@ref arrayCast(ArrayView<const void>) to first cast the array to a concrete
type and then access the particular elements.. See
@ref Containers-ArrayView-usage "ArrayView class docs" for usage examples.

@m_class{m-block m-success}

@par Single-header version
    This specialization, together with @ref ArrayView<void> and the generic
    @ref ArrayView and @ref StaticArrayView, is also available as a
    single-header, dependency-less library in the Magnum Singles repository
    for easier integration into your projects. See the
    @ref Containers-ArrayView-single-header "ArrayView documentation" for more
    information.
*/
template<> class ArrayView<const void> {
    public:
        typedef const void Type;     /**< @brief Element type */

        /**
         * @brief Default constructor
         *
         * Creates an empty @cpp nullptr @ce view. Copy a non-empty @ref Array
         * or @ref ArrayView onto the instance to make it useful.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ ArrayView(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either an ArrayView or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> constexpr /*implicit*/ ArrayView(U) noexcept: _data{}, _size{} {}
        constexpr /*implicit*/ ArrayView() noexcept: _data{}, _size{} {}
        #endif

        /**
         * @brief Construct a view on a type-erased array array with explicit length
         * @param data      Data pointer
         * @param size      Data size in bytes
         */
        constexpr /*implicit*/ ArrayView(const void* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Constructor a view on a typed array with explicit length
         * @param data      Data pointer
         * @param size      Data size
         *
         * Size is recalculated to size in bytes.
         */
        template<class T> constexpr /*implicit*/ ArrayView(const T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct a view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        template<class T, std::size_t size> constexpr /*implicit*/ ArrayView(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}

        /** @brief Construct a const void view on an @ref ArrayView<void> */
        constexpr /*implicit*/ ArrayView(ArrayView<void> array) noexcept: _data{array}, _size{array.size()} {}

        /** @brief Construct a const void view on any @ref ArrayView */
        template<class T> constexpr /*implicit*/ ArrayView(ArrayView<T> array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct a const void view on any @ref StaticArrayView */
        template<std::size_t size, class T> constexpr /*implicit*/ ArrayView(const StaticArrayView<size, T>& array) noexcept: _data{array}, _size{size*sizeof(T)} {}

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. */
        template<class T, class = decltype(Implementation::ErasedArrayViewConverter<const T>::from(std::declval<const T&>()))> constexpr /*implicit*/ ArrayView(const T& other) noexcept: ArrayView{Implementation::ErasedArrayViewConverter<const T>::from(other)} {}

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the view is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to the underlying type */
        constexpr /*implicit*/ operator const void*() const { return _data; }

        /** @brief View data */
        constexpr const void* data() const { return _data; }

        /**
         * @brief View size
         *
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const { return _size; }

        /**
         * @brief Whether the view is empty
         * @m_since_latest
         *
         * @see @ref size()
         */
        constexpr bool isEmpty() const { return !_size; }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief isEmpty()
         * @m_deprecated_since_latest Use @ref isEmpty() instead.
         */
        CORRADE_DEPRECATED("use isEmpty() instead") constexpr bool empty() const { return !_size; }
        #endif

    private:
        const void* _data;
        std::size_t _size;
};

/** @relatesalso ArrayView
@brief Make a view on an array of specific length

Convenience alternative to @ref ArrayView::ArrayView(T*, std::size_t). The
following two lines are equivalent:

@snippet Containers.cpp arrayView
*/
template<class T> constexpr ArrayView<T> arrayView(T* data, std::size_t size) {
    return ArrayView<T>{data, size};
}

/** @relatesalso ArrayView
@brief Make a view on fixed-size array

Convenience alternative to @ref ArrayView::ArrayView(U(&)[size]). The following
two lines are equivalent:

@snippet Containers.cpp arrayView-array
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(T(&data)[size]) {
    return ArrayView<T>{data};
}

/** @relatesalso ArrayView
@brief Make a view on an initializer list
@m_since{2020,06}

Not present as a constructor in order to avoid accidental dangling references
with r-value initializer lists. See the
@ref Containers-ArrayView-initializer-list "class documentation" for more
information.
*/
template<class T> ArrayView<const T> arrayView(std::initializer_list<T> list) {
    return ArrayView<const T>{list.begin(), list.size()};
}

/** @relatesalso ArrayView
@brief Make a view on @ref StaticArrayView

Convenience alternative to @ref ArrayView::ArrayView(StaticArrayView<size, U>).
The following two lines are equivalent:

@snippet Containers.cpp arrayView-StaticArrayView
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(StaticArrayView<size, T> view) {
    return ArrayView<T>{view};
}

/** @relatesalso ArrayView
@brief Make a view on a view

Equivalent to the implicit @ref ArrayView copy constructor --- it shouldn't be
an error to call @ref arrayView() on itself.
*/
template<class T> constexpr ArrayView<T> arrayView(ArrayView<T> view) {
    return view;
}

/** @relatesalso ArrayView
@brief Make a view on an external type / from an external representation

@see @ref Containers-ArrayView-stl
*/
/* There's no restriction that would disallow creating ArrayView from
   e.g. std::vector<T>&& because that would break uses like `consume(foo());`,
   where `consume()` expects a view but `foo()` returns a std::vector. */
template<class T, class U = decltype(Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>()))> constexpr U arrayView(T&& other) {
    return Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(Utility::forward<T>(other));
}

/** @relatesalso ArrayView
@brief Reinterpret-cast an array view

Size of the new array is calculated as @cpp view.size()*sizeof(T)/sizeof(U) @ce.
Expects that both types are [standard layout](https://en.cppreference.com/w/cpp/named_req/StandardLayoutType.html)
and the total byte size doesn't change. Example usage:

@snippet Containers.cpp arrayCast
*/
template<class U, class T> ArrayView<U> arrayCast(ArrayView<T> view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    const std::size_t size = view.size()*sizeof(T)/sizeof(U);
    /* Unlike slice() etc, this is usually not called in tight loops and should
       be as checked as possible, so it's not a debug assert */
    CORRADE_ASSERT(size*sizeof(U) == view.size()*sizeof(T),
        "Containers::arrayCast(): can't reinterpret" << view.size() << sizeof(T) << Utility::Debug::nospace << "-byte items into a" << sizeof(U) << Utility::Debug::nospace << "-byte type", {});
    return {reinterpret_cast<U*>(view.begin()), size};
}

/** @relatesalso ArrayView
@brief Reinterpret-cast a void array view
@m_since{2020,06}

Size of the new array is calculated as @cpp view.size()/sizeof(U) @ce.
Expects that the target type is [standard layout](https://en.cppreference.com/w/cpp/named_req/StandardLayoutType.html)
and the total byte size doesn't change.
*/
template<class U> ArrayView<U> arrayCast(ArrayView<const void> view) {
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    const std::size_t size = view.size()/sizeof(U);
    /* Unlike slice() etc, this is usually not called in tight loops and should
       be as checked as possible, so it's not a debug assert */
    CORRADE_ASSERT(size*sizeof(U) == view.size(),
        "Containers::arrayCast(): can't reinterpret" << view.size() << "bytes into a" << sizeof(U) << Utility::Debug::nospace << "-byte type", {});
    return {reinterpret_cast<U*>(view.data()), size};
}

/** @relatesalso ArrayView
@overload
@m_since{2020,06}
*/
template<class U> ArrayView<U> arrayCast(ArrayView<void> view) {
    auto out = arrayCast<const U>(ArrayView<const void>{view});
    return ArrayView<U>{const_cast<U*>(out.data()), out.size()};
}

/** @relatesalso ArrayView
@brief Array view size

Alias to @ref ArrayView::size(). See also @ref arraySize(T(&)[size_]) for
querying size of a C array and @ref arraySize(U(T::*)[size_]) for querying size
of a C array member.
*/
template<class T> constexpr std::size_t arraySize(ArrayView<T> view) {
    return view.size();
}

/** @overload */
template<std::size_t size_, class T> constexpr std::size_t arraySize(StaticArrayView<size_, T>) {
    return size_;
}

/**
@brief C array size

Equivalent to @ref std::size() from C++17. See also @ref arraySize(U(T::*)[size_])
for querying size of an array member.

@snippet Containers.cpp arraySize
*/
template<std::size_t size_, class T> constexpr std::size_t arraySize(T(&)[size_]) {
    return size_;
}

/**
@brief C array member size
@m_since_latest

Variant of @ref arraySize(T(&)[size_]) that works on array members. Note that
you have to form a pointer to a member with @cpp & @ce for this to work:

@snippet Containers.cpp arraySize-member
*/
template<std::size_t size_, class T, class U> constexpr std::size_t arraySize(U(T::*)[size_]) {
    return size_;
}

/**
@brief Compile-time-sized array view

Like @ref ArrayView, but with compile-time size information. Similar to a
fixed-size @ref std::span from C++2a. Implicitly convertible to an
@ref ArrayView, explicitly convertible from it using the slicing APIs. An
owning version of this container is a @ref StaticArray.

@section Containers-StaticArrayView-usage Usage

The general API is similar to what's shown in
@ref Containers-ArrayView-usage "ArrayView usage docs", except that here are
additional compile-time overloads of @ref slice(), @ref sliceSize(),
@ref prefix(), @ref suffix(), @ref exceptPrefix() and @ref exceptSuffix().

@snippet Containers.cpp StaticArrayView-usage

@section Containers-StaticArrayView-structured-bindings C++17 structured bindings

If @ref Corrade/Containers/StructuredBindings.h is included, the class can be
used in C++17 structured bindings. While the @cpp get<i>() @ce overloads are
defined inside @ref StaticArrayView itself, a separate header is used for the
@m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
template specializations, as those may require @cpp #include <utility> @ce on
some STL implementations. Example:

@snippet Containers-cpp17.cpp StaticArrayView-structured-bindings

@section Containers-StaticArrayView-stl STL compatibility

See @ref Containers-ArrayView-stl "ArrayView STL compatibility" for more
information.

@m_class{m-block m-success}

@par Single-header version
    This class is, together with @ref ArrayView and the @ref ArrayView<void> /
    @ref ArrayView<const void> specializations, is also available as a
    single-header, dependency-less library in the Magnum Singles repository for
    easier integration into your projects. See the
    @ref Containers-ArrayView-single-header "ArrayView documentation" for more
    information.

@see @ref staticArrayView(), @ref arrayCast(StaticArrayView<size, T>),
    @ref ArrayView2, @ref ArrayView3, @ref ArrayView4
*/
/* Underscore at the end to avoid conflict with member size(). It's ugly, but
   having count instead of size_ would make the naming horribly inconsistent. */
template<std::size_t size_, class T> class StaticArrayView {
    public:
        typedef T Type;     /**< @brief Element type */

        enum: std::size_t {
            Size = size_    /**< Array view size */
        };

        /**
         * @brief Default constructor
         *
         * Creates a @cpp nullptr @ce view. Copy a non-empty @ref StaticArray
         * or @ref StaticArrayView onto the instance to make it useful.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ StaticArrayView(std::nullptr_t = nullptr) noexcept;
        #else
        /* To avoid ambiguity in certain cases of passing 0 to overloads that
           take either a StaticArrayView or std::size_t. See the
           constructZeroNullPointerAmbiguity() test for more info. FFS, zero as
           null pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, class = U, typename std::enable_if<std::is_same<std::nullptr_t, U>::value, int>::type = 0> constexpr /*implicit*/ StaticArrayView(U) noexcept: _data{} {}
        constexpr /*implicit*/ StaticArrayView() noexcept: _data{} {}
        #endif

        /**
         * @brief Construct a static view on an array
         * @param data      Data pointer
         *
         * The pointer is assumed to contain at least @ref Size elements, but
         * it can't be checked in any way. Use @ref StaticArrayView(U(&)[size_])
         * or fixed-size slicing from an @ref ArrayView / @ref Array for more
         * safety.
         * @see @ref staticArrayView(T*)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr explicit StaticArrayView(T* data)
        #else
        template<class U, typename std::enable_if<std::is_pointer<U>::value && !std::is_same<U, T(&)[size_]>::value, int>::type = 0> constexpr explicit StaticArrayView(U data)
        #endif
        noexcept: _data(data) {}

        /**
         * @brief Construct a static view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref staticArrayView(T(&)[size])
         */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ StaticArrayView(U(&data)[size_]) noexcept: _data{data} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a static view on an @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_convertible<U*, T*>::value, int>::type = 0
            #endif
        > constexpr /*implicit*/ StaticArrayView(StaticArrayView<size_, U> view) noexcept: _data{view} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on an external type / from an external representation
         *
         * @see @ref Containers-StaticArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. Besides that, to simplify the implementation,
           there's no const-adding conversion. Instead, the implementer is
           supposed to add a StaticArrayViewConverter variant for that. */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ StaticArrayView(U&& other) noexcept: StaticArrayView{Implementation::StaticArrayViewConverter<size_, T, typename std::decay<U&&>::type>::from(Utility::forward<U>(other))} {}

        /**
         * @brief Convert the view to external representation
         *
         * @see @ref Containers-StaticArrayView-stl
         */
        /* To simplify the implementation, there's no ArrayViewConverter
           overload. Instead, the implementer is supposed to extend
           StaticArrayViewConverter specializations for the non-static arrays
           as well. The same goes for const-adding conversions. */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, T, U>::to(std::declval<StaticArrayView<size_, T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::StaticArrayViewConverter<size_, T, U>::to(*this);
        }

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the view is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to the underlying type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief View data */
        constexpr T* data() const { return _data; }

        /**
         * @brief View size
         *
         * @see @ref isEmpty()
         */
        constexpr std::size_t size() const { return size_; }

        /**
         * @brief Whether the view is empty
         * @m_since_latest
         *
         * @see @ref size()
         */
        constexpr bool isEmpty() const { return !size_; }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief isEmpty()
         * @m_deprecated_since_latest Use @ref isEmpty() instead.
         */
        CORRADE_DEPRECATED("use isEmpty() instead") constexpr bool empty() const { return !size_; }
        #endif

        /**
         * @brief Pointer to the first element
         *
         * @see @ref front(), @ref operator[]()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) the last element
         *
         * @see @ref back(), @ref operator[]()
         */
        constexpr T* end() const { return _data + size_; }
        constexpr T* cend() const { return _data + size_; } /**< @overload */

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref isEmpty(), @ref begin(), @ref operator[]()
         */
        constexpr T& front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref isEmpty(), @ref end(), @ref operator[]()
         */
        constexpr T& back() const;

        /**
         * @brief Element access
         * @m_since_latest
         *
         * Expects that @p i is less than @ref size().
         * @see @ref front(), @ref back()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr T& operator[](std::size_t i) const;
        #else
        /* Has to be done this way because otherwise it causes ambiguity with a
           builtin operator[] for pointers if an int or ssize_t is used due to
           the implicit pointer conversion. Sigh. */
        /** @todo clean up once implicit pointer conversion is removed */
        template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr T& operator[](U i) const;
        #endif

        /** @copydoc ArrayView::slice(T*, T*) const */
        /* Unlike sliceSize(T*, size_t), slice<size>(T*), prefix(T*) and
           suffix(T*) this doesn't have ambiguity prevention for slice(0, 0)
           as such use case is rather rare I think */
        constexpr ArrayView<T> slice(T* begin, T* end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        constexpr ArrayView<T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }

        /** @copydoc ArrayView::sliceSize(T*, std::size_t) const */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<T> sliceSize(T* begin, std::size_t size) const;
        #else
        /* To avoid ambiguity when calling sliceSize(0, ...). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<T> sliceSize(U begin, std::size_t size) const {
            return ArrayView<T>(*this).sliceSize(begin, size);
        }
        #endif

        /**
         * @overload
         * @m_since_latest
         */
        constexpr ArrayView<T> sliceSize(std::size_t begin, std::size_t size) const {
            return ArrayView<T>(*this).sliceSize(begin, size);
        }

        /** @copydoc ArrayView::slice(T*) const */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size__> constexpr StaticArrayView<size__, T> slice(T* begin) const;
        #else
        /* To avoid ambiguity when calling slice<size>(0). FFS, zero as null
           pointer was deprecated in C++11 already, why is this still a
           problem?! */
        template<std::size_t size__, class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr StaticArrayView<size__, T> slice(U begin) const {
            return ArrayView<T>(*this).template slice<size__>(begin);
        }
        #endif

        /** @overload */
        template<std::size_t size__> constexpr StaticArrayView<size__, T> slice(std::size_t begin) const {
            return ArrayView<T>(*this).template slice<size__>(begin);
        }

        /**
         * @brief Fixed-size view slice
         * @m_since{2019,10}
         *
         * Expects (at compile time) that @cpp begin_ < end_ @ce and @p end_ is
         * not larger than @ref Size.
         */
        template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> slice() const;

        /**
         * @brief Fixed-size view slice of given size
         * @m_since_latest
         *
         * Expects (at compile time) that @cpp begin_ + size_ @ce is not larger
         * than @ref Size.
         */
        template<std::size_t begin_, std::size_t size__> constexpr StaticArrayView<size__, T> sliceSize() const {
            return slice<begin_, begin_ + size__>();
        }

        /** @copydoc ArrayView::prefix(T*) const */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr ArrayView<T> prefix(T* end) const;
        #else
        /* To avoid ambiguity when calling prefix(0). FFS, zero as null pointer
           was deprecated in C++11 already, why is this still a problem?! */
        template<class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type = 0> constexpr ArrayView<T> prefix(U end) const {
            return ArrayView<T>(*this).prefix(end);
        }
        #endif

        /**
         * @copydoc ArrayView::suffix(T*) const
         * @todo once non-deprecated suffix(std::size_t size) is a thing, add
         *      the ambiguity-preventing template here as well
         */
        constexpr ArrayView<T> suffix(T* begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }

        /** @copydoc ArrayView::prefix(std::size_t) const */
        constexpr ArrayView<T> prefix(std::size_t size) const {
            return ArrayView<T>(*this).prefix(size);
        }

        /* Here will be suffix(std::size_t size), view on the last size
           items, once the deprecated suffix(std::size_t begin) is gone and
           enough time passes to not cause silent breakages in existing code. */

        /**
         * @brief Fixed-size view on the first @p size_ items
         *
         * Equivalent to @cpp data.slice<0, size_>() @ce.
         * @see @ref slice() const
         * @todoc link to suffix() const once it takes size and not begin
         */
        template<std::size_t size__> constexpr StaticArrayView<size__, T> prefix() const {
            return slice<0, size__>();
        }

        /* Here will be suffix<size__>(), view on the last size__ items, once
           the deprecated suffix<begin_>() is gone and enough time passes to
           not cause silent breakages in existing code. */

        /** @copydoc ArrayView::exceptPrefix(std::size_t) const */
        constexpr ArrayView<T> exceptPrefix(std::size_t size__) const {
            return ArrayView<T>(*this).exceptPrefix(size__);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        CORRADE_DEPRECATED("use exceptPrefix() instead") constexpr ArrayView<T> suffix(std::size_t begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }
        #endif

        /**
         * @brief Fixed-size view except the first @p size__ items
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice<size__, Size>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t size__> constexpr StaticArrayView<size_ - size__, T> exceptPrefix() const {
            return slice<size__, size_>();
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptPrefix()
         * @m_deprecated_since_latest Use @ref exceptPrefix() instead.
         */
        template<std::size_t begin_> CORRADE_DEPRECATED("use exceptPrefix() instead") constexpr StaticArrayView<size_ - begin_, T> suffix() const {
            return slice<begin_, size_>();
        }
        #endif

        /** @copydoc ArrayView::exceptSuffix(std::size_t) const */
        constexpr ArrayView<T> exceptSuffix(std::size_t size) const {
            return ArrayView<T>(*this).exceptSuffix(size);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        CORRADE_DEPRECATED("use exceptSuffix() instead") constexpr ArrayView<T> except(std::size_t count) const {
            return ArrayView<T>(*this).except(count);
        }
        #endif

        /**
         * @brief Fixed-size view except the last @p size__ items
         * @m_since_latest
         *
         * Equivalent to @cpp data.slice<0, Size - size__>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t size__> constexpr StaticArrayView<size_ - size__, T> exceptSuffix() const {
            return slice<0, size_ - size__>();
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief exceptSuffix()
         * @m_deprecated_since_latest Use @ref exceptSuffix() instead.
         */
        template<std::size_t count> CORRADE_DEPRECATED("use exceptSuffix() instead") constexpr StaticArrayView<size_ - count, T> except() const {
            return slice<0, size_ - count>();
        }
        #endif

    private:
        #if CORRADE_CXX_STANDARD > 201402
        /* For C++17 structured bindings, if StructuredBindings.h is included
           as well. There doesn't seem to be a way to call those directly, and
           I can't find any practical use of std::tuple_size, tuple_element etc
           on C++11 and C++14, so this is defined only for newer standards. */
        template<std::size_t index> constexpr friend T& get(StaticArrayView<size_, T> value) {
            return value._data[index];
        }
        /* As the view is non-owning, a rvalue doesn't imply that its contents
           are able to be moved out. Thus, unlike StaticArray or Pair/Triple,
           it takes the view by value and has no difference in behavior
           depending on whether the input is T&, const T& or T&&. */
        #endif

        T* _data;
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief Two-component array view
@m_since_latest

Convenience alternative to @cpp StaticArrayView<2, T> @ce. See
@ref StaticArrayView for more information.
@see @ref ArrayView3, @ref ArrayView4, @ref Array2, @ref Array3, @ref Array4
*/
template<class T> using ArrayView2 = StaticArrayView<2, T>;

/**
@brief Three-component array view
@m_since_latest

Convenience alternative to @cpp StaticArrayView<3, T> @ce. See
@ref StaticArrayView for more information.
@see @ref ArrayView2, @ref ArrayView4, @ref Array2, @ref Array3, @ref Array4
*/
template<class T> using ArrayView3 = StaticArrayView<3, T>;

/**
@brief Four-component array view
@m_since_latest

Convenience alternative to @cpp StaticArrayView<4, T> @ce. See
@ref StaticArrayView for more information.
@see @ref ArrayView2, @ref ArrayView3, @ref Array2, @ref Array3, @ref Array4
*/
template<class T> using ArrayView4 = StaticArrayView<4, T>;
#endif

/** @relatesalso StaticArrayView
@brief Make a static view on an array

Convenience alternative to @ref StaticArrayView::StaticArrayView(T*). The
following two lines are equivalent:

@snippet Containers.cpp staticArrayView
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(T* data) {
    return StaticArrayView<size, T>{data};
}

/** @relatesalso StaticArrayView
@brief Make a static view on a fixed-size array

Convenience alternative to @ref StaticArrayView::StaticArrayView(U(&)[size_]).
The following two lines are equivalent:

@snippet Containers.cpp staticArrayView-array
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(T(&data)[size]) {
    return StaticArrayView<size, T>{data};
}

/** @relatesalso StaticArrayView
@brief Make a static view on a view

Equivalent to the implicit @ref StaticArrayView copy constructor --- it
shouldn't be an error to call @ref staticArrayView() on itself.
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(StaticArrayView<size, T> view) {
    return view;
}

/** @relatesalso StaticArrayView
@brief Make a static view on an external type / from an external representation

@see @ref Containers-StaticArrayView-stl
*/
/* There's no restriction that would disallow creating StaticArrayView from
   e.g. std::array<T>&& because that would break uses like `consume(foo());`,
   where `consume()` expects a view but `foo()` returns a std::array. */
template<class T, class U = decltype(Implementation::ErasedStaticArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>()))> constexpr U staticArrayView(T&& other) {
    return Implementation::ErasedStaticArrayViewConverter<typename std::remove_reference<T&&>::type>::from(Utility::forward<T>(other));
}

/** @relatesalso StaticArrayView
@brief Reinterpret-cast a static array view

Size of the new array is calculated as @cpp view.size()*sizeof(T)/sizeof(U) @ce.
Expects that both types are [standard layout](https://en.cppreference.com/w/cpp/named_req/StandardLayoutType.html)
and the total byte size doesn't change. Example usage:

@snippet Containers.cpp arrayCast-StaticArrayView
*/
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), U> arrayCast(StaticArrayView<size, T> view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    constexpr const std::size_t newSize = size*sizeof(T)/sizeof(U);
    static_assert(newSize*sizeof(U) == size*sizeof(T),
        "type sizes are not compatible");
    return StaticArrayView<newSize, U>{reinterpret_cast<U*>(view.begin())};
}

/** @relatesalso StaticArrayView
@brief Reinterpret-cast a statically sized array

Calls @ref arrayCast(StaticArrayView<size, T>) with the argument converted to
@ref StaticArrayView of the same type and size. Example usage:

@snippet Containers.cpp arrayCast-StaticArrayView-array
*/
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), U> arrayCast(T(&data)[size]) {
    return arrayCast<U>(StaticArrayView<size, T>{data});
}

template<class T> constexpr T& ArrayView<T>::front() const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(_size, "Containers::ArrayView::front(): view is empty"), _data[0];
}

template<class T> constexpr T& ArrayView<T>::back() const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(_size, "Containers::ArrayView::back(): view is empty"), _data[_size - 1];
}

template<class T> template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type> constexpr T& ArrayView<T>::operator[](const U i) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(std::size_t(i) < _size,
        "Containers::ArrayView::operator[](): index" << i << "out of range for" << _size << "elements"), _data[i];
}

template<class T> constexpr ArrayView<T> ArrayView<T>::slice(T* begin, T* end) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(_data <= begin && begin <= end && end <= _data + _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end - _data
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        ArrayView<T>{begin, std::size_t(end - begin)};
}

template<class T> constexpr ArrayView<T> ArrayView<T>::slice(std::size_t begin, std::size_t end) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(begin <= end && end <= _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        ArrayView<T>{_data + begin, end - begin};
}

template<std::size_t size_, class T> constexpr T& StaticArrayView<size_, T>::front() const {
    static_assert(size_, "view is empty");
    return _data[0];
}

template<std::size_t size_, class T> constexpr T& StaticArrayView<size_, T>::back() const {
    static_assert(size_, "view is empty");
    return _data[size_ - 1];
}

template<std::size_t size_, class T> template<class U, typename std::enable_if<std::is_convertible<U, std::size_t>::value, int>::type> constexpr T& StaticArrayView<size_, T>::operator[](const U i) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(std::size_t(i) < size_,
        "Containers::StaticArrayView::operator[](): index" << i << "out of range for" << size_ << "elements"), _data[i];
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T> template<std::size_t size_, class U, typename std::enable_if<std::is_convertible<U, T*>::value && !std::is_convertible<U, std::size_t>::value, int>::type> constexpr StaticArrayView<size_, T> ArrayView<T>::slice(const U begin) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(_data <= begin && begin + size_ <= _data + _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << begin + size_ - _data
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        StaticArrayView<size_, T>{begin};
}
#endif

template<class T> template<std::size_t size_> constexpr StaticArrayView<size_, T> ArrayView<T>::slice(std::size_t begin) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(begin + size_ <= _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << begin + size_
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        StaticArrayView<size_, T>{_data + begin};
}

template<class T> template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> ArrayView<T>::slice() const {
    static_assert(begin_ < end_, "fixed-size slice needs to have a positive size");
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(end_ <= _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin_
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end_
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        StaticArrayView<end_ - begin_, T>{_data + begin_};
}

template<std::size_t size_, class T> template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> StaticArrayView<size_, T>::slice() const {
    static_assert(begin_ < end_, "fixed-size slice needs to have a positive size");
    static_assert(end_ <= size_, "slice out of range");
    return StaticArrayView<end_ - begin_, T>{_data + begin_};
}

}}

#endif
