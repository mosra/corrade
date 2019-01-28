#ifndef Corrade_Containers_StaticArray_h
#define Corrade_Containers_StaticArray_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::StaticArray
 */

#include <new>
#include <type_traits>
#include <utility>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Tags.h"

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

The array is by default *default-initialized*, which means that trivial types
are not initialized at all and default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref StaticArray(DefaultInitT) is equivalent to the implicit parameterless
    constructor (useful when you want to make the choice appear explicit).
-   @ref StaticArray(InPlaceInitT, Args&&... args) is equivalent to the
    implicit parameteric constructor (again useful when you want to make the
    choice appear explicit). Same as @ref StaticArray(Args&&... args).
-   @ref StaticArray(ValueInitT) zero-initializes trivial types and calls
    default constructor elsewhere.
-   @ref StaticArray(DirectInitT, Args&&... args) constructs every element of
    the array using provided arguments.
-   @ref StaticArray(NoInitT) does not initialize anything and you need to call
    the constructor on all elements manually using placement new,
    @ref std::uninitialized_copy() or similar. This is the dangerous option.

Example:

@snippet Containers.cpp StaticArray-initialization

@see @ref arrayCast(StaticArray<size, T>&)
*/
/* Underscore at the end to avoid conflict with member size(). It's ugly, but
   having count instead of size_ would make the naming horribly inconsistent. */
template<std::size_t size_, class T> class StaticArray {
    public:
        enum: std::size_t {
            Size = size_    /**< Array size */
        };
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Construct default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e. builtin types are not initialized).
         * @see @ref DefaultInit, @ref StaticArray(ValueInitT)
         */
        explicit StaticArray(DefaultInitT): StaticArray{DefaultInit, std::integral_constant<bool, std::is_pod<T>::value>{}} {}

        /**
         * @brief Construct value-initialized array
         *
         * Creates array of given size, the contents are value-initialized
         * (i.e. builtin types are zero-initialized). For other than builtin
         * types this is the same as @ref StaticArray().
         *
         * Useful if you want to create an array of primitive types and set
         * them to zero.
         * @see @ref ValueInit, @ref StaticArray(DefaultInitT)
         */
        explicit StaticArray(ValueInitT): _data{} {}

        /**
         * @brief Construct the array without initializing its contents
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
         * @brief Construct direct-initialized array
         *
         * Constructs the array using the @ref StaticArray(NoInitT) constructor
         * and then initializes each element with placement new using forwarded
         * @p args.
         * @see @ref StaticArray(InPlaceInitT, Args&&... args)
         */
        template<class ...Args> explicit StaticArray(DirectInitT, Args&&... args);

        /**
         * @brief Construct in-place-initialized array
         *
         * The arguments are forwarded to the array constructor. Same as
         * @ref StaticArray(Args&&... args).
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        template<class ...Args> explicit StaticArray(InPlaceInitT, Args&&... args): _data{std::forward<Args>(args)...} {
            static_assert(sizeof...(args) == size_, "Containers::StaticArray: wrong number of initializers");
        }

        /**
         * @brief Construct default-initialized array
         *
         * Alias to @ref StaticArray(DefaultInitT).
         * @see @ref StaticArray(ValueInitT)
         */
        explicit StaticArray(): StaticArray{DefaultInit} {}

        /**
         * @brief Construct in-place-initialized array
         *
         * Alias to @ref StaticArray(InPlaceInitT, Args&&... args).
         * @see @ref StaticArray(DirectInitT, Args&&... args)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class ...Args> /*implicit*/ StaticArray(Args&&... args);
        #else
        template<class First, class ...Next> /*implicit*/ StaticArray(First&& first, Next&&... next): StaticArray{InPlaceInit, std::forward<First>(first), std::forward<Next>(next)...} {}
        #endif

        /** @brief Copying is not allowed */
        StaticArray(const StaticArray<size_, T>&) = delete;

        /** @brief Moving is not allowed */
        StaticArray(StaticArray<size_, T>&&) = delete;

        ~StaticArray();

        /** @brief Copying is not allowed */
        StaticArray<size_, T>& operator=(const StaticArray<size_, T>&) = delete;

        /** @brief Moving is not allowed */
        StaticArray<size_, T>& operator=(StaticArray<size_, T>&&) = delete;

        /* The following ArrayView conversion are *not* restricted to this&
           because that would break uses like `consume(foo());`, where
           `consume()` expects a view but `foo()` returns an owning array. */

        /**
         * @brief Convert to @ref ArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref arrayView(StaticArray<size, T>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<!std::is_void<U>::value && std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<U>() noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return {_data, size_};
        }

        /**
         * @brief Convert to const @ref ArrayView
         *
         * Enabled only if @cpp T* @ce or @cpp const T* @ce is implicitly
         * convertible to @cpp U* @ce. Expects that both types have the same
         * size.
         * @see @ref arrayView(const StaticArray<size, T>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value || std::is_convertible<T*, const U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<const U>() const noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return {_data, size_};
        }

        /** @overload */
        /*implicit*/ operator ArrayView<const void>() const noexcept {
            /* Yes, the size is properly multiplied by sizeof(T) by the constructor */
            return {_data, size_};
        }

        /**
         * @brief Convert to @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         * @see @ref staticArrayView(StaticArray<size, T>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator StaticArrayView<size_, U>() noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return StaticArrayView<size_, U>{_data};
        }

        /**
         * @brief Convert to const @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce or @cpp const T* @ce is implicitly
         * convertible to @cpp U* @ce. Expects that both types have the same
         * size.
         * @see @ref staticArrayView(const StaticArray<size, T>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value || std::is_convertible<T*, const U*>::value>::type>
        #endif
        /*implicit*/ operator StaticArrayView<size_, const U>() const noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return StaticArrayView<size_, const U>{_data};
        }

        /* `char* a = Containers::Array<char>(5); a[3] = 5;` would result in
           instant segfault, disallowing it in the following conversion
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
         * Equivalent to @ref ArrayView::slice().
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
         * @brief Fixed-size array slice
         *
         * Both @cpp begin @ce and @cpp begin + viewSize @ce are expected to be
         * in range.
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
         * @brief Array prefix
         *
         * Equivalent to @ref ArrayView::prefix().
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
         * Expects (at compile-time) that @p viewSize is not larger than
         * @ref Size.
         */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> prefix();
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> prefix() const; /**< @overload */

        /**
         * @brief Array suffix
         *
         * Equivalent to @ref ArrayView::suffix().
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

/** @relatesalso StaticArray
@brief Make view on @ref StaticArray

Convenience alternative to calling @ref StaticArray::operator ArrayView<U>()
explicitly. The following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(StaticArray<size, T>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso StaticArray
@brief Make view on const @ref StaticArray

Convenience alternative to calling @ref StaticArray::operator ArrayView<U>()
explicitly. The following two lines are equivalent:

@snippet Containers.cpp StaticArray-arrayView-const
*/
template<std::size_t size, class T> constexpr ArrayView<const T> arrayView(const StaticArray<size, T>& array) {
    return ArrayView<const T>{array};
}

