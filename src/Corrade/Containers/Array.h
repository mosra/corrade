#ifndef Corrade_Containers_Array_h
#define Corrade_Containers_Array_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
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

#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility> /* std::swap() */

#include "Corrade/Tags.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/constructHelpers.h"

namespace Corrade { namespace Containers {

namespace Implementation {
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

    template<class T> T* noInitAllocate(std::size_t size, typename std::enable_if<std::is_trivial<T>::value>::type* = nullptr) {
        return new T[size];
    }
    template<class T> T* noInitAllocate(std::size_t size, typename std::enable_if<!std::is_trivial<T>::value>::type* = nullptr) {
        return reinterpret_cast<T*>(new char[size*sizeof(T)]);
    }

    template<class T> auto noInitDeleter(typename std::enable_if<std::is_trivial<T>::value>::type* = nullptr) -> void(*)(T*, std::size_t) {
        return nullptr; /* using the default deleter for T */
    }
    template<class T> auto noInitDeleter(typename std::enable_if<!std::is_trivial<T>::value>::type* = nullptr) -> void(*)(T*, std::size_t) {
        return [](T* data, std::size_t size) {
            if(data) for(T *it = data, *end = data + size; it != end; ++it)
                it->~T();
            delete[] reinterpret_cast<char*>(data);
        };
    }
}

/**
@brief Array
@tparam T   Element type
@tparam D   Deleter type. Defaults to pointer to a @cpp void(T*, std::size_t) @ce
    function, where first is array pointer and second array size.

A RAII owning wrapper around a plain C array. A lighter alternative to
@ref std::vector that's deliberately move-only to avoid accidental copies of
large memory blocks. For a variant with compile-time size information see
@ref StaticArray. A non-owning version of this container is an @ref ArrayView.

The array has a non-changeable size by default and growing functionality is
opt-in, see @ref Containers-Array-growable below for more information.

@section Containers-Array-usage Usage

The @ref Array class provides an access and slicing API similar to
@ref ArrayView, see @ref Containers-ArrayView-usage "its usage docs" for
details. The only difference is due to the owning aspect --- mutable access to
the data is provided only via non @cpp const @ce overloads.

@snippet Containers.cpp Array-usage

@subsection Containers-Array-usage-initialization Array initialization

The array is by default *value-initialized*, which means that trivial types
are zero-initialized and the default constructor is called on other types. It
is possible to initialize the array in a different way using so-called *tags*:

-   @ref Array(DefaultInitT, std::size_t) leaves trivial types uninitialized
    and calls the default constructor elsewhere. In other words,
    @cpp new T[size] @ce. Because of the differing behavior for trivial types
    it's better to explicitly use either the @ref ValueInit or @ref NoInit
    variants instead.
-   @ref Array(ValueInitT, std::size_t) is equivalent to the default case,
    zero-initializing trivial types and calling the default constructor
    elsewhere. Useful when you want to make the choice appear explicit. In
    other words, @cpp new T[size]{} @ce.
-   @ref Array(DirectInitT, std::size_t, Args&&... args) constructs all
    elements of the array using provided arguments. In other words,
    @cpp new T[size]{T{args...}, T{args...}, …} @ce.
-   @ref Array(InPlaceInitT, std::initializer_list<T>) or the
    @ref array(std::initializer_list<T>) shorthand allocates unitialized memory
    and then copy-constructs all elements from the initializer list. In other
    words, @cpp new T[size]{args...} @ce. The class deliberately *doesn't*
    provide an implicit @ref std::initializer_list constructor due to
    @ref Containers-Array-initializer-list "reasons described below".
-   @ref Array(NoInitT, std::size_t) does not initialize anything. Useful for
    trivial types when you'll be overwriting the contents anyway, for
    non-trivial types this is the dangerous option and you need to call the
    constructor on all elements manually using placement new,
    @ref std::uninitialized_copy() or similar --- see the constructor docs for
    an example. In other words, @cpp new char[size*sizeof(T)] @ce for
    non-trivial types to circumvent default construction and @cpp new T[size] @ce
    for trivial types.

@snippet Containers.cpp Array-usage-initialization

<b></b>

@m_class{m-note m-success}

@par Aligned allocations
    Please note that @ref Array allocations are by default only aligned to
    @cpp 2*sizeof(void*) @ce. If you need overaligned memory for working with
    SIMD types, use @ref Utility::allocateAligned() instead.

@subsection Containers-Array-usage-wrapping Wrapping externally allocated arrays

By default the class makes all allocations using @cpp operator new[] @ce and
deallocates using @cpp operator delete[] @ce for given @p T, with some
additional trickery done internally to make the @ref Array(NoInitT, std::size_t)
and @ref Array(DirectInitT, std::size_t, Args&&... args) constructors work.
It's however also possible to wrap an externally allocated array using
@ref Array(T*, std::size_t, D) together with specifying which function to use
for deallocation. By default the deleter is set to @cpp nullptr @ce, which is
equivalent to deleting the contents using @cpp operator delete[] @ce.

For example, properly deallocating array allocated using @ref std::malloc():

@snippet Containers.cpp Array-usage-wrapping

By default, plain function pointers are used to avoid having the type affected
by the deleter function. If the deleter needs to manage some state, a custom
deleter type can be used:

@snippet Containers.cpp Array-usage-deleter

The deleter is called *unconditionally* on destruction, which has some
implications especially in case of stateful deleters. See the documentation of
@ref Array(T*, std::size_t, D) for details.

@section Containers-Array-growable Growable arrays

The @ref Array class provides no reallocation or growing capabilities on its
own, and this functionality is opt-in via free functions from
@ref Corrade/Containers/GrowableArray.h instead. This is done in order to keep
the concept of an owning container decoupled from the extra baggage coming from
custom allocators, type constructibility and such.

As long as the type stored in the array is nothrow-move-constructible, any
@ref Array instance can be converted to a growing container by calling the
family of @ref arrayAppend(), @ref arrayReserve(), @ref arrayResize() ...
functions. A growable array behaves the same as a regular array to its
consumers --- its @ref size() returns the count of *real* elements, while
available capacity can be queried through @ref arrayCapacity(). Example of
populating an array with an undetermined amount of elements:

@snippet Containers.cpp Array-growable

A growable array can be turned back into a regular one using
@ref arrayShrink() if desired. That'll free all extra memory, moving the
elements to an array of exactly the size needed.

@m_class{m-block m-success}

@par Tip
    To save typing, you can make use of ADL and call the @ref arrayAppend()
    etc. functions unqualified, without having them explicitly prefixed with
    @cpp Containers:: @ce.

@subsection Containers-Array-growable-allocators Growable allocators

Similarly to standard containers, growable arrays allow you to use a custom
allocator that matches the documented semantics of @ref ArrayAllocator. It's
also possible to switch between different allocators during the lifetime of an
@ref Array instance --- internally it's the same process as when a non-growable
array is converted to a growable version (or back, with @ref arrayShrink()).

The @ref ArrayAllocator is by default aliased to @ref ArrayNewAllocator, which
uses the standard C++ @cpp new[] @ce / @cpp delete[] @ce constructs and is
fully move-aware, requiring the types to be only nothrow-move-constructible at
the very least. If a type is trivially copyable, the @ref ArrayMallocAllocator
will get picked instead, make use of @ref std::realloc() to avoid unnecessary
memory copies when growing the array. The typeless nature of
@ref ArrayMallocAllocator internals allows for free type-casting of the array
instance with @ref arrayAllocatorCast(), an operation not easily doable using
typed allocators.

@subsection Containers-Array-growable-sanitizer AddressSanitizer container annotations

Because the alloacted growable arrays have an area between @ref size() and
@ref arrayCapacity() that shouldn't be accessed, when building with
[Address Sanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)
enabled, this area is marked as "container overflow". Given the following code,
ASan aborts and produces a failure report similar to the one below:

@m_class{m-code-figure}

@parblock

@snippet Containers.cpp Array-growable-sanitizer

<p></p>

@m_class{m-nopad}

@include containers-growable-array-sanitizer.ansi

@endparblock

@section Containers-Array-views Conversion to array views

Arrays are implicitly convertible to @ref ArrayView as described in the
following table. The conversion is only allowed if @cpp T* @ce is implicitly
convertible to @cpp U* @ce (or both are the same type) and both have the same
size. This also extends to other container types constructibe from
@ref ArrayView, which means for example that a @ref StridedArrayView1D is
implicitly convertible from @ref Array as well.

Owning array type               | ↭ | Non-owning view type
------------------------------- | - | ---------------------
@ref Array "Array<T>"           | → | @ref ArrayView "ArrayView<U>"
@ref Array "Array<T>"           | → | @ref ArrayView "ArrayView<const U>"
@ref Array "const Array<T>"     | → | @ref ArrayView "ArrayView<const U>"

@section Containers-Array-stl STL compatibility

On compilers that support C++2a and @ref std::span, implicit conversion of an
@ref Array to it is provided in @ref Corrade/Containers/ArrayViewStlSpan.h. The
conversion is provided in a separate header to avoid unconditional
@cpp #include <span> @ce, which significantly affects compile times. The
following table lists allowed conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref Array "Array<T>"           | → | @ref std::span "std::span<T>"
@ref Array "Array<T>"           | → | @ref std::span "std::span<const T>"
@ref Array "const Array<T>"     | → | @ref std::span "std::span<const T>"

@anchor Containers-Array-initializer-list

<b></b>

@m_class{m-block m-warning}

@par Conversion from std::initializer_list
    The class deliberately *doesn't* provide a @ref std::initializer_list
    constructor to prevent the same usability issues as with @ref std::vector
    --- see the snippet below. Instead you're expected to use either the
    @ref Array(InPlaceInitT, std::initializer_list<T>) constructor or the
    @ref array(std::initializer_list<T>) shorthand, which are both more
    explicit and thus should prevent accidental use:
@par
    @snippet Containers-stl.cpp Array-initializer-list

<b></b>

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with @ref StaticArray, is also available as a
    single-header [CorradeArray.h](https://github.com/mosra/magnum-singles/tree/master/CorradeArray.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref ArrayTuple
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, class D = void(*)(T*, std::size_t)>
#else
template<class T, class D>
#endif
class Array {
    /* Ideally this could be derived from ArrayView<T>, avoiding a lot of
       redundant code, however I'm unable to find a way to add const/non-const
       overloads of all slicing functions and also prevent const Array<T>& from
       being sliced to a (mutable) ArrayView<T>. */

