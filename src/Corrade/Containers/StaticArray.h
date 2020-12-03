#ifndef Corrade_Containers_StaticArray_h
#define Corrade_Containers_StaticArray_h
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
 * @brief Class @ref Corrade::Containers::StaticArray, alias @ref Corrade::Containers::Array2, @ref Corrade::Containers::Array3, @ref Corrade::Containers::Array4
 */

#include <new>
#include <type_traits>
#include <utility>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Tags.h"
#include "Corrade/Containers/constructHelpers.h"

namespace Corrade { namespace Containers {

/**
@brief Static array wrapper
@tparam size_   Array size
@tparam T       Element type

Provides statically-sized array wrapper with API similar to @ref Array. Useful
as more featureful alternative to plain C arrays or @ref std::array.

Usage example:

@snippet Containers.cpp StaticArray-usage

@section Containers-StaticArray-initialization Array initialization

The array is by default *value-initialized*, which means that trivial types
are zero-initialized and the default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref StaticArray(DefaultInitT) leaves trivial types uninitialized
    and calls the default constructor elsewhere. In other words,
    @cpp T array[size] @ce.
-   @ref StaticArray(ValueInitT) is equivalent to the implicit parameterless
    constructor, zero-initializing trivial types and calling the default
    constructor elsewhere. Useful when you want to make the choice appear
    explicit. In other words, @cpp T array[size]{} @ce.
-   @ref StaticArray(DirectInitT, Args&&... args) constructs every element of
    the array using provided arguments. In other words,
    @cpp T array[size]{T{args...}, T{args...}, …} @ce.
-   @ref StaticArray(InPlaceInitT, Args&&... args) is equivalent to
    @ref StaticArray(Args&&... args), again useful when you want to make the
    choice appear explicit). In other words, @cpp T array[size]{args...} @ce.
-   @ref StaticArray(NoInitT) does not initialize anything and you need to call
    the constructor on all elements manually using placement new,
    @ref std::uninitialized_copy() or similar. This is the dangerous option.

Example:

@snippet Containers.cpp StaticArray-initialization

@section Containers-StaticArray-views Conversion to array views

Arrays are implicitly convertible to @ref ArrayView / @ref StaticArrayView as
described in the following table. The conversion is only allowed if @cpp T* @ce
is implicitly convertible to @cpp U* @ce (or both are the same type) and both
have the same size. This also extends to other container types constructibe
from @ref ArrayView / @ref StaticArrayView, which means for example that a
@ref StridedArrayView1D is implicitly convertible from @ref StaticArray as
well.

Owning array type               | ↭ | Non-owning view type
------------------------------- | - | ---------------------
@ref StaticArray "Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, U>"
@ref StaticArray "Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, const U>"
@ref StaticArray "const Array<size, T>" | → | @ref StaticArrayView "ArrayView<size, const U>"
@ref StaticArray "Array<size, T>" | → | @ref ArrayView "ArrayView<U>"
@ref StaticArray "Array<size, T>" | → | @ref ArrayView "ArrayView<const U>"
@ref StaticArray "const Array<size, T>" | → | @ref ArrayView "ArrayView<const U>"

@section Containers-StaticArray-stl STL compatibility

On compilers that support C++2a and @ref std::span, implicit conversion of an
@ref StaticArray to it is provided in @ref Corrade/Containers/ArrayViewStlSpan.h.
The conversion is provided in a separate header to avoid unconditional
@cpp #include <span> @ce, which significantly affects compile times. The
following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StaticArray "StaticArray<size, T>" | → | @ref std::span "std::span<T, size>"
@ref StaticArray "StaticArray<size, T>" | → | @ref std::span "std::span<const T, size>"
@ref StaticArray "const StaticArray<size, T>" | → | @ref std::span "std::span<const T, size>"

There are some dangerous corner cases due to the way @ref std::span is
designed, see @ref Containers-ArrayView-stl "ArrayView STL compatibility" for
more information.

@m_class{m-block m-success}

@par Single-header version
    This class, together with @ref Array, is also available as a single-header
    [CorradeArray.h](https://github.com/mosra/magnum-singles/tree/master/CorradeArray.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref arrayCast(StaticArray<size, T>&), @ref Array,
    @ref Array2, @ref Array3, @ref Array4
*/
/* Underscore at the end to avoid conflict with member size(). It's ugly, but
   having count instead of size_ would make the naming horribly inconsistent. */
template<std::size_t size_, class T> class StaticArray {
    /* Ideally this could be derived from StaticArrayView<size_, T>, avoiding a
       lot of redundant code, however I'm unable to find a way to add
       const/non-const overloads of all slicing functions and also prevent
       const StaticArray<size_, T>& from being sliced to a (mutable)
       StaticArrayView<size_, T>. */

    public:
        enum: std::size_t {
            Size = size_    /**< Array size */
        };
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Construct a default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e., builtin types are not initialized).
         * @see @ref DefaultInit, @ref StaticArray(ValueInitT)
         */
        explicit StaticArray(DefaultInitT): StaticArray{DefaultInit, std::integral_constant<bool, std::is_standard_layout<T>::value && std::is_trivial<T>::value>{}} {}

