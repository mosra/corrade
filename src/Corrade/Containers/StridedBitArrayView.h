#ifndef Corrade_Containers_StridedBitArrayView_h
#define Corrade_Containers_StridedBitArrayView_h
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
 * @brief Class @ref Corrade::Containers::BasicStridedBitArrayView, alias @ref Corrade::Containers::StridedBitArrayView, @ref Corrade::Containers::MutableStridedBitArrayView, typedef @ref Corrade::Containers::StridedBitArrayView1D, @ref Corrade::Containers::StridedBitArrayView2D, @ref Corrade::Containers::StridedBitArrayView3D, @ref Corrade::Containers::StridedBitArrayView4D, @ref Corrade::Containers::MutableStridedBitArrayView1D, @ref Corrade::Containers::MutableStridedBitArrayView2D, @ref Corrade::Containers::MutableStridedBitArrayView3D, @ref Corrade::Containers::MutableStridedBitArrayView4D
 * @m_since_latest
 */

#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Containers/StridedDimensions.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    /* So ArrayTuple can update the data pointer. Returning a T*& instead of a
       void*& because this also acts as a type disambiguator in the
       constructor, even though it's subsequently cast back to void. */
    template<unsigned dimensions, class T>
        #ifndef CORRADE_MSVC2015_COMPATIBILITY
        /* warns that "the inline specifier cannot be used when a friend
           declaration refers to a specialization of a function template" due
           to friend dataRef<>() being used below. AMAZING */
        inline
        #endif
    T*& dataRef(BasicStridedBitArrayView<dimensions, T>& view) {
        return reinterpret_cast<T*&>(view._data);
    }

    #ifndef CORRADE_NO_DEBUG_ASSERT
    template<unsigned dimensions> constexpr bool isSizeSmallEnoughForBitArrayView(const Size<dimensions>&, Sequence<>) {
        return true;
    }
    template<unsigned dimensions, std::size_t first, std::size_t ...next> constexpr bool isSizeSmallEnoughForBitArrayView(const Size<dimensions>& size, Sequence<first, next...>) {
        return size[first] < std::size_t{1} << (sizeof(std::size_t)*8 - 3) && isSizeSmallEnoughForBitArrayView(size, Sequence<next...>{});
    }
    #endif

    template<std::size_t first, std::size_t ...next> constexpr Size<1 + sizeof...(next)> sizeWithOffset(const Size<1 + sizeof...(next)>& size, std::size_t offset, Implementation::Sequence<first, next...>) {
        return CORRADE_CONSTEXPR_DEBUG_ASSERT(isSizeSmallEnoughForBitArrayView(size, Implementation::Sequence<first, next...>{}),
            "Containers::StridedBitArrayView: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size),
            Size<1 + sizeof...(next)>{(size[first] << 3)|offset, (size[next] << 3)...};
    }

    template<std::size_t ...sequence> constexpr Size<sizeof...(sequence)> sizeWithoutOffset(const Size<sizeof...(sequence)>& size, Implementation::Sequence<sequence...>) {
        return Size<sizeof...(sequence)>{(size[sequence] >> 3)...};
    }
}

