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

#include "String.h"
#include "StringStl.h"

#include <string>
#include <cstring>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers {

namespace {
    enum: std::size_t {
        SmallSize = 0x80,
        SmallSizeMask = 0xc0,
        LargeSizeMask = SmallSizeMask << (sizeof(std::size_t) - 1)*8
    };
}

static_assert(std::size_t(LargeSizeMask) == Implementation::StringViewSizeMask,
    "reserved bits should be the same in String and StringView");

String String::nullTerminatedView(StringView view) {
    if(view.flags() & StringViewFlag::NullTerminated)
        return String{view.data(), view.size(), [](char*, std::size_t){}};
    return String{view};
}

String String::nullTerminatedGlobalView(StringView view) {
    if(view.flags() >= (StringViewFlag::NullTerminated|StringViewFlag::Global))
        return String{view.data(), view.size(), [](char*, std::size_t){}};
    return String{view};
}

inline void String::construct(const char* data, std::size_t size) {
    /* If the size is small enough for SSO, use that. Not using <= because we
       need to store the null terminator as well. */
    if(size < Implementation::SmallStringSize) {
        std::memcpy(_small.data, data, size);
        _small.data[size] = '\0';
        _small.size = size | SmallSize;

    /* Otherwise allocate. Assuming the size is small enough -- this should
       have been checked in the caller already. */
    } else {
        _large.data = new char[size+1];
        std::memcpy(_large.data, data, size);
        _large.data[size] = '\0';
        _large.size = size;
        _large.deleter = nullptr;
    }
}

inline void String::destruct() {
    /* If not SSO, delete the data */
    if(_small.size & 0x80) return;
    if(_large.deleter) _large.deleter(_large.data, _large.size);
    else delete[] _large.data;
}

inline std::pair<const char*, std::size_t> String::dataInternal() const {
    if(_small.size & 0x80)
        return {_small.data, _small.size & ~SmallSizeMask};
    return {_large.data, _large.size & ~LargeSizeMask};
}

String::String() noexcept {
    /* Create a zero-size small string to fullfil the guarantee of data() being
       always non-null and null-terminated */
    _small.data[0] = '\0';
    _small.size = SmallSize;
}

String::String(const StringView view): String{view._data, view._size & ~Implementation::StringViewSizeMask} {}

String::String(const MutableStringView view): String{view._data, view._size & ~Implementation::StringViewSizeMask} {}

String::String(const ArrayView<const char> view): String{view.data(), view.size()} {}

String::String(const ArrayView<char> view): String{view.data(), view.size()} {}

String::String(const char* const data): String{data, data ? std::strlen(data) : 0} {}

String::String(const char* const data, const std::size_t size)
    #ifdef CORRADE_GRACEFUL_ASSERT
    /* Zero-init the contents so the destructor doesn't crash if we assert here */
    : _large{}
    #endif
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
        "Containers::String: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size, );
    CORRADE_ASSERT(data || !size,
        "Containers::String: received a null string of size" << size, );

    construct(data, size);
}

String::String(AllocatedInitT, const StringView view): String{AllocatedInit, view._data, view._size & ~Implementation::StringViewSizeMask} {}

String::String(AllocatedInitT, const MutableStringView view): String{AllocatedInit, view._data, view._size & ~Implementation::StringViewSizeMask} {}

String::String(AllocatedInitT, const ArrayView<const char> view): String{AllocatedInit, view.data(), view.size()} {}

String::String(AllocatedInitT, const ArrayView<char> view): String{AllocatedInit, view.data(), view.size()} {}

String::String(AllocatedInitT, const char* const data): String{AllocatedInit, data, data ? std::strlen(data) : 0} {}

String::String(AllocatedInitT, const char* const data, const std::size_t size)
    #ifdef CORRADE_GRACEFUL_ASSERT
    /* Zero-init the contents so the destructor doesn't crash if we assert here */
    : _large{}
    #endif
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
        "Containers::String: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size, );
    CORRADE_ASSERT(data || !size,
        "Containers::String: received a null string of size" << size, );

    _large.data = new char[size+1];
    std::memcpy(_large.data, data, size);
    _large.data[size] = '\0';
    _large.size = size;
    _large.deleter = nullptr;
}

