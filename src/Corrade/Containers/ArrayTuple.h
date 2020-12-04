#ifndef Corrade_Containers_ArrayTuple_h
#define Corrade_Containers_ArrayTuple_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::ArrayTuple
 * @m_since_latest
 */

#include <initializer_list>
#include <utility>

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/constructHelpers.h"
#include "Corrade/Containers/Tags.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

/**
@brief Array tuple
@m_since_latest

A set of arrays of homogeneous types and varying lengths stored in a single
allocation. Compared to creating several @ref Array instances, this has the
advantage of using a single contiguous piece of memory with less allocator
overhead and potentially better cache performance, especially if there's many
short arrays. On the other hand, when there's many items or you need to
individually @ref Containers-Array-growable "grow each array", using dedicated
@ref Array instances may be a better option.

A common use case is when dealing with C APIs that accept pointers to several
different arrays and the sizes are not known at compile time. The following
snippet shows filling a Vulkan @m_class{m-doc-external} [VkRenderpassCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkRenderPassCreateInfo.html)
structure with dynamic attachment, subpass and subpass dependency description:

@snippet Containers.cpp ArrayTuple-usage

While concrete layout of the `data` array is internal to the implementation,
the `attachments`, `subpasses` and `dependencies` views get set to correctly
sized and aligned non-overlapping sub-ranges that you can fill afterwards. The
memory is owned by the @ref ArrayTuple instance, thus the views will be valid
only for as long as the instance exists. It's up to you what happens to the
views after --- in the above case, all needed information is already contained
in the `info` structure, so the views aren't needed after anymore.

@section Containers-ArrayTuple-nontrivial Storing non-trivial types

The usage isn't limited to just trivial types --- by default (or if you
explicitly specify @ref ValueInit) it'll value-construct the items and will
also correctly call destructors at the end. Moreover, each sub-array is padded
to match alignment requirements of its type. You can also specify @ref NoInit,
which will keep the contents uninitialized, allowing you to use a non-default
constructor or skip zero-initialization of builtin types when not necessary:

@snippet Containers.cpp ArrayTuple-usage-nontrivial

@section Containers-ArrayTuple-allocators-deleters Custom allocations and deleters

Like other containers, it's possible to provide custom-allocated memory. In
this case it's slightly more complicated because the actual memory size isn't
known beforehand, so you'll need to give the class an allocator and deleter
function at once. The following rather contrived example shows creating a
memory-mapped file for 400 MB of measurement data, which will allow it to be
offloaded to disk in case of a memory pressure:

@snippet Containers.cpp ArrayTuple-usage-mmap

See the @ref ArrayTuple(ArrayView<const Item>, A) constructor documentation for
a detailed description of the allocator and deleter signature.
*/
class CORRADE_UTILITY_EXPORT ArrayTuple {
    public:
        class Item;

        /**
         * @brief Deleter type
         *
         * Type returned by @ref deleter(), see the function documentation for
         * more information.
         */
        typedef void(*Deleter)(char*, std::size_t);

        /**
         * @brief Constructor
         *
         * If the @p items view is empty, the constructed instance is
         * equivalent to a moved-from state.
         */
        explicit ArrayTuple(ArrayView<const Item> items = {});
        /** @overload */
        explicit ArrayTuple(std::initializer_list<Item> items): ArrayTuple{arrayView(items)} {}

