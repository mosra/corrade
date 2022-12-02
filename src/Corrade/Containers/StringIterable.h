#ifndef Corrade_Containers_StringIterable_h
#define Corrade_Containers_StringIterable_h
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
 * @brief Class @ref Corrade::Containers::StringIterable, @ref Corrade::Containers::StringIterableIterator
 * @m_since_latest
 */

#include <initializer_list>

#include "Corrade/Containers/Containers.h"
#include "Corrade/Containers/iterableHelpers.h"
#include "Corrade/Utility/Move.h"
#include "Corrade/Utility/visibility.h"

namespace Corrade { namespace Containers {

/**
@brief Wrapper for any sequential container of strings or string views
@m_since_latest

Useful where it's desirable to have a single interface accepting an
@ref ArrayView of @ref String, @ref StringView or @cpp const char* @ce and
anything convertible to these.

This class adds extra indirection to allow iterating over various input
containers with a single code path. Assuming the API itself isn't bottlenecked
on iteration performance, it should be an acceptable tradeoff compared to
having to implement multiple code paths or have extra overloads that unify the
process by copying the data to a temporary container first.

@section Containers-StringIterable-usage Usage

A @ref StringIterable can be implicitly created from an
@ref ArrayView "ArrayView<StringView>" or
@ref StridedArrayView "StridedArrayView1D<StringView>",
(strided) array view of (@cpp const @ce) @ref String, @cpp const char* @ce;
@ref std::initializer_list; and any other type convertible to these. Such as
plain C arrays, @ref Array or
@ref Containers-ArrayView-stl "STL types convertible to an ArrayView".

Example usage --- passing @cpp main() @ce arguments to @ref StringView::join()
to print them:

@snippet Containers.cpp StringIterable-usage

<b></b>

@m_class{m-block m-warning}

@par Dangling references
    Because the type is, like an @ref ArrayView, just a non-owning view on the
    input data, to avoid dangling references it's recommended to *never*
    explicitly instantiate the type. Pass the input data directly instead. The
    snippet below would compile, however on the last line the API will be
    accessing an already-destroyed @link std::initializer_list @endlink:
@par
    @snippet Containers.cpp StringIterable-usage-boom

<b></b>

On the API implementation side, the usual container interface is exposed --- in
particular @ref isEmpty(), @ref size(), @ref operator[](), @ref front(),
@ref back() as well as range-for access:

@snippet Containers.cpp StringIterable-usage-implementation

@see @ref Iterable
*/
class CORRADE_UTILITY_EXPORT StringIterable {
    public:
        /**
         * @brief Default constructor
         *
         * Creates an instance with @cpp nullptr @ce data and size and stride
         * set to @cpp 0 @ce.
         */
        constexpr /*implicit*/ StringIterable(std::nullptr_t = nullptr) noexcept: _data{}, _context{}, _size{}, _stride{}, _accessor{} {}

        /**
         * @brief Construct from any sequential iterable container
         *
         * @p U can be an @ref ArrayView "ArrayView<T>",
         * @ref StridedArrayView "StridedArrayView1D<T>" and any type
         * convertible to these --- see @ref ArrayView and
         * @ref StridedArrayView docs for more information.
         *
         * The `T` can then be (@cpp const @ce) @ref String, @ref StringView,
         * @ref MutableStringView or @cpp const char* @ce.
         */
        /* See the corresponding comment inside the Iterable class for why
           we can'ŧ just accept ArrayView etc. directly here and why we have
           to capture an arbitrary U&& instead */
        template<class U, class = decltype(StringIterable{std::declval<U&&>(), Implementation::IterableOverloadPriority<1>{}})> /*implicit*/ StringIterable(U&& data) noexcept: StringIterable{Utility::forward<U>(data), Implementation::IterableOverloadPriority<1>{}} {}

        /** @brief Construct from an initializer list */
        /*implicit*/ StringIterable(std::initializer_list<StringView> view) noexcept;