        /**
         * @brief Construct a value-initialized array
         *
         * Creates array of given size, the contents are value-initialized
         * (i.e., builtin types are zero-initialized, default constructor
         * called otherwise). This is the same as @ref StaticArray().
         *
         * Useful if you want to create an array of primitive types and set
         * them to zero.
         * @see @ref ValueInit, @ref StaticArray(DefaultInitT)
         */
        explicit StaticArray(ValueInitT): _data{} {}

        /**
         * @brief Construct an array without initializing its contents
         *
         * Creates array of given size, the contents are *not* initialized.
         * Initialize the values using placement new.
         *
         * Useful if you will be overwriting all elements later anyway.
         * @attention Internally the destruction is done using custom deleter
         *      that explicitly calls destructor on *all elements* regardless
         *      of whether they were properly constructed or not.
         * @see @ref NoInit, @ref StaticArray(DirectInitT, Args&&... args)
         */
        explicit StaticArray(NoInitT) {}

        /**
         * @brief Construct a direct-initialized array
         *
         * Constructs the array using the @ref StaticArray(NoInitT) constructor
         * and then initializes each element with placement new using forwarded
         * @p args.
         * @see @ref StaticArray(InPlaceInitT, Args&&... args)
         */
        template<class ...Args> explicit StaticArray(DirectInitT, Args&&... args);

        /**
         * @brief Construct an in-place-initialized array
         *
         * The arguments are forwarded to the array constructor. Same as
         * @ref StaticArray(Args&&... args).
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        template<class ...Args> explicit StaticArray(InPlaceInitT, Args&&... args): _data{std::forward<Args>(args)...} {
            static_assert(sizeof...(args) == size_, "Containers::StaticArray: wrong number of initializers");
        }

        /**
         * @brief Construct a value-initialized array
         *
         * Alias to @ref StaticArray(ValueInitT).
         * @see @ref StaticArray(DefaultInitT)
         */
        explicit StaticArray(): StaticArray{ValueInit} {}

        /**
         * @brief Construct an in-place-initialized array
         *
         * Alias to @ref StaticArray(InPlaceInitT, Args&&... args).
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class ...Args> /*implicit*/ StaticArray(Args&&... args);
        #else
        template<class First, class ...Next, class = typename std::enable_if<std::is_convertible<First&&, T>::value>::type> /*implicit*/ StaticArray(First&& first, Next&&... next): StaticArray{InPlaceInit, std::forward<First>(first), std::forward<Next>(next)...} {}
        #endif

        /** @brief Copy constructor */
        StaticArray(const StaticArray<size_, T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value);

        /** @brief Move constructor */
        StaticArray(StaticArray<size_, T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value);

        ~StaticArray();

        /** @brief Copy assignment */
        StaticArray<size_, T>& operator=(const StaticArray<size_, T>&) noexcept(std::is_nothrow_copy_constructible<T>::value);

        /** @brief Move assignment */
        StaticArray<size_, T>& operator=(StaticArray<size_, T>&&) noexcept(std::is_nothrow_move_constructible<T>::value);

        /* The following view conversion is *not* restricted to this& because
           that would break uses like `consume(foo());`, where `consume()`
           expects a view but `foo()` returns an owning array. */

        /**
         * @brief Convert to external view representation
         *
         * @see @ref Containers-Array-stl
         */
        /* To simplify the implementation, there's no ArrayViewConverter
           overload. Instead, the implementer is supposed to extend
           StaticArrayViewConverter specializations for the non-static arrays
           as well. */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, T, U>::to(std::declval<StaticArrayView<size_, T>>()))> /*implicit*/ operator U() {
            return Implementation::StaticArrayViewConverter<size_, T, U>::to(*this);
        }

