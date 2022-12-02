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

#include "StringIterable.h"

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"

namespace Corrade { namespace Containers {

StringIterable::StringIterable(const ArrayView<const StringView> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{sizeof(StringView)}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const StringView*>(data);
}} {}

StringIterable::StringIterable(const ArrayView<const MutableStringView> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{sizeof(StringView)}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const MutableStringView*>(data);
}} {}

StringIterable::StringIterable(const ArrayView<const String> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{sizeof(String)}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const String*>(data);
}} {}

StringIterable::StringIterable(const ArrayView<const char* const> view, Implementation::IterableOverloadPriority<1>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{sizeof(const char*)}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const char* const*>(data);
}} {}

StringIterable::StringIterable(const StridedArrayView1D<const StringView> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const StringView*>(data);
}} {}

StringIterable::StringIterable(const StridedArrayView1D<const MutableStringView> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const MutableStringView*>(data);
}} {}

StringIterable::StringIterable(const StridedArrayView1D<const String> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const String*>(data);
}} {}

StringIterable::StringIterable(const StridedArrayView1D<const char* const> view, Implementation::IterableOverloadPriority<0>) noexcept: _data{view.data()}, _context{}, _size{view.size()}, _stride{view.stride()}, _accessor{[](const void* data, const void*, std::ptrdiff_t, std::size_t) -> StringView {
    return *static_cast<const char* const*>(data);
}} {}

StringIterable::StringIterable(const std::initializer_list<StringView> view) noexcept: StringIterable{Containers::arrayView(view)} {}

StringView StringIterable::operator[](const std::size_t i) const {
    CORRADE_DEBUG_ASSERT(i < _size, "Containers::StringIterable::operator[](): index" << i << "out of range for" << _size << "elements", _accessor(_data, _context, _stride, i));

    return _accessor(static_cast<const char*>(_data) + i*_stride, _context, _stride, i);
}

StringView StringIterable::front() const {
    CORRADE_DEBUG_ASSERT(_size, "Containers::StringIterable::front(): view is empty", _accessor(_data, _context, _stride, 0));

    return _accessor(_data, _context, _stride, 0);
}

StringView StringIterable::back() const {
    CORRADE_DEBUG_ASSERT(_size, "Containers::StringIterable::back(): view is empty", _accessor(_data, _context, _stride, 0));

    return _accessor(static_cast<const char*>(_data) + (_size - 1)*_stride, _context, _stride, _size - 1);
}

StringView StringIterableIterator::operator*() const {
    return _accessor(_data + _i*_stride, _context, _stride, _i);
}

}}