        /**
         * @brief Construct a custom iterable
         * @param data      Container data pointer
         * @param context   Context pointer or @cpp nullptr @ce
         * @param size      Number of items in the container
         * @param stride    Stride between items in the container
         * @param accessor  Accessor function
         *
         * For item `i`, the @p accessor gets @cpp data + i*stride @ce in the
         * first argument, @p context in the second argument, @p stride in the
         * third argument and `i` in the fourth argument. The @p context is
         * useful for example in case the iterated container contains just
         * offsets to string values stored in an external location. The index
         * can be used for handling various edge cases in the accessor, the
         * stride for example if it's needed to retrieve the previous or next
         * data value as well.
         */
        explicit StringIterable(const void* data, const void* context, std::size_t size, std::ptrdiff_t stride, StringView(*accessor)(const void*, const void*, std::ptrdiff_t, std::size_t)) noexcept: _data{data}, _context{context}, _size{size}, _stride{stride}, _accessor{accessor} {}

        /**
         * @brief Container data pointer
         *
         * Not meant to be used directly, as the returned value may point to an
         * arbitrary string (view) type, with no possibility to distinguish it.
         */
        const void* data() const { return _data; }

        /**
         * @brief Context pointer
         *
         * Filled only by @ref StringIterable(const void*, const void*, std::size_t, std::ptrdiff_t, StringView(*)(const void*, const void*)),
         * @cpp nullptr @ce otherwise.
         */
        const void* context() const { return _context; }

        /**
         * @brief Number of items in the container
         *
         * @see @ref stride(), @ref isEmpty()
         */
        std::size_t size() const { return _size; }

        /**
         * @brief Stride between items in the container
         *
         * Depends on size of the actual string (view) type as well as whether
         * the original container is contiguous.
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
         * Expects that @p i is less than @ref size(). The returned view
         * has @ref StringViewFlag::Global or
         * @ref StringViewFlag::NullTerminated set depending on what the
         * original string type was --- for example, if was a @ref String or
         * a @cpp const char* @ce, all items will have
         * @ref StringViewFlag::NullTerminated set.
         */
        StringView operator[](std::size_t i) const;

        /**
         * @brief Iterator to first element
         *
         * @see @ref front()
         */
        StringIterableIterator begin() const;
        /** @overload */
        StringIterableIterator cbegin() const;

        /**
         * @brief Iterator to (one item after) last element
         *
         * @see @ref back()
         */
        StringIterableIterator end() const;
        /** @overload */
        StringIterableIterator cend() const;

        /**
         * @brief First element
         *
         * Expects there is at least one element. See @ref operator[]() for
         * information about returned @ref StringViewFlags.
         * @see @ref begin()
         */
        StringView front() const;

        /**
         * @brief Last element
         *
         * Expects there is at least one element. See @ref operator[]() for
         * information about returned @ref StringViewFlags.
         * @see @ref end()
         */
        StringView back() const;