    public:
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Deleter type
         *
         * Defaults to pointer to a @cpp void(T*, std::size_t) @ce function,
         * where first is array pointer and second array size.
         */
        typedef D Deleter;

        /** @brief Conversion from `nullptr` */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ Array(std::nullptr_t) noexcept:
        #else
        /* Has to be like this because otherwise Array{0} is ambiguous. FFS
           C++. */
        template<class U, class V = typename std::enable_if<std::is_same<std::nullptr_t, U>::value>::type> /*implicit*/ Array(U) noexcept:
        #endif
            _data{nullptr}, _size{0}, _deleter{} {}

        /**
         * @brief Default constructor
         *
         * Creates a zero-sized array. Move an @ref Array with a nonzero size
         * onto the instance to make it useful.
         */
        /*implicit*/ Array() noexcept: _data(nullptr), _size(0), _deleter{} {}

        /**
         * @brief Construct a default-initialized array
         *
         * Creates array of given size, the contents are default-initialized
         * (i.e. trivial types are not initialized, default constructor called
         * otherwise). If the size is zero, no allocation is done. Because of
         * the differing behavior for trivial types it's better to explicitly
         * use either the @ref Array(ValueInitT, std::size_t) or the
         * @ref Array(NoInitT, std::size_t) variant instead.
         * @see @ref DefaultInit, @ref std::is_trivial
         */
        explicit Array(Corrade::DefaultInitT, std::size_t size): _data{size ? new T[size] : nullptr}, _size{size}, _deleter{nullptr} {}