String::String(char* const data, const std::size_t size, void(*deleter)(char*, std::size_t)) noexcept
    #ifdef CORRADE_GRACEFUL_ASSERT
    /* Zero-init the contents so the destructor doesn't crash if we assert here */
    : _large{}
    #endif
{
    CORRADE_ASSERT(size < std::size_t{1} << (sizeof(std::size_t)*8 - 2),
        "Containers::String: string expected to be smaller than 2^" << Utility::Debug::nospace << sizeof(std::size_t)*8 - 2 << "bytes, got" << size, );
    CORRADE_ASSERT(data && !data[size],
        "Containers::String: can only take ownership of a non-null null-terminated array", );

    _large.data = data;
    _large.size = size;
    _large.deleter = deleter;
}

String::~String() { destruct(); }

String::String(const String& other) {
    const std::pair<const char*, std::size_t> data = other.dataInternal();
    construct(data.first, data.second);
}

String::String(String&& other) noexcept {
    /* Similarly as in operator=(String&&), the following works also in case of
       SSO, as for small string we would be doing a copy of _small.data and
       then also a copy of _small.size *including* the two highest bits */
    _large.data = other._large.data;
    _large.size = other._large.size;
    _large.deleter = other._large.deleter;
    other._large.data = nullptr;
    other._large.size = 0;
    other._large.deleter = nullptr;
}

String& String::operator=(const String& other) {
    destruct();

    const std::pair<const char*, std::size_t> data = other.dataInternal();
    construct(data.first, data.second);
    return *this;
}

String& String::operator=(String&& other) noexcept {
    /* Simply swap the contents, which will do the right thing always:

       - If both are allocated, swapping just swaps the pointers and sizes,
         and each instance will later correctly delete its own.
       - If the other is allocated and ours is small, the other gets our small
         string and we get the pointer and deleter in exchange. We'll delete
         the newly acquired pointer and the other will do nothing as it has our
         small data.
       - If we're allocated and the other is small, it's just the inverse of
         the above.
       - If both are small, there's just data exchange, with neither instance
         deleting anything. */
    using std::swap;
    swap(other._large.data, _large.data);
    swap(other._large.size, _large.size);
    swap(other._large.deleter, _large.deleter);
    return *this;
}

String::operator ArrayView<const char>() const noexcept {
    const std::pair<const char*, std::size_t> data = dataInternal();
    return {data.first, data.second};
}

String::operator ArrayView<const void>() const noexcept {
    const std::pair<const char*, std::size_t> data = dataInternal();
    return {data.first, data.second};
}

String::operator ArrayView<char>() noexcept {
    const std::pair<const char*, std::size_t> data = dataInternal();
    return {const_cast<char*>(data.first), data.second};
}

String::operator ArrayView<void>() noexcept {
    const std::pair<const char*, std::size_t> data = dataInternal();
    return {const_cast<char*>(data.first), data.second};
}

const char* String::data() const {
    if(_small.size & 0x80) return _small.data;
    return _large.data;
}

char* String::data() {
    if(_small.size & 0x80) return _small.data;
    return _large.data;
}

bool String::isEmpty() const {
    if(_small.size & 0x80) return !(_small.size & ~SmallSizeMask);
    return !_large.size;
}

auto String::deleter() const -> Deleter {
    CORRADE_ASSERT(!(_small.size & 0x80),
        "Containers::String::deleter(): cannot call on a SSO instance", {});
    return _large.deleter;
}

std::size_t String::size() const {
    if(_small.size & 0x80) return _small.size & ~SmallSizeMask;
    return _large.size;
}

char* String::begin() {
    if(_small.size & 0x80) return _small.data;
    return _large.data;
}

const char* String::begin() const {
    if(_small.size & 0x80) return _small.data;
    return _large.data;
}

const char* String::cbegin() const {
    if(_small.size & 0x80) return _small.data;
    return _large.data;
}

char* String::end() {
    if(_small.size & 0x80) return _small.data + (_small.size & ~SmallSizeMask);
    return _large.data + _large.size;
}

const char* String::end() const {
    if(_small.size & 0x80) return _small.data + (_small.size & ~SmallSizeMask);
    return _large.data + _large.size;
}

const char* String::cend() const {
    if(_small.size & 0x80) return _small.data + (_small.size & ~SmallSizeMask);
    return _large.data + _large.size;
}

/** @todo does it make a practical sense (debug perf) to rewrite these two
    directly without delegating to size()/begin()/end()? i don't think so */

char& String::front() {
    CORRADE_ASSERT(size(), "Containers::String::front(): string is empty", *begin());
    return *begin();
}

char String::front() const {
    return const_cast<String&>(*this).front();
}

char& String::back() {
    CORRADE_ASSERT(size(), "Containers::String::back(): string is empty", *(end() - 1));
    return *(end() - 1);
}

char String::back() const {
    return const_cast<String&>(*this).back();
}