    private:
        /* See the comment inside Iterable for an explanation of why
           IterableOverloadPriority is used */
        explicit StringIterable(ArrayView<const StringView> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit StringIterable(ArrayView<const MutableStringView> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit StringIterable(ArrayView<const String> view, Implementation::IterableOverloadPriority<1>) noexcept;
        explicit StringIterable(ArrayView<const char* const> view, Implementation::IterableOverloadPriority<1>) noexcept;

        explicit StringIterable(StridedArrayView1D<const StringView> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit StringIterable(StridedArrayView1D<const MutableStringView> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit StringIterable(StridedArrayView1D<const String> view, Implementation::IterableOverloadPriority<0>) noexcept;
        explicit StringIterable(StridedArrayView1D<const char* const> view, Implementation::IterableOverloadPriority<0>) noexcept;

        const void* _data;
        const void* _context;
        std::size_t _size;
        std::ptrdiff_t _stride;
        StringView(*_accessor)(const void*, const void*, std::ptrdiff_t, std::size_t);
};

/**
@brief String iterable iterator
@m_since_latest

Used by @ref StringIterable to provide iterator access to its items.
*/
class CORRADE_UTILITY_EXPORT StringIterableIterator {
    public:
        /** @brief Equality comparison */
        bool operator==(const StringIterableIterator& other) const {
            return _data == other._data && _context == other._context && _stride == other._stride && _i == other._i;
        }

        /** @brief Non-equality comparison */
        bool operator!=(const StringIterableIterator& other) const {
            return _data != other._data || _context != other._context || _stride != other._stride || _i != other._i;
        }

        /** @brief Less than comparison */
        bool operator<(const StringIterableIterator& other) const {
            return _data == other._data && _context == other._context && _stride == other._stride && _i < other._i;
        }

        /** @brief Less than or equal comparison */
        bool operator<=(const StringIterableIterator& other) const {
            return _data == other._data && _context == other._context && _stride == other._stride && _i <= other._i;
        }

        /** @brief Greater than comparison */
        bool operator>(const StringIterableIterator& other) const {
            return _data == other._data && _context == other._context && _stride == other._stride && _i > other._i;
        }

        /** @brief Greater than or equal comparison */
        bool operator>=(const StringIterableIterator& other) const {
            return _data == other._data && _context == other._context && _stride == other._stride && _i >= other._i;
        }

        /** @brief Add an offset */
        StringIterableIterator operator+(std::ptrdiff_t i) const {
            return StringIterableIterator{_data, _context, _stride, _accessor, _i + i};
        }

        /** @brief Add an offset and assign */
        StringIterableIterator& operator+=(std::ptrdiff_t i) {
            _i += i;
            return *this;
        }

        /** @brief Subtract an offset */
        StringIterableIterator operator-(std::ptrdiff_t i) const {
            return StringIterableIterator{_data, _context, _stride, _accessor, _i - i};
        }

        /** @brief Subtract an offset and assign */
        StringIterableIterator& operator-=(std::ptrdiff_t i) {
            _i -= i;
            return *this;
        }

        /** @brief Iterator difference */
        std::ptrdiff_t operator-(const StringIterableIterator& it) const {
            return _i - it._i;
        }

        /** @brief Go back to previous position */
        StringIterableIterator& operator--() {
            --_i;
            return *this;
        }

        /** @brief Advance to next position */
        StringIterableIterator& operator++() {
            ++_i;
            return *this;
        }

        /**
         * @brief Dereference
         *
         * See @ref StringIterable::operator[]() for information about returned
         * @ref StringViewFlags.
         */
        StringView operator*() const;

    private:
        friend StringIterable;

        explicit StringIterableIterator(const void* data, const void* context, std::ptrdiff_t stride, StringView(*accessor)(const void*, const void*, std::ptrdiff_t, std::size_t), std::size_t i) noexcept:
            /* _data{} will cause GCC 4.8 to warn that "parameter 'data' set
                but not used" */
            _data(static_cast<const char*>(data)), _context{context}, _stride{stride}, _accessor{accessor}, _i{i} {}

        const char* _data;
        const void* _context;
        std::ptrdiff_t _stride;
        StringView(*_accessor)(const void*, const void*, std::ptrdiff_t, std::size_t);
        std::size_t _i;
};

/** @relates IterableIterator
@brief Add strided iterator to an offset
*/
inline StringIterableIterator operator+(std::ptrdiff_t i, const StringIterableIterator& it) {
    return it + i;
}

inline StringIterableIterator StringIterable::begin() const {
    return StringIterableIterator{_data, _context, _stride, _accessor, 0};
}

inline StringIterableIterator StringIterable::cbegin() const {
    return StringIterableIterator{_data, _context, _stride, _accessor, 0};
}

inline StringIterableIterator StringIterable::end() const {
    return StringIterableIterator{_data, _context, _stride, _accessor, _size};
}

inline StringIterableIterator StringIterable::cend() const {
    return StringIterableIterator{_data, _context, _stride, _accessor, _size};
}

}}

#endif
