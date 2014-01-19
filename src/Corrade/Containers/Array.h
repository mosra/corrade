#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014
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
 * @brief Class Corrade::Containers::Array
 */

#include <type_traits>
#include <utility>

#include "Corrade/configure.h"

namespace Corrade { namespace Containers {

/**
@brief %Array wrapper with size information

Provides movable RAII wrapper around plain C array. Main use case is storing
binary data of unspecified type, where addition/removal of elements is not
needed or harmful.

However, the class is usable also as lighter non-copyable alternative to
`std::vector`, in STL algorithms in the same way as plain C array and
additionally also in range-based for cycle.

Usage example:
@code
// Create default-initialized array with 5 integers and set them to some value
Containers::Array<int> a(5);
int b = 0;
for(auto& i: a) i = b++; // a = {0, 1, 2, 3, 4}

// Create array from given values
auto b = Containers::Array<int>::from(3, 18, -157, 0);
b[3] = 25; // b = {3, 18, -157, 25}
@endcode

@todo Something like ArrayTuple to create more than one array with single
    allocation and proper alignment for each type? How would non-POD types be
    constructed in that? Will that be useful in more than one place?
*/
template<class T> class Array {
    public:
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Create array from given values
         *
         * Zero argument count creates `nullptr` array.
         */
        template<class ...U> static Array<T> from(U&&... values) {
            return fromInternal(std::forward<U>(values)...);
        }

        /**
         * @brief Create zero-initialized array
         *
         * Creates array of given size, the values are value-initialized
         * (i.e. builtin types are zero-initialized). For other than builtin
         * types this is the same as Array(std::size_t). If the size is zero,
         * no allocation is done.
         */
        static Array<T> zeroInitialized(std::size_t size) {
            if(!size) return nullptr;

            Array<T> array;
            array._data = new T[size]();
            array._size = size;
            return array;
        }

        /** @brief Conversion from nullptr */
        /*implicit*/ Array(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        explicit Array() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Constructor
         *
         * Creates array of given size, the values are default-initialized (i.e.
         * builtin types are not initialized). If the size is zero, no
         * allocation is done.
         * @note Due to ambiguity you can't call directly `%Array(0)` because
         *      it conflicts with Array(std::nullptr_t). You should call
         *      `%Array(nullptr)` instead, which is also `noexcept`.
         * @see zeroInitialized()
         */
        explicit Array(std::size_t size): _data(size ? new T[size] : nullptr), _size(size) {}

        ~Array() { delete[] _data; }

        /** @brief Copying is not allowed */
        Array(const Array<T>&) = delete;

        /** @brief Move constructor */
        Array(Array<T>&& other) noexcept;

        /** @brief Copying is not allowed */
        Array<T>& operator=(const Array<T>&) = delete;

        /** @brief Move assignment */
        Array<T>& operator=(Array<T>&&) noexcept;

        /** @brief Whether the array is non-empty */
        explicit operator bool() const { return _data; }

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
        /*implicit*/ operator const T*() const { return _data; }

        /** @brief %Array data */
        T* data() { return _data; }
        const T* data() const { return _data; }         /**< @overload */

        /** @brief %Array size */
        std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        bool empty() const { return !_size; }

        /** @brief Pointer to first element */
        T* begin() { return _data; }
        const T* begin() const { return _data; }        /**< @overload */
        const T* cbegin() const { return _data; }       /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() { return _data+_size; }
        const T* end() const { return _data+_size; }    /**< @overload */
        const T* cend() const { return _data+_size; }   /**< @overload */

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets internal state to default.
         * Deleting the returned array is user responsibility.
         */
        T* release();

    private:
        template<class ...U> static Array<T> fromInternal(U&&... values) {
            Array<T> array;
            array._size = sizeof...(values);
            array._data = new T[sizeof...(values)] { std::forward<U>(values)... };
            return array;
        }
        /* Specialization for zero argument count */
        static Array<T> fromInternal() { return nullptr; }

        T* _data;
        std::size_t _size;
};

/**
@brief %Array reference wrapper with size information

Immutable wrapper around plain C array. Unlike @ref Array this class doesn't do
any memory management. Main use case is passing array along with size
information to functions etc. If @p T is `const` type, the class is implicitly
constructible also from const references to @ref Array and @ref ArrayReference
of non-const types.

Usage example:
@code
// `a` gets implicitly converted to const array reference
void printArray(Containers::ArrayReference<const float> values) { ... }
Containers::Array<float> a;
printArray(a);

// Wrapping compile-time array with size information
constexpr const int data[] = {5, 17, -36, 185};
Containers::ArrayReference<const int> b =
    {data, std::extent<decltype(data)>()}; // b.size() == 4
@endcode

@see @ref ArrayReference<const void>
@todo What was the reason for no const-correctness at all?
*/
template<class T> class ArrayReference {
    public:
        typedef T Type;     /**< @brief Element type */

        /** @brief Conversion from nullptr */
        constexpr /*implicit*/ ArrayReference(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates empty reference. Copy non-empty Array/ArrayReference onto
         * the instance to make it useful.
         */
        constexpr explicit ArrayReference() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Constructor
         * @param data      Data pointer
         * @param size      Data size
         */
        constexpr /*implicit*/ ArrayReference(T* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Construct reference to fixed-size array
         * @param data      Fixed-size array
         */
        #ifdef CORRADE_GCC46_COMPATIBILITY
        #define size size_ /* With GCC 4.6 it conflicts with size(). WTF. */
        #endif
        template<std::size_t size> constexpr /*implicit*/ ArrayReference(T(&data)[size]) noexcept: _data(data), _size(size) {}
        #ifdef CORRADE_GCC46_COMPATIBILITY
        #undef size
        #endif

        /** @brief Construct reference to Array */
        constexpr /*implicit*/ ArrayReference(Array<T>& array) noexcept: _data(array), _size(array.size()) {}

        /**
         * @brief Construct const reference to Array
         *
         * Enabled only if @p T is `const U`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_same<const U, T>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayReference(const Array<U>& array) noexcept: _data(array), _size(array.size()) {}

        /**
         * @brief Construct const reference from non-const reference
         *
         * Enabled only if @p T is `const U`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_same<const U, T>::value>::type>
        #endif
        constexpr /*implicit*/ ArrayReference(const ArrayReference<U>& array) noexcept: _data(array), _size(array.size()) {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief %Array data */
        constexpr const T* data() const { return _data; }

        /** @brief %Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /** @brief Pointer to first element */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() const { return _data+_size; }
        T* cend() const { return _data+_size; }         /**< @overload */

    private:
        T* _data;
        std::size_t _size;
};

/**
@brief Constant void array reference wrapper with size information

Specialization of @ref ArrayReference which is convertible from @ref Array or
@ref ArrayReference of any type. Size for particular type is recalculated to
size in bytes. This specialization doesn't provide any `begin()`/`end()`
accessors, because it has no use for `void` type.

Usage example:
@code
Containers::Array<int> a(5);

Containers::ArrayReference<const void> b(a); // b.size() == 20
@endcode
*/
template<> class ArrayReference<const void> {
    public:
        typedef const void Type;     /**< @brief Element type */

        /** @brief Conversion from nullptr */
        constexpr /*implicit*/ ArrayReference(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        constexpr explicit ArrayReference() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Constructor
         * @param data      Data pointer
         * @param size      Data size
         */
        constexpr /*implicit*/ ArrayReference(const void* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Constructor from any type
         * @param data      Data pointer
         * @param size      Data size
         *
         * Size is recalculated to size in bytes.
         */
        template<class T> constexpr /*implicit*/ ArrayReference(const T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct reference to fixed-size array
         * @param data      Fixed-size array
         *
         * Size in bytes is calculated automatically.
         */
        #ifdef CORRADE_GCC46_COMPATIBILITY
        #define size size_ /* With GCC 4.6 it conflicts with size(). WTF. */
        #endif
        template<class T, std::size_t size> constexpr /*implicit*/ ArrayReference(T(&data)[size]) noexcept: _data(data), _size(size*sizeof(T)) {}
        #ifdef CORRADE_GCC46_COMPATIBILITY
        #undef size
        #endif

        /** @brief Construct const void reference to any Array */
        template<class T> constexpr /*implicit*/ ArrayReference(const Array<T>& array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Construct const void reference to any ArrayReference */
        template<class T> constexpr /*implicit*/ ArrayReference(const ArrayReference<T>& array) noexcept: _data(array), _size(array.size()*sizeof(T)) {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator const void*() const { return _data; }

        /** @brief %Array data */
        constexpr const void* data() const { return _data; }

        /** @brief %Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

    private:
        const void* _data;
        std::size_t _size;
};

template<class T> inline Array<T>::Array(Array<T>&& other) noexcept: _data(other._data), _size(other._size) {
    other._data = nullptr;
    other._size = 0;
}

template<class T> inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    return *this;
}

template<class T> inline T* Array<T>::release() {
    /** @todo I need `std::exchange` NOW. */
    T* const data = _data;
    _data = nullptr;
    _size = 0;
    return data;
}

}}

#endif
