#ifndef Corrade_Containers_StridedArrayView_h
#define Corrade_Containers_StridedArrayView_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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
 * @brief Class @ref Corrade::Containers::StridedArrayView, @ref Corrade::Containers::StridedIterator
 */

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/Tags.h"

namespace Corrade { namespace Containers {

namespace Implementation {
    #ifndef DOXYGEN_GENERATING_OUTPUT /* it should ignore, but it doesn't */
    /** @todo C++14: use std::make_index_sequence and std::integer_sequence */
    template<std::size_t ...> struct Sequence {};

    /* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };
    #endif

    template<unsigned, class> struct StridedElement;
    template<bool> struct ArrayCastFlattenOrInflate;

    template<unsigned dimensions> constexpr std::size_t largestStride(const StridedDimensions<dimensions, std::size_t>&, const StridedDimensions<dimensions, std::ptrdiff_t>&, Sequence<>) {
        return 0;
    }

    constexpr std::size_t largerStride(std::size_t a, std::size_t b) {
        return a < b ? b : a; /* max(), but named like this to avoid clashes */
    }
    template<unsigned dimensions, std::size_t first, std::size_t ...next> constexpr std::size_t largestStride(const StridedDimensions<dimensions, std::size_t>& size, const StridedDimensions<dimensions, std::ptrdiff_t>& stride, Sequence<first, next...>) {
        return largerStride(size[first]*std::size_t(stride[first] < 0 ? -stride[first] : stride[first]),
            largestStride(size, stride, Sequence<next...>{}));
    }
}

/**
@brief Helper for specifying sizes and strides for @ref StridedArrayView

Main property compared to a plain C array of value is convertibility from/to
@ref StaticArrayView, implicit conversion from/to a scalar type in the
one-dimensional case and element-wise equality comparison. See
@ref StridedArrayView for actual usage examples.
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
        constexpr explicit StridedDimensions(ValueInitT) noexcept: _data{} {}

        /** @brief Construct without initializing the contents */
        explicit StridedDimensions(NoInitT) noexcept {}

        /** @brief Constructor */
        template<class ...Args> constexpr /*implicit*/ StridedDimensions(T first, Args... next) noexcept: _data{T(first), T(next)...} {
            static_assert(sizeof...(Args) + 1 == dimensions, "wrong value count");
        }

        /** @brief Construct from a view */
        constexpr /*implicit*/ StridedDimensions(StaticArrayView<dimensions, const T> values) noexcept: StridedDimensions{values.data(), typename Implementation::GenerateSequence<dimensions>::Type{}} {}

        /** @brief Construct from an array */
        constexpr /*implicit*/ StridedDimensions(const T(&values)[dimensions]) noexcept: StridedDimensions{values, typename Implementation::GenerateSequence<dimensions>::Type{}} {}

        /** @brief Conversion to an array view */
        constexpr /*implicit*/ operator StaticArrayView<dimensions, const T>() const {
            /* GCC 4.8 needs this to be explicit */
            return Containers::staticArrayView(_data);
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
            return CORRADE_CONSTEXPR_ASSERT(i < dimensions,
                "Containers::StridedDimensions::operator[](): dimension" << i << "out of range for" << dimensions << "dimensions"), _data[i];
        }

        /** @brief Element access */
        T& operator[](std::size_t i) {
            CORRADE_ASSERT(i < dimensions,
                "Containers::StridedDimensions::operator[](): dimension" << i << "out of range for" << dimensions << "dimensions", _data[0]);
            return _data[i];
        }

        /** @brief First element */
        constexpr const T* begin() const { return _data; }
        constexpr const T* cbegin() const { return _data; } /**< @overload */

        /** @brief (One item after) last element */
        constexpr const T* end() const { return _data + dimensions; }
        constexpr const T* cend() const { return _data + dimensions; } /**< @overload */

    private:
        template<unsigned, class> friend class StridedArrayView;
        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<unsigned, class> friend struct Implementation::StridedElement;
        template<bool> friend struct Implementation::ArrayCastFlattenOrInflate;
        template<class U, unsigned dimensions_, class T_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, T_>&);

        template<class U, std::size_t ...sequence> constexpr explicit StridedDimensions(const U* values, Implementation::Sequence<sequence...>) noexcept: _data{T(values[sequence])...} {}

        T _data[dimensions];
};

