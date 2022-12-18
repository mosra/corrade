#ifndef Corrade_Containers_StridedDimensions_h
#define Corrade_Containers_StridedDimensions_h
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
 * @brief Class @ref Corrade::Containers::StridedDimensions, alias @ref Corrade::Containers::Size, @ref Corrade::Containers::Stride, typedef @ref Corrade::Containers::Size1D, @ref Corrade::Containers::Size2D, @ref Corrade::Containers::Size3D, @ref Corrade::Containers::Size4D, @ref Corrade::Containers::Stride1D, @ref Corrade::Containers::Stride2D, @ref Corrade::Containers::Stride3D, @ref Corrade::Containers::Stride4D
 * @m_since_latest
 */

#include "Corrade/Tags.h"
#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/sequenceHelpers.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Move.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    template<unsigned, class, class> struct StridedDimensionsConverter;
    /* Used by StridedArrayView, needed here to friend them */
    template<unsigned, class> struct StridedElement;
    template<int> struct ArrayCastFlattenOrInflate;
}

/**
@brief Multi-dimensional size and stride for @ref StridedArrayView
@m_since{2019,10}

Main property compared to a plain C array of value is convertibility from/to
@ref StaticArrayView, implicit conversion from/to a scalar type in the
one-dimensional case and element-wise equality comparison. See
@ref StridedArrayView for actual usage examples.
@see @ref Size, @ref Size1D, @ref Size2D, @ref Size3D, @ref Size4D,
    @ref Stride, @ref Stride1D, @ref Stride2D, @ref Stride3D, @ref Stride4D
*/
template<unsigned dimensions, class T> class StridedDimensions {
    public:
        /**
         * @brief Default constructor
         *
         * Equivalent to @ref StridedDimensions(ValueInitT).
         */
        constexpr /*implicit*/ StridedDimensions() noexcept: _data{} {}

        /** @brief Construct with zero-initialized data */
        constexpr explicit StridedDimensions(Corrade::ValueInitT) noexcept: _data{} {}

        /** @brief Construct without initializing the contents */
        explicit StridedDimensions(Corrade::NoInitT) noexcept {}

        /** @brief Constructor */
        template<class ...Args> constexpr /*implicit*/ StridedDimensions(T first, Args... next) noexcept: _data{T(first), T(next)...} {
            static_assert(sizeof...(Args) + 1 == dimensions, "wrong value count");
        }

        /** @brief Construct from an array */
        constexpr /*implicit*/ StridedDimensions(const T(&values)[dimensions]) noexcept: StridedDimensions{values, typename Implementation::GenerateSequence<dimensions>::Type{}} {}

        /**
         * @brief Construct from an external representation
         * @m_since_latest
         *
         * Conversion from a @ref StaticArrayView of the same type and
         * dimension count is builtin.
         */
        template<class U, class = decltype(Implementation::StridedDimensionsConverter<dimensions, T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))> constexpr /*implicit*/ StridedDimensions(U&& other) noexcept: StridedDimensions{Implementation::StridedDimensionsConverter<dimensions, T, typename std::decay<U&&>::type>::from(Utility::forward<U>(other))} {}

        /**
         * @brief Convert to an external representation
         * @m_since_latest
         *
         * Conversion to a @ref StaticArrayView of the same type and dimension
         * count is builtin.
         */
        template<class U, class = decltype(Implementation::StridedDimensionsConverter<dimensions, T, U>::to(std::declval<StridedDimensions<dimensions, T>>()))> constexpr /*implicit*/ operator U() const noexcept {
            return Implementation::StridedDimensionsConverter<dimensions, T, U>::to(*this);
        }

        /**
         * @brief Conversion to a scalar
         *
         * Enabled only for the one-dimensional case.
         */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        template<unsigned d = dimensions, class = typename std::enable_if<d == 1>::type>
        #endif
        constexpr /*implicit*/ operator T() const { return _data[0]; }

        /** @brief Equality comparison */
        bool operator==(const StridedDimensions<dimensions, T>& other) const {
            for(std::size_t i = 0; i != dimensions; ++i)
                if(_data[i] != other._data[i]) return false;
            return true;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const StridedDimensions<dimensions, T>& other) const {
            return !operator==(other);
        }

        /** @brief Element access */
        constexpr T operator[](std::size_t i) const {
            return CORRADE_CONSTEXPR_DEBUG_ASSERT(i < dimensions,
                "Containers::StridedDimensions::operator[](): dimension" << i << "out of range for" << dimensions << "dimensions"), _data[i];
        }

        /** @brief Element access */
        T& operator[](std::size_t i) {
            CORRADE_DEBUG_ASSERT(i < dimensions,
                "Containers::StridedDimensions::operator[](): dimension" << i << "out of range for" << dimensions << "dimensions", _data[0]);
            return _data[i];
        }

        /** @brief First element */
        T* begin() { return _data; }

        /** @overload */ /* https://github.com/doxygen/doxygen/issues/7472 */
        constexpr const T* begin() const { return _data; }
        constexpr const T* cbegin() const { return _data; } /**< @overload */

        /** @brief (One item after) last element */
        T* end() { return _data + dimensions; }

        /** @overload */ /* https://github.com/doxygen/doxygen/issues/7472 */
        constexpr const T* end() const { return _data + dimensions; }
        constexpr const T* cend() const { return _data + dimensions; } /**< @overload */

    private:
        template<unsigned, class> friend class StridedArrayView;
        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<unsigned, class> friend struct Implementation::StridedElement;
        template<int> friend struct Implementation::ArrayCastFlattenOrInflate;
        template<class U, unsigned dimensions_, class T_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, T_>&);
        template<class U, unsigned dimensions_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, void>&);
        template<class U, unsigned dimensions_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, const void>&);
        template<unsigned newDimensions, class U, unsigned dimensions_> StridedArrayView<newDimensions, U> friend arrayCast(const StridedArrayView<dimensions_, void>&, std::size_t);
        template<unsigned newDimensions, class U, unsigned dimensions_> StridedArrayView<newDimensions, U> friend arrayCast(const StridedArrayView<dimensions_, const void>&, std::size_t);

        template<class U, std::size_t ...sequence> constexpr explicit StridedDimensions(const U* values, Implementation::Sequence<sequence...>) noexcept: _data{T(values[sequence])...} {}

        T _data[dimensions];
};

