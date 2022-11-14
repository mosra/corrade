#ifndef Corrade_Containers_BitArrayView_h
#define Corrade_Containers_BitArrayView_h
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
 * @brief Class @ref Corrade::Containers::BasicBitArrayView, typedef @ref Corrade::Containers::BitArrayView, @ref Corrade::Containers::MutableBitArrayView
 * @m_since_latest
 */

#include <cstddef> /* std::nullptr_t, std::size_t */
#include <type_traits>

#include "Corrade/Tags.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Utility.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    /* So ArrayTuple can update the data pointer */
    template<class T>
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* warns that "the inline specifier cannot be used when a friend
           declaration refers to a specialization of a function template" due
           to friend dataRef<>() being used below. AMAZING */
        inline
        #endif
    T*& dataRef(BasicBitArrayView<T>& view) {
        return view._data;
    }
}

/**
@brief Base for bit array views
@m_since_latest

@m_keywords{BitArrayView MutableBitArrayView}

A view over an arbitrary range of bits, including sub-byte offsets. An owning
version of this containers is a @ref BitArray.

@section Containers-BasicBitArrayView-usage Usage

The class is implicitly convertible from @ref BitArray instances, it's also
possible to implicitly create a @cpp const @ce view on a mutable array. Besides
that, a view can be created manually by specifying a pointer, initial bit
offset and bit count:

@snippet Containers.cpp BitArrayView-usage

@attention Because the size represents bits and because the class additionally
    has to store initial offset in the first byte, on 32-bit systems the size
    is limited to 512M bits --- i.e., 64 MB of memory.

@subsection Containers-BasicBitArrayView-usage-access Data access

Only a small subset of the usual @ref ArrayView access interface is provided
--- @ref size(), @ref isEmpty() and querying particular bits using
@ref operator[](). Unlike e.g. @ref std::vector<bool>, there's no array
subscript operator for *setting* bits as it would have to create a temporary
setter object each time a bit is set, causing unnecessary overhead. Instead,
you're supposed to use @ref set(std::size_t) const,
@ref reset(std::size_t) const or @ref set(std::size_t, bool) const. For a
similar reason, there's no iterator or range-for access.

There's also @ref data() for raw data access, but because the view can point
to an arbitrary bit in the first byte, you're expected to take also
@ref offset() into account when accessing the data.

@subsection Containers-BasicBitArrayView-usage-slicing View slicing

Slicing functions match the @ref ArrayView interface --- @ref slice(),
@ref sliceSize(), @ref prefix(), @ref suffix(), @ref exceptPrefix() and
@ref exceptSuffix(), all operating with bit offsets. No pointer-taking
overloads are provided as byte-level slicing would be too coarse.
*/
/* All member functions are const because the view doesn't own the data */
template<class T> class BasicBitArrayView {
    public:
        /** @brief Default constructor */
        constexpr /*implicit*/ BasicBitArrayView(std::nullptr_t = nullptr) noexcept: _data{}, _sizeOffset{} {}

        /**
         * @brief Constructor
         * @param data      Data pointer
         * @param offset    Bit offset in @p data
         * @param size      Bit count
         *
         * The @p offset is expected to be less than 8, @p size has to fit into
         * 29 bits on 32-bit platforms and 61 bits on 64-bit platforms.
         */
        /*implicit*/ BasicBitArrayView(typename std::conditional<std::is_const<T>::value, const void, void>::type* data, std::size_t offset, std::size_t size) noexcept: BasicBitArrayView<T>{static_cast<T*>(data), offset, size} {}

        /**
         * @brief Constexpr constructor
         *
         * A variant of @ref BasicBitArrayView(typename std::conditional<std::is_const<T>::value, const void, void>::type*, std::size_t, std::size_t)
         * usable in a @cpp constexpr @ce context --- in order to satisfy the
         * restrictions, the @p data parameter has to be (@cpp const @ce)
         * @cpp char* @ce.
         */
        constexpr /*implicit*/ BasicBitArrayView(T* data, std::size_t offset, std::size_t size) noexcept;

        /** @overload */
        constexpr /*implicit*/ BasicBitArrayView(std::nullptr_t, std::size_t offset, std::size_t size) noexcept: BasicBitArrayView{static_cast<T*>(nullptr), offset, size} {}

        /** @brief Construct a @ref BitArrayView from a @ref MutableBitArrayView */
        template<class U, class = typename std::enable_if<std::is_same<const U, T>::value>::type> constexpr /*implicit*/ BasicBitArrayView(BasicBitArrayView<U> mutable_) noexcept: _data{mutable_._data}, _sizeOffset{mutable_._sizeOffset} {}

        /* No bool conversion operator right now, as it's yet unclear what
           semantic should it have -- return false if it's nullptr, if the size
           is zero (or both?). or if all bits are false? */

        /**
         * @brief Array data
         *
         * Use @p offset() to get location of the first bit pointed to by the
         * array.
         */
        constexpr T* data() const { return _data; }

        /**
         * @brief Offset in the first byte
         *
         * The returned value is always less than 8, and is non-zero only if
         * the array was created with a non-zero @p offset passed to the
         * @ref BasicBitArrayView(typename std::conditional<std::is_const<T>::value, const void, void>::type*, std::size_t, std::size_t)
         * constructor.
         */
        constexpr std::size_t offset() const { return _sizeOffset & 0x07; }

        /**
         * @brief Size in bits
         *
         * @see @ref isEmpty(), @ref offset()
         */
        constexpr std::size_t size() const { return _sizeOffset >> 3; }

        /**
         * @brief Whether the view is empty
         *
         * @see @ref size()
         */
        constexpr bool isEmpty() const { return !(_sizeOffset >> 3); }

        /**
         * @brief Bit at given position
         *
         * Expects that @p i is less than @ref size(). Use
         * @ref set(std::size_t) const, @ref reset(std::size_t) const or
         * @ref set(std::size_t, bool) const to set a bit value.
         */
        constexpr bool operator[](std::size_t i) const;

        /**
         * @brief Set a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * @ref MutableBitArrayView.
         * @see @ref operator[]()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        void set(std::size_t i) const;
        #else
        template<class U = T> typename std::enable_if<!std::is_const<U>::value>::type set(std::size_t i) const;
        #endif

        /**
         * @brief Reset a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * @ref MutableBitArrayView.
         * @see @ref operator[]()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        void reset(std::size_t i) const;
        #else
        template<class U = T> typename std::enable_if<!std::is_const<U>::value>::type reset(std::size_t i) const;
        #endif

        /**
         * @brief Set or reset a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * @ref MutableBitArrayView. For a @p value known at compile time,
         * explicitly calling either @ref set(std::size_t) const or
         * @ref reset(std::size_t) const is a simpler operation.
         * @see @ref operator[]()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        void set(std::size_t i, bool value) const;
        #else
        template<class U = T> typename std::enable_if<!std::is_const<U>::value>::type set(std::size_t i, bool value) const;
        #endif

        /**
         * @brief View slice
         *
         * Both arguments are expected to be less than @p size().
         * @see @ref sliceSize(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix()
         */
        constexpr BasicBitArrayView<T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief View slice of given size
         *
         * Equivalent to @cpp data.slice(begin, begin + size) @ce.
         * @see @ref slice(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix()
         */
        constexpr BasicBitArrayView<T> sliceSize(std::size_t begin, std::size_t size) const {
            return slice(begin, begin + size);
        }

        /**
         * @brief View on the first @p size bits
         *
         * Equivalent to @cpp data.slice(0, size) @ce.
         * @see @ref slice(), @ref sliceSize(), @ref exceptPrefix(),
         *      @ref suffix()
         */
        constexpr BasicBitArrayView<T> prefix(std::size_t size) const {
            return slice(0, size);
        }

        /**
         * @brief View on the last @p size bits
         *
         * Equivalent to @cpp data.slice(data.size() - size, data.size()) @ce.
         * @see @ref slice(), @ref sliceSize(), @ref exceptSuffix(),
         *      @ref prefix()
         */
        constexpr BasicBitArrayView<T> suffix(std::size_t size) const {
            return slice((_sizeOffset >> 3) - size, _sizeOffset >> 3);
        }

        /**
         * @brief View except the first @p size bits
         *
         * Equivalent to @cpp data.slice(size, data.size()) @ce.
         * @see @ref slice(), @ref sliceSize(), @ref prefix(),
         *      @ref exceptSuffix()
         */
        constexpr BasicBitArrayView<T> exceptPrefix(std::size_t size) const {
            return slice(size, _sizeOffset >> 3);
        }

        /**
         * @brief View except the last @p size bits
         *
         * Equivalent to @cpp data.slice(0, data.size() - size) @ce.
         * @see @ref slice(), @ref sliceSize(), @ref suffix(),
         *      @ref exceptPrefix()
         */
        constexpr BasicBitArrayView<T> exceptSuffix(std::size_t size) const {
            return slice(0, (_sizeOffset >> 3) - size);
        }

    private:
        /* Needed for mutable/immutable conversion */
        template<class> friend class BasicBitArrayView;
        friend BitArray;
        /* So ArrayTuple can update the data pointer */
        friend T*& Implementation::dataRef<>(BasicBitArrayView<T>&);

        /* Used by BitArray view conversion to avoid going through the
           asserts and value decomposition in the public constructor. */
        constexpr explicit BasicBitArrayView(T* const data, std::size_t sizeOffset): _data{data}, _sizeOffset{sizeOffset} {}

        T* _data;
        /* The low 3 bits are bit offset in the _data, the rest is size in
           bits. On 32bit systems this means the view can only address 512M
           bits (64 MB of memory). */
        std::size_t _sizeOffset;
};

/**
@brief Bit array view
@m_since_latest

Immutable, use @ref MutableBitArrayView for mutable access.
*/
typedef BasicBitArrayView<const char> BitArrayView;

/**
@brief Mutable bit array view
@m_since_latest

@see @ref BitArrayView
*/
typedef BasicBitArrayView<char> MutableBitArrayView;

/**
@debugoperator{BasicBitArrayView}

Prints the value as @cb{.shell-session} {a, b, …} @ce, with each element being
the next 8 bits from the array. To have a monotonic order, the first character
in each element is the first bit in the 8-bit group --- i.e., the order is
reversed compared to binary literals.

For example, the following corresponds to a 64-bit value of
@cpp 0b111'11001100'11110000'01010101 << 7 @ce stored as Little-Endian, and
printing a view on it will show it in reverse order:

@m_class{m-code-figure}

@parblock

@snippet Containers-cpp14.cpp BitArrayView-operator<<

<b></b>

@m_class{m-nopad}

@code{.shell-session}
{10101010, 00001111, 00110011, 111}
@endcode

@endparblock
*/
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, BitArrayView value);

