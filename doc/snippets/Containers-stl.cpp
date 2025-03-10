/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
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
#include <vector>

#include "Corrade/Containers/AnyReference.h"
#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayViewStl.h"
#include "Corrade/Containers/Iterable.h"
#include "Corrade/Containers/PairStl.h"
#include "Corrade/Containers/PointerStl.h"
#include "Corrade/Containers/StringStl.h"
#include "Corrade/Containers/StringIterable.h"
#include "Corrade/Containers/ReferenceStl.h"
#include "Corrade/Containers/TripleStl.h"

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
#include "Corrade/Utility/FileWatcher.h"
#endif

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Corrade;

/* Make sure the name doesn't conflict with any other snippets to avoid linker
   warnings, unlike with `int main()` there now has to be a declaration to
   avoid -Wmisssing-prototypes */
void mainContainersStl();
void mainContainersStl() {
{
/* The include is already above, so doing it again here should be harmless */
/* [ArrayView] */
#include <Corrade/Containers/ArrayViewStl.h>

DOXYGEN_ELLIPSIS()

std::vector<int> a;

Containers::ArrayView<int> b = a;
/* [ArrayView] */
static_cast<void>(b);
}

{
/* [Array-initializer-list] */
std::vector<int> a(5);                  // a.size() == 5
std::vector<int> b{5};                  // b.size() == 1, b[0] == 5

Containers::Array<int> c{5};            // c.size() == 5
auto d = Containers::array<int>({5});   // d.size() == 1, d[0] == 5
/* [Array-initializer-list] */
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN)
{
/* [Iterable-usage] */
void foo(const Containers::Iterable<Utility::FileWatcher>&);

Utility::FileWatcher a{DOXYGEN_ELLIPSIS("a")}, b{DOXYGEN_ELLIPSIS("b")};
Utility::FileWatcher cArray[3]{DOXYGEN_ELLIPSIS(Utility::FileWatcher{"c0"}, Utility::FileWatcher{"c1"}, Utility::FileWatcher{"c2"})};
Containers::Array<Containers::Reference<Utility::FileWatcher>> array{DOXYGEN_ELLIPSIS(InPlaceInit, {a, b})};
std::vector<Utility::FileWatcher> vector{DOXYGEN_ELLIPSIS()};

foo({a, b}); /* passing (references to) variables directly */
foo(cArray); /* passing a C array */
foo(array);  /* passing an array of references */
foo(vector); /* passing a STL vector */
/* [Iterable-usage] */

/* [Iterable-usage-boom] */
Containers::Iterable<Utility::FileWatcher> iterable = {a, b};

foo(iterable); // Boom!
/* [Iterable-usage-boom] */
}
#endif

{
/* The include is already above, so doing it again here should be harmless */
/* [Pair] */
#include <Corrade/Containers/PairStl.h>

DOXYGEN_ELLIPSIS()

std::pair<float, int> a{35.0f, 7};
Containers::Pair<float, int> b{a};

std::pair<bool, int*> c(Containers::pair(false, &b.second()));

auto d = Containers::pair(std::pair<char, double>{'p', 3.14});
    // d is Containers::pair<char, double>
/* [Pair] */
static_cast<void>(c);
static_cast<void>(d);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [Triple] */
#include <Corrade/Containers/TripleStl.h>

DOXYGEN_ELLIPSIS()

std::tuple<float, int, bool> a{35.0f, 7, true};
Containers::Triple<float, int, bool> b{a};

std::tuple<bool, int*, bool> c(Containers::triple(false, &b.second(), true));

auto d = Containers::triple(std::tuple<char, double, bool>{'p', 3.14, true});
    // d is Containers::triple<char, double, bool>
/* [Triple] */
static_cast<void>(c);
static_cast<void>(d);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [Pointer] */
#include <Corrade/Containers/PointerStl.h>

DOXYGEN_ELLIPSIS()

std::unique_ptr<int> a{new int{5}};
Containers::Pointer<int> b = std::move(a);

std::unique_ptr<int> c = Containers::pointer<int>(12);

auto d = Containers::pointer(std::unique_ptr<int>{new int{5}});
        // d is Containers::Pointer<int>
/* [Pointer] */
}

{
/* The include is already above, so doing it again here should be harmless */
/* [StringView] */
#include <Corrade/Containers/StringStl.h>

DOXYGEN_ELLIPSIS()

using namespace Containers::Literals;

std::string a = "Hello\0world!"_s;

Containers::MutableStringView b = a;
b[5] = ' ';
/* [StringView] */
}

{
/* The include is already above, so doing it again here should be harmless */
/* [String] */
#include <Corrade/Containers/StringStl.h>

DOXYGEN_ELLIPSIS()

std::string a = "Hello world!";
Containers::String b = a.substr(5);
/* [String] */
}

{
/* The includes are already above, so doing it again here should be harmless */
/* [StringIterable] */
#include <Corrade/Containers/ArrayViewStl.h>
#include <Corrade/Containers/StringStl.h>

DOXYGEN_ELLIPSIS()

std::vector<std::string> a{"hello", "world", "!"};
Containers::StringIterable b = a;
/* [StringIterable] */
static_cast<void>(b);
}

{
/* The include is already above, so doing it again here should be harmless */
/* [Reference] */
#include <Corrade/Containers/ReferenceStl.h>

DOXYGEN_ELLIPSIS()

int a = 1337;
Containers::Reference<int> b = a;

std::reference_wrapper<int> c = b;
Containers::Reference<const int> d = std::cref(a);
/* [Reference] */
static_cast<void>(c);
static_cast<void>(d);
}
}
