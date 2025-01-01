#ifndef Corrade_Utility_StlForwardTupleSizeElement_h
#define Corrade_Utility_StlForwardTupleSizeElement_h
/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2022 Stanislaw Halik <sthalik@misaki.pl>

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
@brief Forward declaration for @m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size) and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
@m_since_latest

On @ref CORRADE_TARGET_LIBCXX "libc++", @ref CORRADE_TARGET_LIBSTDCXX "libstdc++"
and @ref CORRADE_TARGET_DINKUMWARE "MSVC STL" forward-declares the
@m_class{m-doc-external} [std::tuple_size](https://en.cppreference.com/w/cpp/utility/tuple_size)
and @m_class{m-doc-external} [std::tuple_element](https://en.cppreference.com/w/cpp/utility/tuple_element)
types inside the STL namespace in an implementation-specific way. On other
implementations is equivalent to @cpp #include <utility> @ce, which is
guaranteed to contain those in order to define them for @ref std::pair.
*/

#include <cstddef> /* size_t, std::size_t */

#include "Corrade/configure.h"

#ifdef CORRADE_TARGET_LIBCXX
    /* Defined in <__config>, which is already pulled in from <ciso646> or
       <version> that configure.h has to include in order to detect the STL
       being used. */
    _LIBCPP_BEGIN_NAMESPACE_STD
#elif defined(CORRADE_TARGET_LIBSTDCXX)
    /* Defined in <bits/c++config.h>. Pulled in from <ciso646> or <version> by
       configure.h, but only since GCC 6.1. On versions before <cstddef> from
       above pulls that in. Versions before GCC 4.6(?) had
       _GLIBCXX_BEGIN_NAMESPACE(std) instead, we don't care about those. */
    #include <bits/c++config.h>
    namespace std _GLIBCXX_VISIBILITY(default) { _GLIBCXX_BEGIN_NAMESPACE_VERSION
#elif defined(CORRADE_TARGET_DINKUMWARE)
    /* Defined in <yvals_core.h>, again pulled in from <ciso646> or <version>
       by configure.h */
    _STD_BEGIN
#endif

/** @todo the include is indented to work around acme.py extracting it to the
    top, fix properly */
#if defined(CORRADE_TARGET_LIBCXX) || defined(CORRADE_TARGET_LIBSTDCXX) || defined(CORRADE_TARGET_DINKUMWARE)
    template<size_t, class> struct tuple_element;
    template<class> struct tuple_size;
#else
    #include <utility>
#endif

#ifdef CORRADE_TARGET_LIBCXX
    _LIBCPP_END_NAMESPACE_STD
#elif defined(CORRADE_TARGET_LIBSTDCXX)
    _GLIBCXX_END_NAMESPACE_VERSION }
#elif defined CORRADE_TARGET_MSVC
    _STD_END
#endif

#endif
