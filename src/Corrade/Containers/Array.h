#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
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
 * @brief Class @ref Corrade::Containers::Array
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
@brief Array wrapper with size information

Provides movable RAII wrapper around plain C array. Main use case is storing
binary data of unspecified type, where addition/removal of elements is not
needed or harmful.

However, the class is usable also as lighter non-copyable alternative to
`std::vector`, in STL algorithms in the same way as plain C array and
additionally also in range-based for cycle.

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

-   @ref Array(DefaultInitT, std::size_t) is equivalent to the default case
    (useful when you want to make the choice appear explicit).
-   @ref Array(ValueInitT, std::size_t) zero-initializes trivial types and
    calls default constructor elsewhere.
-   @ref Array(DirectInitT, std::size_t, Args...) constructs all elements of
    the array using provided arguments.
-   @ref Array(NoInitT, std::size_t) does not initialize anything and you need
    to call the constructor on all elements manually using placement new,
    `std::uninitialized_copy` or similar. This is the dangerous option.

Example:
@code
// These are equivalent
Containers::Array<int> a1{5};
Containers::Array<int> a2{Containers::DefaultInit, 5};

// Array of 100 zeros
Containers::Array<int> b{Containers::ValueInit, 100};

// Array of type with no default constructor
struct Vec3 {
    Vec3(float, float, float);
};
Containers::Array<Vec3> c{Containers::DirectInit, 5, 5.2f, 0.4f, 1.0f};

// Manual construction of each element
struct Foo {
    Foo(int index);
};
Containers::Array<Foo> d{Containers::NoInit, 5};
int index = 0;
for(Foo& f: d) new(&f) Foo(index++);
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

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief Array(ValueInitT, std::size_t)
         * @deprecated Use @ref Array(ValueInitT, std::size_t) instead.
         */
        CORRADE_DEPRECATED("use Array(ValueInitT, std::size_t) instead") static Array<T> zeroInitialized(std::size_t size) {
            return Array<T>{ValueInit, size};
        }
        #endif

        /** @brief Conversion from nullptr */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ Array(std::nullptr_t) noexcept:
        #else
        template<class U, class V = typename std::enable_if<std::is_same<std::nullptr_t, U>::value>::type> /*implicit*/ Array(U) noexcept:
        #endif
            _data(nullptr), _size(0) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        /*implicit*/ Array() noexcept: _data(nullptr), _size(0) {}

        /**
         * @brief Construct default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e. builtin types are not initialized). If the size is zero, no
         * allocation is done.
         * @see @ref DefaultInit, @ref Array(ValueInitT, std::size_t)
         */
        explicit Array(DefaultInitT, std::size_t size): _data{size ? new T[size] : nullptr}, _size{size} {}

        /**
         * @brief Construct value-initialized array
         *
         * Creates array of given size, the contents are value-initialized
         * (i.e. builtin types are zero-initialized). For other than builtin
         * types this is the same as @ref Array(std::size_t). If the size is
         * zero, no allocation is done.
         *
         * Useful if you want to create an array of primitive types and sett
         * them to zero.
         * @see @ref ValueInit, @ref Array(DefaultInitT, std::size_t)
         */
        explicit Array(ValueInitT, std::size_t size): _data{size ? new T[size]() : nullptr}, _size{size} {}

        /**
         * @brief Construct the array without initializing its contents
         *
         * Creates array of given size, the contents are *not* initialized. If
         * the size is zero, no allocation is done. Initialize the values using
         * placement new.
         *
         * Useful if you will be overwriting all elements later anyway.
         * @attention The destructor will be called on all values regardless of
         *      whether they were properly initialized or not.
         * @see @ref NoInit, @ref Array(NoInitT, std::size_t)
         */
        explicit Array(NoInitT, std::size_t size): _data{size ? reinterpret_cast<T*>(new char[size*sizeof(T)]) : nullptr}, _size{size} {}

        /**
         * @brief Construct direct-initialized array
         *
         * Each element will be initialized using @p arguments instead of
         * calling default constructor. Note that because the arguments may be
         * used as a parameter in more than one constructor call, they are not
         * forwarded (i.e. rvalue references are *not* preserved).
         */
        template<class... Args> explicit Array(DirectInitT, std::size_t size, Args... args);

        /**
         * @brief Construct default-initialized array
         *
         * Alias to @ref Array(DefaultInitT, std::size_t).
         * @see @ref Array(ValueInitT, std::size_t)
         */
        explicit Array(std::size_t size): Array{DefaultInit, size} {}

        /**
         * @brief Wrap existing array
         *
         * Note that the array will be deleted on destruction.
         */
        explicit Array(T* data, std::size_t size): _data{data}, _size{size} {}

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

        /**
         * @brief Convert to @ref ArrayView
         *
         * Enabled only if `T*` is implicitly convertible to `U*`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<U>() noexcept { return {_data, _size}; }

        /**
         * @brief Convert to const @ref ArrayView
         *
         * Enabled only if `const T*` is implicitly convertible to `U*`.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class V = typename std::enable_if<std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<const U>() const noexcept { return {_data, _size}; }

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
        const T* data() const { return _data; }         /**< @overload */

        /** @brief Array size */
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
            array._data = new T[sizeof...(values)] { T(std::forward<U>(values))... };
            return array;
        }
        /* Specialization for zero argument count */
        static Array<T> fromInternal() { return nullptr; }

        T* _data;
        std::size_t _size;
};

#ifdef CORRADE_BUILD_DEPRECATED
/** @copybrief ArrayView
 * @deprecated Use @ref ArrayView.h and @ref ArrayView instead.
 */
#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using ArrayReference CORRADE_DEPRECATED_ALIAS("use ArrayView.h and ArrayView instead") = ArrayView<T>;
#endif
#endif

template<class T> inline Array<T>::Array(Array<T>&& other) noexcept: _data(other._data), _size(other._size) {
    other._data = nullptr;
    other._size = 0;
}

template<class T> template<class ...Args> Array<T>::Array(DirectInitT, std::size_t size, Args... args): Array{NoInit, size} {
    for(std::size_t i = 0; i != size; ++i)
        new(_data + i) T{args...};
}

template<class T> inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_size, other._size);
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
