/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023
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

#include <string_view>

#include "Corrade/Corrade.h"
#if __has_include(<span>)
#include "Corrade/Containers/ArrayViewStlSpan.h"
#endif
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Containers/StringStlView.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

/* Make sure the name doesn't conflict with any other snippets to avoid linker
   warnings, unlike with `int main()` there now has to be a declaration to
   avoid -Wmisssing-prototypes */
void mainContainersStl2a();
void mainContainersStl2a() {
#if __has_include(<span>)
{
/* The include is already above, so doing it again here should be harmless */
/* [ArrayView] */
#include <Corrade/Containers/ArrayViewStlSpan.h>

DOXYGEN_ELLIPSIS()

std::span<int> a;
Containers::ArrayView<int> b = a;

float c[3]{42.0f, 13.37f, -25.0f};
std::span<float, 3> d = Containers::staticArrayView(c);
/* [ArrayView] */
static_cast<void>(b);
static_cast<void>(d);
}

{
/* The includes are already above, so doing it again here should be harmless */
/* [StringIterable] */
#include <Corrade/Containers/ArrayViewStlSpan.h>
#include <Corrade/Containers/StringStlView.h>

DOXYGEN_ELLIPSIS()

std::span<std::string_view> a = DOXYGEN_ELLIPSIS({});
Containers::StringIterable b = a;
/* [StringIterable] */
static_cast<void>(b);
}
#endif
}