        /** @overload */
        template<class U, class = decltype(Implementation::StaticArrayViewConverter<size_, const T, U>::to(std::declval<StaticArrayView<size_, const T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::StaticArrayViewConverter<size_, const T, U>::to(*this);
        }

        /* `char* a = Containers::StaticArray<char>(5); a[3] = 5;` would result
           in instant segfault, disallowing it in the following conversion
           operators */

        /** @brief Conversion to array type */
        /*implicit*/ operator T*() & { return _data; }

        /** @overload */
        /*implicit*/ operator const T*() const & { return _data; }

        /** @brief Array data */
        T* data() { return _data; }
        const T* data() const { return _data; }             /**< @overload */

        /**
         * @brief Array size
         *
         * Equivalent to @ref Size.
         */
        constexpr std::size_t size() const { return size_; }

        /**
         * @brief Whether the array is empty
         *
         * Always true (it's not possible to create zero-sized C array).
         */
        constexpr bool empty() const { return !size_; }

        /**
         * @brief Pointer to first element
         *
         * @see @ref front()
         */
        T* begin() { return _data; }
        const T* begin() const { return _data; }            /**< @overload */
        const T* cbegin() const { return _data; }           /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        T* end() { return _data + size_; }
        const T* end() const { return _data + size_; }      /**< @overload */
        const T* cend() const { return _data + size_; }     /**< @overload */

        /**
         * @brief First element
         *
         * @see @ref begin()
         */
        T& front() { return _data[0]; }
        const T& front() const { return _data[0]; }         /**< @overload */

        /**
         * @brief Last element
         *
         * @see @ref end()
         */
        T& back() { return _data[size_ - 1]; }
        const T& back() const { return _data[size_ - 1]; }  /**< @overload */

        /**
         * @brief Array slice
         *
         * Equivalent to @ref StaticArrayView::slice(T*, T*) const and
         * overloads.
         */
        ArrayView<T> slice(T* begin, T* end) {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<const T> slice(const T* begin, const T* end) const {
            return ArrayView<const T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<T> slice(std::size_t begin, std::size_t end) {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<const T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<const T>(*this).slice(begin, end);
        }

        /**
         * @brief Static array slice
         *
         * Equivalent to @ref StaticArrayView::slice(T*) const and overloads.
         */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(T* begin) {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> slice(const T* begin) const {
            return ArrayView<const T>(*this).template slice<viewSize>(begin);
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(std::size_t begin) {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> slice(std::size_t begin) const {
            return ArrayView<const T>(*this).template slice<viewSize>(begin);
        }

        /**
         * @brief Static array slice
         * @m_since{2019,10}
         *
         * Equivalent to @ref StaticArrayView::slice() const.
         */
        template<std::size_t begin_, std::size_t end_> StaticArrayView<end_ - begin_, T> slice() {
            return StaticArrayView<size_, T>(*this).template slice<begin_, end_>();
        }

        /**
         * @overload
         * @m_since{2019,10}
         */
        template<std::size_t begin_, std::size_t end_> StaticArrayView<end_ - begin_, const T> slice() const {
            return StaticArrayView<size_, const T>(*this).template slice<begin_, end_>();
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to @ref StaticArrayView::prefix(T*) const and overloads.
         */
        ArrayView<T> prefix(T* end) {
            return ArrayView<T>(*this).prefix(end);
        }
        /** @overload */
        ArrayView<const T> prefix(const T* end) const {
            return ArrayView<const T>(*this).prefix(end);
        }
        /** @overload */
        ArrayView<T> prefix(std::size_t end) {
            return ArrayView<T>(*this).prefix(end);
        }
        /** @overload */
        ArrayView<const T> prefix(std::size_t end) const {
            return ArrayView<const T>(*this).prefix(end);
        }

        /**
         * @brief Static array prefix
         *
         * Equivalent to @ref StaticArrayView::prefix(T*) const and overloads.
         */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> prefix();
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> prefix() const; /**< @overload */

        /**
         * @brief Array suffix
         *
         * Equivalent to @ref StaticArrayView::suffix(T*) const and overloads.
         */
        ArrayView<T> suffix(T* begin) {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @overload */
        ArrayView<const T> suffix(const T* begin) const {
            return ArrayView<const T>(*this).suffix(begin);
        }
        /** @overload */
        ArrayView<T> suffix(std::size_t begin) {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @overload */
        ArrayView<const T> suffix(std::size_t begin) const {
            return ArrayView<const T>(*this).suffix(begin);
        }

        /**
         * @brief Static array suffix
         * @m_since{2019,10}
         *
         * Equivalent to @ref StaticArrayView::suffix() const.
         */
        template<std::size_t begin_> StaticArrayView<size_ - begin_, T> suffix() {
            return StaticArrayView<size_, T>(*this).template suffix<begin_>();
        }

        /**
         * @overload
         * @m_since{2019,10}
         */
        template<std::size_t begin_> StaticArrayView<size_ - begin_, const T> suffix() const {
            return StaticArrayView<size_, const T>(*this).template suffix<begin_>();
        }

        /**
         * @brief Array prefix except the last @p count items
         *
         * Equivalent to @ref StaticArrayView::except(std::size_t) const.
         */
        ArrayView<T> except(std::size_t count) {
            return ArrayView<T>(*this).except(count);
        }
        /** @overload */
        ArrayView<const T> except(std::size_t count) const {
            return ArrayView<const T>(*this).except(count);
        }

        /**
         * @brief Static array prefix except the last @p count items
         *
         * Equivalent to @ref StaticArrayView::except() const.
         */
        template<std::size_t count> StaticArrayView<size_ - count, T> except() {
            return StaticArrayView<size_, T>(*this).template except<count>();
        }
        /** @overload */
        template<std::size_t count> StaticArrayView<size_ - count, const T> except() const {
            return StaticArrayView<size_, const T>(*this).template except<count>();
        }

    private:
        explicit StaticArray(DefaultInitT, std::true_type) {}
        /* GCC 5.3 is not able to initialize non-movable types inside
           constructor initializer list. Reported here:
           https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70395 */
        #if !defined(__GNUC__) || defined(__clang__)
        explicit StaticArray(DefaultInitT, std::false_type): _data{} {}
        #else
        explicit StaticArray(DefaultInitT, std::false_type) {
            for(T& i: _data) new(&i) T{};
        }
        #endif

        union {
            T _data[size_];
        };
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief One-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<1, T> @ce. See @ref StaticArray for
more information. Useful in case you want to take advantage of the @ref NoInit
tag on an arbitrary type and @ref Optional doesn't suit the use case.
@see @ref Array2, @ref Array3, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array1 = StaticArray<1, T>;

/**
@brief Two-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<2, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array3, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array2 = StaticArray<2, T>;

/**
@brief Three-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<3, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array2, @ref Array4, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array3 = StaticArray<3, T>;

/**
@brief Four-component array
@m_since_latest

Convenience alternative to @cpp StaticArray<4, T> @ce. See @ref StaticArray for
more information.
@see @ref Array1, @ref Array2, @ref Array3, @ref ArrayView2, @ref ArrayView3,
    @ref ArrayView4
*/
template<class T> using Array4 = StaticArray<4, T>;
#endif

/** @relatesalso StaticArray
@brief Make view on @ref StaticArray

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(StaticArray<size, T>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso StaticArray
@brief Make view on const @ref StaticArray

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView-const
*/
template<std::size_t size, class T> constexpr ArrayView<const T> arrayView(const StaticArray<size, T>& array) {
    return ArrayView<const T>{array};
}

/** @relatesalso StaticArray
@brief Make static view on @ref StaticArray

Convenience alternative to converting to an @ref StaticArrayView explicitly.
The following two lines are equivalent:

@snippet Containers.cpp StaticArray-staticArrayView

@todoc Make it a real reference once Doxygen is sane
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(StaticArray<size, T>& array) {
    return StaticArrayView<size, T>{array};
}

/** @relatesalso StaticArray
@brief Make static view on const @ref StaticArray

Convenience alternative to converting to an @ref StaticArrayView explicitly.
The following two lines are equivalent:

@snippet Containers.cpp StaticArray-staticArrayView-const

@todoc Make it a real reference once Doxygen is sane
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, const T> staticArrayView(const StaticArray<size, T>& array) {
    return StaticArrayView<size, const T>{array};
}

/** @relatesalso StaticArray
@brief Reinterpret-cast a static array

See @ref arrayCast(StaticArrayView<size, T>) for more information.
*/
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), U> arrayCast(StaticArray<size, T>& array) {
    return arrayCast<U>(staticArrayView(array));
}

/** @overload */
template<class U, std::size_t size, class T> StaticArrayView<size*sizeof(T)/sizeof(U), const U> arrayCast(const StaticArray<size, T>& array) {
    return arrayCast<const U>(staticArrayView(array));
}

/** @relatesalso StaticArray
@brief Static array size

See @ref arraySize(ArrayView<T>) for more information.
*/
template<std::size_t size_, class T> constexpr std::size_t arraySize(const StaticArray<size_, T>&) {
    return size_;
}

template<std::size_t size_, class T> template<class ...Args> StaticArray<size_, T>::StaticArray(DirectInitT, Args&&... args): StaticArray{NoInit} {
    for(T& i: _data)
        /* This works around a featurebug in C++ where new T{} doesn't work for
           an explicit defaulted constructor. Additionally it works around GCC
           4.8 bugs where copy/move construction can't be done with {} for
           plain structs. */
        Implementation::construct(i, std::forward<Args>(args)...);
}

template<std::size_t size_, class T> StaticArray<size_, T>::StaticArray(const StaticArray<size_, T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value): StaticArray{NoInit} {
    for(std::size_t i = 0; i != other.size(); ++i)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_data[i], other._data[i]);
        #else
        new(_data + i) T{other._data[i]};
        #endif
}

template<std::size_t size_, class T> StaticArray<size_, T>::StaticArray(StaticArray<size_, T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value): StaticArray{NoInit} {
    for(std::size_t i = 0; i != other.size(); ++i)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_data[i], std::move(other._data[i]));
        #else
        new(&_data[i]) T{std::move(other._data[i])};
        #endif
}

template<std::size_t size_, class T> StaticArray<size_, T>::~StaticArray() {
    for(T& i: _data) {
        i.~T();
        #ifdef CORRADE_MSVC2015_COMPATIBILITY
        /* Complains i is set but not used for trivially destructible types */
        static_cast<void>(i);
        #endif
    }
}

template<std::size_t size_, class T> StaticArray<size_, T>& StaticArray<size_, T>::operator=(const StaticArray<size_, T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value) {
    for(std::size_t i = 0; i != other.size(); ++i)
        _data[i] = other._data[i];
    return *this;
}

template<std::size_t size_, class T> StaticArray<size_, T>& StaticArray<size_, T>::operator=(StaticArray<size_, T>&& other) noexcept(std::is_nothrow_move_constructible<T>::value) {
    using std::swap;
    for(std::size_t i = 0; i != other.size(); ++i)
        swap(_data[i], other._data[i]);
    return *this;
}

template<std::size_t size_, class T> template<std::size_t viewSize> StaticArrayView<viewSize, T> StaticArray<size_, T>::prefix() {
    static_assert(viewSize <= size_, "prefix size too large");
    return StaticArrayView<viewSize, T>{_data};
}

template<std::size_t size_, class T> template<std::size_t viewSize> StaticArrayView<viewSize, const T> StaticArray<size_, T>::prefix() const {
    static_assert(viewSize <= size_, "prefix size too large");
    return StaticArrayView<viewSize, const T>{_data};
}

namespace Implementation {

/* StaticArray to ArrayView in order to have implicit conversion for
   StridedArrayView without needing to introduce a header dependency. The
   SFINAE needs to be here in order to ensure proper behavior with function
   overloads taking more than one type of (Strided)ArrayView. */
template<class U, std::size_t size, class T> struct ArrayViewConverter<U, StaticArray<size, T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<U>>::type from(StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<class U, std::size_t size, class T> struct ArrayViewConverter<const U, StaticArray<size, T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<const U>>::type from(const StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<class U, std::size_t size, class T> struct ArrayViewConverter<const U, StaticArray<size, const T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<const U>>::type from(const StaticArray<size, const T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<std::size_t size, class T> struct ErasedArrayViewConverter<StaticArray<size, T>>: ArrayViewConverter<T, StaticArray<size, T>> {};
template<std::size_t size, class T> struct ErasedArrayViewConverter<const StaticArray<size, T>>: ArrayViewConverter<const T, StaticArray<size, T>> {};

/* StaticArray to StaticArrayView in order to have implicit conversion for
   StridedArrayView without needing to introduce a header dependency. The
   SFINAE needs to be here in order to ensure proper behavior with function
   overloads taking more than one type of StaticArrayView. */
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, U, StaticArray<size, T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, StaticArrayView<size, U>>::type from(StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, T>{&other[0]};
    }
};
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, const U, StaticArray<size, T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, StaticArrayView<size, const U>>::type from(const StaticArray<size, T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, const T>(&other[0]);
    }
};
template<class U, std::size_t size, class T> struct StaticArrayViewConverter<size, const U, StaticArray<size, const T>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, StaticArrayView<size, const U>>::type from(const StaticArray<size, const T>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return StaticArrayView<size, const T>(&other[0]);
    }
};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<StaticArray<size, T>>: StaticArrayViewConverter<size, T, StaticArray<size, T>> {};
template<std::size_t size, class T> struct ErasedStaticArrayViewConverter<const StaticArray<size, T>>: StaticArrayViewConverter<size, const T, StaticArray<size, T>> {};

}

}}

#endif
