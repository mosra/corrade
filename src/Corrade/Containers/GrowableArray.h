#ifndef Corrade_Containers_GrowableArray_h
#define Corrade_Containers_GrowableArray_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
 * @brief Class @ref Corrade::Containers::ArrayAllocator, @ref Corrade::Containers::ArrayNewAllocator, @ref Corrade::Containers::ArrayMallocAllocator, function @ref Corrade::Containers::arrayAllocatorCast(), @ref Corrade::Containers::arrayIsGrowable(), @ref Corrade::Containers::arrayCapacity(), @ref Corrade::Containers::arrayReserve(), @ref Corrade::Containers::arrayResize(), @ref Corrade::Containers::arrayAppend(), @ref Corrade::Containers::arrayInsert(), @ref Corrade::Containers::arrayRemove(), @ref Corrade::Containers::arrayRemoveUnordered(), @ref Corrade::Containers::arrayRemoveSuffix(), @ref Corrade::Containers::arrayClear(), @ref Corrade::Containers::arrayShrink()
 * @m_since{2020,06}
 *
 * See @ref Containers-Array-growable for more information.
 */

#include <cstdlib>
#include <cstring>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/initializeHelpers.h"
#include "Corrade/Utility/Math.h"

/* No __has_feature on GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60512
   Using a dedicated macro instead: https://stackoverflow.com/a/34814667 */
#ifndef CORRADE_CONTAINERS_NO_SANITIZER_ANNOTATIONS
#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif
#endif
#ifdef __SANITIZE_ADDRESS__
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif
#endif

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
/* https://github.com/llvm/llvm-project/blob/main/compiler-rt/include/sanitizer/common_interface_defs.h */
extern "C" void __sanitizer_annotate_contiguous_container(const void *beg,
    const void *end, const void *old_mid, const void *new_mid)
        /* Declaration of this function in <vector> in MSVC 2022 14.35 and
           earlier STL includes a noexcept for some strange unexplained reason,
           which makes the signature differ from Clang's. See the PR comment
           here:
            https://github.com/microsoft/STL/pull/2071/commits/daa4db9bf10400678438d9c6b33630c7947e469e
           It got then subsequently removed, but without any additional
           explanation in the commit message nor links to corresponding bug
           reports and the STL repo has no tags or mapping to actual releases:
            https://github.com/microsoft/STL/pull/3164/commits/9f503ca22bcc32cd885184ea754ec4223759c431
           So I only accidentally discovered that this commit is in 14.36:
            https://developercommunity.visualstudio.com/t/__sanitizer_annotate_contiguous_containe/10119696
           The difference in noexcept is only a problem with `/std:c++17`
           (where noexcept becomes a part of the function signature) *and* with
           the `/permissive-` flag set, or `/std:c++20` alone (where the flag
           is implicitly enabled). */
        #if defined(CORRADE_TARGET_DINKUMWARE) && _MSC_VER < 1936
        noexcept
        #endif
    ;
#endif

