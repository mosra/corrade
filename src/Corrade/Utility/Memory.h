#ifndef Corrade_Utility_Memory_h
#define Corrade_Utility_Memory_h
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
 * @brief Function @ref Corrade::Utility::allocateAligned()
 * @m_since_latest
 */

/* Not inside System.h because there we don't want the Array include, and
   this header may grow with utilities for vmem-backed non-reallocating
   containers or magic ring buffers */

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/initializeHelpers.h"
#include "Corrade/Utility/Math.h"
#include "Corrade/Utility/visibility.h"

/* In some cases on Unix #include <malloc.h> works, but Apple wants <stdlib.h>
   and elsewhere it likely may only worked because <stdlib.h> was pulled in by
   something else. Furthermore, we need <cstdlib> for std::free(), so include
   it always. */
#include <cstdlib>
#ifdef CORRADE_TARGET_WINDOWS
/* On Windows it's <malloc.h>, and it's relatively tiny without pulling in any
   of the <windows.h> horror, so it's fine and we don't need to forward-declare
   anything */
#include <malloc.h>
#endif

namespace Corrade { namespace Utility {

/**
@brief Allocate aligned memory and value-initialize it
@tparam T           Type of the returned array
@tparam alignment   Allocation alignment, in bytes
@param  size        Count of @p T items to allocate. If @cpp 0 @ce, no
    allocation is done.
@m_since_latest

Compared to the classic C @ref std::malloc() or C++ @cpp new @ce that commonly
aligns only to @cpp 2*sizeof(void*) @ce, this function returns "overaligned"
allocations, which is mainly useful for efficient SIMD operations. Example
usage:

@snippet Utility.cpp allocateAligned

The alignment is implicitly @cpp alignof(T) @ce, but can be overridden with the
@p alignment template parameter. When specified explicitly, it is expected to
be a power-of-two value, at most @cpp 256 @ce bytes and the total byte size
being a multiple of the alignment:

@snippet Utility.cpp allocateAligned-explicit

The returned pointer is always aligned to at least the desired value, but the
alignment can be also higher. For example, allocating a 2 MB buffer may result
in it being aligned to the whole memory page, or small alignment values could
get rounded up to the default @cpp 2*sizeof(void*) @ce alignment.

The function is implemented using @m_class{m-doc-external} [posix_memalign()](https://man.archlinux.org/man/posix_memalign.3)
on @ref CORRADE_TARGET_UNIX "UNIX" systems and @m_class{m-doc-external} [_aligned_malloc()](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc)
on @ref CORRADE_TARGET_WINDOWS "Windows". On other platforms (such as
@ref CORRADE_TARGET_EMSCRIPTEN "Emscripten"), if requested alignment is higher
than platform's default alignment, the allocation is done via a classic
@ref std::malloc() with an @cpp alignment - 1 @ce padding and the returned
pointer is then patched to satisfy the alignment. In all cases the returned
@ref Containers::Array has a custom deleter, which for non-trivial types calls
destructors on all types, and then either @ref std::free() or, in case of
Windows, @m_class{m-doc-external} [_aligned_free()](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-free)
is used to deallocate the memory.

@section Utility-allocateAligned-initialization Array initialization

Like with @ref Containers::Array, the returned array is by default
* *value-initialized*, which means that trivial types are zero-initialized and
the default constructor is called on other types. Different behavior can be
achieved with the following tags, compared to @ref Containers::Array the
initialization is performed separately from the allocation itself with either a
loop or a call to @ref std::memset().

-   @ref allocateAligned(ValueInitT, std::size_t) is equivalent to the default
    case, zero-initializing trivial types and calling the default constructor
    elsewhere. Useful when you want to make the choice appear explicit.
-   @ref allocateAligned(NoInitT, std::size_t) does not initialize anything.
    Useful for trivial types when you'll be overwriting the contents anyway,
    for non-trivial types this is the dangerous option and you need to call the
    constructor on all elements manually using placement new,
    @ref std::uninitialized_copy() or similar --- see the function docs for an
    example.

@see @ref Cpu
*/
template<class T, std::size_t alignment = alignof(T)> inline Containers::Array<T> allocateAligned(std::size_t size);

#ifdef CORRADE_BUILD_DEPRECATED
/**
@brief Allocate aligned memory and default-initialize it
@m_deprecated_since_latest Because C++'s default initialization keeps trivial
    types not initialized, using it is unnecessarily error prone. Use either
    @ref allocateAligned(ValueInitT, std::size_t) or
    @ref allocateAligned(NoInitT, std::size_t) instead to make the choice about
    content initialization explicit.

Compared to @ref allocateAligned(std::size_t), trivial types are not
initialized and default constructor is called otherwise.

Implemented via @ref allocateAligned(NoInitT, std::size_t) with a
loop calling the constructors on the returned allocation in case of non-trivial
types.
@see @ref allocateAligned(ValueInitT, std::size_t), @ref Cpu
*/
template<class T, std::size_t alignment = alignof(T)> CORRADE_DEPRECATED("use allocateAligned(ValueInitT, std::size_t) or allocateAligned(NoInitT, std::size_t) instead") Containers::Array<T> allocateAligned(DefaultInitT, std::size_t size);
#endif

/**
@brief Allocate aligned memory and value-initialize it
@m_since_latest

Same as @ref allocateAligned(std::size_t), just more explicit. Implemented via
@ref allocateAligned(NoInitT, std::size_t) with either a
@ref std::memset() or a loop calling the constructors on the returned
allocation.
@see @ref Cpu
*/
template<class T, std::size_t alignment = alignof(T)> Containers::Array<T> allocateAligned(ValueInitT, std::size_t size);

/**
@brief Allocate aligned memory and leave it uninitialized
@m_since_latest

Compared to @ref allocateAligned(std::size_t), the memory is left in an
unitialized state. For non-trivial types, destruction is always done using a
custom deleter that explicitly calls the destructor on *all elements* --- which
means that for non-trivial types you're expected to construct all elements
using placement new (or for example @ref std::uninitialized_copy()) in order to
avoid calling destructors on uninitialized memory:

@snippet Utility.cpp allocateAligned-NoInit

@see @ref allocateAligned(ValueInitT, std::size_t), @ref Cpu
*/
template<class T, std::size_t alignment = alignof(T)> Containers::Array<T> allocateAligned(NoInitT, std::size_t size);

namespace Implementation {

#ifdef CORRADE_TARGET_WINDOWS
template<class T, typename std::enable_if<std::is_trivially_destructible<T>::value, int>::type = 0> void alignedDeleter(T* const data, std::size_t) {
    _aligned_free(data);
}
template<class T, typename std::enable_if<!std::is_trivially_destructible<T>::value, int>::type = 0> void alignedDeleter(T* const data, std::size_t size) {
    for(std::size_t i = 0; i != size; ++i) data[i].~T();
    _aligned_free(data);
}
#else
template<class T, typename std::enable_if<std::is_trivially_destructible<T>::value, int>::type = 0> void alignedDeleter(T* const data, std::size_t) {
    std::free(data);
}
template<class T, typename std::enable_if<!std::is_trivially_destructible<T>::value, int>::type = 0> void alignedDeleter(T* const data, std::size_t size) {
    for(std::size_t i = 0; i != size; ++i) data[i].~T();
    std::free(data);
}
#ifndef CORRADE_TARGET_UNIX
template<class T, typename std::enable_if<std::is_trivially_destructible<T>::value, int>::type = 0> void alignedOffsetDeleter(T* const data, std::size_t) {
    /* Using a unsigned byte in order to be able to represent a 255 byte offset
       as well */
    std::uint8_t* const dataChar = reinterpret_cast<std::uint8_t*>(data);
    std::free(dataChar - *(dataChar -1));
}
template<class T, typename std::enable_if<!std::is_trivially_destructible<T>::value, int>::type = 0> void alignedOffsetDeleter(T* const data, std::size_t size) {
    for(std::size_t i = 0; i != size; ++i) data[i].~T();

    /* Using a unsigned byte in order to be able to represent a 255 byte offset
       as well */
    std::uint8_t* const dataChar = reinterpret_cast<std::uint8_t*>(data);
    std::free(dataChar - dataChar[-1]);
}
#endif
#endif

}

template<class T, std::size_t alignment> Containers::Array<T> allocateAligned(NoInitT, const std::size_t size) {
    /* On non-Unix non-Windows platforms we're storing the alignment offset
       in a byte right before the returned pointer. Because it's a byte, we
       can represent a value of at most 255 there (256 would make no sense as
       a 256-byte-aligned allocation can be only off by 255 bytes at most).
       Again it's good to have the same requirements on all platforms so
       checking this always. */
    static_assert(alignment && !(alignment & (alignment - 1)) && alignment <= 256,
        "alignment expected to be a power of two not larger than 256");

    /* Required only by aligned_alloc() I think, but it's good to have the same
       requirements on all platforms for better portability */
    CORRADE_ASSERT(size*sizeof(T) % alignment == 0, "Utility::allocateAligned(): total byte size" << size*sizeof(T) << "not a multiple of a" << alignment << Debug::nospace << "-byte alignment", {});

    /* Unix platforms */
    #ifdef CORRADE_TARGET_UNIX
    /* For some reason, allocating zero bytes still returns a non-null pointer
       which seems weird and confusing. Handle that explicitly instead. */
    if(!size) return {};

    /* I would use aligned_alloc() but then there's APPLE who comes and says
       NO. And on top of everything they DARE to have posix_memalign() in a
       different header.

       What's perhaps a bit surprising is that posix_memalign() requires the
       alignment to be >= sizeof(void*). It seems like a strange requirement --
       it could just overalign for lower alignment values instead of failing.
       Which is what we do here. The Windows _aligned_malloc() API doesn't have
       this requirement. */
    void* data{};
    CORRADE_INTERNAL_ASSERT_OUTPUT(posix_memalign(&data, Utility::max(alignment, sizeof(void*)), size*sizeof(T)) == 0);
    return Containers::Array<T>{static_cast<T*>(data), size, Implementation::alignedDeleter<T>};

    /* Windows */
    #elif defined(CORRADE_TARGET_WINDOWS)
    /* Zero size is not allowed: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc */
    if(!size) return {};

    return Containers::Array<T>{static_cast<T*>(_aligned_malloc(size*sizeof(T), alignment)), size, Implementation::alignedDeleter<T>};

    /* Other -- for allocations larger than the min alignment allocate with
       (align - 1) more and align manually, then provide a custom deleter that
       undoes this. */
    #else
    /* Because we always allocate `alignment - 1` more than the size, it means
       even zero-size allocations would be allocations. Not desirable. */
    if(!size) return {};

    /* Using a unsigned byte in order to be able to represent a 255 byte offset
       as well */
    std::uint8_t* pointer;
    std::ptrdiff_t offset;
    if(alignment <= Containers::Implementation::DefaultAllocationAlignment) {
        pointer = static_cast<std::uint8_t*>(std::malloc(size*sizeof(T)));
        offset = 0;
    } else {
        pointer = static_cast<std::uint8_t*>(std::malloc(size*sizeof(T) + alignment - 1));
        /* Ugh, I'm dumb. Can't this be calculated somehow sane? */
        if(reinterpret_cast<std::ptrdiff_t>(pointer) % alignment == 0)
            offset = 0;
        else
            offset = alignment - reinterpret_cast<std::ptrdiff_t>(pointer) % alignment;
    }
    CORRADE_INTERNAL_ASSERT((reinterpret_cast<std::ptrdiff_t>(pointer) + offset) % alignment == 0);

    /* If the offset is zero, use the classic std::free() directly. If not,
       save the offset in the byte right before what the output pointer will
       point to and use a different deleter that will undo this offset before
       calling std::free(). */
    void(*deleter)(T*, std::size_t);
    if(offset == 0) {
        deleter = Implementation::alignedDeleter<T>;
    } else {
        (pointer + offset)[-1] = offset; /* looking great, isn't it */
        deleter = Implementation::alignedOffsetDeleter<T>;
    }
    return Containers::Array<T>{reinterpret_cast<T*>(pointer + offset), size, deleter};
    #endif
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T, std::size_t alignment> Containers::Array<T> allocateAligned(DefaultInitT, const std::size_t size) {
    Containers::Array<T> out = allocateAligned<T, alignment>(NoInit, size);
    Containers::Implementation::arrayConstruct(DefaultInit, out.begin(), out.end());
    return out;
}
#endif

template<class T, std::size_t alignment> Containers::Array<T> allocateAligned(ValueInitT, const std::size_t size) {
    Containers::Array<T> out = allocateAligned<T, alignment>(NoInit, size);
    Containers::Implementation::arrayConstruct(ValueInit, out.begin(), out.end());
    return out;
}

template<class T, std::size_t alignment> inline Containers::Array<T> allocateAligned(std::size_t size) {
    return allocateAligned<T, alignment>(ValueInit, size);
}

}}

#endif