/** @relatesalso StaticArray
@brief Make static view on @ref StaticArray

Convenience alternative to calling @cpp StaticArray::operator StaticArrayView<size_, U>() @ce
explicitly. The following two lines are equivalent:

@snippet Containers.cpp StaticArray-staticArrayView

@todoc Make it a real reference once Doxygen is sane
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(StaticArray<size, T>& array) {
    return StaticArrayView<size, T>{array};
}

/** @relatesalso StaticArray
@brief Make static view on const @ref StaticArray

Convenience alternative to calling @cpp StaticArray::operator StaticArrayView<size_, U>() @ce
explicitly. The following two lines are equivalent:

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
    for(T& i: _data) {
        /* MSVC 2015 needs the braces around */
        new(&i) T{std::forward<Args>(args)...};
    }
}

template<std::size_t size_, class T> StaticArray<size_, T>::~StaticArray() {
    for(T& i: _data) i.~T();
}

template<std::size_t size_, class T> template<std::size_t viewSize> StaticArrayView<viewSize, T> StaticArray<size_, T>::prefix() {
    static_assert(viewSize <= size_, "prefix size too large");
    return StaticArrayView<viewSize, T>{_data};
}

template<std::size_t size_, class T> template<std::size_t viewSize> StaticArrayView<viewSize, const T> StaticArray<size_, T>::prefix() const {
    static_assert(viewSize <= size_, "prefix size too large");
    return StaticArrayView<viewSize, const T>{_data};
}

}}

#endif
