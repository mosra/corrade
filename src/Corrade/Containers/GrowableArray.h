#ifndef Corrade_Containers_GrowableArray_h
#define Corrade_Containers_GrowableArray_h
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
 * @brief Class @ref Corrade::Containers::ArrayAllocator, @ref Corrade::Containers::ArrayNewAllocator, @ref Corrade::Containers::ArrayMallocAllocator, function @ref Corrade::Containers::arrayAllocatorCast(), @ref Corrade::Containers::arrayIsGrowable(), @ref Corrade::Containers::arrayCapacity(), @ref Corrade::Containers::arrayReserve(), @ref Corrade::Containers::arrayResize(), @ref Corrade::Containers::arrayAppend(), @ref Corrade::Containers::arrayRemoveSuffix(), @ref Corrade::Containers::arrayShrink()
 * @m_since{2020,06}
 */

#include <cstdlib>
#include <cstring>

#include "Corrade/Containers/Array.h"
#include "Corrade/Utility/TypeTraits.h"

/* No __has_feature on GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60512
   Using a dedicated macro instead: https://stackoverflow.com/a/34814667 */
#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif
#endif
#ifdef __SANITIZE_ADDRESS__
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
/* https://github.com/llvm-mirror/compiler-rt/blob/master/include/sanitizer/common_interface_defs.h */
extern "C" void __sanitizer_annotate_contiguous_container(const void *beg,
    const void *end, const void *old_mid, const void *new_mid);
#endif