/**
@brief Multi-dimensional array view with size and stride information

Immutable wrapper around continuous sparse range of data, useful for easy
iteration over interleaved arrays. Usage example:

@snippet Containers.cpp StridedArrayView-usage

For convenience, similarly to @ref ArrayView, this class is implicitly
convertible from plain C arrays, @ref ArrayView and @ref StaticArrayView, with
stride equal to array type size. The following two statements are equivalent:

@snippet Containers.cpp StridedArrayView-usage-conversion

Unlike @ref ArrayView, this wrapper doesn't provide direct pointer access
because pointer arithmetic doesn't work as usual here. The
@ref arrayCast(const StridedArrayView<dimensions, T>&) overload also works
slightly differently with strided arrays --- it checks that a type fits into
the stride instead of expecting the total byte size to stay the same.

@section Containers-StridedArrayView-multidimensional Multi-dimensional views

Apart from single-dimensional interleaved arrays, strided array views are
useful also for describing and iteration over multi-dimensional data such as 2D
(sub)images. In that case, @ref operator[]() and iterator access return a view
of one dimension less instead of a direct element reference, and there are
@ref slice(const Size&, const Size&) const, @ref prefix(const Size&) const and
@ref suffix(const Size&) const overloads working on all dimensions at the same
time.

@snippet Containers.cpp StridedArrayView-usage-3d

Both the subscription operator and the slicing operations allow you to change
the view dimension count --- for example, obtaining the fifth image or just a
view on the (now red) center of it. Conversely, it's possible to turn a
lower-dimensional view into a slice in a higher-dimensional view.

@snippet Containers.cpp StridedArrayView-usage-3d-slice-2d

Finally, since the actual view elements can be also non-scalar data, there's an
overload of @ref arrayCast(const StridedArrayView<dimensions, T>&) that can
"extract" an additional dimension out of these or, on the other hand, flatten
it back if the last dimension is tightly packed.

@snippet Containers.cpp StridedArrayView-usage-inflate

@section Containers-StridedArrayView-zero-stride Zero and negative stride

The stride value doesn't have to be just positive. Order of elements in any
dimension of the view can be reversed by calling @ref flipped(), and together
with @ref transposed() it's possible to generate any 90° rotation of the data:

@snippet Containers.cpp StridedArrayView-usage-rotate

Setting stride to @cpp 0 @ce in a particular dimension will reuse the same
memory address for every element. The convenience @ref broadcasted() function
will repeat given dimension given number of times:

@snippet Containers.cpp StridedArrayView-usage-broadcast

@section Containers-StridedArrayView-stl STL compatibility

On compilers that support C++2a and @ref std::span, implicit conversion of it
to a @ref StridedArrayView1D is provided in
@ref Corrade/Containers/ArrayViewStlSpan.h. The conversion is provided in a
separate header to avoid unconditional @cpp #include <span> @ce, which
significantly affects compile times. The following table lists allowed
conversions:

Corrade type                    | ↭ | STL type
------------------------------- | - | ---------------------
@ref StridedArrayView1D "StridedArrayView1D<T>" | ← | @ref std::span "std::span<T>"
@ref StridedArrayView1D "StridedArrayView1D<T>" | ← | @ref std::span "std::span<const T, size>"
@ref StridedArrayView1D "StridedArrayView1D<const T>" | ← | @ref std::span "std::span<const T>"

See @ref Containers-ArrayView-stl "ArrayView STL compatibility" for more
information.

@see @ref StridedIterator, @ref StridedArrayView1D, @ref StridedArrayView2D,
    @ref StridedArrayView3D
*/
/* All member functions are const because the view doesn't own the data */
template<unsigned dimensions, class T> class StridedArrayView {
    static_assert(dimensions, "can't have a zero-dimensional view");

    public:
        /**
         * @brief Underlying type
         *
         * Underlying data type. See also @ref ElementType and @ref ErasedType.
         */
        typedef T Type;

        /**
         * @brief Element type
         *
         * For @ref StridedArrayView1D equivalent to a reference to @ref Type,
         * for higher dimensions a strided view of one dimension less.
         */
        typedef typename std::conditional<dimensions == 1, T&, StridedArrayView<dimensions - 1, T>>::type ElementType;

        /**
         * @brief Erased type
         *
         * Either @cpp void @ce or @cpp const void @ce based on constness
         * of @ref Type.
         */
        typedef typename std::conditional<std::is_const<T>::value, const void, void>::type ErasedType;

        /** @brief Size values */
        typedef StridedDimensions<dimensions, std::size_t> Size;

        /** @brief Stride values */
        typedef StridedDimensions<dimensions, std::ptrdiff_t> Stride;

        enum: unsigned {
            Dimensions = dimensions /**< View dimensions */
        };

        /** @brief Conversion from `nullptr` */
        constexpr /*implicit*/ StridedArrayView(std::nullptr_t) noexcept: _data{}, _size{}, _stride{} {}

        /**
         * @brief Default constructor
         *
         * Creates an empty view. Copy a non-empty @ref Array, @ref ArrayView
         * or @ref StridedArrayView onto the instance to make it useful.
         */
        constexpr /*implicit*/ StridedArrayView() noexcept: _data{}, _size{}, _stride{} {}

        /**
         * @brief Construct a view with explicit size and stride
         * @param data      Continuous view on the data
         * @param member    Pointer to the first member of the strided view
         * @param size      Data size
         * @param stride    Data stride
         *
         * The @p data view is used only for a bounds check --- expects that
         * @p data size is enough for @p size and @p stride in the largest
         * dimension if the stride is either positive or negative. Zero strides
         * unfortunately can't be reliably checked for out-of-bounds
         * conditions, so be extra careful when specifying these.
         */
        constexpr /*implicit*/ StridedArrayView(Containers::ArrayView<ErasedType> data, T* member, const Size& size, const Stride& stride) noexcept: _data{(
            /** @todo can't compare void pointers to check if member is in data,
                    it's not constexpr :( */
            /* If the largest stride is zero, `data` can have *any* size and it
               could be okay, can't reliably test that */
            CORRADE_CONSTEXPR_ASSERT(!Implementation::largestStride(size, stride, typename Implementation::GenerateSequence<dimensions>::Type{}) || Implementation::largestStride(size, stride, typename Implementation::GenerateSequence<dimensions>::Type{}) <= data.size(),
                "Containers::StridedArrayView: data size" << data.size() << "is not enough for" << size << "elements of stride" << stride),
            member)}, _size{size}, _stride{stride} {}

        /**
         * @brief Construct a view with explicit size and stride
         *
         * Equivalent to calling @ref StridedArrayView(Containers::ArrayView<ErasedType>, T*, const Size&, const Stride&)
         * with @p data as the first parameter and @cpp data.data() @ce as the
         * second parameter.
         */
        constexpr /*implicit*/ StridedArrayView(Containers::ArrayView<T> data, const Size& size, const Stride& stride) noexcept: StridedArrayView{data, data.data(), size, stride} {}

        /**
         * @brief Construct a view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only on one-dimensional views and if @cpp T* @ce is
         * implicitly convertible to @cpp U* @ce. Expects that both types have
         * the same size; stride is implicitly set to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U, std::size_t size>
        #else
        template<class U, std::size_t size, unsigned d = dimensions, class = typename std::enable_if<d == 1 && std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(U(&data)[size]) noexcept: _data{data}, _size{size}, _stride{sizeof(T)} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on @ref StridedArrayView
         *
         * Enabled only if @cpp T* @ce is implicitly convertible to @cpp U* @ce.
         * Expects that both types have the same size.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(StridedArrayView<dimensions, U> view) noexcept: _data{view._data}, _size{view._size}, _stride{view._stride} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on @ref ArrayView
         *
         * Enabled only on one-dimensional views and if @cpp T* @ce is
         * implicitly convertible to @cpp U* @ce. Expects that both types have
         * the same size; stride is implicitly set to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U>
        #else
        template<class U, unsigned d = dimensions, class = typename std::enable_if<d == 1 && std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(ArrayView<U> view) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(T)} {
            static_assert(sizeof(T) == sizeof(U), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view on @ref StaticArrayView
         *
         * Enabled only on one-dimensional views and if @cpp T* @ce is
         * implicitly convertible to @cpp U* @ce. Expects that both types have
         * the same size; stride is implicitly set to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size, class U>
        #else
        template<std::size_t size, class U, unsigned d = dimensions, class = typename std::enable_if<d == 1 && std::is_convertible<U*, T*>::value>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(StaticArrayView<size, U> view) noexcept: _data{view.data()}, _size{size}, _stride{sizeof(T)} {
            static_assert(sizeof(U) == sizeof(T), "type sizes are not compatible");
        }

        /**
         * @brief Construct a view from an external view representation
         *
         * Enabled only on one-dimensional views.
         *
         * @see @ref Containers-StridedArrayView-stl
         */
        /* There's no restriction that would disallow creating StridedArrayView
           from e.g. std::vector<T>&& because that would break uses like
           `consume(foo());`, where `consume()` expects a view but `foo()`
           returns a std::vector. Besides that, there's no
           StaticArrayViewConverter overload as we wouldn't be able to infer
           the size parameter. Since ArrayViewConverter is supposed to handle
           conversion from statically sized arrays as well, this is okay. */
        template<class U, unsigned d = dimensions, class = typename std::enable_if<d == 1, decltype(Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(std::declval<U&&>()))>::type> constexpr /*implicit*/ StridedArrayView(U&& other) noexcept: StridedArrayView{Implementation::ArrayViewConverter<T, typename std::decay<U&&>::type>::from(std::forward<U>(other))} {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

        /** @brief Array data */
        constexpr ErasedType* data() const { return _data; }

        /**
         * @brief Array size
         *
         * Returns just @ref std::size_t instead of @ref Size for the
         * one-dimensional case so the usual numeric operations work as
         * expected. Explicitly cast to @ref Size to ensure consistent behavior
         * for all dimensions in generic implementations.
         */
        constexpr typename std::conditional<dimensions == 1, std::size_t, const Size&>::type size() const { return _size; }

        /**
         * @brief Array stride
         *
         * Returns just @ref std::ptrdiff_t instead of @ref Stride for the
         * one-dimensional case so the usual numeric operations work as
         * expected. Explicitly cast to @ref Stride to ensure consistent
         * behavior for all dimensions in generic implementations.
         */
        constexpr typename std::conditional<dimensions == 1, std::ptrdiff_t, const Stride&>::type stride() const { return _stride; }

        /** @brief Whether the array is empty */
        constexpr StridedDimensions<dimensions, bool> empty() const {
            return emptyInternal(typename Implementation::GenerateSequence<dimensions>::Type{});
        }

        /** @brief Element access */
        ElementType operator[](std::size_t i) const;

        /**
         * @brief Iterator to first element
         *
         * @see @ref front()
         */
        StridedIterator<dimensions, T> begin() const { return {_data, _size, _stride, 0}; }
        /** @overload */
        StridedIterator<dimensions, T> cbegin() const { return {_data, _size, _stride, 0}; }

        /**
         * @brief Iterator to (one item after) last element
         *
         * @see @ref back()
         */
        StridedIterator<dimensions, T> end() const {
            return {_data, _size, _stride, _size[0]};
        }
        /** @overload */
        StridedIterator<dimensions, T> cend() const {
            return {_data, _size, _stride, _size[0]};
        }

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref begin()
         */
        ElementType front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref end()
         */
        ElementType back() const;

        /**
         * @brief Array slice
         *
         * Both arguments are expected to be in range. On multi-dimensional
         * views slices just the top-level dimension.
         * @see @ref slice(const Size&, const Size&) const
         */
        StridedArrayView<dimensions, T> slice(std::size_t begin, std::size_t end) const;

        /**
         * @brief Multi-dimensional array slice
         *
         * Values in both arguments are expected to be in range for given
         * dimension. If @p newDimensions is smaller than @ref Dimensions,
         * only the first slice is taken from the remaining dimensions; if
         * @p newDimensions is larger than @ref Dimensions, size of the new
         * dimensions is set to @cpp 1 @ce and and stride to size of @ref Type.
         * @see @ref slice(std::size_t, std::size_t) const, @ref slice() const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> slice(const Size& begin, const Size& end) const;

        /**
         * @brief Expand or shrink dimension count
         *
         * Equivalent to @cpp data.slice<newDimensions>({}, data.size()) @ce.
         * @see @ref slice(const Size&, const Size&) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> slice() const {
            return slice<newDimensions>({}, _size);
        }

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(0, end) @ce. If @p end is @cpp 0 @ce,
         * returns zero-sized @cpp nullptr @ce array.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref prefix(const Size&) const
         */
        StridedArrayView<dimensions, T> prefix(std::size_t end) const {
            if(!end) return nullptr;
            return slice(0, end);
        }

        /**
         * @brief Multi-dimensional array prefix
         *
         * Equivalent to @cpp data.slice<newDimensions>({}, end) @ce. If @p end
         * is @cpp 0 @ce in all dimensions, returns zero-sized @cpp nullptr @ce
         * array.
         * @see @ref slice(const Size&, const Size&) const,
         *      @ref prefix(std::size_t) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> prefix(const Size& end) const {
            if(end == Size{}) return nullptr;
            return slice<newDimensions>({}, end);
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice(begin, data.size()[0]) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref suffix(const Size&) const
         */
        StridedArrayView<dimensions, T> suffix(std::size_t begin) const {
            return slice(begin, _size._data[0]);
        }

        /**
         * @brief Multi-dimensional array suffix
         *
         * Equivalent to @cpp data.slice<newDimensions>(begin, data.size()) @ce.
         * @see @ref slice(const Size&, const Size&) const,
         *      @ref suffix(std::size_t) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> suffix(const Size& begin) const {
            return slice<newDimensions>(begin, _size);
        }

        /**
         * @brief Array suffix
         *
         * Equivalent to @cpp data.slice({}, data.size()[0] - count) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref suffix(const Size&) const
         */
        StridedArrayView<dimensions, T> except(std::size_t count) const {
            return slice({}, _size._data[0] - count);
        }

        /**
         * @brief Multi-dimensional array suffix
         *
         * Equivalent to @cpp data.slice<newDimensions>({}, end) @ce, where
         * @p end is @cpp data.size()[i] - count[i] @ce for all dimensions.
         * @see @ref slice(const Size&, const Size&) const,
         *      @ref except(std::size_t) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> except(const Size& count) const;

        /**
         * @brief Pick every Nth element
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly. Negative @p skip is equivalent to first calling
         * @ref flipped() and then this function with a positive value. On
         * multi-dimensional views affects just the top-level dimension.
         */
        StridedArrayView<dimensions, T> every(std::ptrdiff_t skip) const;

        /**
         * @brief Pick every Nth element
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly. Negative @p skip is equivalent to first calling
         * @ref flipped() and then this function with a positive value.
         */
        StridedArrayView<dimensions, T> every(const Stride& skip) const;

        /**
         * @brief Transpose two dimensions
         *
         * Exchanges dimensions @p dimensionA and @p dimensionB by swapping
         * their size and stride values. Together with @ref flipped() can be
         * used to do arbitrary 90° rotations of the view. This is a
         * non-destructive operation on the view, transposing it again will go
         * back to the original form.
         */
        template<unsigned dimensionA, unsigned dimensionB> StridedArrayView<dimensions, T> transposed() const;

        /**
         * @brief Flip a dimension
         *
         * Flips given @p dimension by making its stride negative and adjusting
         * the internal base data pointer. Together with @ref transposed() can
         * be used to do arbitrary 90° rotations of the view. This is a
         * non-destructive operation on the view, flipping it again will go
         * back to the original form.
         */
        template<unsigned dimension> StridedArrayView<dimensions, T> flipped() const;

        /**
         * @brief Broadcast a dimension
         *
         * Stretches the initial value to @p size in given @p dimension by
         * setting its stride to 0 and size to @p size. To avoid destructive
         * operations on the view, the function expects that size in given
         * dimension is 1. If you need to broadcast a dimension that has more
         * elements, @ref slice() it first.
         */
        template<unsigned dimension> StridedArrayView<dimensions, T> broadcasted(std::size_t size) const;

    private:
        template<unsigned, class> friend class StridedArrayView;

        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<unsigned, class> friend struct Implementation::StridedElement;
        template<bool> friend struct Implementation::ArrayCastFlattenOrInflate;
        template<class U, unsigned dimensions_, class T_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, T_>&);

        /* Internal constructor without type/size checks for things like
           slice() etc. Argument order is different to avoid this function
           getting matched when pass */
        constexpr /*implicit*/ StridedArrayView(const Size& size, const Stride& stride, ErasedType* data) noexcept: _data{data}, _size{size}, _stride{stride} {}

        template<std::size_t ...sequence> constexpr StridedDimensions<dimensions, bool> emptyInternal(Implementation::Sequence<sequence...>) const {
            return StridedDimensions<dimensions, bool>{(_size._data[sequence] == 0)...};
        }

        ErasedType* _data;
        Size _size;
        Stride _stride;
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief One-dimensional strided array view

Convenience alternative to @cpp StridedArrayView<1, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView2D, @ref StridedArrayView3D
*/
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;

/**
@brief Two-dimensional strided array view

Convenience alternative to @cpp StridedArrayView<2, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView1D, @ref StridedArrayView3D
*/
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;

/**
@brief Three-dimensional strided array view

Convenience alternative to @cpp StridedArrayView<3, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView1D, @ref StridedArrayView2D
*/
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;
#endif

/** @relatesalso StridedArrayView
@brief Make a strided view on fixed-size array

Convenience alternative to @ref StridedArrayView::StridedArrayView(U(&)[size]).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-array
*/
template<std::size_t size, class T> constexpr StridedArrayView1D<T> stridedArrayView(T(&data)[size]) {
    /* GCC 4.8 needs this to be explicit */
    return StridedArrayView1D<T>{data};
}

/** @relatesalso StridedArrayView
@brief Make a strided view on @ref ArrayView

Convenience alternative to @ref StridedArrayView::StridedArrayView(ArrayView<U>).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-ArrayView
*/
template<class T> constexpr StridedArrayView1D<T> stridedArrayView(ArrayView<T> view) {
    return view;
}

/** @relatesalso StridedArrayView
@brief Make a strided view on @ref StaticArrayView

Convenience alternative to @ref StridedArrayView::StridedArrayView(StaticArrayView<size, U>).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-StaticArrayView
*/
template<std::size_t size, class T> constexpr StridedArrayView1D<T> stridedArrayView(StaticArrayView<size, T> view) {
    return ArrayView<T>{view};
}

/** @relatesalso StridedArrayView
@brief Make a view on a view

Equivalent to the implicit @ref StridedArrayView copy constructor --- it
shouldn't be an error to call @ref stridedArrayView() on itself.
*/
template<unsigned dimensions, class T> constexpr StridedArrayView<dimensions, T> stridedArrayView(StridedArrayView<dimensions, T> view) {
    return view;
}

/** @relatesalso StridedArrayView
@brief Make a strided view on an external type / from an external representation

@see @ref Containers-ArrayView-stl
*/
/* There's no restriction that would disallow creating StridedArrayView from
   e.g. std::vector<T>&& because that would break uses like `consume(foo());`,
   where `consume()` expects a view but `foo()` returns a std::vector. */
template<class T, class U = decltype(stridedArrayView(Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::declval<T&&>())))> constexpr U stridedArrayView(T&& other) {
    return Implementation::ErasedArrayViewConverter<typename std::remove_reference<T&&>::type>::from(std::forward<T>(other));
}

