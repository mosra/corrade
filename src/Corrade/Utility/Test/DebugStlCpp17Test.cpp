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

#include <sstream>
/* __has_include is supported since C++17, but not all C++17 claiming STL
   implementations might have <filesystem> yet */
#if __has_include(<filesystem>)
#include <filesystem>
#endif

#include "Corrade/TestSuite/Tester.h"
#include "Corrade/Utility/DebugStl.h"
#include "Corrade/Utility/DebugStlStringView.h"

namespace Corrade { namespace Utility { namespace Test { namespace {

struct DebugStlCpp17Test: TestSuite::Tester {
    explicit DebugStlCpp17Test();

    void stringView();
    void stringViewEmpty();

    void filesystemPath();
};

DebugStlCpp17Test::DebugStlCpp17Test() {
    addTests({&DebugStlCpp17Test::stringView,
              &DebugStlCpp17Test::stringViewEmpty,

              &DebugStlCpp17Test::filesystemPath});
}

using namespace std::string_view_literals;

void DebugStlCpp17Test::stringView() {
    std::ostringstream out;
    Debug{&out} << "hello\0world!"sv;
    CORRADE_COMPARE(out.str(), (std::string{"hello\0world!\n", 13}));
}

void DebugStlCpp17Test::stringViewEmpty() {
    std::ostringstream out;
    /* Empty string view should not cause any issues with data access */
    Debug{&out} << "hello" << std::string_view{} << "!";
    CORRADE_COMPARE(out.str(), "hello  !\n");
}

void DebugStlCpp17Test::filesystemPath() {
    #if __has_include(<filesystem>)
    std::ostringstream out;
    /* This type is very special because it has a begin() / end() that return
       std::filesystem::path *again*, so Debug helpfully assumes it's a nested
       iterable container and then dies on infinite recursion.

       Instead, it has to be treated as string-like, but then there's an
       ambiguity between an implicit conversion to std::string and a builtin
       iostream operator<<. For that the operator<<(Debug&, const std::string&)
       had to get changed to not get selected if a type convertible to a
       std::string has an iostream operator<< as well -- the assumption is that
       using the operator<<() is cheaper since it doesn't require allocating a
       string copy. */
    Debug{&out} << std::filesystem::path("/home/mosra");
    /* https://en.cppreference.com/w/cpp/filesystem/path/operator_ltltgtgt as
       well as https://isocpp.org/files/papers/P0218r1.html (2016) says the
       operator<< prints the path quoted. I see no practical advantage of doing
       that (it's like if floats were printed with the `f` suffix always and
       you couldn't get rid of that), but that's secondary. Worse is that MSVC
       as well as MinGW follows this, while both libc++ and libstdc++ on both
       Linux and macOS don't, causing a nasty inconsistency. The reason could
       be that earlier proposals didn't specify this because std::quoted (from
       C++14) was not a thing yet, such as this one from 2012:
       https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3431.html

       Nevertheless. These two comments alone are for me a serious enough
       reason to never even bother using this library. Not even once. */
    #if (defined(CORRADE_TARGET_LIBCXX) || defined(CORRADE_TARGET_LIBSTDCXX)) && !defined(CORRADE_TARGET_WINDOWS)
    CORRADE_COMPARE(out.str(), "/home/mosra\n");
    #else
    CORRADE_COMPARE(out.str(), "\"/home/mosra\"\n");
    #endif
    #else
    CORRADE_SKIP("No <filesystem> header in this STL implementation.");
    #endif
}

}}}}

CORRADE_TEST_MAIN(Corrade::Utility::Test::DebugStlCpp17Test)