template<class T> constexpr BasicBitArrayView<T>::BasicBitArrayView(T* const data, const std::size_t offset, const std::size_t size) noexcept: _data{data}, _sizeOffset{
    (CORRADE_CONSTEXPR_DEBUG_ASSERT(offset < 8,
        "Containers::BitArrayView: offset expected to be smaller than 8 bits, got" << offset),
    CORRADE_CONSTEXPR_DEBUG_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 3),
        "Containers::BitArrayView: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size),
    size << 3 | offset)} {}

template<class T> constexpr bool BasicBitArrayView<T>::operator[](std::size_t i) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArrayView::operator[](): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits"),
        _data[((_sizeOffset & 0x07) + i) >> 3] & (1 << ((_sizeOffset + i) & 0x7));
}

template<class T> template<class U> inline typename std::enable_if<!std::is_const<U>::value>::type BasicBitArrayView<T>::set(std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArrayView::set(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    _data[((_sizeOffset & 0x07) + i) >> 3] |= (1 << ((_sizeOffset + i) & 0x07));
}

template<class T> template<class U> inline typename std::enable_if<!std::is_const<U>::value>::type BasicBitArrayView<T>::reset(std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArrayView::reset(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    _data[((_sizeOffset & 0x07) + i) >> 3] &= ~(1 << ((_sizeOffset + i) & 0x07));
}

template<class T> template<class U> inline typename std::enable_if<!std::is_const<U>::value>::type BasicBitArrayView<T>::set(std::size_t i, bool value) const {
    CORRADE_DEBUG_ASSERT(i < (_sizeOffset >> 3),
        "Containers::BitArrayView::set(): index" << i << "out of range for" << (_sizeOffset >> 3) << "bits", );
    /* http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching */
    char& byte = _data[((_sizeOffset & 0x07) + i) >> 3];
    byte ^= (-char(value) ^ byte) & (1 << ((_sizeOffset + i) & 0x07));
}

template<class T> constexpr BasicBitArrayView<T> BasicBitArrayView<T>::slice(const std::size_t begin, const std::size_t end) const {
    return CORRADE_CONSTEXPR_DEBUG_ASSERT(begin <= end && end <= (_sizeOffset >> 3),
            "Containers::BitArrayView::slice(): slice ["
            << Utility::Debug::nospace << begin
            << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end
            << Utility::Debug::nospace << "] out of range for"
            << (_sizeOffset >> 3) << "bits"),
        /* Using an internal assert-less constructor, the public
           constructor asserts would be redundant */
        BasicBitArrayView<T>{_data + (((_sizeOffset & 0x07) + begin) >> 3), ((_sizeOffset + begin) & 0x7) | ((end - begin) << 3)};
}

}}

#endif