namespace Corrade { namespace Containers {

/** @{ @name Growable array utilities
 *
 * See @ref Containers-Array-growable for more information.
 */

/**
@brief New-based allocator for growable arrays
@m_since{2020,06}

An @ref ArrayAllocator that allocates and deallocates memory using the C++
@cpp new[] @ce / @cpp delete[] @ce constructs, reserving an extra space
* *before* to store array capacity. Expects that @p T is nothrow
move-constructible.
@see @ref Containers-Array-growable
*/
template<class T> struct ArrayNewAllocator {
    typedef T Type; /**< Pointer type */

    /**
     * @brief Allocate (but not construct) an array of given capacity
     *
     * @cpp new[] @ce-allocates a @cpp char @ce array with an extra space to
     * store @p capacity *before* the front, returning it cast to @cpp T* @ce.
     */
    static T* allocate(std::size_t capacity) {
        char* memory = new char[capacity*sizeof(T) + sizeof(std::size_t)];
        reinterpret_cast<std::size_t*>(memory)[0] = capacity;
        return reinterpret_cast<T*>(memory + sizeof(std::size_t));
    }

    /**
     * @brief Reallocate an array to given capacity
     *
     * Calls @p allocate(), move-constructs @p prevSize elements from @p array
     * into the new array, calls destructors on the original elements, calls
     * @ref deallocate() and updates the @p array reference to point to the new
     * array.
     */
    static void reallocate(T*& array, std::size_t prevSize, std::size_t newCapacity);

    /**
     * @brief Deallocate an array
     *
     * Calls @cpp delete[] @ce on a pointer offset by the extra space needed to
     * store its capacity.
     */
    static void deallocate(T* data) {
        delete[] (reinterpret_cast<char*>(data) - sizeof(std::size_t));
    }

    /**
     * @brief Grow the array
     *
     * If current occupied size (including the space needed to store capacity)
     * is less than 64 bytes, the capacity always doubled, with the allocation
     * being at least as large as @cpp __STDCPP_DEFAULT_NEW_ALIGNMENT__ @ce
     * (or @cpp 2*sizeof(std::size_t) @ce when the define is not available
     * pre-C++17). After that, the capacity is increased to 1.5x of current
     * capacity (again including the space needed to store capacity). This is
     * similar to what MSVC STL does with @ref std::vector, except for libc++ /
     * libstdc++, which both use a factor of 2. With a factor of 2 the
     * allocation would crawl forward in memory, never able to reuse the holes
     * after previous allocations, with a factor 1.5 it's possible after four
     * reallocations. Further info in [Folly FBVector docs](https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md#memory-handling).
     */
    static std::size_t grow(T* array, std::size_t desired);

    /**
     * @brief Array capacity
     *
     * Retrieves the capacity that's stored *before* the front of the @p array.
     */
    static std::size_t capacity(T* array) {
        return *reinterpret_cast<std::size_t*>(reinterpret_cast<char*>(array) - sizeof(std::size_t));
    }

    /**
     * @brief Array base address
     *
     * Returns the address with @cpp sizeof(std::size_t) @ce subtracted.
     */
    static void* base(T* array) {
        return reinterpret_cast<char*>(array) - sizeof(std::size_t);
    }

    /**
     * @brief Array deleter
     *
     * Calls a destructor on @p size elements and then delegates into
     * @ref deallocate().
     */
    static void deleter(T* data, std::size_t size) {
        for(T *it = data, *end = data + size; it != end; ++it) it->~T();
        deallocate(data);
    }
};

#ifndef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
namespace Implementation {

/* The std::has_trivial_default_constructor / std::has_trivial_copy_constructor
   is deprecated in GCC 5+ but we can't detect libstdc++ version when using
   Clang. The builtins aren't deprecated but for those GCC commits suicide with
    error: use of built-in trait ‘__has_trivial_copy(T)’ in function signature; use library traits instead
   so, well, i'm defining my own! See CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
   for even more fun stories. */
template<class T> struct IsTriviallyConstructibleOnOldGcc: std::integral_constant<bool, __has_trivial_constructor(T)> {};
/* Need also __has_trivial_destructor() otherwise it says true for types with
   deleted copy and non-trivial destructors */
template<class T> struct IsTriviallyCopyableOnOldGcc: std::integral_constant<bool, __has_trivial_copy(T) && __has_trivial_destructor(T)> {};

}
#endif

/**
@brief Malloc-based allocator for growable arrays
@m_since{2020,06}

An @ref ArrayAllocator that allocates and deallocates memory using the C
@ref std::malloc() / @ref std::free() constructs in order to be able to use
@ref std::realloc() for fast reallocations. Expects that @p T is trivially
copyable. Similarly to @ref ArrayNewAllocator it's reserving an extra space
* *before* to store array capacity.

Compared to @ref ArrayNewAllocator, this allocator stores array capacity in
bytes and, together with the fact that @ref std::free() doesn't care about the
actual array type, growable arrays using this allocator can be freely cast to
different compatible types using @ref arrayAllocatorCast().
@see @ref Containers-Array-growable
*/
template<class T> struct ArrayMallocAllocator {
    static_assert(
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<T>::value
        #else
        Implementation::IsTriviallyCopyableOnOldGcc<T>::value
        #endif
        , "only trivially copyable types are usable with this allocator");

    typedef T Type; /**< Pointer type */

    /**
     * @brief Allocate an array of given capacity
     *
     * @ref std::malloc()'s an @cpp char @ce array with an extra space to store
     * @p capacity *before* the front, returning it cast to @cpp T* @ce.
     */
    static T* allocate(std::size_t capacity) {
        /* Compared to ArrayNewAllocator, here the capacity is stored in bytes
           so it's possible to "reinterpret" the array into a different type
           (as the deleter is a typeless std::free() in any case) */
        const std::size_t inBytes = capacity*sizeof(T) + sizeof(std::size_t);
        char* const memory = static_cast<char*>(std::malloc(inBytes));
        reinterpret_cast<std::size_t*>(memory)[0] = inBytes;
        return reinterpret_cast<T*>(memory + sizeof(std::size_t));
    }

    /**
     * @brief Reallocate an array to given capacity
     *
     * Calls @ref std::realloc() on @p array (offset by the space to store
     * capacity) and then updates the stored capacity to @p newCapacity and the
     * @p array reference to point to the new (offset) location, in case the
     * reallocation wasn't done in-place. The @p prevSize parameter is ignored,
     * as @ref std::realloc() always copies the whole original capacity.
     */
    static void reallocate(T*& array, std::size_t prevSize, std::size_t newCapacity);

    /**
     * @brief Deallocate an array
     *
     * Calls @ref std::free() on a pointer offset by the extra space needed to
     * store its capacity.
     */
    static void deallocate(T* data) {
        if(data) std::free(reinterpret_cast<char*>(data) - sizeof(std::size_t));
    }

    /**
     * @brief Grow the array
     *
     * Behaves the same as @ref ArrayNewAllocator::grow().
     */
    static std::size_t grow(T* array, std::size_t desired);

    /**
     * @brief Array capacity
     *
     * Retrieves the capacity that's stored *before* the front of the @p array.
     */
    static std::size_t capacity(T* array) {
        return (*reinterpret_cast<std::size_t*>(reinterpret_cast<char*>(array) - sizeof(std::size_t)) - sizeof(std::size_t))/sizeof(T);
    }

    /**
     * @brief Array base address
     *
     * Returns the address with @cpp sizeof(std::size_t) @ce subtracted.
     */
    static void* base(T* array) {
        return reinterpret_cast<char*>(array) - sizeof(std::size_t);
    }

    /**
     * @brief Array deleter
     *
     * Since the types have trivial destructors, directly delegates into
     * @ref deallocate(). The @p size parameter is unused.
     */
    static void deleter(T* data, std::size_t size) {
        static_cast<void>(size);
        deallocate(data);
    }
};

#ifdef DOXYGEN_GENERATING_OUTPUT
/**
@brief Allocator for growable arrays
@m_since{2020,06}

Is either @ref ArrayMallocAllocator for trivially copyable @p T, or
@ref ArrayNewAllocator otherwise. See @ref Containers-Array-growable for an
introduction to growable arrays. You can provide your own allocator by
implementing a class that with @ref Type, @ref allocate(), @ref reallocate(),
@ref deallocate(), @ref grow(), @ref capacity(), @ref base() and @ref deleter()
following the documented semantics.
*/
template<class T> struct ArrayAllocator {
    typedef T Type; /**< Pointer type */

    /**
     * @brief Allocate (but not construct) an array of given capacity
     *
     * Implementations are expected to store the @p capacity in a way that
     * makes it possible to retrieve it later via @ref capacity().
     */
    static T* allocate(std::size_t capacity);

    /**
     * @brief Reallocate an array to given capacity
     *
     * Assumes @p array was returned earlier by @ref allocate() or
     * @ref reallocate(). Implementations are expected to either extend
     * @p array in-place to @p newCapacity or allocate a new array with
     * @p newCapacity, move @p prevSize elements from @p array to it and call
     * destructors on their original location, deallocate @p array it and
     * update the reference to point to the new memory.
     */
    static void reallocate(T*& array, std::size_t prevSize, std::size_t newCapacity);

    /**
     * @brief Deallocate (but not destruct) an array
     *
     * Assumes that @p data was returned earlier by @ref allocate() or
     * @ref reallocate(). Implementations are expected to free all memory
     * associated with @p data.
     */
    static void deallocate(T* data);

    /**
     * @brief Grow the array
     *
     * Assumes that @p array is either @cpp nullptr @ce or was returned earlier
     * by @ref allocate() or @ref reallocate(). Implementations are expected to
     * return a new capacity with an an optimal tradeoff between reallocation
     * count and memory usage, the value then being passed to
     * @ref reallocate().
     */
    static std::size_t grow(T* array, std::size_t desired);

    /**
     * @brief Array capacity
     *
     * Implementations are expected to retrieve the capacity information from
     * @p array.
     */
    static std::size_t capacity(T* array);

    /**
     * @brief Array base address
     *
     * Returns base address of the allocation backing @p array. For use by
     * Address Sanitizer to annotate which area of the allocation is safe to
     * access and which not.
     */
    static void* base(T* array);

    /**
     * @brief Array deleter
     *
     * Passed as a function pointer into @ref Array. Calls destructors on
     * @p size elements and delegates into @ref deallocate(). The deleter
     * function pointer is used to distinguish if given array is using this
     * particular allocator --- you might want to turn this function into an
     * exported symbol when growing arrays across shared library boundaries to
     * avoid each library thinking it's using some other allocator and
     * reallocating on each addition.
     */
    static void deleter(T* data, std::size_t size);
};
#else
template<class T> using ArrayAllocator = typename std::conditional<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    Implementation::IsTriviallyCopyableOnOldGcc<T>::value
    #endif
    , ArrayMallocAllocator<T>, ArrayNewAllocator<T>>::type;
#endif

/**
@brief Reinterpret-cast a growable array
@m_since{2020,06}

If the array is growable using @ref ArrayMallocAllocator (which is aliased to
@ref ArrayAllocator for all trivially-copyable types), the deleter is a simple
call to a typeless @ref std::free(). This makes it possible to change the array
type without having to use a different deleter, losing the growable property in
the process. Example usage:

@snippet Containers.cpp arrayAllocatorCast

Equivalently to to @ref arrayCast(), the size of the new array is calculated as
@cpp view.size()*sizeof(T)/sizeof(U) @ce. Expects that both types are
trivially copyable and [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and the total byte size doesn't change.
*/
template<class U, class T> Array<U> arrayAllocatorCast(Array<T>&& array);

/**
@overload
@m_since{2020,06}
*/
template<class U, template<class> class Allocator, class T> Array<U> arrayAllocatorCast(Array<T>&& array) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    static_assert(
        #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
        std::is_trivially_copyable<T>::value && std::is_trivially_copyable<U>::value
        #else
        Implementation::IsTriviallyCopyableOnOldGcc<T>::value && Implementation::IsTriviallyCopyableOnOldGcc<U>::value
        #endif
        , "only trivially copyable types can use the allocator cast");
    CORRADE_ASSERT(array.data() == nullptr ||
        (array.deleter() == Allocator<T>::deleter && std::is_base_of<ArrayMallocAllocator<T>, Allocator<T>>::value),
        "Containers::arrayAllocatorCast(): the array has to use the ArrayMallocAllocator or a derivative", {});
    const std::size_t size = array.size()*sizeof(T)/sizeof(U);
    CORRADE_ASSERT(size*sizeof(U) == array.size()*sizeof(T),
        "Containers::arrayAllocatorCast(): can't reinterpret" << array.size() << sizeof(T) << Utility::Debug::nospace << "-byte items into a" << sizeof(U) << Utility::Debug::nospace << "-byte type", {});
    return Array<U>{reinterpret_cast<U*>(array.release()), size, Allocator<U>::deleter};
}

template<class U, class T> Array<U> arrayAllocatorCast(Array<T>&& array) {
    return arrayAllocatorCast<U, ArrayAllocator, T>(std::move(array));
}

/**
@brief Whether the array is growable
@m_since{2020,06}

Returns @cpp true @ce if the array is growable and using given @p Allocator,
@cpp false @ce otherwise. Note that even non-growable arrays are usable with
the @ref arrayAppend(), @ref arrayReserve(), ... family of utilities --- these
will reallocate the array using provided allocator if needed.
@see @ref arrayShrink(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> bool arrayIsGrowable(Array<T>& array);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class T> class Allocator, class T> inline bool arrayIsGrowable(Array<T>& array) {
    return arrayIsGrowable<T, Allocator<T>>(array);
}
#endif

/**
@brief Array capacity
@m_since{2020,06}

For a growable array returns its capacity, for a non-growable array returns
@ref Array::size().
@see @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> std::size_t arrayCapacity(Array<T>& array);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class T> class Allocator, class T> inline std::size_t arrayCapacity(Array<T>& array) {
    return arrayCapacity<T, Allocator<T>>(array);
}
#endif

/**
@brief Reserve given capacity in an array
@return New capacity of the array
@m_since{2020,06}

If @p array capacity is already large enough, the function returns the current
capacity. Otherwise the memory is reallocated to desired @p capacity, with the
@ref Array::size() staying the same, and @p capacity returned back. Note that
in case the array is non-growable of sufficient size, it's kept as such,
without being reallocated to a growable version.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the original
container, @f$ \mathcal{O}(1) @f$ if the capacity is already large enough or
if the reallocation can be done in-place.
@see @ref arrayCapacity(), @ref arrayIsGrowable(),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> std::size_t arrayReserve(Array<T>& array, std::size_t capacity);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class T> class Allocator, class T> inline std::size_t arrayReserve(Array<T>& array, std::size_t capacity) {
    return arrayReserve<T, Allocator<T>>(array, capacity);
}
#endif

/**
@brief Resize an array to given size, default-initializing new elements
@m_since{2020,06}

If the array is growable and capacity is large enough, calls a destructor on
elements that get cut off the end (if any, and if @p T is not trivially
destructible, in which case nothing is done) and returns. Otherwise, the memory
is reallocated to desired @p size. After that, new elements at the end of the
array are default-initialized using placement-new (and nothing done for trivial
types). Note that in case the array is non-growable of exactly the requested
size, it's kept as such, without being reallocated to a growable version.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the new container,
@f$ \mathcal{O}(1) @f$ if current container size is already exactly of given
size.
@see @ref arrayCapacity(), @ref arrayIsGrowable(), @ref arrayRemoveSuffix()
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayResize(Array<T>& array, DefaultInitT, std::size_t size);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, DefaultInitT, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, DefaultInit, size);
}
#endif

/**
@brief Resize an array to given size, value-initializing new elements
@m_since{2020,06}

Similar to @ref arrayResize(Array<T>&, DefaultInitT, std::size_t) except that
the new elements at the end are not default-initialized, but value-initialized
(i.e., trivial types zero-initialized and default constructor called
otherwise).
@see @ref Array::size(), @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayResize(Array<T>& array, ValueInitT, std::size_t size);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, ValueInitT, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, ValueInit, size);
}
#endif

/**
@brief Resize an array to given size, value-initializing new elements
@m_since{2020,06}

Alias to @ref arrayResize(Array<T>&, ValueInitT, std::size_t).
*/
template<class T, class Allocator = ArrayAllocator<T>> inline void arrayResize(Array<T>& array, std::size_t size) {
    return arrayResize<T, Allocator>(array, ValueInit, size);
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, size);
}
#endif

