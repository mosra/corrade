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

#include "BitArray.h"

#include <cstring>

#include "Corrade/Containers/BitArrayView.h"
#include "Corrade/Utility/Assert.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Containers {

BitArray::BitArray(Corrade::ValueInitT, std::size_t size):
    #ifdef CORRADE_GRACEFUL_ASSERT
    _data{}, /* If the assert fails, don't try to delete a random pointer */
    #endif
    _sizeOffset{size << 3}, _deleter{}
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 3),
        "Containers::BitArray: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size, );
    _data = size ? new char[(size + 7) >> 3]{} : nullptr;
}

BitArray::BitArray(Corrade::NoInitT, std::size_t size):
    #ifdef CORRADE_GRACEFUL_ASSERT
    _data{}, /* If the assert fails, don't try to delete a random pointer */
    #endif
    _sizeOffset{size << 3}, _deleter{}
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 3),
        "Containers::BitArray: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size, );
    _data = size ? new char[(size + 7) >> 3] : nullptr;
}

BitArray::BitArray(Corrade::DirectInitT, const std::size_t size, const bool value):
    #ifdef CORRADE_GRACEFUL_ASSERT
    _data{}, /* If the assert fails, don't try to delete a random pointer */
    #endif
    _sizeOffset{size << 3}, _deleter{}
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 3),
        "Containers::BitArray: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size, );
    if(!size) _data = nullptr;
    else if(!value) _data = new char[(size + 7) >> 3]{};
    else {
        _data = new char[(size + 7) >> 3];
        std::memset(_data, 0xff, (size + 7) >> 3);
    }
}

BitArray::BitArray(void* data, const std::size_t offset, const std::size_t size, const Deleter deleter) noexcept:
    /* Interestingly enough, on GCC 4.8, using _value{} will spam with
        warning: parameter 'data' set but not used [-Wunused-but-set-parameter]
        even though everything works as intended. Using () instead. */
    _data(static_cast<char*>(data)),
    _sizeOffset{size << 3 | offset}, _deleter{deleter}
{
    CORRADE_ASSERT(offset < 8,
        "Containers::BitArray: offset expected to be smaller than 8 bits, got" << offset, );
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 3),
        "Containers::BitArray: size expected to be smaller than 2^" << Utility::Debug::nospace << (sizeof(std::size_t)*8 - 3) << "bits, got" << size, );
}

BitArray::~BitArray() {
    if(_deleter) {
        /* The deleter gets amount of bits spanned by all bits including the
           initial offset -- if offset is 0 and size is 0, it gets 0, but if
           offset is 7 and size 0, then it gets 1. */
        _deleter(_data, ((_sizeOffset >> 3) + (_sizeOffset & 0x07) + 7) >> 3);
    }
    else delete[] _data;
}

BitArray::operator MutableBitArrayView() {
    return MutableBitArrayView{_data, _sizeOffset};
}

BitArray::operator BitArrayView() const {
    return BitArrayView{_data, _sizeOffset};
}

MutableBitArrayView BitArray::slice(const std::size_t begin, const std::size_t end) {
    return MutableBitArrayView{*this}.slice(begin, end);
}
BitArrayView BitArray::slice(const std::size_t begin, const std::size_t end) const {
    return BitArrayView{*this}.slice(begin, end);
}

MutableBitArrayView BitArray::sliceSize(const std::size_t begin, const std::size_t size) {
    return MutableBitArrayView{*this}.sliceSize(begin, size);
}
BitArrayView BitArray::sliceSize(const std::size_t begin, const std::size_t size) const {
    return BitArrayView{*this}.sliceSize(begin, size);
}

MutableBitArrayView BitArray::prefix(const std::size_t size) {
    return MutableBitArrayView{*this}.prefix(size);
}
BitArrayView BitArray::prefix(const std::size_t size) const {
    return BitArrayView{*this}.prefix(size);
}

MutableBitArrayView BitArray::suffix(const std::size_t size) {
    return MutableBitArrayView{*this}.suffix(size);
}
BitArrayView BitArray::suffix(const std::size_t size) const {
    return BitArrayView{*this}.suffix(size);
}

MutableBitArrayView BitArray::exceptPrefix(const std::size_t size) {
    return MutableBitArrayView{*this}.exceptPrefix(size);
}
BitArrayView BitArray::exceptPrefix(const std::size_t size) const {
    return BitArrayView{*this}.exceptPrefix(size);
}

MutableBitArrayView BitArray::exceptSuffix(const std::size_t size) {
    return MutableBitArrayView{*this}.exceptSuffix(size);
}
BitArrayView BitArray::exceptSuffix(const std::size_t size) const {
    return BitArrayView{*this}.exceptSuffix(size);
}

Utility::Debug& operator<<(Utility::Debug& debug, const BitArray& value) {
    return operator<<(debug, BitArrayView{value});
}

Utility::Debug& operator<<(Utility::Debug& debug, BitArrayView value) {
    debug << "{" << Utility::Debug::nospace;

    const auto* data = reinterpret_cast<const unsigned char*>(value.data());
    unsigned char mask = 1 << value.offset();
    for(std::size_t i = 0, iMax = value.size(); i != iMax; ++i) {
        if(!mask) {
            ++data;
            mask = 1;
        }

        if(i && i % 8 == 0) debug << ",";

        debug << (*data & mask ? "1" : "0") << Utility::Debug::nospace;

        mask <<= 1;
    }

    return debug << "}";
}

}}
