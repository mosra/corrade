#ifndef Corrade_Containers_ArrayView_h
#define Corrade_Containers_ArrayView_h
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
size information to functions etc. If @p T is @cpp const @ce type, the class is
implicitly constructible also from const references to @ref Array and
@ref ArrayView of non-const types.

Usage example:

@snippet Containers.cpp ArrayView-usage

@attention Note that when using @cpp Containers::ArrayView<const char> @ce, C
    string literals (such as @cpp "hello" @ce) are implicitly convertible to it
    and the size includes also the zero-terminator (thus in case of
    @cpp "hello" @ce the size would be 6, not 5, as one might expect).

@see @ref ArrayView<const void>, @ref StaticArrayView, @ref arrayView(),
    @ref arrayCast(ArrayView<T>)
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
         * Creates empty view. Copy non-empty @ref Array or @ref ArrayView onto
         * the instance to make it useful.
         */
        constexpr /*implicit*/ ArrayView() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Construct view on an array with explicit length
         * @param data      Data pointer
         * @param size      Data size
         *
         * @see @ref arrayView(T*, std::size_t)
         */
        constexpr /*implicit*/ ArrayView(T* data, std::size_t size) noexcept: _data(data), _size(size) {}

        /**
         * @brief Construct view on a fixed-size array
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
         * @brief Construct view on @ref ArrayView
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
         * @brief Construct view on @ref StaticArrayView
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

        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        explicit operator bool() const { return _size; }
        #endif

        /** @brief Conversion to array type */
        constexpr /*implicit*/ operator T*() const { return _data; }

        /** @brief Array data */
        constexpr T* data() const { return _data; }

        /** @brief Array size */
        constexpr std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        constexpr bool empty() const { return !_size; }

        /**
         * @brief Pointer to first element
         *
         * @see @ref front()
         */
        constexpr T* begin() const { return _data; }
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        T* end() const { return _data+_size; }
        T* cend() const { return _data+_size; }         /**< @overload */

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
        ArrayView<T> slice(T* begin, T* end) const;

        /** @overload */
        ArrayView<T> slice(std::size_t begin, std::size_t end) const {
            return slice(_data + begin, _data + end);
        }

        /**
         * @brief Fixed-size array slice
         *
         * Both @p begin and @cpp begin + viewSize @ce are expected to be in
         * range.
         */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(T* begin) const;

        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> slice(std::size_t begin) const {
            return slice<viewSize>(_data + begin);
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(data.begin(), end) @ce. If @p end is
         * @cpp nullptr @ce, returns zero-sized @cpp nullptr @ce array.
         */
        ArrayView<T> prefix(T* end) const {
            if(!end) return nullptr;
            return slice(_data, end);
        }
        ArrayView<T> prefix(std::size_t end) const { return prefix(_data + end); } /**< @overload */

        /**
         * @brief Fixed-size array prefix
         *
         * Equivalent to @cpp data.slice<viewSize>(data.begin()) @ce.
         */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> prefix() const {
            return slice<viewSize>(_data);
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice(begin, data.end()) @ce. If @p begin is
         * @cpp nullptr @ce and the original array isn't, returns zero-sized
         * @cpp nullptr @ce array.
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
size in bytes. This specialization doesn't provide any @cpp begin() @ce /
@cpp end() @ce accessors, because it has no use for @cpp void @ce type.

Usage example:

@snippet Containers.cpp ArrayView-void-usage
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
         * @brief Construct view on array of explicit length
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

        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
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
@brief Make view on an array of specific length

Convenience alternative to @ref ArrayView::ArrayView(T*, std::size_t). The
following two lines are equivalent:

@snippet Containers.cpp arrayView
*/
template<class T> constexpr ArrayView<T> arrayView(T* data, std::size_t size) {
    return ArrayView<T>{data, size};
}

/** @relatesalso ArrayView
@brief Make view on fixed-size array

Convenience alternative to @ref ArrayView::ArrayView(U(&)[size]). The following
two lines are equivalent:

@snippet Containers.cpp arrayView-array
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(T(&data)[size]) {
    return ArrayView<T>{data};
}

/** @relatesalso ArrayView
@brief Make view on @ref StaticArrayView

Convenience alternative to @ref ArrayView::ArrayView(StaticArrayView<size, U>).
The following two lines are equivalent:

@snippet Containers.cpp arrayView-StaticArrayView
*/
template<std::size_t size, class T> constexpr ArrayView<T> arrayView(StaticArrayView<size, T> view) {
    return ArrayView<T>{view};
}

/** @relatesalso ArrayView
@brief Make view on a view

Equivalent to the implicit @ref ArrayView copy constructor --- it shouldn't be
an error to call @ref arrayView() on itself.
*/
template<class T> constexpr ArrayView<T> arrayView(ArrayView<T> view) {
    return view;
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
        "Containers::arrayCast(): type sizes are not compatible", {});
    return {reinterpret_cast<U*>(view.begin()), size};
}

/** @relatesalso ArrayView
@brief Array view size

Alias to @ref ArrayView::size(), useful as a shorthand in cases like this:

@snippet Containers.cpp arraySize
*/
template<class T> std::size_t arraySize(ArrayView<T> view) {
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

/**
@brief Fixed-size array view

Equivalent to @ref ArrayView, but with compile-time size information.
Convertible from and to @ref ArrayView. Example usage:

@snippet Containers.cpp StaticArrayView-usage

@see @ref staticArrayView(), @ref arrayCast(StaticArrayView<size, T>)
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

        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        constexpr explicit operator bool() const { return size_; }
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
        constexpr T* cbegin() const { return _data; }   /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        T* end() const { return _data + size_; }
        T* cend() const { return _data + size_; }       /**< @overload */

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

        /** @copydoc ArrayView::prefix() const */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> prefix() const {
            return slice<viewSize>(_data);
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

/** @relatesalso StaticArrayView
@brief Make static view on an array

Convenience alternative to @ref StaticArrayView::StaticArrayView(T*). The
following two lines are equivalent:

@snippet Containers.cpp staticArrayView
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(T* data) {
    return StaticArrayView<size, T>{data};
}

/** @relatesalso StaticArrayView
@brief Make static view on a fixed-size array

Convenience alternative to @ref StaticArrayView::StaticArrayView(U(&)[size_]).
The following two lines are equivalent:

@snippet Containers.cpp staticArrayView-array
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(T(&data)[size]) {
    return StaticArrayView<size, T>{data};
}

/** @relatesalso StaticArrayView
@brief Make view on a view

Equivalent to the implicit @ref StaticArrayView copy constructor --- it
shouldn't be an error to call @ref staticArrayView() on itself.
*/
template<std::size_t size, class T> constexpr StaticArrayView<size, T> staticArrayView(StaticArrayView<size, T> view) {
    return view;
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

template<class T> ArrayView<T> ArrayView<T>::slice(T* begin, T* end) const {
    CORRADE_ASSERT(_data <= begin && begin <= end && end <= _data + _size,
        "Containers::ArrayView::slice(): slice [" << Utility::Debug::nospace
        << begin - _data << Utility::Debug::nospace << ":"
        << Utility::Debug::nospace << end - _data << Utility::Debug::nospace
        << "] out of range for" << _size << "elements", nullptr);
    return ArrayView<T>{begin, std::size_t(end - begin)};
}

template<std::size_t size_, class T> T& StaticArrayView<size_, T>::front() const {
    static_assert(size_, "view is empty");
    return _data[0];
}

template<std::size_t size_, class T> T& StaticArrayView<size_, T>::back() const {
    static_assert(size_, "view is empty");
    return _data[size_ - 1];
}

template<class T> template<std::size_t viewSize> StaticArrayView<viewSize, T> ArrayView<T>::slice(T* begin) const {
    CORRADE_ASSERT(_data <= begin && begin + viewSize <= _data + _size,
        "Containers::ArrayView::slice(): slice [" << Utility::Debug::nospace
        << begin - _data << Utility::Debug::nospace << ":"
        << Utility::Debug::nospace << begin + viewSize - _data << Utility::Debug::nospace
        << "] out of range for" << _size << "elements", nullptr);
    return StaticArrayView<viewSize, T>{begin};
}

}}

#endif
