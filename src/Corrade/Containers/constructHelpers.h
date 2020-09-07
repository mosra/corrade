#ifndef Corrade_Containers_constructHelpers_h
#define Corrade_Containers_constructHelpers_h
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

#include <new>
#include <utility>

#include "Corrade/configure.h"

namespace Corrade { namespace Containers { namespace Implementation {

/* Used by Array.h, GrowableArray.h, StaticArray.h and Optional.h; Pointer.h
   has a variant that isn't in-place so it's implemented directly in there. C++
   has a featurebug where a code like

    struct ExplicitDefault {
        explicit ExplicitDefault() = default;
    };

    struct Foo {
        ExplicitDefault a;
    };

    new Foo{};

   will cause a compile error, saying that "chosen constructor is explicit in
   copy-initialization". On GCC it would print a different warning, saying that
   "converting to ‘Foo’ from initializer list would use explicit constructor
   ‘ExplicitDefault::ExplicitDefault()’", however in some variants of the above
   GCC will be happy and Clang not.

   Doesn't happen when doing `new Foo` or `new Foo()`, but because the
   containers *deliberately* prefer {} over () in order to catch implicit
   conversion issues *and* Corrade and Magnum APIs use explicit parameter-less
   constructors very often, this is very likely to be hit by unsuspecting
   users. One workaround is to add a `Foo() {}` constructor to the struct, but
   that might have unwanted consequences, so instead it's getting worked around
   on library side by using a () when the argument list is empty. Further
   details (Clang-specific): https://stackoverflow.com/q/17264067

   This might be fixed in more recent versions of the C++ standard. */
template<class T, class First, class ...Next> inline void construct(T& value, First&& first, Next&& ...next) {
    new(&value) T{std::forward<First>(first), std::forward<Next>(next)...};
}
template<class T> inline void construct(T& value) {
    new(&value) T();
}

#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ < 5
/* Can't use {} because for plain structs it would attempt to initialize the
   first member with `b` instead of calling the copy/move constructor. See
   copyConstructPlainStruct() and moveConstructPlainStruct() tests for Array,
   GrowableArray, StaticArray, Optional and Pointer for details. */
template<class T> inline void construct(T& value, const T& b) {
    new(&value) T(b);
}
template<class T> inline void construct(T& value, T&& b) {
    new(&value) T(std::move(b));
}
#endif

}}}

#endif
