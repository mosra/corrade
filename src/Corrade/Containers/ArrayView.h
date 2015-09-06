#ifndef Corrade_Containers_ArrayView_h
#define Corrade_Containers_ArrayView_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
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
 * @brief Class @ref Corrade::Containers::ArrayView
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

@see @ref ArrayView<const void>
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
         * Enabled only if `U*` is implicitly convertible to `T*`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U, std::size_t size>
        #else
        template<class U, std::size_t size, class V = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(U(&data)[size]) noexcept: _data{data}, _size{size} {}

        /**
         * @brief Construct view of @ref ArrayView
         *
         * Enabled only if `U*` is implicitly convertible to `T*`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayView(ArrayView<U> array) noexcept: _data{array}, _size{array.size()} {}

        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2015 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        explicit operator bool() const { return _data; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief Array data */
        constexpr const T* data() const { return _data; }

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
        ArrayView<T> slice(std::size_t begin, std::size_t end) {
            return slice(_data + begin, _data + end);
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

template<class T> ArrayView<T> ArrayView<T>::slice(T* begin, T* end) const {
    CORRADE_ASSERT(_data <= begin && begin <= end && end <= _data + _size,
        "Containers::ArrayView::slice(): slice out of range", nullptr);
    return ArrayView<T>{begin, std::size_t(end - begin)};
}

}}

#endif