        /**
         * @brief Construct using a custom allocation
         *
         * The @p allocator needs to callable or implement a call operator with
         * a signature of @cpp std::pair<char*, D>(*)(std::size_t, std::size_t) @ce.
         * It gets passed a pair of desired allocation size and alignment of
         * the allocation and should return a pair of an allocated memory
         * pointer and a deleter instance that will be later used to delete the
         * allocation. The allocation alignment is useful mainly when
         * allocating over-aligned types, such as SIMD vectors, as C++ is only
         * guaranteed to align correctly only for the largest standard typeś.
         *
         * The deleter type `D` needs to be one of the following and it gets
         * passed the allocation pointer together with its size (which was
         * earlier passed to the allocator):
         *
         * -    a (stateful) functor, implementing @cpp void operator()(char*, std::size_t) @ce,
         * -    a plain stateless function pointer @cpp void(*)(char*, std::size_t) @ce,
         * -    or a @ref std::nullptr_t, which is equivalent to using the
         *      standard @cpp delete[] @ce.
         *
         * In case of a stateful deleter, its contents are stored inside the
         * allocated memory alongside other metadata. It gets copied to a
         * temporary location before being called (to prevent it from freeing
         * the memory from under itself) and its destructor is called
         * afterwards.
         */
        template<class A> explicit ArrayTuple(ArrayView<const Item> items, A allocator);
        /** @overload */
        template<class A> explicit ArrayTuple(std::initializer_list<Item> items, A allocator): ArrayTuple{arrayView(items), allocator} {}

        /** @brief Copying is not allowed */
        ArrayTuple(const ArrayTuple&) = delete;

        /**
         * @brief Move constructor
         *
         * Resets data pointer and deleter of @p other to be equivalent to a
         * default-constructed instance.
         */
        ArrayTuple(ArrayTuple&& other) noexcept;

        /**
         * @brief Destructor
         *
         * Calls @ref deleter() on the owned @ref data().
         */
        ~ArrayTuple();

        /** @brief Copying is not allowed */
        ArrayTuple& operator=(const ArrayTuple&) = delete;

        /**
         * @brief Move assignment
         *
         * Swaps data pointer and deleter of the two instances.
         */
        ArrayTuple& operator=(ArrayTuple&& other) noexcept;

        /**
         * @brief Move-conversion to an @ref Array
         *
         * Meant for dealing with APIs that accept untyped arrays as a storage.
         * To avoid unforeseeable consequences stemming from the deleter
         * storing its state inside the array it's deleting, the conversion is
         * allowed only in case the array tuple stores trivially-destructible
         * types and has either a default or a stateless @ref deleter(). If you
         * need to create an @ref Array regardless, do it manually using
         * @ref deleter(), @ref size() and @ref release().
         */
        /*implicit*/ operator Array<char>() &&;

        /**
         * @brief Array tuple data
         *
         * Note that the data contents and layout is implementation-defined.
         */
        char* data() { return _data; }
        const char* data() const { return _data; } /**< @overload */

        /** @brief Array tuple data size */
        std::size_t size() const { return _size; }

        /**
         * @brief Array tuple deleter
         *
         * If set to @cpp nullptr @ce, the contents are deleted using standard
         * @cpp operator delete[] @ce. The returned type is always a plain
         * function pointer even in case the array has a custom stateful
         * deleter, as the function has to perform destructor calls for
         * non-trivially-destructible array items before to executing the
         * actual memory deleter.
         * @see @ref ArrayTuple(ArrayView<const Item>, A)
         */
        Deleter deleter() const { return _deleter; }

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets data pointer, size and deleter
         * to be equivalent to a default-constructed instance. Deleting the
         * returned array is user responsibility --- usually the array has a
         * custom @ref deleter() that additionally takes care of calling
         * destructors of non-trivially-destructible array types. Using a plain
         * @cpp delete[] @ce is appropriate only if @ref deleter() is
         * @cpp nullptr @ce.
         * @see @ref operator Array<char>()
         */
        char* release();

    private:
        static std::pair<std::size_t, std::size_t> sizeAlignmentFor(ArrayView<const Item> items, const Item& arrayDeleterItem, std::size_t& destructibleItemCount, bool& arrayDeleterItemNeeded);

        void create(ArrayView<const Item> items, const Item& arrayDeleterItem, std::size_t destructibleItemCount, bool arrayDeleterItemNeeded);

        char* _data;
        std::size_t _size;
        Deleter _deleter;
};

#ifdef CORRADE_MSVC2015_COMPATIBILITY
namespace Implementation {
    /* Just copies of the lambdas below because MSVC 2015 can't understand
       template parameters inside lambdas */
    template<class T> inline void callDeleter(char* data, std::size_t) {
        reinterpret_cast<T*>(data)->~T();
    }
    template<class D> inline void wrapStatefulDeleter(char* state, std::size_t size) {
        D deleter = *reinterpret_cast<D*>(state);
        deleter(state + sizeof(D) - size, size);
        deleter.~D();
    }
}
#endif

