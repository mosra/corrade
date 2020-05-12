#ifndef Corrade_Containers_OptionalStl_h
#define Corrade_Containers_OptionalStl_h
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

/** @file
@brief STL compatibility for @ref Corrade::Containers::Optional

Including this header allows you to *explicitly* convert between
@ref Corrade::Containers::Optional and @ref std::optional using copy / move
construction and assignment. See @ref Containers-Optional-stl for more
information.
*/

#include <optional>

#include "Corrade/Containers/Optional.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

template<class T> struct OptionalConverter<T, std::optional<T>> {
    static Optional<T> from(const std::optional<T>& other) {
        return other ? Optional<T>{*other} : Containers::NullOpt;
    }

    static Optional<T> from(std::optional<T>&& other) {
        return other ? Optional<T>{*std::move(other)} : Containers::NullOpt;
    }

    static std::optional<T> to(const Optional<T>& other) {
        return other ? std::optional<T>{*other} : std::nullopt;
    }

    static std::optional<T> to(Optional<T>&& other) {
        return other ? std::optional<T>{*std::move(other)} : std::nullopt;
    }
};

template<class T> struct DeducedOptionalConverter<std::optional<T>>: OptionalConverter<T, std::optional<T>> {};

}}}
#endif

#endif
