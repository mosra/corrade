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

#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StridedDimensions.h"
#include "Corrade/Containers/StructuredBindings.h"
#include "Corrade/Containers/Triple.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

/* Make sure the name doesn't conflict with any other snippets to avoid linker
   warnings, unlike with `int main()` there now has to be a declaration to
   avoid -Wmisssing-prototypes */
void mainContainersCpp17();
void mainContainersCpp17() {
{
/* The include is already above, so doing it again here should be harmless */
/* [Pair-structured-bindings] */
#include <Corrade/Containers/StructuredBindings.h>

DOXYGEN_ELLIPSIS()

auto [first, second] = Containers::pair(42, 3.14f);
/* [Pair-structured-bindings] */
static_cast<void>(first);
static_cast<void>(second);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [StaticArray-structured-bindings] */
#include <Corrade/Containers/StructuredBindings.h>

DOXYGEN_ELLIPSIS()

auto [a, b, c] = Containers::Array3<int>{7, 13, 29};
/* [StaticArray-structured-bindings] */
static_cast<void>(a);
static_cast<void>(b);
static_cast<void>(c);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [StaticArrayView-structured-bindings] */
#include <Corrade/Containers/StructuredBindings.h>

DOXYGEN_ELLIPSIS()

auto [a, b, c] = Containers::ArrayView3<int>{DOXYGEN_ELLIPSIS()};
/* [StaticArrayView-structured-bindings] */
static_cast<void>(a);
static_cast<void>(b);
static_cast<void>(c);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [StridedDimensions-structured-bindings] */
#include <Corrade/Containers/StructuredBindings.h>

DOXYGEN_ELLIPSIS()

auto [height, width] = Containers::Size2D(16, 32);
/* [StridedDimensions-structured-bindings] */
static_cast<void>(height);
static_cast<void>(width);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [Triple-structured-bindings] */
#include <Corrade/Containers/StructuredBindings.h>

DOXYGEN_ELLIPSIS()

auto [first, second, third] = Containers::triple(42, false, 3.14f);
/* [Triple-structured-bindings] */
static_cast<void>(first);
static_cast<void>(second);
static_cast<void>(third);
}
}