namespace Implementation {

template<unsigned dimensions, class T> struct StridedDimensionsConverter<dimensions, T, StaticArrayView<std::size_t(dimensions), const T>> {
    constexpr static StridedDimensions<dimensions, T> from(StaticArrayView<dimensions, const T> view) {
        return fromInternal(view.data(), typename GenerateSequence<dimensions>::Type{});
    }
    constexpr static StaticArrayView<dimensions, const T> to(const StridedDimensions<dimensions, T>& dimensions_) {
        return StaticArrayView<dimensions, const T>{dimensions_.begin()};
    }
    template<std::size_t ...sequence> constexpr static StridedDimensions<dimensions, T> fromInternal(const T* data, Sequence<sequence...>) {
        return {data[sequence]...};
    }
};

}

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief Multi-dimensional size for @ref StridedArrayView
@m_since_latest

@see @ref Size1D, @ref Size2D, @ref Size3D, @ref Size4D, @ref Stride
*/
template<unsigned dimensions> using Size = StridedDimensions<dimensions, std::size_t>;
#endif

/**
@brief Size for @ref StridedArrayView1D
@m_since_latest

@see @ref Size2D, @ref Size3D, @ref Size4D, @ref Stride1D
*/
typedef Size<1> Size1D;

/**
@brief Size for @ref StridedArrayView2D
@m_since_latest

@see @ref Size1D, @ref Size3D, @ref Size4D, @ref Stride2D
*/
typedef Size<2> Size2D;

/**
@brief Size for @ref StridedArrayView3D
@m_since_latest

@see @ref Size1D, @ref Size2D, @ref Size4D, @ref Stride3D
*/
typedef Size<3> Size3D;

/**
@brief Size for @ref StridedArrayView4D
@m_since_latest

@see @ref Size1D, @ref Size2D, @ref Size3D, @ref Stride4D
*/
typedef Size<4> Size4D;

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief Multi-dimensional stride for @ref StridedArrayView
@m_since_latest

@see @ref Stride1D, @ref Size2D, @ref Size3D, @ref Size4D, @ref Stride
*/
template<unsigned dimensions> using Stride = StridedDimensions<dimensions, std::ptrdiff_t>;
#endif

/**
@brief Stride for @ref StridedArrayView1D
@m_since_latest

@see @ref Stride2D, @ref Stride3D, @ref Stride4D, @ref Size1D
*/
typedef Stride<1> Stride1D;

/**
@brief Stride for @ref StridedArrayView2D
@m_since_latest

@see @ref Stride1D, @ref Stride3D, @ref Stride4D, @ref Size2D
*/
typedef Stride<2> Stride2D;

/**
@brief Stride for @ref StridedArrayView3D
@m_since_latest

@see @ref Stride1D, @ref Stride2D, @ref Stride4D, @ref Size3D
*/
typedef Stride<3> Stride3D;

/**
@brief Stride for @ref StridedArrayView4D
@m_since_latest

@see @ref Stride1D, @ref Stride2D, @ref Stride3D, @ref Size4D
*/
typedef Stride<4> Stride4D;

}}

#endif