/** @relatesalso StridedArrayView
@brief Reinterpret-cast a strided array view

Size of the new array is the same as original. Expects that both types are
[standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and @cpp sizeof(U) @ce is not larger than any @ref StridedArrayView::stride() "stride()"
of the original array. Works with negative and zero strides as well, however
note that no type compatibility checks can be done for zero strides, so be
extra careful in that case.

@snippet Containers.cpp arrayCast-StridedArrayView
*/
template<class U, unsigned dimensions, class T> StridedArrayView<dimensions, U> arrayCast(const StridedArrayView<dimensions, T>& view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    #ifndef CORRADE_NO_DEBUG
    for(unsigned i = 0; i != dimensions; ++i) {
        CORRADE_ASSERT(!view._stride._data[i] || sizeof(U) <= std::size_t(view._stride._data[i] < 0 ? -view._stride._data[i] : view._stride._data[i]),
            "Containers::arrayCast(): can't fit a" << sizeof(U) << Utility::Debug::nospace << "-byte type into a stride of" << view._stride._data[i], {});
    }
    #endif
    return StridedArrayView<dimensions, U>{view._size, view._stride, view._data};
}

namespace Implementation {

template<bool> struct ArrayCastFlattenOrInflate;
template<> struct ArrayCastFlattenOrInflate<true> { /* flatten */
    template<unsigned newDimensions, class U, unsigned dimensions, class T> static StridedArrayView<newDimensions, U> cast(const StridedArrayView<dimensions, T>& view) {
        static_assert(newDimensions + 1 == dimensions, "mosra messed up");
        CORRADE_ASSERT(sizeof(T) == std::size_t(view._stride[dimensions - 1]),
            "Containers::arrayCast(): last dimension needs to be tightly packed in order to be flattened, expected stride" << sizeof(T) << "but got" << view.stride()[dimensions - 1], {});
        CORRADE_ASSERT(sizeof(T)*view._size._data[dimensions - 1] == sizeof(U),
            "Containers::arrayCast(): last dimension needs to have byte size equal to new type size in order to be flattened, expected" << sizeof(U) << "but got" << sizeof(T)*view._size._data[dimensions - 1], {});
        return StridedArrayView<newDimensions, U>{
            Containers::StaticArrayView<dimensions, const std::size_t>(view._size).template prefix<newDimensions>(),
            Containers::StaticArrayView<dimensions, const std::ptrdiff_t>(view._stride).template prefix<newDimensions>(),
            view._data};
    }
};
template<> struct ArrayCastFlattenOrInflate<false> { /* inflate */
    template<unsigned newDimensions, class U, unsigned dimensions, class T> static StridedArrayView<newDimensions, U> cast(const StridedArrayView<dimensions, T>& view) {
        static_assert(newDimensions == dimensions + 1, "mosra messed up");
        constexpr std::size_t lastDimensionSize = sizeof(T)/sizeof(U);
        static_assert(sizeof(T) % lastDimensionSize == 0, "original type not a multiply of inflated type");
        std::size_t size[newDimensions];
        std::ptrdiff_t stride[newDimensions];
        size[dimensions] = lastDimensionSize;
        stride[dimensions] = sizeof(U);
        for(std::size_t i = 0; i != dimensions; ++i) {
            size[i] = view._size._data[i];
            stride[i] = view._stride._data[i];
        }
        return StridedArrayView<newDimensions, U>{
            StaticArrayView<newDimensions, const std::size_t>(size), StaticArrayView<newDimensions, const std::ptrdiff_t>(stride),
            view._data};
    }
};

}

