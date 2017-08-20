#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Implementation {
    template<class T> struct DefaultDeleter {
        T operator()() const { return T{}; }
    };
    template<class T> struct DefaultDeleter<void(*)(T*, std::size_t)> {
        void(*operator()() const)(T*, std::size_t) { return nullptr; }
    };

    template<class T, class D> struct CallDeleter {
        void operator()(D deleter, T* data, std::size_t size) const {
            deleter(data, size);
        }
    };
    template<class T> struct CallDeleter<T, void(*)(T*, std::size_t)> {
        void operator()(void(*deleter)(T*, std::size_t), T* data, std::size_t size) const {
            if(deleter) deleter(data, size);
            else delete[] data;
        }
    };

    template<class T> void noInitDeleter(T* data, std::size_t size) {
        if(data) for(T *it = data, *end = data + size; it != end; ++it)
            it->~T();
        delete[] reinterpret_cast<char*>(data);
    }
}

/**
@brief Array wrapper with size information
@tparam T   Element type
@tparam D   Deleter type. Defaults to pointer to `void(T*, std::size_t)`
    function, where first is array pointer and second array size

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

## Wrapping externally allocated arrays

By default the class makes all allocations using `operator new[]` and
deallocates using `operator delete[]` for given @p T, with some additional
trickery done internally to make the @ref Array(NoInitT, std::size_t) and
@ref Array(DirectInitT, std::size_t, ...) constructors work. When wrapping an
externally allocated array using @ref Array(T*, std::size_t, D), it is possible
to specify which function to use for deallocation. By default the deleter is
set to `nullptr`, which is equivalent to deleting the contents using
`operator delete[]`.

For example, properly deallocating array allocated using `std::malloc()`:
@code
const int* data = reinterpret_cast<int*>(std::malloc(25*sizeof(int)));

// Will call std::free() on destruction
Containers::Array<int> array{data, 25, [](int* data, std::size_t) { std::free(data); }};
@endcode

By default, plain function pointers are used to avoid having the type affected
by the deleter function. If the deleter needs to manage some state, a custom
deleter type can be used:
@code
struct UnmapBuffer {
    UnmapBuffer(GLuint id): _id{id} {}
    void operator()(T*, std::size_t) { glUnmapNamedBuffer(_id); }

private:
    GLuint _id;
};

GLuint buffer;
char* data = reinterpret_cast<char*>(glMapNamedBuffer(buffer, GL_READ_WRITE));

// Will unmap the buffer on destruction
Containers::Array<char, UnmapBuffer> array{data, bufferSize, UnmapBuffer{buffer}};
@endcode

@see @ref arrayCast(Array<T, D>&)

@todo Something like ArrayTuple to create more than one array with single
    allocation and proper alignment for each type? How would non-POD types be
    constructed in that? Will that be useful in more than one place?
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, class D = void(*)(T*, std::size_t)>
#else
template<class T, class D>
#endif
class Array {
    public:
        typedef T Type;     /**< @brief Element type */
        typedef D Deleter;  /**< @brief Deleter type */

        #ifdef CORRADE_BUILD_DEPRECATED
        /**
         * @copybrief Array(InPlaceInitT, std::initializer_list<T>)
         * @deprecated Use @ref Array(InPlaceInitT, std::initializer_list<T>) instead.
         */
        template<class ...U> CORRADE_DEPRECATED("use Array(InPlaceInitT, std::initializer_list<T>) instead") static Array<T, D> from(U&&... values) {
            return Array<T, D>{InPlaceInit, {T(std::forward<U>(values))...}};
        }

        /**
         * @copybrief Array(ValueInitT, std::size_t)
         * @deprecated Use @ref Array(ValueInitT, std::size_t) instead.
         */
        CORRADE_DEPRECATED("use Array(ValueInitT, std::size_t) instead") static Array<T, D> zeroInitialized(std::size_t size) {
            return Array<T>{ValueInit, size};
        }
        #endif

        /** @brief Conversion from nullptr */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ Array(std::nullptr_t) noexcept:
        #else
        template<class U, class V = typename std::enable_if<std::is_same<std::nullptr_t, U>::value>::type> /*implicit*/ Array(U) noexcept:
        #endif
            /* GCC <=4.8 breaks on _deleter{} */
            _data{nullptr}, _size{0}, _deleter(Implementation::DefaultDeleter<D>{}()) {}

        /**
         * @brief Default constructor
         *
         * Creates zero-sized array. Move array with nonzero size onto the
         * instance to make it useful.
         */
        /* GCC <=4.8 breaks on _deleter{} */
        /*implicit*/ Array() noexcept: _data(nullptr), _size(0), _deleter(Implementation::DefaultDeleter<D>{}()) {}

        /**
         * @brief Construct default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e. builtin types are not initialized). If the size is zero, no
         * allocation is done.
         * @see @ref DefaultInit, @ref Array(ValueInitT, std::size_t)
         */
        explicit Array(DefaultInitT, std::size_t size): _data{size ? new T[size] : nullptr}, _size{size}, _deleter{nullptr} {}

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
        explicit Array(ValueInitT, std::size_t size): _data{size ? new T[size]() : nullptr}, _size{size}, _deleter{nullptr} {}

        /**
         * @brief Construct the array without initializing its contents
         *
         * Creates array of given size, the contents are *not* initialized. If
         * the size is zero, no allocation is done. Initialize the values using
         * placement new.
         *
         * Useful if you will be overwriting all elements later anyway.
         * @attention Internally the data are allocated as `char` array and
         *      destruction is done using custom deleter that explicitly calls
         *      destructor on *all elements* regardless of whether they were
         *      properly constructed or not and then deallocates the data as
         *      `char` array.
         * @see @ref NoInit, @ref Array(DirectInitT, std::size_t, Args...),
         *      @ref deleter()
         */
        explicit Array(NoInitT, std::size_t size): _data{size ? reinterpret_cast<T*>(new char[size*sizeof(T)]) : nullptr}, _size{size}, _deleter{Implementation::noInitDeleter} {}

        /**
         * @brief Construct direct-initialized array
         *
         * Allocates the array using the @ref Array(NoInitT, std::size_t)
         * constructor and then initializes each element with placement new
         * using forwarded @p arguments.
         */
        template<class... Args> explicit Array(DirectInitT, std::size_t size, Args&&... args);

        /**
         * @brief Construct list-initialized array
         *
         * Allocates the array using the @ref Array(NoInitT, std::size_t)
         * constructor and then copy-initializes each element with placement
         * new using values from @p list.
         */
        /* There is no initializer-list constructor because it would make
           Containers::Array<std::size_t>{5} behave differently (and that would
           break things *badly* as this is a *very* common use-case) */
        explicit Array(InPlaceInitT, std::initializer_list<T> list);

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
         * Note that the array will be deleted on destruction using given
         * @p deleter. See class documentation for more information about
         * custom deleters and @ref ArrayView for non-owning array wrapper.
         */
        /* GCC <=4.8 breaks on _deleter{} */
        explicit Array(T* data, std::size_t size, D deleter = Implementation::DefaultDeleter<D>{}()): _data{data}, _size{size}, _deleter(deleter) {}

        ~Array() { Implementation::CallDeleter<T, D>{}(_deleter, _data, _size); }

        /** @brief Copying is not allowed */
        Array(const Array<T, D>&) = delete;

        /** @brief Move constructor */
        Array(Array<T, D>&& other) noexcept;

        /** @brief Copying is not allowed */
        Array<T, D>& operator=(const Array<T, D>&) = delete;

        /** @brief Move assignment */
        Array<T, D>& operator=(Array<T, D>&&) noexcept;

        #ifndef CORRADE_MSVC2017_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC <= 2017 to avoid ambiguous operator+() when doing
           pointer arithmetic. */
        explicit operator bool() const { return _data; }
        #endif

        /**
         * @brief Convert to @ref ArrayView
         *
         * Enabled only if `T*` is implicitly convertible to `U*`. Expects that
         * both types have the same size.
         * @see @ref arrayView(Array<T, D>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<!std::is_void<U>::value && std::is_convertible<T*, U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<U>() noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return {_data, _size};
        }

        /**
         * @brief Convert to const @ref ArrayView
         *
         * Enabled only if `T*` or `const T*` is implicitly convertible to `U*`.
         * Expects that both types have the same size.
         * @see @ref arrayView(const Array<T, D>&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<T*, U*>::value || std::is_convertible<T*, const U*>::value>::type>
        #endif
        /*implicit*/ operator ArrayView<const U>() const noexcept {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
            return {_data, _size};
        }

        /** @overload */
        /*implicit*/ operator ArrayView<const void>() const noexcept {
            /* Yes, the size is properly multiplied by sizeof(T) by the constructor */
            return {_data, _size};
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
        const T* data() const { return _data; }         /**< @overload */

        /**
         * @brief Array deleter
         *
         * If set to `nullptr`, the contents are deleted using standard
         * `operator delete[]`.
         * @see @ref Array(T*, std::size_t, D)
         */
        D deleter() const { return _deleter; }

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
         * @brief Fixed-size array slice
         *
         * Both @p begin and `begin + size` are expected to be in range.
         */
        template<std::size_t size> StaticArrayView<size, T> slice(T* begin) {
            return ArrayView<T>(*this).template slice<size>(begin);
        }
        /** @overload */
        template<std::size_t size> StaticArrayView<size, const T> slice(const T* begin) const {
            return ArrayView<const T>(*this).template slice<size>(begin);
        }
        /** @overload */
        template<std::size_t size> StaticArrayView<size, T> slice(std::size_t begin) {
            return slice<size>(_data + begin);
        }
        /** @overload */
        template<std::size_t size> StaticArrayView<size, const T> slice(std::size_t begin) const {
            return slice<size>(_data + begin);
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
        T* _data;
        std::size_t _size;
        D _deleter;
};

/** @relatesalso ArrayView
@brief Make view on @ref Array

Convenience alternative to calling @ref Array::operator ArrayView<U>()
explicitly. The following two lines are equivalent:
@code
Containers::Array<std::uint32_t> data;

Containers::ArrayView<std::uint32_t> a{data};
auto b = Containers::arrayView(data);
@endcode
*/
template<class T, class D> inline ArrayView<T> arrayView(Array<T, D>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso ArrayView
@brief Make view on const @ref Array

Convenience alternative to calling @ref Array::operator ArrayView<U>()
explicitly. The following two lines are equivalent:
@code
const Containers::Array<std::uint32_t> data;

Containers::ArrayView<const std::uint32_t> a{data};
auto b = Containers::arrayView(data);
@endcode
*/
template<class T, class D> inline ArrayView<const T> arrayView(const Array<T, D>& array) {
    return ArrayView<const T>{array};
}

/** @relatesalso Array
@brief Reinterpret-cast an array

See @ref arrayCast(ArrayView<T>) for more information.
*/
template<class U, class T, class D> inline ArrayView<U> arrayCast(Array<T, D>& array) {
    return arrayCast<U>(arrayView(array));
}

/** @overload */
template<class U, class T, class D> inline ArrayView<const U> arrayCast(const Array<T, D>& array) {
    return arrayCast<const U>(arrayView(array));
}

/** @relatesalso Array
@brief Array size

See @ref arraySize(ArrayView<T>) for more information.
*/
template<class T> std::size_t arraySize(const Array<T>& view) {
    return view.size();
}

#ifdef CORRADE_BUILD_DEPRECATED
/** @copybrief ArrayView
 * @deprecated Use @ref ArrayView.h and @ref ArrayView instead.
 */
#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using ArrayReference CORRADE_DEPRECATED_ALIAS("use ArrayView.h and ArrayView instead") = ArrayView<T>;
#endif
#endif

template<class T, class D> inline Array<T, D>::Array(Array<T, D>&& other) noexcept: _data{other._data}, _size{other._size}, _deleter{other._deleter} {
    other._data = nullptr;
    other._size = 0;
}

template<class T, class D> template<class ...Args> Array<T, D>::Array(DirectInitT, std::size_t size, Args&&... args): Array{NoInit, size} {
    for(std::size_t i = 0; i != size; ++i)
        new(_data + i) T{std::forward<Args>(args)...};
}

template<class T, class D> Array<T, D>::Array(InPlaceInitT, std::initializer_list<T> list): Array{NoInit, list.size()} {
    std::size_t i = 0;
    for(const T& item: list) new(_data + i++) T{item};
}

template<class T, class D> inline Array<T, D>& Array<T, D>::operator=(Array<T, D>&& other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_size, other._size);
    swap(_deleter, other._deleter);
    return *this;
}

template<class T, class D> inline T* Array<T, D>::release() {
    /** @todo I need `std::exchange` NOW. */
    T* const data = _data;
    _data = nullptr;
    _size = 0;
    return data;
}

}}

#endif
