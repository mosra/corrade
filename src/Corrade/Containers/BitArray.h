#ifndef Corrade_Containers_BitArray_h
#define Corrade_Containers_BitArray_h
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
 * @brief Class @ref Corrade::Containers::BitArray
 * @m_since_latest
 */

#include <cstddef>
#include <type_traits>
#include <utility> /* std::swap() */ /** @todo make our own */

#include "Corrade/Tags.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

/**
@brief Bit array
@m_since_latest

Owning container for an array of bits. Eight times more memory-efficient than
@ref Array "Array<bool>"; a lighter alternative to @ref std::vector<bool>.
A non-owning version of this container is a @ref BitArrayView and a
@ref MutableBitArrayView, implemented using a generic @ref BasicBitArrayView.

As with @ref Array itself, the container is non-copyable with a size specified
upfront. At the moment, there's no growing functionality.

@section Containers-BitArray-usage Usage

The following snippet shows using a bit array for marking which mesh vertices
are used by an index buffer:

@snippet Containers.cpp BitArray-usage

The @ref BitArray class provides access and slicing APIs similar to
@ref BasicBitArrayView "BitArrayView", see @ref Containers-BasicBitArrayView-usage "its usage docs"
for details. All @ref BitArray slicing APIs return a (mutable)
@ref BasicBitArrayView "BitArrayView", additionally @ref BitArray instances are also implicitly convertible to it. The only difference is due to the owning
aspect --- mutable access to the data is provided only via non @cpp const @ce
overloads.

@attention Consistently with @ref BitArrayView, because the size represents
    bits and because the class additionally has to store initial offset in the
    first byte, on 32-bit systems the size is limited to 512M bits --- i.e.,
    64 MB of memory.

@subsection Containers-BitArray-usage-initialization Bit array initialization

The following explicit initialization constructors are provided, similarly to
the @ref Array class:

-   @ref BitArray(ValueInitT, std::size_t) zero-initializes the array. This is
    equivalent to @cpp new char[(size + 7)/8]{} @ce.
-   @ref BitArray(DirectInitT, std::size_t, bool) fills the whole array with
    given bit value. This is equivalent to @cpp new char[(size + 7)/8]{value*'\xff', value*'\xff', …} @ce.
-   @ref BitArray(NoInitT, std::size_t) keeps the contents uninitialized.
    Useful when you'll be overwriting the contents anyway. Equivalent to
    @cpp new char[(size + 7)/8] @ce.

Unlike an @ref Array, there's no @ref DefaultInitT constructor, as the same
behavior is already provided by @ref BitArray(NoInitT, std::size_t).

@snippet Containers.cpp BitArray-usage-initialization

<b></b>

@m_class{m-note m-success}

@par Aligned allocations
    Please note that @ref BitArray allocations are by default only aligned to
    @cpp 2*sizeof(void*) @ce. If you need overaligned memory for working with
    SIMD types, use @ref Utility::allocateAligned() instead.

@subsection Containers-BitArray-usage-wrapping Wrapping externally allocated bit arrays

Similarly to @ref Array, by default the class makes all allocations using
@cpp operator new[] @ce and deallocates using @cpp operator delete[] @ce. It's
however also possible to wrap an externally allocated array using
@ref BitArray(void*, std::size_t, std::size_t, Deleter) together with
specifying which function to use for deallocation. In addition to a size in
bits, there's also an offset argument specifying the starting bit in the fist
byte, consistently with a @ref BasicBitArrayView "BitArrayView". Often it will
be @cpp 0 @ce however.

For example, properly deallocating a bit array allocated using @ref std::malloc():

@snippet Containers.cpp BitArray-usage-wrapping

@see @ref EnumSet, @ref BigEnumSet
*/
class CORRADE_UTILITY_EXPORT BitArray {
    public:
        typedef void(*Deleter)(char*, std::size_t); /**< @brief Deleter type */

        /**
         * @brief Default constructor
         *
         * Creates a zero-sized array. Move a @ref BitArray with a nonzero size
         * onto the instance to make it useful.
         */
        /*implicit*/ BitArray(std::nullptr_t = nullptr) noexcept: _data{}, _sizeOffset{}, _deleter{} {}

        /**
         * @brief Construct a zero-initialized array
         * @param size      Size in bits
         *
         * If the size is zero, no allocation is done.
         * @see @relativeref{Corrade,ValueInit},
         *      @ref BitArray(NoInitT, std::size_t),
         *      @ref BitArray(DirectInitT, std::size_t, bool)
         */
        explicit BitArray(Corrade::ValueInitT, std::size_t size);