/** @relatesalso StridedArrayView
@brief Reinterpret-cast and flatten or inflate a strided array view

If @p newDimensions is one less than @p dimensions, flattens the last dimension
into a tightly packed new type @p U, expecting the last dimension to be tightly
packed and its stride equal to size of @p U. If @p newDimensions is one more
than @p dimensions, inflates the last dimension into the new type @p U, its
element count being ratio of @p T and @p U sizes. This operation can be used
for example to peek into individual channels pixel data:

@snippet Containers.cpp arrayCast-StridedArrayView-inflate
*/
template<unsigned newDimensions, class U, unsigned dimensions, class T> StridedArrayView<newDimensions, U> arrayCast(const StridedArrayView<dimensions, T>& view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    static_assert(newDimensions == dimensions - 1 || newDimensions == dimensions + 1, "can cast only into one more or one less dimension");
    return Implementation::ArrayCastFlattenOrInflate<newDimensions < dimensions>::template cast<newDimensions, U>(view);
}

/**
@brief Strided array view iterator

Used by @ref StridedArrayView to provide iterator access to its items.
*/
template<unsigned dimensions, class T> class StridedIterator {
    public:
        /**
         * @brief Underlying type
         *
         * Underlying data type. See also @ref ElementType.
         */
        typedef T Type;

        /**
         * @brief Element type
         *
         * For @ref StridedArrayView1D iterators equivalent to a reference to
         * @ref Type, for higher dimensions a strided view of one dimension
         * less.
         */
        typedef typename std::conditional<dimensions == 1, T&, StridedArrayView<dimensions - 1, T>>::type ElementType;

        #ifndef DOXYGEN_GENERATING_OUTPUT
        /*implicit*/ StridedIterator(typename std::conditional<std::is_const<T>::value, const void, void>::type* data, const StridedDimensions<dimensions, std::size_t>& size, const StridedDimensions<dimensions, std::ptrdiff_t>& stride, std::size_t i) noexcept: _data{data}, _size{size}, _stride{stride}, _i{i} {}
        #endif

        /** @brief Equality comparison */
        bool operator==(StridedIterator<dimensions, T> other) const {
            return _data == other._data && _i == other._i;
        }

        /** @brief Non-equality comparison */
        bool operator!=(StridedIterator<dimensions, T> other) const {
            return _data != other._data || _i != other._i;
        }

        /** @brief Less than comparison */
        bool operator<(StridedIterator<dimensions, T> other) const {
            return _data == other._data && _i < other._i;
        }

        /** @brief Less than or equal comparison */
        bool operator<=(StridedIterator<dimensions, T> other) const {
            return _data == other._data && _i <= other._i;
        }

        /** @brief Greater than comparison */
        bool operator>(StridedIterator<dimensions, T> other) const {
            return _data == other._data && _i > other._i;
        }

        /** @brief Greater than or equal comparison */
        bool operator>=(StridedIterator<dimensions, T> other) const {
            return _data == other._data && _i >= other._i;
        }

        /** @brief Add an offset */
        StridedIterator<dimensions, T> operator+(std::ptrdiff_t i) const {
            return {_data, _size, _stride, _i + i};
        }

        /** @brief Subtract an offset */
        StridedIterator<dimensions, T> operator-(std::ptrdiff_t i) const {
            return {_data, _size, _stride, _i - i};
        }

        /** @brief Iterator difference */
        std::ptrdiff_t operator-(StridedIterator<dimensions, T> it) const {
            return _i - it._i;
        }

        /** @brief Go back to previous position */
        StridedIterator<dimensions, T>& operator--() {
            --_i;
            return *this;
        }

        /** @brief Advance to next position */
        StridedIterator<dimensions, T>& operator++() {
            ++_i;
            return *this;
        }

        /** @brief Dereference */
        ElementType operator*() const {
            return Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, _i);
        }

    private:
        typename std::conditional<std::is_const<T>::value, const void, void>::type* _data;
        StridedDimensions<dimensions, std::size_t> _size;
        StridedDimensions<dimensions, std::ptrdiff_t> _stride;
        std::size_t _i;
};