/**
@brief Resize an array to given size, keeping new elements uninitialized
@m_since{2020,06}

Similar to @ref arrayResize(Array<T>&, DefaultInitT, std::size_t) except that
the new elements at the end are not default-initialized, but left in an
uninitialized state instead.
@see @ref Array::size(), @ref arrayIsGrowable(), @ref Containers-Array-growable,
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t)
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayResize(Array<T>& array, NoInitT, std::size_t size);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, NoInitT, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, NoInit, size);
}
#endif

/**
@brief Resize an array to given size, constructing new elements using provided arguments
@m_since{2020,06}

Similar to @ref arrayResize(Array<T>&, DefaultInitT, std::size_t) except that
the new elements at the end are constructed using placement-new with provided
@p args.
*/
template<class T, class... Args> void arrayResize(Array<T>& array, DirectInitT, std::size_t size, Args&&... args);

/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator, class... Args> void arrayResize(Array<T>& array, DirectInitT, std::size_t size, Args&&... args);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class... Args> inline void arrayResize(Array<T>& array, DirectInitT, std::size_t size, Args&&... args) {
    arrayResize<T, Allocator<T>>(array, DirectInit, size, std::forward<Args>(args)...);
}
#endif

/**
@brief Copy-append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

If the array is not growable or the capacity is not large enough, the array capacity is grown first. Then, @p value is copy-constructed at the end of the
array and @ref Array::size() increased by 1.

Amortized complexity is @f$ \mathcal{O}(1) @f$ providing the allocator growth
ratio is exponential.
@see @ref arrayCapacity(), @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> T& arrayAppend(Array<T>& array, const T& value);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline T& arrayAppend(Array<T>& array, const T& value) {
    return arrayAppend<T, Allocator<T>>(array, value);
}
#endif

/**
@brief In-place append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

Similar to @ref arrayAppend(Array<T>&, const T&) except that the new element
is constructed using placement-new with provided @p args.
*/
template<class T, class... Args> T& arrayAppend(Array<T>& array, InPlaceInitT, Args&&... args);

