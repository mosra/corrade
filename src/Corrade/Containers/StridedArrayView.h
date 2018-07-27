#ifndef Corrade_Containers_StridedArrayView_h
#define Corrade_Containers_StridedArrayView_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::StridedArrayView, @ref Corrade::Containers::StridedIterator
 */

#include <type_traits>
#include <utility>

#include "Corrade/Containers/ArrayView.h"

namespace Corrade { namespace Containers {

/**
@brief Array view with size and stride information

Immutable wrapper around continuous sparse range of data, useful for easy
iteration over interleaved arrays. Usage example:

@snippet Containers.cpp StridedArrayView-usage

For convenience, similarly to @ref ArrayView, this class is implicitly
convertible from plain C arrays, @ref ArrayView and
@link StaticArrayView @endlink, with stride equal to array type size.

@snippet Containers.cpp StridedArrayView-usage-conversion

Unlike @ref ArrayView, this wrapper doesn't provide direct pointer access
because pointer arithmetic doesn't work as usual here.
@see @ref StridedIterator
*/
/* All member functions are const because the view doesn't own the data */
template<class T> class StridedArrayView {
    public:
        typedef T Type;     /**< @brief Element type */

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ StridedArrayView(std::nullptr_t) noexcept: _data{}, _size{}, _stride{} {}

        /**
         * @brief Default constructor
         *
         * Creates empty view. Copy non-empty @ref Array or @ref ArrayView onto
         * the instance to make it useful.
         */
        constexpr /*implicit*/ StridedArrayView() noexcept: _data{}, _size{}, _stride{} {}

        /**
         * @brief Construct view on an array with explicit length
         * @param data      Data pointer
         * @param size      Data size
         * @param stride    Data stride
         */
        /*implicit*/ StridedArrayView(T* data, std::size_t size, std::size_t stride) noexcept: _data{reinterpret_cast<const char*>(data)}, _size{size}, _stride{stride} {}

        /**
         * @brief Construct view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size; stride is implicitly set
         * to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U, std::size_t size>
        #else
        template<class U, std::size_t size, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        /*implicit*/ StridedArrayView(U(&data)[size]) noexcept: _data{reinterpret_cast<const char*>(data)}, _size{size}, _stride{sizeof(T)} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct view on @ref StridedArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size; stride is implicitly set
         * to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        /*implicit*/ StridedArrayView(StridedArrayView<U> view) noexcept: _data{reinterpret_cast<const char*>(view.data())}, _size{view.size()}, _stride{view.stride()} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct view on @ref ArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size; stride is implicitly set
         * to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        /*implicit*/ StridedArrayView(ArrayView<U> view) noexcept: _data{reinterpret_cast<const char*>(view.data())}, _size{view.size()}, _stride{sizeof(T)} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct view on @ref StaticArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size; stride is implicitly set
         * to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size, class U>
        #else
        template<std::size_t size, class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        /*implicit*/ StridedArrayView(StaticArrayView<size, U> view) noexcept: _data{reinterpret_cast<const char*>(view.data())}, _size{size}, _stride{sizeof(T)} {
            static_assert(sizeof(U) == sizeof(T), "type sizes are not compatible");
        }

        /** @brief Whether the array is non-empty */
        explicit operator bool() const { return _data; }

        /** @brief Array data */
        constexpr const void* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Array stride */
        constexpr std::size_t stride() const { return _stride; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /** @brief Element access */
        T& operator[](std::size_t i) const {
            return *const_cast<T*>(reinterpret_cast<const T*>(_data + i*_stride));
        }

        /**
         * @brief Iterator to first element
         *
         * @see @ref front()
         */
        constexpr StridedIterator<T> begin() const { return {_data, _stride}; }
        /** @overload */
        constexpr StridedIterator<T> cbegin() const { return {_data, _stride}; }

        /**
         * @brief Iterator to (one item after) last element
         *
         * @see @ref back()
         */
        StridedIterator<T> end() const { return {_data+_size*_stride, _stride}; }
        /** @overload */
        StridedIterator<T> cend() const { return {_data+_size*_stride, _stride}; }

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
        StridedArrayView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(0, end) @ce. If @p end is @cpp 0 @ce,
         * returns zero-sized @cpp nullptr @ce array.
         */
        StridedArrayView<T> prefix(std::size_t end) const {
            if(!end) return nullptr;
            return slice(0, end);
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice(begin, data.size()) @ce.
         */
        StridedArrayView<T> suffix(std::size_t begin) const {
            return slice(begin, _size);
        }

    private:
        const char* _data;
        std::size_t _size, _stride;
};

/**
@brief Strided array view iterator

Used by @ref StridedArrayView to provide iterator access to its items.
*/
template<class T> class StridedIterator {
    public:
        #ifndef DOXYGEN_GENERATING_OUTPUT
        constexpr /*implicit*/ StridedIterator(const char* i, std::size_t stride): _i{i}, _stride{stride} {}
        #endif

        /** @brief Equality comparison */
        constexpr bool operator==(StridedIterator<T> other) const {
            return _i == other._i;
        }

        /** @brief Non-equality comparison */
        constexpr bool operator!=(StridedIterator<T> other) const {
            return _i != other._i;
        }

        /** @brief Less than comparison */
        constexpr bool operator<(StridedIterator<T> other) const {
            return _i < other._i;
        }

        /** @brief Less than or equal comparison */
        constexpr bool operator<=(StridedIterator<T> other) const {
            return _i <= other._i;
        }

        /** @brief Greater than comparison */
        constexpr bool operator>(StridedIterator<T> other) const {
            return _i > other._i;
        }

        /** @brief Greater than or equal comparison */
        constexpr bool operator>=(StridedIterator<T> other) const {
            return _i >= other._i;
        }

        /** @brief Add an offset */
        StridedIterator<T> operator+(std::ptrdiff_t i) const {
            return {_i + i*_stride, _stride};
        }

        /** @brief Subtract an offset */
        StridedIterator<T> operator-(std::ptrdiff_t i) const {
            return {_i - i*_stride, _stride};
        }

        /** @brief Iterator difference */
        std::ptrdiff_t operator-(StridedIterator<T> it) const {
            return (_i - it._i)/_stride;
        }

        /** @brief Go back to previous position */
        StridedIterator<T>& operator--() {
            _i -= _stride;
            return *this;
        }

        /** @brief Advance to next position */
        StridedIterator<T>& operator++() {
            _i += _stride;
            return *this;
        }

        /** @brief Dereference */
        T& operator*() const { return *const_cast<T*>(reinterpret_cast<const T*>(_i)); }

    private:
        const char* _i;
        std::size_t _stride;
};

/** @relates StridedIterator
@brief Add strided iterator to an offset
*/
template<class T> inline StridedIterator<T> operator+(std::ptrdiff_t i, StridedIterator<T> it) {
    return it + i;
}

template<class T> T& StridedArrayView<T>::front() const {
    CORRADE_ASSERT(_size, "Containers::StridedArrayView::front(): view is empty", (*this)[0]);
    return (*this)[0];
}

template<class T> T& StridedArrayView<T>::back() const {
    CORRADE_ASSERT(_size, "Containers::StridedArrayView::back(): view is empty", (*this)[_size - 1]);
    return (*this)[_size - 1];
}

template<class T> StridedArrayView<T> StridedArrayView<T>::slice(std::size_t begin, std::size_t end) const {
    CORRADE_ASSERT(begin <= end && end <= _size,
        "Containers::StridedArrayView::slice(): slice [" << Utility::Debug::nospace
        << begin << Utility::Debug::nospace << ":"
        << Utility::Debug::nospace << end << Utility::Debug::nospace
        << "] out of range for" << _size << "elements", nullptr);
    return StridedArrayView<T>{const_cast<T*>(reinterpret_cast<const T*>(_data + begin*_stride)), std::size_t(end - begin), _stride};
}

}}

#endif