/**
@brief Array tuple item

Stores desired size and output view reference. See @ref ArrayTuple for usage introduction.
*/
class ArrayTuple::Item {
    public:
        /**
         * @brief Construct a view with value-initialized elements
         * @param[in] size          Desired view size
         * @param[out] outputView   Desired type and a reference where to store
         *      the resulting view
         *
         * All @p size elements are value-initialized (i.e., builtin types are
         * zero-initialized and the default constructor gets called otherwise).
         * Expects that @p T is default-constructible. If it's not, you have to
         * use @ref Item(NoInitT, std::size_t, ArrayView<T>&) instead and then
         * manually construct each item in-place.
         */
        template<class T> /*implicit*/ Item(ValueInitT, std::size_t size, ArrayView<T>& outputView): Item{NoInit, size, outputView} {
            static_assert(std::is_default_constructible<T>::value,
                "can't default-init a type with no default constructor, use NoInit instead and manually initialize each item");
            _constructor = [](void* data) {
                /* Default-construct the T and work around various compiler
                   issues, see construct() for details */
                Implementation::construct(*static_cast<T*>(data));
            };
        }

        /**
         * @brief Construct a view with value-initialized elements
         *
         * Alias to @ref Item(ValueInitT, std::size_t, ArrayView<T>&).
         */
        template<class T> /*implicit*/ Item(std::size_t size, ArrayView<T>& outputView): Item{ValueInit, size, outputView} {}

        /**
         * @brief Construct a view without initializing its elements
         * @param[in] size          Desired view size
         * @param[out] outputView   Desired type and a reference where to store
         *      the resulting view
         *
         * Initialize the values using placement new. Useful if you will be
         * overwriting all elements later anyway, or if the elements have no
         * default constructor. Note that for non-trivial types the destructor
         * gets finally called on *all elements*, regardless of whether they
         * were properly constructed or not.
         */
        template<class T> /*implicit*/ Item(NoInitT, std::size_t size, ArrayView<T>& outputView):
            _elementSize{sizeof(T)}, _elementAlignment{alignof(T)}, _elementCount{size},
            _constructor{},
            _destructor{std::is_trivially_destructible<T>::value ? static_cast<void(*)(char*, std::size_t)>(nullptr) :
                /* MSVC 2015 complains that
                    error C2061: syntax error: identifier 'T'
                   in the lambda, working around that by passing a pointer to
                   a real function instead */
                #ifndef CORRADE_MSVC2015_COMPATIBILITY
                [](char* data, std::size_t) {
                    reinterpret_cast<T*>(data)->~T();
                }
                #else
                Implementation::callDeleter<T>
                #endif
            },
            _destinationPointer{&reinterpret_cast<void*&>(Implementation::dataRef(outputView))}
        {
            /* Populate size of the output view. Pointer gets update inside
               create(). */
            outputView = {nullptr, size};
        }

    private:
        friend ArrayTuple;

        /* The three following constructors are used by
           ArrayTuple(ArrayView<const Item>, A), depending on what the actual
           deleter type is */

        /* If the deleter is the default one, we'll store a wrapper around
           delete[] that might (or might not) get used */
        explicit Item(void*, std::nullptr_t*& deleterDestination): _elementSize{0}, _elementAlignment{0}, _elementCount{1}, _constructor{}, _destructor{[](char* data, std::size_t) {
            delete[] data;
        }}, _destinationPointer{reinterpret_cast<void**>(&deleterDestination)} {}

        /* Otherwise, if the deleter is a stateless function pointer, we let
           the destructor pointer empty. The pointer, once it's known, will be
           saved to the location provided via _outputPointer. */
        explicit Item(void*, void(**& deleterDestination)(char*, std::size_t)): _elementSize{sizeof(void(*)(char*, std::size_t))}, _elementAlignment{0}, _elementCount{1}, _constructor{}, _destructor{}, _destinationPointer{reinterpret_cast<void**>(&deleterDestination)} {}