        /**
         * @brief Construct a value-initialized array
         *
         * Creates array of given size, the contents are value-initialized
         * (i.e. builtin types are zero-initialized, default constructor called
         * otherwise). This is the same as @ref Array(std::size_t). If the size
         * is zero, no allocation is done.
         * @see @ref ValueInit, @ref Array(DefaultInitT, std::size_t)
         */
        explicit Array(Corrade::ValueInitT, std::size_t size): _data{size ? new T[size]() : nullptr}, _size{size}, _deleter{nullptr} {}

        /**
         * @brief Construct an array without initializing its contents
         *
         * Creates an array of given size, the contents are *not* initialized.
         * If the size is zero, no allocation is done. Useful if you will be
         * overwriting all elements later anyway or if you need to call custom
         * constructors in a way that's not expressible via any other
         * @ref Array constructor.
         *
         * For trivial types is equivalent to @ref Array(DefaultInitT, std::size_t),
         * with @ref deleter() being the default (@cpp nullptr @ce) as well.
         * For non-trivial types, the data are allocated as a @cpp char @ce
         * array. Destruction is done using a custom deleter that explicitly
         * calls the destructor on *all elements* and then deallocates the data
         * as a @cpp char @ce array again --- which means that for non-trivial
         * types you're expected to construct all elements using placement new
         * (or for example @ref std::uninitialized_copy()) in order to avoid
         * calling destructors on uninitialized memory:
         *
         * @snippet Containers.cpp Array-NoInit
         *
         * @see @ref NoInit, @ref Array(DirectInitT, std::size_t, Args&&... args),
         *      @ref Array(InPlaceInitT, std::initializer_list<T>),
         *      @ref array(std::initializer_list<T>), @ref deleter(),
         *      @ref std::is_trivial
         */
        explicit Array(Corrade::NoInitT, std::size_t size): _data{size ? Implementation::noInitAllocate<T>(size) : nullptr}, _size{size}, _deleter{Implementation::noInitDeleter<T>()} {}

