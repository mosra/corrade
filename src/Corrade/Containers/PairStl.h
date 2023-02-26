#ifndef Corrade_Containers_PairStl_h
#define Corrade_Containers_PairStl_h
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
@brief STL compatibility for @ref Corrade::Containers::Pair
@m_since_latest

Including this header allows you to *explicitly* convert between
@ref Corrade::Containers::Pair and @ref std::pair using copy / move
construction and assignment. See @ref Containers-Pair-stl for more information.
*/

#include <utility>

#include "Corrade/Containers/Pair.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT

#if CORRADE_CXX_STANDARD >= 201703
namespace std {
template<class F, class S> struct tuple_size<Corrade::Containers::Pair<F, S>> : std::integral_constant<std::size_t, 2> {};
template<class F, class S> struct tuple_element<0, Corrade::Containers::Pair<F, S>> { using type = F; };
template<class F, class S> struct tuple_element<1, Corrade::Containers::Pair<F, S>> { using type = S; };
}
#endif

namespace Corrade { namespace Containers {

#if CORRADE_CXX_STANDARD >= 201703
template<std::size_t N, class F, class S> constexpr const std::tuple_element_t<N, Pair<F, S>>& get(const Pair<F, S>& value) { if constexpr(N == 1) return value.second(); else return value.first(); }
template<std::size_t N, class F, class S> constexpr std::tuple_element_t<N, Pair<F, S>>& get(Pair<F, S>& value) { if constexpr(N == 1) return value.second(); else return value.first(); }
template<std::size_t N, class F, class S> constexpr std::tuple_element_t<N, Pair<F, S>> get(Pair<F, S>&& value) { if constexpr(N == 1) return value.second(); else return value.first(); }
#endif

namespace Implementation {

template<class F, class S> struct PairConverter<F, S, std::pair<F, S>> {
    static Pair<F, S> from(const std::pair<F, S>& other) {
        return {other.first, other.second};
    }

    static Pair<F, S> from(std::pair<F, S>&& other) {
        return {Utility::move(other.first), Utility::move(other.second)};
    }

    static std::pair<F, S> to(const Pair<F, S>& other) {
        return {other.first(), other.second()};
    }

    static std::pair<F, S> to(Pair<F, S>&& other) {
        return {Utility::move(other.first()), Utility::move(other.second())};
    }
};

template<class F, class S> struct DeducedPairConverter<std::pair<F, S>>: PairConverter<F, S, std::pair<F, S>> {};

}}}
#endif

#endif
