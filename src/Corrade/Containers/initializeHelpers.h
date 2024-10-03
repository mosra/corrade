#ifndef Corrade_Containers_initializeHelpers_h
#define Corrade_Containers_initializeHelpers_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
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
#include "Corrade/Containers/constructHelpers.h"

/* Stuff shared by GrowableArray.h and Utility/Memory.h */

namespace Corrade { namespace Containers { namespace Implementation {

enum: std::size_t {
    DefaultAllocationAlignment =
        /* Emscripten has __STDCPP_DEFAULT_NEW_ALIGNMENT__ set to 16 but
           actually does just 8, so don't use this macro there:
           https://github.com/emscripten-core/emscripten/issues/10072 */
        #if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        __STDCPP_DEFAULT_NEW_ALIGNMENT__
        #else
        2*sizeof(std::size_t)
        #endif
};

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_constructor(T)
    #else
    std::is_trivially_constructible<T>::value
    #endif
, int>::type = 0> inline void arrayConstruct(Corrade::DefaultInitT, T*, T*) {
    /* Nothing to do */
}

template<class T, typename std::enable_if<!
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_constructor(T)
    #else
    std::is_trivially_constructible<T>::value
    #endif
, int>::type = 0> inline void arrayConstruct(Corrade::DefaultInitT, T* begin, T* const end) {
    /* Needs to be < because sometimes begin > end. No {}, we want trivial
       types non-initialized */
    for(; begin < end; ++begin) new(begin) T;
}

template<class T, typename std::enable_if<
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_constructor(T)
    #else
    std::is_trivially_constructible<T>::value
    #endif
, int>::type = 0> inline void arrayConstruct(Corrade::ValueInitT, T* const begin, T* const end) {
    if(begin < end) std::memset(begin, 0, (end - begin)*sizeof(T));
}

template<class T, typename std::enable_if<!
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    __has_trivial_constructor(T)
    #else
    std::is_trivially_constructible<T>::value
    #endif
, int>::type = 0> inline void arrayConstruct(Corrade::ValueInitT, T* begin, T* const end) {
    /* Needs to be < because sometimes begin > end. The () instead of {} works
       around a featurebug in C++ where new T{} doesn't work for an explicit
       defaulted constructor. For details see constructHelpers.h and
       GrowableArrayTest::constructorExplicitInCopyInitialization(). */
    for(; begin < end; ++begin) new(begin) T();
}

}}}

#endif
