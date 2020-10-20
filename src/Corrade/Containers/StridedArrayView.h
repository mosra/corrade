#ifndef Corrade_Containers_StridedArrayView_h
#define Corrade_Containers_StridedArrayView_h
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
 * @brief Class @ref Corrade::Containers::StridedArrayView, @ref Corrade::Containers::StridedIterator, alias @ref Corrade::Containers::StridedArrayView1D, @ref Corrade::Containers::StridedArrayView2D, @ref Corrade::Containers::StridedArrayView3D, @ref Corrade::Containers::StridedArrayView4D
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
    template<int> struct ArrayCastFlattenOrInflate;

    /* Used in the assertion that data array is large enough. If any size
       element is zero, the data can be zero-sized as well. Other we have to
       compare against max stride. */
    template<unsigned dimensions> constexpr bool isAnySizeZero(const StridedDimensions<dimensions, std::size_t>&, Sequence<>) {
        return false;
    }
    template<unsigned dimensions, std::size_t first, std::size_t ...next> constexpr bool isAnySizeZero(const StridedDimensions<dimensions, std::size_t>& size, Sequence<first, next...>) {
        return !size[first] || isAnySizeZero(size, Sequence<next...>{});
    }
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

    /* Calculates stride when just size is passed */
    template<unsigned dimensions> constexpr std::ptrdiff_t strideForSizeInternal(const StridedDimensions<dimensions, std::size_t>&, std::size_t, Sequence<>) {
        return 1;
    }
    template<unsigned dimensions, std::size_t first, std::size_t ...next> constexpr std::ptrdiff_t strideForSizeInternal(const StridedDimensions<dimensions, std::size_t>& size, std::size_t index, Sequence<first, next...>) {
        /* GCC since version 10.2 complains that
            warning: comparison of unsigned expression in ‘< 0’ is always false [-Wtype-limits]
           and there's no way to silence that except for a pragma (doing things
           like `first && first > index` doesn't change anything). There was
           nothing in the 10.2 changelog mentioning this and the only vaguely
           relevant bug is https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95148
           (which complains about the inability to circumvent this, but not
           about the stupidity of this warning being trigerred in a template
           code) */
        #if defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 102
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wtype-limits"
        #endif
        return (first > index ? size[first] : 1)*strideForSizeInternal(size, index, Sequence<next...>{});
        #if defined(CORRADE_TARGET_GCC) && __GNUC__*100 + __GNUC_MINOR__ >= 102
        #pragma GCC diagnostic pop
        #endif
    }
    template<unsigned dimensions, std::size_t ...index> constexpr StridedDimensions<dimensions, std::ptrdiff_t> strideForSize(const StridedDimensions<dimensions, std::size_t>& size, std::size_t typeSize, Sequence<index...>) {
        return {std::ptrdiff_t(typeSize)*strideForSizeInternal(size, index, typename GenerateSequence<dimensions>::Type{})...};
    }
    #ifndef CORRADE_NO_PYTHON_COMPATIBILITY
    /* so Python buffer protocol can point to the size / stride members */
    template<unsigned dimensions, class T> StridedDimensions<dimensions, std::size_t>& sizeRef(StridedArrayView<dimensions, T>& view) {
        return view._size;
    }
    template<unsigned dimensions, class T> StridedDimensions<dimensions, std::ptrdiff_t>& strideRef(StridedArrayView<dimensions, T>& view) {
        return view._stride;
    }
    #endif
}

