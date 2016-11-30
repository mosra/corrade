#ifndef Corrade_Containers_ArrayView_h
#define Corrade_Containers_ArrayView_h
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
 * @brief Class @ref Corrade::Containers::ArrayView, @ref Corrade::Containers::StaticArrayView
 */

#include <type_traits>
#include <utility>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Containers {

/**
@brief Array view with size information

Immutable wrapper around continuous range of data. Unlike @ref Array this class
doesn't do any memory management. Main use case is passing array along with
size information to functions etc. If @p T is `const` type, the class is
implicitly constructible also from const references to @ref Array and
@ref ArrayView of non-const types.

Usage example:
@code
// `a` gets implicitly converted to const array view
void printArray(Containers::ArrayView<const float> values) { ... }
Containers::Array<float> a;
printArray(a);

// Wrapping compile-time array with size information
constexpr const int data[] = {5, 17, -36, 185};
Containers::ArrayView<const int> b = data; // b.size() == 4

// Wrapping general array with size information
const int* data2;
Containers::ArrayView<const int> c{data2, 3};
@endcode

@attention Note that when using `Containers::ArrayView<const char>`, C string
    literals (such as `"hello"`) are implicitly convertible to it and the size
    includes also the zero-terminator (thus in case of `"hello"` the size would
    be 6, not 5, as one might expect).

@see @ref ArrayView<const void>, @ref StaticArrayView
@todo What was the reason for no const-correctness at all?
*/
template<class T> class ArrayView {
    public:
        typedef T Type;     /**< @brief Element type */

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ ArrayView(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates empty view. Copy non-empty @ref Array or @ref ArrayView onto
         * the instance to make it useful.
         */
        constexpr /*implicit*/ ArrayView() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Constructor
         * @param data      Data pointer
         * @param size      Data size
         */
        constexpr /*implicit*/ ArrayView(T* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Construct view of fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U, std::size_t size>
        #else
        template<class U, std::size_t size, class V = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(U(&data)[size]) noexcept: _data{data}, _size{size} {}

        /**
         * @brief Construct view on @ref ArrayView
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(ArrayView<U> array) noexcept: _data{array}, _size{array.size()} {}

        /**
         * @brief Construct view on @ref StaticArrayView
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size, class U>
        #else
        template<std::size_t size, class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(StaticArrayView<size, U> array) noexcept: _data{array}, _size{size} {}

        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2015 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief Array data */
        constexpr T* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /** @brief Pointer to first element */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() const { return _data+_size; }
        T* cend() const { return _data+_size; }         /**< @overload */

        /**
         * @brief Array slice
         *
         * Both arguments are expected to be in range.
         */
        ArrayView<T> slice(T* begin, T* end) const;

        /** @overload */
        ArrayView<T> slice(std::size_t begin, std::size_t end) const {
            return slice(_data + begin, _data + end);
        }

        /**
         * @brief Fixed-size array slice
         *
         * Both @p begin and `begin + size` are expected to be in range.
         */
        template<std::size_t size> StaticArrayView<size, T> slice(T* begin) const;

        /** @overload */
        template<std::size_t size> StaticArrayView<size, T> slice(std::size_t begin) const {
            return slice<size>(_data + begin);
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to `data.slice(data.begin(), end)`. If @p end is
         * `nullptr`, returns zero-sized `nullptr` array.
         */
        ArrayView<T> prefix(T* end) const {
            if(!end) return nullptr;
            return slice(_data, end);
        }
        ArrayView<T> prefix(std::size_t end) const { return prefix(_data + end); } /**< @overload */

        /**
         * @brief Array suffix
         *
         * Equivalent to `data.slice(begin, data.end())`. If @p begin is
         * `nullptr` and the original array isn't, returns zero-sized `nullptr`
         * array.
         */
        ArrayView<T> suffix(T* begin) const {
            if(_data && !begin) return nullptr;
            return slice(begin, _data + _size);
        }
        ArrayView<T> suffix(std::size_t begin) const { return suffix(_data + begin); } /**< @overload */

    private:
        T* _data;
        std::size_t _size;
};

/**
@brief Constant void array view with size information

Specialization of @ref ArrayView which is convertible from @ref Array or
@ref ArrayView of any type. Size for particular type is recalculated to
size in bytes. This specialization doesn't provide any `begin()`/`end()`
accessors, because it has no use for `void` type.

Usage example:
@code
Containers::Array<int> a(5);

Containers::ArrayView<const void> b(a); // b.size() == 20
@endcode
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
         * @brief Constructor
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
         * @brief Construct view on fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        template<class T, std::size_t size> constexpr /*implicit*/ ArrayView(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}

        /** @brief Construct const void view on any @ref ArrayView */
        template<class T> constexpr /*implicit*/ ArrayView(const ArrayView<T>& array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct const void view on any @ref StaticArrayView */
        template<std::size_t size, class T> constexpr /*implicit*/ ArrayView(const StaticArrayView<size, T>& array) noexcept: _data{array}, _size{size*sizeof(T)} {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

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

/**
@brief Fixed-size array view

Equivalent to @ref ArrayView, but with compile-time size information.
Convertible from and to @ref ArrayView. Example usage:
@code
Containers::ArrayView<int> data;

// Take elements 7 to 11
Containers::StaticArrayView<5, int> fiveInts = data.slice<5>(7);

// Convert back to ArrayView
Containers::ArrayView<int> fiveInts2 = data; // fiveInts2.size() == 5
Containers::ArrayView<int> threeInts = data.slice(2, 5);
@endcode
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
         * @brief Constructor
         * @param data      Data pointer
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        constexpr explicit StaticArrayView(T* data)
        #else
        template<class U, class = typename std::enable_if<!std::is_same<U, T(&)[size_]>::value>::type> constexpr explicit StaticArrayView(U data)
        #endif
        noexcept: _data(data) {}

        /**
         * @brief Construct view of fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`. Note
         * that, similarly as with raw pointers, you need to ensure that both
         * types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StaticArrayView(U(&data)[size_]) noexcept: _data{data} {}

        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2015 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief Array data */
        constexpr T* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return size_; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !size_; }

        /** @brief Pointer to first element */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() const { return _data + size_; }
        T* cend() const { return _data + size_; }       /**< @overload */

        /** @copydoc ArrayView::slice(T*, T*) const */
        ArrayView<T> slice(T* begin, T* end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<T>(*this).slice(begin, end);
        }

        /** @copydoc ArrayView::slice(T*) const */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(T* begin) const {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(std::size_t begin) const {
            return ArrayView<T>(*this).template slice<viewSize>(begin);
        }

        /** @copydoc ArrayView::prefix(T*) const */
        ArrayView<T> prefix(T* end) const {
            return ArrayView<T>(*this).prefix(end);
        }
        /** @overload */
        ArrayView<T> prefix(std::size_t end) const {
            return ArrayView<T>(*this).prefix(end);
        }

        /** @copydoc ArrayView::suffix(T*) const */
        ArrayView<T> suffix(T* begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @overload */
        ArrayView<T> suffix(std::size_t begin) const {
            return ArrayView<T>(*this).suffix(begin);
        }

    private:
        T* _data;
};

template<class T> ArrayView<T> ArrayView<T>::slice(T* begin, T* end) const {
    CORRADE_ASSERT(_data <= begin && begin <= end && end <= _data + _size,
        "Containers::ArrayView::slice(): slice out of range", nullptr);
    return ArrayView<T>{begin, std::size_t(end - begin)};
}

template<class T> template<std::size_t size> StaticArrayView<size, T> ArrayView<T>::slice(T* begin) const {
    CORRADE_ASSERT(_data <= begin && begin + size <= _data + _size,
        "Containers::ArrayView::slice(): slice out of range", nullptr);
    return StaticArrayView<size, T>{begin};
}

}}

#endif