/** @relates StridedIterator
@brief Add strided iterator to an offset
*/
template<unsigned dimensions, class T> inline StridedIterator<dimensions, T> operator+(std::ptrdiff_t i, StridedIterator<dimensions, T> it) {
    return it + i;
}

namespace Implementation {
    template<unsigned dimensions, class T> struct StridedElement {
        static StridedArrayView<dimensions - 1, T> get(typename std::conditional<std::is_const<T>::value, const void, void>::type* data, const StridedDimensions<dimensions, std::size_t>& size, const StridedDimensions<dimensions, std::ptrdiff_t>& stride, std::size_t i) {
            return StridedArrayView<dimensions - 1, T>{
                Containers::StaticArrayView<dimensions, const std::size_t>(size).template suffix<1>(),
                Containers::StaticArrayView<dimensions, const std::ptrdiff_t>(stride).template suffix<1>(),
                static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(data) + i*stride._data[0]};
        }
    };
    template<class T> struct StridedElement<1, T> {
        static T& get(typename std::conditional<std::is_const<T>::value, const void, void>::type* data, const StridedDimensions<1, std::size_t>&, const StridedDimensions<1, std::ptrdiff_t>& stride, std::size_t i) {
            return *(reinterpret_cast<T*>(static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(data) + i*stride._data[0]));
        }
    };
}

