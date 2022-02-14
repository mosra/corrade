#ifndef Corrade_Containers_TripleStl_h
#define Corrade_Containers_TripleStl_h
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
@brief STL compatibility for @ref Corrade::Containers::Triple
@m_since_latest

Including this header allows you to *explicitly* convert between
@ref Corrade::Containers::Triple and a three-element @ref std::tuple using copy
/ move construction and assignment. See @ref Containers-Triple-stl for more
information.
*/

#include <tuple>

#include "Corrade/Containers/Triple.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

template<class F, class S, class T> struct TripleConverter<F, S, T, std::tuple<F, S, T>> {
    static Triple<F, S, T> from(const std::tuple<F, S, T>& other) {
        return {std::get<0>(other), std::get<1>(other), std::get<2>(other)};
    }

    static Triple<F, S, T> from(std::tuple<F, S, T>&& other) {
        return {Utility::move(std::get<0>(other)), Utility::move(std::get<1>(other)), Utility::move(std::get<2>(other))};
    }

    static std::tuple<F, S, T> to(const Triple<F, S, T>& other) {
        return std::tuple<F, S, T>{other.first(), other.second(), other.third()};
    }

    static std::tuple<F, S, T> to(Triple<F, S, T>&& other) {
        return std::tuple<F, S, T>{Utility::move(other.first()), Utility::move(other.second()), Utility::move(other.third())};
    }
};

template<class F, class S, class T> struct DeducedTripleConverter<std::tuple<F, S, T>>: TripleConverter<F, S, T, std::tuple<F, S, T>> {};

}}}
#endif

#endif
