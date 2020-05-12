#ifndef Corrade_Containers_PointerStl_h
#define Corrade_Containers_PointerStl_h
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
@brief STL compatibility for @ref Corrade::Containers::Pointer

Including this header allows you to seamlessly convert between
@ref Corrade::Containers::Pointer and @ref std::unique_ptr using move
construction / assignment. See @ref Containers-Pointer-stl for more
information.
*/

#include <memory>

#include "Corrade/Containers/Pointer.h"

/* Listing these namespaces doesn't add anything to the docs, so don't */
#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers { namespace Implementation {

template<class T> struct PointerConverter<T, std::unique_ptr<T>> {
    static Pointer<T> from(std::unique_ptr<T>&& other) {
        return Pointer<T>{other.release()};
    }

    static std::unique_ptr<T> to(Pointer<T>&& other) {
        return std::unique_ptr<T>{other.release()};
    }
};

template<class T> struct DeducedPointerConverter<std::unique_ptr<T>>: PointerConverter<T, std::unique_ptr<T>> {};

}}}
#endif

#endif