/* This crap tool can't distinguish between these and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator, class... Args> T& arrayAppend(Array<T>& array, InPlaceInitT, Args&&... args);

/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class... Args> inline T& arrayAppend(Array<T>& array, InPlaceInitT, Args&&... args) {
    return arrayAppend<T, Allocator<T>>(array, InPlaceInit, std::forward<Args>(args)...);
}
#endif

/**
@brief Move-append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

Calls @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args) with @p value.
*/
template<class T, class Allocator = ArrayAllocator<T>> inline T& arrayAppend(Array<T>& array, T&& value) {
    return arrayAppend<T, Allocator>(array, InPlaceInit, std::move(value));
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline T& arrayAppend(Array<T>& array, T&& value) {
    return arrayAppend<T, Allocator<T>>(array, InPlaceInit, std::move(value));
}
#endif

/**
@brief Append a list of items to an array
@return View on the newly appended items
@m_since{2020,06}

Like @ref arrayAppend(Array<T>&, const T&), but inserting multiple values at
once.
@see @ref arrayResize(Array<T>&, NoInitT, std::size_t)
*/
template<class T, class Allocator = ArrayAllocator<T>> Containers::ArrayView<T> arrayAppend(Array<T>& array, Containers::ArrayView<const T> values);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline Containers::ArrayView<T> arrayAppend(Array<T>& array, Containers::ArrayView<const T> values) {
    return arrayAppend<T, Allocator<T>>(array, values);
}
#endif

/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator = ArrayAllocator<T>> Containers::ArrayView<T>  arrayAppend(Array<T>& array, std::initializer_list<T> values);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline Containers::ArrayView<T>  arrayAppend(Array<T>& array, std::initializer_list<T> values) {
    return arrayAppend<T, Allocator<T>>(array, values);
}
#endif

/**
@brief Append given count of uninitialized values to the array
@return View on the newly appended items
@m_since{2020,06}

A lower-level variant of
@ref arrayAppend(Array<T>& array, Containers::ArrayView<const T>) where the new
values are meant to be initialized in-place after, instead of being copied from
a pre-existing location.
*/
template<class T, class Allocator = ArrayAllocator<T>> Containers::ArrayView<T> arrayAppend(Array<T>& array, NoInitT, std::size_t count);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline Containers::ArrayView<T> arrayAppend(Array<T>& array, NoInitT, std::size_t count) {
    return arrayAppend<T, Allocator<T>>(array, NoInit, count);
}
#endif

/**
@brief Remove a suffix from the array
@m_since{2020,06}

Expects that @p count is not larger than @ref Array::size(). If the array is
not growable, all its elements except the suffix are first reallocated to a
growable version. Otherwise, a destructor is called on removed elements and the
@ref Array::size() is decreased by @p count.
@see @ref arrayIsGrowable(), @ref arrayResize()
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayRemoveSuffix(Array<T>& array, std::size_t count = 1);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayRemoveSuffix(Array<T>& array, std::size_t count = 1) {
    arrayRemoveSuffix<T, Allocator<T>>(array, count);
}
#endif

/**
@brief Convert an array back to non-growable
@m_since{2020,06}

Allocates a @ref NoInit array that's exactly large enough to fit
@ref Array::size() elements, move-constructs the elements there and frees the
old memory using @ref Array::deleter(). If the array is not growable, it's
assumed to be already as small as possible, and nothing is done.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the container,
@f$ \mathcal{O}(1) @f$ if the array is already non-growable.
@see @ref arrayShrink(Array<T>&, DefaultInitT), @ref arrayIsGrowable(),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayShrink(Array<T>& array, NoInitT = NoInit);

/**
@brief Convert an array back to non-growable using a default initialization
@m_since_latest

Allocates a @ref DefaultInit array that's exactly large enough to fit
@ref Array::size() elements, move-assigns the elements there and frees the old
memory using @ref Array::deleter(). If the array is not growable, it's assumed
to be already as small as possible, and nothing is done.

Compared to @ref arrayShrink(Array<T>&, NoInitT) this overload works only with
types that are default-constructible, and the resulting array instance always
has a default (@cpp nullptr @ce) deleter. This is useful when it's not possible
to use custom deleters, such as in plugin implementations.
@see @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayShrink(Array<T>& array, DefaultInitT);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayShrink(Array<T>& array) {
    arrayShrink<T, Allocator<T>>(array);
}
#endif

/* Since 1.8.17, the original short-hand group closing doesn't work anymore.
   FFS. */
/**
 * @}
 */

namespace Implementation {

/* Used to avoid calling getter functions to speed up debug builds */
template<class T> struct ArrayGuts {
    T* data;
    std::size_t size;
    void(*deleter)(T*, std::size_t);
};

template<class T> inline void arrayConstruct(DefaultInitT, T*, T*, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Nothing to do */
}

template<class T> inline void arrayConstruct(DefaultInitT, T* begin, T* const end, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Needs to be < because sometimes begin > end. No {}, we want trivial
       types non-initialized */
    for(; begin < end; ++begin) new(begin) T;
}

template<class T> inline void arrayConstruct(ValueInitT, T* const begin, T* const end, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    if(begin < end) std::memset(begin, 0, (end - begin)*sizeof(T));
}

template<class T> inline void arrayConstruct(ValueInitT, T* begin, T* const end, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Needs to be < because sometimes begin > end */
    for(; begin < end; ++begin) new(begin) T{};
}

template<class T> inline void arrayMoveConstruct(T* const src, T* const dst, const std::size_t count, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    std::memcpy(dst, src, count*sizeof(T));
}

template<class T> inline void arrayMoveConstruct(T* src, T* dst, const std::size_t count, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    static_assert(std::is_nothrow_move_constructible<T>::value,
        "noexcept move-constructible type is required");
    for(T* end = src + count; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(*dst, std::move(*src));
        #else
        new(dst) T{std::move(*src)};
        #endif
}

template<class T> inline void arrayMoveAssign(T* const src, T* const dst, const std::size_t count, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    std::memcpy(dst, src, count*sizeof(T));
}

template<class T> inline void arrayMoveAssign(T* src, T* dst, const std::size_t count, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    static_assert(std::is_nothrow_move_assignable<T>::value,
        "noexcept move-assignable type is required");
    for(T* end = src + count; src != end; ++src, ++dst)
        *dst = std::move(*src);
}

template<class T> inline void arrayCopyConstruct(const T* const src, T* const dst, const std::size_t count, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    std::memcpy(dst, src, count*sizeof(T));
}

template<class T> inline void arrayCopyConstruct(const T* src, T* dst, const std::size_t count, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_copyable<T>::value
    #else
    IsTriviallyCopyableOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    for(const T* end = src + count; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
        Implementation::construct(*dst, *src);
        #else
        new(dst) T{*src};
        #endif
}

template<class T> inline void arrayDestruct(T*, T*, typename std::enable_if<std::is_trivially_destructible<T>::value>::type* = nullptr) {
    /* Nothing to do */
}

template<class T> inline void arrayDestruct(T* begin, T* const end, typename std::enable_if<!std::is_trivially_destructible<T>::value>::type* = nullptr) {
    /* Needs to be < because sometimes begin > end */
    for(; begin < end; ++begin) begin->~T();
}

inline std::size_t arrayGrowth(const std::size_t currentCapacity, const std::size_t desiredCapacity, const std::size_t sizeOfT) {
    /** @todo pick a nice value when current = 0 and desired > 1 */
    const std::size_t currentCapacityInBytes = sizeOfT*currentCapacity + sizeof(std::size_t);

    constexpr std::size_t MinAllocatedSize =
        #ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
        __STDCPP_DEFAULT_NEW_ALIGNMENT__
        #else
        2*sizeof(std::size_t);
        #endif
        ;

    /* For small allocations we want to tightly fit into size buckets (8, 16,
       32, 64 bytes), so it's better to double the capacity every time. For
       larger, increase just by 50%. The capacity is calculated including the
       space needed to store the capacity value (so e.g. a 16-byte allocation
       can store two ints, but when it's doubled to 32 bytes, it can store
       six of them). */
    std::size_t grown;
    if(currentCapacityInBytes < MinAllocatedSize)
        grown = MinAllocatedSize;
    else if(currentCapacityInBytes < 64)
        grown = currentCapacityInBytes*2;
    else
        grown = currentCapacityInBytes + currentCapacityInBytes/2;

    const std::size_t candidate = (grown - sizeof(std::size_t))/sizeOfT;
    return desiredCapacity > candidate ? desiredCapacity : candidate;
}

}

template<class T> void ArrayNewAllocator<T>::reallocate(T*& array, const std::size_t prevSize, const std::size_t newCapacity) {
    T* newArray = allocate(newCapacity);
    static_assert(std::is_nothrow_move_constructible<T>::value,
        "noexcept move-constructible type is required");
    for(T *src = array, *end = src + prevSize, *dst = newArray; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && __GNUC__ < 5
        Implementation::construct(*dst, std::move(*src));
        #else
        new(dst) T{std::move(*src)};
        #endif
    for(T *it = array, *end = array + prevSize; it < end; ++it) it->~T();
    deallocate(array);
    array = newArray;
}

template<class T> void ArrayMallocAllocator<T>::reallocate(T*& array, std::size_t, const std::size_t newCapacity) {
    const std::size_t inBytes = newCapacity*sizeof(T) + sizeof(std::size_t);
    char* const memory = static_cast<char*>(std::realloc(reinterpret_cast<char*>(array) - sizeof(std::size_t), inBytes));
    reinterpret_cast<std::size_t*>(memory)[0] = inBytes;
    array = reinterpret_cast<T*>(memory + sizeof(std::size_t));
}

template<class T> std::size_t ArrayNewAllocator<T>::grow(T* const array, const std::size_t desiredCapacity) {
    return Implementation::arrayGrowth(array ? capacity(array) : 0, desiredCapacity, sizeof(T));
}

template<class T> std::size_t ArrayMallocAllocator<T>::grow(T* const array, const std::size_t desiredCapacity) {
    return Implementation::arrayGrowth(array ? capacity(array) : 0, desiredCapacity, sizeof(T));
}

template<class T, class Allocator> bool arrayIsGrowable(Array<T>& array) {
    return array.deleter() == Allocator::deleter;
}

template<class T, class Allocator> std::size_t arrayCapacity(Array<T>& array) {
    if(array.deleter() == Allocator::deleter)
        return Allocator::capacity(array.data());
    return array.size();
}

template<class T, class Allocator> std::size_t arrayReserve(Array<T>& array, const std::size_t capacity) {
    /* Direct access & value caching to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    const bool hasGrowingDeleter = arrayGuts.deleter == Allocator::deleter;

    /* If the capacity is large enough, nothing to do (even if we have the
       array allocated by something different) */
    const std::size_t currentCapacity = arrayCapacity(array);
    if(currentCapacity >= capacity) return currentCapacity;

    /* Otherwise allocate a new array, move the previous data there and replace
       the old Array instance with it. Array's deleter will take care of
       destructing & deallocating the previous memory. */
    if(!hasGrowingDeleter) {
        T* newArray = Allocator::allocate(capacity);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, arrayGuts.size);
        array = Array<T>{newArray, arrayGuts.size, Allocator::deleter};
    } else Allocator::reallocate(arrayGuts.data, arrayGuts.size, capacity);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /** @todo with std::realloc, is that really a new allocation? what should I
        do here? */
    __sanitizer_annotate_contiguous_container(
        Allocator::base(arrayGuts.data),
        arrayGuts.data + capacity,
        arrayGuts.data + capacity, /* ASan assumes this for new allocations */
        arrayGuts.data + arrayGuts.size);
    #endif

    return capacity;
}

template<class T, class Allocator> void arrayResize(Array<T>& array, NoInitT, const std::size_t size) {
    /* Direct access & value caching to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    const bool hasGrowingDeleter = arrayGuts.deleter == Allocator::deleter;

    /* New size is the same as the old one, nothing to do */
    if(arrayGuts.size == size) return;

    /* Reallocate if we don't have our growable deleter, as the default deleter
       might then call destructors even in the non-initialized area ... */
    if(!hasGrowingDeleter) {
        T* newArray = Allocator::allocate(size);
        Implementation::arrayMoveConstruct<T>(array, newArray,
            /* Move the min of the two sizes -- if we shrink, move only what
               will fit in the new array; if we extend, move only what's
               initialized in the original and left the rest not initialized */
            arrayGuts.size < size ? arrayGuts.size : size);
        array = Array<T>{newArray, size, Allocator::deleter};

        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        /* This should basically be a no-op, right? */
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size);
        #endif

    /* ... or the desired size is larger than the capacity. In that case make
       use of the reallocate() function that might be able to grow in-place. */
    } else if(Allocator::capacity(array) < size) {
        Allocator::reallocate(arrayGuts.data,
            /* Move the min of the two sizes -- if we shrink, move only what
               will fit in the new array; if we extend, move only what's
               initialized in the original and left the rest not initialized */
            arrayGuts.size < size ? arrayGuts.size : size, size);
        arrayGuts.size = size;

        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        /** @todo with std::realloc, is that really a new allocation? what
            should I do here? */
        /* This should basically be a no-op, right? */
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size);
        #endif

    /* Otherwise call a destructor on the extra elements. If we get here, we
       have our growable deleter and didn't need to reallocate (which would
       make this unnecessary). */
    } else {
        Implementation::arrayDestruct<T>(arrayGuts.data + size, arrayGuts.data + arrayGuts.size);
        /* This is a NoInit resize, so not constructing the new elements, only
           updating the size */
        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + Allocator::capacity(array),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + size);
        #endif
        arrayGuts.size = size;
    }
}