char& String::operator[](std::size_t i) {
    if(_small.size & 0x80) return _small.data[i];
    return _large.data[i];
}

char String::operator[](std::size_t i) const {
    if(_small.size & 0x80) return _small.data[i];
    return _large.data[i];
}

MutableStringView String::slice(char* const begin, char* const end) {
    return MutableStringView{*this}.slice(begin, end);
}

StringView String::slice(const char* const begin, const char* const end) const {
    return StringView{*this}.slice(begin, end);
}

MutableStringView String::slice(const std::size_t begin, const std::size_t end) {
    return MutableStringView{*this}.slice(begin, end);
}

StringView String::slice(const std::size_t begin, const std::size_t end) const {
    return StringView{*this}.slice(begin, end);
}

MutableStringView String::prefix(char* const end) {
    return MutableStringView{*this}.prefix(end);
}

StringView String::prefix(const char* const end) const {
    return StringView{*this}.prefix(end);
}

MutableStringView String::prefix(const std::size_t end) {
    return MutableStringView{*this}.prefix(end);
}

StringView String::prefix(const std::size_t end) const {
    return StringView{*this}.prefix(end);
}

MutableStringView String::suffix(char* const begin) {
    return MutableStringView{*this}.suffix(begin);
}

StringView String::suffix(const char* const begin) const {
    return StringView{*this}.suffix(begin);
}

MutableStringView String::suffix(const std::size_t begin) {
    return MutableStringView{*this}.suffix(begin);
}

StringView String::suffix(const std::size_t begin) const {
    return StringView{*this}.suffix(begin);
}

MutableStringView String::except(const std::size_t count) {
    return MutableStringView{*this}.except(count);
}

StringView String::except(const std::size_t count) const {
    return StringView{*this}.except(count);
}

Array<MutableStringView> String::split(const char delimiter) & {
    return MutableStringView{*this}.split(delimiter);
}

Array<StringView> String::split(const char delimiter) const & {
    return StringView{*this}.split(delimiter);
}

Array<MutableStringView> String::splitWithoutEmptyParts(const char delimiter) & {
    return MutableStringView{*this}.splitWithoutEmptyParts(delimiter);
}

Array<StringView> String::splitWithoutEmptyParts(const char delimiter) const & {
    return StringView{*this}.splitWithoutEmptyParts(delimiter);
}

Array<MutableStringView> String::splitWithoutEmptyParts(const StringView delimiters) & {
    return MutableStringView{*this}.splitWithoutEmptyParts(delimiters);
}

Array<StringView> String::splitWithoutEmptyParts(const StringView delimiters) const & {
    return StringView{*this}.splitWithoutEmptyParts(delimiters);
}

Array<MutableStringView> String::splitWithoutEmptyParts() & {
    return MutableStringView{*this}.splitWithoutEmptyParts();
}

Array<StringView> String::splitWithoutEmptyParts() const & {
    return StringView{*this}.splitWithoutEmptyParts();
}

Array3<MutableStringView> String::partition(const char separator) & {
    return MutableStringView{*this}.partition(separator);
}

Array3<StringView> String::partition(const char separator) const & {
    return StringView{*this}.partition(separator);
}

bool String::hasPrefix(const StringView prefix) const {
    return StringView{*this}.hasPrefix(prefix);
}

bool String::hasSuffix(const StringView suffix) const {
    return StringView{*this}.hasSuffix(suffix);
}

MutableStringView String::stripPrefix(const StringView prefix) & {
    return MutableStringView{*this}.stripPrefix(prefix);
}

StringView String::stripPrefix(const StringView prefix) const & {
    return StringView{*this}.stripPrefix(prefix);
}

MutableStringView String::stripSuffix(const StringView suffix) & {
    return MutableStringView{*this}.stripSuffix(suffix);
}

StringView String::stripSuffix(const StringView suffix) const & {
    return StringView{*this}.stripSuffix(suffix);
}

char* String::release() {
    CORRADE_ASSERT(!(_small.size & 0x80),
        "Containers::String::release(): cannot call on a SSO instance", {});
    char* data = _large.data;

    /* Create a zero-size small string to fullfil the guarantee of data() being
       always non-null and null-terminated */
    _small.data[0] = '\0';
    _small.size = SmallSize;

    return data;
}

namespace Implementation {

String StringConverter<std::string>::from(const std::string& other) {
    return String{other.data(), other.size()};
}

std::string StringConverter<std::string>::to(const String& other) {
    return std::string{other.data(), other.size()};
}

}

}}