/**
@brief Helper for specifying sizes and strides for @ref StridedArrayView
@m_since{2019,10}

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
            return staticArrayView(_data);
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

/**
@brief Multi-dimensional array view with size and stride information

Immutable wrapper around continuous sparse range of data, useful for easy
iteration over interleaved arrays and for describing multi-dimensional data.
Usage example:

@snippet Containers.cpp StridedArrayView-usage

For convenience, similarly to @ref ArrayView, this class is implicitly
convertible from plain C arrays, @ref ArrayView and @ref StaticArrayView, with
stride equal to array type size. The following two statements are equivalent:

@snippet Containers.cpp StridedArrayView-usage-conversion

When constructing, if you don't specify the stride, the constructor assumes
contiguous data and calculates the stride automatically --- stride of a
dimension is stride of the next dimension times next dimension size, while last
dimension stride is implicitly @cpp sizeof(T) @ce. This is especially useful
for "reshaping" linear data as a multi-dimensional view. Again, the following
two statements are equivalent:

@snippet Containers.cpp StridedArrayView-usage-reshape

Unlike @ref ArrayView, this wrapper doesn't provide direct pointer access
because pointer arithmetic doesn't work as usual here. The
@ref arrayCast(const StridedArrayView<dimensions, T>&) overload also works
slightly differently with strided arrays --- it checks that a type fits into
the stride instead of expecting the total byte size to stay the same.

@section Containers-StridedArrayView-multidimensional Multi-dimensional views

Strided array views are very useful for describing and iteration over
multi-dimensional data such as 2D (sub)images. In that case, @ref operator[]()
and iterator access return a view of one dimension less instead of a direct
element reference, and there are @ref slice(const Size&, const Size&) const,
@ref prefix(const Size&) const and @ref suffix(const Size&) const overloads
working on all dimensions at the same time.

@snippet Containers.cpp StridedArrayView-usage-3d

Both the subscription operator and the slicing operations allow you to change
the view dimension count --- for example, obtaining the fifth image or just a
view on the (now red) center of it. Conversely, it's possible to turn a
lower-dimensional view into a slice in a higher-dimensional view.

@snippet Containers.cpp StridedArrayView-usage-3d-slice-2d

Finally, since the actual view elements can be also non-scalar data, there's an
overload of @ref arrayCast(const StridedArrayView<dimensions, T>&) that can
"extract" an additional dimension out of these or, on the other hand, flatten
it back if the last dimension is contiguous.

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

@anchor Containers-StridedArrayView-single-header

<b></b>

@m_class{m-block m-success}

@par Single-header version
    This class, together with the @ref StridedArrayView<dimensions, void> /
    @ref StridedArrayView<dimensions, const void> specializations is also
    available as a single-header [CorradeStridedArrayView.h](https://github.com/mosra/magnum-singles/tree/master/CorradeStridedArrayView.h)
    library in the Magnum Singles repository for easier integration into your
    projects. See @ref corrade-singles for more information.

@see @ref StridedArrayView<dimensions, void>,
    @ref StridedArrayView<dimensions, const void>, @ref StridedIterator,
    @ref StridedArrayView1D, @ref StridedArrayView2D, @ref StridedArrayView3D,
    @ref StridedArrayView4D
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

        /**
         * @brief Size values
         * @m_since{2019,10}
         */
        typedef StridedDimensions<dimensions, std::size_t> Size;

        /**
         * @brief Stride values
         * @m_since{2019,10}
         */
        typedef StridedDimensions<dimensions, std::ptrdiff_t> Stride;

        enum: unsigned {
            /**
             * View dimensions
             * @m_since{2019,10}
             */
            Dimensions = dimensions
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
        constexpr /*implicit*/ StridedArrayView(ArrayView<ErasedType> data, T* member, const Size& size, const Stride& stride) noexcept: _data{(
            /** @todo can't compare void pointers to check if member is in data,
                    it's not constexpr :( */
            /* If any size is zero, data can be zero-sized too. If the largest
               stride is zero, `data` can have *any* size and it could be okay,
               can't reliably test that */
            CORRADE_CONSTEXPR_ASSERT(Implementation::isAnySizeZero(size, typename Implementation::GenerateSequence<dimensions>::Type{}) || Implementation::largestStride(size, stride, typename Implementation::GenerateSequence<dimensions>::Type{}) <= data.size(),
                "Containers::StridedArrayView: data size" << data.size() << "is not enough for" << size << "elements of stride" << stride)
            #ifdef CORRADE_NO_ASSERT
            , static_cast<void>(data)
            #endif
            , member)}, _size{size}, _stride{stride} {}

        /**
         * @brief Construct a view with explicit size and stride
         * @m_since{2019,10}
         *
         * Equivalent to calling @ref StridedArrayView(ArrayView<ErasedType>, T*, const Size&, const Stride&)
         * with @p data as the first parameter and @cpp data.data() @ce as the
         * second parameter.
         */
        constexpr /*implicit*/ StridedArrayView(ArrayView<T> data, const Size& size, const Stride& stride) noexcept: StridedArrayView{data, data.data(), size, stride} {}

        /**
         * @brief Construct a view with explicit size
         * @m_since{2019,10}
         *
         * Assuming @p data are contiguous, stride is calculated implicitly
         * from @p size --- stride of a dimension is stride of the next
         * dimension times next dimension size, while last dimension stride is
         * implicitly @cpp sizeof(T) @ce.
         */
        constexpr /*implicit*/ StridedArrayView(ArrayView<T> data, const Size& size) noexcept: StridedArrayView{data, data.data(), size, Implementation::strideForSize(size, sizeof(T), typename Implementation::GenerateSequence<dimensions>::Type{})} {}

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
         * @brief Construct a view on an external type
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

        /**
         * @brief Whether the view is contiguous from given dimension further
         * @m_since{2020,06}
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
         * @m_since{2020,06}
         *
         * Returns a view large as the product of sizes in all dimensions.
         * Expects that the view is contiguous.
         * @see @ref isContiguous() const
         */
        ArrayView<T> asContiguous() const;

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
         * @brief Slice to a member
         * @m_since_latest
         *
         * Returns a view on a particular structure member. Example usage:
         *
         * @snippet Containers.cpp StridedArrayView-slice-member
         *
         * @see @ref StridedArrayView::StridedArrayView(ArrayView<ErasedType>, T*, const Size&, const Stride&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class U> StridedArrayView<dimensions, U> slice(U T::*member) const;
        #else
        template<class U, class V = T, class = typename std::enable_if<std::is_class<V>::value || std::is_union<V>::value>::type> StridedArrayView<dimensions, U> slice(U V::*member) const {
            return StridedArrayView<dimensions, U>{_size, _stride, &(static_cast<T*>(_data)->*member)};
        }
        #endif

        /**
         * @brief Array prefix
         *
         * Equivalent to @cpp data.slice(0, end) @ce.
         * @see @ref slice(std::size_t, std::size_t) const,
         *      @ref prefix(const Size&) const
         */
        StridedArrayView<dimensions, T> prefix(std::size_t end) const {
            return slice(0, end);
        }

        /**
         * @brief Multi-dimensional array prefix
         *
         * Equivalent to @cpp data.slice<newDimensions>({}, end) @ce.
         * @see @ref slice(const Size&, const Size&) const,
         *      @ref prefix(std::size_t) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> prefix(const Size& end) const {
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
         * @m_since{2019,10}
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
         * @m_since{2019,10}
         *
         * Equivalent to @cpp data.slice<newDimensions>({}, end) @ce, where
         * @p end is @cpp data.size()[i] - count[i] @ce for all dimensions.
         * @see @ref slice(const Size&, const Size&) const,
         *      @ref except(std::size_t) const
         */
        template<unsigned newDimensions = dimensions> StridedArrayView<newDimensions, T> except(const Size& count) const;

        /**
         * @brief Pick every Nth element
         * @m_since{2019,10}
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly. Negative @p skip is equivalent to first calling
         * @ref flipped() and then this function with a positive value. On
         * multi-dimensional views affects just the top-level dimension.
         */
        StridedArrayView<dimensions, T> every(std::ptrdiff_t skip) const;

        /**
         * @brief Pick every Nth element
         * @m_since{2019,10}
         *
         * Multiplies @ref stride() with @p skip and adjusts @ref size()
         * accordingly. Negative @p skip is equivalent to first calling
         * @ref flipped() and then this function with a positive value.
         */
        StridedArrayView<dimensions, T> every(const Stride& skip) const;

        /**
         * @brief Transpose two dimensions
         * @m_since{2019,10}
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
         * @m_since{2019,10}
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
         * @m_since{2019,10}
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

        #ifndef CORRADE_NO_PYTHON_COMPATIBILITY
        /* so Python buffer protocol can point to the size / stride members */
        friend StridedDimensions<dimensions, std::size_t>& Implementation::sizeRef<>(StridedArrayView<dimensions, T>&);
        friend StridedDimensions<dimensions, std::ptrdiff_t>& Implementation::strideRef<>(StridedArrayView<dimensions, T>&);
        #endif
        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<unsigned, class> friend struct Implementation::StridedElement;
        template<int> friend struct Implementation::ArrayCastFlattenOrInflate;
        template<class U, unsigned dimensions_, class T_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, T_>&);
        template<class U, unsigned dimensions_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, void>&);
        template<class U, unsigned dimensions_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, const void>&);
        template<unsigned newDimensions, class U, unsigned dimensions_> StridedArrayView<newDimensions, U> friend arrayCast(const StridedArrayView<dimensions_, void>&, std::size_t);
        template<unsigned newDimensions, class U, unsigned dimensions_> StridedArrayView<newDimensions, U> friend arrayCast(const StridedArrayView<dimensions_, const void>&, std::size_t);

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