        /**
         * @brief Construct an array without initializing its contents
         * @param size      Size in bits
         *
         * The contents are *not* initialized. If the size is zero, no
         * allocation is done. Useful if you will be overwriting all elements
         * later anyway.
         * @see @relativeref{Corrade,NoInit},
         *      @ref BitArray(ValueInitT, std::size_t),
         *      @ref BitArray(DirectInitT, std::size_t, bool)
         */
        explicit BitArray(Corrade::NoInitT, std::size_t size);

        /**
         * @brief Construct an array initialized to a particular bit value
         * @param size      Size in bits
         * @param value     Bit value
         *
         * If the size is zero, no allocation is done.
         * @see @relativeref{Corrade,NoInit},
         *      @ref BitArray(ValueInitT, std::size_t),
         *      @ref BitArray(NoInitT, std::size_t)
         */
        explicit BitArray(Corrade::DirectInitT, std::size_t size, bool value);

        /**
         * @brief Take ownership of an external bit array
         * @param data      Data
         * @param offset    Initial bit offset in @p data. Expected to be less
         *      than 8.
         * @param size      Size in bits, excluding @p offset
         * @param deleter   Deleter. Use @cpp nullptr @ce for the standard
         *      @cpp delete[] @ce.
         *
         * The @p deleter will be *unconditionally* called on destruction with
         * @p data and @cpp (offset + size + 7)/8 @ce (i.e., size including the
         * initial offset in bytes) as an argument. In particular, it will be
         * also called if @p data is @cpp nullptr @ce or @p size is @cpp 0 @ce.
         *
         * In case of a moved-out instance, the deleter gets reset to a
         * default-constructed value alongside the array pointer and size. It
         * effectively means @cpp delete[] nullptr @ce gets called when
         * destructing a moved-out instance (which is a no-op).
         * @see @ref Containers-BitArray-usage-wrapping
         */
        explicit BitArray(void* data, std::size_t offset, std::size_t size, Deleter deleter) noexcept;

        /* No ownership-taking overload taking const void* at the moment, as
           (unlike with a String) I don't see a need for such a footgun. */

        /** @brief Copying is not allowed */
        BitArray(const BitArray&) = delete;

        /**
         * @brief Move constructor
         *
         * Resets data pointer, offset, size and deleter of @p other to be
         * equivalent to a default-constructed instance.
         */
        BitArray(BitArray&& other) noexcept;

        /**
         * @brief Destructor
         *
         * Calls @ref deleter() on the owned @ref data().
         */
        ~BitArray();

        /** @brief Copying is not allowed */
        BitArray& operator=(const BitArray&) = delete;

        /**
         * @brief Move assignment
         *
         * Swaps data pointer, offset, size and deleter of the two instances.
         */
        BitArray& operator=(BitArray&& other) noexcept;

        /** @brief Conversion to a view */
        /*implicit*/ operator MutableBitArrayView();
        /*implicit*/ operator BitArrayView() const; /**< @overload */

        /* No bool conversion operator right now, as it's yet unclear what
           semantic should it have -- return false if it's nullptr, if the size
           is zero (or both?). or if all bits are false? */

        /**
         * @brief Array data
         *
         * Use @ref offset() to get location of the first bit pointed to by the
         * array.
         */
        char* data() { return _data; }
        const char* data() const { return _data; } /**< @overload */

        /**
         * @brief Array deleter
         *
         * If set to @cpp nullptr @ce, the contents are deleted using standard
         * @cpp operator delete[] @ce.
         * @see @ref BitArray(void*, std::size_t, std::size_t, Deleter)
         */
        Deleter deleter() const { return _deleter; }

        /**
         * @brief Offset in the first byte
         *
         * The returned value is always less than @cpp 8 @ce, and is non-zero
         * only if the array was created with a non-zero @p offset passed to
         * the @ref BitArray(void*, std::size_t, std::size_t, Deleter)
         * constructor.
         * @see @ref size()
         */
        std::size_t offset() const { return _sizeOffset & 0x07; }

        /**
         * @brief Size in bits
         *
         * @see @ref isEmpty(), @ref offset()
         */
        std::size_t size() const { return _sizeOffset >> 3; }

        /**
         * @brief Whether the array is empty
         *
         * @see @ref size()
         */
        bool isEmpty() const { return !(_sizeOffset >> 3); }

        /**
         * @brief Bit at given position
         *
         * Expects that @p i is less than @ref size(). Use @ref set(std::size_t),
         * @ref reset(std::size_t) or @ref set(std::size_t, bool) to set a bit
         * value.
         */
        bool operator[](std::size_t i) const;

        /**
         * @brief Set a bit at given position
         *
         * Expects that @p i is less than @ref size().
         * @see @ref operator[]()
         */
        void set(std::size_t i);