/**
@brief Base for strided bit array views
@m_since_latest

@m_keywords{StridedBitArrayView MutableStridedBitArrayView}

A view over an arbitrary multi-dimensional strided range of bits, including
sub-byte offsets --- a bit-sized variant of a @ref StridedArrayView. A
multi-dimensional counterpart to a @ref BasicBitArrayView "BitArrayView".

@section Containers-BasicStridedBitArrayView-usage Usage

The class is meant to be used through either the @ref StridedBitArrayView or
@ref MutableStridedBitArrayView typedefs and their dimension-specific variants.
The class is implicitly convertible from @ref BasicBitArrayView "BitArrayView"
instances, it's also possible to implicitly create a @cpp const @ce view on a
mutable (strided) bit array view. Besides that, a strided view can be created
manually by specifying the original contiguous view, bit count and a stride
between the bits:

@snippet Containers-cpp14.cpp StridedBitArrayView-usage

@attention Similarly to @ref BasicBitArrayView "BitArrayView", because the size
    represents bits and because the class additionally has to store initial
    offset in the first byte, on 32-bit systems the size is limited to 512M
    bits --- i.e., 64 MB of memory.

When constructing, if you don't specify the stride, the constructor assumes
contiguous data and calculates the stride automatically --- stride of a
dimension is stride of the next dimension times next dimension size, while last
dimension stride is implicitly 1 bit. This is especially useful for "reshaping"
linear data as a multi-dimensional view. The following two statements are
equivalent:

@snippet Containers-cpp14.cpp StridedBitArrayView-usage-reshape

A strided bit array view can be also sliced directly from a
@ref StridedArrayView using @relativeref{StridedArrayView,sliceBit()}, see its
documentation for an example.

@subsection Containers-BasicStridedBitArrayView-usage-access Data access

Similarly to @ref BasicBitArrayView "BitArrayView", only a small subset of the
usual @ref StridedArrayView access interface is provided --- @ref size(),
@ref isEmpty() and querying subviews or bits using @ref operator[](). Setting
bits can be done only in one-dimensional views using @ref set(std::size_t) const,
@ref reset(std::size_t) const or @ref set(std::size_t, bool) const.

@subsection Containers-BasicStridedBitArrayView-usage-slicing View slicing and transformation

Slicing functions match the @ref StridedArrayView interface --- @ref slice(),
@ref sliceSize(), @ref prefix(), @ref suffix(), @ref exceptPrefix() and
@ref exceptSuffix(), all operating with bit offsets and with shorthand versions
operating only on the top-level dimension. The @ref every(), @ref transposed(),
@ref flipped() and @ref broadcasted() transformation APIs work equivalently to
their @ref StridedArrayView counterparts.

@see @ref StridedBitArrayView1D, @ref StridedBitArrayView2D,
    @ref StridedBitArrayView3D, @ref StridedBitArrayView4D,
    @ref MutableStridedBitArrayView1D, @ref MutableStridedBitArrayView2D,
    @ref MutableStridedBitArrayView3D, @ref MutableStridedBitArrayView4D
*/
template<unsigned dimensions, class T> class BasicStridedBitArrayView {
    static_assert(dimensions, "can't have a zero-dimensional view");

    public:
        /**
         * @brief Element type
         *
         * For @ref StridedBitArrayView1D / @ref MutableStridedBitArrayView1D
         * is equivalent to @cpp bool @ce, for higher dimensions to a strided
         * view of one dimension less. Compared to
         * @ref StridedArrayView::ElementType the single-dimension type is not
         * a reference as it's not possible to address individual bits.
         * Instead, @ref set() / @ref reset() has to be used to modify the
         * values.
         */
        typedef typename std::conditional<dimensions == 1, bool, BasicStridedBitArrayView<dimensions - 1, T>>::type ElementType;

        /**
         * @brief Erased type
         *
         * Either a @cpp const void @ce for a @ref StridedBitArrayView or a
         * @cpp void @ce for a @ref MutableStridedBitArrayView.
         */
        typedef typename std::conditional<std::is_const<T>::value, const void, void>::type ErasedType;

        enum: unsigned {
            /** View dimensions */
            Dimensions = dimensions
        };

        /**
         * @brief Default constructor
         *
         * Creates an empty view. Copy a non-empty @ref BitArray,
         * @ref BasicBitArrayView "BitArrayView" or
         * @ref BasicStridedBitArrayView "StridedBitArrayView" onto the
         * instance to make it useful.
         */
        constexpr /*implicit*/ BasicStridedBitArrayView(std::nullptr_t = nullptr) noexcept: _data{}, _sizeOffset{}, _stride{} {}

        /**
         * @brief Construct a view with explicit offset, size and stride
         * @param data      Continuous view on the data
         * @param begin     Pointer to the first byte of the strided view
         * @param offset    Offset of the first bit in @p begin
         * @param size      Bit count
         * @param stride    Data stride in bits
         *
         * The @p data view is used only for a bounds check --- expects that
         * it's large enough to fit @p offset, @p size and @p stride in the
         * largest dimension if the stride is either positive or negative. Zero
         * strides unfortunately can't be reliably checked for out-of-bounds
         * conditions, so be extra careful when specifying these. The @p offset
         * is expected to be less than 8 and not less than offset in @p data if
         * @p begin is equal to @cpp data.data() @ce; @p size in each dimension
         * has to fit into 29 bits on 32-bit platforms and 61 bits on 64-bit
         * platforms.
         */
        constexpr /*implicit*/ BasicStridedBitArrayView(BasicBitArrayView<T> data, ErasedType* begin, std::size_t offset, const Size<dimensions>& size, const Stride<dimensions>& stride) noexcept;

        /**
         * @brief Construct a view with explicit size and stride
         *
         * Equivalent to calling @ref BasicStridedBitArrayView(BasicBitArrayView<T>, ErasedType*, std::size_t, const Size<dimensions>&, const Stride<dimensions>&)
         * with @cpp data.data() @ce as @p begin and @cpp data.offset() @ce as
         * @p offset.
         */
        constexpr /*implicit*/ BasicStridedBitArrayView(BasicBitArrayView<T> data, const Size<dimensions>& size, const Stride<dimensions>& stride) noexcept: BasicStridedBitArrayView{data, data.data(), data.offset(), size, stride} {}

        /**
         * @brief Construct a view with explicit size
         *
         * Assuming @p data is contiguous, stride is calculated implicitly
         * from @p size --- stride of a dimension is stride of the next
         * dimension times next dimension size, while last dimension stride is
         * implicitly 1 bit. In a one-dimensional case you probably want to use
         * @ref BasicStridedBitArrayView(BasicBitArrayView<U>) instead.
         */
        constexpr /*implicit*/ BasicStridedBitArrayView(BasicBitArrayView<T> data, const Size<dimensions>& size) noexcept: BasicStridedBitArrayView{data, data.data(), data.offset(), size, Implementation::strideForSize<dimensions>(size._data, 1, typename Implementation::GenerateSequence<dimensions>::Type{})} {}

        /* Unlike with StridedArrayView, the following constructors, compatible
           with BitArrayView, are not defined here:
            BasicStridedBitArrayView(ErasedType*, std::size_t, std::size_t)
            BasicStridedBitArrayView(U(&)[size], std::size_t, std::size_t)
           The reason is that, in case of the second variant, it'd be very
           close to
            BasicStridedBitArrayView(BitArrayView, const Size&, const Stride&)
           in an one-dimensional case --- i.e., the U[] implicitly convertible
           to a BitArrayView, the *bit offset* directly convertible to a 1D
           *size* and the *size* directly convertible to a 1D *stride*. Which
           is an absolutely undesirable footgun, to have the arguments
           interpreted as something completely different by accident. */

        /** @brief Construct a @ref StridedBitArrayView from a @ref MutableStridedBitArrayView */
        template<class U, class = typename std::enable_if<std::is_same<const U, T>::value>::type> constexpr /*implicit*/ BasicStridedBitArrayView(const BasicStridedBitArrayView<dimensions, U>& mutable_) noexcept: _data{mutable_._data}, _sizeOffset{mutable_._sizeOffset}, _stride{mutable_._stride} {}

        /**
         * @brief Construct from a @ref StridedBitArrayView of smaller dimension count
         *
         * The extra dimensions are added at the front, with sizes being
         * @cpp 1 @ce and strides equal to size times stride of @p other in the
         * first dimension. To reduce dimension count you can use
         * @ref operator[](), potentially in combination with @ref transposed().
         */
        template<unsigned lessDimensions
            #ifndef DOXYGEN_GENERATING_OUTPUT
            /* MSVC needs the (), otherwise it gets totally confused and starts
               complaining that "error C2947: expecting '>' to terminate
               template-parameter-list, found '>'". HAHA. (TBH, parsing this is
               a hell.) */
            , class = typename std::enable_if<(lessDimensions < dimensions)>::type
            #endif
        > /*implicit*/ BasicStridedBitArrayView(const BasicStridedBitArrayView<lessDimensions, T>& other) noexcept;

        /**
         * @brief Construct from a @ref BasicBitArrayView
         *
         * Enabled only on one-dimensional views, in case of a
         * @ref MutableStridedBitArrayView only if @p view is also mutable.
         * Stride is implicitly set to 1 bit.
         */
        template<class U
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , unsigned d = dimensions, class = typename std::enable_if<d == 1 && std::is_convertible<U*, T*>::value>::type
            #endif
        > constexpr /*implicit*/ BasicStridedBitArrayView(BasicBitArrayView<U> view) noexcept: _data{view.data()}, _sizeOffset{view.size() << 3 | view.offset()}, _stride{1} {}

        /* No bool conversion operator right now, as it's yet unclear what
           semantic should it have -- return false if it's nullptr, if the size
           is zero (or both?), or if all bits are false? */

        /** @brief Array data */
        constexpr ErasedType* data() const { return _data; }

        /**
         * @brief Bit offset
         *
         * Added to the pointer returned by @ref data() to get location of the
         * first bit. The returned value is always less than 8 and is non-zero
         * if the view was originally constructed with a non-zero offset or if
         * it's a result of slicing that wasn't on a byte boundary.
         */
        constexpr std::size_t offset() const { return _sizeOffset._data[0] & 0x07; }

        /**
         * @brief Array size
         *
         * Returns just @ref std::size_t instead of @ref Size for the
         * one-dimensional case so the usual numeric operations work as
         * expected. Explicitly cast to @ref Size to ensure consistent behavior
         * for all dimensions in generic implementations.
         *
         * Compared to @ref StridedArrayView::size() returns by value and not
         * via @cpp const& @ce, as internally the size is stored together with
         * @ref offset().
         * @see @ref stride(), @ref isEmpty()
         */
        /* However it *has to be* const because otherwise it's impossible to
           easily call view.size()[0] in a constexpr context. C++ you utter
           crap. */
        constexpr typename std::conditional<dimensions == 1, std::size_t, const Size<dimensions>>::type size() const { return sizeInternal(); }

        /**
         * @brief Array stride
         *
         * Returns just @ref std::ptrdiff_t instead of @ref Stride for the
         * one-dimensional case so the usual numeric operations work as
         * expected. Explicitly cast to @ref Stride to ensure consistent
         * behavior for all dimensions in generic implementations.
         * @see @ref size()
         */
        constexpr typename std::conditional<dimensions == 1, std::ptrdiff_t, const Stride<dimensions>&>::type stride() const { return _stride; }

        /**
         * @brief Whether the view is empty
         *
         * @see @ref size()
         */
        constexpr StridedDimensions<dimensions, bool> isEmpty() const {
            return isEmptyInternal(typename Implementation::GenerateSequence<dimensions>::Type{});
        }

        /**
         * @brief Whether the view is contiguous from given dimension further
         *
         * The view is considered contiguous if its last dimension has
         * @ref stride() equal to the type size and every dimension before that
         * until and including @p dimension has its stride equal to element
         * count times stride of the dimension that follows it.
         *
         * Note that even if the data are tightly packed in memory, this
         * function may return @cpp false @ce --- for example, a contiguous
         * view with two dimensions @ref transposed() is no longer contiguous,
         * same as with zero or negative strides.
         * @see @ref asContiguous()
         */
        template<unsigned dimension = 0> bool isContiguous() const;

        /**
         * @brief Convert the view to a contiguous one
         *
         * Returns a view large as the product of sizes in all dimensions.
         * Expects that the view is contiguous.
         * @see @ref isContiguous() const
         */
        BasicBitArrayView<T> asContiguous() const;

        /**
         * @brief Convert the view to a contiguous one from given dimension further
         *
         * Returns a view with the last @p dimension having size as the product
         * of sizes in this and all following dimensions, and stride equal to
         * @cpp sizeof(T) @ce. Expects that @ref isContiguous() "isContiguous<dimension>()"
         * returns @cpp true @ce.
         *
         * Assuming the view is contiguous, calling this function with
         * @p dimension equal to @ref Dimensions minus one will return the same
         * view; calling it with @cpp 0 @ce will return a
         * @ref StridedBitArrayView1D with stride of 1 bit; while the
         * non-templated @ref asContiguous() will return a @ref BitArrayView
         * (where the stride is implicitly defined as 1 bit).
         */
        template<unsigned dimension> BasicStridedBitArrayView<dimension + 1, T> asContiguous() const;

        /**
         * @brief Element at given position
         *
         * Expects that @p i is less than @ref size(). Setting bit values is
         * only possible on mutable single-dimensional views with
         * @ref set(std::size_t) const, @ref reset(std::size_t) const or
         * @ref set(std::size_t, bool) const.
         */
        ElementType operator[](std::size_t i) const;

        /**
         * @brief Set a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * single-dimensional @ref MutableStridedBitArrayView.
         * @see @ref operator[](), @ref reset(),
         *      @ref set(std::size_t, bool) const
         */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        template<class U = T, class = typename std::enable_if<!std::is_const<U>::value && dimensions == 1>::type>
        #endif
        void set(std::size_t i) const;

        /**
         * @brief Reset a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * single-dimensional @ref MutableStridedBitArrayView.
         * @see @ref operator[](), @ref set()
         */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        template<class U = T, class = typename std::enable_if<!std::is_const<U>::value && dimensions == 1>::type>
        #endif
        void reset(std::size_t i) const;

        /**
         * @brief Set or reset a bit at given position
         *
         * Expects that @p i is less than @ref size(). Enabled only on a
         * single-dimensional @ref MutableStridedBitArrayView. For a @p value
         * known at compile time, explicitly calling either
         * @ref set(std::size_t) const or @ref reset(std::size_t) const is a
         * simpler operation.
         * @see @ref operator[]()
         */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        template<class U = T, class = typename std::enable_if<!std::is_const<U>::value && dimensions == 1>::type>
        #endif
        void set(std::size_t i, bool value) const;

        /**
         * @brief View slice in the first dimension
         *
         * Both arguments are expected to be in range. On multi-dimensional
         * views slices just the top-level dimension, use
         * @ref slice(const Size<dimensions>&, const Size<dimensions>&) const
         * to slice in all dimensions.
         * @see @ref sliceSize(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix()
         */
        BasicStridedBitArrayView<dimensions, T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief View slice in all dimensions
         *
         * Values in both arguments are expected to be in range for given
         * dimension.
         * @see @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref slice(std::size_t, std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> slice(const Size<dimensions>& begin, const Size<dimensions>& end) const;

        /**
         * @brief View slice of given size in the first dimension
         *
         * Equivalent to @cpp data.slice(begin, begin + size) @ce. On
         * multi-dimensional views slices just the top-level dimension, use
         * @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const
         * to slice in all dimensions.
         * @see @ref slice(), @ref prefix(), @ref suffix(),
         *      @ref exceptPrefix(), @ref exceptSuffix()
         */
        BasicStridedBitArrayView<dimensions, T> sliceSize(std::size_t begin, std::size_t size) const {
            return slice(begin, begin + size);
        }

        /**
         * @brief View slice of given size in all dimensions
         *
         * Equivalent to @cpp data.slice(begin, end) @ce, where @p end is
         * @cpp begin[i] + size[i] @ce for all dimensions.
         * @see @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref slice(const Size<dimensions>&, const Size<dimensions>&) const
         */
        BasicStridedBitArrayView<dimensions, T> sliceSize(const Size<dimensions>& begin, const Size<dimensions>& size) const;

        /**
         * @brief View on the first @p size bits in the first dimension
         *
         * Equivalent to @cpp data.slice(0, size) @ce. On multi-dimensional
         * views slices just the top-level dimension, use
         * @ref prefix(const Size<dimensions>&) const to slice in
         * all dimensions.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref suffix(std::size_t) const,
         *      @ref exceptPrefix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> prefix(std::size_t size) const {
            return slice(0, size);
        }

        /**
         * @brief View on the first @p size bits in all dimensions
         *
         * Equivalent to @cpp data.slice({}, size) @ce.
         * @see @ref slice(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref suffix(const Size<dimensions>&) const,
         *      @ref exceptPrefix(const Size<dimensions>&) const,
         *      @ref prefix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> prefix(const Size<dimensions>& size) const {
            return slice({}, size);
        }

        /**
         * @brief View on the last @p size bits in the first dimension
         *
         * Equivalent to @cpp data.slice(data.size()[0] - size, data.size()[0]) @ce.
         * On multi-dimensional views slices just the top-level dimension, use
         * @ref suffix(const Size<dimensions>&) const to slice in
         * all dimensions.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref prefix(std::size_t) const,
         *      @ref exceptSuffix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> suffix(std::size_t size) const {
            const std::size_t viewSize = _sizeOffset._data[0] >> 3;
            return slice(viewSize - size, viewSize);
        }

        /**
         * @brief View on the last @p size bits in all dimensions
         *
         * Equivalent to @cpp data.slice(end, data.size()) @ce,
         * where @p end is @cpp data.size()[i] - size[i] @ce for all
         * dimensions.
         * @see @ref slice(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref prefix(const Size<dimensions>&) const,
         *      @ref exceptSuffix(const Size<dimensions>&) const,
         *      @ref suffix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> suffix(const Size<dimensions>& size) const;

        /**
         * @brief View except the first @p size bits in the first dimension
         *
         * Equivalent to @cpp data.slice(size, data.size()[0]) @ce. On
         * multi-dimensional views slices just the top-level dimension, use
         * @ref exceptPrefix(const Size<dimensions>&) const to
         * slice in all dimensions.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref prefix(std::size_t) const,
         *      @ref exceptSuffix(const Size<dimensions>&) const
         */
        BasicStridedBitArrayView<dimensions, T> exceptPrefix(std::size_t size) const {
            return slice(size, _sizeOffset._data[0] >> 3);
        }

        /**
         * @brief View except the first @p size bits in all dimensions
         *
         * Equivalent to @cpp data.slice(size, data.size()) @ce.
         * @see @ref slice(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref prefix(const Size<dimensions>&) const,
         *      @ref exceptSuffix(const Size<dimensions>&) const,
         *      @ref exceptSuffix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> exceptPrefix(const Size<dimensions>& size) const {
            return slice(size, sizeInternal());
        }

        /**
         * @brief View except the last @p size bits in the first dimension
         *
         * Equivalent to @cpp data.slice({}, data.size()[0] - size) @ce. On
         * multi-dimensional views slices just the top-level dimension, use
         * @ref exceptSuffix(const Size<dimensions>&) const to
         * slice in all dimensions.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref sliceSize(std::size_t, std::size_t) const,
         *      @ref suffix(std::size_t) const,
         *      @ref exceptPrefix(const Size<dimensions>&) const
         */
        BasicStridedBitArrayView<dimensions, T> exceptSuffix(std::size_t size) const {
            return slice({}, (_sizeOffset._data[0] >> 3) - size);
        }

        /**
         * @brief View except the last @p size bits in all dimensions
         *
         * Equivalent to @cpp data.slice({}, end) @ce, where
         * @p end is @cpp data.size()[i] - size[i] @ce for all dimensions.
         * @see @ref slice(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref sliceSize(const Size<dimensions>&, const Size<dimensions>&) const,
         *      @ref suffix(const Size<dimensions>&) const,
         *      @ref exceptPrefix(std::size_t) const
         */
        BasicStridedBitArrayView<dimensions, T> exceptSuffix(const Size<dimensions>& size) const;

        /**
         * @brief Pick every Nth bit
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly. On multi-dimensional views affects just the top-level
         * dimension, use @ref every(const Size<dimensions>&) const to pick in
         * all dimensions.
         * @attention Unlike @ref StridedArrayView::every(), this function
         *      doesn't accept negative values. Call @ref flipped() first and
         *      then this function with a positive value a to achieve the
         *      desired effect.
         * @todoc Remove the note once the behavior is deprecated in
         *      StridedArrayView
         */
        BasicStridedBitArrayView<dimensions, T> every(std::size_t skip) const;

        /**
         * @brief Pick every Nth bit
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly.
         * @attention Unlike @ref StridedArrayView::every(), this function
         *      doesn't accept negative values. Call @ref flipped() first and
         *      then this function with a positive value a to achieve the
         *      desired effect.
         * @todoc Remove the note once the behavior is deprecated in
         *      StridedArrayView
         */
        BasicStridedBitArrayView<dimensions, T> every(const Size<dimensions>& skip) const;

        /**
         * @brief Transpose two dimensions
         *
         * Exchanges dimensions @p dimensionA and @p dimensionB by swapping
         * their size and stride values. Together with @ref flipped() can be
         * used to do arbitrary 90° rotations of the view. This is a
         * non-destructive operation on the view, transposing it again will go
         * back to the original form.
         */
        template<unsigned dimensionA, unsigned dimensionB> BasicStridedBitArrayView<dimensions, T> transposed() const;

        /**
         * @brief Flip a dimension
         *
         * Flips given @p dimension by making its stride negative and adjusting
         * the internal base data pointer. Together with @ref transposed() can
         * be used to do arbitrary 90° rotations of the view. This is a
         * non-destructive operation on the view, flipping it again will go
         * back to the original form.
         */
        template<unsigned dimension> BasicStridedBitArrayView<dimensions, T> flipped() const;

        /**
         * @brief Broadcast a dimension
         *
         * Stretches the initial value to @p size in given @p dimension by
         * setting its stride to 0 and size to @p size. To avoid destructive
         * operations on the view, the function expects that size in given
         * dimension is 1. If you need to broadcast a dimension that has more
         * than one bit, @ref slice() it first.
         */
        template<unsigned dimension> BasicStridedBitArrayView<dimensions, T> broadcasted(std::size_t size) const;

    private:
        /* Needed for mutable/immutable conversion */
        template<unsigned, class> friend class BasicStridedBitArrayView;
        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<unsigned, class> friend struct Implementation::StridedBitElement;
        /* Used by StridedArrayView::sliceBit() */
        template<unsigned, class> friend class StridedArrayView;
        /* So ArrayTuple can update the data pointer */
        friend T*& Implementation::dataRef<>(BasicStridedBitArrayView<dimensions, T>&);

        /* Internal constructor without type/size checks for things like
           slice() etc. Argument order is different to avoid this function
           getting matched when passing a pointer instead of a view to the
           constructor. */
        constexpr /*implicit*/ BasicStridedBitArrayView(const Size<dimensions>& sizeOffset, const Stride<dimensions>& stride, ErasedType* data) noexcept: _data{data}, _sizeOffset{sizeOffset}, _stride{stride} {}

        /* Compared to size() returns Size<dimensions> even in 1D */
        constexpr Size<dimensions> sizeInternal() const {
            return Implementation::sizeWithoutOffset(_sizeOffset, typename Implementation::GenerateSequence<dimensions>::Type{});
        }

        template<std::size_t ...sequence> constexpr StridedDimensions<dimensions, bool> isEmptyInternal(Implementation::Sequence<sequence...>) const {
            return StridedDimensions<dimensions, bool>{(_sizeOffset._data[sequence] >> 3 == 0)...};
        }

        ErasedType* _data;
        Size<dimensions> _sizeOffset;
        Stride<dimensions> _stride;
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief Strided bit array view
@m_since_latest

Immutable, use @ref MutableStridedBitArrayView for mutable access.
@see @ref StridedBitArrayView1D, @ref StridedBitArrayView2D,
    @ref StridedBitArrayView3D, @ref StridedBitArrayView4D
*/
template<unsigned dimensions> using StridedBitArrayView = BasicStridedBitArrayView<dimensions, const char>;
#endif

/**
@brief One-dimensional strided bit array view
@m_since_latest

Immutable, use @ref MutableStridedBitArrayView1D for mutable access.
@see @ref StridedBitArrayView2D, @ref StridedBitArrayView3D,
    @ref StridedBitArrayView4D
*/
typedef StridedBitArrayView<1> StridedBitArrayView1D;

/**
@brief Two-dimensional strided bit array view
@m_since_latest

Immutable, use @ref MutableStridedBitArrayView2D for mutable access.
@see @ref StridedBitArrayView1D, @ref StridedBitArrayView3D,
    @ref StridedBitArrayView4D
*/
typedef StridedBitArrayView<2> StridedBitArrayView2D;

/**
@brief Three-dimensional strided bit array view
@m_since_latest

Immutable, use @ref MutableStridedBitArrayView3D for mutable access.
@see @ref StridedBitArrayView1D, @ref StridedBitArrayView2D,
    @ref StridedBitArrayView4D
*/
typedef StridedBitArrayView<3> StridedBitArrayView3D;

/**
@brief Four-dimensional strided bit array view
@m_since_latest

Immutable, use @ref MutableStridedBitArrayView4D for mutable access.
@see @ref StridedBitArrayView1D, @ref StridedBitArrayView2D,
    @ref StridedBitArrayView3D
*/
typedef StridedBitArrayView<4> StridedBitArrayView4D;

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief Mutable strided bit array view
@m_since_latest

@see @ref MutableStridedBitArrayView1D, @ref MutableStridedBitArrayView2D,
    @ref MutableStridedBitArrayView3D, @ref MutableStridedBitArrayView4D,
    @ref StridedBitArrayView
*/
template<unsigned dimensions> using MutableStridedBitArrayView = BasicStridedBitArrayView<dimensions, char>;
#endif

/**
@brief One-dimensional mutable strided bit array view
@m_since_latest

@see @ref MutableStridedBitArrayView2D, @ref MutableStridedBitArrayView3D,
    @ref MutableStridedBitArrayView4D, @ref StridedBitArrayView1D
*/
typedef MutableStridedBitArrayView<1> MutableStridedBitArrayView1D;

/**
@brief Two-dimensional mutable strided bit array view
@m_since_latest

@see @ref MutableStridedBitArrayView1D, @ref MutableStridedBitArrayView3D,
    @ref MutableStridedBitArrayView4D, @ref StridedBitArrayView2D
*/
typedef MutableStridedBitArrayView<2> MutableStridedBitArrayView2D;

/**
@brief Three-dimensional mutable strided bit array view
@m_since_latest

@see @ref MutableStridedBitArrayView1D, @ref MutableStridedBitArrayView2D,
    @ref MutableStridedBitArrayView4D, @ref StridedBitArrayView3D
*/
typedef MutableStridedBitArrayView<3> MutableStridedBitArrayView3D;

/**
@brief Four-dimensional mutable strided bit array view
@m_since_latest

@see @ref MutableStridedBitArrayView1D, @ref MutableStridedBitArrayView2D,
    @ref MutableStridedBitArrayView3D, @ref StridedBitArrayView4D
*/
typedef MutableStridedBitArrayView<4> MutableStridedBitArrayView4D;

/**
@debugoperator{BasicStridedBitArrayView}
@m_since_latest

Prints the value as nested @cb{.shell-session} {a, b, …} @ce, with each
element in the last dimension being the next 8 bits (or less, if the last
dimension is smaller). To have a monotonic order, the first character in each
element is the first bit in the 8-bit group --- i.e., the order is reversed
compared to binary literals.

For example, expanding upon the snippet in
@ref BitArrayView::operator<<(Utility::Debug&, BitArrayView) "operator<<(Utility::Debug&, BitArrayView)",
the following 64-bit number, shifted by 5 bits and grouped into 3 rows
containing 8 bits each, will print like this on a Little-Endian machine:

@m_class{m-code-figure}

@parblock

@snippet Containers-cpp14.cpp StridedBitArrayView-operator<<

<b></b>

@m_class{m-nopad}

@code{.shell-session}
{{11110000}, {11001100}, {10101010}}
@endcode

@endparblock
*/
template<unsigned dimensions> Utility::Debug& operator<<(Utility::Debug& debug, const StridedBitArrayView<dimensions>& value) {
    debug << "{" << Utility::Debug::nospace;

    for(std::size_t i = 0, iMax = value.size()[0]; i != iMax; ++i) {
        if(i) debug << ",";
        debug << value[i] << Utility::Debug::nospace;
    }

    return debug << "}";
}

/**
 * @debugoperator{BasicStridedBitArrayView}
 * @m_since_latest
 */
/* Printing a mutable view doesn't work without this proxying overload */
template<unsigned dimensions> inline Utility::Debug& operator<<(Utility::Debug& debug, const MutableStridedBitArrayView<dimensions>& value) {
    return debug << StridedBitArrayView<dimensions>{value};
}

#ifndef DOXYGEN_GENERATING_OUTPUT
CORRADE_UTILITY_EXPORT Utility::Debug& operator<<(Utility::Debug& debug, const StridedBitArrayView1D& value);
#endif

template<unsigned dimensions, class T> constexpr BasicStridedBitArrayView<dimensions, T>::BasicStridedBitArrayView(BasicBitArrayView<T> data, ErasedType* begin, std::size_t offset, const Size<dimensions>& size, const Stride<dimensions>& stride) noexcept:
    _data{(
        /* A strided array view is usually not created from scratch in tight
           loops (except for slicing, which uses a different constructor) and
           should be as checked as possible, so it's not a debug assert.

           If any size is zero, data can be zero-sized too. If the largest
           stride is zero, `data` can have *any* size and it could be okay,
           can't reliably test that. */
        CORRADE_CONSTEXPR_ASSERT(Implementation::isAnyDimensionZero(size._data, typename Implementation::GenerateSequence<dimensions>::Type{}) || Implementation::largestStride(size._data, stride._data, typename Implementation::GenerateSequence<dimensions>::Type{}) <= data.size(),
            "Containers::StridedBitArrayView: data size" << data.size() << "is not enough for" << size << "bits of stride" << stride),
        /** @todo unlike in StridedArrayView we *could* also check if "end" is
            contained in data, however one still can't perform arithmetic on a
            null pointer in a constexpr context and given that we're counting
            bits here the comparisons would be way too complex to avoid pointer
            overflows and rounding errors */
        CORRADE_CONSTEXPR_ASSERT(begin != data.data() || (begin == data.data() && offset >= data.offset()),
            "Containers::StridedBitArrayView: offset" << offset << "is less than data offset" << data.offset() << "in the same byte"),
        #ifdef CORRADE_NO_ASSERT
        static_cast<void>(data),
        #endif
        begin)},
    _sizeOffset{
        (CORRADE_CONSTEXPR_DEBUG_ASSERT(offset < 8,
            "Containers::StridedBitArrayView: offset expected to be smaller than 8 bits, got" << offset),
        /* Size checked to be small enough inside sizeWithOffset() */
        Implementation::sizeWithOffset(size, offset, typename Implementation::GenerateSequence<dimensions>::Type{}))},
    _stride{stride} {}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<unsigned dimensions, class T> template<unsigned lessDimensions, class> BasicStridedBitArrayView<dimensions, T>::BasicStridedBitArrayView(const BasicStridedBitArrayView<lessDimensions, T>& other) noexcept: _data{other._data}, _sizeOffset{Corrade::NoInit}, _stride{Corrade::NoInit} {
    /* Set size and stride in the extra dimensions */
    constexpr std::size_t extraDimensions = dimensions - lessDimensions;
    const std::size_t stride = (other._sizeOffset._data[0] >> 3)*other._stride._data[0];
    for(std::size_t i = 0; i != extraDimensions; ++i) {
        _sizeOffset._data[i] = 1 << 3;
        _stride._data[i] = stride;
    }
    /* Copy size and stride in the existing dimensions */
    for(std::size_t i = 0; i != lessDimensions; ++i) {
        _sizeOffset._data[extraDimensions + i] = other._sizeOffset._data[i] & ~0x07;
        _stride._data[extraDimensions + i] = other._stride._data[i];
    }
    /* Transfer the offset as well */
    _sizeOffset._data[0] |= other._sizeOffset._data[0] & 0x07;
}
#endif

template<unsigned dimensions, class T> template<unsigned dimension> bool BasicStridedBitArrayView<dimensions, T>::isContiguous() const {
    static_assert(dimension < dimensions, "dimension out of bounds");
    std::size_t nextDimensionSize = 1;
    for(std::size_t i = dimensions; i != dimension; --i) {
        if(std::size_t(_stride._data[i - 1]) != nextDimensionSize)
            return false;
        nextDimensionSize *= _sizeOffset._data[i - 1] >> 3;
    }

    return true;
}

template<unsigned dimensions, class T> BasicBitArrayView<T> BasicStridedBitArrayView<dimensions, T>::asContiguous() const {
    CORRADE_DEBUG_ASSERT(isContiguous(),
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous", {});
    std::size_t size = 1;
    for(std::size_t i = 0; i != dimensions; ++i)
        size *= _sizeOffset._data[i] >> 3;
    return {_data, _sizeOffset._data[0] & 0x07, size};
}

template<unsigned dimensions, class T> template<unsigned dimension> BasicStridedBitArrayView<dimension + 1, T> BasicStridedBitArrayView<dimensions, T>::asContiguous() const {
    static_assert(dimension < dimensions, "dimension out of bounds");
    CORRADE_DEBUG_ASSERT(isContiguous<dimension>(),
        "Containers::StridedBitArrayView::asContiguous(): the view is not contiguous from dimension" << dimension, {});

    Size<dimension + 1> sizeOffset;
    Stride<dimension + 1> stride;
    for(std::size_t i = 0; i != dimension; ++i) {
        /* Clear offset from the first dimension */
        sizeOffset._data[i] = _sizeOffset._data[i] & ~0x07;
        stride._data[i] = _stride._data[i];
    }

    sizeOffset._data[dimension] = 1 << 3;
    stride._data[dimension] = 1;
    for(std::size_t i = dimension; i != dimensions; ++i)
        sizeOffset._data[dimension] *= _sizeOffset._data[i] >> 3;

    /* Put the offset back */
    sizeOffset._data[0] |= _sizeOffset._data[0] & 0x07;

    return {sizeOffset, stride, _data};
}

namespace Implementation {
    template<unsigned dimensions, class T> struct StridedBitElement {
        static BasicStridedBitArrayView<dimensions - 1, T> get(T* data, const Size<dimensions>& sizeOffset, const Stride<dimensions>& stride, std::size_t i) {
            const std::ptrdiff_t offsetInBits = (sizeOffset._data[0] & 0x07) + i*stride._data[0];
            Size<dimensions - 1> outputSizeOffset{Corrade::NoInit};
            Stride<dimensions - 1> outputStride{Corrade::NoInit};
            outputSizeOffset._data[0] = sizeOffset._data[1]|(offsetInBits & 0x07);
            for(std::size_t j = 1; j != dimensions - 1; ++j)
                outputSizeOffset._data[j] = sizeOffset._data[j + 1];
            for(std::size_t j = 0; j != dimensions - 1; ++j)
                outputStride._data[j] = stride._data[j + 1];
            return BasicStridedBitArrayView<dimensions - 1, T>{outputSizeOffset, outputStride, static_cast<T*>(data) + (offsetInBits >> 3)};
        }
    };
    template<class T> struct StridedBitElement<1, T> {
        static bool get(T* data, const Size1D& sizeOffset, const Stride1D& stride, std::size_t i) {
            const std::ptrdiff_t offsetInBits = (sizeOffset._data[0] & 0x07) + i*stride._data[0];
            return static_cast<T*>(data)[offsetInBits >> 3] & (1 << (offsetInBits & 0x07));
        }
    };
}

template<unsigned dimensions, class T> auto BasicStridedBitArrayView<dimensions, T>::operator[](const std::size_t i) const -> ElementType {
    CORRADE_DEBUG_ASSERT(i < _sizeOffset._data[0] >> 3,
        "Containers::StridedBitArrayView::operator[](): index" << i << "out of range for" << (_sizeOffset._data[0] >> 3) << "elements", {});
    return Implementation::StridedBitElement<dimensions, T>::get(static_cast<T*>(_data), _sizeOffset, _stride, i);
}

template<unsigned dimensions, class T> template<class U, class> inline void BasicStridedBitArrayView<dimensions, T>::set(std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < _sizeOffset._data[0] >> 3,
        "Containers::StridedBitArrayView::set(): index" << i << "out of range for" << (_sizeOffset._data[0] >> 3) << "bits", );
    const std::ptrdiff_t offsetInBits = (_sizeOffset._data[0] & 0x07) + i*_stride._data[0];
    static_cast<T*>(_data)[offsetInBits >> 3] |= (1 << (offsetInBits & 0x07));
}

template<unsigned dimensions, class T> template<class U, class> inline void BasicStridedBitArrayView<dimensions, T>::reset(std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < _sizeOffset._data[0] >> 3,
        "Containers::StridedBitArrayView::reset(): index" << i << "out of range for" << (_sizeOffset._data[0] >> 3) << "bits", );
    const std::ptrdiff_t offsetInBits = (_sizeOffset._data[0] & 0x07) + i*_stride._data[0];
    static_cast<T*>(_data)[offsetInBits >> 3] &= ~(1 << (offsetInBits & 0x07));
}

template<unsigned dimensions, class T> template<class U, class> inline void BasicStridedBitArrayView<dimensions, T>::set(std::size_t i, bool value) const {
    CORRADE_DEBUG_ASSERT(i < _sizeOffset._data[0] >> 3,
        "Containers::StridedBitArrayView::set(): index" << i << "out of range for" << (_sizeOffset._data[0] >> 3) << "bits", );
    const std::ptrdiff_t offsetInBits = (_sizeOffset._data[0] & 0x07) + i*_stride._data[0];
    /* http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching */
    char& byte = static_cast<T*>(_data)[offsetInBits >> 3];
    byte ^= (-char(value) ^ byte) & (1 << (offsetInBits & 0x07));
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::slice(const std::size_t begin, const std::size_t end) const {
    CORRADE_DEBUG_ASSERT(begin <= end && end <= (_sizeOffset._data[0] >> 3),
        "Containers::StridedBitArrayView::slice(): slice [" << Utility::Debug::nospace
        << begin << Utility::Debug::nospace << ":"
        << Utility::Debug::nospace << end << Utility::Debug::nospace
        << "] out of range for" << (_sizeOffset._data[0] >> 3) << "elements", {});

    const std::ptrdiff_t offsetInBits = (_sizeOffset._data[0] & 0x07) + begin*_stride._data[0];

    /* Data pointer is whole bytes */
    T* const data = static_cast<T*>(_data) + (offsetInBits >> 3);

    /* The new offset is the remaining bits in the last byte, combine with the
       new size */
    Size<dimensions> sizeOffset = _sizeOffset;
    sizeOffset._data[0] = (std::size_t(end - begin) << 3) | (offsetInBits & 0x07);

    /* Stride stays the same */
    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, _stride, data};
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::slice(const Size<dimensions>& begin, const Size<dimensions>& end) const {
    Size<dimensions> sizeOffset{Corrade::NoInit};
    std::ptrdiff_t offsetInBits = _sizeOffset._data[0] & 0x07;

    /* Set sizes in all dimensions; calculate bit offset based on offsets of
       all source dimensions */
    for(std::size_t i = 0; i != dimensions; ++i) {
        /* Unlike plain slice(begin, end), complex dimension-changing slicing
           is usually not called in tight loops and should be as checked as
           possible, so it's not a debug assert */
        CORRADE_ASSERT(begin._data[i] <= end._data[i] && end._data[i] <= (_sizeOffset._data[i] >> 3),
            "Containers::StridedBitArrayView::slice(): slice [" << Utility::Debug::nospace
            << begin << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end << Utility::Debug::nospace
            << "] out of range for" << sizeInternal() << "elements in dimension" << i,
            {});
        sizeOffset._data[i] = std::size_t(end._data[i] - begin._data[i]) << 3;
        offsetInBits += begin._data[i]*_stride._data[i];
    }

    /* Data pointer is whole bytes */
    T* const data = static_cast<T*>(_data) + (offsetInBits >> 3);

    /* The new offset is the remaining bits in the last byte */
    sizeOffset._data[0] |= offsetInBits & 0x07;

    /* Stride stays the same */
    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, _stride, data};
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::sliceSize(const Size<dimensions>& begin, const Size<dimensions>& size) const {
    Size<dimensions> end{Corrade::NoInit};
    for(std::size_t i = 0; i != dimensions; ++i)
        end._data[i] = begin._data[i] + size._data[i];
    return slice(begin, end);
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::suffix(const Size<dimensions>& size) const {
    Size<dimensions> begin{Corrade::NoInit};
    Size<dimensions> end{Corrade::NoInit};
    for(std::size_t i = 0; i != dimensions; ++i) {
        const std::size_t viewDimensionSize = _sizeOffset._data[i] >> 3;
        begin._data[i] = viewDimensionSize - size._data[i];
        end._data[i] = viewDimensionSize;
    }
    return slice(begin, end);
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::exceptSuffix(const Size<dimensions>& size) const {
    Size<dimensions> end{Corrade::NoInit};
    for(std::size_t i = 0; i != dimensions; ++i)
        end._data[i] = (_sizeOffset._data[i] >> 3) - size._data[i];
    return slice({}, end);
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::every(const std::size_t step) const {
    Size<dimensions> steps;
    steps._data[0] = step;
    for(std::size_t i = 1; i != dimensions; ++i)
        steps._data[i] = 1;
    return every(steps);
}

template<unsigned dimensions, class T> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::every(const Size<dimensions>& step) const {
    /* Unlike plain slice(begin, end), complex slicing is usually not called in
       tight loops and should be as checked as possible, so it's not a debug
       assert */
    CORRADE_ASSERT(!Implementation::isAnyDimensionZero(step._data, typename Implementation::GenerateSequence<dimensions>::Type{}),
        "Containers::StridedBitArrayView::every(): expected a non-zero step, got" << step, {});

    Size<dimensions> sizeOffset{Corrade::NoInit};
    Stride<dimensions> stride = _stride;
    for(std::size_t dimension = 0; dimension != dimensions; ++dimension) {
        /* Taking every 5th element of a 6-element array should result in 2
           elements. This operation also clears the offset from the first 3
           bits. */
        sizeOffset._data[dimension] = (((_sizeOffset._data[dimension] >> 3) + step._data[dimension] - 1)/step._data[dimension]) << 3;
        stride._data[dimension] *= step._data[dimension];
    }

    /* Put back the offset */
    sizeOffset._data[0] |= _sizeOffset._data[0] & 0x07;

    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, stride, _data};
}

template<unsigned dimensions, class T> template<unsigned dimensionA, unsigned dimensionB> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::transposed() const {
    static_assert(dimensionA < dimensions && dimensionB < dimensions,
        "dimensions out of range");

    Size<dimensions> sizeOffset = _sizeOffset;
    Stride<dimensions> stride = _stride;

    /* Clear offset from the first dimension, swap the dimensions and then put
       it back */
    sizeOffset._data[0] &= ~0x07;
    std::swap(sizeOffset._data[dimensionA], sizeOffset._data[dimensionB]);
    std::swap(stride._data[dimensionA], stride._data[dimensionB]);
    sizeOffset._data[0] |= _sizeOffset._data[0] & 0x07;

    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, stride, _data};
}

template<unsigned dimensions, class T> template<unsigned dimension> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::flipped() const {
    static_assert(dimension < dimensions, "dimension out of range");

    /* Calculate offset of the last bit in this dimension */
    const std::ptrdiff_t sizeInDimension = _sizeOffset._data[dimension] >> 3;
    const std::ptrdiff_t offsetInBits =  (_sizeOffset._data[0] & 0x07) + _stride._data[dimension]*(sizeInDimension ? sizeInDimension - 1 : 0);

    /* Data pointer is whole bytes */
    T* const data = static_cast<T*>(_data) + (offsetInBits >> 3);

    /* The new offset is remaining bits in the last byte */
    Size<dimensions> sizeOffset = _sizeOffset;
    sizeOffset._data[0] &= ~0x07;
    sizeOffset._data[0] |= offsetInBits & 0x07;

    /* Flip the stride in this dimension */
    Stride<dimensions> stride = _stride;
    stride._data[dimension] *= -1;

    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, stride, data};
}

template<unsigned dimensions, class T> template<unsigned dimension> BasicStridedBitArrayView<dimensions, T> BasicStridedBitArrayView<dimensions, T>::broadcasted(std::size_t size) const {
    static_assert(dimension < dimensions, "dimension out of range");
    /* Unlike plain slice(begin, end), complex slicing is usually not called in
       tight loops and should be as checked as possible, so it's not a debug
       assert */
    CORRADE_ASSERT(_sizeOffset._data[dimension] >> 3 == 1,
        "Containers::StridedBitArrayView::broadcasted(): can't broadcast dimension" << dimension << "with" << (_sizeOffset._data[dimension] >> 3) << "elements", {});

    /* Clear offset from the first dimension, update the size and then put it
       back */
    Size<dimensions> sizeOffset = _sizeOffset;
    sizeOffset._data[0] &= ~0x07;
    sizeOffset._data[dimension] = size << 3;
    sizeOffset._data[0] |= _sizeOffset._data[0] & 0x07;

    /* Set stride to 0 */
    Stride<dimensions> stride = _stride;
    stride._data[dimension] = 0;

    return BasicStridedBitArrayView<dimensions, T>{sizeOffset, stride, _data};
}

}}

#endif