template<class T, class Allocator> void arrayResize(Array<T>& array, DefaultInitT, const std::size_t size) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, NoInit, size);
    Implementation::arrayConstruct(DefaultInit, array + prevSize, array.end());
}

template<class T, class Allocator> void arrayResize(Array<T>& array, ValueInitT, const std::size_t size) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, NoInit, size);
    Implementation::arrayConstruct(ValueInit, array + prevSize, array.end());
}

template<class T, class Allocator, class... Args> void arrayResize(Array<T>& array, DirectInitT, const std::size_t size, Args&&... args) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, NoInit, size);

    /* In-place construct the new elements. No helper function for this as
       there's no way we could memcpy such a thing. */
    for(T* it = array + prevSize; it < array.end(); ++it)
        Implementation::construct(*it, std::forward<Args>(args)...);
}

template<class T, class... Args> inline void arrayResize(Array<T>& array, DirectInitT, const std::size_t size, Args&&... args) {
    arrayResize<T, ArrayAllocator<T>, Args...>(array, DirectInit, size, std::forward<Args>(args)...);
}

namespace Implementation {

template<class T, class Allocator> T* arrayGrowBy(Array<T>& array, const std::size_t count) {
    /* Direct access & caching to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* For arrays with an unknown deleter we'll always copy-allocate to a new
       place */
    const std::size_t desiredCapacity = arrayGuts.size + count;
    std::size_t capacity;
    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    T* oldMid = nullptr;
    #endif
    if(arrayGuts.deleter != Allocator::deleter) {
        capacity = Allocator::grow(nullptr, desiredCapacity);
        T* const newArray = Allocator::allocate(capacity);
        arrayMoveConstruct<T>(arrayGuts.data, newArray, arrayGuts.size);
        array = Array<T>{newArray, arrayGuts.size, Allocator::deleter};

    /* Otherwise, if there's no space anymore, reallocate, which might be able
       to grow in-place */
    } else {
        capacity = Allocator::capacity(arrayGuts.data);
        if(arrayGuts.size + count > capacity) {
            capacity = Allocator::grow(arrayGuts.data, desiredCapacity);
            Allocator::reallocate(arrayGuts.data, arrayGuts.size, capacity);
        } else {
            #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
            oldMid = arrayGuts.data + arrayGuts.size;
            #endif
        }
    }

    /* Increase array size and return the previous end pointer */
    T* const it = arrayGuts.data + arrayGuts.size;
    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    __sanitizer_annotate_contiguous_container(
        Allocator::base(arrayGuts.data),
        arrayGuts.data + capacity,
        /* For a new allocation, ASan assumes the previous middle pointer is at
           the end of the array. If we grew an existing allocation, the
           previous middle is set what __sanitier_acc() received as a middle
           value before */
        /** @todo with std::realloc possibly happening in reallocate(), is that
            really a new allocation? what should I do there? */
        oldMid ? oldMid : arrayGuts.data + capacity,
        arrayGuts.data + arrayGuts.size + count);
    #endif
    arrayGuts.size += count;
    return it;
}

}

