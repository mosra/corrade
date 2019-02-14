/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>

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

#include "Corrade/Corrade.h"
#if __has_include(<span>)
#include "Corrade/Containers/ArrayViewStlSpan.h"
#endif

using namespace Corrade;

int main() {
#if __has_include(<span>)
{
/* [ArrayView] */
std::span<int> a;
Containers::ArrayView<int> b = a;

float c[3]{42.0f, 13.37f, -25.0f};
std::span<float, 3> d = Containers::staticArrayView(c);
/* [ArrayView] */
static_cast<void>(b);
static_cast<void>(d);
}

{
int data[3]{};
/* [ArrayView-stupid-span] */
std::span<int> a;
std::span<int, 3> b{data};
Containers::ArrayView<int> c = a;               // correct
Containers::ArrayView<int> d = b;               // correct
//Containers::StaticArrayView<3, int> e = a;    // correctly doesn't compile
Containers::StaticArrayView<3, int> f = b;      // correct
//Containers::StaticArrayView<4, int> g = b;    // correctly doesn't compile

std::span<int> i = c;                           // correct
std::span<int, 3> j = c;                        // incorrectly compiles, UB :(
std::span<int, 3> k = f;                        // correct
std::span<int, 4> l = f;                        // incorrectly compiles, UB :(
/* [ArrayView-stupid-span] */
static_cast<void>(d);
static_cast<void>(i);
static_cast<void>(j);
static_cast<void>(k);
static_cast<void>(l);
}
#endif
}