template<unsigned dimensions, class T> auto StridedArrayView<dimensions, T>::operator[](const std::size_t i) const -> ElementType {
    CORRADE_ASSERT(i < _size._data[0], "Containers::StridedArrayView::operator[](): index" << i << "out of range for" << _size._data[0] << "elements", (Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, i)));
    return Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, i);
}

template<unsigned dimensions, class T> auto StridedArrayView<dimensions, T>::front() const -> ElementType {
    CORRADE_ASSERT(_size[0], "Containers::StridedArrayView::front(): view is empty", (Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, 0)));
    return Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, 0);
}

template<unsigned dimensions, class T> auto StridedArrayView<dimensions, T>::back() const -> ElementType {
    CORRADE_ASSERT(_size[0], "Containers::StridedArrayView::back(): view is empty", (Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, _size._data[0] - 1)));
    return Implementation::StridedElement<dimensions, T>::get(_data, _size, _stride, _size._data[0] - 1);
}

template<unsigned dimensions, class T> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::slice(std::size_t begin, std::size_t end) const {
    CORRADE_ASSERT(begin <= end && end <= _size._data[0],
        "Containers::StridedArrayView::slice(): slice [" << Utility::Debug::nospace
        << begin << Utility::Debug::nospace << ":"
        << Utility::Debug::nospace << end << Utility::Debug::nospace
        << "] out of range for" << _size._data[0] << "elements", {});
    Size size = _size;
    size._data[0] = std::size_t(end - begin);
    return StridedArrayView<dimensions, T>{size, _stride,
        static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(_data) + begin*_stride[0]};
}