template<class T, class Allocator> inline T& arrayAppend(Array<T>& array, const T& value) {
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, 1);
    /* Can't use {}, see the GCC 4.8-specific overload for details */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
    Implementation::construct(*it, value);
    #else
    new(it) T{value};
    #endif
    return *it;
}

template<class T, class Allocator> inline Containers::ArrayView<T> arrayAppend(Array<T>& array, const std::initializer_list<T> values) {
    return arrayAppend(array, {values.begin(), values.size()});
}

template<class T, class Allocator> inline Containers::ArrayView<T> arrayAppend(Array<T>& array, const Containers::ArrayView<const T> values) {
    /* Direct access & caching to speed up debug builds */
    const std::size_t valueCount = values.size();

    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, valueCount);
    Implementation::arrayCopyConstruct<T>(values.data(), it, valueCount);
    return {it, valueCount};
}

template<class T, class Allocator, class... Args> T& arrayAppend(Array<T>& array, InPlaceInitT, Args&&... args) {
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, 1);
    /* No helper function as there's no way we could memcpy such a thing. */
    /* On GCC 4.8 this includes another workaround, see the 4.8-specific
       overload docs for details */
    Implementation::construct(*it, std::forward<Args>(args)...);
    return *it;
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T, class... Args> inline T& arrayAppend(Array<T>& array, InPlaceInitT, Args&&... args) {
    return arrayAppend<T, ArrayAllocator<T>>(array, InPlaceInit, std::forward<Args>(args)...);
}
#endif

