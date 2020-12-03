#ifndef Corrade_Containers_ArrayView_h
#define Corrade_Containers_ArrayView_h
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
 * @brief Class @ref Corrade::Containers::ArrayView, @ref Corrade::Containers::StaticArrayView, alias @ref Corrade::Containers::ArrayView2, @ref Corrade::Containers::ArrayView3, @ref Corrade::Containers::ArrayView4
 */

#include <initializer_list>
#include <type_traits>
#include <utility>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class, class> struct ArrayViewConverter;
    template<class> struct ErasedArrayViewConverter;
    /* so ArrayTuple can update the data pointer */
    template<class T> T*& dataRef(Containers::ArrayView<T>& view) {
        return view._data;
    }
    #ifndef CORRADE_NO_PYTHON_COMPATIBILITY
    /* so Python buffer protocol can point to the size member */
    template<class T> std::size_t& sizeRef(Containers::ArrayView<T>& view) {
        return view._size;
    }
    #endif
}

/**
@brief Array view with size information

Immutable wrapper around continuous range of data, similar to a dynamic
@ref std::span from C++2a. Unlike @ref Array this class doesn't do any memory
management. Main use case is passing array along with size information to
functions etc. If @p T is @cpp const @ce type, the class is implicitly
constructible also from const references to @ref Array and @ref ArrayView of
non-const types. There's also a variant with compile-time size information
called @ref StaticArrayView.

Usage example:

@snippet Containers.cpp ArrayView-usage

@attention Note that when using @cpp Containers::ArrayView<const char> @ce, C
    string literals (such as @cpp "hello" @ce) are implicitly convertible to it
    and the size includes also the zero-terminator (thus in case of
    @cpp "hello" @ce the size would be 6, not 5, as one might expect).

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
documentation of @ref Containers-Array-stl "Array",
@ref Containers-StaticArray-stl "StaticArray" and
@ref Containers-StridedArrayView-stl "StridedArrayView" for more information.

@anchor Containers-ArrayView-single-header

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with the @ref ArrayView<void> /
    @ref ArrayView<const void> specializations and @ref StaticArrayView, is
    also available as a single-header, dependency-less [CorradeArrayView.h](https://github.com/mosra/magnum-singles/tree/master/CorradeArrayView.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information. The above
    mentioned STL compatibility is included as well, but disabled by default.
    Enable it for @ref std::vector and @ref std::array by specifying
    @cpp #define CORRADE_ARRAYVIEW_STL_COMPATIBILITY @ce and for @ref std::span
    by compiling as C++2a and specifying
    @cpp #define CORRADE_ARRAYVIEW_STL_SPAN_COMPATIBILITY @ce before including
    the file. Including it multiple times with different macros defined works
    as well.

@see @ref ArrayView<void>, @ref ArrayView<const void>, @ref StridedArrayView,
    @ref arrayView(), @ref arrayCast(ArrayView<T>)
*/
/* All member functions are const because the view doesn't own the data */
template<class T> class ArrayView {
    public:
        typedef T Type;     /**< @brief Element type */

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ ArrayView(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates an empty view. Copy a non-empty @ref Array or @ref ArrayView
         * onto the instance to make it useful.
         */
        constexpr /*implicit*/ ArrayView() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Construct a view on an array with explicit length
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
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U, std::size_t size>
        #else
        template<class U, std::size_t size, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(U(&data)[size]) noexcept: _data{data}, _size{size} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on @ref ArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(ArrayView<U> view) noexcept: _data{view}, _size{view.size()} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref arrayView(StaticArrayView<size, T>)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size, class U>
        #else
        template<std::size_t size, class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(StaticArrayView<size, U> view) noexcept: _data{view}, _size{size} {
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
        template<class U, class = decltype(Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ ArrayView(U&& other) noexcept: ArrayView{Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(std::forward<U>(other))} {}

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

        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. On MSVC 2019 this works properly when
           /permissive- is enabled, but I can neither detect a presence of that
           flag nor force it for all users (since it often ICEs and breaks 3rd
           party code), so disabling for 2019 as well. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief View data */
        constexpr T* data() const { return _data; }

        /** @brief View size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the view is empty */
        constexpr bool empty() const { return !_size; }

        /**
         * @brief Pointer to first element
         *
         * @see @ref front()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        constexpr T* end() const { return _data+_size; }
        constexpr T* cend() const { return _data+_size; } /**< @overload */

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref begin()
         */
        T& front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref end()
         */
        T& back() const;

        /**
         * @brief Array slice
         *
         * Both arguments are expected to be in range.
         */
        constexpr ArrayView<T> slice(T* begin, T* end) const;

        /** @overload */
        constexpr ArrayView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief Static array slice
         *
         * Both @p begin and @cpp begin + viewSize @ce are expected to be in
         * range.
         */
        template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> slice(T* begin) const;

        /** @overload */
        template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> slice(std::size_t begin) const;

        /**
         * @brief Static array slice
         * @m_since{2019,10}
         *
         * At compile time expects that @cpp begin < end_ @ce, at runtime that
         * @p end_ is not larger than @ref size().
         */
        template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> slice() const;

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(data.begin(), end) @ce. If @p end is
         * @cpp nullptr @ce, returns zero-sized @cpp nullptr @ce array.
         * @see @ref slice(T*, T*) const
         */
        constexpr ArrayView<T> prefix(T* end) const {
            return end ? slice(_data, end) : nullptr;
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(0, end) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr ArrayView<T> prefix(std::size_t end) const {
            return slice(0, end);
        }

        /**
         * @brief Static array prefix
         *
         * Equivalent to @cpp data.slice<0, end_>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t end_> constexpr StaticArrayView<end_, T> prefix() const {
            return slice<0, end_>();
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice(begin, data.end()) @ce. If @p begin is
         * @cpp nullptr @ce and the original array isn't, returns zero-sized
         * @cpp nullptr @ce array.
         * @see @ref slice(T*, T*) const
         */
        constexpr ArrayView<T> suffix(T* begin) const {
            return _data && !begin ? nullptr : slice(begin, _data + _size);
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice(begin, data.size()) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr ArrayView<T> suffix(std::size_t begin) const {
            return slice(begin, _size);
        }

        /**
         * @brief Array prefix except the last @p count items
         * @m_since{2019,10}
         *
         * Equivalent to @cpp data.slice(0, data.size() - count) @ce.
         * @see @ref slice(std::size_t, std::size_t) const
         */
        constexpr ArrayView<T> except(std::size_t count) const {
            return slice(0, _size - count);
        }

    private:
        friend T*& Implementation::dataRef<>(Containers::ArrayView<T>&);
        #ifndef CORRADE_NO_PYTHON_COMPATIBILITY
        /* so Python buffer protocol can point to the size member */
        friend std::size_t& Implementation::sizeRef<>(Containers::ArrayView<T>&);
        #endif
        T* _data;
        std::size_t _size;
};

/**
@brief Void array view with size information
@m_since{2019,10}

Specialization of @ref ArrayView which is convertible from a compile-time
array, @ref Array, @ref ArrayView or @ref StaticArrayView of any non-constant
type and also any non-constant type convertible to them. Size for a particular
type is recalculated to a size in bytes. This specialization doesn't provide
any accessors besides @ref data(), because it has no use for the @cpp void @ce
type. Instead, use @ref arrayCast(ArrayView<void>) to first cast the array to a
concrete type and then access the particular elements.

Usage example:

@snippet Containers.cpp ArrayView-void-usage

<b></b>

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

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ ArrayView(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        constexpr /*implicit*/ ArrayView() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Construct a view on array of explicit length
         * @param data      Data pointer
         * @param size      Data size
         */
        constexpr /*implicit*/ ArrayView(void* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Constructor from any type
         * @param data      Data pointer
         * @param size      Data size
         *
         * Size is recalculated to size in bytes.
         */
        template<class T> constexpr /*implicit*/ ArrayView(T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct a view on fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        template<class T, std::size_t size
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<!std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ ArrayView(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}

        /** @brief Construct const void view on any @ref ArrayView */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<!std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ ArrayView(ArrayView<T> array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct const void view on any @ref StaticArrayView */
        template<std::size_t size, class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<!std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ ArrayView(const StaticArrayView<size, T>& array) noexcept: _data{array}, _size{size*sizeof(T)} {}

        /**
         * @brief Construct a view on an external type
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. */
        template<class T, class = decltype(Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(std::declval<T&&>()))> constexpr /*implicit*/ ArrayView(T&& other) noexcept: ArrayView{Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(other)} {}

        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. On MSVC 2019 this works properly when
           /permissive- is enabled, but I can neither detect a presence of that
           flag nor force it for all users (since it often ICEs and breaks 3rd
           party code), so disabling for 2019 as well. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator void*() const { return _data; }

        /** @brief Array data */
        constexpr void* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

    private:
        void* _data;
        std::size_t _size;
};

/**
@brief Constant void array view with size information

Specialization of @ref ArrayView which is convertible from a compile-time
array, @ref Array, @ref ArrayView or @ref StaticArrayView of any type and also
any type convertible to them. Size for a particular type is recalculated to
a size in bytes. This specialization doesn't provide any accessors besides
@ref data(), because it has no use for the @cpp void @ce type. Instead, use
@ref arrayCast(ArrayView<const void>) to first cast the array to a concrete
type and then access the particular elements.

Usage example:

@snippet Containers.cpp ArrayView-const-void-usage

<b></b>

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

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ ArrayView(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        constexpr /*implicit*/ ArrayView() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Construct a view on array of explicit length
         * @param data      Data pointer
         * @param size      Data size
         */
        constexpr /*implicit*/ ArrayView(const void* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Constructor from any type
         * @param data      Data pointer
         * @param size      Data size
         *
         * Size is recalculated to size in bytes.
         */
        template<class T> constexpr /*implicit*/ ArrayView(const T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct a view on fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        template<class T, std::size_t size> constexpr /*implicit*/ ArrayView(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}

        /** @brief Construct const void view on an @ref ArrayView<void> */
        constexpr /*implicit*/ ArrayView(ArrayView<void> array) noexcept: _data{array}, _size{array.size()} {}

        /** @brief Construct const void view on any @ref ArrayView */
        template<class T> constexpr /*implicit*/ ArrayView(ArrayView<T> array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct const void view on any @ref StaticArrayView */
        template<std::size_t size, class T> constexpr /*implicit*/ ArrayView(const StaticArrayView<size, T>& array) noexcept: _data{array}, _size{size*sizeof(T)} {}

        /**
         * @brief Construct a view on an external type
         *
         * @see @ref Containers-ArrayView-stl
         */
        /* There's no restriction that would disallow creating ArrayView from
           e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. */
        template<class T, class = decltype(Implementation::ErasedArrayViewConverter<const T>::from(std::declval<const T&>()))> constexpr /*implicit*/ ArrayView(const T& other) noexcept: ArrayView{Implementation::ErasedArrayViewConverter<const T>::from(other)} {}

        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. On MSVC 2019 this works properly when
           /permissive- is enabled, but I can neither detect a presence of that
           flag nor force it for all users (since it often ICEs and breaks 3rd
           party code), so disabling for 2019 as well. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator const void*() const { return _data; }

        /** @brief Array data */
        constexpr const void* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

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
with r-value initializer lists. See
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
    return Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::forward<T>(other));
}

/** @relatesalso ArrayView
@brief Reinterpret-cast an array view

Size of the new array is calculated as @cpp view.size()*sizeof(T)/sizeof(U) @ce.
Expects that both types are [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and the total byte size doesn't change. Example usage:

@snippet Containers.cpp arrayCast
*/
template<class U, class T> ArrayView<U> arrayCast(ArrayView<T> view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    const std::size_t size = view.size()*sizeof(T)/sizeof(U);
    CORRADE_ASSERT(size*sizeof(U) == view.size()*sizeof(T),
        "Containers::arrayCast(): can't reinterpret" << view.size() << sizeof(T) << Utility::Debug::nospace << "-byte items into a" << sizeof(U) << Utility::Debug::nospace << "-byte type", {});
    return {reinterpret_cast<U*>(view.begin()), size};
}

/** @relatesalso ArrayView
@brief Reinterpret-cast a void array view
@m_since{2020,06}

Size of the new array is calculated as @cpp view.size()/sizeof(U) @ce.
Expects that the target type is [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and the total byte size doesn't change.
*/
template<class U> ArrayView<U> arrayCast(ArrayView<const void> view) {
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    const std::size_t size = view.size()/sizeof(U);
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

Alias to @ref ArrayView::size(), useful as a shorthand in cases like this:

@snippet Containers.cpp arraySize
*/
template<class T> constexpr std::size_t arraySize(ArrayView<T> view) {
    return view.size();
}

/** @overload */
template<std::size_t size_, class T> constexpr std::size_t arraySize(StaticArrayView<size_, T>) {
    return size_;
}

/** @overload */
template<std::size_t size_, class T> constexpr std::size_t arraySize(T(&)[size_]) {
    return size_;
}

namespace Implementation {
    template<std::size_t, class, class> struct StaticArrayViewConverter;
    template<class> struct ErasedStaticArrayViewConverter;
}

/**
@brief Fixed-size array view

Equivalent to @ref ArrayView, but with compile-time size information. Similar
to a fixed-size @ref std::span from C++2a. Convertible from and to
@ref ArrayView. Example usage:

@snippet Containers.cpp StaticArrayView-usage

@section Containers-StaticArrayView-stl STL compatibility

See @ref Containers-ArrayView-stl "ArrayView STL compatibility" for more
information.

@m_class{m-block m-success}

@par Single-header version
    This class is, together with @ref ArrayView and the
    @ref ArrayView<const void> specialization, is also available as a
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

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ StaticArrayView(std::nullptr_t) noexcept: _data(nullptr) {}

        /**
         * @brief Default constructor
         *
         * Creates empty view. Copy non-empty @ref StaticArrayView onto the
         * instance to make it useful.
         */
        constexpr /*implicit*/ StaticArrayView() noexcept: _data(nullptr) {}

        /**
         * @brief Construct static view on an array
         * @param data      Data pointer
         *
         * @see @ref staticArrayView(T*)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr explicit StaticArrayView(T* data)
        #else
        template<class U, class = typename std::enable_if<std::is_pointer<U>::value && !std::is_same<U, T(&)[size_]>::value>::type> constexpr explicit StaticArrayView(U data)
        #endif
        noexcept: _data(data) {}

        /**
         * @brief Construct static view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Note that, similarly as with raw pointers, you need to ensure that
         * both types have the same size.
         * @see @ref staticArrayView(T(&)[size])
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StaticArrayView(U(&data)[size_]) noexcept: _data{data} {}

        /**
         * @brief Construct static view on @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StaticArrayView(StaticArrayView<size_, U> view) noexcept: _data{view} {
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
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ StaticArrayView(U&& other) noexcept: StaticArrayView{Implementation::StaticArrayViewConverter<size_, T, typename std::decay<U&&>::type>::from(std::forward<U>(other))} {}

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

        #ifndef CORRADE_MSVC2019_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. On MSVC 2019 this works properly when
           /permissive- is enabled, but I can neither detect a presence of that
           flag nor force it for all users (since it often ICEs and breaks 3rd
           party code), so disabling for 2019 as well. */
        constexpr explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief Array data */
        constexpr T* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return size_; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !size_; }

        /**
         * @brief Pointer to first element
         *
         * @see @ref front()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; } /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        constexpr T* end() const { return _data + size_; }
        constexpr T* cend() const { return _data + size_; } /**< @overload */

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref begin()
         */
        T& front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref end()
         */
        T& back() const;

        /** @copydoc ArrayView::slice(T*, T*) const */
        constexpr ArrayView<T> slice(T* begin, T* end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        constexpr ArrayView<T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }

        /** @copydoc ArrayView::slice(T*) const */
        template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> slice(T* begin) const {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }
        /** @overload */
        template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> slice(std::size_t begin) const {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }

        /**
         * @brief Static array slice
         * @m_since{2019,10}
         *
         * Expects (at compile time) that @cpp begin < end_ @ce and @p end_ is
         * not larger than @ref Size.
         */
        template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> slice() const;

        /** @copydoc ArrayView::prefix(T*) const */
        constexpr ArrayView<T> prefix(T* end) const {
            return ArrayView<T>(*this).prefix(end);
        }
        /** @overload */
        constexpr ArrayView<T> prefix(std::size_t end) const {
            return ArrayView<T>(*this).prefix(end);
        }

        /**
         * @brief Static array prefix
         *
         * Equivalent to @cpp data.slice<0, end_>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t end_> constexpr StaticArrayView<end_, T> prefix() const {
            return slice<0, end_>();
        }

        /** @copydoc ArrayView::suffix(T*) const */
        constexpr ArrayView<T> suffix(T* begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @overload */
        constexpr ArrayView<T> suffix(std::size_t begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }

        /**
         * @brief Static array suffix
         * @m_since{2019,10}
         *
         * Equivalent to @cpp data.slice<begin_, Size>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t begin_> constexpr StaticArrayView<size_ - begin_, T> suffix() const {
            return slice<begin_, size_>();
        }

        /** @copydoc ArrayView::except(std::size_t) const */
        constexpr ArrayView<T> except(std::size_t count) const {
            return ArrayView<T>(*this).except(count);
        }

        /**
         * @brief Static array prefix except the last @p count items
         *
         * Equivalent to @cpp data.slice<0, Size - count>() @ce.
         * @see @ref slice() const
         */
        template<std::size_t count> constexpr StaticArrayView<size_ - count, T> except() const {
            return slice<0, size_ - count>();
        }

    private:
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
    return Implementation::ErasedStaticArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::forward<T>(other));
}

/** @relatesalso StaticArrayView
@brief Reinterpret-cast a static array view

Size of the new array is calculated as @cpp view.size()*sizeof(T)/sizeof(U) @ce.
Expects that both types are [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
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

template<class T> T& ArrayView<T>::front() const {
    CORRADE_ASSERT(_size, "Containers::ArrayView::front(): view is empty", _data[0]);
    return _data[0];
}

template<class T> T& ArrayView<T>::back() const {
    CORRADE_ASSERT(_size, "Containers::ArrayView::back(): view is empty", _data[_size - 1]);
    return _data[_size - 1];
}

template<class T> constexpr ArrayView<T> ArrayView<T>::slice(T* begin, T* end) const {
    return CORRADE_CONSTEXPR_ASSERT(_data <= begin && begin <= end && end <= _data + _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end - _data
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        ArrayView<T>{begin, std::size_t(end - begin)};
}

template<class T> constexpr ArrayView<T> ArrayView<T>::slice(std::size_t begin, std::size_t end) const {
    return CORRADE_CONSTEXPR_ASSERT(begin <= end && end <= _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        ArrayView<T>{_data + begin, end - begin};
}

template<std::size_t size_, class T> T& StaticArrayView<size_, T>::front() const {
    static_assert(size_, "view is empty");
    return _data[0];
}

template<std::size_t size_, class T> T& StaticArrayView<size_, T>::back() const {
    static_assert(size_, "view is empty");
    return _data[size_ - 1];
}

template<class T> template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> ArrayView<T>::slice(T* begin) const {
    return CORRADE_CONSTEXPR_ASSERT(_data <= begin && begin + viewSize <= _data + _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin - _data
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << begin + viewSize - _data
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        StaticArrayView<viewSize, T>{begin};
}

template<class T> template<std::size_t viewSize> constexpr StaticArrayView<viewSize, T> ArrayView<T>::slice(std::size_t begin) const {
    return CORRADE_CONSTEXPR_ASSERT(begin + viewSize <= _size,
            "Containers::ArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << begin + viewSize
            << Utility::Debug::nospace << "] out of range for" << _size
            << "elements"),
        StaticArrayView<viewSize, T>{_data + begin};
}

template<class T> template<std::size_t begin_, std::size_t end_> constexpr StaticArrayView<end_ - begin_, T> ArrayView<T>::slice() const {
    static_assert(begin_ < end_, "fixed-size slice needs to have a positive size");
    return CORRADE_CONSTEXPR_ASSERT(end_ <= _size,
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
    static_assert(end_ <= size_, "slice out of bounds");
    return StaticArrayView<end_ - begin_, T>{_data + begin_};
}

}}

#endif