        /* Otherwise, if stateful, we need a wrapper. The deleter function
           pointer retrieves the deleter state pointer, calls it and then
           destructs the deleter state. The state, once it's known, will be
           saved to the location provided via _outputPointer. */
        template<class D, class = typename std::enable_if<!std::is_convertible<D, void(*)(char*, std::size_t)>::value>::type> explicit Item(void*, D*& deleterDestination): _elementSize{sizeof(D)}, _elementAlignment{alignof(D)}, _elementCount{1}, _constructor{}, _destructor{
            /* MSVC 2015 complains that
                error C2146: syntax error: missing ';' before identifier 'deleter'
                error C2061: syntax error: identifier 'D
               in the lambda, working around that by passing a pointer to a
               real function instead */
            #ifndef CORRADE_MSVC2015_COMPATIBILITY
            [](char* state, std::size_t size) {
                /* Make a copy of the deleter first to avoid the deleter
                   deleting its own state, because then ~D() would touch a
                   memory that's no longer there */
                D deleter = *reinterpret_cast<D*>(state);
                deleter(state + sizeof(D) - size, size);
                deleter.~D();
            }
            #else
            Implementation::wrapStatefulDeleter<D>
            #endif
        }, _destinationPointer{reinterpret_cast<void**>(&deleterDestination)} {}

        /* In case of a memory deleter item, element size is 0 for the default
           deleter, sizeof(void*) for stateless function pointers and size of
           the deleter state otherwise. */
        std::size_t _elementSize,
            /* alignment is 0 if the deleter is the default one or a stateless
               function pointer which doesn't need to be stored explicitly, and
               alignment of the deleter state for stateful deleters */
            _elementAlignment,
            /* element count is always 1 */
            _elementCount;

        /* Constructor is null if using the NoInit constructor; in case of
           memory deleters it's null always */
        void(*_constructor)(void*);

        /* Destructor is set for non-trivially-destructible types; in case of
           memory deleters only if it's a default or a stateful deleter */
        void(*_destructor)(char*, std::size_t);

        /* Output pointer is always set */
        void** _destinationPointer;
};

template<class A> ArrayTuple::ArrayTuple(ArrayView<const Item> items, A allocator) {
    /* The allocator is expected to return std::pair<char*, D>, where the
       second value is a deleter instance */
    typedef decltype(allocator(std::size_t{}, std::size_t{}).second) D;

    D* deleterDestination = nullptr;
    Item arrayDeleterItem{nullptr, deleterDestination};

    /* Calculate total size and allocate the memory using the custom allocator,
       and get a memory pointer and a deleter instance back */
    std::size_t destructibleItemCount;
    bool arrayDeleterItemNeeded;
    std::pair<std::size_t, std::size_t> sizeAlignment = sizeAlignmentFor(items, arrayDeleterItem, destructibleItemCount, arrayDeleterItemNeeded);
    /* This needs to be const in order to pass const D& to the construct()
       workaround below, which will then trigger correct copy constructor call
       on GCC 4.8. Sigh. */
    const std::pair<char*, D> allocated = allocator(sizeAlignment.first, sizeAlignment.second);
    _size = sizeAlignment.first;
    _data = allocated.first;

    /* Create the internal state, which, in case the deleter is not default,
       will populate the deleterDestination pointer above. To which we then
       save the deleter -- either its state for a stateful one, or a function
       pointer for a stateless one. */
    create(items, arrayDeleterItem, destructibleItemCount, arrayDeleterItemNeeded);
    /* Doing a placement-new initialization as calling a copy constructor to an
       uninitialized memory may not do the right thing. In case it's a plain
       function pointer, the two are equivalent. Also using a helper
       instead of D{allocated.second} in order to avoid various
       compiler-specific issues related to {}, see construct() for details. */
    if(deleterDestination)
        Implementation::construct(*deleterDestination, allocated.second);
}

}}

#endif