namespace Corrade { namespace Containers {

namespace Implementation {

template<class T> struct AllocatorTraits {
    /** @todo assert that this is not higher than platform default allocation
        alignment once we have an alternative allocator */
    enum: std::size_t {
        Offset = alignof(T) < sizeof(std::size_t) ? sizeof(std::size_t) :
            (alignof(T) < Implementation::DefaultAllocationAlignment ?
                alignof(T) : Implementation::DefaultAllocationAlignment)
    };
};

}

/** @{ @name Growable array utilities
 *
 * See @ref Containers-Array-growable for more information.
 */

/**
@brief New-based allocator for growable arrays
@m_since{2020,06}

An @ref ArrayAllocator that allocates and deallocates memory using the C++
@cpp new[] @ce / @cpp delete[] @ce constructs, reserving an extra space
* *before* to store array capacity.

All reallocation operations expect that @p T is nothrow move-constructible.
@see @ref Containers-Array-growable
*/
template<class T> struct ArrayNewAllocator {
    typedef T Type; /**< Pointer type */

    enum: std::size_t {
        /** @copydoc ArrayMallocAllocator::AllocationOffset */
        AllocationOffset = Implementation::AllocatorTraits<T>::Offset
    };

    /**
     * @brief Allocate (but not construct) an array of given capacity
     *
     * @cpp new[] @ce-allocates a @cpp char @ce array with an extra space to
     * store @p capacity *before* the front, returning it cast to @cpp T* @ce.
     * The allocation is guaranteed to follow `T` allocation requirements up to
     * the platform default allocation alignment.
     */
    static T* allocate(std::size_t capacity) {
        char* const memory = new char[capacity*sizeof(T) + AllocationOffset];
        reinterpret_cast<std::size_t*>(memory)[0] = capacity;
        return reinterpret_cast<T*>(memory + AllocationOffset);
    }

    /**
     * @brief Reallocate an array to given capacity
     *
     * Calls @p allocate(), move-constructs @p prevSize elements from @p array
     * into the new array, calls destructors on the original elements, calls
     * @ref deallocate() and updates the @p array reference to point to the new
     * array. The allocation is guaranteed to follow `T` allocation
     * requirements up to the platform default allocation alignment.
     */
    static void reallocate(T*& array, std::size_t prevSize, std::size_t newCapacity);

    /**
     * @brief Deallocate an array
     *
     * Calls @cpp delete[] @ce on a pointer offset by the extra space needed to
     * store its capacity.
     */
    static void deallocate(T* data) {
        delete[] (reinterpret_cast<char*>(data) - AllocationOffset);
    }

    /**
     * @brief Grow an array
     *
     * If current occupied size (including the space needed to store capacity)
     * is less than 64 bytes, the capacity always doubled, with the allocation
     * being at least as large as @cpp __STDCPP_DEFAULT_NEW_ALIGNMENT__ @ce
     * (*usually* @cpp 2*sizeof(std::size_t) @ce). After that, the capacity is
     * increased to 1.5x of current capacity (again including the space needed
     * to store capacity). This is similar to what MSVC STL does with
     * @ref std::vector, except for libc++ / libstdc++, which both use a factor
     * of 2. With a factor of 2 the allocation would crawl forward in memory,
     * never able to reuse the holes after previous allocations, with a factor
     * 1.5 it's possible after four reallocations. Further info in
     * [Folly FBVector docs](https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md#memory-handling).
     */
    static std::size_t grow(T* array, std::size_t desired);

    /**
     * @brief Array capacity
     *
     * Retrieves the capacity that's stored *before* the front of the @p array.
     */
    static std::size_t capacity(T* array) {
        return *reinterpret_cast<std::size_t*>(reinterpret_cast<char*>(array) - AllocationOffset);
    }

    /**
     * @brief Array base address
     *
     * Returns the address with @ref AllocationOffset subtracted.
     */
    static void* base(T* array) {
        return reinterpret_cast<char*>(array) - AllocationOffset;
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

/**
@brief Malloc-based allocator for growable arrays
@m_since{2020,06}

An @ref ArrayAllocator that allocates and deallocates memory using the C
@ref std::malloc() / @ref std::free() constructs in order to be able to use
@ref std::realloc() for fast reallocations. Similarly to @ref ArrayNewAllocator
it's reserving an extra space *before* to store array capacity.

All reallocation operations expect that @p T is trivially copyable. If it's
not, use @ref ArrayNewAllocator instead.

Compared to @ref ArrayNewAllocator, this allocator stores array capacity in
bytes and, together with the fact that @ref std::free() doesn't care about the
actual array type, growable arrays using this allocator can be freely cast to
different compatible types using @ref arrayAllocatorCast().
@see @ref Containers-Array-growable
*/
template<class T> struct ArrayMallocAllocator {
    static_assert(
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T)
        #else
        std::is_trivially_copyable<T>::value
        #endif
        , "only trivially copyable types are usable with this allocator");

    typedef T Type; /**< Pointer type */

    enum: std::size_t {
        /**
         * Offset at the beginning of the allocation to store allocation
         * capacity. At least as large as @ref std::size_t. If the type
         * alignment is larger than that (for example @cpp double @ce on a
         * 32-bit platform), then it's equal to type alignment, but only at
         * most as large as the default allocation alignment.
         */
        AllocationOffset = Implementation::AllocatorTraits<T>::Offset
    };

    /**
     * @brief Allocate an array of given capacity
     *
     * @ref std::malloc()'s an @cpp char @ce array with an extra space to store
     * @p capacity *before* the front, returning it cast to @cpp T* @ce. The
     * allocation is guaranteed to follow `T` allocation requirements up to the
     * platform default allocation alignment.
     */
    static T* allocate(std::size_t capacity) {
        /* Compared to ArrayNewAllocator, here the capacity is stored in bytes
           so it's possible to "reinterpret" the array into a different type
           (as the deleter is a typeless std::free() in any case) */
        const std::size_t inBytes = capacity*sizeof(T) + AllocationOffset;
        char* const memory = static_cast<char*>(std::malloc(inBytes));
        CORRADE_ASSERT(memory,
            "Containers::ArrayMallocAllocator: can't allocate" << inBytes << "bytes", {});
        reinterpret_cast<std::size_t*>(memory)[0] = inBytes;
        return reinterpret_cast<T*>(memory + AllocationOffset);
    }

    /**
     * @brief Reallocate an array to given capacity
     *
     * Calls @ref std::realloc() on @p array (offset by the space to store
     * capacity) and then updates the stored capacity to @p newCapacity and the
     * @p array reference to point to the new (offset) location, in case the
     * reallocation wasn't done in-place. The @p prevSize parameter is ignored,
     * as @ref std::realloc() always copies the whole original capacity. The
     * allocation is guaranteed to follow `T` allocation requirements up to the
     * platform default allocation alignment.
     */
    static void reallocate(T*& array, std::size_t prevSize, std::size_t newCapacity);

    /**
     * @brief Deallocate an array
     *
     * Calls @ref std::free() on a pointer offset by the extra space needed to
     * store its capacity.
     */
    static void deallocate(T* data) {
        if(data) std::free(reinterpret_cast<char*>(data) - AllocationOffset);
    }

    /**
     * @brief Grow an array
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
        return (*reinterpret_cast<std::size_t*>(reinterpret_cast<char*>(array) - AllocationOffset) - AllocationOffset)/sizeof(T);
    }

    /**
     * @brief Array base address
     *
     * Returns the address with @ref AllocationOffset subtracted.
     */
    static void* base(T* array) {
        return reinterpret_cast<char*>(array) - AllocationOffset;
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
@ref ArrayNewAllocator otherwise, in which case reallocation operations expect
@p T to be nothrow move-constructible.

See @ref Containers-Array-growable for an introduction to growable arrays.

You can provide your own allocator by implementing a class that with @ref Type,
@ref allocate(), @ref reallocate(), @ref deallocate(), @ref grow(),
@ref capacity(), @ref base() and @ref deleter() following the documented
semantics.
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
     * destructors on their original location, deallocate the @p array and
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
     * @brief Grow an array
     *
     * Assumes that @p array is either @cpp nullptr @ce or was returned earlier
     * by @ref allocate() or @ref reallocate(). Implementations are expected to
     * return a new capacity with an an optimal tradeoff between reallocation
     * count and memory usage, the value then being passed to
     * @ref reallocate().
     *
     * See documentation of a particular implementation (such as
     * @ref ArrayNewAllocator::grow()) for details about growth strategy.
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
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
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

Equivalently to @ref arrayCast(), the size of the new array is calculated as
@cpp view.size()*sizeof(T)/sizeof(U) @ce. Expects that both types are
trivially copyable and [standard layout](https://en.cppreference.com/w/cpp/named_req/StandardLayoutType.html)
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
        #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
        __has_trivial_copy(T) && __has_trivial_destructor(T) && __has_trivial_copy(U) && __has_trivial_destructor(U)
        #else
        std::is_trivially_copyable<T>::value && std::is_trivially_copyable<U>::value
        #endif
        , "only trivially copyable types can use the allocator cast");

    /* If the array is default-constructed or just generally empty with the
       default deleter, just pass it through without changing anything. This
       behavior is consistent with calling `arrayResize(array, 0)`,
       `arrayReserve(array, 0)` and such, which also just pass empty arrays
       through without affecting their deleter. */
    if(array.isEmpty() && !array.data() && !array.deleter())
        return {};

    /* Unlike arrayInsert() etc, this is not called that often and should be as
       checked as possible, so it's not a debug assert */
    CORRADE_ASSERT(array.deleter() == Allocator<T>::deleter && (std::is_base_of<ArrayMallocAllocator<T>, Allocator<T>>::value),
        "Containers::arrayAllocatorCast(): the array has to use the ArrayMallocAllocator or a derivative", {});
    const std::size_t size = array.size()*sizeof(T)/sizeof(U);
    CORRADE_ASSERT(size*sizeof(U) == array.size()*sizeof(T),
        "Containers::arrayAllocatorCast(): can't reinterpret" << array.size() << sizeof(T) << Utility::Debug::nospace << "-byte items into a" << sizeof(U) << Utility::Debug::nospace << "-byte type", {});
    return Array<U>{reinterpret_cast<U*>(array.release()), size, Allocator<U>::deleter};
}

template<class U, class T> Array<U> arrayAllocatorCast(Array<T>&& array) {
    return arrayAllocatorCast<U, ArrayAllocator, T>(Utility::move(array));
}

/**
@brief Whether an array is growable
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

For a growable array using given @p Allocator returns its capacity, otherwise
returns @ref Array::size().

This function is equivalent to calling @relativeref{std::vector,capacity()} on
a @ref std::vector.
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
if the reallocation can be done in-place. On top of what the @p Allocator (or
the default @ref ArrayAllocator) itself needs, @p T is required to be nothrow
move-constructible and move-assignable.

This function is equivalent to calling @relativeref{std::vector,reserve()} on
a @ref std::vector.
@m_keywords{reserve()}
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

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Resize an array to given size, default-initializing new elements
@m_deprecated_since_latest Because C++'s default initialization keeps trivial
    types not initialized, using it is unnecessarily error prone. Use either
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t) or
    @ref arrayResize(Array<T>&, NoInitT, std::size_t) instead to make the
    choice about content initialization explicit.

If the array is growable and capacity is large enough, calls a destructor on
elements that get cut off the end (if any, and if @p T is not trivially
destructible, in which case nothing is done) and returns. Otherwise, the memory
is reallocated to desired @p size. After that, new elements at the end of the
array are default-initialized using placement-new (and nothing done for trivial
types). Note that in case the array is non-growable of exactly the requested
size, it's kept as such, without being reallocated to a growable version.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the new container,
@f$ \mathcal{O}(1) @f$ if current container size is already exactly of given
size. On top of what the @p Allocator (or the default @ref ArrayAllocator)
itself needs, @p T is required to be nothrow move-constructible and
default-constructible.
@see @ref Array::size(), @ref arrayCapacity(), @ref arrayIsGrowable(),
    @ref arrayRemoveSuffix(), @ref arrayResize(Array<T>&, std::size_t),
    @ref arrayResize(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref arrayResize(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> CORRADE_DEPRECATED("use arrayResize(Array<T>, ValueInitT, std::size_t) or arrayResize(Array<T>, NoInitT, std::size_t) instead") void arrayResize(Array<T>& array, Corrade::DefaultInitT, std::size_t size);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_deprecated_since_latest Because C++'s default initialization keeps trivial
    types not initialized, using it is unnecessarily error prone. Use either
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t) or
    @ref arrayResize(Array<T>&, NoInitT, std::size_t) instead to make the
    choice about content initialization explicit.

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline CORRADE_DEPRECATED("use arrayResize(Array<T>, ValueInitT, std::size_t) or arrayResize(Array<T>, NoInitT, std::size_t) instead") void arrayResize(Array<T>& array, Corrade::DefaultInitT, std::size_t size) {
    CORRADE_IGNORE_DEPRECATED_PUSH
    arrayResize<T, Allocator<T>>(array, Corrade::DefaultInit, size);
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif
#endif

/**
@brief Resize an array to given size, value-initializing new elements
@m_since{2020,06}

If the array is growable and capacity is large enough, calls a destructor on
elements that get cut off the end (if any, and if @p T is not trivially
destructible, in which case nothing is done) and returns. Otherwise, the memory
is reallocated to desired @p size. After that, new elements at the end of the
array are value-initialized (i.e., zero-initialized for trivial types and using
placement new otherwise). Note that in case the array is non-growable of
exactly the requested size, it's kept as such, without being reallocated to a
growable version.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the new container,
@f$ \mathcal{O}(1) @f$ if current container size is already exactly of given
size. On top of what the @p Allocator (or the default @ref ArrayAllocator)
itself needs, @p T is required to be nothrow move-constructible and
default-constructible.
@see @ref Array::size(), @ref arrayCapacity(), @ref arrayIsGrowable(),
    @ref arrayRemoveSuffix(), @ref arrayResize(Array<T>&, std::size_t),
    @ref arrayResize(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref arrayResize(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayResize(Array<T>& array, Corrade::ValueInitT, std::size_t size);

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
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, Corrade::ValueInitT, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, Corrade::ValueInit, size);
}
#endif

/**
@brief Resize an array to given size, value-initializing new elements
@m_since{2020,06}

Alias to @ref arrayResize(Array<T>&, ValueInitT, std::size_t).

This function is equivalent to calling @relativeref{std::vector,resize()} on
a @ref std::vector.
@m_keywords{resize()}
@see @ref arrayResize(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref arrayResize(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> inline void arrayResize(Array<T>& array, std::size_t size) {
    return arrayResize<T, Allocator>(array, Corrade::ValueInit, size);
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

Similar to @ref arrayResize(Array<T>&, ValueInitT, std::size_t) except that the
new elements at the end are not value-initialized, but left in an uninitialized
state instead. I.e., placement-new is meant to be used on *all* newly added
elements with a non-trivially-copyable @p T.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible.
@see @ref arrayResize(Array<T>&, std::size_t),
    @ref arrayResize(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t),
    @ref arrayResize(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayResize(Array<T>& array, Corrade::NoInitT, std::size_t size);

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
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, Corrade::NoInitT, std::size_t size) {
    arrayResize<T, Allocator<T>>(array, Corrade::NoInit, size);
}
#endif

/**
@brief Resize an array to given size, constructing new elements using provided arguments
@m_since{2020,06}

Similar to @ref arrayResize(Array<T>&, ValueInitT, std::size_t) except that
the new elements at the end are constructed using placement-new with provided
@p args.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible and constructible from
provided @p args.
@see @ref arrayResize(Array<T>&, std::size_t),
    @ref arrayResize(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&.... args),
    @ref Containers-Array-growable
*/
template<class T, class ...Args> void arrayResize(Array<T>& array, Corrade::DirectInitT, std::size_t size, Args&&... args);

/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator, class ...Args> void arrayResize(Array<T>& array, Corrade::DirectInitT, std::size_t size, Args&&... args);

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
template<template<class> class Allocator, class T, class ...Args> inline void arrayResize(Array<T>& array, Corrade::DirectInitT, std::size_t size, Args&&... args) {
    arrayResize<T, Allocator<T>>(array, Corrade::DirectInit, size, Utility::forward<Args>(args)...);
}
#endif

/**
@brief Resize an array to given size, copy-constructing new elements using the provided value
@m_since_latest

Calls @ref arrayResize(Array<T>&, DirectInitT, std::size_t, Args&&... args)
with @p value.

This function is equivalent to calling @relativeref{std::vector,resize()} on
a @ref std::vector.
@m_keywords{resize()}
@see @ref arrayResize(Array<T>&, std::size_t),
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> inline void arrayResize(Array<T>& array, std::size_t size, const typename std::common_type<T>::type& value) {
    arrayResize<T, Allocator>(array, Corrade::DirectInit, size, value);
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayResize(Array<T>& array, std::size_t size, const typename std::common_type<T>::type& value) {
    arrayResize<T, Allocator<T>>(array, size, value);
}
#endif

/**
@brief Copy-append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

If the array is not growable or the capacity is not large enough, the array
capacity is grown first according to rules described in the
@ref ArrayAllocator::grow() "grow()" function of a particular allocator. Then,
@p value is copy-constructed at the end of the array and @ref Array::size()
increased by 1.

Amortized complexity is @f$ \mathcal{O}(1) @f$ providing the allocator growth
ratio is exponential. On top of what the @p Allocator (or the default
@ref ArrayAllocator) itself needs, @p T is required to be nothrow
move-constructible and copy-constructible.

@m_class{m-note m-warning}

@par
    To have the append operation as performant as possible, the @p value
    reference is expected to *not* point inside @p array. If you need to append
    values from within the array itself, use the list-taking
    @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type)
    overload, which handles this case.

This function is equivalent to calling @relativeref{std::vector,push_back()} on
a @ref std::vector.
@m_keywords{push_back()}
@see @ref arrayCapacity(), @ref arrayIsGrowable(),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> T& arrayAppend(Array<T>& array, const typename std::common_type<T>::type& value);

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
template<template<class> class Allocator, class T> inline T& arrayAppend(Array<T>& array, const typename std::common_type<T>::type& value) {
    return arrayAppend<T, Allocator<T>>(array, value);
}
#endif

/**
@brief In-place append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

Similar to @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&)
except that the new element is constructed using placement-new with provided
@p args.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible and constructible from
provided @p args.

@m_class{m-note m-warning}

@par
    The behavior is undefined if any @p args are pointing inside the @p array
    items or their internals as the implementation has no way to check for such
    scenario. If you want to have robust checks against such cases, use the
    @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&)
    overloads which perform a copy or move instead of an in-place construction,
    or the list-taking @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type)
    which detects and appropriately adjusts the view in case it's a
    slice of the @p array itself.

This function is equivalent to calling @relativeref{std::vector,emplace_back()}
on a @ref std::vector.
@m_keywords{emplace_back()}
@see @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class ...Args> T& arrayAppend(Array<T>& array, Corrade::InPlaceInitT, Args&&... args);

/* This crap tool can't distinguish between these and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator, class ...Args> T& arrayAppend(Array<T>& array, Corrade::InPlaceInitT, Args&&... args);

/**
@overload
@m_since{2020,06}

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class ...Args> inline T& arrayAppend(Array<T>& array, Corrade::InPlaceInitT, Args&&... args) {
    return arrayAppend<T, Allocator<T>>(array, Corrade::InPlaceInit, Utility::forward<Args>(args)...);
}
#endif

/**
@brief Move-append an item to an array
@return Reference to the newly appended item
@m_since{2020,06}

Calls @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args) with @p value.

@m_class{m-note m-warning}

@par
    To have the append operation as performant as possible, the @p value
    reference is expected to *not* point inside @p array. If you need to
    move-append values from within the array itself, move them to a temporary
    location first.

@see @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> inline T& arrayAppend(Array<T>& array, typename std::common_type<T>::type&& value) {
    CORRADE_DEBUG_ASSERT(std::size_t(&value - array.data()) >= (arrayCapacity<T, Allocator>(array)),
        "Containers::arrayAppend(): use the list variant to append values from within the array itself", *array.data());
    return arrayAppend<T, Allocator>(array, Corrade::InPlaceInit, Utility::move(value));
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
template<template<class> class Allocator, class T> inline T& arrayAppend(Array<T>& array, typename std::common_type<T>::type&& value) {
    return arrayAppend<T, Allocator<T>>(array, Corrade::InPlaceInit, Utility::move(value));
}
#endif

/**
@brief Copy-append a list of items to an array
@return View on the newly appended items
@m_since{2020,06}

Like @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
but inserting multiple values at once.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible and
copy-constructible.

Compared to the single-value @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
this function also handles the case where @p values are a slice of the @p array
itself. In particular, if the @p array needs to be reallocated in order to fit
the new items, the @p values to append are then copied from the new location.
@see @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayAppend(Array<T>& array, typename std::common_type<ArrayView<const T>>::type values);

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
template<template<class> class Allocator, class T> inline ArrayView<T> arrayAppend(Array<T>& array, typename std::common_type<ArrayView<const T>>::type values) {
    return arrayAppend<T, Allocator<T>>(array, values);
}
#endif

/**
@overload
@m_since{2020,06}
*/
template<class T, class Allocator = ArrayAllocator<T>> inline ArrayView<T>  arrayAppend(Array<T>& array, std::initializer_list<typename std::common_type<T>::type> values) {
    return arrayAppend<T, Allocator>(array, arrayView(values));
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
template<template<class> class Allocator, class T> inline ArrayView<T>  arrayAppend(Array<T>& array, std::initializer_list<typename std::common_type<T>::type> values) {
    return arrayAppend<T, Allocator<T>>(array, values);
}
#endif

/**
@brief Append given count of value-initialized values to an array
@return View on the newly appended items
@m_since_latest

A variant of @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type)
where the new values are value-initialized (i.e., trivial types
zero-initialized and default constructor called otherwise), instead of being
copied from a pre-existing location.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible and
default-constructible.
@see @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayResize(Array<T>&, ValueInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayAppend(Array<T>& array, Corrade::ValueInitT, std::size_t count);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline ArrayView<T> arrayAppend(Array<T>& array, Corrade::ValueInitT, std::size_t count) {
    return arrayAppend<T, Allocator<T>>(array, Corrade::ValueInit, count);
}
#endif

/**
@brief Append given count of uninitialized values to an array
@return View on the newly appended items
@m_since{2020,06}

A variant of @ref arrayAppend(Array<T>&, ValueInitT, std::size_t) where the new
values are left uninitialized --- i.e., placement-new is meant to be used on
* *all* appended elements with a non-trivially-copyable @p T.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible.
@see @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayResize(Array<T>&, NoInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayAppend(Array<T>& array, Corrade::NoInitT, std::size_t count);

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
template<template<class> class Allocator, class T> inline ArrayView<T> arrayAppend(Array<T>& array, Corrade::NoInitT, std::size_t count) {
    return arrayAppend<T, Allocator<T>>(array, Corrade::NoInit, count);
}
#endif

/**
@brief Append given count of values to an array, constructing each using provided arguments
@return View on the newly appended items
@m_since_latest

Similar to @ref arrayAppend(Array<T>&, ValueInitT, std::size_t) except that
the elements are constructed using placement-new with provided @p args.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible and constructible from
provided @p args.
@see @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class ...Args> ArrayView<T> arrayAppend(Array<T>& array, Corrade::DirectInitT, std::size_t count, Args&&... args);

/**
@overload
@m_since_latest
*/
template<class T, class Allocator, class ...Args> ArrayView<T> arrayAppend(Array<T>& array, Corrade::DirectInitT, std::size_t count, Args&&... args);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class ...Args> inline ArrayView<T> arrayAppend(Array<T>& array, Corrade::DirectInitT, std::size_t count, Args&&... args) {
    return arrayAppend<T, Allocator<T>>(array, Corrade::DirectInit, count, Utility::forward<Args>(args)...);
}
#endif

/**
@brief Copy-insert an item into an array
@return Reference to the newly inserted item
@m_since_latest

Expects that @p index is not larger than @ref Array::size(). If the array is
not growable or the capacity is not large enough, the array capacity is grown
first according to rules described in the
@ref ArrayAllocator::grow() "grow()" function of a particular allocator. Then,
items starting at @p index are moved one item forward, @p value is copied to
@p index and @ref Array::size() is increased by 1.

Amortized complexity is @f$ \mathcal{O}(n) @f$. On top of what the @p Allocator
(or the default @ref ArrayAllocator) itself needs, @p T is required to be
nothrow move-constructible, nothrow move-assignable and copy-constructible.

@m_class{m-note m-warning}

@par
    To have the insert operation as performant as possible, the @p value
    reference is expected to *not* point inside @p array. If you need to insert
    values from within the array itself, use the list-taking
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type)
    overload, which handles this case.

This function is equivalent to calling @relativeref{std::vector,insert()} on
a @ref std::vector.
@m_keywords{insert()}
@see @ref arrayCapacity(), @ref arrayIsGrowable(),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, const typename std::common_type<T>::type&),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> T& arrayInsert(Array<T>& array, std::size_t index, const typename std::common_type<T>::type& value);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> T& arrayInsert(Array<T>& array, std::size_t index, const typename std::common_type<T>::type& value) {
    return arrayInsert<T, Allocator<T>>(array, index, value);
}
#endif

/**
@brief In-place insert an item into an array
@return Reference to the newly inserted item
@m_since_latest

Similar to @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&)
except that the new element is constructed using placement-new with provided
@p args.

On top of what the @p Allocator (or the default @ref ArrayAllocator) itself
needs, @p T is required to be nothrow move-constructible, nothrow
move-assignable and constructible from provided @p args.

@m_class{m-note m-warning}

@par
    The behavior is undefined if any @p args are pointing inside the @p array
    items or their internals as the implementation has no way to check for such
    scenario. If you want to have robust checks against such cases, use the
    @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&)
    overloads which perform a copy or move instead of an in-place construction,
    or the list-taking @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type)
    which detects and appropriately adjusts the view in case it's a slice of
    the @p array itself.

This function is equivalent to calling @relativeref{std::vector,emplace()}
on a @ref std::vector.
@m_keywords{emplace()}
@see @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, InPlaceInitT, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class ...Args> T& arrayInsert(Array<T>& array, std::size_t index, Corrade::InPlaceInitT, Args&&... args);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest
*/
template<class T, class Allocator, class ...Args> T& arrayInsert(Array<T>& array, std::size_t index, Corrade::InPlaceInitT, Args&&... args);

/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class ...Args> T& arrayInsert(Array<T>& array, std::size_t index, Corrade::InPlaceInitT, Args&&... args) {
    return arrayInsert<T, Allocator<T>>(array, index, Utility::forward<Args>(args)...);
}
#endif

/**
@brief Move-insert an item into an array
@return Reference to the newly appended item
@m_since_latest

Calls @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args)
with @p value.

@m_class{m-note m-warning}

@par
    To have the insert operation as performant as possible, the @p value
    reference is expected to *not* point inside @p array. If you need to
    move-insert values from within the array itself, move them to a temporary
    location first.

@see @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, typename std::common_type<T>::type&&),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> inline T& arrayInsert(Array<T>& array, std::size_t index, typename std::common_type<T>::type&& value) {
    CORRADE_DEBUG_ASSERT(std::size_t(&value - array.data()) >= (arrayCapacity<T, Allocator>(array)),
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself", *array.data());
    return arrayInsert<T, Allocator>(array, index, Corrade::InPlaceInit, Utility::move(value));
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline T& arrayInsert(Array<T>& array, std::size_t index, typename std::common_type<T>::type&& value) {
    return arrayInsert<T, Allocator<T>>(array, index, Corrade::InPlaceInit, Utility::move(value));
}
#endif

/**
@brief Copy-insert a list of items into an array
@return View on the newly appended items
@m_since_latest

Like @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
but inserting multiple values at once.

Amortized complexity is @f$ \mathcal{O}(m + n) @f$, where @f$ m @f$ is the
number of items being inserted and @f$ n @f$ is the existing array size. On top
of what the @p Allocator (or the default @ref ArrayAllocator) itself needs,
@p T is required to be nothrow move-constructible, nothrow move-assignable and
copy-constructible.

Compared to the single-value @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
this function also handles the case where @p values are a slice of the @p array
itself. In particular, if the @p array needs to be reallocated in order to fit
the new items, the @p values to insert are then copied from the new location.
It's however expected that the slice and @p index don't overlap --- in that
case the caller has to handle that on its own, such as by splitting the
insertion in two.
@see @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, typename std::common_type<ArrayView<const T>>::type),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, typename std::common_type<ArrayView<const T>>::type values);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, typename std::common_type<ArrayView<const T>>::type values) {
    return arrayInsert<T, Allocator<T>>(array, index, values);
}
#endif

/**
@overload
@m_since_latest
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T>  arrayInsert(Array<T>& array, std::size_t index, std::initializer_list<typename std::common_type<T>::type> values) {
    return arrayInsert<T, Allocator>(array, index, arrayView(values));
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline ArrayView<T>  arrayInsert(Array<T>& array, std::size_t index, std::initializer_list<typename std::common_type<T>::type> values) {
    return arrayInsert<T, Allocator<T>>(array, index, values);
}
#endif

/**
@brief Insert given count of value-initialized values into an array
@return View on the newly inserted items
@m_since_latest

A variant of @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<ArrayView<const T>>::type)
where the new values are value-initialized (i.e., trivial types
zero-initialized and default constructor called otherwise), instead of being
copied from a pre-existing location.

Amortized complexity is @f$ \mathcal{O}(m + n) @f$, where @f$ m @f$ is the
number of items being inserted and @f$ n @f$ is the existing array size. On top
of what the @p Allocator (or the default @ref ArrayAllocator) itself needs,
@p T is required to be nothrow move-constructible, nothrow move-assignable and
default-constructible.
@see @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, ValueInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::ValueInitT, std::size_t count);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::ValueInitT, std::size_t count) {
    return arrayInsert<T, Allocator<T>>(array, index, Corrade::ValueInit, count);
}
#endif

/**
@brief Insert given count of uninitialized values into an array
@return View on the newly inserted items
@m_since_latest

A variant of @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t)
where the new values are left uninitialized. Independently of whether the array
was reallocated to fit the new items or the items were just shifted around
because the capacity was large enough, the new values are always uninitialized
--- i.e., placement-new is meant to be used on *all* inserted elements with a
non-trivially-copyable @p T.

Amortized complexity is @f$ \mathcal{O}(n) @f$, where @f$ n @f$ is the existing
array size. On top of what the @p Allocator (or the default @ref ArrayAllocator)
itself needs, @p T is required to be nothrow move-constructible and nothrow
move-assignable.
@see @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, DirectInitT, std::size_t, Args&&... args),
    @ref arrayAppend(Array<T>&, NoInitT, std::size_t),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::NoInitT, std::size_t count);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::NoInitT, std::size_t count) {
    return arrayInsert<T, Allocator<T>>(array, index, Corrade::NoInit, count);
}
#endif

/**
@brief Insert given count of values into an array, constructing each using provided arguments
@return View on the newly inserted items
@m_since_latest

Similar to @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t)
except that the elements are constructed using placement-new with provided
@p args.

On top of what the @p Allocator (or the default @ref ArrayAllocator)
itself needs, @p T is required to be nothrow move-constructible, nothrow
move-assignable and constructible from provided @p args.
@see @ref arrayInsert(Array<T>&, std::size_t, const typename std::common_type<T>::type&),
    @ref arrayInsert(Array<T>&, std::size_t, typename std::common_type<T>::type&&),
    @ref arrayInsert(Array<T>&, std::size_t, InPlaceInitT, Args&&... args),
    @ref arrayInsert(Array<T>&, std::size_t, ValueInitT, std::size_t),
    @ref arrayInsert(Array<T>&, std::size_t, NoInitT, std::size_t),
    @ref arrayAppend(Array<T>&, DirectInitT, std::size_t, Args&&... args),
    @ref Containers-Array-growable
*/
template<class T, class ...Args> ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::DirectInitT, std::size_t count, Args&&... args);

/**
@overload
@m_since_latest
*/
template<class T, class Allocator, class ...Args> ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::DirectInitT, std::size_t count, Args&&... args);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T, class ...Args> inline ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, Corrade::DirectInitT, std::size_t count, Args&&... args) {
    return arrayInsert<T, Allocator<T>>(array, index, Corrade::DirectInit, count, Utility::forward<Args>(args)...);
}
#endif

/**
@brief Remove an element from an array
@m_since_latest

Expects that @cpp index + count @ce is not larger than @ref Array::size(). If
the array is not growable, all elements except the removed ones are reallocated
to a growable version. Otherwise, items starting at @cpp index + count @ce are
moved @cpp count @ce items backward and the @ref Array::size() is decreased by
@p count.

Amortized complexity is @f$ \mathcal{O}(m + n) @f$ where @f$ m @f$ is the
number of items being removed and @f$ n @f$ is the array size after removal. On
top of what the @p Allocator (or the default @ref ArrayAllocator) itself needs,
@p T is required to be nothrow move-constructible and nothrow move-assignable.

This function is equivalent to calling @relativeref{std::vector,erase()} on a
@ref std::vector.
@m_keywords{erase()}
@see @ref arrayIsGrowable(), @ref arrayRemoveUnordered(),
    @ref arrayRemoveSuffix(), @ref arrayClear()
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayRemove(Array<T>& array, std::size_t index, std::size_t count = 1);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayRemove(Array<T>& array, std::size_t index, std::size_t count = 1) {
    arrayRemove<T, Allocator<T>>(array, index, count);
}
#endif

/**
@brief Remove an element from an unordered array
@m_since_latest

A variant of @ref arrayRemove() that is more efficient in case the order of
items in the array doesn't have to be preserved. Expects that
@cpp index + count @ce is not larger than @ref Array::size(). If the array is
not growable, all elements except the removed ones are reallocated to a
growable version. Otherwise, the last @cpp min(count, array.size() - index - count) @ce
items are moved over the items at @p index and the @ref Array::size() is
decreased by @p count.

Amortized complexity is @f$ \mathcal{O}(m) @f$ where @f$ m @f$ is the number of
items being removed. On top of what the @p Allocator (or the default
@ref ArrayAllocator) itself needs, @p T is required to be nothrow
move-constructible and nothrow move-assignable.
@see @ref arrayIsGrowable(), @ref arrayRemoveSuffix(), @ref arrayClear()
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayRemoveUnordered(Array<T>& array, std::size_t index, std::size_t count = 1);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayRemoveUnordered(Array<T>& array, std::size_t index, std::size_t count = 1) {
    arrayRemoveUnordered<T, Allocator<T>>(array, index, count);
}
#endif

/**
@brief Remove a suffix from an array
@m_since{2020,06}

Expects that @p count is not larger than @ref Array::size(). If the array is
not growable, all its elements except the removed suffix are reallocated to a
growable version. Otherwise, a destructor is called on removed elements and the
@ref Array::size() is decreased by @p count.

Amortized complexity is @f$ \mathcal{O}(m) @f$ where @f$ m @f$ is the number of
items removed. On top of what the @p Allocator (or the default
@ref ArrayAllocator) itself needs, @p T is required to be nothrow
move-constructible.

With @p count set to @cpp 1 @ce, this function is equivalent to calling
@relativeref{std::vector,pop_back()} on a @ref std::vector.
@m_keywords{pop_back()}
@see @ref arrayIsGrowable(), @ref arrayRemove(), @ref arrayRemoveUnordered(),
    @ref arrayClear(), @ref arrayResize()
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
@brief Clear an array
@m_since_latest

If the array is not growable, it's replaced by an empty instance, freeing its
contents as a whole. Otherwise a destructor is called on all existing elements
and the @ref Array::size() is set to @cpp 0 @ce, with @ref arrayCapacity()
staying the same as before.

Amortized complexity is @f$ \mathcal{O}(n) @f$ where @f$ n @f$ is the number of
items in the array. On top of what the @p Allocator (or the default
@ref ArrayAllocator) itself needs, @p T is required to be nothrow
move-constructible.

This function is equivalent to calling @relativeref{std::vector,clear()} on a
@ref std::vector.
@m_keywords{clear()}
@see @ref arrayIsGrowable(), @ref arrayRemove(), @ref arrayRemoveUnordered(),
    @ref arrayRemoveSuffix(), @ref arrayResize()
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayClear(Array<T>& array);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayClear(Array<T>& array) {
    arrayClear<T, Allocator<T>>(array);
}
#endif

/**
@brief Convert an array back to non-growable
@m_since{2020,06}

Allocates a @relativeref{Corrade,NoInit} array that's exactly large enough to
fit @ref Array::size() elements, move-constructs the elements there and frees
the old memory using @ref Array::deleter(). If the array is not growable using
given @p Allocator, it's assumed to be already as small as possible, and
nothing is done.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the container,
@f$ \mathcal{O}(1) @f$ if the array is already non-growable. No constraints
on @p T from @p Allocator (or the default @ref ArrayAllocator) apply here but
@p T is required to be nothrow move-constructible.

This function is equivalent to calling @relativeref{std::vector,shrink_to_fit()}
on a @ref std::vector.
@m_keywords{shrink_to_fit()}
@see @ref arrayShrink(Array<T>&, ValueInitT), @ref arrayIsGrowable(),
    @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayShrink(Array<T>& array, Corrade::NoInitT = Corrade::NoInit);

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
template<template<class> class Allocator, class T> inline void arrayShrink(Array<T>& array, Corrade::NoInitT = Corrade::NoInit) {
    arrayShrink<T, Allocator<T>>(array, Corrade::NoInit);
}
#endif

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Convert an array back to non-growable using a default initialization
@m_deprecated_since_latest Because C++'s default initialization keeps trivial
    types not initialized, the @ref Array::Array(DefaultInitT, std::size_t)
    constructor is deprecated. Use @ref arrayShrink(Array<T>&, ValueInitT)
    instead.

Allocates a @relativeref{Corrade,DefaultInit} array that's exactly large enough
to fit @ref Array::size() elements, move-assigns the elements there and frees
the old memory using @ref Array::deleter(). If the array is not growable using
given @p Allocator, it's assumed to be already as small as possible, and
nothing is done.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the container,
@f$ \mathcal{O}(1) @f$ if the array is already non-growable. No constraints on
@p T from @p Allocator (or the default @ref ArrayAllocator) apply here but @p T
is required to be default-constructible and nothrow move-assignable.

Compared to @ref arrayShrink(Array<T>&, NoInitT), the resulting array instance
always has a default (@cpp nullptr @ce) deleter. This is useful when it's not
possible to use custom deleters, such as in plugin implementations.
@see @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> CORRADE_DEPRECATED("use arrayShrink(Array<T>&, ValueInitT) instead") void arrayShrink(Array<T>& array, Corrade::DefaultInitT);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_deprecated_since_latest Because C++'s default initialization keeps trivial
    types not initialized, the @ref Array::Array(DefaultInitT, std::size_t)
    constructor is deprecated. Use @ref arrayShrink(Array<T>&, ValueInitT)
    instead.

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline CORRADE_DEPRECATED("use arrayShrink(Array<T>&, ValueInitT) instead") void arrayShrink(Array<T>& array, Corrade::DefaultInitT) {
    CORRADE_IGNORE_DEPRECATED_PUSH
    arrayShrink<T, Allocator<T>>(array, Corrade::DefaultInit);
    CORRADE_IGNORE_DEPRECATED_POP
}
#endif
#endif

/**
@brief Convert an array back to non-growable using a value initialization
@m_since_latest

Allocates a @relativeref{Corrade,ValueInit} array that's exactly large enough
to fit @ref Array::size() elements, move-assigns the elements there and frees
the old memory using @ref Array::deleter(). If the array is not growable using
given @p Allocator, it's assumed to be already as small as possible, and
nothing is done.

Complexity is at most @f$ \mathcal{O}(n) @f$ in the size of the container,
@f$ \mathcal{O}(1) @f$ if the array is already non-growable. No constraints on
@p T from @p Allocator (or the default @ref ArrayAllocator) apply here but @p T
is required to be default-constructible and nothrow move-assignable.

Compared to @ref arrayShrink(Array<T>&, NoInitT), the resulting array instance
always has a default (@cpp nullptr @ce) deleter. This is useful when it's not
possible to use custom deleters, such as in plugin implementations.
@see @ref arrayIsGrowable(), @ref Containers-Array-growable
*/
template<class T, class Allocator = ArrayAllocator<T>> void arrayShrink(Array<T>& array, Corrade::ValueInitT);

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
/**
@overload
@m_since_latest

Convenience overload allowing to specify just the allocator template, with
array type being inferred.
*/
template<template<class> class Allocator, class T> inline void arrayShrink(Array<T>& array, Corrade::ValueInitT) {
    arrayShrink<T, Allocator<T>>(array, Corrade::ValueInit);
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

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayMoveConstruct(T* const src, T* const dst, const std::size_t count) {
    /* Apparently memcpy() can't be called with null pointers, even if size is
       zero. I call that bullying. */
    if(count) std::memcpy(dst, src, count*sizeof(T));
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(T) || !__has_trivial_destructor(T)
    #else
    !std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayMoveConstruct(T* src, T* dst, const std::size_t count) {
    static_assert(std::is_nothrow_move_constructible<T>::value,
        "nothrow move-constructible type is required");
    for(T* end = src + count; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(*dst, Utility::move(*src));
        #else
        new(dst) T{Utility::move(*src)};
        #endif
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayMoveAssign(T* const src, T* const dst, const std::size_t count) {
    /* Apparently memcpy() can't be called with null pointers, even if size is
       zero. I call that bullying. */
    if(count) std::memcpy(dst, src, count*sizeof(T));
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(T) || !__has_trivial_destructor(T)
    #else
    !std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayMoveAssign(T* src, T* dst, const std::size_t count) {
    static_assert(std::is_nothrow_move_assignable<T>::value,
        "nothrow move-assignable type is required");
    for(T* end = src + count; src != end; ++src, ++dst)
        *dst = Utility::move(*src);
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayCopyConstruct(const T* const src, T* const dst, const std::size_t count) {
    /* Apparently memcpy() can't be called with null pointers, even if size is
       zero. I call that bullying. */
    if(count) std::memcpy(dst, src, count*sizeof(T));
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(T) || !__has_trivial_destructor(T)
    #else
    !std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayCopyConstruct(const T* src, T* dst, const std::size_t count) {
    for(const T* end = src + count; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
        Implementation::construct(*dst, *src);
        #else
        new(dst) T{*src};
        #endif
}

template<class T, typename std::enable_if<std::is_trivially_destructible<T>::value, int>::type = 0> inline void arrayDestruct(T*, T*) {
    /* Nothing to do */
}

template<class T, typename std::enable_if<!std::is_trivially_destructible<T>::value, int>::type = 0> inline void arrayDestruct(T* begin, T* const end) {
    /* Needs to be < because sometimes begin > end */
    for(; begin < end; ++begin) begin->~T();
}

template<class T> inline std::size_t arrayGrowth(const std::size_t currentCapacity, const std::size_t desiredCapacity) {
    /** @todo pick a nice value when current = 0 and desired > 1 */
    const std::size_t currentCapacityInBytes = sizeof(T)*currentCapacity + Implementation::AllocatorTraits<T>::Offset;

    /* For small allocations we want to tightly fit into size buckets (8, 16,
       32, 64 bytes), so it's better to double the capacity every time. For
       larger, increase just by 50%. The capacity is calculated including the
       space needed to store the capacity value (so e.g. a 16-byte allocation
       can store two ints, but when it's doubled to 32 bytes, it can store
       six of them). */
    std::size_t grown;
    if(currentCapacityInBytes < DefaultAllocationAlignment)
        grown = DefaultAllocationAlignment;
    else if(currentCapacityInBytes < 64)
        grown = currentCapacityInBytes*2;
    else
        grown = currentCapacityInBytes + currentCapacityInBytes/2;

    const std::size_t candidate = (grown - Implementation::AllocatorTraits<T>::Offset)/sizeof(T);
    return desiredCapacity > candidate ? desiredCapacity : candidate;
}

}

template<class T> void ArrayNewAllocator<T>::reallocate(T*& array, const std::size_t prevSize, const std::size_t newCapacity) {
    T* newArray = allocate(newCapacity);
    static_assert(std::is_nothrow_move_constructible<T>::value,
        "nothrow move-constructible type is required");
    for(T *src = array, *end = src + prevSize, *dst = newArray; src != end; ++src, ++dst)
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
        Implementation::construct(*dst, Utility::move(*src));
        #else
        new(dst) T{Utility::move(*src)};
        #endif
    for(T *it = array, *end = array + prevSize; it < end; ++it) it->~T();
    deallocate(array);
    array = newArray;
}

template<class T> void ArrayMallocAllocator<T>::reallocate(T*& array, std::size_t, const std::size_t newCapacity) {
    const std::size_t inBytes = newCapacity*sizeof(T) + AllocationOffset;
    char* const memory = static_cast<char*>(std::realloc(reinterpret_cast<char*>(array) - AllocationOffset, inBytes));
    CORRADE_ASSERT(memory,
        "Containers::ArrayMallocAllocator: can't reallocate" << inBytes << "bytes", );
    reinterpret_cast<std::size_t*>(memory)[0] = inBytes;
    array = reinterpret_cast<T*>(memory + AllocationOffset);
}

template<class T> std::size_t ArrayNewAllocator<T>::grow(T* const array, const std::size_t desiredCapacity) {
    return Implementation::arrayGrowth<T>(array ? capacity(array) : 0, desiredCapacity);
}

template<class T> std::size_t ArrayMallocAllocator<T>::grow(T* const array, const std::size_t desiredCapacity) {
    return Implementation::arrayGrowth<T>(array ? capacity(array) : 0, desiredCapacity);
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
    const std::size_t currentCapacity = arrayCapacity<T, Allocator>(array);
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

template<class T, class Allocator> void arrayResize(Array<T>& array, Corrade::NoInitT, const std::size_t size) {
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

#ifdef CORRADE_BUILD_DEPRECATED
template<class T, class Allocator> void arrayResize(Array<T>& array, Corrade::DefaultInitT, const std::size_t size) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, Corrade::NoInit, size);
    Implementation::arrayConstruct(Corrade::DefaultInit, array + prevSize, array.end());
}
#endif

template<class T, class Allocator> void arrayResize(Array<T>& array, Corrade::ValueInitT, const std::size_t size) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, Corrade::NoInit, size);
    Implementation::arrayConstruct(Corrade::ValueInit, array + prevSize, array.end());
}

template<class T, class Allocator, class ...Args> void arrayResize(Array<T>& array, Corrade::DirectInitT, const std::size_t size, Args&&... args) {
    const std::size_t prevSize = array.size();
    arrayResize<T, Allocator>(array, Corrade::NoInit, size);

    /* In-place construct the new elements. No helper function for this as
       there's no way we could memcpy such a thing. */
    for(T* it = array + prevSize; it < array.end(); ++it)
        Implementation::construct(*it, Utility::forward<Args>(args)...);
}

template<class T, class ...Args> inline void arrayResize(Array<T>& array, Corrade::DirectInitT, const std::size_t size, Args&&... args) {
    arrayResize<T, ArrayAllocator<T>>(array, Corrade::DirectInit, size, Utility::forward<Args>(args)...);
}

namespace Implementation {

template<class T, class Allocator> T* arrayGrowBy(Array<T>& array, const std::size_t count) {
    /* Direct access & caching to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* No values to add, early exit */
    if(!count)
        return arrayGuts.data + arrayGuts.size;

    /* For arrays with an unknown deleter we'll always copy-allocate to a new
       place. Not using reallocate() as we don't know where the original memory
       comes from. */
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

template<class T, class Allocator> inline T& arrayAppend(Array<T>& array, const typename std::common_type<T>::type& value) {
    CORRADE_DEBUG_ASSERT(std::size_t(&value - array.data()) >= arrayCapacity(array),
        "Containers::arrayAppend(): use the list variant to append values from within the array itself", *array.data());
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, 1);
    /* Can't use {}, see the GCC 4.8-specific overload for details */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
    Implementation::construct(*it, value);
    #else
    new(it) T{value};
    #endif
    return *it;
}

template<class T, class Allocator> inline ArrayView<T> arrayAppend(Array<T>& array, const typename std::common_type<ArrayView<const T>>::type values) {
    /* Direct access & caching to speed up debug builds */
    const T* const valueData = values.data();
    const std::size_t valueCount = values.size();

    /* If the values are actually a slice of the original array, we need to
       relocate the view after growing because it may point to a stale location
       afterwards. If the offset is outside of the [0, capacity) range of the
       original array, we don't relocate. Similar check is in arrayInsert(),
       where it additionally has to adjust the offset based on whether the
       values are before or after the insertion point. */
    std::size_t relocateOffset = std::size_t(valueData - array.data());
    if(relocateOffset >= arrayCapacity<T, Allocator>(array))
        relocateOffset = ~std::size_t{};

    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, valueCount);
    Implementation::arrayCopyConstruct<T>(
        /* If values were a slice of the original array, relocate the view
           pointer relative to the (potentially reallocated) array. It may have
           pointed into the (potentially uninitialized) capacity, in which case
           we'll likely copy some garbage or we overwrite ourselves, but that's
           the user fault (and ASan would catch it). OTOH, if the capacity
           wouldn't be taken into account above, we may end up reading from
           freed memory, which is far worse. */
        relocateOffset != ~std::size_t{} ? array.data() + relocateOffset :
            valueData,
        it, valueCount);
    return {it, valueCount};
}

template<class T, class ...Args> inline T& arrayAppend(Array<T>& array, Corrade::InPlaceInitT, Args&&... args) {
    return arrayAppend<T, ArrayAllocator<T>>(array, Corrade::InPlaceInit, Utility::forward<Args>(args)...);
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T, class Allocator, class ...Args> T& arrayAppend(Array<T>& array, Corrade::InPlaceInitT, Args&&... args) {
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, 1);
    /* No helper function as there's no way we could memcpy such a thing. */
    /* On GCC 4.8 this includes another workaround, see the 4.8-specific
       overload docs for details */
    Implementation::construct(*it, Utility::forward<Args>(args)...);
    return *it;
}
#endif

template<class T, class Allocator> ArrayView<T> arrayAppend(Array<T>& array, Corrade::NoInitT, const std::size_t count) {
    T* const it = Implementation::arrayGrowBy<T, Allocator>(array, count);
    return {it, count};
}

template<class T, class Allocator> ArrayView<T> arrayAppend(Array<T>& array, Corrade::ValueInitT, const std::size_t count) {
    const ArrayView<T> out = arrayAppend<T, Allocator>(array, Corrade::NoInit, count);
    Implementation::arrayConstruct(Corrade::ValueInit, out.begin(), out.end());
    return out;
}

template<class T, class Allocator, class ...Args> ArrayView<T> arrayAppend(Array<T>& array, Corrade::DirectInitT, const std::size_t count, Args&&... args) {
    const ArrayView<T> out = arrayAppend<T, Allocator>(array, Corrade::NoInit, count);

    /* In-place construct the new elements. No helper function for this as
       there's no way we could memcpy such a thing. */
    for(T* it = out.begin(); it < out.end(); ++it)
        Implementation::construct(*it, Utility::forward<Args>(args)...);

    return out;
}

template<class T, class ...Args> ArrayView<T> arrayAppend(Array<T>& array, Corrade::DirectInitT, const std::size_t count, Args&&... args) {
    return arrayAppend<T, ArrayAllocator<T>>(array, Corrade::DirectInit, count, Utility::forward<Args>(args)...);
}

namespace Implementation {

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayShiftForward(T* const src, T* const dst, const std::size_t count) {
    /* Compared to the non-trivially-copyable variant below, just delegate to
       memmove() and assume it can figure out how to copy from back to front
       more efficiently that we ever could.

       Same as with memcpy(), apparently memmove() can't be called with null
       pointers, even if size is zero. I call that bullying. */
    if(count) std::memmove(dst, src, count*sizeof(T));
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(T) || !__has_trivial_destructor(T)
    #else
    !std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayShiftForward(T* const src, T* const dst, const std::size_t count) {
    static_assert(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value,
        "nothrow move-constructible and move-assignable type is required");

    /* Count of non-overlapping items, which will be move-constructed on one
       side and destructed on the other. The rest will be move-assigned. */
    const std::size_t nonOverlappingCount = src + count < dst ? count : dst - src;

    /* Move-construct the non-overlapping elements. Doesn't matter if going
       forward or backward as we're not overwriting anything, but go backward
       for consistency with the move-assignment loop below. */
    for(T *end = src + count - nonOverlappingCount, *constructSrc = src + count, *constructDst = dst + count; constructSrc > end; --constructSrc, --constructDst) {
        /* Can't use {}, see the GCC 4.8-specific overload for details */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
        Implementation::construct(*(constructDst - 1), Utility::move(*(constructSrc - 1)));
        #else
        new(constructDst - 1) T{Utility::move(*(constructSrc - 1))};
        #endif
    }

    /* Move-assign overlapping elements, going backwards to avoid overwriting
       values that are yet to be moved. This loop is never entered if
       nonOverlappingCount >= count. */
    for(T *assignSrc = src + count - nonOverlappingCount, *assignDst = dst + count - nonOverlappingCount; assignSrc > src; --assignSrc, --assignDst)
        *(assignDst - 1) = Utility::move(*(assignSrc - 1));

    /* Destruct non-overlapping elements in the newly-formed gap so the calling
       code can assume uninitialized memory both in all cases. Here it again
       doesn't matter if going forward or backward, but go backward for
       consistency. */
    /** @todo prefer a move assignment instead -- needs arrayGrowAtBy() to
        return some sort of a flag that tells the caller whether the new items
        have to be constructed or moved -- and then, if moved, the NoInit
        variant would be calling a destructor on its own, to make sure the
        caller can always placement-new the items without risking any resource
        leaks */
    for(T *destructSrc = src + nonOverlappingCount; destructSrc != src; --destructSrc)
        (destructSrc - 1)->~T();
}

template<class T, class Allocator> T* arrayGrowAtBy(Array<T>& array, const std::size_t index, const std::size_t count) {
    /* Direct access & caching to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    CORRADE_DEBUG_ASSERT(index <= arrayGuts.size, "Containers::arrayInsert(): can't insert at index" << index << "into an array of size" << arrayGuts.size, arrayGuts.data);

    /* No values to add, early exit */
    if(!count)
        return arrayGuts.data + index;

    /* For arrays with an unknown deleter we'll always move-allocate to a new
       place, the parts before and after index separately. Not using
       reallocate() as we don't know where the original memory comes from. */
    const std::size_t desiredCapacity = arrayGuts.size + count;
    std::size_t capacity;
    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    T* oldMid = nullptr;
    #endif
    bool needsShiftForward = false;
    if(arrayGuts.deleter != Allocator::deleter) {
        capacity = Allocator::grow(nullptr, desiredCapacity);
        T* const newArray = Allocator::allocate(capacity);
        arrayMoveConstruct<T>(arrayGuts.data, newArray, index);
        arrayMoveConstruct<T>(arrayGuts.data + index, newArray + index + count, arrayGuts.size - index);
        array = Array<T>{newArray, arrayGuts.size, Allocator::deleter};

    /* Otherwise, if there's no space anymore, reallocate. which might be able
       to grow in-place. However we still need to shift the part after index
       forward. */
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

        /** @todo do this as part of a reallocation instead of a second pass
            to speed up -- Allocator::reallocate() could keep the original
            memory intact instead of deleting it, move just the first `index`
            data, then return the new pointer (instead of modifying the
            original) and leave the rest to be moved by us. The case of
            reallocating in-place still needs to be taken care of tho (return
            a null pointer, in which case arrayShiftForward() still gets
            called?) */
        needsShiftForward = true;
    }

    /* Increase array size and return the position at index */
    T* const it = arrayGuts.data + index;
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

    /* Perform a shift of elements after index. Needs to be done after the ASan
       annotation is updated, otherwise it'll trigger a failure due to outdated
       bounds information. */
    if(needsShiftForward)
        arrayShiftForward(arrayGuts.data + index, arrayGuts.data + index + count, arrayGuts.size - index);

    arrayGuts.size += count;
    return it;
}

}

template<class T, class Allocator> inline T& arrayInsert(Array<T>& array, std::size_t index, const typename std::common_type<T>::type& value) {
    CORRADE_DEBUG_ASSERT(std::size_t(&value - array.data()) >= arrayCapacity(array),
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself", *array.data());
    T* const it = Implementation::arrayGrowAtBy<T, Allocator>(array, index, 1);
    /* Can't use {}, see the GCC 4.8-specific overload for details */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) &&  __GNUC__ < 5
    Implementation::construct(*it, value);
    #else
    new(it) T{value};
    #endif
    return *it;
}

template<class T, class Allocator> inline ArrayView<T> arrayInsert(Array<T>& array, std::size_t index, const typename std::common_type<ArrayView<const T>>::type values) {
    /* Direct access & caching to speed up debug builds */
    const T* const valueData = values.data();
    const std::size_t valueCount = values.size();

    /* If the values are actually a slice of the original array, we need to
       relocate the view after growing because it may point to a stale location
       afterwards. If the offset is outside of the [0, capacity) range of the
       original array, we don't relocate. Similar but simpler check is in
       arrayAppend(). */
    std::size_t relocateOffset = std::size_t(valueData - array.data());
    if(relocateOffset < arrayCapacity<T, Allocator>(array)) {
        /* If we're inserting before the original slice, the new offset has to
           include also the inserted size */
        if(index <= relocateOffset)
            relocateOffset += valueCount;
        /* Otherwise the index should not point inside the slice, as we'd have
           to split the copy into two parts. The assumption is that this is a
           very rare scenario (with very questionable practical usefulness),
           and the caller should handle that on its own. */
        else CORRADE_DEBUG_ASSERT(relocateOffset + valueCount <= index,
            "Containers::arrayInsert(): attempting to insert a slice [" << Utility::Debug::nospace << relocateOffset << Utility::Debug::nospace << ":" << Utility::Debug::nospace << relocateOffset + valueCount << Utility::Debug::nospace << "] into itself at index" << index, {});
    } else relocateOffset = ~std::size_t{};

    T* const it = Implementation::arrayGrowAtBy<T, Allocator>(array, index, valueCount);
    Implementation::arrayCopyConstruct<T>(
        /* If values were a slice of the original array, relocate the view
           pointer relative to the (potentially reallocated) array. Similarly
           as with arrayAppend(), it may have pointed into the capacity, which
           we handle by copying potential garbage instead of accessing freed
           memory. */
        relocateOffset != ~std::size_t{} ? array.data() + relocateOffset :
            valueData,
        it, valueCount);
    return {it, valueCount};
}

template<class T, class ...Args> inline T& arrayInsert(Array<T>& array, std::size_t index, Corrade::InPlaceInitT, Args&&... args) {
    return arrayInsert<T, ArrayAllocator<T>>(array, index, Corrade::InPlaceInit, Utility::forward<Args>(args)...);
}

/* This crap tool can't distinguish between this and above overload, showing
   just one with the docs melted together. More useless than showing nothing
   at all, so hiding this one from it until it improves. */
#ifndef DOXYGEN_GENERATING_OUTPUT
template<class T, class Allocator, class ...Args> T& arrayInsert(Array<T>& array, std::size_t index, Corrade::InPlaceInitT, Args&&... args) {
    T* const it = Implementation::arrayGrowAtBy<T, Allocator>(array, index, 1);
    /* No helper function as there's no way we could memcpy such a thing. */
    /* On GCC 4.8 this includes another workaround, see the 4.8-specific
       overload docs for details */
    Implementation::construct(*it, Utility::forward<Args>(args)...);
    return *it;
}
#endif

template<class T, class Allocator> ArrayView<T> arrayInsert(Array<T>& array, const std::size_t index, Corrade::NoInitT, const std::size_t count) {
    T* const it = Implementation::arrayGrowAtBy<T, Allocator>(array, index, count);
    return {it, count};
}

template<class T, class Allocator> ArrayView<T> arrayInsert(Array<T>& array, const std::size_t index, Corrade::ValueInitT, const std::size_t count) {
    const ArrayView<T> out = arrayInsert<T, Allocator>(array, index, Corrade::NoInit, count);
    Implementation::arrayConstruct(Corrade::ValueInit, out.begin(), out.end());
    return out;
}

template<class T, class Allocator, class ...Args> ArrayView<T> arrayInsert(Array<T>& array, const std::size_t index, Corrade::DirectInitT, const std::size_t count, Args&&... args) {
    const ArrayView<T> out = arrayInsert<T, Allocator>(array, index, Corrade::NoInit, count);

    /* In-place construct the new elements. No helper function for this as
       there's no way we could memcpy such a thing. */
    for(T* it = out.begin(); it < out.end(); ++it)
        Implementation::construct(*it, Utility::forward<Args>(args)...);

    return out;
}

template<class T, class ...Args> ArrayView<T> arrayInsert(Array<T>& array, const std::size_t index, Corrade::DirectInitT, const std::size_t count, Args&&... args) {
    return arrayInsert<T, ArrayAllocator<T>>(array, index, Corrade::DirectInit, count, Utility::forward<Args>(args)...);
}

namespace Implementation {

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_copy(T) && __has_trivial_destructor(T)
    #else
    std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayShiftBackward(T* const src, T* const dst, const std::size_t moveCount, std::size_t) {
    /* Compared to the non-trivially-copyable variant below, just delegate to
       memmove() and assume it can figure out how to copy from front to back
       more efficiently that we ever could.

       Same as with memcpy(), apparently memmove() can't be called with null
       pointers, even if size is zero. I call that bullying. */
    if(moveCount) std::memmove(dst, src, moveCount*sizeof(T));
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(T) || !__has_trivial_destructor(T)
    #else
    !std::is_trivially_copyable<T>::value
    #endif
, int>::type = 0> inline void arrayShiftBackward(T* const src, T* const dst, const std::size_t moveCount, std::size_t destructCount) {
    static_assert(std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value,
        "nothrow move-constructible and move-assignable type is required");

    /* Move-assign later elements to earlier */
    for(T *end = src + moveCount, *assignSrc = src, *assignDst = dst; assignSrc != end; ++assignSrc, ++assignDst)
        *assignDst = Utility::move(*assignSrc);

    /* Destruct remaining moved-out elements */
    for(T *end = src + moveCount, *destructSrc = end - destructCount; destructSrc != end; ++destructSrc)
        destructSrc->~T();
}

}

template<class T, class Allocator> void arrayRemove(Array<T>& array, const std::size_t index, const std::size_t count) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    CORRADE_DEBUG_ASSERT(index + count <= arrayGuts.size, "Containers::arrayRemove(): can't remove" << count << "elements at index" << index << "from an array of size" << arrayGuts.size, );

    /* Nothing to remove, yay! */
    if(!count) return;

    /* If we don't have our own deleter, we need to reallocate in order to
       store the capacity. Move the parts before and after the index separately,
       which will also cause the removed elements to be properly destructed, so
       nothing else needs to be done. Not using reallocate() as we don't know
       where the original memory comes from. */
    if(arrayGuts.deleter != Allocator::deleter) {
        T* const newArray = Allocator::allocate(arrayGuts.size - count);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, index);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data + index + count, newArray + index, arrayGuts.size - index - count);
        array = Array<T>{newArray, arrayGuts.size - count, Allocator::deleter};

        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        /* This should basically be a no-op, right? */
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size);
        #endif

    /* Otherwise shift the elements after index backward */
    } else {
        Implementation::arrayShiftBackward(arrayGuts.data + index + count, arrayGuts.data + index, arrayGuts.size - index - count, count);
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

template<class T, class Allocator> void arrayRemoveUnordered(Array<T>& array, const std::size_t index, const std::size_t count) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    CORRADE_DEBUG_ASSERT(index + count <= arrayGuts.size, "Containers::arrayRemoveUnordered(): can't remove" << count << "elements at index" << index << "from an array of size" << arrayGuts.size, );

    /* Nothing to remove, yay! */
    if(!count) return;

    /* If we don't have our own deleter, we need to reallocate in order to
       store the capacity. Move the parts before and after the index separately,
       which will also cause the removed elements to be properly destructed, so
       nothing else needs to be done. Not using reallocate() as we don't know
       where the original memory comes from. */
    if(arrayGuts.deleter != Allocator::deleter) {
        T* const newArray = Allocator::allocate(arrayGuts.size - count);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, index);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data + index + count, newArray + index, arrayGuts.size - index - count);
        array = Array<T>{newArray, arrayGuts.size - count, Allocator::deleter};

        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        /* This should basically be a no-op, right? */
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data + arrayGuts.size);
        #endif

    /* Otherwise move the last count elements over the ones at index, or less
       if there's not that many after the removed range */
    } else {
        const std::size_t moveCount = Utility::min(count, arrayGuts.size - count - index);
        Implementation::arrayShiftBackward(arrayGuts.data + arrayGuts.size - moveCount, arrayGuts.data + index, moveCount, count);
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

template<class T, class Allocator> void arrayRemoveSuffix(Array<T>& array, const std::size_t count) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);
    CORRADE_DEBUG_ASSERT(count <= arrayGuts.size, "Containers::arrayRemoveSuffix(): can't remove" << count << "elements from an array of size" << arrayGuts.size, );

    /* Nothing to remove, yay! */
    if(!count) return;

    /* If we don't have our own deleter, we need to reallocate in order to
       store the capacity. That'll also cause the excessive elements to be
       properly destructed, so nothing else needs to be done. Not using
       reallocate() as we don't know where the original memory comes from. */
    if(arrayGuts.deleter != Allocator::deleter) {
        T* const newArray = Allocator::allocate(arrayGuts.size - count);
        Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, arrayGuts.size - count);
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
        Implementation::arrayDestruct<T>(arrayGuts.data + arrayGuts.size - count, arrayGuts.data + arrayGuts.size);
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

template<class T, class Allocator> void arrayClear(Array<T>& array) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, simply free the existing contents */
    if(arrayGuts.deleter != Allocator::deleter) {
        array = {};

    /* Otherwise call the destructor on the excessive elements and update the
       size */
    } else {
        Implementation::arrayDestruct<T>(arrayGuts.data, arrayGuts.data + arrayGuts.size);
        #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
        __sanitizer_annotate_contiguous_container(
            Allocator::base(arrayGuts.data),
            arrayGuts.data + Allocator::capacity(arrayGuts.data),
            arrayGuts.data + arrayGuts.size,
            arrayGuts.data);
        #endif
        arrayGuts.size = 0;
    }
}

template<class T, class Allocator> void arrayShrink(Array<T>& array, Corrade::NoInitT) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, assume the array size equals its
       capacity and do nothing */
    if(arrayGuts.deleter != Allocator::deleter)
        return;

    /* Even if we don't need to shrink, reallocating to an usual array with
       common deleters to avoid surprises */
    Array<T> newArray{Corrade::NoInit, arrayGuts.size};
    Implementation::arrayMoveConstruct<T>(arrayGuts.data, newArray, arrayGuts.size);
    array = Utility::move(newArray);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /* Nothing to do (not annotating the arrays with default deleter) */
    #endif
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T, class Allocator> void arrayShrink(Array<T>& array, Corrade::DefaultInitT) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, assume the array size equals its
       capacity and do nothing */
    if(arrayGuts.deleter != Allocator::deleter)
        return;

    /* Even if we don't need to shrink, reallocating to an usual array with
       common deleters to avoid surprises */
    CORRADE_IGNORE_DEPRECATED_PUSH
    Array<T> newArray{Corrade::DefaultInit, arrayGuts.size};
    CORRADE_IGNORE_DEPRECATED_POP
    Implementation::arrayMoveAssign<T>(arrayGuts.data, newArray, arrayGuts.size);
    array = Utility::move(newArray);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /* Nothing to do (not annotating the arrays with default deleter) */
    #endif
}
#endif

template<class T, class Allocator> void arrayShrink(Array<T>& array, Corrade::ValueInitT) {
    /* Direct access to speed up debug builds */
    auto& arrayGuts = reinterpret_cast<Implementation::ArrayGuts<T>&>(array);

    /* If not using our growing allocator, assume the array size equals its
       capacity and do nothing */
    if(arrayGuts.deleter != Allocator::deleter)
        return;

    /* Even if we don't need to shrink, reallocating to an usual array with
       common deleters to avoid surprises */
    Array<T> newArray{Corrade::ValueInit, arrayGuts.size};
    Implementation::arrayMoveAssign<T>(arrayGuts.data, newArray, arrayGuts.size);
    array = Utility::move(newArray);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    /* Nothing to do (not annotating the arrays with default deleter) */
    #endif
}

}}

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
#undef _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif

#endif
