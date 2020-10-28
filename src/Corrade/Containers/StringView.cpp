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

#include "StringView.h"
#include "StringStl.h"

#include <cstring>
#include <string>

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Utility/DebugStl.h"

namespace Corrade { namespace Containers {

template<class T> BasicStringView<T>::BasicStringView(T* data) noexcept: BasicStringView{data,
    data ? std::strlen(data) : 0,
    data ? StringViewFlag::NullTerminated : StringViewFlags{}} {}

template<class T> BasicStringView<T>::BasicStringView(String& string) noexcept: BasicStringView{string.data(), string.size(), StringViewFlag::NullTerminated} {}

/* Yes, I'm also surprised this works. On Windows (MSVC, clang-cl and MinGw) it
   needs an explicit export otherwise the symbol doesn't get exported. */
template<> template<> CORRADE_UTILITY_EXPORT BasicStringView<const char>::BasicStringView(const String& string) noexcept: BasicStringView{string.data(), string.size(), StringViewFlag::NullTerminated} {}

template<class T> BasicStringView<T>::BasicStringView(ArrayView<T> other, StringViewFlags flags) noexcept: BasicStringView{other.data(), other.size(), flags} {}

template<class T> BasicStringView<T>::operator ArrayView<T>() const noexcept {
    return {_data, size()};
}

template<class T> BasicStringView<T>::operator ArrayView<typename std::conditional<std::is_const<T>::value, const void, void>::type>() const noexcept {
    return {_data, size()};
}

/** @todo does it make a practical sense (debug perf) to rewrite these two
    directly without delegating to size()? i don't think so */

template<class T> T& BasicStringView<T>::front() const {
    CORRADE_ASSERT(size(), "Containers::StringView::front(): view is empty", _data[0]);
    return _data[0];
}

template<class T> T& BasicStringView<T>::back() const {
    CORRADE_ASSERT(size(), "Containers::StringView::back(): view is empty", _data[size() - 1]);
    return _data[size() - 1];
}

template<class T> Array<BasicStringView<T>> BasicStringView<T>::split(const char delimiter) const {
    Array<BasicStringView<T>> parts;
    T* const end = this->end();
    T* oldpos = _data;
    T* pos;
    while(oldpos < end && (pos = static_cast<T*>(std::memchr(oldpos, delimiter, end - oldpos)))) {
        arrayAppend(parts, slice(oldpos, pos));
        oldpos = pos + 1;
    }

    if(!isEmpty())
        arrayAppend(parts, suffix(oldpos));

    return parts;
}

template<class T> Array<BasicStringView<T>> BasicStringView<T>::splitWithoutEmptyParts(const char delimiter) const {
    Array<BasicStringView<T>> parts;
    T* const end = this->end();
    T* oldpos = _data;

    while(oldpos < end) {
        T* pos = static_cast<T*>(std::memchr(oldpos, delimiter, end - oldpos));
        /* Not sure why memchr can't just do this, it would make much more
           sense */
        if(!pos) pos = end;

        if(pos != oldpos)
            arrayAppend(parts, slice(oldpos, pos));

        oldpos = pos + 1;
    }

    return parts;
}

namespace {

/* I don't want to include <algorithm> just for std::find_first_of() and
   unfortunately there's no equivalent in the C string library. Coming close
   are strpbrk() or strcspn() but both of them work with null-terminated
   strings, which is absolutely useless here, not to mention that both do
   *exactly* the same thing, with one returning a pointer but the other an
   offset, so what's the point of having both? What the hell. And there's no
   memcspn() or whatever which would take explicit lengths. Which means I'm
   left to my own devices. Looking at how strpbrk() / strcspn() is done, it
   ranges from trivial code:

    https://github.com/bminor/newlib/blob/6497fdfaf41d47e835fdefc78ecb0a934875d7cf/newlib/libc/string/strcspn.c

   to extremely optimized machine-specific code (don't look, it's GPL):

    https://github.com/bminor/glibc/blob/43b1048ab9418e902aac8c834a7a9a88c501620a/sysdeps/x86_64/multiarch/strcspn-c.c

   and the only trick I realized above the nested loop is using memchr() in an
   inverse way. In all honesty, I think that'll still be *at least* as fast as
   std::find_first_of() because I doubt STL implementations explicitly optimize
   for that case. Yes, std::string::find_first_of() probably would have that,
   but I'd first need to allocate to make use of that and FUCK NO. */
inline const char* findFirstOf(const char* begin, const char* const end, const char* const characters, std::size_t characterCount) {
    for(; begin != end; ++begin)
        if(std::memchr(characters, *begin, characterCount)) return begin;
    return end;
}

}

template<class T> Array<BasicStringView<T>> BasicStringView<T>::splitWithoutEmptyParts(const Containers::StringView delimiters) const {
    Array<BasicStringView<T>> parts;
    const char* const characters = delimiters.begin();
    const std::size_t characterCount = delimiters.size();
    T* const end = this->end();
    T* oldpos = _data;

    while(oldpos < end) {
        T* const pos = const_cast<T*>(findFirstOf(oldpos, end, characters, characterCount));
        if(pos != oldpos)
            arrayAppend(parts, slice(oldpos, pos));

        oldpos = pos + 1;
    }

    return parts;
}

#ifndef CORRADE_MSVC2019_COMPATIBILITY
namespace {
    using namespace Containers::Literals;
    constexpr Containers::StringView Whitespace = " \t\f\v\r\n"_s;
}
#endif

template<class T> Array<BasicStringView<T>> BasicStringView<T>::splitWithoutEmptyParts() const {
    /* If I use an externally defined view here, MSVC (2015, 2017, 2019) will
       blow up on the explicit template instantiation with

        ..\src\Corrade\Containers\StringView.cpp(176): error C2946: explicit instantiation; 'Corrade::Containers::BasicStringView<const char>::<lambda_e55a1a450af96fadfe37cfb50a99d6f7>' is not a template-class specialization

       I spent an embarrassing amount of time trying to find what lambda it
       doesn't like, reimplemented std::find_first_of() used in
       splitWithoutEmptyParts(), added a non-asserting variants of slice() etc,
       but nothing helped. Only defining CORRADE_NO_ASSERT at the very top made
       the problem go away, and I discovered this only by accident after
       removing basically all other code. WHAT THE FUCK, MSVC. */
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    using namespace Containers::Literals;
    return splitWithoutEmptyParts(" \t\f\v\r\n"_s);
    #else
    return splitWithoutEmptyParts(Whitespace);
    #endif
}

template<class T> Array3<BasicStringView<T>> BasicStringView<T>::partition(const char separator) const {
    /** @todo partition() using multiple characters, would need implementing
        a non-shitty strstr() that can work on non-null-terminated strings */
    /** @todo and then rpartition(), which has absolutely no standard library
        functions either, SIGH */

    const std::size_t size = this->size();
    T* const pos = static_cast<T*>(std::memchr(_data, separator, size));
    return {
        pos ? prefix(pos) : *this,
        pos ? slice(pos, pos + 1) : suffix(size),
        pos ? suffix(pos + 1) : suffix(size)
    };
}

template<class T> bool BasicStringView<T>::hasPrefix(const StringView prefix) const {
    const std::size_t prefixSize = prefix.size();
    if(size() < prefixSize) return false;

    return std::strncmp(_data, prefix.data(), prefixSize) == 0;
}

template<class T> bool BasicStringView<T>::hasSuffix(const StringView suffix) const {
    const std::size_t size = this->size();
    const std::size_t suffixSize = suffix.size();
    if(size < suffixSize) return false;

    return std::strncmp(_data + size - suffixSize, suffix.data(), suffixSize) == 0;
}

template<class T> BasicStringView<T> BasicStringView<T>::stripPrefix(const StringView prefix) const {
    CORRADE_ASSERT(hasPrefix(prefix),
        "Containers::StringView::stripPrefix(): string doesn't begin with" << prefix, {});
    return suffix(prefix.size());
}

template<class T> BasicStringView<T> BasicStringView<T>::stripSuffix(const StringView suffix) const {
    CORRADE_ASSERT(hasSuffix(suffix),
        "Containers::StringView::stripSuffix(): string doesn't end with" << suffix, {});
    return prefix(size() - suffix.size());
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template class
    /* GCC needs the export macro on the class definition (and here it warns
       that the type is already defined so the export is ignored), while Clang
       and MSVC need it here (and ignore it on the declaration) */
    #if defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC)
    CORRADE_UTILITY_EXPORT
    #endif
    BasicStringView<char>;
template class
    #if defined(CORRADE_TARGET_CLANG) || defined(CORRADE_TARGET_MSVC)
    CORRADE_UTILITY_EXPORT
    #endif
    BasicStringView<const char>;
#endif

bool operator==(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    return aSize == (b._size & ~Implementation::StringViewSizeMask) &&
        std::memcmp(a._data, b._data, aSize) == 0;
}

bool operator!=(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    return aSize != (b._size & ~Implementation::StringViewSizeMask) ||
        std::memcmp(a._data, b._data, aSize) != 0;
}

namespace {
    /* Because std::min needs <algorithm> and is shitty */
    inline std::size_t min(std::size_t a, std::size_t b) { return b < a ? b : a; }
}

bool operator<(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    const std::size_t bSize = b._size & ~Implementation::StringViewSizeMask;
    const int result = std::memcmp(a._data, b._data, min(aSize, bSize));
    if(result != 0) return result < 0;
    if(aSize < bSize) return true;
    return false;
}

bool operator<=(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    const std::size_t bSize = b._size & ~Implementation::StringViewSizeMask;
    const int result = std::memcmp(a._data, b._data, min(aSize, bSize));
    if(result != 0) return result < 0;
    if(aSize <= bSize) return true;
    return false;
}

bool operator>=(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    const std::size_t bSize = b._size & ~Implementation::StringViewSizeMask;
    const int result = std::memcmp(a._data, b._data, min(aSize, bSize));
    if(result != 0) return result > 0;
    if(aSize >= bSize) return true;
    return false;
}

bool operator>(const StringView a, const StringView b) {
    /* Not using the size() accessor to speed up debug builds */
    const std::size_t aSize = a._size & ~Implementation::StringViewSizeMask;
    const std::size_t bSize = b._size & ~Implementation::StringViewSizeMask;
    const int result = std::memcmp(a._data, b._data, min(aSize, bSize));
    if(result != 0) return result > 0;
    if(aSize > bSize) return true;
    return false;
}

Utility::Debug& operator<<(Utility::Debug& debug, const StringViewFlag value) {
    debug << "Containers::StringViewFlag" << Utility::Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(v) case StringViewFlag::v: return debug << "::" #v;
        _c(Global)
        _c(NullTerminated)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Utility::Debug::nospace << reinterpret_cast<void*>(std::size_t(value)) << Utility::Debug::nospace << ")";
}

Utility::Debug& operator<<(Utility::Debug& debug, const StringViewFlags value) {
    return enumSetDebugOutput(debug, value, "Containers::StringViewFlags{}", {
        StringViewFlag::Global,
        StringViewFlag::NullTerminated});
}

namespace Implementation {

StringView StringViewConverter<const char, std::string>::from(const std::string& other) {
    return StringView{other.data(), other.size(), StringViewFlag::NullTerminated};
}

std::string StringViewConverter<const char, std::string>::to(StringView other) {
    return std::string{other.data(), other.size()};
}

MutableStringView StringViewConverter<char, std::string>::from(std::string& other) {
    return MutableStringView{other.size() ? &other[0] : nullptr, other.size(), StringViewFlag::NullTerminated};
}

std::string StringViewConverter<char, std::string>::to(MutableStringView other) {
    return std::string{other.data(), other.size()};
}

}

}}
