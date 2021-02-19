#ifndef Corrade_Containers_BigEnumSet_h
#define Corrade_Containers_BigEnumSet_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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
 * @brief Class @ref Corrade::Containers::BigEnumSet
 * @m_since_latest
 *
 * @see @ref Corrade/Containers/BigEnumSet.hpp
 */

#include <cstdint>

#include "Corrade/Containers/EnumSet.h" /* reusing the macros */
#include "Corrade/Containers/sequenceHelpers.h"
#include "Corrade/Utility/Assert.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<class T> constexpr std::uint64_t bigEnumSetElementValue(std::size_t i, T value) {
        return static_cast<typename std::underlying_type<T>::type>(value)/64 == i ? (1ull << (static_cast<typename std::underlying_type<T>::type>(value) % 64)) : 0;
    }
}

/**
@brief Set of more than 64 enum values
@tparam T           Enum type
@tparam size        How many 64-bit integers to use to store the value
@m_since_latest

A variant of @ref EnumSet that is able to handle sets of more than 64 values
(which is the largest standard integer type) by treating the @cpp enum @ce
values as bit positions instead of bit values. Internally an array is used for
storage and the class doesn't provide any equivalent to
@ref EnumSet::operator UnderlyingType().

While it's *theoretically* possible to store up to @f$ 2^{64} @f$ different
values, the storage is artificially limited to 8192 values, which fits into
1 kB. With a 16-bit @p T and larger, you're expected to set the @p size
template parameter to a reasonable upper bound, not larger than @cpp 128 @ce.
On construction, the enum value is checked against this limit to ensure no
bits are ignored by accident.

Below is a side-by-side comparison of an equivalent enum set implementation in
@ref EnumSet and @ref BigEnumSet --- the only difference is enum values being
specified as @cpp i @ce instead of @cpp 1 << i @ce and the @cpp typedef @ce,
the @ref CORRADE_ENUMSET_OPERATORS() / @ref CORRADE_ENUMSET_FRIEND_OPERATORS()
macro is the same:

@m_class{m-row m-container-inflate}

@parblock

@m_div{m-col-m-6}
@snippet Containers.cpp BigEnumSet-usage1
@m_enddiv

@m_div{m-col-m-6}
@snippet Containers.cpp BigEnumSet-usage2
@m_enddiv

@endparblock

@see @ref bigEnumSetDebugOutput()
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T, std::size_t size = (1 << (sizeof(T)*8 - 6))>
#else
template<class T, std::size_t size>
#endif
class BigEnumSet {
    static_assert(std::is_enum<T>::value, "BigEnumSet type must be a strongly typed enum");
    static_assert(size && size <= std::uint64_t{1} << (sizeof(T)*8 - 6), "size out of range for given underlying type");
    static_assert(size <= 128, "BigEnumSet size is capped at 1 kB (8192 different values) to prevent accidents");

    public:
        typedef T Type; /**< @brief Enum type */

        enum: std::size_t {
            Size = size /**< Count of 64-bit integers storing this set */
        };

        /** @brief Create an empty set */
        constexpr /*implicit*/ BigEnumSet() noexcept: _data{} {}

        /** @brief Create a set from one value */
        constexpr /*implicit*/ BigEnumSet(T value) noexcept: BigEnumSet<T, size>{
            (CORRADE_CONSTEXPR_ASSERT(static_cast<typename std::underlying_type<T>::type>(value) < size*64,
                "Containers::BigEnumSet: value" << static_cast<typename std::underlying_type<T>::type>(value) << "too large for a" << size*64 << Utility::Debug::nospace << "-bit storage"
            ), nullptr),
            value, typename Implementation::GenerateSequence<Size>::Type{}} {}

        /**
         * @brief Create an uninitialized set
         *
         * The contents are left in an undefined state.
         */
        explicit BigEnumSet(NoInitT) {}

        /** @brief Equality comparison */
        constexpr bool operator==(const BigEnumSet<T, size>& other) const {
            return equalsInternal(other, typename Implementation::GenerateSequence<Size>::Type{});
        }

        /** @brief Non-equality comparison */
        constexpr bool operator!=(const BigEnumSet<T, size>& other) const {
            return !operator==(other);
        }

        /**
         * @brief Stored data
         *
         * Returns an array of size @ref Size.
         */
        constexpr const std::uint64_t* data() const { return +_data; }

        /**
         * @brief Whether @p other is a subset of this (@f$ a \supseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == other @ce.
         */
        constexpr bool operator>=(const BigEnumSet<T, size>& other) const {
            return (*this & other) == other;
        }

        /**
         * @brief Whether @p other is a superset of this (@f$ a \subseteq o @f$)
         *
         * Equivalent to @cpp (a & other) == a @ce.
         */
        constexpr bool operator<=(const BigEnumSet<T, size>& other) const {
            return (*this & other) == *this;
        }

        /** @brief Union of two sets */
        constexpr BigEnumSet<T, size> operator|(const BigEnumSet<T, size>& other) const {
            return orInternal(other, typename Implementation::GenerateSequence<Size>::Type{});
        }

        /** @brief Union two sets and assign */
        BigEnumSet<T, size>& operator|=(const BigEnumSet<T, size>& other) {
            for(std::size_t i = 0; i != Size; ++i)
                _data[i] |= other._data[i];
            return *this;
        }

        /** @brief Intersection of two sets */
        constexpr BigEnumSet<T, size> operator&(const BigEnumSet<T, size>& other) const {
            return andInternal(other, typename Implementation::GenerateSequence<Size>::Type{});
        }

        /** @brief Intersect two sets and assign */
        BigEnumSet<T, size>& operator&=(const BigEnumSet<T, size>& other) {
            for(std::size_t i = 0; i != Size; ++i)
                _data[i] &= other._data[i];
            return *this;
        }

        /** @brief XOR of two sets */
        constexpr BigEnumSet<T, size> operator^(const BigEnumSet<T, size>& other) const {
            return xorInternal(other, typename Implementation::GenerateSequence<Size>::Type{});
        }

        /** @brief XOR two sets and assign */
        BigEnumSet<T, size>& operator^=(const BigEnumSet<T, size>& other) {
            for(std::size_t i = 0; i != Size; ++i)
                _data[i] ^= other._data[i];
            return *this;
        }

        /** @brief Set complement */
        constexpr BigEnumSet<T, size> operator~() const {
            return inverseInternal(typename Implementation::GenerateSequence<Size>::Type{});
        }

        /**
         * @brief Boolean conversion
         *
         * Returns @cpp true @ce if at least one bit is set, @cpp false @ce
         * otherwise.
         */
        constexpr explicit operator bool() const {
            return nonZeroInternal(typename Implementation::GenerateSequence<Size>::Type{});
        }

    private:
        /* Used by the BigEnumSet(T) constructor, void* to avoid accidental
           matches by users */
        template<std::size_t ...sequence> constexpr explicit BigEnumSet(void*, T value, Implementation::Sequence<sequence...>) noexcept: _data{Implementation::bigEnumSetElementValue(sequence, value)...} {}

        /* Used by the *Internal() functions below, void* to avoid accidental
           matches by users */
        template<class First, class ...Next> constexpr explicit BigEnumSet(void*, First first, Next... next) noexcept: _data{first, next...} {
            static_assert(sizeof...(Next) + 1 == Size, "improper value count for construction");
        }

        constexpr bool nonZeroInternal(Implementation::Sequence<>) const {
            return false;
        }

        template<std::size_t first, std::size_t ...next> constexpr bool nonZeroInternal(Implementation::Sequence<first, next...>) const {
            return _data[first] || nonZeroInternal(Implementation::Sequence<next...>{});
        }

        constexpr bool equalsInternal(const BigEnumSet<T, size>&, Implementation::Sequence<>) const {
            return true;
        }

        template<std::size_t first, std::size_t ...next> constexpr bool equalsInternal(const BigEnumSet<T, size>& other, Implementation::Sequence<first, next...>) const {
            return _data[first] == other._data[first] && equalsInternal(other, Implementation::Sequence<next...>{});
        }

        template<std::size_t ...sequence> constexpr BigEnumSet<T, size> orInternal(const BigEnumSet<T, size>& other, Implementation::Sequence<sequence...>) const {
            return BigEnumSet<T, size>{nullptr, (_data[sequence] | other._data[sequence])...};
        }

        template<std::size_t ...sequence> constexpr BigEnumSet<T, size> andInternal(const BigEnumSet<T, size>& other, Implementation::Sequence<sequence...>) const {
            return BigEnumSet<T, size>{nullptr, (_data[sequence] & other._data[sequence])...};
        }

        template<std::size_t ...sequence> constexpr BigEnumSet<T, size> xorInternal(const BigEnumSet<T, size>& other, Implementation::Sequence<sequence...>) const {
            return BigEnumSet<T, size>{nullptr, (_data[sequence] ^ other._data[sequence])...};
        }

        template<std::size_t ...sequence> constexpr BigEnumSet<T, size> inverseInternal(Implementation::Sequence<sequence...>) const {
            return BigEnumSet<T, size>{nullptr, ~_data[sequence]...};
        }

        std::uint64_t _data[Size];
};

}}

#endif