/**
@brief Multi-dimensional void array view with size and stride information
@m_since{2020,06}

Specialization of @ref StridedArrayView which is convertible from a
compile-time array, @ref Array, @ref ArrayView, @ref StaticArrayView or
@ref StridedArrayView of any non-constant type and also any non-constant type
convertible to them. For arrays and non-strided views, size is taken as element
count of the original array and stride becomes the original type size; for
strided views the size and stride is taken as-is. This specialization doesn't
provide any accessors besides @ref data(), because it has no use for the
@cpp void @ce type. Instead, use @ref arrayCast(const StridedArrayView<dimensions, void>&)
to first cast the array to a concrete type and then access the particular
elements.

@m_class{m-block m-success}

@par Single-header version
    This specialization, together with @ref StridedArrayView<dimensions, const void>
    and the generic @ref StridedArrayView is also available as a single-header,
    dependency-less library in the Magnum Singles repository for easier
    integration into your projects. See the
    @ref Containers-StridedArrayView-single-header "StridedArrayView documentation"
    for more information.
*/
template<unsigned dimensions> class StridedArrayView<dimensions, void> {
    static_assert(dimensions, "can't have a zero-dimensional view");

    public:
        /**
         * @brief Underlying type
         *
         * Underlying data type. See also and @ref ErasedType.
         */
        typedef void Type;

        /**
         * @brief Erased type
         *
         * Equivalent to @ref Type
         */
        typedef void ErasedType;

        /** @brief Size values */
        typedef StridedDimensions<dimensions, std::size_t> Size;

        /** @brief Stride values */
        typedef StridedDimensions<dimensions, std::ptrdiff_t> Stride;

        enum: unsigned {
            Dimensions = dimensions     /**< View dimensions */
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
        constexpr /*implicit*/ StridedArrayView(ArrayView<void> data, void* member, const Size& size, const Stride& stride) noexcept: _data{(
            /** @todo can't compare void pointers to check if member is in data,
                    it's not constexpr :( */
            /* If any size is zero, data can be zero-sized too. If the largest
               stride is zero, `data` can have *any* size and it could be okay,
               can't reliably test that */
            CORRADE_CONSTEXPR_ASSERT(Implementation::isAnySizeZero(size, typename Implementation::GenerateSequence<dimensions>::Type{}) || Implementation::largestStride(size, stride, typename Implementation::GenerateSequence<dimensions>::Type{}) <= data.size(),
                "Containers::StridedArrayView: data size" << data.size() << "is not enough for" << size << "elements of stride" << stride)
            #ifdef CORRADE_NO_ASSERT
            , static_cast<void>(data)
            #endif
            , member)}, _size{size}, _stride{stride} {}

        /**
         * @brief Construct a view with explicit size and stride
         *
         * Equivalent to calling @ref StridedArrayView(ArrayView<void>, void*, const Size&, const Stride&)
         * with @p data as the first parameter and @cpp data.data() @ce as the
         * second parameter.
         */
        constexpr /*implicit*/ StridedArrayView(ArrayView<void> data, const Size& size, const Stride& stride) noexcept: StridedArrayView{data, data.data(), size, stride} {}

        /* size-only constructor not provided for void overloads as there's
           little chance one would want an implicit stride of 1 */

        /**
         * @brief Construct a void view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only on one-dimensional views. Size is set to @p size,
         * stride is to @cpp sizeof(T) @ce.
         */
        template<class T, std::size_t size
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , unsigned d = dimensions, class = typename std::enable_if<d == 1 && !std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ StridedArrayView(T(&data)[size]) noexcept: _data{data}, _size{size}, _stride{sizeof(T)} {}

        /** @brief Construct a void view on any @ref StridedArrayView */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<!std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ StridedArrayView(StridedArrayView<dimensions, T> view) noexcept: _data{view._data}, _size{view._size}, _stride{view._stride} {}

        /**
         * @brief Construct a void view on any @ref ArrayView
         *
         * Enabled only on one-dimensional views. Size is set to @p view's
         * size, stride is to @cpp sizeof(T) @ce.
         */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , unsigned d = dimensions, class = typename std::enable_if<d == 1 && !std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ StridedArrayView(ArrayView<T> view) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(T)} {}

        /**
         * @brief Construct a void view on any @ref StaticArrayView
         *
         * Enabled only on one-dimensional views. Size is set to @p view's
         * size, stride is to @cpp sizeof(T) @ce.
         */
        template<std::size_t size, class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , unsigned d = dimensions, class = typename std::enable_if<d == 1 && !std::is_const<T>::value>::type
            #endif
        > constexpr /*implicit*/ StridedArrayView(StaticArrayView<size, T> view) noexcept: _data{view.data()}, _size{size}, _stride{sizeof(T)} {}

        /**
         * @brief Construct a view on an external type
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
        template<class T, unsigned d = dimensions, class = typename std::enable_if<d == 1, decltype(Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(std::declval<T&&>()))>::type> constexpr /*implicit*/ StridedArrayView(T&& other) noexcept: StridedArrayView{Implementation::ErasedArrayViewConverter<typename std::decay<T&&>::type>::from(other)} {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

        /** @brief Array data */
        constexpr void* data() const { return _data; }

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

    private:
        template<unsigned, class> friend class StridedArrayView;

        /* Internal constructor without type/size checks for things like
           slice() etc. Argument order is different to avoid this function
           getting matched when pass */
        constexpr /*implicit*/ StridedArrayView(const Size& size, const Stride& stride, void* data) noexcept: _data{data}, _size{size}, _stride{stride} {}

        template<std::size_t ...sequence> constexpr StridedDimensions<dimensions, bool> emptyInternal(Implementation::Sequence<sequence...>) const {
            return StridedDimensions<dimensions, bool>{(_size._data[sequence] == 0)...};
        }

        void* _data;
        Size _size;
        Stride _stride;
};

/**
@brief Multi-dimensional const void array view with size and stride information
@m_since{2020,06}

Specialization of @ref StridedArrayView which is convertible from a
compile-time array, @ref Array, @ref ArrayView, @ref StaticArrayView or
@ref StridedArrayView of any type and also any type convertible to them. For
arrays and non-strided views, size is taken as element count of the original
array and  stride becomes the original type size; for strided views the size
and stride is taken as-is. This specialization doesn't provide any
accessors besides @ref data(), because it has no use for the @cpp void @ce
type. Instead, use @ref arrayCast(const StridedArrayView<dimensions, const void>&)
to first cast the array to a concrete type and then access the particular
elements.

@m_class{m-block m-success}

@par Single-header version
    This specialization, together with @ref StridedArrayView<dimensions, void>
    and the generic @ref StridedArrayView is also available as a single-header,
    dependency-less library in the Magnum Singles repository for easier
    integration into your projects. See the
    @ref Containers-StridedArrayView-single-header "StridedArrayView documentation"
    for more information.
*/
template<unsigned dimensions> class StridedArrayView<dimensions, const void> {
    static_assert(dimensions, "can't have a zero-dimensional view");

    public:
        /**
         * @brief Underlying type
         *
         * Underlying data type. See also and @ref ErasedType.
         */
        typedef const void Type;

        /**
         * @brief Erased type
         *
         * Equivalent to @ref Type
         */
        typedef const void ErasedType;

        /** @brief Size values */
        typedef StridedDimensions<dimensions, std::size_t> Size;

        /** @brief Stride values */
        typedef StridedDimensions<dimensions, std::ptrdiff_t> Stride;

        enum: unsigned {
            Dimensions = dimensions     /**< View dimensions */
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
        constexpr /*implicit*/ StridedArrayView(ArrayView<const void> data, const void* member, const Size& size, const Stride& stride) noexcept: _data{(
            /** @todo can't compare void pointers to check if member is in data,
                    it's not constexpr :( */
            /* If any size is zero, data can be zero-sized too. If the largest
               stride is zero, `data` can have *any* size and it could be okay,
               can't reliably test that */
            CORRADE_CONSTEXPR_ASSERT(Implementation::isAnySizeZero(size, typename Implementation::GenerateSequence<dimensions>::Type{}) || Implementation::largestStride(size, stride, typename Implementation::GenerateSequence<dimensions>::Type{}) <= data.size(),
                "Containers::StridedArrayView: data size" << data.size() << "is not enough for" << size << "elements of stride" << stride)
            #ifdef CORRADE_NO_ASSERT
            , static_cast<void>(data)
            #endif
            , member)}, _size{size}, _stride{stride} {}

        /* size-only constructor not provided for void overloads as there's
           little chance one would want an implicit stride of 1 */

        /**
         * @brief Construct a view with explicit size and stride
         *
         * Equivalent to calling @ref StridedArrayView(ArrayView<const void>, const void*, const Size&, const Stride&)
         * with @p data as the first parameter and @cpp data.data() @ce as the
         * second parameter.
         */
        constexpr /*implicit*/ StridedArrayView(ArrayView<const void> data, const Size& size, const Stride& stride) noexcept: StridedArrayView{data, data.data(), size, stride} {}

        /**
         * @brief Construct a const void view on a fixed-size array
         * @param data      Fixed-size array
         *
         * Enabled only on one-dimensional views. Size is set to @p size,
         * stride is to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class T, std::size_t size>
        #else
        template<class T, std::size_t size, unsigned d = dimensions, class = typename std::enable_if<d == 1>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(T(&data)[size]) noexcept: _data{data}, _size{size}, _stride{sizeof(T)} {}

        /** @brief Construct a const void view on any @ref StridedArrayView */
        template<class T> constexpr /*implicit*/ StridedArrayView(StridedArrayView<dimensions, T> view) noexcept: _data{view._data}, _size{view._size}, _stride{view._stride} {}

        /**
         * @brief Construct a const void view on any @ref ArrayView
         *
         * Enabled only on one-dimensional views. Size is set to @p view's
         * size, stride is to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class T>
        #else
        template<class T, unsigned d = dimensions, class = typename std::enable_if<d == 1>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(ArrayView<T> view) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(T)} {}

        /**
         * @brief Construct a const void view on any @ref StaticArrayView
         *
         * Enabled only on one-dimensional views. Size is set to @p view's
         * size, stride is to @cpp sizeof(T) @ce.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<std::size_t size, class T>
        #else
        template<std::size_t size, class T, unsigned d = dimensions, class = typename std::enable_if<d == 1>::type>
        #endif
        constexpr /*implicit*/ StridedArrayView(StaticArrayView<size, T> view) noexcept: _data{view.data()}, _size{size}, _stride{sizeof(T)} {}

        /**
         * @brief Construct a view on an external type
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
        template<class T, unsigned d = dimensions, class = typename std::enable_if<d == 1, decltype(Implementation::ErasedArrayViewConverter<const T>::from(std::declval<const T&>()))>::type> constexpr /*implicit*/ StridedArrayView(const T& other) noexcept: StridedArrayView{Implementation::ErasedArrayViewConverter<const T>::from(other)} {}

        /** @brief Whether the array is non-empty */
        constexpr explicit operator bool() const { return _data; }

        /** @brief Array data */
        constexpr const void* data() const { return _data; }

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

    private:
        template<unsigned, class> friend class StridedArrayView;

        /* Basically just so these can access the _size / _stride without going
           through getters (which additionally flatten their types for 1D) */
        template<class U, unsigned dimensions_> friend StridedArrayView<dimensions_, U> arrayCast(const StridedArrayView<dimensions_, const void>&);
        template<unsigned newDimensions, class U, unsigned dimensions_> StridedArrayView<newDimensions, U> friend arrayCast(const StridedArrayView<dimensions_, const void>&, std::size_t);

        /* Internal constructor without type/size checks for things like
           slice() etc. Argument order is different to avoid this function
           getting matched when pass */
        constexpr /*implicit*/ StridedArrayView(const Size& size, const Stride& stride, const void* data) noexcept: _data{data}, _size{size}, _stride{stride} {}

        template<std::size_t ...sequence> constexpr StridedDimensions<dimensions, bool> emptyInternal(Implementation::Sequence<sequence...>) const {
            return StridedDimensions<dimensions, bool>{(_size._data[sequence] == 0)...};
        }

        const void* _data;
        Size _size;
        Stride _stride;
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
/**
@brief One-dimensional strided array view
@m_since{2019,10}

Convenience alternative to @cpp StridedArrayView<1, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView2D, @ref StridedArrayView3D, @ref StridedArrayView4D
*/
template<class T> using StridedArrayView1D = StridedArrayView<1, T>;

/**
@brief Two-dimensional strided array view
@m_since{2019,10}

Convenience alternative to @cpp StridedArrayView<2, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView1D, @ref StridedArrayView3D, @ref StridedArrayView4D
*/
template<class T> using StridedArrayView2D = StridedArrayView<2, T>;

/**
@brief Three-dimensional strided array view
@m_since{2019,10}

Convenience alternative to @cpp StridedArrayView<3, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView1D, @ref StridedArrayView2D, @ref StridedArrayView4D
*/
template<class T> using StridedArrayView3D = StridedArrayView<3, T>;

/**
@brief Four-dimensional strided array view
@m_since{2019,10}

Convenience alternative to @cpp StridedArrayView<4, T> @ce. See
@ref StridedArrayView for more information.
@see @ref StridedArrayView1D, @ref StridedArrayView2D, @ref StridedArrayView3D
*/
template<class T> using StridedArrayView4D = StridedArrayView<4, T>;
#endif

/** @relatesalso StridedArrayView
@brief Make an one-dimensional strided view
@m_since{2020,06}

Convenience alternative to @ref StridedArrayView::StridedArrayView(ArrayView<ErasedType>, T*, const Size&, const Stride&).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-data-member

See also @ref StridedArrayView::slice() const for slicing into @cpp struct @ce
members.
*/
template<class T> constexpr StridedArrayView1D<T> stridedArrayView(ArrayView<typename StridedArrayView1D<T>::ErasedType> data, T* member, std::size_t size, std::ptrdiff_t stride) {
    return StridedArrayView1D<T>{data, member, size, stride};
}

#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
/* Otherwise GCC 4.8 (and maybe 4.9?) would need an explicit arrayView(data),
   which is too annoying. I would ideally also to do this for Array, but that
   would introduce a header dependency :( */
template<std::size_t size_, class T, class U> constexpr StridedArrayView1D<T> stridedArrayView(U(&data)[size_], T* member, std::size_t size, std::ptrdiff_t stride) {
    return StridedArrayView1D<T>{data, member, size, stride};
}
#endif

/** @relatesalso StridedArrayView
@brief Make a strided view on a fixed-size array
@m_since{2019,10}

Convenience alternative to @ref StridedArrayView::StridedArrayView(U(&)[size]).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-array
*/
template<std::size_t size, class T> constexpr StridedArrayView1D<T> stridedArrayView(T(&data)[size]) {
    /* GCC 4.8 needs this to be explicit */
    return StridedArrayView1D<T>{data};
}

/** @relatesalso StridedArrayView
@brief Make a strided view on an initializer list
@m_since{2020,06}

Not present as a constructor in order to avoid accidental dangling references
with r-value initializer lists. See
@ref Containers-ArrayView-initializer-list "ArrayView documentation" for more
information.
*/
template<class T> StridedArrayView1D<const T> stridedArrayView(std::initializer_list<T> list) {
    return StridedArrayView1D<const T>{arrayView(list)};
}

/** @relatesalso StridedArrayView
@brief Make a strided view on @ref ArrayView
@m_since{2019,10}

Convenience alternative to @ref StridedArrayView::StridedArrayView(ArrayView<U>).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-ArrayView
*/
template<class T> constexpr StridedArrayView1D<T> stridedArrayView(ArrayView<T> view) {
    return view;
}

/** @relatesalso StridedArrayView
@brief Make a strided view on @ref StaticArrayView
@m_since{2019,10}

Convenience alternative to @ref StridedArrayView::StridedArrayView(StaticArrayView<size, U>).
The following two lines are equivalent:

@snippet Containers.cpp stridedArrayView-StaticArrayView
*/
template<std::size_t size, class T> constexpr StridedArrayView1D<T> stridedArrayView(StaticArrayView<size, T> view) {
    return ArrayView<T>{view};
}

/** @relatesalso StridedArrayView
@brief Make a view on a view
@m_since{2019,10}

Equivalent to the implicit @ref StridedArrayView copy constructor --- it
shouldn't be an error to call @ref stridedArrayView() on itself.
*/
template<unsigned dimensions, class T> constexpr StridedArrayView<dimensions, T> stridedArrayView(StridedArrayView<dimensions, T> view) {
    return view;
}

/** @relatesalso StridedArrayView
@brief Make a strided view on an external type / from an external representation
@m_since{2019,10}

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
extra careful in that case. Example usage:

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

/** @relatesalso StridedArrayView
@brief Reinterpret-cast a void strided array view
@m_since{2020,06}

Size of the new array is the same as original. Expects that the target type is
[standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and @cpp sizeof(U) @ce is not larger than any @ref StridedArrayView::stride() "stride()"
of the original array. Works with negative and zero strides as well, however
note that no type compatibility checks can be done for zero strides, so be
extra careful in that case.
*/
template<class U, unsigned dimensions> StridedArrayView<dimensions, U> arrayCast(const StridedArrayView<dimensions, const void>& view) {
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    #ifndef CORRADE_NO_DEBUG
    for(unsigned i = 0; i != dimensions; ++i) {
        CORRADE_ASSERT(!view._stride._data[i] || sizeof(U) <= std::size_t(view._stride._data[i] < 0 ? -view._stride._data[i] : view._stride._data[i]),
            "Containers::arrayCast(): can't fit a" << sizeof(U) << Utility::Debug::nospace << "-byte type into a stride of" << view._stride._data[i], {});
    }
    #endif
    return StridedArrayView<dimensions, U>{view._size, view._stride, view._data};
}

/** @relatesalso StridedArrayView
@overload
@m_since{2020,06}
*/
template<class U, unsigned dimensions> StridedArrayView<dimensions, U> arrayCast(const StridedArrayView<dimensions, void>& view) {
    auto out = arrayCast<const U, dimensions>(StridedArrayView<dimensions, const void>{view});
    return StridedArrayView<dimensions, U>{out._size, out._stride, const_cast<void*>(out._data)};
}

namespace Implementation {

/* Even though this *could* be done directly as SFINAEd overloads of
   arrayCast() (using a default non-type template parameter or a default
   function parameter), it's done like this to avoid default parameters in the
   arrayCast() so it can be forward-declared to break up header dependencies,
   and also to make the friend declaration inside StridedArrayView simpler */
template<int dimensions> struct ArrayCastFlattenOrInflate {
    static_assert(dimensions == 0, "can only inflate into one more dimension or flatten into the same / one less dimension");
};
template<> struct ArrayCastFlattenOrInflate<-1> {
    template<class U, unsigned dimensions, class T> static StridedArrayView<dimensions - 1, U> cast(const StridedArrayView<dimensions, T>& view) {
        #ifndef CORRADE_NO_DEBUG
        /* The last dimension is flattened, so not testing its stride */
        for(unsigned i = 0; i != dimensions - 1; ++i) {
            CORRADE_ASSERT(!view._stride._data[i] || sizeof(U) <= std::size_t(view._stride._data[i] < 0 ? -view._stride._data[i] : view._stride._data[i]),
                "Containers::arrayCast(): can't fit a" << sizeof(U) << Utility::Debug::nospace << "-byte type into a stride of" << view._stride._data[i], {});
        }
        #endif
        CORRADE_ASSERT(sizeof(T) == std::size_t(view._stride[dimensions - 1]),
            "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride" << sizeof(T) << "but got" << view.stride()[dimensions - 1], {});
        CORRADE_ASSERT(sizeof(T)*view._size._data[dimensions - 1] == sizeof(U),
            "Containers::arrayCast(): last dimension needs to have byte size equal to new type size in order to be flattened, expected" << sizeof(U) << "but got" << sizeof(T)*view._size._data[dimensions - 1], {});
        return StridedArrayView<dimensions - 1, U>{
            StaticArrayView<dimensions, const std::size_t>(view._size).template prefix<dimensions - 1>(),
            StaticArrayView<dimensions, const std::ptrdiff_t>(view._stride).template prefix<dimensions - 1>(),
            view._data};
    }
};
template<> struct ArrayCastFlattenOrInflate<0> {
    template<class U, unsigned dimensions, class T> static StridedArrayView<dimensions, U> cast(const StridedArrayView<dimensions, T>& view) {
        #ifndef CORRADE_NO_DEBUG
        /* The last dimension is flattened, so not testing its stride */
        for(unsigned i = 0; i != dimensions - 1; ++i) {
            CORRADE_ASSERT(!view._stride._data[i] || sizeof(U) <= std::size_t(view._stride._data[i] < 0 ? -view._stride._data[i] : view._stride._data[i]),
                "Containers::arrayCast(): can't fit a" << sizeof(U) << Utility::Debug::nospace << "-byte type into a stride of" << view._stride._data[i], {});
        }
        #endif
        CORRADE_ASSERT(sizeof(T) == std::size_t(view._stride[dimensions - 1]),
            "Containers::arrayCast(): last dimension needs to be contiguous in order to be flattened, expected stride" << sizeof(T) << "but got" << view.stride()[dimensions - 1], {});
        CORRADE_ASSERT(sizeof(T)*view._size._data[dimensions - 1] % sizeof(U) == 0,
            "Containers::arrayCast(): last dimension needs to have byte size divisible by new type size in order to be flattened, but for a" << sizeof(U) << Utility::Debug::nospace << "-byte type got" << sizeof(T)*view._size._data[dimensions - 1], {});

        StridedDimensions<dimensions, std::size_t> size;
        StridedDimensions<dimensions, std::ptrdiff_t> stride;
        size._data[dimensions - 1] = sizeof(T)*view._size._data[dimensions - 1]/sizeof(U);
        stride._data[dimensions - 1] = sizeof(U);
        for(std::size_t i = 0; i != dimensions - 1; ++i) {
            size._data[i] = view._size._data[i];
            stride._data[i] = view._stride._data[i];
        }
        return StridedArrayView<dimensions, U>{size, stride, view._data};
    }
};
template<> struct ArrayCastFlattenOrInflate<+1> {
    template<class U, unsigned dimensions, class T> static StridedArrayView<dimensions + 1, U> cast(const StridedArrayView<dimensions, T>& view) {
        constexpr std::size_t lastDimensionSize = sizeof(T)/sizeof(U);
        static_assert(sizeof(T) % lastDimensionSize == 0, "original type not a multiply of inflated type");
        /* No need to have a runtime check for the "stride fitting" like above
           as it is already checked with the above static_assert() at compile
           time -- assuming no stride was smaller than sizeof(T) before, it
           won't be smaller than sizeof(U) either since sizeof(T) > sizeof(U) */
        StridedDimensions<dimensions + 1, std::size_t> size;
        StridedDimensions<dimensions + 1, std::ptrdiff_t> stride;
        size._data[dimensions] = lastDimensionSize;
        stride._data[dimensions] = sizeof(U);
        for(std::size_t i = 0; i != dimensions; ++i) {
            size._data[i] = view._size._data[i];
            stride._data[i] = view._stride._data[i];
        }
        return StridedArrayView<dimensions + 1, U>{size, stride, view._data};
    }
};

}

/** @relatesalso StridedArrayView
@brief Reinterpret-cast and inflate or flatten a strided array view
@m_since{2019,10}

If @cpp newDimensions > dimensions @ce, inflates the last dimension into the
new type @p U, its element count being ratio of @p T and @p U sizes. The
@p newDimensions template parameter is expected to always be one more than
@p dimensions. This operation can be used for example to peek into individual
channels pixel data:

@snippet Containers.cpp arrayCast-StridedArrayView-inflate

If @cpp newDimensions < dimensions @ce, flattens the last dimension into a
contiguous new type @p U, expecting the last dimension to be contiguous and its
stride equal to size of @p U. The @p newDimensions template parameter is
expected to always be one less than @p dimensions.

Lastly, if @cpp newDimensions == dimensions @ce, the last dimension is
reinterpreted as @p U, expecting it to be contiguous and its total byte size
being divisible by the size of @p U. The resulting last dimension has a size
that's a ratio of @p T and @p U sizes and stride equivalent to @p U, being
again contiguous.

Expects that both types are [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType) and @cpp sizeof(U) @ce is not larger than any
@ref StridedArrayView::stride() "stride()" of the original array. Works with
negative and zero strides as well, however note that no type compatibility
checks can be done for zero strides, so be extra careful in that case.
@see @ref StridedArrayView::isContiguous()
*/
template<unsigned newDimensions, class U, unsigned dimensions, class T> StridedArrayView<newDimensions, U> arrayCast(const StridedArrayView<dimensions, T>& view) {
    static_assert(std::is_standard_layout<T>::value, "the source type is not standard layout");
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    return Implementation::ArrayCastFlattenOrInflate<int(newDimensions) - int(dimensions)>::template cast<U>(view);
}

/**
@brief Reinterpret-cast and inflate an array view
@m_since{2020,06}

Converts @p view to a @ref StridedArrayView1D and calls the above function.
*/
template<unsigned newDimensions, class U, class T> inline StridedArrayView<newDimensions, U> arrayCast(const ArrayView<T>& view) {
    return arrayCast<newDimensions, U, 1, T>(view);
}

/** @relatesalso StridedArrayView
@brief Reinterpret-cast and inflate a void strided array view
@m_since{2020,06}

Inflates the last dimension into the new type @p U, its element count being
@p lastDimensionSize. The @p newDimensions template parameter is expected to
always be one more than @p dimensions. For flattening the view (inverse of this
operation) you need to cast to a concrete type first.

Expects that the target type is [standard layout](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
and @cpp sizeof(U) @ce is not larger than any
@ref StridedArrayView::stride() "stride()" of the original array. Works with
negative and zero strides as well, however note that no type compatibility
checks can be done for zero strides, so be extra careful in that case.
*/
template<unsigned newDimensions, class U, unsigned dimensions> StridedArrayView<newDimensions, U> arrayCast(const StridedArrayView<dimensions, const void>& view, std::size_t lastDimensionSize) {
    static_assert(std::is_standard_layout<U>::value, "the target type is not standard layout");
    static_assert(newDimensions == dimensions + 1, "can inflate only into one more dimension");
    #ifndef CORRADE_NO_DEBUG
    /* Not testing the last dimension, because that one has to satisfy the
       (stricter) next condition as well */
    for(unsigned i = 0; i != dimensions - 1; ++i) {
        CORRADE_ASSERT(!view._stride._data[i] || sizeof(U) <= std::size_t(view._stride._data[i] < 0 ? -view._stride._data[i] : view._stride._data[i]),
            "Containers::arrayCast(): can't fit a" << sizeof(U) << Utility::Debug::nospace << "-byte type into a stride of" << view._stride._data[i], {});
    }
    #endif
    CORRADE_ASSERT(!view._stride._data[dimensions - 1] || lastDimensionSize*sizeof(U) <= std::size_t(view._stride._data[dimensions - 1] < 0 ? -view._stride._data[dimensions - 1] : view._stride._data[dimensions - 1]),
        "Containers::arrayCast(): can't fit" << lastDimensionSize << sizeof(U) << Utility::Debug::nospace << "-byte items into a stride of" << view._stride._data[dimensions - 1], {});
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

/** @relatesalso StridedArrayView
@overload
@m_since{2020,06}
*/
template<unsigned newDimensions, class U, unsigned dimensions> StridedArrayView<newDimensions, U> arrayCast(const StridedArrayView<dimensions, void>& view, std::size_t lastDimensionSize) {
    auto out = arrayCast<newDimensions, const U, dimensions>(StridedArrayView<dimensions, const void>{view}, lastDimensionSize);
    return StridedArrayView<newDimensions, U>{out._size, out._stride, const_cast<void*>(out._data)};
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

template<unsigned dimensions, class T> template<unsigned dimension> bool StridedArrayView<dimensions, T>::isContiguous() const {
    static_assert(dimension < dimensions, "dimension out of bounds");
    std::size_t nextDimensionSize = sizeof(T);
    for(std::size_t i = dimensions; i != dimension; --i) {
        if(std::size_t(_stride[i - 1]) != nextDimensionSize) return false;
        nextDimensionSize *= _size[i - 1];
    }

    return true;
}

template<unsigned dimensions, class T> ArrayView<T> StridedArrayView<dimensions, T>::asContiguous() const {
    CORRADE_ASSERT(isContiguous(), "Containers::StridedArrayView::asContiguous(): the view is not contiguous", {});
    std::size_t size = _size[0];
    for(std::size_t i = 1; i != dimensions; ++i) size *= _size[i];
    return {static_cast<T*>(_data), size};
}

namespace Implementation {
    template<unsigned dimensions, class T> struct StridedElement {
        static StridedArrayView<dimensions - 1, T> get(typename std::conditional<std::is_const<T>::value, const void, void>::type* data, const StridedDimensions<dimensions, std::size_t>& size, const StridedDimensions<dimensions, std::ptrdiff_t>& stride, std::size_t i) {
            return StridedArrayView<dimensions - 1, T>{
                StridedDimensions<dimensions - 1, std::size_t>(size._data + 1, typename Implementation::GenerateSequence<dimensions - 1>::Type{}),
                StridedDimensions<dimensions - 1, std::ptrdiff_t>(stride._data + 1, typename Implementation::GenerateSequence<dimensions - 1>::Type{}),
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