template<unsigned dimensions, class T> template<unsigned newDimensions> StridedArrayView<newDimensions, T> StridedArrayView<dimensions, T>::slice(const Size& begin, const Size& end) const {
    constexpr unsigned minDimensions = dimensions < newDimensions ? dimensions : newDimensions;
    StridedDimensions<newDimensions, std::size_t> size{NoInit};
    StridedDimensions<newDimensions, std::ptrdiff_t> stride{NoInit};
    auto data = static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(_data);

    /* Adjust data pointer based on offsets of all source dimensions */
    for(std::size_t i = 0; i != dimensions; ++i) {
        CORRADE_ASSERT(begin._data[i] <= end._data[i] && end._data[i] <= _size._data[i],
            "Containers::StridedArrayView::slice(): slice [" << Utility::Debug::nospace
            << begin << Utility::Debug::nospace << ":"
            << Utility::Debug::nospace << end << Utility::Debug::nospace
            << "] out of range for" << _size << "elements in dimension" << i,
            {});
        data += begin._data[i]*_stride[i];
    }

    /* Set size and stride values for all destination dimensions that are in
       source as well */
    for(std::size_t i = 0; i != minDimensions; ++i) {
        size._data[i] = std::size_t(end._data[i] - begin._data[i]);
        stride._data[i] = _stride._data[i];
    }

    /* Set size in the extra dimensions to 1 and stride to type size */
    for(std::size_t i = minDimensions; i < newDimensions; ++i) {
        size._data[i] = 1;
        stride._data[i] = sizeof(T);
    }

    return StridedArrayView<newDimensions, T>{size, stride, data};
}

