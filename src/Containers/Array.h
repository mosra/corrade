#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#include "corradeCompatibility.h"

namespace Corrade { namespace Containers {

/**
@brief %Array wrapper with size information

Provides movable RAII wrapper around plain C array. Main use case is storing
binary data of unspecified type, where direct element access might be harmful.

However, the class is usable also as lighter non-copyable alternative to
`std::vector`, in STL algorithms in the same way as plain C array and
additionally also in range-based for cycle.
*/
template<class T> class Array {
    public:
        typedef T Type;     /**< @brief Element type */

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

        #ifndef CORRADE_GCC45_COMPATIBILITY
        /** @brief Conversion from nullptr */
        /*implicit*/ Array(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}
        #endif

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        /* implicit where nullptr is not supported, as explicitly specifying
           the type is much less convenient than simply typing nullptr */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        explicit
        #endif
        Array() noexcept: _data(nullptr), _size(0) {}

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
        /* Special "copymove" constructor to work around issues with std::map */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        Array(const Array<T>&) = delete;
        #else
        Array(const Array<T>& other);
        #endif

        /** @brief Move constructor */
        Array(Array<T>&& other) noexcept;

        /** @brief Copying is not allowed */
        Array<T>& operator=(const Array<T>&) = delete;

        /** @brief Move assignment */
        Array<T>& operator=(Array<T>&&) noexcept;

        /** @brief Whether the array is empty */
        bool empty() const { return !_size; }

        /** @brief %Array size */
        std::size_t size() const { return _size; }

        /** @brief Pointer to first element */
        T* begin() { return _data; }
        const T* begin() const { return _data; }        /**< @overload */
        const T* cbegin() const { return _data; }       /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() { return _data+_size; }
        const T* end() const { return _data+_size; }    /**< @overload */
        const T* cend() const { return _data+_size; }   /**< @overload */

        /** @brief Conversion to array type */
        operator T*() { return _data; }
        operator const T*() const { return _data; }     /**< @overload */

    private:
        T* _data;
        std::size_t _size;
};

/**
@brief %Array reference wrapper with size information

Immutable wrapper around plain C array. Unlike Array this class doesn't do any
memory management. Main use case is passing array around along with size
information. If @p T is `const` type, the class is implicitly constructible
also from const references to Array and ArrayReference of non-const types.
*/
template<class T> class ArrayReference {
    public:
        typedef T Type;     /**< @brief Element type */

        #ifndef CORRADE_GCC45_COMPATIBILITY
        /** @brief Conversion from nullptr */
        constexpr /*implicit*/ ArrayReference(std::nullptr_t) noexcept: _data(nullptr), _size(0) {}
        #endif

        /**
         * @brief Default constructor
         *
         * Creates empty reference. Copy non-empty Array/ArrayReference onto
         * the instance to make it useful.
         */
        constexpr
        /* implicit where nullptr is not supported, as explicitly specifying
           the type is much less convenient than simply typing nullptr */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        explicit
        #endif
        ArrayReference() noexcept: _data(nullptr), _size(0) {}

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

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /** @brief %Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Pointer to first element */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() const { return _data+_size; }
        T* cend() const { return _data+_size; }         /**< @overload */

        /** @brief Conversion to array type */
        constexpr operator T*() const { return _data; }

    private:
        T* _data;
        std::size_t _size;
};

/**
@brief Constant void array reference wrapper with size information

Specialization of ArrayReference, which is convertible from Array or
ArrayReference of any type, size for particular type is recalculated to size in
bytes. This specialization doesn't provide any `begin()`/`end()` accessors,
because it has no use for `void` type.
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
        template<class T> constexpr /*implicit*/ ArrayReference(const T* data, std::size_t size) noexcept: _data(data), _size(size*sizeof(T)) {}

        /**
         * @brief Construct reference to fixed-size array
         * @param data      Fixed-size array
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

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /** @brief %Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Conversion to array type */
        constexpr operator const void*() const { return _data; }

    private:
        const void* _data;
        std::size_t _size;
};

template<class T> inline Array<T>::Array(Array<T>&& other) noexcept: _data(other._data), _size(other._size) {
    other._data = nullptr;
    other._size = 0;
}

/* Special "copymove" constructor to work around issues with std::map */
#ifdef CORRADE_GCC45_COMPATIBILITY
template<class T> inline Array<T>::Array(const Array<T>& other): _data(other._data), _size(other._size) {
    const_cast<Array<T>&>(other)._data = nullptr;
    const_cast<Array<T>&>(other)._size = 0;
}
#endif

template<class T> inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    return *this;
}

}}

#endif
