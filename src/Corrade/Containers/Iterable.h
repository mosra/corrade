#ifndef Corrade_Containers_Iterable_h
#define Corrade_Containers_Iterable_h
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
 * @brief Class @ref Corrade::Containers::Iterable, @ref Corrade::Containers::IterableIterator
 * @m_since_latest
 */

#include <initializer_list>

/* While not strictly needed, without AnyReference included passing {a, b, c}
   to the constructor would result in pages of crazy errors, none of them
   mentioning that one needs to include AnyReference. Ah C++ :/ */
#include "Corrade/Containers/AnyReference.h"

#include "Corrade/Containers/iterableHelpers.h"
#include "Corrade/Utility/DebugAssert.h"
#include "Corrade/Utility/Move.h"

namespace Corrade { namespace Containers {

/**
@brief Wrapper for any sequential container of values or references
@m_since_latest

Useful in scenarios where, given a heavy or a move-only @cpp T @ce, it's
desirable to have an API accept @ref ArrayView "ArrayView<const Reference<T>>"
to account for cases where instances are scattered around and can't be put into
a linear container, but also accept just plain @ref ArrayView "ArrayView<T>"
and other variants for convenience.

This class adds extra indirection to allow iterating over various input
containers with a single code path. Assuming the API itself isn't bottlenecked
on iteration performance, it should be an acceptable tradeoff compared to
having to implement multiple code paths or have extra overloads that unify the
process by copying the data to a temporary container first. In contrast, if the
type doesn't need to be taken via a reference, it's preferable to accept just
@ref ArrayView "ArrayView<T>" or @ref StridedArrayView "StridedArrayView*D<T>"
instead of using this class.

@section Containers-Iterable-usage Usage

An @ref Iterable "Iterable<T>" can be implicitly created from an
@ref ArrayView "ArrayView<T>", @ref StridedArrayView "StridedArrayView1D<T>";
(strided) array view of (@cpp const @ce) @ref Reference "Reference<T>",
@ref MoveReference "MoveReference<T>" or @ref AnyReference "AnyReference<T>";
@ref std::initializer_list; and any other type convertible to these. Such as
plain C arrays, @ref Array or
@ref Containers-ArrayView-stl "STL types convertible to an ArrayView".

Example usage --- passing a list of non-copyable @ref Utility::FileWatcher
instances to an API:

@snippet Containers-stl.cpp Iterable-usage

<b></b>

@m_class{m-block m-warning}

@par Dangling references
    Because the type is, like an @ref ArrayView, just a non-owning view on the
    input data, to avoid dangling references it's recommended to *never*
    explicitly instantiate the type. Pass the input data directly instead. The
    snippet below would compile, however on the last line the API will be
    accessing an already-destroyed @link std::initializer_list @endlink:
@par
    @snippet Containers-stl.cpp Iterable-usage-boom

<b></b>

On the API implementation side, the usual container interface is exposed --- in
particular @ref isEmpty(), @ref size(), @ref operator[](), @ref front(),
@ref back() as well as range-for access:

@snippet Containers.cpp Iterable-usage-implementation

@see @ref StringIterable
*/
template<class T> class Iterable {
    /** @todo Iterable<T&> and <T&&> specializations taking only Reference /
        MoveReference, once actually needed */
    static_assert(!std::is_reference<T>::value, "iterables of references are not supported at the moment");

    public:
        typedef T Type;     /**< @brief Element type */

        /**
         * @brief Default constructor
         *
         * Creates an instance with @cpp nullptr @ce data and size and stride
         * set to @cpp 0 @ce.
         */
        constexpr /*implicit*/ Iterable(std::nullptr_t = nullptr) noexcept: _data{}, _size{}, _stride{}, _accessor{} {}

        /**
         * @brief Construct from any sequential iterable container
         *
         * @p U can be an @ref ArrayView "ArrayView<T>",
         * @ref StridedArrayView "StridedArrayView1D<T>"; (strided) array view
         * of (@cpp const @ce) @ref Reference "Reference<T>",
         * @ref MoveReference "MoveReference<T>" or
         * @ref AnyReference "AnyReference<T>"; and any other type convertible
         * to these --- see @ref ArrayView and @ref StridedArrayView docs for
         * more information.
         */
        /* This has to accept any type and then delegate to private
           constructors instead of directly taking ArrayView<T> etc., due to
           how overload resolution works in copy initialization as opposed to
           a direct constructor/function call. If it would take ArrayView<T>
           directly, the following scenario wouldn't work because
           `T[] -> ArrayView<T> -> Iterable<T>` is one custom conversion
           sequence more than allowed in a copy initialization, and to make
           that work, this class would have to replicate all [Strided]ArrayView
           constructors, which isn't feasible.

            void foo(const Iterable<int>&);
            int data[]{ 5, 17, 13 };
            foo(data); // won't match the Iterable(ArrayView<T>) constructor
            Iterable<int> i = data; // won't work either
            Iterable<int> i{data};  // would work

           By accepting any type, it's just one custom conversion
           (`T[] -> Iterable<T>`), so `foo(data)` works, and then the
           constructor delegation will work because there any number of custom
           conversions is allowed. Spec for reference:
            https://en.cppreference.com/w/cpp/language/copy_initialization
        */
        template<class U, class = decltype(Iterable{std::declval<U&&>(), Implementation::IterableOverloadPriority<1>{}})> /*implicit*/ Iterable(U&& data): Iterable{Utility::forward<U>(data), Implementation::IterableOverloadPriority<1>{}} {}

        /**
         * @brief Construct from an initializer list
         *
         * In order to be able to accept also lists of non-copyable types, the
         * constructor takes a reference type. Then, to accept both
         * @cpp {a, b, c} @ce and @cpp {T{…}, T{…}, T{…}} @ce, it's an
         * @ref AnyReference instead of a @ref Reference.
         */
        /*implicit*/ Iterable(std::initializer_list<AnyReference<T>> view) noexcept;

        /** @overload */
        /* Variant of the above that accepts "AnyReference<mutable T>" and
           passes through to AnyReference<T> */
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> /*implicit*/ Iterable(std::initializer_list<AnyReference<typename std::remove_const<T>::type>> view) noexcept: Iterable{reinterpret_cast<std::initializer_list<AnyReference<T>>&>(view)} {}

        /**
         * @brief Data pointer
         *
         * Not meant to be used directly, as the returned value may point to an
         * actual value but also to a reference to the value, with no
         * possibility to distinguish between the two. The returned pointer is
         * @cpp const @ce even when @p T is mutable as the data may point to
         * @ref Reference "const Reference<T>" and similar.
         */
        const void* data() const { return _data; }

        /**
         * @brief Number of items in the container
         *
         * @see @ref stride(), @ref isEmpty()
         */
        std::size_t size() const { return _size; }

        /**
         * @brief Stride between items in the container
         *
         * For a contiguous array of @p T it's equal to @cpp sizeof(T) @ce, for
         * references it's @cpp sizeof(void*) @ce, for a strided view it's
         * equivalent to @ref StridedArrayView::stride().
         * @see @ref size()
         */
        std::ptrdiff_t stride() const { return _stride; }

        /**
         * @brief Whether the container is empty
         *
         * @see @ref size()
         */
        bool isEmpty() const { return !_size; }

        /**
         * @brief Element access
         *
         * Expects that @p i is less than @ref size().
         */
        T& operator[](std::size_t i) const;

        /**
         * @brief Iterator to first element
         *
         * @see @ref front()
         */
        IterableIterator<T> begin() const {
            return IterableIterator<T>{_data, _stride, _accessor, 0};
        }
        /** @overload */
        IterableIterator<T> cbegin() const {
            return IterableIterator<T>{_data, _stride, _accessor, 0};
        }

        /**
         * @brief Iterator to (one item after) last element
         *
         * @see @ref back()
         */
        IterableIterator<T> end() const {
            return IterableIterator<T>{_data, _stride, _accessor, _size};
        }
        /** @overload */
        IterableIterator<T> cend() const {
            return IterableIterator<T>{_data, _stride, _accessor, _size};
        }

        /**
         * @brief First element
         *
         * Expects there is at least one element.
         * @see @ref begin()
         */
        T& front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element.
         * @see @ref end()
         */
        T& back() const;

    private:
        /* The ArrayView variants take IterableOverloadPriority<1>, while
           the StridedArrayView variants take IterableOverloadPriority<0>; the
           public constructor then passes IterableOverloadPriority<1>{}. Which
           has the effect that if a type is convertible to both an ArrayView
           and a StridedArrayView, ArrayView gets a priority (because
           IterableOverloadPriority<0> is a base of IterableOverloadPriority<1>
           thus considered as a more complex conversion).

           Originally this used std::is_convertible / std::is_constructible,
           but both of them refuse to work on some compilers when a
           forward-declared type is involved. See the
           IterableTest::overloadsWithForwardDeclaredType() test for a
           repro case. */

        explicit Iterable(ArrayView<T> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit Iterable(ArrayView<const Reference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit Iterable(ArrayView<const MoveReference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit Iterable(ArrayView<const AnyReference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept;

        /* Variants of the above that accept "WhateverReference<mutable T>"
           and pass through to WhateverReference<T> */
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(ArrayView<const Reference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<1>) noexcept: Iterable{reinterpret_cast<const ArrayView<const Reference<T>>&>(view), Implementation::IterableOverloadPriority<1>{}} {}
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(ArrayView<const MoveReference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<1>) noexcept: Iterable{reinterpret_cast<const ArrayView<const MoveReference<T>>&>(view), Implementation::IterableOverloadPriority<1>{}} {}
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(ArrayView<const AnyReference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<1>) noexcept: Iterable{reinterpret_cast<const ArrayView<const AnyReference<T>>&>(view), Implementation::IterableOverloadPriority<1>{}} {}

        explicit Iterable(StridedArrayView1D<T> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit Iterable(StridedArrayView1D<const Reference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit Iterable(StridedArrayView1D<const MoveReference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit Iterable(StridedArrayView1D<const AnyReference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept;

        /* Variants of the above that accept "WhateverReference<mutable T>"
           and pass through to WhateverReference<T> */
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(StridedArrayView1D<const Reference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<0>) noexcept: Iterable{reinterpret_cast<const StridedArrayView1D<const Reference<T>>&>(view), Implementation::IterableOverloadPriority<0>{}} {}
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(StridedArrayView1D<const MoveReference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<0>) noexcept: Iterable{reinterpret_cast<const StridedArrayView1D<const MoveReference<T>>&>(view), Implementation::IterableOverloadPriority<0>{}} {}
        template<class U = typename std::remove_const<T>::type, class = typename std::enable_if<!std::is_same<T, U>::value>::type> explicit Iterable(StridedArrayView1D<const AnyReference<typename std::remove_const<T>::type>> view, Implementation::IterableOverloadPriority<0>) noexcept: Iterable{reinterpret_cast<const StridedArrayView1D<const AnyReference<T>>&>(view), Implementation::IterableOverloadPriority<0>{}} {}

        const void* _data;
        std::size_t _size;
        std::ptrdiff_t _stride;
        T&(*_accessor)(const void*);
};

/**
@brief Iterable iterator
@m_since_latest

Used by @ref Iterable to provide iterator access to its items.
*/
template<class T> class IterableIterator {
    public:
        typedef T Type;     /**< @brief Element type */

        /** @brief Equality comparison */
        bool operator==(const IterableIterator<T>& other) const {
            return _data == other._data && _stride == other._stride && _i == other._i;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const IterableIterator<T>& other) const {
            return _data != other._data || _stride != other._stride || _i != other._i;
        }

        /** @brief Less than comparison */
        bool operator<(const IterableIterator<T>& other) const {
            return _data == other._data && _stride == other._stride && _i < other._i;
        }

        /** @brief Less than or equal comparison */
        bool operator<=(const IterableIterator<T>& other) const {
            return _data == other._data && _stride == other._stride && _i <= other._i;
        }

        /** @brief Greater than comparison */
        bool operator>(const IterableIterator<T>& other) const {
            return _data == other._data && _stride == other._stride && _i > other._i;
        }

        /** @brief Greater than or equal comparison */
        bool operator>=(const IterableIterator<T>& other) const {
            return _data == other._data && _stride == other._stride && _i >= other._i;
        }

        /** @brief Add an offset */
        IterableIterator<T> operator+(std::ptrdiff_t i) const {
            return IterableIterator<T>{_data, _stride, _accessor, _i + i};
        }

        /** @brief Add an offset and assign */
        IterableIterator<T>& operator+=(std::ptrdiff_t i) {
            _i += i;
            return *this;
        }

        /** @brief Subtract an offset */
        IterableIterator<T> operator-(std::ptrdiff_t i) const {
            return IterableIterator<T>{_data, _stride, _accessor, _i - i};
        }

        /** @brief Subtract an offset and assign */
        IterableIterator<T>& operator-=(std::ptrdiff_t i) {
            _i -= i;
            return *this;
        }

        /** @brief Iterator difference */
        std::ptrdiff_t operator-(const IterableIterator<T>& it) const {
            return _i - it._i;
        }

        /** @brief Go back to previous position */
        IterableIterator<T>& operator--() {
            --_i;
            return *this;
        }

        /** @brief Advance to next position */
        IterableIterator<T>& operator++() {
            ++_i;
            return *this;
        }

        /** @brief Dereference */
        T& operator*() const {
            return _accessor(_data + _i*_stride);
        }

    private:
        friend Iterable<T>;

        explicit IterableIterator(const void* data, std::ptrdiff_t stride, T&(*accessor)(const void*), std::size_t i) noexcept:
            /* _data{} will cause GCC 4.8 to warn that "parameter 'data' set
                but not used" */
            _data(static_cast<const char*>(data)), _stride{stride}, _accessor{accessor}, _i{i} {}

        const char* _data;
        std::ptrdiff_t _stride;
        T&(*_accessor)(const void*);
        std::size_t _i;
};

/** @relates IterableIterator
@brief Add strided iterator to an offset
*/
template<class T> inline IterableIterator<T> operator+(std::ptrdiff_t i, const IterableIterator<T>& it) {
    return it + i;
}

template<class T> inline Iterable<T>::Iterable(const ArrayView<T> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(T)}, _accessor{[](const void* data) -> T& {
    return *const_cast<T*>(static_cast<const T*>(data));
}} {}

template<class T> inline Iterable<T>::Iterable(const ArrayView<const Reference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(Reference<T>)}, _accessor{[](const void* data) -> T& {
    return **static_cast<const Reference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const ArrayView<const MoveReference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(MoveReference<T>)}, _accessor{[](const void* data) -> T& {
    return **static_cast<const MoveReference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const ArrayView<const AnyReference<T>> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _size{view.size()}, _stride{sizeof(AnyReference<T>)}, _accessor{[](const void* data) -> T& {
    return **static_cast<const AnyReference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const StridedArrayView1D<T> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data) -> T& {
    return *const_cast<T*>(static_cast<const T*>(data));
}} {}

template<class T> inline Iterable<T>::Iterable(const StridedArrayView1D<const Reference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data) -> T& {
    return **static_cast<const Reference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const StridedArrayView1D<const MoveReference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data) -> T& {
    return **static_cast<const MoveReference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const StridedArrayView1D<const AnyReference<T>> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data) -> T& {
    return **static_cast<const AnyReference<T>*>(data);
}} {}

template<class T> inline Iterable<T>::Iterable(const std::initializer_list<AnyReference<T>> view) noexcept: _data{view.begin()}, _size{view.size()}, _stride{sizeof(AnyReference<T>)}, _accessor{[](const void* data) -> T& {
    return **static_cast<const AnyReference<T>*>(data);
}} {}

template<class T> T& Iterable<T>::operator[](const std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < _size, "Containers::Iterable::operator[](): index" << i << "out of range for" << _size << "elements", _accessor(_data));

    return _accessor(static_cast<const char*>(_data) + i*_stride);
}

template<class T> T& Iterable<T>::front() const {
    CORRADE_DEBUG_ASSERT(_size, "Containers::Iterable::front(): view is empty", _accessor(_data));

    return _accessor(_data);
}

template<class T> T& Iterable<T>::back() const {
    CORRADE_DEBUG_ASSERT(_size, "Containers::Iterable::back(): view is empty", _accessor(_data));

    return _accessor(static_cast<const char*>(_data) + (_size - 1)*_stride);
}

}}

#endif