template<unsigned dimensions, class T> template<unsigned newDimensions> StridedArrayView<newDimensions, T> StridedArrayView<dimensions, T>::except(const Size& count) const {
    Size end{NoInit};
    for(std::size_t i = 0; i != dimensions; ++i)
        end._data[i] = _size._data[i] - count._data[i];
    return slice<newDimensions>({}, end);
}

template<unsigned dimensions, class T> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::every(const std::ptrdiff_t step) const {
    Stride steps;
    steps[0] = step;
    for(std::size_t i = 1; i != dimensions; ++i) steps[i] = 1;
    return every(steps);
}

template<unsigned dimensions, class T> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::every(const Stride& step) const {
    ErasedType* data = _data;
    Size size = _size;
    Stride stride = _stride;
    for(std::size_t dimension = 0; dimension != dimensions; ++dimension) {
        CORRADE_ASSERT(step[dimension], "Containers::StridedArrayView::every(): step in dimension" << dimension << "is zero", {});

        /* If step is negative, adjust also data pointer */
        std::size_t divisor;
        if(step[dimension] < 0) {
            data = static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(data) + _stride._data[dimension]*(_size._data[dimension] ? _size._data[dimension] - 1 : 0);
            divisor = -step[dimension];
        } else divisor = step[dimension];

        /* Taking every 5th element of a 6-element array should result in 2
           elements */
        size[dimension] = (size[dimension] + divisor - 1)/divisor;
        stride[dimension] *= step[dimension];
    }

    return StridedArrayView<dimensions, T>{size, stride, data};
}

template<unsigned dimensions, class T> template<unsigned dimensionA, unsigned dimensionB> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::transposed() const {
    static_assert(dimensionA < dimensions && dimensionB < dimensions,
        "dimensions out of range");

    Size size = _size;
    Stride stride = _stride;
    std::swap(size._data[dimensionA], size._data[dimensionB]);
    std::swap(stride._data[dimensionA], stride._data[dimensionB]);
    return StridedArrayView{size, stride, _data};
}

template<unsigned dimensions, class T> template<unsigned dimension> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::flipped() const {
    static_assert(dimension < dimensions, "dimension out of range");

    ErasedType* data = static_cast<typename std::conditional<std::is_const<T>::value, const char, char>::type*>(_data) + _stride._data[dimension]*(_size._data[dimension] ? _size._data[dimension] - 1 : 0);
    Stride stride = _stride;
    stride._data[dimension] *= -1;
    return StridedArrayView{_size, stride, data};
}

template<unsigned dimensions, class T> template<unsigned dimension> StridedArrayView<dimensions, T> StridedArrayView<dimensions, T>::broadcasted(std::size_t size) const {
    static_assert(dimension < dimensions, "dimension out of range");
    CORRADE_ASSERT(_size._data[dimension] == 1,
        "Containers::StridedArrayView::broadcasted(): can't broadcast dimension" << dimension << "with" << _size._data[dimension] << "elements", {});

    Size size_ = _size;
    size_._data[dimension] = size;
    Stride stride = _stride;
    stride._data[dimension] = 0;
    return StridedArrayView{size_, stride, _data};
}

}}

#endif
