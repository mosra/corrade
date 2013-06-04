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

#include <utility>

#include "corradeCompatibility.h"

namespace Corrade { namespace Containers {

/**
@brief %Array wrapper with size information

Provides moveable RAII wrapper around plain C array. Usable in STL algorithms
in the same way as plain C array and additionally also in range-based for cycle.
*/
template<class T> class Array {
    public:
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        explicit Array() noexcept: _data(nullptr), _size(0) {}
        #else
        explicit Array(): _data(nullptr), _size(0) {}
        #endif

        /**
         * @brief Constructor
         *
         * Creates array of given size, the values are default-initialized (i.e.
         * builtin types are not initialized).
         */
        explicit Array(std::size_t size): _data(size ? new T[size] : nullptr), _size(size) {}

        ~Array() { delete[] _data; }

        /** @brief Copying is not allowed */
        Array(const Array<T>&) = delete;

        /** @brief Move constructor */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        Array(Array<T>&& other) noexcept;
        #else
        Array(Array<T>&& other);
        #endif

        /** @brief Copying is not allowed */
        Array<T>& operator=(const Array<T>&) = delete;

        /** @brief Move assignment */
        #ifndef CORRADE_GCC45_COMPATIBILITY
        Array<T>& operator=(Array<T>&&) noexcept;
        #else
        Array<T>& operator=(Array<T>&&);
        #endif

        /** @brief Whether the array is empty */
        bool empty() const { return !_size; }

        /** @brief %Array size */
        std::size_t size() const { return _size; }

        /** @brief Pointer to first element */
        T* begin() { return _data; }
        const T* begin() const { return _data; }       /**< @overload */
        const T* cbegin() const { return _data; }      /**< @overload */

        /** @brief Pointer to (one item after) last element */
        T* end() { return _data+_size; }
        const T* end() const { return _data+_size; }   /**< @overload */
        const T* cend() const { return _data+_size; }  /**< @overload */

        /** @brief Conversion to array type */
        operator T*() { return _data; }
        operator const T*() const { return _data; } /**< @overload */

        /** @brief Element access */
        T& operator[](std::size_t i) { return _data[i]; }
        const T& operator[](std::size_t i) const { return _data[i]; } /**< @overload */

    private:
        T* _data;
        std::size_t _size;
};

#ifndef CORRADE_GCC45_COMPATIBILITY
template<class T> inline Array<T>::Array(Array<T>&& other) noexcept:
#else
template<class T> inline Array<T>::Array(Array<T>&& other):
#endif
    _data(other._data), _size(other._size)
{
    other._data = nullptr;
    other._size = 0;
}

#ifndef CORRADE_GCC45_COMPATIBILITY
template<class T> inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept
#else
template<class T> inline Array<T>& Array<T>::operator=(Array<T>&& other)
#endif
{
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    return *this;
}

}}

#endif
