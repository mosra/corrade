#ifndef Corrade_Containers_initializeHelpers_h
#define Corrade_Containers_initializeHelpers_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021
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

#include <new>
#include <cstddef>
#include <cstring>

#include "Corrade/configure.h"
#include "Corrade/Tags.h"

/* Stuff shared by GrowableArray.h */

namespace Corrade { namespace Containers { namespace Implementation {

#ifndef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
/* The std::has_trivial_default_constructor / std::has_trivial_copy_constructor
   is deprecated in GCC 5+ but we can't detect libstdc++ version when using
   Clang. The builtins aren't deprecated but for those GCC commits suicide with
    error: use of built-in trait ‘__has_trivial_copy(T)’ in function signature; use library traits instead
   so, well, i'm defining my own! See CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
   for even more fun stories. */
template<class T> struct IsTriviallyConstructibleOnOldGcc: std::integral_constant<bool, __has_trivial_constructor(T)> {};
/* Need also __has_trivial_destructor() otherwise it says true for types with
   deleted copy and non-trivial destructors */
template<class T> struct IsTriviallyCopyableOnOldGcc: std::integral_constant<bool, __has_trivial_copy(T) && __has_trivial_destructor(T)> {};
#endif

enum: std::size_t {
    MinAllocatedSize =
        /* Emscripten has __STDCPP_DEFAULT_NEW_ALIGNMENT__ set to 16 but
           actually does just 8, so don't use this macro there:
           https://github.com/emscripten-core/emscripten/issues/10072 */
        #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        __STDCPP_DEFAULT_NEW_ALIGNMENT__
        #else
        2*sizeof(std::size_t)
        #endif
};

template<class T> inline void arrayConstruct(Corrade::DefaultInitT, T*, T*, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Nothing to do */
}

template<class T> inline void arrayConstruct(Corrade::DefaultInitT, T* begin, T* const end, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Needs to be < because sometimes begin > end. No {}, we want trivial
       types non-initialized */
    for(; begin < end; ++begin) new(begin) T;
}

template<class T> inline void arrayConstruct(Corrade::ValueInitT, T* const begin, T* const end, typename std::enable_if<
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    if(begin < end) std::memset(begin, 0, (end - begin)*sizeof(T));
}

template<class T> inline void arrayConstruct(Corrade::ValueInitT, T* begin, T* const end, typename std::enable_if<!
    #ifdef CORRADE_STD_IS_TRIVIALLY_TRAITS_SUPPORTED
    std::is_trivially_constructible<T>::value
    #else
    IsTriviallyConstructibleOnOldGcc<T>::value
    #endif
>::type* = nullptr) {
    /* Needs to be < because sometimes begin > end */
    for(; begin < end; ++begin) new(begin) T{};
}

}}}

#endif