        /**
         * @brief Construct a direct-initialized array
         *
         * Allocates the array using the @ref Array(NoInitT, std::size_t)
         * constructor and then initializes each element with placement new
         * using forwarded @p args.
         */
        template<class... Args> explicit Array(Corrade::DirectInitT, std::size_t size, Args&&... args);

        /**
         * @brief Construct a list-initialized array
         *
         * Allocates the array using the @ref Array(NoInitT, std::size_t)
         * constructor and then copy-initializes each element with placement
         * new using values from @p list. To save typing you can also use the
         * @ref array(std::initializer_list<T>) shorthand.
         *
         * Not present as an implicit constructor in order to avoid the same
         * usability issues as with @ref std::vector --- see the
         * @ref Containers-Array-initializer-list "class documentation" for
         * more information.
         */
        /*implicit*/ Array(Corrade::InPlaceInitT, std::initializer_list<T> list);

        /**
         * @brief Construct a value-initialized array
         *
         * Alias to @ref Array(ValueInitT, std::size_t).
         * @see @ref Array(DefaultInitT, std::size_t)
         */
        explicit Array(std::size_t size): Array{Corrade::ValueInit, size} {}

        /**
         * @brief Wrap an existing array with an explicit deleter
         *
         * The @p deleter will be *unconditionally* called on destruction with
         * @p data and @p size as an argument. In particular, it will be also
         * called if @p data is @cpp nullptr @ce or @p size is @cpp 0 @ce.
         *
         * In case of a moved-out instance, the deleter gets reset to a
         * default-constructed value alongside the array pointer and size. For
         * plain deleter function pointers it effectively means
         * @cpp delete[] nullptr @ce gets called when destructing a moved-out
         * instance (which is a no-op), for stateful deleters you have to
         * ensure the deleter similarly does nothing in its default state.
         * @see @ref Containers-Array-usage-wrapping
         */
        /** @todo invent some trickery to provide the default only if D is not
            a function pointer, to avoid accidental malloc()/delete[]
            mismatches (String already has the deleter explicit, e.g.) */
        /* GCC <=4.8 breaks on _deleter{} */
        explicit Array(T* data, std::size_t size, D deleter = {}): _data{data}, _size{size}, _deleter(deleter) {}

        /** @brief Copying is not allowed */
        Array(const Array<T, D>&) = delete;

        /**
         * @brief Move constructor
         *
         * Resets data pointer, size and deleter of @p other to be equivalent
         * to a default-constructed instance.
         */
        Array(Array<T, D>&& other) noexcept;

        /**
         * @brief Destructor
         *
         * Calls @ref deleter() on the owned @ref data().
         */
        ~Array() { Implementation::CallDeleter<T, D>{}(_deleter, _data, _size); }

        /** @brief Copying is not allowed */
        Array<T, D>& operator=(const Array<T, D>&) = delete;

        /**
         * @brief Move assignment
         *
         * Swaps data pointer, size and deleter of the two instances.
         */
        Array<T, D>& operator=(Array<T, D>&& other) noexcept;

        /* The following view conversion is *not* restricted to this& because
           that would break uses like `consume(foo());`, where `consume()`
           expects a view but `foo()` returns an owning array. */

        /**
         * @brief Convert to external view representation
         *
         * @see @ref Containers-Array-stl
         */
        template<class U, class = decltype(Implementation::ArrayViewConverter<T, U>::to(std::declval<ArrayView<T>>()))> /*implicit*/ operator U() {
            return Implementation::ArrayViewConverter<T, U>::to(*this);
        }

        /** @overload */
        template<class U, class = decltype(Implementation::ArrayViewConverter<const T, U>::to(std::declval<ArrayView<const T>>()))> constexpr /*implicit*/ operator U() const {
            return Implementation::ArrayViewConverter<const T, U>::to(*this);
        }

        #ifndef CORRADE_MSVC_COMPATIBILITY
        /** @brief Whether the array is non-empty */
        /* Disabled on MSVC w/o /permissive- to avoid ambiguous operator+()
           when doing pointer arithmetic. */
        explicit operator bool() const { return _data; }
        #endif

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
         * @brief Array slice
         *
         * Equivalent to @ref ArrayView::slice(T*, T*) const and overloads.
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
         * Equivalent to @ref ArrayView::slice(T*) const and overloads.
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
         * @brief Fixed-size array slice
         * @m_since{2019,10}
         *
         * Equivalent to @ref ArrayView::slice() const.
         */
        template<std::size_t begin_, std::size_t end_> StaticArrayView<end_ - begin_, T> slice() {
            return ArrayView<T>(*this).template slice<begin_, end_>();
        }

