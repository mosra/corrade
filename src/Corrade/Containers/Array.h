#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include <new>
#include <type_traits>
#include <utility>

#include "Corrade/configure.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Tags.h"

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
@tparam D   Deleter type. Defaults to pointer to @cpp void(T*, std::size_t) @ce
    function, where first is array pointer and second array size

Provides movable RAII wrapper around plain C array. Main use case is storing
binary data of unspecified type, where addition/removal of elements is not
needed or harmful.

However, the class is usable also as a lighter non-copyable alternative to
@ref std::vector; usable in STL algorithms in the same way as plain C array and
additionally also in range-for cycle.

Usage example:

@snippet Containers.cpp Array-usage

@section Containers-Array-initialization Array initialization

The array is by default *default-initialized*, which means that trivial types
are not initialized at all and default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref Array(DefaultInitT, std::size_t) is equivalent to the default case
    (useful when you want to make the choice appear explicit).
-   @ref Array(ValueInitT, std::size_t) zero-initializes trivial types and
    calls default constructor elsewhere.
-   @ref Array(DirectInitT, std::size_t, Args&&... args) constructs all
    elements of the array using provided arguments.
-   @ref Array(InPlaceInitT, std::initializer_list<T>) allocates unitialized
    memory and then copy-constructs all elements from the initializer list
-   @ref Array(NoInitT, std::size_t) does not initialize anything and you need
    to call the constructor on all elements manually using placement new,
    @ref std::uninitialized_copy() or similar. This is the dangerous option.

Example:

@snippet Containers.cpp Array-initialization

@section Containers-Array-wrapping Wrapping externally allocated arrays

By default the class makes all allocations using @cpp operator new[] @ce and
deallocates using @cpp operator delete[] @ce for given @p T, with some
additional trickery done internally to make the @ref Array(NoInitT, std::size_t)
and @ref Array(DirectInitT, std::size_t, Args&&... args) constructors work.
When wrapping an externally allocated array using @ref Array(T*, std::size_t, D),
it is possible to specify which function to use for deallocation. By default
the deleter is set to @cpp nullptr @ce, which is equivalent to deleting the
contents using @cpp operator delete[] @ce.

For example, properly deallocating array allocated using @ref std::malloc():

@snippet Containers.cpp Array-wrapping

By default, plain function pointers are used to avoid having the type affected
by the deleter function. If the deleter needs to manage some state, a custom
deleter type can be used:

@snippet Containers.cpp Array-deleter

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
         * @attention Internally the data are allocated as @cpp char @ce array
         *      and destruction is done using custom deleter that explicitly
         *      calls destructor on *all elements* regardless of whether they
         *      were properly constructed or not and then deallocates the data
         *      as @cpp char @ce array.
         * @see @ref NoInit, @ref Array(DirectInitT, std::size_t, Args&&... args),
         *      @ref deleter()
         */
        explicit Array(NoInitT, std::size_t size): _data{size ? reinterpret_cast<T*>(new char[size*sizeof(T)]) : nullptr}, _size{size}, _deleter{Implementation::noInitDeleter} {}

        /**
         * @brief Construct direct-initialized array
         *
         * Allocates the array using the @ref Array(NoInitT, std::size_t)
         * constructor and then initializes each element with placement new
         * using forwarded @p args.
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

        /* The following ArrayView conversion are *not* restricted to this&
           because that would break uses like `consume(foo());`, where
           `consume()` expects a view but `foo()` returns an owning array. */

        /**
         * @brief Convert to @ref ArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
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
         * Enabled only if @cpp T* @ce or @cpp const T* @ce is implicitly
         * convertible to @cpp U* @ce. Expects that both types have the same
         * size.
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
        /*implicit*/ operator T*() & { return _data; }

        /** @overload */
        /*implicit*/ operator const T*() const & { return _data; }

        /** @brief Array data */
        T* data() { return _data; }
        const T* data() const { return _data; }         /**< @overload */

        /**
         * @brief Array deleter
         *
         * If set to @cpp nullptr @ce, the contents are deleted using standard
         * @cpp operator delete[] @ce.
         * @see @ref Array(T*, std::size_t, D)
         */
        D deleter() const { return _deleter; }

        /** @brief Array size */
        std::size_t size() const { return _size; }

        /** @brief Whether the array is empty */
        bool empty() const { return !_size; }

        /**
         * @brief Pointer to first element
         *
         * @see @ref front()
         */
        T* begin() { return _data; }
        const T* begin() const { return _data; }        /**< @overload */
        const T* cbegin() const { return _data; }       /**< @overload */

        /**
         * @brief Pointer to (one item after) last element
         *
         * @see @ref back()
         */
        T* end() { return _data+_size; }
        const T* end() const { return _data+_size; }    /**< @overload */
        const T* cend() const { return _data+_size; }   /**< @overload */

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref begin()
         */
        T& front();
        const T& front() const; /**< @overload */

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref end()
         */
        T& back();
        const T& back() const; /**< @overload */

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
            return ArrayView<T>(*this).slice(begin, end);
        }
        /** @overload */
        ArrayView<const T> slice(std::size_t begin, std::size_t end) const {
            return ArrayView<const T>(*this).slice(begin, end);
        }

        /**
         * @brief Fixed-size array slice
         *
         * Both @cpp begin @ce and @cpp begin + size @ce are expected to be in
         * range.
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
            return ArrayView<T>(*this).template slice<size>(begin);
        }
        /** @overload */
        template<std::size_t size> StaticArrayView<size, const T> slice(std::size_t begin) const {
            return ArrayView<const T>(*this).template slice<size>(begin);
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
        /** @overload */
        ArrayView<T> prefix(std::size_t end) {
            return ArrayView<T>(*this).prefix(end);
        }
        /** @overload */
        ArrayView<const T> prefix(std::size_t end) const {
            return ArrayView<const T>(*this).prefix(end);
        }

        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, T> prefix() {
            return ArrayView<T>(*this).template prefix<viewSize>();
        }
        /** @overload */
        template<std::size_t viewSize> StaticArrayView<viewSize, const T> prefix() const {
            return ArrayView<const T>(*this).template prefix<viewSize>();
        }

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
        /** @overload */
        ArrayView<T> suffix(std::size_t begin) {
            return ArrayView<T>(*this).suffix(begin);
        }
        /** @overload */
        ArrayView<const T> suffix(std::size_t begin) const {
            return ArrayView<const T>(*this).suffix(begin);
        }

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

@snippet Containers.cpp Array-arrayView
*/
template<class T, class D> inline ArrayView<T> arrayView(Array<T, D>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso ArrayView
@brief Make view on const @ref Array

Convenience alternative to calling @ref Array::operator ArrayView<U>()
explicitly. The following two lines are equivalent:

@snippet Containers.cpp Array-arrayView-const
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

template<class T, class D> const T& Array<T, D>::front() const {
    CORRADE_ASSERT(_size, "Containers::Array::front(): array is empty", _data[0]);
    return _data[0];
}

template<class T, class D> const T& Array<T, D>::back() const {
    CORRADE_ASSERT(_size, "Containers::Array::back(): array is empty", _data[_size - 1]);
    return _data[_size - 1];
}

template<class T, class D> T& Array<T, D>::front() {
    return const_cast<T&>(static_cast<const Array<T, D>&>(*this).front());
}

template<class T, class D> T& Array<T, D>::back() {
    return const_cast<T&>(static_cast<const Array<T, D>&>(*this).back());
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
