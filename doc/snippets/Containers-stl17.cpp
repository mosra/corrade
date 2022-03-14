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

#include <string>

#include "Corrade/Containers/OptionalStl.h"
#include "Corrade/Containers/StringStlView.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

int main() {
{
/* The include is already above, so doing it again here should be harmless */
/* [Optional] */
#include <Corrade/Containers/Optional.h>

DOXYGEN_ELLIPSIS()

std::optional<int> a{5};
Containers::Optional<int> b(a);

std::optional<std::string> c(Containers::Optional<std::string>{"hello"});

auto d = Containers::optional(std::optional<int>{17});
        // d is Containers::Optional<int>
/* [Optional] */
}

{
/* The include is already above, so doing it again here should be harmless */
/* [StringView] */
#include <Corrade/Containers/StringStlView.h>

DOXYGEN_ELLIPSIS()

std::string_view a = "Hello world!";
Containers::StringView b = a.substr(5);
/* [StringView] */
static_cast<void>(b);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [String] */
#include <Corrade/Containers/StringStlView.h>

DOXYGEN_ELLIPSIS()

std::string_view a = "Hello world!";
Containers::String b = a.substr(5);
/* [String] */
}
}
