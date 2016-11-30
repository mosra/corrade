#ifndef Corrade_Containers_StaticArray_h
#define Corrade_Containers_StaticArray_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016
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
 * @brief Class @ref Corrade::Containers::StaticArray
 */

#include <type_traits>
#include <utility>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Tags.h"

#ifdef CORRADE_BUILD_DEPRECATED
#include "Corrade/Utility/Macros.h"
#endif

namespace Corrade { namespace Containers {

/**
@brief Static array wrapper
@tparam T       Element type
@tparam size    Array size

Provides statically-sized array wrapper with API similar to @ref Array. Useful
as more featureful alternative to plain C arrays or `std::array`.

Usage example:
@code
// Create default-initialized array with 5 integers and set them to some value
Containers::Array<int> a{5};
int b = 0;
for(auto& i: a) i = b++; // a = {0, 1, 2, 3, 4}

// Create array from given values
auto b = Containers::Array<int>::from(3, 18, -157, 0);
b[3] = 25; // b = {3, 18, -157, 25}
@endcode

## Array initialization

The array is by default *default-initialized*, which means that trivial types
are not initialized at all and default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref StaticArray(DefaultInitT) is equivalent to the default case (useful
    when you want to make the choice appear explicit).
-   @ref StaticArray(ValueInitT) zero-initializes trivial types and calls
    default constructor elsewhere.
-   @ref StaticArray(DirectInitT, Args...) constructs all elements of the array
    using provided arguments.
-   @ref StaticArray(NoInitT) does not initialize anything and you need to call
    the constructor on all elements manually using placement new,
    `std::uninitialized_copy` or similar. This is the dangerous option.

Example:
@code
// These are equivalent
Containers::StaticArray<5, int> a1{5};
Containers::StaticArray<5, int> a2{Containers::DefaultInit};

// Array of 100 zeros
Containers::StaticArray<100, int> b{Containers::ValueInit};

// Array of type with no default constructor
struct Vec3 {
    Vec3(float, float, float);
};
Containers::StaticArray<5, Vec3> c{Containers::DirectInit, 5.2f, 0.4f, 1.0f};

// Manual construction of each element
struct Foo {
    Foo(int index);
};
Containers::StaticArray<5, Foo> d{Containers::NoInit};
int index = 0;
for(Foo& f: d) new(&f) Foo(index++);
@endcode
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
         * @see @ref NoInit, @ref StaticArray(DirectInitT, Args...)
         */
        explicit StaticArray(NoInitT) {}

        /**
         * @brief Construct direct-initialized array
         *
         * Constructs the array using the @ref StaticArray(NoInitT) "StaticArray(NoInitT)"
         * constructor and then initializes each element with placement new
         * using forwarded @p arguments.
         * @see @ref StaticArray(InPlaceInitT, Args&&...)
         */
        template<class ...Args> explicit StaticArray(DirectInitT, Args&&... args);

        /**
         * @brief Construct array in-place
         *
         * The arguments are forwarded to the array constructor.
         * @see @ref StaticArray(DirectInitT, Args&&...)
         */
        template<class ...Args> explicit StaticArray(InPlaceInitT, Args&&... args): _data{std::forward<Args>(args)...} {}

        /**
         * @brief Construct default-initialized array
         *
         * Alias to @ref StaticArray(DefaultInitT).
         * @see @ref StaticArray(ValueInitT)
         */
        explicit StaticArray(): StaticArray{DefaultInit} {}

        /** @brief Copying is not allowed */
        StaticArray(const StaticArray<size_, T>&) = delete;

        /** @brief Moving is not allowed */
        StaticArray(StaticArray<size_, T>&&) = delete;

        ~StaticArray();

        /** @brief Copying is not allowed */
        StaticArray<size_, T>& operator=(const StaticArray<size_, T>&) = delete;

        /** @brief Moving is not allowed */
        StaticArray<size_, T>& operator=(StaticArray<size_, T>&&) = delete;

        /**
         * @brief Convert to @ref ArrayView
         *
         * Enabled only if `T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<U>() noexcept { return {_data, size_}; }

        /**
         * @brief Convert to const @ref ArrayView
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<const U>() const noexcept { return {_data, size_}; }

        /**
         * @brief Convert to @ref StaticArrayView
         *
         * Enabled only if `T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator StaticArrayView<size_, U>() noexcept {
            return StaticArrayView<size_, U>{_data};
        }

        /**
         * @brief Convert to const @ref ArrayView
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator StaticArrayView<size_, const U>() const noexcept {
            return StaticArrayView<size_, const U>{_data};
        }

        /* `char* a = Containers::Array<char>(5); a[3] = 5;` would result in
           instant segfault, disallowing it in the following conversion
           operators */

        /** @brief Conversion to array type */
        /*implicit*/ operator T*()
        #ifndef CORRADE_GCC47_COMPATIBILITY
        &
        #endif
        { return _data; }

        /** @overload */
        /*implicit*/ operator const T*() const
        #ifndef CORRADE_GCC47_COMPATIBILITY
        &
        #endif
        { return _data; }

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

        /** @brief Pointer to first element */
        T* begin() { return _data; }
        const T* begin() const { return _data; }            /**< @overload */
        const T* cbegin() const { return _data; }           /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() { return _data + size_; }
        const T* end() const { return _data + size_; }      /**< @overload */
        const T* cend() const { return _data + size_; }     /**< @overload */

        /**
         * @brief Reference to array slice
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
            return slice(_data + begin, _data + end);
        }
        /** @overload */
        ArrayView<const T> slice(std::size_t begin, std::size_t end) const {
            return slice(_data + begin, _data + end);
        }

        /**
         * @brief Fixed-size array slice
         *
         * Both @p begin and `begin + viewSize` are expected to be in range.
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
            return slice<viewSize>(_data + begin);
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> slice(std::size_t begin) const {
            return slice<viewSize>(_data + begin);
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
        ArrayView<T> prefix(std::size_t end) { return prefix(_data + end); } /**< @overload */
        ArrayView<const T> prefix(std::size_t end) const { return prefix(_data + end); } /**< @overload */

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
        ArrayView<T> suffix(std::size_t begin) { return suffix(_data + begin); } /**< @overload */
        ArrayView<const T> suffix(std::size_t begin) const { return suffix(_data + begin); } /**< @overload */

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

template<std::size_t size_, class T> template<class ...Args> StaticArray<size_, T>::StaticArray(DirectInitT, Args&&... args): StaticArray{NoInit} {
    for(T& i: _data) {
        /* MSVC 2015 needs the braces around */
        new(&i) T{std::forward<Args>(args)...};
    }
}

template<std::size_t size_, class T> StaticArray<size_, T>::~StaticArray() {
    for(T& i: _data) i.~T();
}

}}

#endif
