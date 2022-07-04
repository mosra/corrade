#ifndef Corrade_Containers_StringStlHash_h
#define Corrade_Containers_StringStlHash_h
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
@brief STL @ref std::hash compatibility for @ref Corrade::Containers::String and @ref Corrade::Containers::BasicStringView "StringView"
@m_since_latest

Including this header adds a @ref std::hash specialization for
@ref Corrade::Containers::String / @ref Corrade::Containers::BasicStringView "StringView",
making them usable in @ref std::unordered_map and @ref std::unordered_set. See
@ref Containers-String-stl "String STL compatibility" and
@ref Containers-BasicStringView-stl "StringView STL compatibility" for more
information.
*/

/* Alone, <functional> is relatively big (9kLOC on GCC 11 -std=c++11), but when
   combined with <unordered_map> (12k) it results in just 13k, so it's not like
   we'd gain anything by trying to find a forward declaration. On libc++ 11
   it's 22k for <functional> (!!) and 36k (!!!) for <unordered_map> either
   alone or together with <functional>. So even though the numbers are QUITE
   HORRENDOUS, a forward declaration wouldn't help there either. On GCC 11 and
   -std=c++20 the size goes up to 30k as well. Haha.

   Compared to these, I don't feel like bothering to optimize my side of the
   include chain, even though the MurmurHash2 include could potentially get
   deinlined. */
#include <functional>

#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/MurmurHash2.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace std {

template<> struct hash<Corrade::Containers::StringView> {
    std::size_t operator()(Corrade::Containers::StringView key) const {
        const Corrade::Utility::MurmurHash2 hash;
        const Corrade::Utility::HashDigest<sizeof(std::size_t)> digest = hash(key.data(), key.size());
        return *reinterpret_cast<const std::size_t*>(digest.byteArray());
    }
};

template<> struct hash<Corrade::Containers::MutableStringView>: hash<Corrade::Containers::StringView> {};

template<> struct hash<Corrade::Containers::String> {
    std::size_t operator()(const Corrade::Containers::String& key) const {
        const Corrade::Utility::MurmurHash2 hash;
        const Corrade::Utility::HashDigest<sizeof(std::size_t)> digest = hash(key.data(), key.size());
        return *reinterpret_cast<const std::size_t*>(digest.byteArray());
    }
};

}
#endif

#endif