        /**
         * @brief Reset a bit at given position
         *
         * Expects that @p i is less than @ref size().
         * @see @ref operator[]()
         */
        void reset(std::size_t i);

        /**
         * @brief Set or reset a bit at given position
         *
         * Expects that @p i is less than @ref size(). For a @p value known at
         * compile time, explicitly calling either @ref set(std::size_t) or
         * @ref reset(std::size_t) is more efficient.
         * @see @ref operator[]()
         */
        void set(std::size_t i, bool value);

        /**
         * @brief View on a slice
         *
         * Equivalent to @ref BasicBitArrayView::slice().
         */
        MutableBitArrayView slice(std::size_t begin, std::size_t end);
        BitArrayView slice(std::size_t begin, std::size_t end) const; /**< @overload */

        /**
         * @brief View on a slice of given size
         *
         * Equivalent to @ref BasicBitArrayView::sliceSize().
         */
        MutableBitArrayView sliceSize(std::size_t begin, std::size_t size);
        BitArrayView sliceSize(std::size_t begin, std::size_t size) const; /**< @overload */

        /**
         * @brief View on the first @p size bits
         *
         * Equivalent to @ref BasicBitArrayView::prefix().
         */
        MutableBitArrayView prefix(std::size_t size);
        BitArrayView prefix(std::size_t size) const; /**< @overload */

        /**
         * @brief View on the last @p size bits
         *
         * Equivalent to @ref BasicBitArrayView::suffix().
         */
        MutableBitArrayView suffix(std::size_t size);
        BitArrayView suffix(std::size_t size) const; /**< @overload */

        /**
         * @brief View except the first @p size bits
         *
         * Equivalent to @ref BasicBitArrayView::exceptPrefix().
         */
        MutableBitArrayView exceptPrefix(std::size_t size);
        BitArrayView exceptPrefix(std::size_t size) const; /**< @overload */

        /**
         * @brief View except the last @p size bits
         *
         * Equivalent to @ref BasicBitArrayView::exceptSuffix().
         */
        MutableBitArrayView exceptSuffix(std::size_t size);
        BitArrayView exceptSuffix(std::size_t size) const; /**< @overload */

        /**
         * @brief Release data storage
         *
         * Returns the data pointer and resets data pointer, offset, size and
         * deleter to be equivalent to a default-constructed instance. Deleting
         * the returned array is user responsibility --- note the array might
         * have a custom @ref deleter() and so @cpp delete[] @ce might not be
         * always appropriate.
         */
        char* release();

    private:
        char* _data;
        /* The low 3 bits are bit offset in the _data, the rest is size in
           bits. On 32bit systems this means the view can only address 512M
           bits (64 MB of memory). While the offset is used only by the
           wrapping constructor and thus could theoretically be omitted, it
           makes the restrictions match BitArrayView. If it wouldn't match,
           slicing would be impossible for large arrays. */
        std::size_t _sizeOffset;
        void(*_deleter)(char*, std::size_t);
};

/**
@debugoperator{BitArray}

Equivalent to @ref BitArrayView::operator<<(Utility::Debug&, BitArrayView) "operator<<(Utility::Debug&, BitArrayView)",
see its documentation for more information.
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, const BitArray& value);

inline BitArray::BitArray(BitArray&& other) noexcept: _data{other._data}, _sizeOffset{other._sizeOffset}, _deleter{other._deleter} {
    other._data = nullptr;
    other._sizeOffset = 0;
    other._deleter = {};
}

inline BitArray& BitArray::operator=(BitArray&& other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_sizeOffset, other._sizeOffset);
    swap(_deleter, other._deleter);
    return *this;
}

inline bool BitArray::operator[](std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArray::operator[](): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", {});
    return _data[((_sizeOffset & 0x07) + i) >> 3] & (1 << ((_sizeOffset + i) & 0x7));
}

inline void BitArray::set(std::size_t i) {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArray::set(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    _data[((_sizeOffset & 0x07) + i) >> 3] |= (1 << ((_sizeOffset + i) & 0x07));
}

inline void BitArray::reset(std::size_t i) {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArray::reset(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    _data[((_sizeOffset & 0x07) + i) >> 3] &= ~(1 << ((_sizeOffset + i) & 0x07));
}

inline void BitArray::set(std::size_t i, bool value) {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArray::set(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    /* http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching */
    char& byte = _data[((_sizeOffset & 0x07) + i) >> 3];
    byte ^= (-char(value) ^ byte) & (1 << ((_sizeOffset + i) & 0x07));
}

inline char* BitArray::release() {
    char* const data = _data;
    _data = nullptr;
    _sizeOffset = 0;
    _deleter = {};
    return data;
}

}}

#endif