        /**
         * @overload
         * @m_since{2019,10}
         */
        template<std::size_t begin_, std::size_t end_> StaticArrayView<end_ - begin_, const T> slice() const {
            return ArrayView<const T>(*this).template slice<begin_, end_>();
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to @ref ArrayView::prefix(T*) const and overloads.
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
         * Equivalent to @ref ArrayView::suffix(T*) const and overloads.
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
         * @brief Array prefix except the last @p count items
         * @m_since{2019,10}
         *
         * Equivalent to @ref ArrayView::except().
         */
        ArrayView<T> except(std::size_t count) {
            return ArrayView<T>(*this).except(count);
        }
        /**
         * @overload
         * @m_since{2019,10}
         */
        ArrayView<const T> except(std::size_t count) const {
            return ArrayView<const T>(*this).except(count);
        }

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets data pointer, size and deleter
         * to be equivalent to a default-constructed instance. Deleting the
         * returned array is user responsibility --- note the array might have
         * a custom @ref deleter() and so @cpp delete[] @ce might not be always
         * appropriate.
         */
        T* release();

    private:
        T* _data;
        std::size_t _size;
        D _deleter;
};

/** @relatesalso Array
@brief Construct a list-initialized array
@m_since{2020,06}

Convenience shortcut to the @ref Array::Array(InPlaceInitT, std::initializer_list<T>)
constructor. Not present as an implicit constructor in order to avoid the same
usability issues as with @ref std::vector --- see the
@ref Containers-Array-initializer-list "class documentation" for more
information.
*/
template<class T> inline Array<T> array(std::initializer_list<T> list) {
    return Array<T>{Corrade::InPlaceInit, list};
}

/** @relatesalso ArrayView
@brief Make a view on an @ref Array

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

@snippet Containers.cpp Array-arrayView
*/
template<class T, class D> inline ArrayView<T> arrayView(Array<T, D>& array) {
    return ArrayView<T>{array};
}

/** @relatesalso ArrayView
@brief Make a view on a const @ref Array

Convenience alternative to converting to an @ref ArrayView explicitly. The
following two lines are equivalent:

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
    other._deleter = D{};
}

template<class T, class D> template<class ...Args> Array<T, D>::Array(Corrade::DirectInitT, std::size_t size, Args&&... args): Array{Corrade::NoInit, size} {
    for(std::size_t i = 0; i != size; ++i)
        /* This works around a featurebug in C++ where new T{} doesn't work for
           an explicit defaulted constructor. Additionally it works around GCC
           4.8 bugs where copy/move construction can't be done with {} for
           plain structs. */
        Implementation::construct(_data[i], Utility::forward<Args>(args)...);
}

template<class T, class D> Array<T, D>::Array(Corrade::InPlaceInitT, std::initializer_list<T> list): Array{Corrade::NoInit, list.size()} {
    std::size_t i = 0;
    for(const T& item: list)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(_data[i++], item);
        #else
        new(_data + i++) T{item};
        #endif
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
    T* const data = _data;
    _data = nullptr;
    _size = 0;
    _deleter = D{};
    return data;
}

namespace Implementation {

/* Array to ArrayView in order to have implicit conversion for StridedArrayView
   without needing to introduce a header dependency. The SFINAE needs to be
   here in order to ensure proper behavior with function overloads taking more
   than one type of (Strided)ArrayView. */
template<class U, class T, class D> struct ArrayViewConverter<U, Array<T, D>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<U>>::type from(Array<T, D>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<U>>::type from(Array<T, D>&& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<class U, class T, class D> struct ArrayViewConverter<const U, Array<T, D>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<const U>>::type from(const Array<T, D>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<class U, class T, class D> struct ArrayViewConverter<const U, Array<const T, D>> {
    template<class V = U> constexpr static typename std::enable_if<std::is_convertible<T*, V*>::value, ArrayView<const U>>::type from(const Array<const T, D>& other) {
        static_assert(sizeof(T) == sizeof(U), "types are not compatible");
        return {&other[0], other.size()};
    }
};
template<class T, class D> struct ErasedArrayViewConverter<Array<T, D>>: ArrayViewConverter<T, Array<T, D>> {};
template<class T, class D> struct ErasedArrayViewConverter<const Array<T, D>>: ArrayViewConverter<const T, Array<T, D>> {};

}

}}

#endif