template<class T, class Allocator> Containers::ArrayView<T> arrayAppend(Array<T>& array, NoInitT, const std::size_t count) {
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, count);
    return {it, count};
}

template<class T, class Allocator> void arrayRemoveSuffix(Array<T>& array, const std::size_t count) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    CORRADE_ASSERT(count <= arrayGuts.size, "Containers::arrayRemoveSuffix(): can't remove" << count << "elements from an array of size" << arrayGuts.size, );

    /* Nothing to remove, yay! */
    if(!count) return;

    /* If we don't have our own deleter, we need to reallocate in order to
       store the capacity. That'll also cause the excessive elements to be
       properly destructed, so nothing else needs to be done. Not using
       reallocate() as we don't know where the original memory comes from. */
    if(arrayGuts.deleter != Allocator::deleter) {
        T* const newArray = Allocator::allocate(arrayGuts.size - count);
        Implementation::arrayMoveConstruct<T>(array, newArray, arrayGuts.size - count);
        array = Array<T>{newArray, arrayGuts.size - count, Allocator::deleter};

        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        /* This should basically be a no-op, right? */
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size);
        #endif

    /* Otherwise call the destructor on the excessive elements and update the
       size */
    } else {
        T* const end = arrayGuts.data + arrayGuts.size;
        Implementation::arrayDestruct<T>(end - count, end);
        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + Allocator::capacity(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size - count);
        #endif
        arrayGuts.size -= count;
    }
}

template<class T, class Allocator> void arrayShrink(Array<T>& array, NoInitT) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, assume the array size equals its
       capacity and do nothing */
    if(arrayGuts.deleter != Allocator::deleter)
        return;

    /* Even if we don't need to shrink, reallocating to an usual array with
       common deleters to avoid surprises */
    Array<T> newArray{NoInit, arrayGuts.size};
    Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, arrayGuts.size);
    array = std::move(newArray);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /* Nothing to do (not annotating the arrays with default deleter) */
    #endif
}

template<class T, class Allocator> void arrayShrink(Array<T>& array, DefaultInitT) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, assume the array size equals its
       capacity and do nothing */
    if(arrayGuts.deleter != Allocator::deleter)
        return;

    /* Even if we don't need to shrink, reallocating to an usual array with
       common deleters to avoid surprises */
    Array<T> newArray{DefaultInit, arrayGuts.size};
    Implementation::arrayMoveAssign<T>(arrayGuts.data, newArray, arrayGuts.size);
    array = std::move(newArray);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /* Nothing to do (not annotating the arrays with default deleter) */
    #endif
}

}}

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
#undef _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif

#endif
